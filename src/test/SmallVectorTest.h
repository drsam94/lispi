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

        SmallVector<int, 3> svCopy{sv};
        TS_ASSERT_EQ(svCopy.size(), 10);
        TS_ASSERT_EQ(svCopy[0], 1);
        TS_ASSERT_EQ(svCopy.back(), 10);

        SmallVector<int, 3> svc2;
        svc2 = svCopy;
        TS_ASSERT_EQ(svc2.size(), 10);
        TS_ASSERT_EQ(svc2[1], 2);
        TS_ASSERT_EQ(svc2.back(), 10);

        svCopy.erase(std::remove_if(svCopy.begin(), svCopy.end(),
            [](int x) { return x % 2 == 0; }), svCopy.end());
        TS_ASSERT_EQ(svCopy.size(), 5);
        TS_ASSERT_EQ(svCopy[0], 1);
        TS_ASSERT_EQ(svCopy.back(), 9);

        SmallVector<int, 3> svMoved{std::move(svc2)};
        TS_ASSERT_EQ(svMoved.size(), 10);
        TS_ASSERT_EQ(svMoved[0], 1);
        TS_ASSERT_EQ(svMoved.back(), 10);

        svMoved.clear();
        TS_ASSERT_EQ(svMoved.size(), 0);
        svMoved.push_back(2);
        TS_ASSERT_EQ(svMoved.size(), 1);
        TS_ASSERT_EQ(svMoved[0], 2);
        svMoved.push_back(3);
        svMoved.push_back(4);
        svMoved.pop_back();
        TS_ASSERT_EQ(svMoved.back(), 3);
        TS_ASSERT_EQ(svMoved.size(), 2);

        SmallVector<int, 3> test;
        test.push_back(2);
        SmallVector<int, 3> test2 = std::move(test);
        TS_ASSERT_EQ(test2.size(), 1);
        TS_ASSERT_EQ(test2[0], 2);
    }
};
