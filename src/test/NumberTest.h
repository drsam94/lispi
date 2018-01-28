// (c) Sam Donow 2017-2018
#include "test/TestSuite.h"
#include "data/Number.h"
#pragma once

struct NumberTester : Tester<NumberTester> {
#define N(x) Number{x##L}
    void run() {
        initialize();
        TS_ASSERT_EQ(N(1) * N(2), N(2));
        TS_ASSERT_EQ(N(1) + N(2), N(3));
        TS_ASSERT_EQ(N(1) + Number{2.5}, Number{3.5});
        TS_ASSERT_EQ(N(1), N(1));
        TS_ASSERT_NEQ(N(1), Number{1.1});
        TS_ASSERT_NEQ(Number{0.9}, Number{1.1});
        TS_ASSERT_EQ(N(5) - N(3), N(2));
        TS_ASSERT_EQ(N(-5) * N(20), N(-100));
    }
#undef N
};

