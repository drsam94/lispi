// (c) 2017 Sam Donow
#include "Parser.h"

#include <math.h>
std::optional<std::shared_ptr<SExpr>>
Parser::parse(std::vector<Token> &tokens) {
    if (tokens.empty() ||
        !(tokens.begin()->isOpenParen() ||
          tokens.begin()->getType() == TokenType::Quote)) {
        return std::nullopt;
    }
    auto[ret, it] = parseImpl(tokens.begin(), tokens.end());
    tokens.erase(tokens.begin(), it);
    return ret;
}

Atom Parser::atomFromToken(Token token) {
    switch (token.getType()) {
    case TokenType::Symbol: {
        return Atom{Symbol{std::string{token.getText()}}};
    }
    case TokenType::String: {
        // TODO: handle this properly with quotes and escape sequences and stuff
        return Atom{std::string{token.getText()}};
    }
    case TokenType::Number: {
        // TODO handle this properly for more general inputs
        double d;
        sscanf(token.getText().data(), "%lf", &d);
        if (fmod(d, 1.0) != 0.0) {
            return Atom{Number{d}};
        }
        return Atom{Number{static_cast<long>(d)}};
    }

    case TokenType::Paren:
    case TokenType::Trivia:
    case TokenType::Error:
    case TokenType::Unset:
    case TokenType::Quote:
        break;
    }
    return Atom{};
}

template <typename Iterator>
std::pair<std::optional<std::shared_ptr<SExpr>>, Iterator>
Parser::parseImpl(Iterator first, Iterator last) {
    Iterator curr = first;
    if (curr->isOpenParen()) {
        // Consume open paren if it is there
        ++curr;
    }

    SExprPtr sexpr = nullptr;
    SExpr* currSexpr = nullptr;
    while (curr != last) {
        if (curr->isCloseParen()) {
            return {sexpr, ++curr};
        } else if (curr->getType() == TokenType::Quote) {
            // The Quote character is really just syntactic sugar for the
            // special form quote
            SExprPtr newSexpr = std::make_shared<SExpr>(Atom{Symbol{"quote"}});
            const bool isEntire = sexpr == nullptr;
            if (isEntire) {
                sexpr = std::move(newSexpr);
                currSexpr = sexpr.get();
            } else {
                currSexpr->cdr = std::make_shared<SExpr>(std::move(newSexpr));
                currSexpr = currSexpr->cdr.getSExpr()->car.getSExpr().get();
            }
            auto[cdr, next] = parseImpl(++curr, last);
            if (!cdr) {
                return {std::nullopt, first};
            }
            currSexpr->cdr = std::make_shared<SExpr>(std::move(*cdr));
            currSexpr = currSexpr->cdr.getSExpr().get();
            curr = next;
            if (isEntire) {
                return {sexpr, curr};
            }
        } else if (curr->isOpenParen()) {
            auto[ret, next] = parseImpl(curr, last);
            if (!ret) {
                return {std::nullopt, first};
            }
            const std::shared_ptr<SExpr>& ptr = *ret;
            if (sexpr == nullptr) {
                sexpr = std::make_shared<SExpr>(ptr);
                currSexpr = sexpr.get();
            } else {
                currSexpr->cdr = std::make_shared<SExpr>(ptr);
                currSexpr = currSexpr->cdr.getSExpr().get();
            }
            curr = next;
        } else if (curr->getType() != TokenType::Trivia) {
            Atom atom = atomFromToken(std::move(*curr));
            if (!sexpr) {
                sexpr = std::make_shared<SExpr>(std::move(atom));
                currSexpr = sexpr.get();
            } else {
                currSexpr->cdr = std::make_shared<SExpr>(std::move(atom));
                currSexpr = currSexpr->cdr.getSExpr().get();
            }
            ++curr;
        } else {
            ++curr;
        }
    }
    return {std::nullopt, first};
}
