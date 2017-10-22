// (c) Sam Donow 2017
#include "Lexer.h"
#include "TestSuite.h"
#include <string>
#include <string_view>
#include <iostream>

using namespace std;

void runTest(string_view programText, std::vector<Token> expectedTokens) {
    Lexer lex;
    std::vector<Token> actualTokens  = lex.getTokens(programText);
    TS_ASSERT_EQ(actualTokens.size(), expectedTokens.size());
    for (auto itTest = actualTokens.begin(), itGold = expectedTokens.begin();
         itTest < actualTokens.end() && itGold < expectedTokens.end();
         ++itTest, ++itGold) {
        TS_ASSERT_EQ(*itTest, *itGold);
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    runTest("(+ 1 2)"sv,  {{TokenType::Paren, "("sv},
                           {TokenType::Symbol, "+"sv},
                           {TokenType::Number, "1"sv},
                           {TokenType::Number, "2"sv},
                           {TokenType::Paren, ")"sv}});

    runTest("(atom? (quote (234 <=>)))"sv, {{TokenType::Paren, "("sv},
                                             {TokenType::Symbol, "atom?"sv},
                                             {TokenType::Paren, "("sv},
                                             {TokenType::Symbol, "quote"sv},
                                             {TokenType::Paren, "("sv},
                                             {TokenType::Number, "234"sv},
                                             {TokenType::Symbol, "<=>"sv},
                                             {TokenType::Paren, ")"sv},
                                             {TokenType::Paren, ")"sv},
                                             {TokenType::Paren, ")"sv}});
    TS_SUMMARIZE();
}
