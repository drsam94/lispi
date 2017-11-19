// (c) 2017 Sam Donow
#pragma once

#include "Number.h"

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <optional>
#include <variant>
#include <vector>
#include <unordered_map>

struct SExpr;
class SymbolTable;

// Class used for representing "symbols" -- the data is just a string, but we want a
// distinct type
struct Symbol {
    std::string val;

    const std::string& operator+() const { return val; }

    friend std::ostream &operator<<(std::ostream &os, const Symbol &sym) {
        return os << +sym;
    }
};

// Type describing a function in lisp: a list of formal parameters together with
// a definition
class LispFunction {
  public:
    std::vector<Symbol> formalParameters;
    std::shared_ptr<SExpr> definition;

    LispFunction(std::vector<Symbol>&& formals, const std::shared_ptr<SExpr> defn,
        const std::shared_ptr<SymbolTable> scope, bool isClosure) : formalParameters(std::move(formals)),
        definition(defn) {
            if (isClosure) {
                defnScope.emplace<std::shared_ptr<SymbolTable>>(scope);
            } else {
                defnScope.emplace<SymbolTable*>(scope.get());
            }
        }
    std::shared_ptr<SymbolTable> funcScope() const;
  private:
    // This should be a shared_ptr for lambdas (closures), but shouldn't
    // be for normal functions (e.g) define: having this be a shared_ptr for
    // functions that are (define)'d leads to a cycle, and the SymbolTables
    // can't be garbage collected. Solution: functions should have a raw pointer
    // to their enclosing scope, while closures have
    std::variant<std::shared_ptr<SymbolTable>, SymbolTable*> defnScope;
};


// An Atom is any entity in lisp other than an SExpr (aka pair, cons cell, list)
struct Atom {
    std::variant<std::monostate, Number, bool,
                 std::string, Symbol, LispFunction> data;

    Atom() = default;
    template <typename T, typename = std::enable_if_t<
                              !std::is_same_v<Atom, std::remove_reference<T>>>>
    explicit Atom(T &&val) : data(std::forward<T>(val)) {}

    template<typename T>
    decltype(auto) get() const {
        return std::get<T>(data);
    }

    template<typename T>
    bool contains() const {
        return std::holds_alternative<T>(data);
    }

    friend std::ostream &operator<<(std::ostream &os, const Atom &atom) {
        return std::visit(Visitor {
            [&os](const std::monostate&) -> std::ostream& { return os << "<none>"; },
            [&os](const LispFunction&) -> std::ostream& { return os << "<func>"; },
            [&os](const auto &n) -> std::ostream& { return os << n; }
        }, atom.data);
    }

    bool operator==(const Atom& other) const {
        return std::visit(Visitor{
            [](const Number& n1, const Number& n2) { return n1 == n2; },
            [](bool b1, bool b2) { return b1 == b2; },
            [](const std::string& s1, const std::string& s2) { return s1 == s2; },
            [](const auto&, const auto&) { return false; }
        }, data, other.data);
    }
};

// Any piece of data: can be an Atom or an SExpr
class Datum {
    std::variant<Atom, std::shared_ptr<SExpr>> data;
  public:
    template<typename T>
    std::optional<T> getAtomicValue() const {
        if (!std::holds_alternative<Atom>(data)) {
            return std::nullopt;
        }
        const Atom& atom = std::get<Atom>(data);
        if (!atom.contains<T>()) {
            return std::nullopt;
        }
        return atom.get<T>();
    }

    const std::shared_ptr<SExpr>& getSExpr() const {
        return std::get<std::shared_ptr<SExpr>>(data);
    }

    const Atom& getAtom() const {
        return std::get<Atom>(data);
    }

    bool isAtomic() const noexcept {
        return std::holds_alternative<Atom>(data);
    }

    bool isNil() const noexcept {
        return !isAtomic() && getSExpr() == nullptr;
    }

    Datum() = default;
    Datum(Atom atom) {
        data.emplace<Atom>(std::move(atom));
    }

    Datum(std::shared_ptr<SExpr> ptr) {
        data.emplace<std::shared_ptr<SExpr>>(std::move(ptr));
    }

    friend std::ostream &operator<<(std::ostream& os, const Datum& datum) {
        return std::visit(Visitor {
            [&os](const Atom &atom) -> std::ostream& { return os << atom; },
            [&os](const std::shared_ptr<SExpr>&) -> std::ostream& {
                return os << "<sexpr>";
            }
        }, datum.data);
    }

    bool operator==(const Datum& other) const {
        return std::visit(Visitor {
            [](const Atom& a1, const Atom& a2) { return a1 == a2; },
            [](const auto&, const auto&) { return false; }
        }, data, other.data);
    }
};

// An SExpr/cons cell/pair
// TODO: this is implemented in such a way that makes parsing of SExprs easy,
// but doesn't make implementing operators on cons cells as nice, particularly wrt
// proper implementation of the standard Lisp garbage collection mechanism. This
// shouldn't have much overhead over the current method of using a LinkedList anyway.
// Furthermore, it's really confusing that you can construct an SExpr from a shared_ptr<SExpr>, and
// this caused at least one bug...and it lookd like there is also a leak from this. This is the next
// top priority for cleaning up
struct SExpr : std::enable_shared_from_this<SExpr> {
    Datum car;
    std::list<Datum> cdr;

    explicit SExpr(Atom atom) : car{std::move(atom)} {}

    explicit SExpr(std::shared_ptr<SExpr> ptr) :
        car{std::move(ptr)} {}
};

using BuiltInFunc = Datum(const std::list<Datum> &, std::shared_ptr<SymbolTable>);
using SpecialForm = std::function<BuiltInFunc>;

class SymbolTable : public std::enable_shared_from_this<SymbolTable> {
  private:
    std::unordered_map<std::string, std::variant<Datum, SpecialForm>> table;
    std::shared_ptr<SymbolTable> parent;

  public:
    explicit SymbolTable(std::shared_ptr<SymbolTable> p)
        : parent(std::move(p)) {}
    std::variant<Datum, SpecialForm> &operator[](const std::string &s) {
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

    std::variant<Datum, SpecialForm> &emplace(const std::string &s,
                                              const Datum &datum) {
        return table.emplace(s, datum).first->second;
    }

    std::variant<Datum, SpecialForm> &emplace(const std::string &s,
                                              SpecialForm form) {
        return table.emplace(s, form).first->second;
    }

    std::shared_ptr<SymbolTable> makeChild() {
        return std::make_shared<SymbolTable>(shared_from_this());
    }
};
