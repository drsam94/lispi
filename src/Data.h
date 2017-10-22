// (c) 2017 Sam Donow
#pragma once

#include<variant>
#include<string>
#include<memory>
#include<list>
#include<functional>

struct Symbol {
    std::string val;

    const std::string& operator+() { return val; }
};

struct Atom {
    std::variant<std::monostate, long, double, bool, std::string, Symbol> data;
};

class SExpr;
struct Datum {
    // TODO: clean this up a lot
    std::variant<Atom, std::shared_ptr<SExpr>> data;

    template<typename T>
    std::optional<T> getAtomicValue() const {
        if (!std::holds_alternative<Atom>(data)) {
            return std::nullopt;
        }
        const Atom& atom = std::get<Atom>(data);
        if (!std::holds_alternative<T>(atom.data)) {
            return std::nullopt;
        }
        return std::get<T>(atom.data);
    }

    bool isAtomic() const {
        return std::holds_alternative<Atom>(data);
    }

    Datum() = default;
    Datum(Atom atom) {
        data.emplace<Atom>(std::move(atom));
    }

    Datum(std::shared_ptr<SExpr> ptr) {
        data.emplace<std::shared_ptr<SExpr>>(std::move(ptr));
    }
};

struct SExpr : std::enable_shared_from_this<SExpr> {
    Datum car;
    std::list<Datum> cdr;

    SExpr(Atom atom) {
        car.data.emplace<Atom>(std::move(atom));
    }

    SExpr(std::shared_ptr<SExpr> ptr) {
        car.data.emplace<std::shared_ptr<SExpr>>(std::move(ptr));
    }
};

using SpecialForm = std::function<Datum(std::list<Datum>&)>;
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