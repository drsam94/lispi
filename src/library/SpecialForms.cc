// (c) Sam Donow 2017
#include "SpecialForms.h"
#include "Evaluator.h"

void SpecialForms::insertIntoScope(SymbolTable& st) {
    st.emplace("lambda", &SpecialForms::lambdaImpl);
    st.emplace("if", &SpecialForms::ifImpl);
    st.emplace("define", &SpecialForms::defineImpl);
    st.emplace("quote", &SpecialForms::quoteImpl);
}

Datum SpecialForms::lambdaImpl(const SExprPtr& inputs,
                               const std::shared_ptr<SymbolTable>& st) {
    std::vector<Symbol> formals;
    if (inputs->size() != 2) {
        throw LispError("Lambda must have param list and body");
    }

    auto inputIt = inputs->begin();
    if (inputIt->isAtomic()) {
        throw LispError("Lambda parameter list must be a list");
    }
    const SExprPtr& expr = inputIt->getSExpr();
    for (const Datum& datum : *expr) {
        std::optional<Symbol> param = datum.getAtomicValue<Symbol>();
        if (!param) {
            throw LispError("Formal parameter ", expr->car,
                            " is not an identifier");
        } else {
            formals.emplace_back(std::move(*param));
        }
    }

    ++inputIt;
    const Datum& defn = *inputIt;
    std::shared_ptr<SExpr> impl = defn.isAtomic()
                                      ? std::make_shared<SExpr>(defn.getAtom())
                                      : defn.getSExpr();
    return {Atom{LispFunction{std::move(formals), impl, st, true}}};
}

Datum SpecialForms::defineImpl(const SExprPtr& inputs,
                               const std::shared_ptr<SymbolTable>& st) {
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
        std::optional<Datum> value = Evaluator::eval(inputIt->getSExpr(), st);
        if (!value) {
            throw LispError("Invalid variable definition ", *inputIt);
        }
        st->emplace(+*varName, *value);
        return {};
    }
    const SExprPtr& declarator = inputIt->getSExpr();
    std::optional<Symbol> funName = declarator->car.getAtomicValue<Symbol>();
    if (!funName) {
        throw LispError("Name ", declarator->car, " is not an identifier");
    }
    std::vector<Symbol> formals;
    for (const Datum& datum : *declarator->cdr) {
        std::optional<Symbol> param = datum.getAtomicValue<Symbol>();
        if (!param) {
            throw LispError("Name ", datum, " is not an identifier");
        }
        formals.emplace_back(std::move(*param));
    }
    ++inputIt;
    const Datum& defn = *inputIt;
    std::shared_ptr<SExpr> impl = defn.isAtomic()
                                      ? std::make_shared<SExpr>(defn.getAtom())
                                      : defn.getSExpr();
    st->emplace(+*funName,
                Datum{Atom{LispFunction{std::move(formals), impl, st, false}}});
    return {};
}

Datum SpecialForms::ifImpl(const SExprPtr& inputs,
                           const std::shared_ptr<SymbolTable>& st) {
    if (inputs->size() != 3) {
        throw LispError("if takes 3 arguments, found ", inputs->size());
    }
    auto inputIt = inputs->begin();
    Datum cond = Evaluator::computeArg(*inputIt, st);
    ++inputIt;
    if (!cond.isTrue()) {
        ++inputIt;
    }
    return Evaluator::computeArg(*inputIt, st);
}

Datum SpecialForms::quoteImpl(const SExprPtr& inputs,
                              const std::shared_ptr<SymbolTable>&) {
    if (inputs->size() != 1) {
        throw LispError("quote requires exactly one argument");
    }
    return Datum{inputs->car.getSExpr()};
}
