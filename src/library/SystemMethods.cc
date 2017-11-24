// (c) Sam Donow 2017
#include "SystemMethods.h"
#include "data/Data.h"
#include "Evaluator.h"
#include <iostream>

void SystemMethods::insertIntoScope(SymbolTable& st) {
    st.emplace("+", &SystemMethods::add);
    st.emplace("-", &SystemMethods::sub);
    st.emplace("*", &SystemMethods::mul);
    st.emplace("car", &SystemMethods::car);
    st.emplace("cdr", &SystemMethods::cdr);
    st.emplace("eq?", &SystemMethods::eqQ);
    st.emplace("null?", &SystemMethods::nullQ);
    st.emplace("list", &SystemMethods::list);
    st.emplace("display", &SystemMethods::display);
}

Datum SystemMethods::add(LispArgs args,
                         const std::shared_ptr<SymbolTable>& st) {
    Number sum{};
    for (const Datum &datum : args) {
        if (auto val = Evaluator::getOrEvaluate<Number>(datum, st); bool(val)) {
            sum += *val;
        } else {
            throw LispError("TypeError: ", datum, " is not a number");
        }
    }
    return {Atom{sum}};
}

Datum SystemMethods::sub(LispArgs args,
                         const std::shared_ptr<SymbolTable>& st) {
    Number diff{};
    for (auto it = args.begin(); it != args.end(); ++it) {
        if (auto val = Evaluator::getOrEvaluate<Number>(*it, st); bool(val)) {
            if (it == args.begin() && args.size() > 1) {
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

Datum SystemMethods::mul(LispArgs args,
                         const std::shared_ptr<SymbolTable>& st) {
    Number prod = 1_N;
    for (const Datum& datum : args) {
        if (auto val = Evaluator::getOrEvaluate<Number>(datum, st); bool(val)) {
            prod *= *val;
        } else {
            throw LispError("TypeError: ", datum, " is not a number");
        }
    }
    return {Atom{prod}};
}

Datum SystemMethods::car(LispArgs args, const std::shared_ptr<SymbolTable>& st) {
    Datum arg = Evaluator::computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        throw LispError("car requires a cons cell");
    }
    return arg.getSExpr()->car;
}

Datum SystemMethods::cdr(LispArgs args, const std::shared_ptr<SymbolTable>& st) {
    Datum arg = Evaluator::computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        throw LispError("cdr requires a cons cell");
    }
    return Datum{arg.getSExpr()->cdr};
}

Datum SystemMethods::eqQ(LispArgs args, const std::shared_ptr<SymbolTable>& st) {
    if (args.size() != 2) {
        throw LispError("Function expects 2 arguments, received ", args.size());
    }

    auto it = args.begin();
    Datum first = Evaluator::computeArg(*it, st);
    ++it;
    Datum second = Evaluator::computeArg(*it, st);
    return Datum{Atom{first == second}};
}

Datum SystemMethods::list(LispArgs args, const std::shared_ptr<SymbolTable>& st) {
    SExprPtr ret = std::make_shared<SExpr>(nullptr);
    SExpr* curr = ret.get();
    bool first = true;
    for (const Datum& datum : args) {
        if (!first) {
            curr->cdr = std::make_shared<SExpr>(nullptr);
            curr = curr->cdr.get();
        }
        curr->car = Evaluator::computeArg(datum, st);
        first = false;
    }
    return Datum{ret};
}

Datum SystemMethods::nullQ(LispArgs args, const std::shared_ptr<SymbolTable>& st) {
    if (args.size() != 1) {
        throw LispError("null? expects only 1 argument");
    }
    Datum arg = Evaluator::computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        return Datum{Atom{false}};
    }
    const bool isNull = arg.getSExpr() == nullptr;
    return Datum{Atom{isNull}};
}

Datum SystemMethods::display(LispArgs args, const std::shared_ptr<SymbolTable>& st) {
    if (args.empty()) {
        return {};
    }
    std::cout << Evaluator::computeArg(*args.begin(), st);
    return {};
}
