// (c) Sam Donow 2018
#pragma once

#include "util/SmallVector.h"

#include "test/TestSuite.h"

class SmallVectorTester {
  public:

    void run() {
        SmallVector<int, 3> sv;
        TS_ASSERT(sv.empty());
        TS_ASSERT_EQ(sv.size(), 0);
        sv.push_back(1);
        TS_ASSERT_EQ(sv.size(), 1);
        TS_ASSERT_EQ(sv[0], 1);
        TS_ASSERT_EQ(sv.at(0), 1);
        const auto &svc = sv;
        TS_ASSERT_EQ(sv[0], 1);
        TS_ASSERT_EQ(sv.at(0), 1);

        sv.emplace_back(2);
        TS_ASSERT_EQ(sv.size(), 2);
        sv.push_back(3);
        TS_ASSERT_EQ(sv.size(), 3);
        TS_ASSERT_EQ(sv[1], 2);
        TS_ASSERT_EQ(sv[2], 3);
        sv.insert(sv.end(), {4,5,6,7,8,9});
        TS_ASSERT_EQ(sv.size(), 9);
        TS_ASSERT_EQ(sv.front(), 1);
        TS_ASSERT_EQ(*sv.begin(), 1);
        TS_ASSERT_EQ(sv.back(), 9);
        auto &back = sv.emplace_back(10);
        TS_ASSERT_EQ(&back, &sv[9]);

        int sum = 0;
        for (int x : sv) {
            sum += x;
        }
        TS_ASSERT_EQ(sum, 55);
        for (const int& x : svc) {
            sum += x;
        }
        TS_ASSERT_EQ(sum, 110);
    }
};
