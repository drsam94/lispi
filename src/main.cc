// (c) Sam Donow 2017
#include "core/Evaluator.h"
#include "core/Lexer.h"
#include "core/Parser.h"

#include <iostream>
#include <string>
#include <string_view>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
using namespace std;

std::pair<string_view, bool> getline() {
    static char* line = nullptr;
    if (line != nullptr) {
        free(line);
        line = nullptr;
    }
    line = readline("> ");

    if (line != nullptr && *line != '\0') {
        add_history(line);
    }
    return {line, line == nullptr};
}

int main(int argc, char** argv) {
    // readline init
    //
    // disable tab completion
    rl_bind_key('\t', rl_insert);
    int opt;
    bool debugPrintTokens = false;
    while ((opt = getopt(argc, argv, "t")) != -1) {
        switch (opt) {
            case 't':
                debugPrintTokens = true;
                break;
        }
    }
    Lexer lex;
    Parser parser;
    Evaluator evaluator;
    string bufferedInput;
    vector<Token> tokens;
    do {
        try {
            auto [inputLine, eof] = getline();
            if (eof) {
                exit(0);
            }
            while (!inputLine.empty()) {
                auto && [ token, modInput ] = lex.next(inputLine);
                tokens.emplace_back(move(token));
                inputLine = modInput;
            }
            if (debugPrintTokens) {
                for (const Token& token : tokens) {
                    cout << token << ", ";
                }
                cout << endl;
            }
            auto expr = parser.parse(tokens);
            while (expr) {
                auto result = evaluator.eval(*expr);
                cout << result << endl;
                expr = parser.parse(tokens);
            }
        } catch (const LispError& err) {
            cout << "error: " << err.what() << endl;
        }
    } while (true);
}
