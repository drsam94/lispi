// (c) Sam Donow 2017
#include "SpecialForms.h"
#include "Evaluator.h"

void SpecialForms::insertIntoScope(SymbolTable& st) {
    st.emplace("lambda", &SpecialForms::lambdaImpl);
    st.emplace("if", &SpecialForms::ifImpl);
    st.emplace("define", &SpecialForms::defineImpl);
    st.emplace("quote", &SpecialForms::quoteImpl);
    st.emplace("and", &SpecialForms::andImpl);
    st.emplace("or", &SpecialForms::orImpl);
    st.emplace("begin", &SpecialForms::beginImpl);
}

Datum SpecialForms::lambdaImpl(LispArgs args, SymbolTable& st) {
    std::vector<Symbol> formals;
    if (args.size() < 2) {
        throw LispError("Lambda must have param list and body");
    }

    auto inputIt = args.begin();
    if (inputIt->isAtomic()) {
        throw LispError("Lambda parameter list must be a list");
    }
    const SExprPtr& expr = inputIt->getSExpr();
    if (expr != nullptr) {
        for (const Datum& datum : *expr) {
            std::optional<Symbol> param = datum.getAtomicValue<Symbol>();
            if (!param) {
                throw LispError("Formal parameter ", expr->car,
                                " is not an identifier");
            } else {
                formals.emplace_back(std::move(*param));
            }
        }
    }

    ++inputIt;
    const Datum& defn = *inputIt;
    std::shared_ptr<SExpr> impl = defn.isAtomic()
                                      ? std::make_shared<SExpr>(defn.getAtom())
                                      : defn.getSExpr();
    return {Atom{LispFunction{std::move(formals), impl, st, true}}};
}

Datum SpecialForms::defineImpl(LispArgs args, SymbolTable& st) {
    if (args.size() < 2) {
        throw LispError("Function definition requires declarator and body");
    }
    auto inputIt = args.begin();
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
        st.emplace(+*varName, *value);
        return {};
    }
    const SExprPtr& declarator = inputIt->getSExpr();
    std::optional<Symbol> funName = declarator->car.getAtomicValue<Symbol>();
    if (!funName) {
        throw LispError("Name ", declarator->car, " is not an identifier");
    }
    std::vector<Symbol> formals;
    for (const Datum& datum : LispArgs(declarator->cdr)) {
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
    st.emplace(+*funName,
                Datum{Atom{LispFunction{std::move(formals), impl, st, false}}});
    return {};
}

Datum SpecialForms::ifImpl(LispArgs args, SymbolTable& st) {
    if (args.size() != 3) {
        throw LispError("if takes 3 arguments, found ", args.size());
    }
    auto inputIt = args.begin();
    Datum cond = Evaluator::computeArg(*inputIt, st);
    ++inputIt;
    if (!cond.isTrue()) {
        ++inputIt;
    }
    return Evaluator::computeArg(*inputIt, st);
}

Datum SpecialForms::quoteImpl(LispArgs args, SymbolTable&) {
    if (args.size() != 1) {
        throw LispError("quote requires exactly one argument");
    }
    return Datum{args.begin()->getSExpr()};
}

Datum SpecialForms::andImpl(LispArgs args, SymbolTable& st) {
    if (args.empty()) {
        return {Atom{true}};
    }
    Datum ret;
    for (const Datum& datum : args) {
        ret = Evaluator::computeArg(datum, st);
        if (!ret.isTrue()) {
            return ret;
        }
    }
    return ret;
}

Datum SpecialForms::orImpl(LispArgs args, SymbolTable& st) {
    if (args.empty()) {
        return {Atom{false}};
    }
    Datum ret;
    for (const Datum& datum : args) {
        ret = Evaluator::computeArg(datum, st);
        if (ret.isTrue()) {
            return ret;
        }
    }
    return ret;
}

Datum SpecialForms::beginImpl(LispArgs args, SymbolTable& st) {
    Datum ret;
    for (const Datum& datum : args) {
        ret = Evaluator::computeArg(datum, st);
    }
    return ret;
}
