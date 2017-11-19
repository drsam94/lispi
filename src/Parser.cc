// (c) 2017 Sam Donow
#include "Parser.h"

#include <math.h>
std::optional<std::shared_ptr<SExpr>>
Parser::parse(std::vector<Token> &tokens) {
    if (tokens.empty() || !tokens.begin()->isOpenParen()) {
        return std::nullopt;
    }
    auto[ret, it] = parseImpl(tokens.begin(), tokens.end());
    tokens.erase(tokens.begin(), it);
    return ret;
}

Atom Parser::atomFromToken(Token token) {
    Atom atom;
    switch (token.getType()) {
    case TokenType::Symbol: {
        atom.data = Symbol{std::string{token.getText()}};
        break;
    }
    case TokenType::String: {
        // TODO: handle this properly with quotes and escape sequences and stuff
        atom.data.emplace<std::string>(token.getText());
        break;
    }
    case TokenType::Number: {
        // TODO handle this properly for more general inputs
        double d;
        sscanf(token.getText().data(), "%lf", &d);
        if (fmod(d, 1.0) != 0.0) {
            atom.data.emplace<Number>(d);
        } else {
            atom.data.emplace<Number>(static_cast<long>(d));
        }
        break;
    }

    case TokenType::Paren:
    case TokenType::Trivia:
    case TokenType::Error:
    case TokenType::Unset:
        break;
    }
    return atom;
}

template <typename Iterator>
std::pair<std::optional<std::shared_ptr<SExpr>>, Iterator>
Parser::parseImpl(Iterator first, Iterator last) {
    // Consume openParen
    Iterator curr = first;
    ++curr;

    SExprPtr sexpr = nullptr;
    SExpr* currSexpr = nullptr;
    while (curr != last) {
        if (curr->isCloseParen()) {
            return {sexpr, ++curr};
        } else if (curr->isOpenParen()) {
            auto[ret, next] = parseImpl(curr, last);
            if (!ret) {
                return {std::nullopt, first};
            }
            const std::shared_ptr<SExpr>& ptr = *ret;
            if (!sexpr) {
                sexpr = std::make_shared<SExpr>(ptr);
                currSexpr = sexpr.get();
            } else {
                currSexpr->cdr = std::make_shared<SExpr>(ptr);
                currSexpr = currSexpr->cdr.get();
            }
            curr = next;
        } else if (curr->getType() != TokenType::Trivia) {
            Atom atom = atomFromToken(std::move(*curr));
            if (!sexpr) {
                sexpr = std::make_shared<SExpr>(std::move(atom));
                currSexpr = sexpr.get();
            } else {
                currSexpr->cdr = std::make_shared<SExpr>(std::move(atom));
                currSexpr = currSexpr->cdr.get();
            }
            ++curr;
        } else {
            ++curr;
        }
    }
    return {std::nullopt, first};
}
