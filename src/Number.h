// (c) 2017 Sam Donow
#pragma once

#include "util/Util.h"
#include <iostream>
#include <math.h>
#include <variant>

// Type to encapsulate numbers in lisp. Only implementing a subset of features for now
class Number {
    // Need to support Rationals and bignums, maintain the idea of "exactness"...
    std::variant<long, double> data;

    static bool fuzzyEq(double a, double b) {
        static constexpr double epsilon = 0x1p-20;
        return fabs(a - b) < epsilon;
    }
  public:
    // Default construct with long value, as long will get coerced to other
    // types in operations
    constexpr Number() : data{0L} {}
    constexpr Number (long lnum) : data{lnum} {}
    constexpr Number (double dnum) : data{dnum} {}

    const Number operator+(const Number& other) const {
        return std::visit([](auto v1, auto v2) { return Number{v1 + v2}; },
            data, other.data);
    }

    const Number operator-(const Number& other) const {
        return std::visit([](auto v1, auto v2) { return Number{v1 - v2}; },
            data, other.data);
    }

    const Number operator-() const {
        return std::visit([](auto v1) { return Number{-v1}; }, data);
    }

    const Number operator+() const {
        return std::visit([](auto v1) { return Number{+v1}; }, data);
    }

    const Number operator*(const Number& other) const {
        return std::visit([](auto v1, auto v2) { return Number{v1 * v2}; },
            data, other.data);
    }

    // Note: (long / long) => Rational in Lisp
    const Number operator/(const Number& other) const {
        return std::visit([](auto v1, auto v2) { return Number{v1 / v2}; },
            data, other.data);
    }

    Number& operator+=(const Number& other) {
        return *this = *this + other;
    }
    Number& operator-=(const Number& other) {
        return *this = *this - other;
    }
    Number& operator*=(const Number& other) {
        return *this = *this * other;
    }
    Number& operator/=(const Number& other) {
        return *this = *this / other;
    }

    bool operator==(const Number& other) const {
        return std::visit(
            Visitor{[](double v1, double v2) { return fuzzyEq(v1, v2); },
                    [](double v1, long v2) { return fuzzyEq(v1, static_cast<double>(v2)); },
                    [](long v1, double v2) { return fuzzyEq(static_cast<double>(v1), v2); },
                    [](auto v1, auto v2) { return v1 == v2; }},
            data, other.data);
    }
    bool operator!=(const Number& other) const {
        return !(*this == other);
    }

    friend std::ostream &operator<<(std::ostream &os, const Number &num) {
        return std::visit([&os](auto v) -> std::ostream& { return os << v; }, num.data);
    }
};

// User-defined literals for Numbers
inline constexpr Number operator ""_N(unsigned long long int x) {
    return Number{static_cast<long>(x)};
}

inline constexpr Number operator ""_N(long double x) {
    return Number{static_cast<double>(x)};
}
