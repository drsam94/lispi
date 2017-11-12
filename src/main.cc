// (c) Sam Donow 2017
#include "Evaluator.h"
#include "Lexer.h"
#include "Parser.h"

#include <iostream>
#include <string>
#include <string_view>

using namespace std;
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    Lexer lex;
    // Parser parser;
    string programText;
    string line;
    // TODO: parse a bit at a time, REPL
    while (!cin.eof()) {
        getline(cin, line);
        programText += line;
    }
    // TODO: support parsing more than one SExpr

    std::vector<Token> tokens = lex.getTokens(programText);
    Parser parser;
    auto expr = parser.parse(tokens);

    Evaluator evaluator;
    auto result = evaluator.eval(*expr);
    cout << *result->getAtomicValue<Number>() << std::endl;
    return 0;
}
