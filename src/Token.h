// (c) 2017 Sam Donow
#pragma once
#include "util/Enum.h"
#include <iostream>
#include <string>

ENUM(TokenType, uint8_t, Paren, String, Symbol, Number, Trivia, Error)

class Token {
    TokenType type{TokenType::Error};
    std::string data;
  public:
    Token() = default;

    Token(TokenType ty, std::string_view val) : type(ty), data(val) {}

    TokenType getType() const { return type; }

    std::string_view getText() const { return data; }

    bool isOpenParen() const { return type == TokenType::Paren && data[0] == '('; }

    bool isCloseParen() const { return type == TokenType::Paren && data[0] == ')'; }

    friend std::ostream& operator<<(std::ostream&, const Token&);
    friend bool operator==(const Token&, const Token&);
};

inline std::ostream& operator<<(std::ostream& os, const Token &token) {
    return os << "Token(" << token.type << ": '" << token.data << "')";
}

inline bool operator==(const Token& first, const Token& second) {
    return first.type == second.type && first.data == second.data;
}


