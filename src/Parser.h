// (c) 2017 Sam Donow
#pragma once

#include <list>
#include <memory>
#include "Token.h"

class SExpr {
    std::string car;
    std::list<std::shared_ptr<SExpr>> cdr;
}
class Parser {
    vector<SExpr> parse
}
