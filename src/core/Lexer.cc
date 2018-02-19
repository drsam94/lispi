// (c) 2017-2018 Sam Donow
#include "Lexer.h"
#include "util/Util.h"
#include <cstdlib>
#include <tuple>

std::pair<Token, std::string_view> Lexer::getWhile(TokenType type, std::string_view input,
                                                   bool (*pred)(char)) {
    auto it = input.begin();
    for (; it < input.end() && pred(*it); ++it)
        ;
    size_t len = static_cast<size_t>(std::distance(input.begin(), it));
    return {{type, input.substr(0, len)}, input.substr(len)};
}

// TODO: use some sort of real parsing/lexing framework
std::pair<Token, std::string_view> Lexer::next(std::string_view input) {
    auto it = input.begin();
    for (; it < input.end() && !isDelimeter(*it); ++it)
        ;
    if (it == input.begin()) {
        // We start with a delimeter.
        if (isParen(*it)) {
            return {{TokenType::Paren, input.substr(0, 1)}, input.substr(1)};
        }
        if (isSpace(*it)) {
            return getWhile(TokenType::Trivia, input, &Lexer::isSpace);
        }
        if (isDoubleQuote(*it)) {
            bool isEscaped = false;
            for (++it; it < input.end() && !(!isEscaped && isDoubleQuote(*it)); ++it)
                isEscaped = !isEscaped && *it == '\\';
            if (it == input.end()) {
                throw "Unterminated String";
            }
            const size_t len = static_cast<size_t>(std::distance(input.begin(), it));
            return {{TokenType::String, input.substr(1, len - 1)}, input.substr(len + 1)};
        }
        if (*it == '\'') {
            return {{TokenType::Quote, input.substr(0, 1)}, input.substr(1)};
        }
        if (++it == input.end()) {
            return {{TokenType::Trivia, {}}, {}};
        }
    }
    std::string_view tokenText{
        &*input.begin(), static_cast<size_t>(std::distance(input.begin(), it))};
    std::string tokenString{tokenText};
    char* endptr;
    const char *cstr = +tokenString;
    std::strtod(cstr, &endptr);
    if (endptr < cstr + tokenString.size()) {
        // It's not a number
        return {{TokenType::Symbol, tokenText}, input.substr(tokenText.size())};
    }
    return {{TokenType::Number, tokenText}, input.substr(tokenText.size())};
}

std::vector<Token> Lexer::getTokens(std::string_view input) {
    std::vector<Token> tokens;
    while (!input.empty()) {
        auto [token, nextInput] = next(input);
        if (token.getType() != TokenType::Trivia) {
            tokens.emplace_back(std::move(token));
        }
        input = nextInput;
    }
    return tokens;
}
