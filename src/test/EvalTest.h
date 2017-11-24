// (c) Sam Donow 2017
#pragma once

#include "Lexer.h"
#include "Parser.h"
#include "Evaluator.h"

class EvalTester {
    Datum eval(std::string_view programText) {
        Lexer lex;
        Parser parser;
        Evaluator ev;
        std::vector<Token> tokens = lex.getTokens(programText);
        auto expr = parser.parse(tokens);
        if (!expr) {
            return {};
        }
        std::optional<Datum> evaluated;
        while (expr) {
            evaluated = ev.eval(*expr);
            if (!evaluated) {
                return {};
            }
            expr = parser.parse(tokens);
        }
        return *evaluated;
    }

    Number evNum(std::string_view programText) {
        Datum val = eval(programText);
        std::optional<Number> num = val.getAtomicValue<Number>();
        if (!num) {
            TS_ASSERT(false);
            return {};
        }
        return *num;
    }

  public:
    void run() {
        TS_ASSERT_EQ(evNum("(+ 45 23)"), 68_N);
        TS_ASSERT_EQ(evNum("(+ 1 2 3 4 )"), 10_N);
        TS_ASSERT_EQ(evNum("((lambda (x) x) 4)"), 4_N);
        TS_ASSERT_EQ(evNum("((lambda (x) (+ x x)) 4)"), 8_N);
        TS_ASSERT_EQ(evNum("((lambda (x y) (+ x y)) 4 5)"), 9_N);
        TS_ASSERT_EQ(evNum("((lambda (x y) (x y)) (lambda (z) (+ z z z)) 5)"), 15_N);

        TS_ASSERT_EQ(evNum("(if #f 4 5)"), 5_N);
        TS_ASSERT_EQ(evNum("(if 1 4 5)"), 4_N);

        TS_ASSERT_EQ(evNum("(+ 3.4 4.5)"), 7.9_N);
        TS_ASSERT_EQ(evNum("(* 4 (+ 2 3))"), 20_N);

        TS_ASSERT_EQ(evNum("(define (f x) (- x 3))\n"
                    "(f 75)"),
                    72_N);
        TS_ASSERT_EQ(evNum("(car (quote (1 2)))"), 1_N);
        TS_ASSERT_EQ(evNum("(car '(1 2))"), 1_N);
        TS_ASSERT_EQ(evNum("(car (cdr '(1 2 3)))"), 2_N);
        TS_ASSERT_EQ(evNum("(if (eq? (+ 3 4) 7) 5 4)"), 5_N);

        TS_ASSERT_EQ(eval("(null? '())"), Datum{Atom{true}});
        TS_ASSERT_EQ(eval("(null? (cdr '(1)))"), Datum{Atom{true}});
        TS_ASSERT_EQ(eval("(null? '(1))"), Datum{Atom{false}});
        TS_ASSERT_EQ(evNum("(car (cdr (list 1 2 3)))"), 2_N);

        TS_ASSERT_EQ(eval("(or)"), Datum{Atom{false}});
        TS_ASSERT_EQ(eval("(and)"), Datum{Atom{true}});
        TS_ASSERT_EQ(evNum("(or #f 5)"), 5_N);
        TS_ASSERT_EQ(eval("(and 5 3 #f)"), Datum{Atom{false}});
        TS_ASSERT_EQ(evNum("(and 1 2 3)"), 3_N);
        TS_ASSERT_EQ(eval("(or #f (eq? 1 0) #f)"), Datum{Atom{false}});
        // test short circuiting
        TS_ASSERT_EQ(eval("(define (f) (f f))\n"
                    "(and #f (f))"), Datum{Atom{false}});
        TS_ASSERT_EQ(eval("(define (f) (f f))\n"
                    "(or #t (f))"), Datum{Atom{true}});

        TS_ASSERT_EQ(evNum("(begin (+ 1 2) (- 1 2) (* 1 2))"), 2_N);
        TS_ASSERT_EQ(evNum("(*)"), 1_N);
        TS_ASSERT_EQ(evNum("(cond ((eq? 1 2) 1) "
                                 "((eq? 1 1) 2) "
                                 "((eq? 2 2) 3))"), 2_N);
        TS_ASSERT_EQ(evNum("(cond (#f 1) (else 2))"), 2_N);
    }
};
