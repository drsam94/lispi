// (c) Sam Donow 2017
#include "test/TestSuite.h"
#include "test/EvalTest.h"
#include "test/NumberTest.h"
#include "test/LexerTest.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    {
        LexerTester tester;
        tester.run();
    }
    {
        NumberTester tester;
        tester.run();
    }
    {
        EvalTester tester;
        tester.run();
    }
    TS_SUMMARIZE();
}
