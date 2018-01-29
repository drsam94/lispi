// (c) 2017 Sam Donow
#pragma once
#include "util/Enum.h"
#include "data/Token.h"

#include <iostream>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>
/// The Lexer is responsible for converting the input text to a stream of tokens,
/// which can either be accessed one at a time through ::next or all at once through
/// getTokens()
class Lexer {
    static constexpr bool isParen(char c) { return c == '(' || c == ')'; }

    static constexpr bool isDelimeter(char c) {
        return isSpace(c) || isParen(c) || isDoubleQuote(c) || c == ';' ||
               c == '\'' || c == '`' || c == '|' || c == '[' || c == ']' ||
               c == '{' || c == '}';
    }
    static constexpr bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\0'; }

    static constexpr bool isDoubleQuote(char c) { return c == '"'; }

    std::pair<Token, std::string_view>
    getWhile(TokenType type, std::string_view input, bool (*pred)(char));

  public:
    std::pair<Token, std::string_view> next(std::string_view input);

    std::vector<Token> getTokens(std::string_view input);

};
