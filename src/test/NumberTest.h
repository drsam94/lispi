#include "test/TestSuite.h"
#include "Number.h"
#pragma once

struct NumberTester {

    void run() {
        TS_ASSERT_EQ(1_N * 2_N, 2_N);
        TS_ASSERT_EQ(1_N + 2_N, 3_N);
        TS_ASSERT_EQ(1_N + 2.5_N, 3.5_N);
        TS_ASSERT_EQ(1_N, 1_N);
        TS_ASSERT_NEQ(1_N, 1.1_N);
        TS_ASSERT_NEQ(0.9_N, 1.1_N);
        TS_ASSERT_EQ(5_N - 3_N, 2_N);
        TS_ASSERT_EQ(-5_N * 20_N, -100_N);
    }
};
