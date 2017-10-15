// (c) 2017 Sam Donow
#pragma once

#include <list>
#include <memory>
#include <vector>
#include <optional>
#include <utility>

#include "Token.h"
#include "Data.h"

class Parser {
  public:
    std::optional<SExpr> parse(std::vector<Token> &tokens) {
        if (tokens.empty() || tokens.begin()->isOpenParen()) {
            return nullopt;
        }
        auto [ret, it] = parseImpl(tokens.begin(), tokens.end());
        tokens.erase(tokens.begin(), it);
        return ret;
    }

  private:

    Atom atomFromToken(Token token) {
        switch (token.getType()) {
            case TokenType::Symbol: {
                Atom atom;
                atom.data.emplace<Symbol>(token.getText());
                return;
            }
            case TokenType::String: {
                Atom atom;
                // TODO: handle this properly with quotes and escape sequences and stuff
                atom.data.emplace<std::string>(token.getText());
                return atom;
            }
            case TokenType::Number {
                // TODO handle this properly for integers
                double d;
                Atom atom;
                sscanf(token.getText().data(), "%g", &d);
                atom.data.emplace<double>(d);
                return atom;
            }

            case TokenType::Paren:
            case TokenType::Trivia:
                return {};
        }
    }

    template<typename Iterator>
    std::pair<optional<SExpr>, Iterator>
    parseImpl(Iterator first, Iterator last) {
        // Consume openParen
        auto curr = first;
        ++curr;
        std::optional<SExpr> sexpr = nullopt;
        while (curr != last) {
            if (curr->isCloseParen()) {
                return {sexpr, ++curr};
            } else if (curr->isOpenParen()) {
                auto [ret, next] = parseImpl(curr, last);
                if (!ret) {
                    return {nullopt, last};
                }
                auto ptr = make_shared<SExpr>(std::move(*ret));
                if (!sexpr) {
                    sexpr.emplace(std::move(ptr));
                } else {
                    sexpr->cdr.emplace_back(std::move(ptr));
                }
                curr = next;
            } else if (!curr->isTrivia()) {
                Atom atom = atomFromToken(std::move(*curr));
                if (!sexpr) {
                    sexpr.emplace(std::move(atom));
                } else {
                    sexpr->cdr.emplace_back(std::move(atom));
                }
                ++curr;
            } else {
                ++curr;
            }
        }
        return {nullopt, last};
    }
}
