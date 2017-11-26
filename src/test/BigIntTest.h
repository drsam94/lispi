// (c) Sam Donow 2017;
#pragma once
#include "test/TestSuite.h"
#include "data/BigInt.h"

struct BigIntTester {

    void run() {
        TS_ASSERT_EQ(BigInt{1} * BigInt{2}, BigInt{2});
        TS_ASSERT_EQ(BigInt{5} + BigInt{4}, BigInt{9});
        TS_ASSERT_EQ(BigInt{5} - BigInt{4}, BigInt{1});
        TS_ASSERT_EQ(BigInt{5} - BigInt{6}, BigInt{-1});
        TS_ASSERT_EQ(BigInt{-3} * BigInt{2}, BigInt{-6});
        TS_ASSERT_EQ(BigInt{-3} * BigInt{-2}, BigInt{6});
    }
};
