// (c) 2017 Sam Donow
#pragma once
#include "util/Enum.h"

ENUM(TokenType, uint8_t, Paren, String, Symbol, Number, Trivia, Error)

class Token {
    TokenType type;
    // Lisp offers arbitrary precision, rigth?
    std::variant<long, double, std::string> data;
    Token() : type(TokenType::Error) {}
    Token(const Token&) = default;
    Token operator=(const Token&) = default;
    Token(const Token&&) = default;
    Token operator=(const Token&&) = default;

    template <TokenType::EnumT _ty>
    explicit Token(std::string_view val) : type(_ty) {
        if (!parseVal<_ty>(val)) {
            type = TokenType::Error;
        }
    }
  private:
    template <TokenType::EnumT _ty>
    bool parseVal(std::string_view val);
};

template<>
bool parseVal<TokenType::Paren>(std::string_view val) {
    if (val.size() == 1 && (val[0] == ')' || val[0] == '('))
        data.emplace(val);
        return true;
    }
    return false;
}

template<>
bool parseVal<TokenType::Symbol>(std::string_view val) {
    if (val.empty()) {
        return false;
    }
    data.emplace(val);
}

template<>
bool parseVal<TokenType::String>(std::string_view val) {
    // TODO: Presumably, we want to store the actual string
    // getting rid of escaped chars and such
    data.emplace(val)
    return true;
}

template<>
bool parseVal<TokenType::Trivia>(std::string_view val) {
    data.emplace(val);
    return true;
}

template<>
bool parseVal<TokenType::Number>(std::string_view val) {
    double g = 0.;
    const bool ret = sscanf(+std::string(val), "%g", &g) == 1;
    data.emplace(g);
    return ret;
}

