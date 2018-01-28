// (c) Sam Donow 2017
#include "SystemMethods.h"
#include "data/Data.h"
#include "Evaluator.h"
#include "util/function_traits.h"
#include <iostream>
#include <numeric>
#include <utility>

template<auto func>
class FixedArityFunction {
    using TupleT = typename function_traits<decltype(func)>::ArgTupleType;
    static constexpr size_t Arity = function_traits<decltype(func)>::arity;
    static EvalResult apply(LispArgs args, SymbolTable& st, Evaluator& ev) {
        TupleT argsToPass = setupArgs(args.begin(), args.end(), st, ev);
        return std::apply(func, argsToPass);
    }

    template <typename FwdIterator>
    static TupleT setupArgs(FwdIterator start, FwdIterator end, SymbolTable& st,
                            Evaluator& ev) {
        TupleT argsToPass;
        validateArgsImpl(start, end, argsToPass, st, ev,
                         std::make_index_sequence<Arity>{});
        return argsToPass;
    }

    template <typename FwdIterator, size_t... Is>
    static void validateArgsImpl(FwdIterator start, FwdIterator end,
                                 TupleT& argTuple, SymbolTable& st, Evaluator& ev, std::index_sequence<Is...>) {
        if constexpr (sizeof...(Is) == 0) {
            if (start != end) {
                throw ArityError(Arity + static_cast<size_t>(std::distance(start, end)),
                                 Arity);
            }
        } else {
            constexpr size_t currIndex = util::index_sequence_head_v<Is...>;
            auto&& arg = ev.getOrEvaluateE<std::tuple_element_t<currIndex, TupleT>>(*start, st);
            std::get<currIndex>(argTuple) = arg;
            validateArgsImpl(++start, end, argTuple, st, ev, util::index_sequence_tail_t<Is...>{});
        }
    }

  public:
    static void insert(SymbolTable& st, const std::string& s) {
        st.emplace(s, &FixedArityFunction::apply);
    }
};

void SystemMethods::insertIntoScope(SymbolTable& st) {
    st.emplace("+", &SystemMethods::add);
    st.emplace("-", &SystemMethods::sub);
    st.emplace("*", &SystemMethods::mul);
    st.emplace("/", &SystemMethods::div);

    FixedArityFunction<SystemMethods::quotient>::insert(st, "quotient");
    FixedArityFunction<SystemMethods::remainder>::insert(st, "remainder");
    FixedArityFunction<SystemMethods::modulo>::insert(st, "modulo");

    FixedArityFunction<SystemMethods::inc>::insert(st, "1+");
    FixedArityFunction<SystemMethods::dec>::insert(st, "-1+");
    FixedArityFunction<SystemMethods::abs>::insert(st, "abs");

    st.emplace("=", &SystemMethods::eq);
    st.emplace("<", &SystemMethods::lt);
    st.emplace(">", &SystemMethods::gt);
    st.emplace("<=",&SystemMethods::le);
    st.emplace("<=",&SystemMethods::ge);
    st.emplace("zero?", &SystemMethods::zeroQ);
    st.emplace("positive?", &SystemMethods::positiveQ);
    st.emplace("negative?", &SystemMethods::negativeQ);
    //st.emplace("odd?", &SystemMethods::oddQ);
    //st.emplace("even?", &SystemMethods::evenQ);
    st.emplace("exact?", &SystemMethods::exactQ);
    st.emplace("inexact?", &SystemMethods::inexactQ);

    st.emplace("car", &SystemMethods::car);
    st.emplace("cdr", &SystemMethods::cdr);
    st.emplace("cons", &SystemMethods::cons);

    st.emplace("eq?", &SystemMethods::eqQ);
    st.emplace("null?", &SystemMethods::nullQ);
    st.emplace("list", &SystemMethods::list);
    st.emplace("display", &SystemMethods::display);
}

EvalResult SystemMethods::add(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{util::foldr(
        args.begin(), args.end(), Number{0L},
        [&](const Datum& datum, const Number& number) {
            return number + ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

EvalResult SystemMethods::sub(LispArgs args, SymbolTable& st, Evaluator& ev) {
    Number diff{};
    for (auto it = args.begin(); it != args.end(); ++it) {
        const Number val = ev.getOrEvaluateE<Number>(*it, st);
        if (it == args.begin() && args.size() > 1) {
            diff = val;
        } else {
            diff -= val;
        }
    }
    return Datum{Atom{diff}};
}

EvalResult SystemMethods::mul(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{util::foldr(
        args.begin(), args.end(), Number{1L},
        [&](const Datum& datum, const Number& number) {
            return number * ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

EvalResult SystemMethods::div(LispArgs args, SymbolTable& st, Evaluator& ev) {
    Number quot{1L};
    for (auto it = args.begin(); it != args.end(); ++it) {
        const Number val = ev.getOrEvaluateE<Number>(*it, st);
        if (it == args.begin() && args.size() > 1) {
            quot = val;
        } else {
            quot /= val;
        }
    }
    return Datum{Atom{quot}};
}

Datum SystemMethods::quotient(Number first, Number second) {
    if (unlikely(!(first.isExact() && second.isExact()))) {
        throw LispError("quotient arguments must be exact");
    }
    return {Atom{Number{first.as<BigInt>() / second.as<BigInt>()}}};
}

Datum SystemMethods::remainder(Number first, Number second) {
    if (unlikely(!(first.isExact() && second.isExact()))) {
        throw LispError("remainder arguments must be exact");
    }
    return {Atom{first % second}};
}

Datum SystemMethods::modulo(Number first, Number second) {
    if (unlikely(!(first.isExact() && second.isExact()))) {
        throw LispError("modulo arguments must be exact");
    }
    Number remainder = first % second;
    if (first * second < Number{0L}) {
        return {Atom{second + remainder}};
    }
    return {Atom{remainder}};
}

Datum SystemMethods::inc(Number x) {
    return {Atom{x + Number{1L}}};
}

Datum SystemMethods::dec(Number x) {
    return {Atom{x - Number{1L}}};
}

Datum SystemMethods::abs(Number x) {
    return Datum{Atom{x.abs()}};
}

EvalResult SystemMethods::eq(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return Datum::False();
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first == ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

EvalResult SystemMethods::lt(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return Datum::False();
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first < ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

EvalResult SystemMethods::gt(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return Datum::False();
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first > ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

EvalResult SystemMethods::le(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return Datum::False();
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first <= ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

EvalResult SystemMethods::ge(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return Datum::False();
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first >= ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

EvalResult SystemMethods::exactQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{ev.getOrEvaluateE<Number>(*args.begin(), st).isExact()}};
}

EvalResult SystemMethods::inexactQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{!ev.getOrEvaluateE<Number>(*args.begin(), st).isExact()}};
}

EvalResult SystemMethods::zeroQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{ev.getOrEvaluateE<Number>(*args.begin(), st) == Number{0L}}};
}

EvalResult SystemMethods::positiveQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{ev.getOrEvaluateE<Number>(*args.begin(), st) > Number{0L}}};
}

EvalResult SystemMethods::negativeQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{ev.getOrEvaluateE<Number>(*args.begin(), st) < Number{0L}}};
}

EvalResult SystemMethods::car(LispArgs args, SymbolTable& st, Evaluator& ev) {
    Datum arg = ev.computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        throw LispError("car requires a cons cell");
    }
    return arg.getSExpr()->car;
}

EvalResult SystemMethods::cdr(LispArgs args, SymbolTable& st, Evaluator& ev) {
    Datum arg = ev.computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        throw LispError("cdr requires a cons cell");
    }
    return Datum{arg.getSExpr()->cdr};
}

EvalResult SystemMethods::cons(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 2) {
        throw LispError("Function expects 2 arguments, received ", args.size());
    }
    auto it = args.begin();
    auto ret = std::make_shared<SExpr>(ev.computeArg(*it++, st));
    ret->cdr = ev.computeArg(*it, st);
    return Datum{ret};
}

EvalResult SystemMethods::eqQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 2) {
        throw LispError("Function expects 2 arguments, received ", args.size());
    }

    auto it = args.begin();
    Datum first = ev.computeArg(*it, st);
    ++it;
    Datum second = ev.computeArg(*it, st);
    return Datum{Atom{first == second}};
}

EvalResult SystemMethods::list(LispArgs args, SymbolTable& st, Evaluator& ev) {
    SExprPtr ret = std::make_shared<SExpr>(nullptr);
    SExpr* curr = ret.get();
    bool first = true;
    for (const Datum& datum : args) {
        if (!first) {
            curr->cdr = std::make_shared<SExpr>(nullptr);
            curr = curr->cdr.getSExpr().get();
        }
        curr->car = ev.computeArg(datum, st);
        first = false;
    }
    return Datum{ret};
}

EvalResult SystemMethods::nullQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 1) {
        throw LispError("null? expects only 1 argument");
    }
    Datum arg = ev.computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        return Datum::True();
    }
    const bool isNull = arg.getSExpr() == nullptr;
    return Datum{Atom{isNull}};
}

EvalResult SystemMethods::display(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return Datum{};
    }
    std::cout << ev.computeArg(*args.begin(), st);
    return Datum{};
}
