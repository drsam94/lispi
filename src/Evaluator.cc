// (c) 2017 Sam Donow
#include "Evaluator.h"
#include "util/Util.h"
#include "library/SpecialForms.h"

Evaluator::Evaluator() : globalScope(std::make_shared<SymbolTable>(nullptr)) {
    // TODO: have a distinciton between true "special forms" (i.e if, quote)
    // and library-defined functions (like "+")

    // Procedures
    globalScope->emplace("+", &Evaluator::builtinAdd);
    globalScope->emplace("-", &Evaluator::builtinSub);
    globalScope->emplace("*", &Evaluator::builtinMul);
    globalScope->emplace("car", &Evaluator::builtinCar);
    globalScope->emplace("cdr", &Evaluator::builtinCdr);
    globalScope->emplace("list", &Evaluator::builtinList);
    globalScope->emplace("eq?", &Evaluator::builtinEqQ);
    globalScope->emplace("null?", &Evaluator::builtinNullQ);
    // Special Forms
    SpecialForms::insertIntoScope(*globalScope);
    // Special Globals
    globalScope->emplace("#t", Datum{Atom{true}});
    globalScope->emplace("#f", Datum{Atom{false}});
}

template <typename T>
std::optional<T>
Evaluator::getOrEvaluate(const Datum& datum,
                         const std::shared_ptr<SymbolTable>& st) {
    if (datum.isAtomic()) {
        const Atom& val = datum.getAtom();
        if constexpr (!std::is_same_v<T, Symbol>) {
            if (val.contains<Symbol>()) {
                return getOrEvaluate<T>(std::get<Datum>(st->get(val)), st);
            }
        }
        return val.get<T>();
    } else {
        const auto& expr = datum.getSExpr();
        const std::optional<Datum> result = eval(expr, st);
        if (!result)
            return std::nullopt;
        return getOrEvaluate<T>(*result, st);
    }
}

Datum Evaluator::computeArg(const Datum& datum,
                                   const std::shared_ptr<SymbolTable>& st) {
    if (datum.isAtomic()) {
        const Atom& val = datum.getAtom();
        if (val.contains<Symbol>()) {
            // Note: this is where we need the SpecialForm/BuiltinFunc distinction;
            // this should be able to return, e.g "+", but not "if"
            return computeArg(std::get<Datum>(st->get(val)), st);
        }
        return Datum{val};
    } else {
        const SExprPtr& expr = datum.getSExpr();
        if (expr == nullptr) {
            return Datum{expr};
        } else {
            std::optional<Datum> ret = eval(expr, st);
            if (!ret) {
                throw LispError("Failed to evaluate argument");
            }
            return *ret;
        }
    }
}

std::optional<Datum>
Evaluator::evalFunction(const LispFunction& func, const SExprPtr& args,
                        const std::shared_ptr<SymbolTable>& scope) {
    // TODO: some dialects of lisp support optional and variadic parameters...
    if (func.formalParameters.size() != args->size()) {
        return std::nullopt;
    }
    auto formalIt = func.formalParameters.begin();
    auto actualIt = args->begin();
    auto funcScope = func.funcScope();
    for (; formalIt != func.formalParameters.end() && actualIt != args->end();
        ++formalIt, ++actualIt) {
        Datum result = computeArg(*actualIt, scope);
        funcScope->emplace(+*formalIt, result);
    }
    if (func.definition->cdr == nullptr) {
        // Workaround to support things like lambda (x) x...this is probably not
        // fully compliant with the spec
        auto sym = func.definition->car.getAtomicValue<Symbol>();
        if (sym) {
            return std::get<Datum>(funcScope->get(*sym));
        }
        return computeArg(func.definition->car, funcScope);
    } else {
        return eval(func.definition, funcScope);
    }
}

std::optional<Datum>
Evaluator::eval(const std::shared_ptr<SExpr> &expr,
                const std::shared_ptr<SymbolTable>& scope) {
    auto sym = expr->car.getAtomicValue<Symbol>();
    if (sym) {
        const auto &scopeElem = (*scope)[+*sym];
        if (std::holds_alternative<SpecialForm>(scopeElem)) {
            return std::get<SpecialForm>(scopeElem)(expr->cdr, scope);
        }

        const auto &func = std::get<Datum>(scopeElem).getAtomicValue<LispFunction>();
        if (!func) {
            return std::nullopt;
        }

        return evalFunction(*func, expr->cdr, scope);
    }

    if (!expr->car.isAtomic()) {
        const auto &carFunc = eval(expr->car.getSExpr(), scope);
        if (!carFunc) {
            return std::nullopt;
        }
        const auto &func = carFunc->getAtomicValue<LispFunction>();
        if (!func) {
            return std::nullopt;
        }

        return evalFunction(*func, expr->cdr, scope);
    } else {
        return std::nullopt;
    }
}

Datum Evaluator::builtinAdd(const SExprPtr& inputs,
                            const std::shared_ptr<SymbolTable>& st) {
    Number sum{};
    for (const Datum &datum : *inputs) {
        if (auto val = Evaluator::getOrEvaluate<Number>(datum, st); bool(val)) {
            sum += *val;
        } else {
            throw LispError("TypeError: ", datum, " is not a number");
        }
    }
    return {Atom{sum}};
}

Datum Evaluator::builtinSub(const SExprPtr& inputs,
                            const std::shared_ptr<SymbolTable>& st) {
    Number diff{};
    for (auto it = inputs->begin(); it != inputs->end(); ++it) {
        if (auto val = Evaluator::getOrEvaluate<Number>(*it, st); bool(val)) {
            if (it == inputs->begin() && inputs->size() > 1) {
                diff = *val;
            } else {
                diff -= *val;
            }
        } else {
            throw LispError("TypeError: ", *it, " is not a number");
        }
    }
    return {Atom{diff}};
}

Datum Evaluator::builtinMul(const SExprPtr& inputs,
                            const std::shared_ptr<SymbolTable>& st) {
    Number prod{1L};
    for (const Datum& datum : *inputs) {
        if (auto val = Evaluator::getOrEvaluate<Number>(datum, st); bool(val)) {
            prod *= *val;
        } else {
            throw LispError("TypeError: ", datum, " is not a number");
        }
    }
    return {Atom{prod}};
}

Datum Evaluator::builtinCar(const SExprPtr& inputs, const std::shared_ptr<SymbolTable>& st) {
    Datum arg = computeArg(inputs->car, st);
    if (arg.isAtomic()) {
        throw LispError("car requires a cons cell");
    }
    return arg.getSExpr()->car;
}

Datum Evaluator::builtinCdr(const SExprPtr& inputs, const std::shared_ptr<SymbolTable>& st) {
    Datum arg = computeArg(inputs->car, st);
    if (arg.isAtomic()) {
        throw LispError("cdr requires a cons cell");
    }
    return Datum{arg.getSExpr()->cdr};
}

Datum Evaluator::builtinEqQ(const SExprPtr& inputs, const std::shared_ptr<SymbolTable>& st) {
    if (inputs->size() != 2) {
        throw LispError("Function expects 2 arguments, received ", inputs->size());
    }

    auto it = inputs->begin();
    Datum first = computeArg(*it, st);
    ++it;
    Datum second = computeArg(*it, st);
    return Datum{Atom{first == second}};
}

Datum Evaluator::builtinList(const SExprPtr& inputs, const std::shared_ptr<SymbolTable>& st) {
    SExprPtr ret = std::make_shared<SExpr>(nullptr);
    SExpr* curr = ret.get();
    bool first = true;
    for (const Datum& datum : *inputs) {
        if (!first) {
            curr->cdr = std::make_shared<SExpr>(nullptr);
            curr = curr->cdr.get();
        }
        curr->car = computeArg(datum, st);
        first = false;
    }
    return Datum{ret};
}

Datum Evaluator::builtinNullQ(const SExprPtr& inputs, const std::shared_ptr<SymbolTable>& st) {
    if (inputs->cdr != nullptr) {
        throw LispError("null? expects only 1 argument");
    }
    Datum arg = computeArg(inputs->car, st);
    if (arg.isAtomic()) {
        return Datum{Atom{false}};
    }
    const bool isNull = arg.getSExpr() == nullptr;
    return Datum{Atom{isNull}};
}
