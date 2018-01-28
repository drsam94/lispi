// (c) Sam Donow 2017-2018
#pragma once

#include "data/BigInt.h"
#include "data/Error.h"
#include "data/Rational.h"
#include "util/Util.h"

#include <iostream>
#include <math.h>
#include <variant>

/// Type to encapsulate numbers as represented in Scheme
class Number {
    using Rat = Rational<BigInt>;
    // TODO: add support for Complex
    // std::variant<BigInt, double, Rational, Complex> data
    std::variant<BigInt, double, Rat> data;

  public:
    // Default construct with long value, as long will get coerced to other
    // types in operations
    Number() : data{BigInt{0}} {}
    Number (long lnum) : data{BigInt{static_cast<int>(lnum)}} {}
    Number (const BigInt& bnum) : data{bnum} {}
    Number (double dnum) : data{dnum} {}
    Number (const Rat& rat) : data{rat} {}

  private:
    template<template<typename> typename F, typename R>
    const R opImpl(const Number& other) const {
        return std::visit(Visitor {
            [](double v1, const BigInt& v2) {
                return R{F<double>{}(v1, static_cast<double>(v2))};
            },
            [](const BigInt& v1, double v2) {
                return R{F<double>{}(static_cast<double>(v1), v2)};
            },

            [](const Rat& v1, double v2) {
                return R{F<double>{}(static_cast<double>(v1), v2)};
            },
            [](double v1, const Rat& v2) {
                return R{F<double>{}(v1, static_cast<double>(v2))};
            },

            [](double v1, double v2) {
                return R{F<double>{}(v1, v2)};
            },
            [](const BigInt& v1, const BigInt& v2) {
                return R{F<BigInt>{}(v1, v2)};
            },
            [](const Rat& v1, const Rat& v2) {
                return R{F<Rat>{}(v1, v2)};
            },
            [](const BigInt& v1, const Rat& v2) {
                return R{F<Rat>{}(Rat{v1},v2)};
            },
            [](const Rat& v1, const BigInt& v2) {
                return R{F<Rat>{}(v1, Rat{v2})};
            }
        }, data, other.data);
    }

  public:
    const Number operator+(const Number& other) const { return opImpl<std::plus, Number>(other); }

    const Number operator-(const Number& other) const { return opImpl<std::minus, Number>(other); }

    const Number operator*(const Number& other) const { return opImpl<std::multiplies, Number>(other); }

    const Number operator-() const {
        return std::visit([](const auto& v1) { return Number{-v1}; }, data);
    }

    const Number operator+() const {
        return std::visit([](const auto& v1) { return Number{+v1}; }, data);
    }

    const Number operator/(const Number& other) const {
        return std::visit(Visitor {
            [](double v1, const BigInt& v2) { return Number{v1 / static_cast<double>(v2)}; },
            [](double v1, const Rat& v2) { return Number{v1 / static_cast<double>(v2)}; },
            [](const BigInt& v1, double v2) { return Number{static_cast<double>(v1) / v2}; },
            [](const Rat& v1, double v2) { return Number{static_cast<double>(v1) / v2}; },
            [](double v1, double v2) { return Number{v1 / v2}; },

            [](const Rat& v1, const BigInt& v2) { return Number{v1 / Rat{v2}}; },
            [](const BigInt& v1, const Rat& v2) { return Number{Rat{v1} / v2}; },
            [](const BigInt& v1, const BigInt& v2) { return Number{Rat{v1, v2}}; },
            [](const Rat& v1, const Rat& v2) { return Number{v1 / v2}; }
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
        return opImpl<std::equal_to, bool>(other);
    }
    bool operator!=(const Number& other) const {
        return opImpl<std::not_equal_to, bool>(other);
    }
    bool operator<(const Number& other) const {
        return opImpl<std::less, bool>(other);
    }
    bool operator>(const Number& other) const {
        return opImpl<std::greater, bool>(other);
    }
    bool operator<=(const Number& other) const {
        return opImpl<std::less_equal, bool>(other);
    }
    bool operator>=(const Number& other) const {
        return opImpl<std::greater_equal, bool>(other);
    }

    friend std::ostream& operator<<(std::ostream& os, const Number& num) {
        return std::visit([&os](auto v) -> std::ostream& { return os << v; }, num.data);
    }

    Number abs() const {
        return std::visit(Visitor{
            [](const BigInt& v) -> Number { return v.abs(); },
            [](double v) -> Number { return fabs(v); },
            [](const Rat& v) -> Number { return v < Rat{BigInt{0}} ? -v : v; }
        }, data);
    }

    bool isExact() const { return std::holds_alternative<BigInt>(data); }

    template<typename T>
    decltype(auto) as() const {
        return std::get<T>(data);
    }
};
