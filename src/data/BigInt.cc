// (c) Sam Donow 2017
#include "BigInt.h"
#include "util/Util.h"

#include <algorithm>

void BigInt::sumAbsVal(const BigInt& a, const BigInt& b, BigInt& sum) {
    uint64_t carry = 0;
    if (a.data.size() < b.data.size()) {
        sumAbsVal(b, a, sum);
        return;
    }
    sum.data.reserve(a.data.size() + 1);
    auto aIt = a.data.begin();
    for (auto bIt = b.data.begin(); bIt != b.data.end(); ++aIt, ++bIt) {
        const uint64_t digitSum =
            static_cast<uint64_t>(*aIt) + static_cast<uint64_t>(*bIt) + carry;
        sum.data.emplace_back(digitSum & MAX_DIGIT);
        carry = digitSum / MAX_DIGIT;
    }
    uint32_t bExt = b.isNegative ? 0U : MAX_DIGIT;
    for (; aIt != a.data.end(); ++aIt) {
        const uint64_t digitSum = static_cast<uint64_t>(*aIt) + bExt + carry;
        sum.data.emplace_back(digitSum & MAX_DIGIT);
        carry = digitSum >> 32;
    }
    if (carry != 0) {
        sum.data.emplace_back(carry);
    }
}

void BigInt::mulAbsVal(const BigInt& a, const BigInt& b, BigInt& prod) {
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
            const uint64_t digitProd =
                (static_cast<uint64_t>(*aIt) * static_cast<uint64_t>(*bIt)) + carry;
            current.data.emplace_back(digitProd % MAX_DIGIT);
            carry = digitProd / MAX_DIGIT;
        }
        if (carry != 0) {
            current.data.emplace_back(carry);
        }
    }
    prod = std::accumulate(parts.begin(), parts.end(), BigInt{});
}

void BigInt::diffAbsVal(const BigInt& a, const BigInt& b, BigInt& diff) {
    if (auto cmp = a.compare(-b); cmp == 0) {
        return;
    } else if (cmp < 0) {
        diffAbsVal(b, a, diff);
        diff.isNegative = true;
        return;
    }

    uint32_t carry = 0;
    auto aIt = a.data.begin();
    for (auto bIt = b.data.begin(); bIt != b.data.end(); ++aIt, ++bIt) {
        const uint32_t aDig = *aIt - carry;
        if (aDig >= *bIt) {
            diff.data.emplace_back(aDig - *bIt);
            carry = carry == 1 && *aIt == 0 ? 1 : 0;
        } else {
            carry = 1;
            // Parentheses prevent overflow; wrapping would yield the correct answer, but
            // this is nicer at least conceptually
            diff.data.emplace_back((MAX_DIGIT - *bIt) + *aIt);
        }
    }
    for (; aIt != a.data.end(); ++aIt) {
        const uint32_t aDig = *aIt - carry;
        diff.data.emplace_back(aDig);
        carry = carry == 1 && *aIt == 0 ? 1 : 0;
    }
}

uint32_t BigInt::divisionDigit(const BigInt& dividend, const BigInt& divisor) {
    return util::binSearch<uint32_t>([&](uint32_t value) {
        // TODO: define operator*(uint32_t) so this doesn't involve an
        // unnecssary vector construction?
        BigInt c{value};
        BigInt r = dividend - (divisor * c);
        if (r.isNegative) {
            return 1;
        } else if (r < divisor) {
            return 0;
        } else {
            return -1;
        }
    });
}

void BigInt::divAbsVal(const BigInt& dividend, const BigInt& divisor, BigInt& ret) {
    BigInt currentGrouping{empty_construct{}};
    for (uint32_t digit : dividend.data) {
        currentGrouping.data.insert(currentGrouping.data.begin(), digit);
        if (divisor > currentGrouping) {
            if (!ret.data.empty()) {
                // Don't add leading zeroes
                ret.data.push_back(0);
            }
        } else {
            uint32_t div = divisionDigit(currentGrouping, divisor);
            ret.data.push_back(div);
            currentGrouping -= divisor * BigInt{div};
        }
    }
    // we added the digits front to back
    std::reverse(ret.data.begin(), ret.data.end());
}

std::pair<BigInt, uint32_t> BigInt::divAndMod(uint32_t modulus) {
    BigInt ret{empty_construct{}};
    uint64_t digiPair = 0;
    for (uint32_t digit : data) {
        digiPair <<= 32;
        digiPair += digit;
        if (modulus > digiPair) {
            if (!ret.data.empty()) {
                ret.data.push_back(0);
            }
        } else {
            uint64_t div = digiPair / modulus;
            ret.data.push_back(static_cast<uint32_t>(div));
            digiPair -= div * static_cast<uint64_t>(modulus);
        }
    }
    std::reverse(ret.data.begin(), ret.data.end());
    BigInt mod = *this - (ret * BigInt{static_cast<int>(modulus)});
    return {ret, mod.data[0]};
}

void BigInt::canonicalize() {
    auto nzIt = data.begin();
    for (auto it = data.begin(); it != data.end(); ++it) {
        nzIt = (*it == 0) ? it : data.end();
    }
    data.erase(nzIt, data.end());
    if (data.empty()) {
        data.push_back(0);
        isNegative = false;
    }
}

int64_t BigInt::compare(const BigInt& other) const {
    if (!sameSign(other)) {
        return isNegative ? -1 : 1;
    } else if (data.size() < other.data.size()) {
        return -1;
    } else if (data.size() > other.data.size()) {
        return 1;
    }
    for (auto aIt = data.begin(), bIt = other.data.begin(); aIt != data.end(); ++aIt, ++bIt) {
        // This will become diff = *aIt <=> *bIt
        if (int64_t diff = static_cast<int64_t>(*aIt) - static_cast<int64_t>(*bIt);
            diff != 0) {
            return diff;
        }
    }
    return 0;
}

std::ostream& operator<<(std::ostream& os, const BigInt& val) {
    static constexpr uint32_t MODULUS = 1'000'000'000U;
    if (val.data.empty()) {
        return os << "<empty>";
    }
    if (val.data.size() == 1) {
        if (val.isNegative) {
            os << "-";
        }
        return os << val.data.front();
    }
    BigInt v = val;
    std::vector<uint32_t> partsToPrint;
    if (v.isNegative) {
        v = -v;
        os << "-";
    }
    do {
        auto[div, mod] = v.divAndMod(MODULUS);
        partsToPrint.emplace_back(mod);
        v = std::move(div);
    } while (v != BigInt{0} && !v.data.empty());
    for (size_t i = partsToPrint.size() - 1; i >= 1; --i) {
        char buf[32];
        sprintf(buf, "%010u", partsToPrint[i]);
        os << buf;
    }
    return os << partsToPrint[0];
}
