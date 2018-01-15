// (c) Sam Donow 2017-2018
#pragma once

#include "data/BigInt.h"
#include "data/Error.h"
#include "util/Util.h"

#include <iostream>
#include <math.h>
#include <variant>

// Type to encapsulate numbers in lisp. Only implementing a subset of features for now
class Number {
    // TODO:
    // std::variant<BigInt, double, Rational, Complex> data
    // Sadly this means that we can't ever do simple integer math on longs, but
    // BigInt should be optimized for the small cases
    std::variant<BigInt, double> data;

    static bool fuzzyEq(double a, double b) {
        static constexpr double epsilon = 0x1p-20;
        return fabs(a - b) < epsilon;
    }
  public:
    // Default construct with long value, as long will get coerced to other
    // types in operations
    Number() : data{BigInt{0}} {}
    Number (long lnum) : data{BigInt{static_cast<int>(lnum)}} {}
    Number (const BigInt& bnum) : data{bnum} {}
    Number (double dnum) : data{dnum} {}

    const Number operator+(const Number& other) const {
        return std::visit(Visitor {
            [](double v1, const BigInt& v2) { return Number{v1 + static_cast<double>(v2)}; },
            [](const BigInt& v1, double v2) { return Number{static_cast<double>(v1) + v2}; },
            [](const auto& v1, const auto& v2) { return Number{v1 + v2}; }
        }, data, other.data);
    }

    const Number operator-(const Number& other) const {
        return std::visit(Visitor {
            [](double v1, const BigInt& v2) { return Number{v1 - static_cast<double>(v2)}; },
            [](const BigInt& v1, double v2) { return Number{static_cast<double>(v1) - v2}; },
            [](const auto& v1, const auto& v2) { return Number{v1 - v2}; }
        }, data, other.data);
    }

    const Number operator-() const {
        return std::visit([](const auto& v1) { return Number{-v1}; }, data);
    }

    const Number operator+() const {
        return std::visit([](const auto& v1) { return Number{+v1}; }, data);
    }

    const Number operator*(const Number& other) const {
        return std::visit(Visitor {
            [](double v1, const BigInt& v2) { return Number{v1 * static_cast<double>(v2)}; },
            [](const BigInt& v1, double v2) { return Number{static_cast<double>(v1) * v2}; },
            [](const auto& v1, const auto& v2) { return Number{v1 * v2}; },
        }, data, other.data);
    }

    // Note: (long / long) => Rational in Lisp
    const Number operator/(const Number& other) const {
        return std::visit(Visitor {
            [](double v1, const BigInt& v2) { return Number{v1 / static_cast<double>(v2)}; },
            [](const BigInt& v1, double v2) { return Number{static_cast<double>(v1) / v2}; },
            [](const auto& v1, const auto& v2) { return Number{v1 / v2}; }
        }, data, other.data);
    }

    const Number operator%(const Number& other) const {
        return std::visit(
            Visitor{[](const BigInt& v1, const BigInt& v2) { return Number{v1 % v2}; },
                    [](const auto&, const auto&) -> Number { throw TypeError("exact", "inexact"); }},
            data, other.data);
    }

    Number& operator+=(const Number& other) { return *this = *this + other; }
    Number& operator-=(const Number& other) { return *this = *this - other; }
    Number& operator*=(const Number& other) { return *this = *this * other; }
    Number& operator/=(const Number& other) { return *this = *this / other; }
    Number& operator%=(const Number& other) { return *this = *this % other; }

    bool operator==(const Number& other) const {
        return std::visit(
            Visitor{[](double v1, double v2) { return fuzzyEq(v1, v2); },
                    [](double v1, const BigInt& v2) { return fuzzyEq(v1, static_cast<double>(v2)); },
                    [](const BigInt& v1, double v2) { return fuzzyEq(static_cast<double>(v1), v2); },
                    [](const auto& v1, const auto& v2) { return v1 == v2; }},
            data, other.data);
    }
    bool operator!=(const Number& other) const { return !(*this == other); }

    bool operator<(const Number& other) const {
        return std::visit(Visitor {
            [](double x, const BigInt& y) { return x < static_cast<double>(y); },
            [](const BigInt& x, double y) { return static_cast<double>(x) < y; },
            [](const auto& x, const auto& y) { return x < y; }
        }, data, other.data);
    }
    bool operator>(const Number& other) const {
        return std::visit(Visitor {
            [](double x, const BigInt& y) { return x > static_cast<double>(y); },
            [](const BigInt& x, double y) { return static_cast<double>(x) > y; },
            [](const auto& x, const auto& y) { return x > y; }
        }, data, other.data);
    }
    bool operator<=(const Number& other) const {
        return !(*this > other);
    }
    bool operator>=(const Number& other) const {
        return !(*this < other);
    }

    friend std::ostream& operator<<(std::ostream& os, const Number& num) {
        return std::visit([&os](auto v) -> std::ostream& { return os << v; }, num.data);
    }

    Number abs() const {
        return std::visit(Visitor{[](const BigInt& v) -> Number { return v.abs(); },
                                  [](double v) -> Number { return fabs(v); }},
                          data);
    }

    bool isExact() const { return std::holds_alternative<BigInt>(data); }
};
