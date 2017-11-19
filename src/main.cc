// (c) Sam Donow 2017
#include "Evaluator.h"
#include "Lexer.h"
#include "Parser.h"

#include <iostream>
#include <string>
#include <string_view>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
using namespace std;

string_view getline() {
    static char *line = nullptr;
    if (line != nullptr) {
        free(line);
        line = nullptr;
    }
    line = readline(">");

    if (line != nullptr && *line != '\0') {
        add_history(line);
    }
    return line;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // readline init
    //
    // disable tab completion
    rl_bind_key('\t', rl_insert);

    Lexer lex;
    Parser parser;
    Evaluator evaluator;
    string bufferedInput;
    vector<Token> tokens;
    do  {
        string_view inputLine = getline();
        while (!inputLine.empty()) {
            auto &&[token, modInput] = lex.next(inputLine);
            tokens.emplace_back(move(token));
            inputLine = modInput;
        }
        auto expr = parser.parse(tokens);
        while (expr) {
            auto result = evaluator.eval(*expr);
            cout << *result << endl;
            expr = parser.parse(tokens);
        }
    } while (true);
    return 0;
}
