#include <string>
#include <iostream>
#include "Parser.h"
#include "Lexer.h"

int main(int argc, char **argv) {
    // TODO: make this real
    Lexer lex;
    Parser parser;
    std::string line;
    while (!std::cin.eof()) {
        std::getline(std::cin, line);
        std::vector<Token> tokens = lex.getTokens(line);
        std::optional<SExpr> sexpr = parser.parse(tokens);
    }
    return 0;
}
