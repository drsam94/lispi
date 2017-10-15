// (c) Samuel Donow 2017
#pragma once
#include<array>
#include<string>
#include<cstddef>
#include<algorithm>
#include<iostream>

class EnumInternal {
  protected:
    template<size_t N>
    static constexpr std::array<std::string, N> internalGetArr(const std::string &&str) {
        std::array<std::string, N> ret;
        auto isWhitespace = [](char c) {
            return c == '\n' || c == ',' || c == '\0' || c == ' ' || c == '\t';
        };
        auto endIt = str.cbegin();
        auto beginIt = str.cbegin();
        auto arrIt = ret.begin();
        do {
            beginIt = std::find_if_not(endIt, str.cend(), isWhitespace);
            endIt = std::find_if(beginIt, str.end(), isWhitespace);
            *arrIt = std::string(&*beginIt, endIt - beginIt);
        } while (++arrIt != ret.end());
        return ret;
    }
};

#define ENUM(EnumType, UnderlyingType, ...) \
class EnumType : EnumInternal { \
  public: \
    enum EnumT : UnderlyingType { __VA_ARGS__, Unset }; \
    static inline const size_t Count = UnderlyingType(EnumT::Unset) + 1; \
  private: \
    UnderlyingType val; \
    static inline const std::array<std::string, Count> _names = \
        internalGetArr<Count>(#__VA_ARGS__); \
  public: \
    EnumType() : val(EnumT::Unset) {} \
    constexpr EnumType(UnderlyingType ul) : val(ul) {} \
    constexpr EnumType(EnumT other) : val(other) {}; \
    operator EnumT() const { return static_cast<EnumT>(val); } \
    explicit operator UnderlyingType() const { return toUnderlying(); } \
    UnderlyingType toUnderlying() const { return val; } \
    static EnumType parse(const std::string &name) { \
        if (auto it = std::find(_names.begin(), _names.end(), name); it != _names.end()) \
            return static_cast<UnderlyingType>(it - _names.begin()); \
        else return EnumT::Unset; \
    } \
    const std::string &str() const { return _names[size_t(toUnderlying())]; } \
    friend std::ostream &operator<<(std::ostream &os, const EnumType &val) { \
        return os << val.str(); \
    } \
    bool valid() const { return static_cast<EnumT>(val) != EnumT::Unset; } \
    static std::array<EnumType, Count - 1> values() { \
        std::array<EnumType, Count - 1> ret; \
        for (UnderlyingType i = 0; i < EnumT::Unset; ++i) { \
            ret[i] = static_cast<EnumType>(i); \
        } \
        return ret; \
    } \
}; \
namespace std { \
    template <> struct hash<EnumType> { \
        size_t operator()(const EnumType &x) const { \
            return static_cast<size_t>(x.toUnderlying()); \
        } \
    }; \
}
