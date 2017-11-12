// (c) 2017 Sam Donow
#pragma once
#include "util/Enum.h"
#include "Token.h"

#include <string_view>
#include <utility>
#include <vector>
#include <iostream>

/// The Lexer is responsible for converting the input text to a stream of tokens,
/// which can either be accessed one at a time through ::next or all at once through
/// getTokens()
class Lexer {
    static bool isParen(char c) { return c == '(' || c == ')'; }

    static bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

    static bool isNumeric(char c) {
        // TODO: support exponential forms, etc
        return (c >= '0' && c <= '9') || c == '.';
    }
    static bool isQuote(char c) { return c == '"'; }

    static bool isSymbolic(char c) {
        return !isParen(c) && !isSpace(c) && !isNumeric(c) && !isQuote(c);
    }

    std::pair<Token, std::string_view>
    getToken(TokenType type, std::string_view input, bool (*pred)(char));

  public:
    std::pair<Token, std::string_view> next(std::string_view input);

    std::vector<Token> getTokens(std::string_view input);
};
