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
struct LispFunction {
    std::vector<Symbol> formalParameters;
    std::shared_ptr<SExpr> definition;
    std::shared_ptr<SymbolTable> defnScope;
};

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
};

struct Datum {
    // TODO: clean this up a lot
    std::variant<Atom, std::shared_ptr<SExpr>> data;

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

    bool isAtomic() const {
        return std::holds_alternative<Atom>(data);
    }

    bool isNil() const {
        return !isAtomic() && getSExpr() == nullptr;
    }

    Datum() = default;
    Datum(Atom atom) {
        data.emplace<Atom>(std::move(atom));
    }

    Datum(std::shared_ptr<SExpr> ptr) {
        data.emplace<std::shared_ptr<SExpr>>(std::move(ptr));
    }

    friend std::ostream &operator<<(std::ostream &os, const Datum &datum) {
        return std::visit(Visitor {
            [&os](const Atom &atom) -> std::ostream& { return os << atom; },
            [&os](const std::shared_ptr<SExpr> &) -> std::ostream& {
                return os << "<sexpr>";
            }
        }, datum.data);
    }
};

struct SExpr : std::enable_shared_from_this<SExpr> {
    Datum car;
    // TODO: make this a proper cons cell so that manipulations are
    // more efficient
    std::list<Datum> cdr;

    SExpr(Atom atom) {
        car.data.emplace<Atom>(std::move(atom));
    }

    SExpr(std::shared_ptr<SExpr> ptr) {
        car.data.emplace<std::shared_ptr<SExpr>>(std::move(ptr));
    }
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
