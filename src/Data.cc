#include "Data.h"

LispFunction::LispFunction(std::vector<Symbol> &&formals,
                           const std::shared_ptr<SExpr> defn,
                           const std::shared_ptr<SymbolTable> scope,
                           bool isClosure)
    : formalParameters(std::move(formals)), definition(defn) {
    if (isClosure) {
        defnScope.emplace<std::shared_ptr<SymbolTable>>(scope);
    } else {
        defnScope.emplace<SymbolTable *>(scope.get());
    }
}

std::shared_ptr<SymbolTable> LispFunction::funcScope() const {
    if (std::holds_alternative<SymbolTable *>(defnScope)) {
        return std::get<SymbolTable *>(defnScope)->makeChild();
    } else {
        return std::get<std::shared_ptr<SymbolTable>>(defnScope)->makeChild();
    }
}

std::ostream& operator<<(std::ostream& os, const Atom& atom) {
    return std::visit(Visitor {
        [&os](const std::monostate&) -> std::ostream& { return os << "<none>"; },
        [&os](const LispFunction&) -> std::ostream& { return os << "<func>"; },
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

std::ostream &operator<<(std::ostream& os, const Datum& datum) {
    return std::visit(Visitor {
        [&os](const Atom &atom) -> std::ostream& { return os << atom; },
        [&os](const std::shared_ptr<SExpr>&) -> std::ostream& {
            return os << "<sexpr>";
        }
    }, datum.data);
}

bool Datum::operator==(const Datum& other) const {
    return std::visit(Visitor {
        [](const Atom& a1, const Atom& a2) { return a1 == a2; },
        [](const auto&, const auto&) { return false; }
    }, data, other.data);
}

std::variant<Datum, SpecialForm>& SymbolTable::
operator[](const std::string& s) {
    if (auto it = table.find(s); it == table.end()) {
        if (parent == nullptr) {
            throw "A runtime error which should be handled in some way";
        } else {
            return (*parent)[s];
        }
    } else {
        return it->second;
    }
}
