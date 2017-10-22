// (c) Sam Donow 2017
#include "Lexer.h"
#include "Parser.h"
#include "Evaluator.h"
#include "TestSuite.h"
#include <string>
#include <string_view>
#include <iostream>

using namespace std;

void runLexTest(string_view programText, std::vector<Token> expectedTokens) {
    Lexer lex;
    std::vector<Token> actualTokens  = lex.getTokens(programText);
    TS_ASSERT_EQ(actualTokens.size(), expectedTokens.size());
    for (auto itTest = actualTokens.begin(), itGold = expectedTokens.begin();
         itTest < actualTokens.end() && itGold < expectedTokens.end();
         ++itTest, ++itGold) {
        TS_ASSERT_EQ(*itTest, *itGold);
    }
}

void runEvalTest(string_view programText, long result) {
    Lexer lex;
    Parser parser;
    Evaluator ev;
    auto tokens = lex.getTokens(programText);
    auto expr   = parser.parse(tokens);
    if (!expr) {
        TS_ASSERT(false);
    } else {
        TS_ASSERT_EQ(*ev.eval(*expr)->getAtomicValue<double>(), result);
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    runLexTest("(+ 1 2)"sv, {{TokenType::Paren, "("sv},
                              {TokenType::Symbol, "+"sv},
                              {TokenType::Number, "1"sv},
                              {TokenType::Number, "2"sv},
                              {TokenType::Paren, ")"sv}});

    runLexTest("(atom? (quote (234 <=>)))"sv, {{TokenType::Paren, "("sv},
                                                {TokenType::Symbol, "atom?"sv},
                                                {TokenType::Paren, "("sv},
                                                {TokenType::Symbol, "quote"sv},
                                                {TokenType::Paren, "("sv},
                                                {TokenType::Number, "234"sv},
                                                {TokenType::Symbol, "<=>"sv},
                                                {TokenType::Paren, ")"sv},
                                                {TokenType::Paren, ")"sv},
                                                {TokenType::Paren, ")"sv}});

    runEvalTest("(+ 45 23)"sv, 68);
    runEvalTest("(+ 1 2 3 4 )"sv, 10);
    runEvalTest("((lambda (x) x) 4)"sv, 4);
    runEvalTest("((lambda (x) (+ x x)) 4)"sv, 8);
    runEvalTest("((lambda (x y) (+ x y)) 4 5)"sv, 9);
    runEvalTest("((lambda (x y) (x y)) (lambda (z) (+ z z z)) 5)"sv, 15);
    TS_SUMMARIZE();
}
