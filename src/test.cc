// (c) Sam Donow 2017-2018
#include "test/TestSuite.h"
#include "test/BigIntTest.h"
#include "test/EvalTest.h"
#include "test/LexerTest.h"
#include "test/NumberTest.h"
#include "test/RationalTest.h"
#include "test/SmallVectorTest.h"

#include <algorithm>
#include <string>
#include <unistd.h>
#include <unordered_map>
int main(int argc, char **argv) {
    int opt;
    auto it = TestSuite::testers->end();
    while ((opt = getopt(argc, argv, "lt:")) != -1) {
        switch (opt) {
            case 't':
                it = TestSuite::testers->find(optarg);
                if (it == TestSuite::testers->end()) {
                    std::cout << "No tester named " << optarg << "\n";
                    return 1;
                }
                break;
            case 'l':
                for (const auto& entry : *TestSuite::testers) {
                    std::cout << entry.first << "\n";
                }
                return 0;
        }
    }
    if (it == TestSuite::testers->end()) {
        for (const auto& entry : *TestSuite::testers) {
            entry.second();
        }
    } else {
        it->second();
    }
    TS_SUMMARIZE();
}
