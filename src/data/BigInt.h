// (c) Sam Donow 2017-2018
#pragma once
#include "util/Util.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

// Arbitrary Precision integer
class BigInt {
    // Binary data of the integers, split into 32-bit integers for ease of performing
    // arithmetic with carry. Negative integers represented using twos complement
    // TODO: This is not highly optimized, with the idea being that the first priority is to
    // get a working implementation that can then be improved upon; most of these implementations
    // are the typical naive algorithms (other than that they operate on "digits" of size
    // 2**32)

    // Implementation: Signed Magnitude; a twos complement representation seems quite
    // difficult with the varying size
    // TODO: use a small vector type that supports SSO
    std::vector<uint32_t> data{};
    bool isNegative = false;
    static constexpr uint32_t MAX_DIGIT = std::numeric_limits<uint32_t>::max();

    // Helpers that compute the absolute value of an operation, storing the
    // result in the third parameter, which is an outparam
    static void sumAbsVal(const BigInt& a, const BigInt& b, BigInt& sum);

    static void mulAbsVal(const BigInt& a, const BigInt& b, BigInt& prod);

    static void diffAbsVal(const BigInt& a, const BigInt& b, BigInt& diff);

    // Helper for the standard division algorithm: finds for
    // a = dividend, b = divisor, returns c,r such that
    // a = bc + r where r < b
    static uint32_t divisionDigit(const BigInt& dividend, const BigInt& divisor);

    static void divAbsVal(const BigInt& dividend, const BigInt& divisor, BigInt& ret);

    // essentially a specialized version of divAbsVal when the divisor is a single digit
    std::pair<BigInt, uint32_t> divAndMod(uint32_t modulus);

    // Standardize the form of the integer: this mostly entails trimming leading zeroes
    void canonicalize();

    // returns true if this * other < 0
    bool sameSign(const BigInt& other) const {
        return isNegative == other.isNegative;
    }

    struct empty_construct {};
    BigInt(empty_construct) : data{} {}

  public:
    // TODO: should we even have this default constructor? It made sense at one point that
    // the default would be zero, but because I use outparams so much, the empty construct
    // one is much more useful
    BigInt() : data{} { data.push_back(0); }
    BigInt(int val) : data{} {
        if (val >= 0) {
            data.push_back(static_cast<uint32_t>(val));
        } else {
            isNegative = true;
            data.push_back(static_cast<uint32_t>(-val));
        }
    }
    BigInt(uint32_t val) : data{} {
        data.push_back(val);
    }

    // Disable construction from double
    template<typename = void> BigInt(double) = delete;
    template<typename = void> BigInt& operator=(double) = delete;

    // Comparison Operators

    // generate all the other comparison operators
    int64_t compare(const BigInt& other) const noexcept;
    SPACESHIP_BOILERPLATE(BigInt, compare, int64_t)

    // Unary Arithmetic Operators
    const BigInt operator-() const {
        BigInt ret = *this;
        ret.isNegative = !isNegative;
        return ret;
    }

    const BigInt& operator+() const noexcept {
        return *this;
    }

    // Binary Arithmetic Operators
    const BigInt operator+(const BigInt& other) const {
        BigInt ret{empty_construct{}};
        if (sameSign(other)) {
            sumAbsVal(*this, other, ret);
        } else if (isNegative) {
            diffAbsVal(other, *this, ret);
        } else {
            diffAbsVal(*this, other, ret);
        }
        ret.canonicalize();
        return ret;
    }

    const BigInt operator-(const BigInt& other) const {
        return *this + -other;
    }

    const BigInt operator*(const BigInt& other) const {
        BigInt ret{empty_construct{}};
        mulAbsVal(*this, other, ret);
        ret.canonicalize();
        return sameSign(other) ? ret : -ret;
    }

    const BigInt operator/(const BigInt& other) const {
        BigInt ret{empty_construct{}};
        divAbsVal(*this, other, ret);
        ret.canonicalize();
        return sameSign(other) ? ret : -ret;
    }

    const BigInt operator%(const BigInt& other) const {
        BigInt ret{empty_construct{}};
        divAbsVal(*this, other, ret);
        ret.canonicalize();
        BigInt returnVal = this->abs() - (other.abs() * ret.abs());
        return isNegative ? -returnVal : returnVal;
    }

    // Modifying Assignment Operators
    BigInt& operator+=(const BigInt& other) {
        return (*this = *this + other);
    }
    BigInt& operator-=(const BigInt& other) {
        return (*this = *this - other);
    }
    BigInt& operator*=(const BigInt& other) {
        return (*this = *this * other);
    }
    BigInt& operator/=(const BigInt& other) {
        return (*this = *this / other);
    }
    BigInt& operator%=(const BigInt& other) {
        return (*this = *this % other);
    }

    explicit operator double() const noexcept {
        constexpr double exp = 0x1p32;
        double result{};
        for (auto it = data.rbegin(); it != data.rend(); ++it) {
            result *= exp;
            result += *it;
        }
        return result;
    }
    friend std::ostream& operator<<(std::ostream& os, const BigInt& val);

    BigInt abs() const {
        BigInt ret = *this;
        ret.isNegative = false;
        return ret;
    }
};
