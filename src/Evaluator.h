// (c) 2017 Sam Donow
#pragma once
#include "Data.h"

#include <memory>
class Evaluator {
    std::shared_ptr<SymbolTable> globalScope;

    /// If the given datum is atomic, get the value of the desired type (if it
    /// exists) otherwise, recursively evaluate until we return something of the
    /// desired type.
    /// return nullopt if there is a failure at any point
    template <typename T>
    static std::optional<T> getOrEvaluate(const Datum &datum,
                                   const std::shared_ptr<SymbolTable>& st);

    /// Evaluate an argument in the context of expanding an argument to a function in an
    /// SExpr. As the context is a run-time needed computation, throw if the evaluation fails
    /// instead of returning an optional
    static Datum computeArg(const Datum &datum, const std::shared_ptr<SymbolTable>& st);

    /// Evaluate a lisp function on the given args
    static std::optional<Datum> evalFunction(const LispFunction &func,
                                      const SExprPtr& args,
                                      const std::shared_ptr<SymbolTable>& st);

    /// Builtin Functions
    static BuiltInFunc builtinAdd;
    static BuiltInFunc builtinSub;
    static BuiltInFunc builtinMul;
    static BuiltInFunc builtinDiv;
    static BuiltInFunc builtinCar;
    static BuiltInFunc builtinCdr;
    static BuiltInFunc builtinEqQ;
    static BuiltInFunc builtinNullQ;
    static BuiltInFunc builtinList;

    /// Special Forms
    static BuiltInFunc builtinLambdaSF;
    static BuiltInFunc builtinIfSF;
    static BuiltInFunc builtinDefineSF;
    static BuiltInFunc builtinQuoteSF;
  public:
    /// On construction, we populate the global scope with all of the special
    /// forms and language-level functions
    Evaluator();

    /// Main public interface: evaluates an expression in a given scope
    static std::optional<Datum> eval(const SExprPtr& expr,
                              const std::shared_ptr<SymbolTable>& scope);
    std::optional<Datum> eval(const SExprPtr& expr) {
        return eval(expr, globalScope);
    }
};
