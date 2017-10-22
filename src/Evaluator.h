// (c) 2017 Sam Donow
#pragma once

#include "Data.h"

#include <memory>
class Evaluator {
    std::shared_ptr<SymbolTable> globalScope;
  public:
    Evaluator() : globalScope(std::make_shared<SymbolTable>(nullptr)) {
        globalScope->emplace("+", [](std::list<Datum> &inputs) -> Datum {
            double sum{};
            for (Datum &datum : inputs) {
                if (auto val = datum.getAtomicValue<double>(); bool(val)) {
                    sum += *val;
                } else {
                    throw "a structured runtime error";
                }
            }
            Datum datum;
            Atom atom;
            atom.data = sum;
            datum.data.emplace<Atom>(std::move(atom));
            return datum;
        });
    }

    std::optional<Datum> eval(SExpr &expr) {
        // TODO: recursively eval, support more special forms, etc;
        auto sym = expr.car.getAtomicValue<Symbol>();
        if (!sym) {
            return std::nullopt;
        }

        auto &scopeElem = (*globalScope)[+*sym];
        if (!std::holds_alternative<SpecialForm>(scopeElem)) {
            return std::nullopt;
        }

        return std::get<SpecialForm>(scopeElem)(expr.cdr);
    }
};
