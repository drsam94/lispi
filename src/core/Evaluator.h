// (c) 2017 Sam Donow
#pragma once
#include "data/Data.h"

#include <memory>
class Evaluator {
    std::shared_ptr<SymbolTable> globalScope;

  public:
      /// If the given datum is atomic, get the value of the desired type (if it
    /// exists) otherwise, recursively evaluate until we return something of the
    /// desired type.
    /// return nullopt if there is a failure at any point
    template <typename T>
    std::optional<T> getOrEvaluate(const Datum& datum, SymbolTable& st);

    template <typename T>
    T getOrEvaluateE(const Datum& datum, SymbolTable& st);
    /// Evaluate an argument in the context of expanding an argument to a function in an
    /// SExpr. As the context is a run-time needed computation, throw if the evaluation fails
    /// instead of returning an optional
    Datum computeArg(const Datum& datum, SymbolTable& st) {
        return std::visit(Visitor{
            [](const Datum& d) { return d; },
            [this](const FunctionCall& fc) { return evalFunction(fc); }
        }, computeArgResult(datum, st));
    }

    EvalResult computeArgResult(const Datum& datum, SymbolTable& st);

    /// Evaluate a lisp function on the given args
    Datum evalFunction(const std::shared_ptr<LispFunction>& func,
                       const SExprPtr& args,
                       SymbolTable& st);

    Datum evalFunction(const FunctionCall &fc);

    /// On construction, we populate the global scope with all of the special
    /// forms and language-level functions
    Evaluator();
    Evaluator(const Evaluator&) = delete;
    Evaluator(Evaluator&&) = delete;
    Evaluator operator=(const Evaluator&) = delete;
    Evaluator operator==(Evaluator&&) = delete;
    /// Main public interface: evaluates an expression in a given scope
    EvalResult eval(const SExprPtr& expr, SymbolTable& scope);
    Datum evalDatum(const SExprPtr& expr, SymbolTable& scope) {
        return std::visit(Visitor{
            [](const Datum& datum) { return datum; },
            [this](const FunctionCall& fc) { return evalFunction(fc); }
        }, eval(expr, scope));
    }

    Datum eval(const SExprPtr& expr) {
        return evalDatum(expr, *globalScope);
    }
};

template <typename T>
std::optional<T>
Evaluator::getOrEvaluate(const Datum& datum, SymbolTable& st) {
    if (datum.isAtomic()) {
        const Atom& val = datum.getAtom();
        if constexpr (!std::is_same_v<T, Symbol>) {
            if (val.contains<Symbol>()) {
                return getOrEvaluate<T>(std::get<Datum>(st.get(val)), st);
            }
        }
        return val.get<T>();
    } else {
        const auto& expr = datum.getSExpr();
        const Datum result = evalDatum(expr, st);
        return getOrEvaluate<T>(result, st);
    }
}

template <typename T>
T Evaluator::getOrEvaluateE(const Datum& datum, SymbolTable& st) {
    std::optional<T> val = getOrEvaluate<T>(datum, st);
    if (!val) {
        throw LispError("Type Error, TODO description");
    }
    return *val;
}
