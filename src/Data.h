// (c) 2017 Sam Donow
#pragma once

#include "Number.h"
#include "util/Util.h"

#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

struct SExpr;
using SExprPtr = std::shared_ptr<SExpr>;
class SymbolTable;

// Class used for representing "symbols" -- the data is just a string, but we want a
// distinct type
struct Symbol {
    std::string val;

    const std::string& operator+() const { return val; }

    friend std::ostream& operator<<(std::ostream& os, const Symbol& sym) {
        return os << +sym;
    }
};

// Type describing a function in lisp: a list of formal parameters together with
// a definition
class LispFunction {
  public:
    std::vector<Symbol> formalParameters;
    SExprPtr definition;

    LispFunction(std::vector<Symbol>&& formals, const SExprPtr defn,
        const std::shared_ptr<SymbolTable> scope, bool isClosure);

    std::shared_ptr<SymbolTable> funcScope() const;
  private:
    // This should be a shared_ptr for lambdas (closures), but shouldn't
    // be for normal functions (e.g) define: having this be a shared_ptr for
    // functions that are (define)'d leads to a cycle, and the SymbolTables
    // can't be garbage collected. Solution: functions should have a raw (non-owning)
    // pointer to their enclosing scope, while closures have a shared_ptr, as the
    // closure may need to otherwise outlast the scope
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

    friend std::ostream &operator<<(std::ostream &os, const Atom &atom);

    bool operator==(const Atom& other) const;
};

// Any piece of data: can be an Atom or an SExpr
class Datum {
    std::variant<Atom, SExprPtr> data;
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

    const SExprPtr& getSExpr() const {
        return std::get<SExprPtr>(data);
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

    Datum(SExprPtr ptr) {
        data.emplace<SExprPtr>(std::move(ptr));
    }

    friend std::ostream &operator<<(std::ostream& os, const Datum& datum);

    bool operator==(const Datum& other) const;
};

// An SExpr/cons cell/pair
struct SExpr : std::enable_shared_from_this<SExpr> {
    Datum car;
    SExprPtr cdr;

    explicit SExpr(Atom atom) : car{std::move(atom)} {}

    explicit SExpr(std::shared_ptr<SExpr> ptr) :
        car{std::move(ptr)} {}

    class iterator {
        friend struct SExpr;
        SExprPtr curr;

        iterator(SExprPtr _curr) : curr(_curr) {}
      public:
        Datum& operator*() { return curr->car; }
        Datum* operator->() { return &curr->car; }
        iterator& operator++() { curr = curr->cdr; return *this; }

        bool operator==(const iterator &other) const { return curr == other.curr; }
        bool operator!=(const iterator &other) const { return !(*this == other); }
    };

    iterator begin() { return iterator(shared_from_this()); }
    iterator end() { return iterator(nullptr); }
    // TODO: don't depend on this too much as it is O(N)
    size_t size() const {
        return 1 + (cdr == nullptr ? 0 : cdr->size());
    }
};

using BuiltInFunc = Datum(const SExprPtr&, std::shared_ptr<SymbolTable>);
using SpecialForm = std::function<BuiltInFunc>;

class SymbolTable : public std::enable_shared_from_this<SymbolTable> {
  private:
    std::unordered_map<std::string, std::variant<Datum, SpecialForm>> table;
    std::shared_ptr<SymbolTable> parent;

  public:
    explicit SymbolTable(std::shared_ptr<SymbolTable> p)
        : parent(std::move(p)) {}
    std::variant<Datum, SpecialForm> &operator[](const std::string &s);

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

class LispError : public std::runtime_error {
  public:
    explicit LispError(const std::string& what_arg)
        : std::runtime_error(what_arg) {}
    explicit LispError(const char* what_arg) : std::runtime_error(what_arg) {}

    // stringstream seems pretty slow, but error generation isn't exactly the
    // sort of thing that needs to be fast
    template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) > 1)>>
    LispError(Ts&&... args)
        : LispError(stringConcat(std::forward<Ts>(args)...)) {}
};
