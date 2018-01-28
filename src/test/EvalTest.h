// (c) Sam Donow 2017-2018
#pragma once

#include "Lexer.h"
#include "Parser.h"
#include "Evaluator.h"

#include "test/TestSuite.h"

class EvalTester : public Tester<EvalTester> {
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
        initialize();
        TS_ASSERT_EQ(evNum("(+ 45 23)"), 68L);
        TS_ASSERT_EQ(evNum("(+ 1 2 3 4 )"), 10L);
        TS_ASSERT_EQ(evNum("((lambda (x) x) 4)"), 4L);
        TS_ASSERT_EQ(evNum("((lambda (x) (+ x x)) 4)"), 8L);
        TS_ASSERT_EQ(evNum("((lambda (x y) (+ x y)) 4 5)"), 9L);
        TS_ASSERT_EQ(evNum("((lambda (x y) (x y)) (lambda (z) (+ z z z)) 5)"), 15L);

        TS_ASSERT_EQ(evNum("(if #f 4 5)"), 5L);
        TS_ASSERT_EQ(evNum("(if 1 4 5)"), 4L);

        TS_ASSERT_EQ(evNum("(+ 3.4 4.5)"), 7.9);
        TS_ASSERT_EQ(evNum("(* 4 (+ 2 3))"), 20L);

        TS_ASSERT_EQ(evNum("(define (f x) (- x 3))\n"
                    "(f 75)"),
                    72L);
        TS_ASSERT_EQ(evNum("(car (quote (1 2)))"), 1L);
        TS_ASSERT_EQ(evNum("(car '(1 2))"), 1L);
        TS_ASSERT_EQ(evNum("(car (cdr '(1 2 3)))"), 2L);
        TS_ASSERT_EQ(evNum("(if (eq? (+ 3 4) 7) 5 4)"), 5L);

        TS_ASSERT_EQ(eval("(null? '())"), Datum{Atom{true}});
        TS_ASSERT_EQ(eval("(null? (cdr '(1)))"), Datum{Atom{true}});
        TS_ASSERT_EQ(eval("(null? '(1))"), Datum{Atom{false}});
        TS_ASSERT_EQ(evNum("(car (cdr (list 1 2 3)))"), 2L);

        TS_ASSERT_EQ(eval("(or)"), Datum{Atom{false}});
        TS_ASSERT_EQ(eval("(and)"), Datum{Atom{true}});
        TS_ASSERT_EQ(evNum("(or #f 5)"), 5L);
        TS_ASSERT_EQ(eval("(and 5 3 #f)"), Datum{Atom{false}});
        TS_ASSERT_EQ(evNum("(and 1 2 3)"), 3L);
        TS_ASSERT_EQ(eval("(or #f (eq? 1 0) #f)"), Datum{Atom{false}});
        // test short circuiting
        TS_ASSERT_EQ(eval("(define (f) (f f))\n"
                    "(and #f (f))"), Datum{Atom{false}});
        TS_ASSERT_EQ(eval("(define (f) (f f))\n"
                    "(or #t (f))"), Datum{Atom{true}});

        TS_ASSERT_EQ(evNum("(begin (+ 1 2) (- 1 2) (* 1 2))"), 2L);
        TS_ASSERT_EQ(evNum("(*)"), 1L);
        TS_ASSERT_EQ(evNum("(cond ((eq? 1 2) 1) "
                                 "((eq? 1 1) 2) "
                                 "((eq? 2 2) 3))"), 2L);
        TS_ASSERT_EQ(evNum("(cond (#f 1) (else 2))"), 2L);

        TS_ASSERT_REP(eval("(cons 1 '())"), "'(1)");
        TS_ASSERT_EQ(evNum("(cdr (cons 1 2))"), 2L);
        TS_ASSERT_EQ(evNum("(car (cons 1 2))"), 1L);

        TS_ASSERT_EQ(eval("(= 1 (- 2 1) (* 1 1) (+ 0.5 0.5))"), Datum{Atom{true}});
        TS_ASSERT_EQ(eval("(< 1 2 3 (+ 4 5) (* 5 5))"), Datum{Atom{true}});

        TS_ASSERT_EQ(evNum("(quotient 4 2)"), 2L);
        TS_ASSERT_EQ(evNum("(quotient 5 2)"), 2L);
        TS_ASSERT_EQ(evNum("(quotient 7 1)"), 7L);
        TS_ASSERT_EQ(evNum("(quotient 2 4)"), 0L);
        TS_ASSERT_EQ(evNum("(quotient 4 -2)"), -2L);
        TS_ASSERT_EQ(evNum("(quotient -4 2)"), -2L);
        TS_ASSERT_EQ(evNum("(remainder 4 2)"), 0L);
        TS_ASSERT_EQ(evNum("(modulo 4 2)"), 0L);
        TS_ASSERT_EQ(evNum("(remainder 7 2)"), 1L);
        TS_ASSERT_EQ(evNum("(remainder (+ 4 3) 3)"), 1L);
        TS_ASSERT_EQ(evNum("(remainder 7 -2)"), 1L);
        TS_ASSERT_EQ(evNum("(remainder -7 3)"), -1L);
        TS_ASSERT_EQ(evNum("(modulo -7 3)"), 2L);
        TS_ASSERT_EQ(evNum("(modulo 7 -3)"), -2L);

        TS_ASSERT_EQ(evNum("(/ 2)"), Rational<BigInt>(BigInt{1}, BigInt{2}));
        TS_ASSERT_EQ(eval("(= (/ 2 3) (/ 4 6))"), Datum::True());
        TS_ASSERT_EQ(eval("(= (+ (/ 2 3) 1) (/ 5 3))"), Datum::True());

        TS_ASSERT_EQ(evNum("(1+ 1)"), 2L);
        TS_ASSERT_EQ(evNum("(-1+ 2.5)"), 1.5);
        TS_ASSERT_EQ(evNum("(abs 5)"), 5L);
        TS_ASSERT_EQ(evNum("(abs -5)"), 5L);
        TS_ASSERT_EQ(evNum("(abs 5.5)"), 5.5);
        TS_ASSERT_EQ(evNum("(abs -5.5)"), 5.5);

        // Without tail call elimination, this would almost certainly smash the stack anf
        // fail
        TS_ASSERT_EQ(evNum("(define (count acc) (if (= acc 1000000) acc (count (+ acc 1))))\n"
                           "(count 0)"), 1000000L);
    }
};
