#include <string>
#include <iostream>
#include "Parser.h"

int main(int argc, char **argv) {
    // TODO: make this real
    Program p{};
    std::string line;
    while (!std::cin.eof()) {
        std::getline(std::cin, line);
        p.process(line);
    }
    return 0;
}
