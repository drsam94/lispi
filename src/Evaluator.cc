// (c) 2017 Sam Donow
#include "Evaluator.h"
#include "util/Util.h"

Evaluator::Evaluator() : globalScope(std::make_shared<SymbolTable>(nullptr)) {
    // TODO: have a distinciton between true "special forms" (i.e if, quote)
    // and library-defined functions (like "+")
    globalScope->emplace("+", &Evaluator::builtinAdd);
    globalScope->emplace("lambda", &Evaluator::builtinLambdaSF);
    globalScope->emplace("if", &Evaluator::builtinIfSF);
    globalScope->emplace("nil", Datum{nullptr});
}

template <typename T>
std::optional<T> Evaluator::getOrEvaluate(const Datum &datum,
                                          std::shared_ptr<SymbolTable> st) {
    return std::visit(
        Visitor{[&](const Atom &val) -> std::optional<T> {
                    if constexpr(!std::is_same_v<T, Symbol>) {
                        if (val.contains<Symbol>()) {
                            // TODO: write helpers so this is less disgusting
                            return Evaluator::getOrEvaluate<T>(std::get<Datum>(
                                (*st)[+val.get<Symbol>()]), st);
                        }
                    }
                    return val.get<T>();
                },
                [&](const std::shared_ptr<SExpr> expr) -> std::optional<T> {
                    auto result = Evaluator::eval(*expr, st);
                    if (!result)
                        return std::nullopt;
                    return Evaluator::getOrEvaluate<T>(*result, st);
                }},
        datum.data);
}

/// Template specialization for just getting or evaluator a raw datum
template <>
std::optional<Datum> Evaluator::getOrEvaluate(const Datum &datum,
                                              std::shared_ptr<SymbolTable> st) {
    return std::visit(
        Visitor{[&](const Atom &val) -> std::optional<Datum> {
                    if (val.contains<Symbol>()) {
                        // This won't DTRT if you have a symbol which
                        // maps to an SExpr (but not a fn)
                        return getOrEvaluate<Datum>(
                            std::get<Datum>((*st)[+val.get<Symbol>()]),
                            st);
                    }
                    return Datum{val};
                },
                [&](const std::shared_ptr<SExpr> expr) -> std::optional<Datum> {
                    if (expr == nullptr) {
                        return Datum{expr};
                    } else {
                        return Evaluator::eval(*expr, st);
                    }
                }},
        datum.data);
}

std::optional<Datum>
Evaluator::evalFunction(const LispFunction &func, const std::list<Datum> &args,
                        std::shared_ptr<SymbolTable> scope) {
    // TODO: some dialects of lisp support optional and variadic parameters...
    if (func.formalParameters.size() != args.size()) {
        return std::nullopt;
    }
    auto formalIt = func.formalParameters.begin();
    auto actualIt = args.begin();
    auto funcScope = func.defnScope->makeChild();
    for (; formalIt != func.formalParameters.end() && actualIt != args.end();
        ++formalIt, ++actualIt) {
        auto result = getOrEvaluate<Datum>(*actualIt, scope);
        if (!result) {
            return std::nullopt;
        }
        funcScope->emplace(+*formalIt, *result);
    }
    if (func.definition->cdr.empty()) {
        // Workaround to support things like lambda (x) x...this is probably not
        // fully compliant with the spec
        auto sym = func.definition->car.getAtomicValue<Symbol>();
        if (sym) {
            return std::get<Datum>((*funcScope)[+*sym]);
        }
        return getOrEvaluate<Datum>(func.definition->car, funcScope);
    } else {
        return eval(*func.definition, funcScope);
    }
}


std::optional<Datum>
Evaluator::eval(const SExpr &expr,
                std::shared_ptr<SymbolTable> scope) {
    auto sym = expr.car.getAtomicValue<Symbol>();
    if (sym) {
        const auto &scopeElem = (*scope)[+*sym];
        if (std::holds_alternative<SpecialForm>(scopeElem)) {
            return std::get<SpecialForm>(scopeElem)(expr.cdr, scope);
        }

        auto func = std::get<Datum>(scopeElem).getAtomicValue<LispFunction>();
        if (!func) {
            return std::nullopt;
        }

        return evalFunction(*func, expr.cdr, scope);
    }

    if (!expr.car.isAtomic()) {
        auto carFunc = eval(*expr.car.getSExpr(), scope);
        if (!carFunc) {
            return std::nullopt;
        }
        auto func = carFunc->getAtomicValue<LispFunction>();
        if (!func) {
            return std::nullopt;
        }

        return evalFunction(*func, expr.cdr, scope);
    } else {
        return std::nullopt;
    }
}

Datum Evaluator::builtinAdd(const std::list<Datum> &inputs,
                 std::shared_ptr<SymbolTable> st) {
    Number sum{};
    for (const Datum &datum : inputs) {
        if (auto val = Evaluator::getOrEvaluate<Number>(datum, st); bool(val)) {
            sum += *val;
        } else {
            throw "TODO: a structured runtime error";
        }
    }
    return {Atom(sum)};
}

Datum Evaluator::builtinLambdaSF(const std::list<Datum> &inputs,
                      std::shared_ptr<SymbolTable> st) {
    std::vector<Symbol> formals;
    if (inputs.size() != 2) {
        throw "TODO: a structured runtime error";
    }

    auto inputIt = inputs.begin();
    if (inputIt->isAtomic()) {
        throw "TODO: a structured runtime error";
    }
    const auto &expr = inputIt->getSExpr();
    // TODO: allow easier flat iteration over an sexpr
    auto param = expr->car.getAtomicValue<Symbol>();
    if (!param) {
        throw "TODO: a structured runtime error";
    } else {
        formals.emplace_back(std::move(*param));
    }
    for (const Datum &datum : expr->cdr) {
        auto nextParam = datum.getAtomicValue<Symbol>();
        if (!nextParam) {
            throw "TODO: a structured runtime error";
        } else {
            formals.emplace_back(std::move(*nextParam));
        }
    }

    ++inputIt;
    const auto &defn = *inputIt;
    std::shared_ptr<SExpr> impl = defn.isAtomic()
                                      ? std::make_shared<SExpr>(defn.getAtom())
                                      : defn.getSExpr();
    return {Atom{LispFunction{std::move(formals), impl, st}}};
}

Datum Evaluator::builtinIfSF(const std::list<Datum> &inputs,
                             std::shared_ptr<SymbolTable> st) {
    if (inputs.size() != 3) {
        throw "TODO: a structured runtime error";
    }
    auto inputIt = inputs.begin();
    auto cond = getOrEvaluate<Datum>(*inputIt, st);
    if (!cond) {
        throw "TODO: a structured runtime error";
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
        throw "TODO: a structured runtime error";
    }
}
