// (c) Sam Donow 2017
#pragma once
// TODO: Break this up, add aditional features (like testing external representations)
#include "Lexer.h"
#include "Parser.h"
#include "Evaluator.h"

class EvalTester {
    void runEvalTest(std::string_view programText, const Datum& result) {
        Lexer lex;
        Parser parser;
        Evaluator ev;
        std::vector<Token> tokens = lex.getTokens(programText);
        auto expr = parser.parse(tokens);
        TS_ASSERT(bool(expr));
        while (expr) {
            std::optional<Datum> evaluated = ev.eval(*expr);
            TS_ASSERT(bool(evaluated));
            expr = parser.parse(tokens);
            if (!expr) {
                TS_ASSERT_EQ(*evaluated, result);
            }
        }
    }

    void runEvalTest(std::string_view programText, const Number& result) {
        runEvalTest(programText, Datum{Atom{result}});
    }

  public:
    void run() {
        runEvalTest("(+ 45 23)", 68_N);
        runEvalTest("(+ 1 2 3 4 )", 10_N);
        runEvalTest("((lambda (x) x) 4)", 4_N);
        runEvalTest("((lambda (x) (+ x x)) 4)", 8_N);
        runEvalTest("((lambda (x y) (+ x y)) 4 5)", 9_N);
        runEvalTest("((lambda (x y) (x y)) (lambda (z) (+ z z z)) 5)", 15_N);

        runEvalTest("(if #f 4 5)", 5_N);
        runEvalTest("(if 1 4 5)", 4_N);

        runEvalTest("(+ 3.4 4.5)", 7.9_N);
        runEvalTest("(* 4 (+ 2 3))", 20_N);

        runEvalTest("(define (f x) (- x 3))\n"
                    "(f 75)",
                    72_N);
        runEvalTest("(car (quote (1 2)))", 1_N);
        runEvalTest("(car '(1 2))", 1_N);
        runEvalTest("(car (cdr '(1 2 3)))", 2_N);
        runEvalTest("(if (eq? (+ 3 4) 7) 5 4)", 5_N);

        runEvalTest("(null? '())", Datum{Atom{true}});
        runEvalTest("(null? (cdr '(1)))", Datum{Atom{true}});
        runEvalTest("(null? '(1))", Datum{Atom{false}});
        runEvalTest("(car (cdr (list 1 2 3)))", 2_N);

        runEvalTest("(or)", Datum{Atom{false}});
        runEvalTest("(and)", Datum{Atom{true}});
        runEvalTest("(or #f 5)", 5_N);
        runEvalTest("(and 5 3 #f)", Datum{Atom{false}});
        runEvalTest("(and 1 2 3)", 3_N);
        runEvalTest("(or #f (eq? 1 0) #f)", Datum{Atom{false}});
        // test short circuiting
        runEvalTest("(define (f) (f f))\n"
                    "(and #f (f))", Datum{Atom{false}});
        runEvalTest("(define (f) (f f))\n"
                    "(or #t (f))", Datum{Atom{true}});

        runEvalTest("(begin (+ 1 2) (- 1 2) (* 1 2))", 2_N);
    }
};
