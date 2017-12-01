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

Datum SpecialForms::lambdaImpl(LispArgs args, SymbolTable& st) {
    auto[formals, defn] = parseFuncDefn(std::move(args));
    SExprPtr impl =
        defn.isAtomic() ? std::make_shared<SExpr>(defn.getAtom()) : defn.getSExpr();
    return {Atom{LispFunction{std::move(formals), std::move(impl), st, true}}};
}
// TODO: how to do named lambda? We don't want to add a new child scope; so we probably need to
// connect the name with the scope, but we don't want to have a circular referenct with the
// function referring to a scope the scope referring to the funciton. Should each scope have
// a notion of "enclosing function name" (empty for non-function based scopes) to aid with
// tail recursion or something? hmm...interesting stuff
#if 0
Datum SpecialForms::namedlLambdaImpl(LispArgs args, SymbolTable& st) {
    auto[formals, defn] = parseFuncDefn(args, st);
    if (formals.empty()) {
        throw Error("Named lambda must have name");
    }
    Symbol funName = std::move(*formals.begin());
    formals.erase(formals.begin());

    SExprPtr impl =
        defn.isAtomic() ? std::make_shared<SExpr>(defn.getAtom()) : defn.getSExpr();

    return {Atom{LispFunction{std::move(formals), std::move(impl), st, true}}};
}
#endif

Datum SpecialForms::defineImpl(LispArgs args, SymbolTable& st) {
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
    auto [formals, defn] = parseFuncDefn(std::move(args));
    if (formals.empty()) {
        throw LispError("Function definition must have name");
    }
    Symbol funName = std::move(*formals.begin());
    formals.erase(formals.begin());

    SExprPtr impl =
        defn.isAtomic() ? std::make_shared<SExpr>(defn.getAtom()) : defn.getSExpr();
    st.emplace(+funName,
               Datum{Atom{LispFunction{std::move(formals), std::move(impl), st, false}}});
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

Datum SpecialForms::condImpl(LispArgs args, SymbolTable& st) {
    for (const Datum& datum : args) {
        if (unlikely(datum.isAtomic())) {
            throw LispError("Condition clauses must be pairs");
        }
        const SExprPtr& condPair = datum.getSExpr();
        if (auto sym = condPair->car.getAtomicValue<Symbol>();
            (sym && +*sym == "else") || Evaluator::computeArg(condPair->car, st).isTrue()) {
            return beginImpl(condPair->cdr.getSExpr(), st);
        }
    }
    // ret value unspecified if all conds false and no else
    return {};
}
