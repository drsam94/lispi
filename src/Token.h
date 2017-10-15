// (c) 2017 Sam Donow
#pragma once
#include "util/Enum.h"

ENUM(TokenType, uint8_t, Paren, String, Symbol, Number, Trivia, Error)

class Token {
    TokenType type;
    std::string data;
  public:
    Token() : type(TokenType::Error) {}
    Token(const Token&) = default;
    Token operator=(const Token&) = default;
    Token(const Token&&) = default;
    Token operator=(const Token&&) = default;

    Token(TokenType ty, std::string_view val) : type(ty), data(val) {}

    TokenType getType() const { return type; }

    std::string_view getText() const { return data; }

    bool isOpenParen() const { return type == TokenType::Paren && get<std::string>(data)[0] == '('; }

    bool isCloseParen() const { return type == TokenType::Paren && get<std::string>(data)[0] == ')'; }
  private:
};


