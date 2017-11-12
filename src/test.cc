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

void runEvalTest(string_view programText, Number result) {
    Lexer lex;
    Parser parser;
    Evaluator ev;
    auto tokens = lex.getTokens(programText);
    auto expr   = parser.parse(tokens);
    if (!expr) {
        TS_ASSERT(false);
    } else {
        TS_ASSERT_EQ(*ev.eval(*expr)->getAtomicValue<Number>(), result);
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {

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

    runEvalTest("(+ 45 23)"sv, 68L);
    runEvalTest("(+ 1 2 3 4 )"sv, 10L);
    runEvalTest("((lambda (x) x) 4)"sv, 4L);
    runEvalTest("((lambda (x) (+ x x)) 4)"sv, 8L);
    runEvalTest("((lambda (x y) (+ x y)) 4 5)"sv, 9L);
    // Turns out that after all of the effort I went into getting this test
    // to work, this syntax isn't even supported in clisp, to apply a function passed as
    // an arg you need (apply f (args...)) ... seems kind of odd though, this makes sense to
    // me...maybe I'll just call it a language extension until/unless I see a reason not
    // to allow it
    runEvalTest("((lambda (x y) (x y)) (lambda (z) (+ z z z)) 5)"sv, 15L);

    runEvalTest("(if nil 4 5)"sv, 5L);
    runEvalTest("(if 1 4 5)"sv, 4L);

    runEvalTest("(+ 3.4 4.5)", 7.9);
    TS_SUMMARIZE();
}
