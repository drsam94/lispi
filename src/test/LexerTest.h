// (c) Sam Donow 2017-2018
#pragma once
#include "test/TestSuite.h"
#include "Lexer.h"

#include <vector>
#include <string_view>
class LexerTester : public Tester<LexerTester> {
    void runLexTest(std::string_view programText,
                    std::vector<Token> expectedTokens) {
        Lexer lex;
        std::vector<Token> actualTokens = lex.getTokens(programText);
        TS_ASSERT_EQ(actualTokens.size(), expectedTokens.size());
        for (auto itTest = actualTokens.begin(),
                  itGold = expectedTokens.begin();
             itTest < actualTokens.end() && itGold < expectedTokens.end();
             ++itTest, ++itGold) {
            TS_ASSERT_EQ(*itTest, *itGold);
        }
    }

  public:
    void run() {
        initialize();
        runLexTest("(+ 1 2)", {{TokenType::Paren, "("},
                               {TokenType::Symbol, "+"},
                               {TokenType::Number, "1"},
                               {TokenType::Number, "2"},
                               {TokenType::Paren, ")"}});

        runLexTest("(atom? (quote (234 <=>)))", {{TokenType::Paren, "("},
                                                 {TokenType::Symbol, "atom?"},
                                                 {TokenType::Paren, "("},
                                                 {TokenType::Symbol, "quote"},
                                                 {TokenType::Paren, "("},
                                                 {TokenType::Number, "234"},
                                                 {TokenType::Symbol, "<=>"},
                                                 {TokenType::Paren, ")"},
                                                 {TokenType::Paren, ")"},
                                                 {TokenType::Paren, ")"}});

        runLexTest("(car '(1 2))", {{TokenType::Paren, "("},
                                    {TokenType::Symbol, "car"},
                                    {TokenType::Quote, "'"},
                                    {TokenType::Paren, "("},
                                    {TokenType::Number, "1"},
                                    {TokenType::Number, "2"},
                                    {TokenType::Paren, ")"},
                                    {TokenType::Paren, ")"}});
        runLexTest("'(1 2)", {{TokenType::Quote, "'"},
                              {TokenType::Paren, "("},
                              {TokenType::Number, "1"},
                              {TokenType::Number, "2"},
                              {TokenType::Paren, ")"}});

        runLexTest("(1+ -1)", {{TokenType::Paren, "("},
                               {TokenType::Symbol, "1+"},
                               {TokenType::Number, "-1"},
                               {TokenType::Paren, ")"}});
    }
};
