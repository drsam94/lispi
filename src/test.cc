// (c) Sam Donow 2017-2018
#include "test/TestSuite.h"
#include "test/BigIntTest.h"
#include "test/EvalTest.h"
#include "test/NumberTest.h"
#include "test/LexerTest.h"
#include "test/SmallVectorTest.h"

#include <string>
#include <unistd.h>
#include <unordered_map>
#include <functional>
int main(int argc, char **argv) {
    std::unordered_map<std::string, std::function<void()>> testers;
    testers.emplace("lex", []()
    {
        LexerTester tester;
        tester.run();
    });
    testers.emplace("num", []()
    {
        NumberTester tester;
        tester.run();
    });
    testers.emplace("eval", []()
    {
        EvalTester tester;
        tester.run();
    });
    testers.emplace("big", []()
    {
        BigIntTester tester;
        tester.run();
    });
    testers.emplace("vec", []()
    {
        SmallVectorTester tester;
        tester.run();
    });

    int opt;
    auto it = testers.end();
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
            case 't':
                it = testers.find(optarg);
                break;
        }
    }
    if (it == testers.end()) {
        for (const auto& entry : testers) {
            entry.second();
        }
    } else {
        it->second();
    }
    TS_SUMMARIZE();
}
