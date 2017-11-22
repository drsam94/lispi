// (c) 2017 Sam Donow
#pragma once
#include <iostream>
#include <sstream>

// This file contains macros for writing very lightweight unit test runners
namespace TestSuite {
    inline size_t passedTests = 0;
    inline size_t failedTests = 0;
}

#define TEST_ASSERT_(x, msg)                                                   \
    do {                                                                       \
        if (!(x)) {                                                            \
            ++TestSuite::failedTests;                                          \
            std::cout << "TEST FAILED(" << __PRETTY_FUNCTION__ << "("          \
                 << __FILE__ << ":" << __LINE__ << ") " << msg << std::endl;   \
        } else {                                                               \
            ++TestSuite::passedTests;                                          \
        }                                                                      \
    } while (0);

#define TS_ASSERT(x) TEST_ASSERT_((x), #x " failed")

// Prints a helpful message so long as the input types support operator<<
#define TS_ASSERT_EQ(x, y)                                                     \
    do {                                                                       \
        const auto _tmp_x = (x);                                               \
        const auto _tmp_y = (y);                                               \
        std::stringstream _stream;                                             \
        const bool _result = _tmp_x == _tmp_y;                                 \
        if (_result) {                                                         \
            ++TestSuite::passedTests;                                          \
            break;                                                             \
        }                                                                      \
        _stream << #x " != " #y << " (" << _tmp_x << " != " << _tmp_y << ")";  \
        TEST_ASSERT_(false, _stream.str())                                     \
    } while (0);

#define TS_ASSERT_NEQ(x, y)                                                    \
    do {                                                                       \
        const auto _tmp_x = (x);                                               \
        const auto _tmp_y = (y);                                               \
        std::stringstream _stream;                                             \
        const bool _result = _tmp_x != _tmp_y;                                 \
        if (_result) {                                                         \
            ++TestSuite::passedTests;                                          \
            break;                                                             \
        }                                                                      \
        _stream << #x " == " #y << " (" << _tmp_x << " == " << _tmp_y << ")";  \
        TEST_ASSERT_(false, _stream.str())                                     \
    } while (0);

#define TS_SUMMARIZE()                                                         \
    std::cout << "TESTS COMPLETE. PASSED (" << TestSuite::passedTests          \
              << ") FAILED: " << TestSuite::failedTests << std::endl;          \
    return static_cast<int>(TestSuite::failedTests);
