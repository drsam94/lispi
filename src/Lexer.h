// (c) 2017 Sam Donow
#pragma once
#include "util/Enum.h"
#include "Token.h"

#include <string_view>
#include <utility>
#include <vector>
#include <iostream>

class Lexer {
    static bool isParen(char c) { return c == '(' || c == ')'; }

    static bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

    static bool isNumeric(char c) {
        // TODO: parse more than just decimal ints
        return c >= '0' && c <= '9';
    }
    static bool isQuote(char c) { return c == '"'; }

    static bool isSymbolic(char c) {
        return !isParen(c) && !isSpace(c) && !isNumeric(c) && !isQuote(c);
    }

    std::pair<Token, std::string_view> getToken(TokenType type, std::string_view input,
            bool (*pred)(char)) {
        auto it = input.begin();
        for (; it < input.end() && pred(*it); ++it);
        size_t len = std::distance(input.begin(), it);
        return {Token(type, input.substr(0, len)), input.substr(len)};
    }
  public:
    std::pair<Token, std::string_view> next(std::string_view input) {
        if (isParen(input[0])) {
            return getToken(TokenType::Paren, input, &Lexer::isParen);
        } else if (isSpace(input[0])) {
            return getToken(TokenType::Trivia, input, &Lexer::isSpace);
        } else if (isNumeric(input[0])) {
            return getToken(TokenType::Number, input, &Lexer::isNumeric);
        } else if (isQuote(input[0])) {
            // TODO: make this actually work, this only grabs the empty string
            return getToken(TokenType::String, input, &Lexer::isQuote);
        } else {
            return getToken(TokenType::Symbol, input, &Lexer::isSymbolic);
        }
    }

    std::vector<Token> getTokens(std::string_view input) {
        std::vector<Token> tokens;
        while (!input.empty()) {
            std::tie(tokens.emplace_back(), input) = next(input);
        }
        return tokens;
    }
};
