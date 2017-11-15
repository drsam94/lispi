// (c) 2017 Sam Donow
#pragma once

#include "util/Util.h"
#include <iostream>
#include <variant>
// Type to encapsulate numbers in lisp. Only implementing a subset of features for now
class Number {
    std::variant<long, double> data;
  public:
    // Default construct with long value, as long will get coerced to other
    // types in operations
    Number() : data{0L} {}
    Number (long lnum) : data{lnum} {}
    Number (double dnum) : data{dnum} {}

    const Number operator+(const Number& other) const {
        return std::visit(Visitor {
            [](long v1, long v2) { return Number{v1 + v2}; },
            [](double v1, double v2) { return Number{v1 + v2}; },
            [](long v1, double v2) { return Number{double(v1) + v2}; },
            [](double v1, long v2) { return Number{v1 + double(v2)}; }
        }, data, other.data);
    }

    const Number operator-(const Number& other) const {
        return std::visit(Visitor {
            [](long v1, long v2) { return Number{v1 - v2}; },
            [](double v1, double v2) { return Number{v1 - v2}; },
            [](long v1, double v2) { return Number{double(v1) - v2}; },
            [](double v1, long v2) { return Number{v1 - double(v2)}; }
        }, data, other.data);
    }

    Number& operator+=(const Number& other) {
        return *this = *this + other;
    }
    Number& operator-=(const Number& other) {
        return *this = *this - other;
    }
    bool operator==(const Number& other) const {
        return std::visit(Visitor {
            [](long v1, long v2) { return v1 == v2; },
            [](double v1, double v2) { return v1 == v2; },
            [](long v1, double v2) { return double(v1) == v2; },
            [](double v1, long v2) { return v1 == double(v2); }
        }, data, other.data);
    }

    friend std::ostream &operator<<(std::ostream &os, const Number &num) {
        return std::visit(Visitor {
            [&os](auto v)   -> std::ostream& { return os << v; },
            [&os](double v) -> std::ostream& { return os << v; }
        }, num.data);
    }
};
