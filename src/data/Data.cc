#include "Data.h"

LispFunction::LispFunction(std::vector<Symbol> &&formals,
                           const SExprPtr& defn,
                           SymbolTable& scope,
                           bool isClosure)
    : formalParameters(std::move(formals)), definition(defn),
        defnScope{} {

    if (isClosure) {
        defnScope = scope.shared_from_this();
    } else {
        defnScope = &scope;
    }
}

std::shared_ptr<SymbolTable> LispFunction::funcScope() const {
    if (std::holds_alternative<SymbolTable*>(defnScope)) {
        return std::get<SymbolTable*>(defnScope)->makeChild();
    } else {
        return std::get<std::shared_ptr<SymbolTable>>(defnScope)->makeChild();
    }
}

std::ostream& operator<<(std::ostream& os, const Atom& atom) {
    return std::visit(Visitor {
        [&os](const std::monostate&) -> std::ostream& { return os << std::endl; },
        [&os](const LispFunction&) -> std::ostream& { return os << "<func>"; },
        [&os](bool b) -> std::ostream& { return os << (b ? "#t" : "#f"); },
        [&os](const auto &n) -> std::ostream& { return os << n; }
    }, atom.data);
}

bool Atom::operator==(const Atom& other) const {
    return std::visit(Visitor{
        [](const Number& n1, const Number& n2) { return n1 == n2; },
        [](bool b1, bool b2) { return b1 == b2; },
        [](const std::string& s1, const std::string& s2) { return s1 == s2; },
        [](const auto&, const auto&) { return false; }
    }, data, other.data);
}

std::ostream& operator<<(std::ostream& os, const Datum& datum) {
    return std::visit(
        Visitor{[&os](const Atom& atom) -> std::ostream& { return os << atom; },
                [&os](const SExprPtr& sexpr) -> std::ostream& {
                    if (sexpr == nullptr) {
                        return os << "'()";
                    } else {
                        return os << *sexpr;
                    }
                }},
        datum.data);
}

std::ostream& operator<<(std::ostream& os, const SExpr& expr) {
    os << "'(";
    for (auto it = expr.begin(); it != expr.end();) {
        const Datum& datum = *it;
        os << datum << (++it == expr.end() ? ")" : " ");
    }
    return os;
}

bool Datum::operator==(const Datum& other) const {
    return std::visit(
        Visitor{[](const Atom& a1, const Atom& a2) { return a1 == a2; },
                [](const auto&, const auto&) { return false; }},
        data, other.data);
}

std::variant<Datum, SpecialForm>& SymbolTable::
operator[](const std::string& s) {
    if (auto it = table.find(s); it == table.end()) {
        if (unlikely(parent == nullptr)) {
            throw LispError("Undefined Symbol: ", s);
        } else {
            return (*parent)[s];
        }
    } else {
        return it->second;
    }
}

std::variant<Datum, SpecialForm>& SymbolTable::get(const Symbol& s) {
    return (*this)[+s];
}

std::variant<Datum, SpecialForm>& SymbolTable::get(const Atom& atom) {
    return (*this)[+atom.get<Symbol>()];
}

LispError::~LispError() = default;
