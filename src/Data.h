// (c) 2017 Sam Donow
#pragma once

#include<variant>
#include<string>
#include<memory>
#include<list>
#include<functional>

struct Symbol {
    std::string val;
};

struct Atom {
    std::variant<std::monostate, long, double, std::string, Symbol> data;
};

class SExpr;
using Datum = std::variant<Atom, std::shared_ptr<SExpr>>;

struct SExpr : std::enable_shared_from_this<SExpr> {
    Datum car;
    std::list<Datum> cdr;

    SExpr(Atom atom) {
        car.emplace(std::move(atom));
    }

    SExpr(std::shared_ptr<SExpr> ptr) {
        car.emplace(std::move(ptr));
    }
};

using SpecialForm = std::function<Datum(std::list<Datum>&)>;
class SymbolTable : public std::enable_shared_from_this {
  private:
    std::unordered_map<std::string, std::variant<Datum, SpecialForm>> table;
    std::shared_ptr<SymbolTable> parent;

  public:
    explicit SymbolTable(std::shared_ptr<SymbolTable> p) : parent(std::move(p)) {}
    std::variant<Datum, SpecialForm> &operator[](const std::string &s) const {
        if (auto it = table.find(s); it == table.end()) {
            if (parent == nullptr) {
                throw "A runtime error which should be handled in some way";
            } else {
                return (*parent)[s];
            }
        } else {
            return *it;
        }
    }

    std::variant<Datum, SpecialForm> &emplace(const std::string &s, const Datum &datum) {
        return *table.emplace(s, datum).first;
    }

    shared_ptr<SymbolTable> makeChild() {
        return std:::make_shared<SymbolTable>(shared_from_this());
    }
};
