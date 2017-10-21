// (c) 2017 Sam Donow
#pragma once

#include "Token.h"
#include "Data.h"

#include <list>
#include <memory>
#include <vector>
#include <optional>
#include <utility>


class Parser {
  public:
    std::optional<SExpr> parse(std::vector<Token> &tokens) {
        if (tokens.empty() || tokens.begin()->isOpenParen()) {
            return std::nullopt;
        }
        auto [ret, it] = parseImpl(tokens.begin(), tokens.end());
        tokens.erase(tokens.begin(), it);
        return ret;
    }

  private:

    Atom atomFromToken(Token token) {
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
                // TODO handle this properly for integers
                double d;
                sscanf(token.getText().data(), "%lf", &d);
                atom.data.emplace<double>(d);
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

    template<typename Iterator>
    std::pair<std::optional<SExpr>, Iterator>
    parseImpl(Iterator first, Iterator last) {
        // Consume openParen
        auto curr = first;
        ++curr;
        std::optional<SExpr> sexpr = std::nullopt;
        while (curr != last) {
            if (curr->isCloseParen()) {
                return {sexpr, ++curr};
            } else if (curr->isOpenParen()) {
                auto [ret, next] = parseImpl(curr, last);
                if (!ret) {
                    return {std::nullopt, last};
                }
                auto ptr = std::make_shared<SExpr>(std::move(*ret));
                if (!sexpr) {
                    sexpr.emplace(std::move(ptr));
                } else {
                    sexpr->cdr.emplace_back(std::move(ptr));
                }
                curr = next;
            } else if (curr->getType() != TokenType::Trivia) {
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
        return {std::nullopt, last};
    }
};
