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

EvalResult Evaluator::computeArgResult(const Datum& datum, SymbolTable& st) {
    if (datum.isAtomic()) {
        const Atom& val = datum.getAtom();
        if (val.contains<Symbol>()) {
            // Note: this is where we need the SpecialForm/BuiltinFunc distinction;
            // this should be able to return, e.g "+", but not "if"
            return computeArgResult(std::get<Datum>(st.get(val)), st);
        }
        return Datum{val};
    } else {
        const SExprPtr& expr = datum.getSExpr();
        if (expr == nullptr) {
            return Datum{expr};
        } else {
            return eval(expr, st);
        }
    }
}

Datum
Evaluator::evalFunction(const std::shared_ptr<LispFunction>& func, const SExprPtr& args,
                        SymbolTable& scope) {
    if (func->formalParameters.size() != args->size()) {
        throw ArityError(func->formalParameters.size(), args->size());
    }
    return evalFunction(FunctionCall{func, LispArgs{args}, scope});
}

Datum Evaluator::evalFunction(const FunctionCall &fc) {
    const FunctionCall *call = &fc;
    EvalResult result;
    std::shared_ptr<SymbolTable> origFuncScope = call->func->funcScope();
    while (true) {
        auto formalIt = call->func->formalParameters.begin();
        auto actualIt = call->args.begin();
        std::shared_ptr<SymbolTable> funcScope = &call->func == &fc.func ? origFuncScope : call->func->funcScope();

        for (; formalIt != call->func->formalParameters.end() && actualIt != call->args.end();
            ++formalIt, ++actualIt) {
            Datum argResult = computeArg(*actualIt, *call->scope);
            funcScope->emplace(+*formalIt, argResult);
        }

        if (call->func->definition->cdr.getSExpr() == nullptr) {
            // Workaround to support things like lambda (x) x...this is probably not
            // fully compliant with the spec
            auto sym = call->func->definition->car.getAtomicValue<Symbol>();
            if (sym) {
                return std::get<Datum>(funcScope->get(*sym));
            }
            return computeArg(call->func->definition->car, *funcScope);
        } else {
            result = eval(call->func->definition, *funcScope);
            if (std::holds_alternative<Datum>(result)) {
                return std::get<Datum>(result);
            }
            call = &std::get<FunctionCall>(result);
        }
    }
}

EvalResult
Evaluator::eval(const SExprPtr& expr, SymbolTable& scope) {
    auto sym = expr->car.getAtomicValue<Symbol>();
    if (sym) {
        return std::visit(Visitor {
            [&](SpecialForm sf) -> EvalResult { return sf(expr->cdr.getSExpr(), scope, *this); },
            [&](LispFunction& lf) -> EvalResult { return FunctionCall{
                std::shared_ptr<LispFunction>(scope.shared_from_this(), &lf),
                LispArgs{expr->cdr.getSExpr()}, scope};
            },
            [&](const Datum& datum) -> EvalResult {
                const auto& func = datum.getAtomicValue<std::shared_ptr<LispFunction>>();
                if (!func) { throw LispError("Can't find function ", *sym); }
                else { return FunctionCall{*func, LispArgs{expr->cdr.getSExpr()}, scope}; }
            }
        }, scope[+*sym]);
    }

    if (!expr->car.isAtomic()) {
        const Datum& carFunc = std::visit(Visitor{
            [this](const Datum& d) { return d; },
            [this](const FunctionCall& f) { return evalFunction(f); }
        }, eval(expr->car.getSExpr(), scope));
        const auto& func = carFunc.getAtomicValue<std::shared_ptr<LispFunction>>();
        if (!func) {
            throw LispError("Can't evaluate non function");
        }

        return FunctionCall{*func, expr->cdr.getSExpr(), scope};
    } else {
        throw LispError("Can't evaluate non function");
    }
}
