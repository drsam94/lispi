// (c) 2017 Sam Donow
#pragma once

#include "data/Token.h"
#include "data/Data.h"

#include <list>
#include <memory>
#include <vector>
#include <optional>
#include <utility>


/// The parser takes in a stream of tokens, and parses their structure; it always
/// returns an SExpr, as that is the only fundamental structure in Lisp
class Parser {
  public:
    std::optional<SExprPtr> parse(std::vector<Token>& tokens);

  private:
    static Atom atomFromToken(const Token& token);

    template<typename Iterator>
    std::pair<std::optional<SExprPtr>, Iterator>
    static parseImpl(Iterator first, Iterator last);
 };
