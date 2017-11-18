// (c) Samuel Donow 2017
#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

class EnumInternal {
  protected:
    // This will be called at the beginning of the program, during static variable
    // initialization time. We could add compile time support, but it isn't trivial
    // as std::array, std::string, std::algorithms don't have constexpr support
    template <size_t N>
    static inline std::array<std::string, N>
    internalGetArr(const std::string &&str) {
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

#define ENUM(EnumType, UnderlyingType, ...)                                    \
    class EnumType : EnumInternal {                                            \
      public:                                                                  \
        enum EnumT : UnderlyingType { __VA_ARGS__, Unset };                    \
        static inline const size_t Count = UnderlyingType(EnumT::Unset) + 1;   \
                                                                               \
      private:                                                                 \
        UnderlyingType _val;                                                   \
        static inline const std::array<std::string, Count> _names =            \
            internalGetArr<Count>(#__VA_ARGS__);                               \
                                                                               \
      public:                                                                  \
        EnumType() : _val(EnumT::Unset) {}                                     \
        constexpr EnumType(UnderlyingType ul) : _val(ul) {}                    \
        constexpr EnumType(EnumT other) : _val(other){};                       \
        operator EnumT() const { return static_cast<EnumT>(_val); }            \
        explicit operator UnderlyingType() const { return toUnderlying(); }    \
        UnderlyingType toUnderlying() const { return _val; }                   \
        static EnumType parse(std::string_view name) {                         \
            if (auto it = std::find(_names.begin(), _names.end(), name);       \
                it != _names.end())                                            \
                return static_cast<UnderlyingType>(it - _names.begin());       \
            else                                                               \
                return EnumT::Unset;                                           \
        }                                                                      \
        const std::string &str() const {                                       \
            return _names[static_cast<size_t>(toUnderlying())];                \
        }                                                                      \
        friend std::ostream &operator<<(std::ostream &os,                      \
                                        const EnumType &val) {                 \
            return os << val.str();                                            \
        }                                                                      \
        bool valid() const {                                                   \
            return static_cast<EnumT>(_val) != EnumT::Unset;                   \
        }                                                                      \
        static std::array<EnumType, Count - 1> values() {                      \
            std::array<EnumType, Count - 1> ret;                               \
            for (UnderlyingType i = 0; i < EnumT::Unset; ++i) {                \
                ret[i] = static_cast<EnumType>(i);                             \
            }                                                                  \
            return ret;                                                        \
        }                                                                      \
    };                                                                         \
    namespace std {                                                            \
    template <> struct hash<EnumType> {                                        \
        size_t operator()(const EnumType &x) const {                           \
            return static_cast<size_t>(x.toUnderlying());                      \
        }                                                                      \
    };                                                                         \
    }
