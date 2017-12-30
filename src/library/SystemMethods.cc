// (c) Sam Donow 2017
#include "SystemMethods.h"
#include "data/Data.h"
#include "Evaluator.h"
#include <iostream>
#include <utility>

// Alright, this is kind of complex and I am tired
template<BuiltInFunc func, typename... TypePack>
class FixedArityFunction {
    using TupleT = std::tuple<TypePack...>;
    static constexpr size_t Arity = std::tuple_size_v<TupleT>;
    static Datum apply(LispArgs args, SymbolTable& st, Evaluator& ev) {
        TupleT fwdArgs;
        validateArgs(args.begin(), args.end());
        return func(std::move(args), st, ev);
    }

    template<typename FwdIterator>
    static void validateArgs(FwdIterator start, FwdIterator end) {
        validateArgsImpl(start, end, std::make_index_sequence<Arity>{});
    }

    template<typename FwdIterator, size_t... Is>
    static void validateArgsImpl(FwdIterator start, FwdIterator end, std::index_sequence<Is...>) {
        if constexpr (sizeof...(Is) == 0) {
            if (start != end) {
                throw ArityError(Arity + static_cast<size_t>(std::distance(start, end)), Arity);
            }
        } else {
            if (!start->template hasAtomicValue<std::tuple_element_t<util::index_sequence_head_v<Is...>, TupleT>>()) {
                // need some sort of type string mapping
                throw TypeError("Something", "Not that");
            }
            validateArgsImpl(++start, end, util::index_sequence_tail_t<Is...>{});
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
    //st.emplace("/", &SystemMethods::div);

    FixedArityFunction<SystemMethods::quotient, Number, Number>::insert(st, "quotient");
    FixedArityFunction<SystemMethods::remainder, Number, Number>::insert(st, "remainder");
    FixedArityFunction<SystemMethods::modulo, Number, Number>::insert(st, "modulo");

    st.emplace("1+", &SystemMethods::inc);
    st.emplace("-1+", &SystemMethods::dec);
    st.emplace("abs", &SystemMethods::abs);

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

Datum SystemMethods::add(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{util::foldr(args.begin(), args.end(), 0_N,
                                  [&](const Datum& datum, const Number& number) {
                                      return number + ev.getOrEvaluateE<Number>(datum, st);
                                  })}};
}

Datum SystemMethods::sub(LispArgs args, SymbolTable& st, Evaluator& ev) {
    Number diff{};
    for (auto it = args.begin(); it != args.end(); ++it) {
        const Number val = ev.getOrEvaluateE<Number>(*it, st);
        if (it == args.begin() && args.size() > 1) {
            diff = val;
        } else {
            diff -= val;
        }
    }
    return {Atom{diff}};
}

Datum SystemMethods::mul(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return Datum{Atom{util::foldr(args.begin(), args.end(), 1_N,
                                  [&](const Datum& datum, const Number& number) {
                                      return number * ev.getOrEvaluateE<Number>(datum, st);
                                  })}};
}

Datum SystemMethods::quotient(LispArgs args, SymbolTable& st, Evaluator& ev) {
    auto it = args.begin();
    const Number& first  = ev.getOrEvaluateE<Number>(*it++, st);
    const Number& second = ev.getOrEvaluateE<Number>(*it, st);
    if (unlikely(!(first.isExact() && second.isExact()))) {
        throw LispError("quotient arguments must be exact");
    }
    return {Atom{first / second}};
}

Datum SystemMethods::remainder(LispArgs args, SymbolTable& st, Evaluator& ev) {
    auto it = args.begin();
    const Number& first  = ev.getOrEvaluateE<Number>(*it++, st);
    const Number& second = ev.getOrEvaluateE<Number>(*it, st);
    if (unlikely(!(first.isExact() && second.isExact()))) {
        throw LispError("remainder arguments must be exact");
    }
    return {Atom{first % second}};
}

Datum SystemMethods::modulo(LispArgs args, SymbolTable& st, Evaluator& ev) {
    auto it = args.begin();
    const Number& first  = ev.getOrEvaluateE<Number>(*it++, st);
    const Number& second = ev.getOrEvaluateE<Number>(*it, st);
    if (unlikely(!(first.isExact() && second.isExact()))) {
        throw LispError("modulo arguments must be exact");
    }
    Number remainder = first % second;
    if (first * second < 0_N) {
        return {Atom{second + remainder}};
    }
    return {Atom{remainder}};
}

Datum SystemMethods::inc(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 1) {
        throw LispError("1+ requires exactly 1 argument");
    }
    return {Atom{ev.getOrEvaluateE<Number>(*args.begin(), st) + 1_N}};
}

Datum SystemMethods::dec(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 1) {
        throw LispError("-1+ requires exactly 1 argument");
    }
    return {Atom{ev.getOrEvaluateE<Number>(*args.begin(), st) - 1_N}};
}

Datum SystemMethods::abs(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 1) {
        throw LispError("1+ requires exactly 1 argument");
    }
    return {Atom{ev.getOrEvaluateE<Number>(*args.begin(), st).abs()}};
}

Datum SystemMethods::eq(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return {Atom{false}};
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first == ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

Datum SystemMethods::lt(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return {Atom{false}};
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first < ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

Datum SystemMethods::gt(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return {Atom{false}};
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first > ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

Datum SystemMethods::le(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return {Atom{false}};
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first <= ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

Datum SystemMethods::ge(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return {Atom{false}};
    }
    auto it = args.begin();
    const Number& first = ev.getOrEvaluateE<Number>(*it, st);
    return Datum{
        Atom{util::foldr(++it, args.end(), true, [&](const Datum& datum, bool acc) {
            return acc && first >= ev.getOrEvaluateE<Number>(datum, st);
        })}};
}

Datum SystemMethods::exactQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return {Atom{ev.getOrEvaluateE<Number>(*args.begin(), st).isExact()}};
}

Datum SystemMethods::inexactQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return {Atom{!ev.getOrEvaluateE<Number>(*args.begin(), st).isExact()}};
}

Datum SystemMethods::zeroQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return {Atom{ev.getOrEvaluateE<Number>(*args.begin(), st) == 0_N}};
}

Datum SystemMethods::positiveQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return {Atom{ev.getOrEvaluateE<Number>(*args.begin(), st) > 0_N}};
}

Datum SystemMethods::negativeQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    return {Atom{ev.getOrEvaluateE<Number>(*args.begin(), st) < 0_N}};
}

Datum SystemMethods::car(LispArgs args, SymbolTable& st, Evaluator& ev) {
    Datum arg = ev.computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        throw LispError("car requires a cons cell");
    }
    return arg.getSExpr()->car;
}

Datum SystemMethods::cdr(LispArgs args, SymbolTable& st, Evaluator& ev) {
    Datum arg = ev.computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        throw LispError("cdr requires a cons cell");
    }
    return Datum{arg.getSExpr()->cdr};
}

Datum SystemMethods::cons(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 2) {
        throw LispError("Function expects 2 arguments, received ", args.size());
    }
    auto it = args.begin();
    auto ret = std::make_shared<SExpr>(ev.computeArg(*it++, st));
    ret->cdr = ev.computeArg(*it, st);
    return {ret};
}

Datum SystemMethods::eqQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 2) {
        throw LispError("Function expects 2 arguments, received ", args.size());
    }

    auto it = args.begin();
    Datum first = ev.computeArg(*it, st);
    ++it;
    Datum second = ev.computeArg(*it, st);
    return Datum{Atom{first == second}};
}

Datum SystemMethods::list(LispArgs args, SymbolTable& st, Evaluator& ev) {
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

Datum SystemMethods::nullQ(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.size() != 1) {
        throw LispError("null? expects only 1 argument");
    }
    Datum arg = ev.computeArg(*args.begin(), st);
    if (arg.isAtomic()) {
        return Datum{Atom{false}};
    }
    const bool isNull = arg.getSExpr() == nullptr;
    return Datum{Atom{isNull}};
}

Datum SystemMethods::display(LispArgs args, SymbolTable& st, Evaluator& ev) {
    if (args.empty()) {
        return {};
    }
    std::cout << ev.computeArg(*args.begin(), st);
    return {};
}
