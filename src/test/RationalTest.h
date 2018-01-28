// (c) Sam Donow 2018
#include "test/TestSuite.h"
#include "data/Rational.h"
#pragma once

struct RationalTester : Tester<RationalTester> {
    void run() {
        initialize();
        using R = Rational<int>;
        TS_ASSERT_EQ(R(2, 1).numerator(), 2);
        TS_ASSERT_EQ(R(2, 1).denominator(), 1);
        TS_ASSERT_EQ(R(1, 3) * R(3, 1), R(1));
        TS_ASSERT_EQ(R(1, 3) + R(1, 4), R(7, 12));
        TS_ASSERT_EQ(R(1, 2), R(70, 140));
        TS_ASSERT_EQ(R(5, 1) - R(10, 2), R(0, 23));
        TS_ASSERT_EQ(R(-1, 2) * R(1, -2), R(1, 4));
        TS_ASSERT_EQ(R(23, -57), R(-23, 57));
        TS_ASSERT_EQ(R(1, 4) / R(1, 3), R(3, 4));
        TS_ASSERT_REP(R(23, 46), "1/2");
        TS_ASSERT_REP(R(-7, 4), "-7/4");
    }
};

