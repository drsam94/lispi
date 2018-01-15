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
        TS_ASSERT_EQ(BigInt{3} - BigInt{3}, BigInt{0});
        TS_ASSERT(BigInt{-3} < BigInt{4});
        TS_ASSERT_REP(BigInt{2}, "2");
        TS_ASSERT_REP(BigInt{-123}, "-123");
        TS_ASSERT_REP(BigInt{std::numeric_limits<int32_t>::max()} + BigInt{1},
                    "2147483648");
        TS_ASSERT_EQ(BigInt{28} / BigInt{3}, BigInt{9});
        TS_ASSERT_EQ(BigInt{4} / BigInt{-2}, BigInt{-2});
        TS_ASSERT_EQ((BigInt{std::numeric_limits<int32_t>::max()} +
                     BigInt{std::numeric_limits<int32_t>::max()}) / BigInt{2},
                     BigInt{std::numeric_limits<int32_t>::max()});

    }
};
