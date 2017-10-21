// (c) Sam Donow 2017
#include "Parser.h"
#include "Lexer.h"

#include <string>
#include <string_view>
#include <iostream>

using namespace std;
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    Lexer lex;
    //Parser parser;
    string programText;
    string line;
    // TODO: parse a bit at a time, REPL
    while (!cin.eof()) {
        getline(cin, line);
        programText += line;
    }
    // TODO: instead of just printing tokens, go forward
    for (Token token : lex.getTokens(programText)) {
        cout << token << '\n';
    }

    // TODO: Parse
    // TODO: Evaluate
    return 0;
}
