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
    std::optional<T> getOrEvaluate(const Datum &datum,
                                   std::shared_ptr<SymbolTable> st);

    /// Evaluate a lisp function on the given args
    std::optional<Datum> evalFunction(const LispFunction &func,
                                      const std::list<Datum> &args,
                                      std::shared_ptr<SymbolTable> st);

  public:
    /// On construction, we populate the global scope with all of the special
    /// forms and language-level functions
    Evaluator();

    /// Main public interface: evaluates an expression in a given scope
    std::optional<Datum> eval(const SExpr &expr,
                              std::shared_ptr<SymbolTable> scope = nullptr);
};
