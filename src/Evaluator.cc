// (c) 2017 Sam Donow
#include "Evaluator.h"
#include "util/Util.h"
#include "library/SpecialForms.h"
#include "library/SystemMethods.h"

Evaluator::Evaluator() : globalScope(std::make_shared<SymbolTable>(nullptr)) {
    // TODO: have a distinciton between true "special forms" (i.e if, quote)
    // and library-defined functions (like "+")

    SystemMethods::insertIntoScope(*globalScope);
    SpecialForms::insertIntoScope(*globalScope);

    // Special Globals
    globalScope->emplace("#t", Datum{Atom{true}});
    globalScope->emplace("#f", Datum{Atom{false}});
}

Datum Evaluator::computeArg(const Datum& datum, SymbolTable& st) {
    if (datum.isAtomic()) {
        const Atom& val = datum.getAtom();
        if (val.contains<Symbol>()) {
            // Note: this is where we need the SpecialForm/BuiltinFunc distinction;
            // this should be able to return, e.g "+", but not "if"
            return computeArg(std::get<Datum>(st.get(val)), st);
        }
        return Datum{val};
    } else {
        const SExprPtr& expr = datum.getSExpr();
        if (expr == nullptr) {
            return Datum{expr};
        } else {
            std::optional<Datum> ret = eval(expr, st);
            if (!ret) {
                throw LispError("Failed to evaluate argument");
            }
            return *ret;
        }
    }
}

std::optional<Datum>
Evaluator::evalFunction(const LispFunction& func, const SExprPtr& args,
                        SymbolTable& scope) {
    std::optional<Datum> ret = std::nullopt;
    LispArgs currArgs{args};
    while (!ret) {
        // TODO: some dialects of lisp support optional and variadic parameters...
        if (func.formalParameters.size() != args->size()) {
            return std::nullopt;
        }
        auto formalIt = func.formalParameters.begin();
        auto actualIt = args->begin();
        auto funcScope = func.funcScope();
        for (; formalIt != func.formalParameters.end() && actualIt != args->end();
            ++formalIt, ++actualIt) {
            Datum result = computeArg(*actualIt, scope);
            funcScope->emplace(+*formalIt, result);
        }

        if (func.definition->cdr.getSExpr() == nullptr) {
            // Workaround to support things like lambda (x) x...this is probably not
            // fully compliant with the spec
            auto sym = func.definition->car.getAtomicValue<Symbol>();
            if (sym) {
                return std::get<Datum>(funcScope->get(*sym));
            }
            return computeArg(func.definition->car, *funcScope);
        } else {
            ret = eval(func.definition, *funcScope);
            if (!ret) {
                currArgs = std::move(argsToTailRecurse);
            }
        }
    }
    return ret;
}

std::optional<Datum>
Evaluator::eval(const SExprPtr& expr, SymbolTable& scope) {
    auto sym = expr->car.getAtomicValue<Symbol>();
    if (sym) {
        const auto &scopeElem = scope[+*sym];
        if (std::holds_alternative<SpecialForm>(scopeElem)) {
            return std::get<SpecialForm>(scopeElem)(expr->cdr.getSExpr(), scope, *this);
        }

        if (&scopeElem == currentFunction) {
            // There are probably corner cases where this won't work
            argsToTailRecurse = LispArgs(expr->cdr.getSExpr());
            return std::nullopt;
        }
        const auto &func = std::get<Datum>(scopeElem).getAtomicValue<LispFunction>();
        if (!func) {
            throw LispError("Can't find function ", *sym);
        }

        ScopeGuard guard{[this, cf = currentFunction] {
            this->currentFunction = cf;
        }};
        currentFunction = &scopeElem;
        return evalFunction(*func, expr->cdr.getSExpr(), scope);
    }

    if (!expr->car.isAtomic()) {
        const auto &carFunc = eval(expr->car.getSExpr(), scope);
        if (!carFunc) {
            throw LispError("Can't evaluate non function");
        }
        const auto &func = carFunc->getAtomicValue<LispFunction>();
        if (!func) {
            throw LispError("Can't evaluate non function");
        }

        return evalFunction(*func, expr->cdr.getSExpr(), scope);
    } else {
        throw LispError("Can't evaluate non function");
    }
}
