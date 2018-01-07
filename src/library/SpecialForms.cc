// (c) Sam Donow 2017
#include "SpecialForms.h"
#include "Evaluator.h"

void SpecialForms::insertIntoScope(SymbolTable& st) {
    st.emplace("lambda", &SpecialForms::lambdaImpl);
    st.emplace("named-lambda", &SpecialForms::namedLambdaImpl);
    st.emplace("if", &SpecialForms::ifImpl);
    st.emplace("define", &SpecialForms::defineImpl);
    st.emplace("quote", &SpecialForms::quoteImpl);
    st.emplace("and", &SpecialForms::andImpl);
    st.emplace("or", &SpecialForms::orImpl);
    st.emplace("begin", &SpecialForms::beginImpl);
    st.emplace("cond", &SpecialForms::condImpl);
}

std::pair<std::vector<Symbol>, Datum> parseFuncDefn(LispArgs args) {
    std::vector<Symbol> formals;
    if (args.size() < 2) {
        throw LispError("Definition must have param list and body");
    }

    auto inputIt = args.begin();
    if (inputIt->isAtomic()) {
        throw LispError("Parameter list must be a list");
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

    return {formals, *++inputIt};
}

EvalResult SpecialForms::lambdaImpl(LispArgs args, SymbolTable& st, Evaluator&) {
    auto[formals, defn] = parseFuncDefn(std::move(args));
    SExprPtr impl =
        defn.isAtomic() ? std::make_shared<SExpr>(defn.getAtom()) : defn.getSExpr();
    auto &elem = st.emplaceAnon(LispFunction{std::move(formals), std::move(impl), st});
    return Datum{Atom{std::shared_ptr<LispFunction>(st.shared_from_this(), &std::get<LispFunction>(elem))}};
}

EvalResult SpecialForms::namedLambdaImpl(LispArgs args, SymbolTable& st, Evaluator&) {
    auto[formals, defn] = parseFuncDefn(std::move(args));
    if (formals.empty()) {
        throw LispError("Named lambda must have name");
    }
    Symbol funName = std::move(*formals.begin());
    formals.erase(formals.begin());

    SExprPtr impl =
        defn.isAtomic() ? std::make_shared<SExpr>(defn.getAtom()) : defn.getSExpr();
    auto &elem = st.emplace(+funName, LispFunction{std::move(formals), std::move(impl), st});
    return Datum{Atom{std::shared_ptr<LispFunction>(st.shared_from_this(), &std::get<LispFunction>(elem))}};
}

EvalResult SpecialForms::defineImpl(LispArgs args, SymbolTable& st, Evaluator& ev) {
    auto inputIt = args.begin();
    if (inputIt->isAtomic()) {
        // We are defining a constant
        std::optional<Symbol> varName = inputIt->getAtomicValue<Symbol>();
        if (!varName) {
            throw LispError("Name ", *inputIt, " is not an identifier");
        }
        ++inputIt;
        Datum value = ev.evalDatum(inputIt->getSExpr(), st);
        st.emplace(+*varName, value);
        return Datum{};
    }
    auto [formals, defn] = parseFuncDefn(std::move(args));
    if (formals.empty()) {
        throw LispError("Function definition must have name");
    }
    Symbol funName = std::move(*formals.begin());
    formals.erase(formals.begin());

    SExprPtr impl =
        defn.isAtomic() ? std::make_shared<SExpr>(defn.getAtom()) : defn.getSExpr();
    st.emplace(+funName, LispFunction{std::move(formals), std::move(impl), st});
    return Datum{};
}

EvalResult SpecialForms::ifImpl(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 3) {
        throw LispError("if takes 3 arguments, found ", args.size());
    }
    auto inputIt = args.begin();
    Datum cond = ev.computeArg(*inputIt, st);
    ++inputIt;
    if (!cond.isTrue()) {
        ++inputIt;
    }
    return ev.computeArgResult(*inputIt, st);
}

EvalResult SpecialForms::quoteImpl(LispArgs args, SymbolTable&, Evaluator&) {
    if (args.size() != 1) {
        throw LispError("quote requires exactly one argument");
    }
    return Datum{args.begin()->getSExpr()};
}

EvalResult SpecialForms::andImpl(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return Datum{Atom{true}};
    }
    Datum ret;
    for (const Datum& datum : args) {
        ret = ev.computeArg(datum, st);
        if (!ret.isTrue()) {
            return ret;
        }
    }
    return ret;
}

EvalResult SpecialForms::orImpl(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return Datum{Atom{false}};
    }
    Datum ret;
    for (const Datum& datum : args) {
        ret = ev.computeArg(datum, st);
        if (ret.isTrue()) {
            return ret;
        }
    }
    return ret;
}

EvalResult SpecialForms::beginImpl(LispArgs args, SymbolTable& st, Evaluator& ev) {
    EvalResult ret;
    for (const Datum& datum : args) {
        ret = ev.computeArgResult(datum, st);
    }
    return ret;
}

EvalResult SpecialForms::condImpl(LispArgs args, SymbolTable& st, Evaluator& ev) {
    for (const Datum& datum : args) {
        if (unlikely(datum.isAtomic())) {
            throw LispError("Condition clauses must be pairs");
        }
        const SExprPtr& condPair = datum.getSExpr();
        if (auto sym = condPair->car.getAtomicValue<Symbol>();
            (sym && +*sym == "else") || ev.computeArg(condPair->car, st).isTrue()) {
            return beginImpl(condPair->cdr.getSExpr(), st, ev);
        }
    }
    // ret value unspecified if all conds false and no else
    return Datum{};
}
