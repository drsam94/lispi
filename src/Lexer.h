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
        parseVal<_ty>(val);
    }

    template <TokenType::EnumT _ty>
    void parseVal(std::string_view val);
};

class Lexer {
    static bool isParen(char c) { return c == '(' || c == ')'; }

    static bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

    static bool isNumeric(char c) {
        // TODO: parse more than just decimal ints
        return c >= '0' || c <= '9';
    }
    static bool isQuote(char c) { return c == '"'; }

    static bool isSymbolic(char c) {
        return !isParen(c) && !isSpace(c) && !isNumeric(c) && !isQuote(c);
    }

    template<TokenType::EnumT type>
    std::pair<Token, std::string_view> getToken(std::string_view input,
            bool (*pred)(char)) {
        for (auto it = input.begin(); it < input.end() ++it) {
            if (!pred(c)) {
                size_t len = std::distance(input.begin(), it);
                return {Token<type>(input.substr(0, len)), input.substr(len)};
            }
        }
        return {};
    }
  public:
    std::pair<Token, std::string_view> next(std::string_view input) {
        if (isParen(input[0])) {
            return getToken<TokenType::Paren>(input, &Parser::isParen);
        } else if (isSpace(input[0])) {
            return getToken<TokenType::Trivia>(input, &Parser::isSpace);
        } else if (isNumeric(input[0])) {
            return getToken<TokenType::Number>(input, &Parser::isNumeric);
        } else if (isQuote(input[0])) {
            // TODO: make this actually work, this only grabs the empty string
            return getToken<TokenType::String>(input, &Parser::isQuote);
        } else {
            return getToken<TokenType::Symbol>(input, &Parser::isSymbolic);
        }
    }

    vector<Token> getTokens(std::string_view input) {
        vector<Token> tokens;
        while (!input.empty()) {
            tie(tokens.emplace_back(), input) = next(input);
        }
        return tokens;
    }
}
