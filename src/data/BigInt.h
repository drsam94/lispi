// (c) Sam Donow 2017
#pragma once
#include <algorithm>
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
    std::vector<uint32_t> data{};
    static constexpr uint32_t MAX_DIGIT = std::numeric_limits<uint32_t>::max();

    // TODO: This actually works independent of sign, I believe.
    static void sumAbsVal(const BigInt& a, const BigInt& b, BigInt& sum) {
        uint64_t carry = 0;
        if (a.data.size() < b.data.size()) {
            sumAbsVal(b, a, sum);
            return;
        }
        sum.data.reserve(a.data.size() + 1);
        auto aIt = a.data.begin();
        for (auto bIt = b.data.begin(); bIt != b.data.end(); ++aIt, ++bIt) {
            const uint64_t digitSum = static_cast<uint64_t>(*aIt) +
                                      static_cast<uint64_t>(*bIt) + carry;
            sum.data.emplace_back(digitSum & MAX_DIGIT);
            carry = digitSum / MAX_DIGIT;
        }
        uint32_t bExt = b.isNegative() ? 0U : MAX_DIGIT;
        for (; aIt != a.data.end(); ++aIt) {
            const uint64_t digitSum =
                static_cast<uint64_t>(*aIt) + bExt + carry;
            sum.data.emplace_back(digitSum & MAX_DIGIT);
            carry = digitSum >> 32;
        }
        if (carry != 0) {
            if (a.isNegative() && b.isNegative() && !sum.isNegative()) {
                // The sum overflowed in the negative direction
                sum.data.emplace_back(MAX_DIGIT + carry);
            } else if (!a.isNegative() && !b.isNegative()) {
                sum.data.emplace_back(carry);
            }
        }
    }

    // Uses the standard naive multiplication algorithm
    static void mulAbsVal(const BigInt& a, const BigInt& b, BigInt& prod) {
        if (a.data.size() < b.data.size()) {
            mulAbsVal(b, a, prod);
            return;
        }
        std::vector<BigInt> parts;
        for (auto bIt = b.data.begin(); bIt != b.data.end(); ++bIt) {
            BigInt& current = parts.emplace_back(BigInt{empty_construct{}});
            for (auto cIt = b.data.begin(); cIt != bIt; ++cIt) {
                current.data.push_back(0);
            }
            uint64_t carry = 0;
            for (auto aIt = a.data.begin(); aIt != a.data.end(); ++aIt) {
                const uint64_t digitProd = (static_cast<uint64_t>(*aIt) *
                                            static_cast<uint64_t>(*bIt)) + carry;
                current.data.emplace_back(digitProd % MAX_DIGIT);
                carry = digitProd / MAX_DIGIT;
            }
            if (carry != 0) {
                current.data.emplace_back(carry);
            }
        }
        prod = std::accumulate(parts.begin(), parts.end(), BigInt{});
    }

    bool isNegative() const {
        static constexpr uint32_t SIGN_BIT = (1u << 31);
        return data.back() & SIGN_BIT;
    }

    std::pair<BigInt, uint32_t> divAndMod(uint32_t modulus) {
        std::vector<uint32_t> quotientDigits;
        uint64_t digiPair = 0;
        for (uint32_t digit : data) {
            digiPair <<= 32;
            digiPair += digit;
            if (modulus > digiPair) {
                if (!quotientDigits.empty()) {
                    quotientDigits.push_back(0);
                }
            } else {
                uint64_t div = digiPair / modulus;
                quotientDigits.push_back(static_cast<uint32_t>(div));
                digiPair -= div * static_cast<uint64_t>(modulus);
            }
        }
        BigInt ret{empty_construct{}};
        ret.data.reserve(quotientDigits.size());
        for (auto it = quotientDigits.rbegin(); it != quotientDigits.rend(); ++it) {
            ret.data.push_back(*it);
        }
        BigInt mod = *this - (ret * modulus);
        return {ret, mod.data[0]};
    }

    void canonicalize() {
        auto nzIt = data.begin();
        const uint32_t extDigit = isNegative() ? MAX_DIGIT : 0;
        // Start iterating from the second digit, so that we don't blow away
        // -1 and 0
        for (auto it = data.begin(); it != data.end(); ++it) {
            nzIt = (*it == extDigit) ? it : data.end();
        }
        if (nzIt != data.begin()) {
            data.erase(nzIt, data.end());
        }
    }

    struct empty_construct {};
    BigInt(empty_construct) : data{} {}
  public:
    BigInt() : data{} {data.push_back(0); }
    BigInt(long val) : data{} { data.push_back(static_cast<unsigned long>(val)); }

    // Comparison Operators
    bool operator==(const BigInt& other) const {
        auto[aIt, bIt] = std::mismatch(data.begin(), data.end(),
                                       other.data.begin(), other.data.end());
        return aIt == data.end() && bIt == other.data.end();
    }
    bool  operator!=(const BigInt& other) const {
        return !(*this == other);
    }

    // Arithmetic Operators
    BigInt operator+(const BigInt& other) const {
        BigInt ret{empty_construct{}};
        sumAbsVal(*this, other, ret);
        ret.canonicalize();
        return ret;
    }

    BigInt operator-() const {
        BigInt ret{empty_construct{}};
        ret.data.resize(data.size());
        std::transform(data.begin(), data.end(), ret.data.begin(),
                       [](uint32_t digit) { return ~digit + 1; });
        ret.canonicalize();
        return ret;
    }

    BigInt operator-(const BigInt& other) const {
        return *this + -other;
    }

    BigInt operator*(const BigInt& other) const {
        BigInt ret{empty_construct{}};
        const bool neg = other.isNegative() != isNegative();
        mulAbsVal(this->abs(), other.abs(), ret);
        ret.canonicalize();
        return neg ? -ret : ret;
    }

    // Modifying Assignment Operators
    BigInt operator+=(const BigInt& other) {
        *this = *this + other;
        return *this;
    }
    BigInt operator-=(const BigInt& other) {
        *this = *this - other;
        return *this;
    }
    BigInt operator*=(const BigInt& other) {
        *this = *this * other;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const BigInt& val) {
        static constexpr uint32_t MODULUS = 1'000'000'000U;
        if (val.data.empty()) {
            return os << "<empty>";
        }
        if (val.data.size() == 1) {
            return os << static_cast<int32_t>(val.data[0]);
        }
        BigInt v = val;
        std::vector<uint32_t> partsToPrint;
        if (v.isNegative()) {
            v = -v;
            os << "-";
        }
        do {
            auto [div, mod] = v.divAndMod(MODULUS);
            partsToPrint.emplace_back(mod);
            v = std::move(div);
        } while (v != BigInt{0});
        for (size_t i = partsToPrint.size() - 1; i >= 1; --i) {
            char buf[32];
            sprintf(buf, "%010d", partsToPrint[i]);
            os << buf;
        }
        return os << partsToPrint[0];
    }

    BigInt abs() const {
        return isNegative() ? -(*this) : *this;
    }
};
