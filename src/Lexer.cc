// (c) 2017 Sam Donow
#include "Lexer.h"
#include <tuple>

std::pair<Token, std::string_view>
Lexer::getToken(TokenType type, std::string_view input, bool (*pred)(char)) {
    auto it = input.begin();
    for (; it < input.end() && pred(*it); ++it)
        ;
    size_t len = std::distance(input.begin(), it);
    return { {type, input.substr(0, len)}, input.substr(len)};
}

std::pair<Token, std::string_view> Lexer::next(std::string_view input) {
    if (isParen(input[0])) {
        return { {TokenType::Paren, input.substr(0, 1)}, input.substr(1)};
    } else if (input[0] == '\'') {
        return { {TokenType::Quote, input.substr(0, 1)}, input.substr(1)};
    } else if (isSpace(input[0])) {
        return getToken(TokenType::Trivia, input, &Lexer::isSpace);
    } else if (isNumeric(input[0])) {
        return getToken(TokenType::Number, input, &Lexer::isNumeric);
    } else if (isDoubleQuote(input[0])) {
        // TODO: make this actually work, this only grabs the empty string
        return getToken(TokenType::String, input, &Lexer::isDoubleQuote);
    } else {
        return getToken(TokenType::Symbol, input, &Lexer::isSymbolic);
    }
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
