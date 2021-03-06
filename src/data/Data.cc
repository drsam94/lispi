#include "Data.h"

LispFunction::LispFunction(std::vector<Symbol> &&formals,
                           const SExprPtr& defn,
                           SymbolTable& scope)
    : formalParameters(std::move(formals)), definition(defn),
        defnScope{scope.shared_from_this()} {
}

std::shared_ptr<SymbolTable> LispFunction::funcScope() const {
    return defnScope.lock()->makeChild();
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

// TODO: for eqv? support pointer equality of SExprs and of reference-counted
// strings
bool Datum::operator==(const Datum& other) const {
    return std::visit(
        Visitor{[](const Atom& a1, const Atom& a2) { return a1 == a2; },
                [](const auto&, const auto&) { return false; }},
        data, other.data);
}

SymbolTable::value_type& SymbolTable::
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

SymbolTable::value_type& SymbolTable::get(const Symbol& s) {
    return (*this)[+s];
}

SymbolTable::value_type& SymbolTable::get(const Atom& atom) {
    return (*this)[+atom.get<Symbol>()];
}
