#include "Evaluator.h"
// (c) 2017 Sam Donow
#include "util/Util.h"

Evaluator::Evaluator() : globalScope(std::make_shared<SymbolTable>(nullptr)) {
    // TODO: have a distinciton between true "special forms" (i.e if, quote)
    // and library-defined functions (like "+")
    globalScope->emplace("+", &Evaluator::builtinAdd);
    globalScope->emplace("-", &Evaluator::builtinSub);
    globalScope->emplace("*", &Evaluator::builtinMul);
    globalScope->emplace("lambda", &Evaluator::builtinLambdaSF);
    globalScope->emplace("if", &Evaluator::builtinIfSF);
    globalScope->emplace("define", &Evaluator::builtinDefineSF);
    globalScope->emplace("nil", Datum{nullptr});
    globalScope->emplace("#t", Datum{Atom{true}});
    globalScope->emplace("#f", Datum{Atom{false}});
    globalScope->emplace("quote", &Evaluator::builtinQuoteSF);
    globalScope->emplace("car", &Evaluator::builtinCarSF);
    globalScope->emplace("cdr", &Evaluator::builtinCdrSF);
    globalScope->emplace("eq?", &Evaluator::builtinEqSF);
}

template <typename T>
std::optional<T> Evaluator::getOrEvaluate(const Datum& datum,
                                          std::shared_ptr<SymbolTable> st) {
    if (datum.isAtomic()) {
        const Atom &val = datum.getAtom();
        if constexpr (!std::is_same_v<T, Symbol>) {
            if (val.contains<Symbol>()) {
                // TODO: write helpers so this is less disgusting
                return Evaluator::getOrEvaluate<T>(
                    std::get<Datum>((*st)[+val.get<Symbol>()]), st);
            }
        }
        return val.get<T>();
    } else {
        const auto& expr = datum.getSExpr();
        const std::optional<Datum> result = Evaluator::eval(expr, st);
        if (!result)
            return std::nullopt;
        return Evaluator::getOrEvaluate<T>(*result, st);
    }
}

/// Template specialization for just getting or evaluator a raw datum
template <>
std::optional<Datum> Evaluator::getOrEvaluate(const Datum &datum,
                                              std::shared_ptr<SymbolTable> st) {
    if (datum.isAtomic()) {
        const Atom &val = datum.getAtom();
            if (val.contains<Symbol>()) {
                // TODO: write helpers so this is less disgusting
                return Evaluator::getOrEvaluate<Datum>(
                    std::get<Datum>((*st)[+val.get<Symbol>()]), st);
            }
        return Datum{val};
    } else {
        const auto &expr = datum.getSExpr();
        if (expr == nullptr) {
            return Datum{expr};
        } else {
            return Evaluator::eval(expr, st);
        }
    }
}

std::optional<Datum>
Evaluator::evalFunction(const LispFunction& func, const SExprPtr& args,
                        std::shared_ptr<SymbolTable> scope) {
    // TODO: some dialects of lisp support optional and variadic parameters...
    if (func.formalParameters.size() != args->size()) {
        return std::nullopt;
    }
    auto formalIt = func.formalParameters.begin();
    auto actualIt = args->begin();
    auto funcScope = func.funcScope();
    for (; formalIt != func.formalParameters.end() && actualIt != args->end();
        ++formalIt, ++actualIt) {
        auto result = getOrEvaluate<Datum>(*actualIt, scope);
        if (!result) {
            return std::nullopt;
        }
        funcScope->emplace(+*formalIt, *result);
    }
    if (func.definition->cdr == nullptr) {
        // Workaround to support things like lambda (x) x...this is probably not
        // fully compliant with the spec
        auto sym = func.definition->car.getAtomicValue<Symbol>();
        if (sym) {
            return std::get<Datum>((*funcScope)[+*sym]);
        }
        return getOrEvaluate<Datum>(func.definition->car, funcScope);
    } else {
        return eval(func.definition, funcScope);
    }
}

std::optional<Datum>
Evaluator::eval(const std::shared_ptr<SExpr> &expr,
                std::shared_ptr<SymbolTable> scope) {
    auto sym = expr->car.getAtomicValue<Symbol>();
    if (sym) {
        const auto &scopeElem = (*scope)[+*sym];
        if (std::holds_alternative<SpecialForm>(scopeElem)) {
            return std::get<SpecialForm>(scopeElem)(expr->cdr, scope);
        }

        auto func = std::get<Datum>(scopeElem).getAtomicValue<LispFunction>();
        if (!func) {
            return std::nullopt;
        }

        return evalFunction(*func, expr->cdr, scope);
    }

    if (!expr->car.isAtomic()) {
        auto carFunc = eval(expr->car.getSExpr(), scope);
        if (!carFunc) {
            return std::nullopt;
        }
        auto func = carFunc->getAtomicValue<LispFunction>();
        if (!func) {
            return std::nullopt;
        }

        return evalFunction(*func, expr->cdr, scope);
    } else {
        return std::nullopt;
    }
}

Datum Evaluator::builtinAdd(const SExprPtr& inputs,
                            std::shared_ptr<SymbolTable> st) {
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
                            std::shared_ptr<SymbolTable> st) {
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
                            std::shared_ptr<SymbolTable> st) {
    Number prod{1L};
    for (const Datum &datum : *inputs) {
        if (auto val = Evaluator::getOrEvaluate<Number>(datum, st); bool(val)) {
            prod *= *val;
        } else {
            throw LispError("TypeError: ", datum, " is not a number");
        }
    }
    return {Atom{prod}};
}

Datum Evaluator::builtinLambdaSF(const SExprPtr& inputs,
                      std::shared_ptr<SymbolTable> st) {
    std::vector<Symbol> formals;
    if (inputs->size() != 2) {
        throw LispError("Lambda must have param list and body");
    }

    auto inputIt = inputs->begin();
    if (inputIt->isAtomic()) {
        throw LispError("Lambda parameter list must be a list");
    }
    const SExprPtr &expr = inputIt->getSExpr();
    for (const Datum &datum : *expr) {
        std::optional<Symbol> param = datum.getAtomicValue<Symbol>();
        if (!param) {
            throw LispError("Formal parameter ", expr->car, " is not an identifier");
        } else {
            formals.emplace_back(std::move(*param));
        }
    }

    ++inputIt;
    const Datum &defn = *inputIt;
    std::shared_ptr<SExpr> impl = defn.isAtomic()
                                      ? std::make_shared<SExpr>(defn.getAtom())
                                      : defn.getSExpr();
    return {Atom{LispFunction{std::move(formals), impl, st, true}}};
}

Datum Evaluator::builtinDefineSF(const SExprPtr& inputs,
                        std::shared_ptr<SymbolTable> st) {
    if (inputs->size() != 2) {
        throw LispError("Function definition requires declarator and body");
    }
    auto inputIt = inputs->begin();
    if (inputIt->isAtomic()) {
        // We are defining a constant
        std::optional<Symbol> varName = inputIt->getAtomicValue<Symbol>();
        if (!varName) {
            throw LispError("Name ", *inputIt, " is not an identifier");
        }
        ++inputIt;
        std::optional<Datum> value = eval(inputIt->getSExpr(), st);
        if (!value) {
            throw LispError("Invalid variable definition ", *inputIt);
        }
        st->emplace(+*varName, *value);
        return {};
    }
    const SExprPtr &declarator = inputIt->getSExpr();
    std::optional<Symbol> funName = declarator->car.getAtomicValue<Symbol>();
    if (!funName) {
        throw LispError("Name ", declarator->car, " is not an identifier");
    }
    std::vector<Symbol> formals;
    for (const Datum &datum : *declarator->cdr) {
        std::optional<Symbol> param = datum.getAtomicValue<Symbol>();
        if (!param) {
            throw LispError("Name ", datum, " is not an identifier");
        }
        formals.emplace_back(std::move(*param));
    }
    ++inputIt;
    const Datum &defn = *inputIt;
    std::shared_ptr<SExpr> impl = defn.isAtomic()
                                    ? std::make_shared<SExpr>(defn.getAtom())
                                    : defn.getSExpr();
    st->emplace(+*funName, Datum{Atom{LispFunction{std::move(formals), impl, st, false}}});
    return {};
}

Datum Evaluator::builtinIfSF(const SExprPtr& inputs,
                             std::shared_ptr<SymbolTable> st) {
    if (inputs->size() != 3) {
        throw LispError("if takes 3 arguments, found ", inputs->size());
    }
    auto inputIt = inputs->begin();
    std::optional<Datum> cond = getOrEvaluate<Datum>(*inputIt, st);
    if (!cond) {
        throw LispError("Error evaluating condition ", *inputIt);
    }
    if (cond->isNil()) {
        ++inputIt;
        ++inputIt;
    } else {
        ++inputIt;
    }
    if (auto ret = Evaluator::getOrEvaluate<Datum>(*inputIt, st); bool(ret)) {
        return *ret;
    } else {
        throw LispError("Error evaluating expression, ", *inputIt);
    }
}

Datum Evaluator::builtinQuoteSF(const SExprPtr& inputs, std::shared_ptr<SymbolTable>) {
    if (inputs->size() != 1) {
        throw LispError("quote requires exactly one argument");
    }
    return Datum{inputs->car.getSExpr()};
}

Datum Evaluator::builtinCarSF(const SExprPtr& inputs, std::shared_ptr<SymbolTable> st) {
    std::optional<Datum> arg = Evaluator::getOrEvaluate<Datum>(inputs->car, st);
    if (!arg || arg->isAtomic()) {
        throw LispError("car requires a cons cell");
    }
    return arg->getSExpr()->car;
}

Datum Evaluator::builtinCdrSF(const SExprPtr& inputs, std::shared_ptr<SymbolTable> st) {
    std::optional<Datum> arg = Evaluator::getOrEvaluate<Datum>(inputs->car, st);
    if (!arg || arg->isAtomic()) {
        throw LispError("cdr requires a cons cell");
    }
    return Datum{arg->getSExpr()->cdr};
}

Datum Evaluator::builtinEqSF(const SExprPtr& inputs, std::shared_ptr<SymbolTable> st) {
    if (inputs->size() != 2) {
        throw LispError("Function expects 2 arguments, received ", inputs->size());
    }

    auto it = inputs->begin();
    std::optional<Datum> first = Evaluator::getOrEvaluate<Datum>(*it, st);
    ++it;
    std::optional<Datum> second = Evaluator::getOrEvaluate<Datum>(*it, st);
    if (!first || !second) {
        throw LispError("Error evaluating arguments to eq");
    }

    return Datum{Atom{*first == *second}};
}

