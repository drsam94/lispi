// (c) Sam Donow 2017-2018
#include "data/Error.h"
#include "util/Util.h"
#pragma once

/// A general Rational type. TODO: optimize for our case of Numeric=BigInt
template<typename Numeric>
class Rational {
    Numeric high{};
    Numeric low{};
  public:

    Rational(Numeric x, Numeric y) : high(x), low(y) {
        bool isNegative = false;
        if (high < Numeric{0}) {
            high *= Numeric{-1};
            isNegative = !isNegative;
        }
        if (low < Numeric{0}) {
            low *= Numeric{-1};
            isNegative = !isNegative;
        }
        const Numeric z = gcd(high, low);
        high /= z;
        low  /= z;
        if (isNegative) {
            high *= Numeric{-1};
        }
    }

    explicit Rational(Numeric x) : Rational(x, Numeric{1}) {}
    Rational() : Rational(Numeric{0}, Numeric{1}) {}
    static Numeric gcd(const Numeric& a, const Numeric& b) {
        if (b == Numeric{}) {
            return a;
        } else {
            return gcd(b, a % b);
        }
    }

    const Numeric& numerator() const noexcept { return high; }
    const Numeric& denominator() const noexcept { return low; }
    Rational inverse() const {
        if (unlikely(low == Numeric{})) {
            throw LispError("Division by zero");
        } else {
            return {low, high};
        }
    }

    Rational operator+() const noexcept {
        return {+high, +low};
    }
    Rational operator+(const Rational& other) const noexcept {
        return {high * other.low + low * other.high, low * other.low};
    }
    Rational operator-() const noexcept {
        return {-high, low};
    }
    Rational operator-(const Rational& other) const noexcept {
        return *this + (-other);
    }
    Rational operator*(const Rational& other) const noexcept {
        return {high * other.high, low * other.low};
    }
    Rational operator/(const Rational& other) const {
        return *this * other.inverse();
    }

    Rational& operator+=(const Rational& other) noexcept {
        return (*this = *this + other);
    }
    Rational& operator-=(const Rational& other) noexcept {
        return (*this = *this - other);
    }
    Rational& operator*=(const Rational& other) noexcept {
        return (*this = *this * other);
    }
    Rational& operator/=(const Rational& other) {
        return (*this = *this / other);
    }

    explicit operator double() const noexcept {
        // There are almost certainly better ways of doing this
        return static_cast<double>(high) / static_cast<double>(low);
    }
    // C++20 operator<=>
    Numeric compare(const Rational& other) const noexcept {
        return high * other.low - other.high * low;
    }
    SPACESHIP_BOILERPLATE(Rational, compare, Numeric)

    friend std::ostream& operator<<(std::ostream& os, const Rational& r) noexcept {
        return os << r.high << "/" << r.low;
    }
};
