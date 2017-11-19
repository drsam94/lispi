#!/bin/sh
# simple script to run all tests in all compilers
cd src
make clean
make sanitize
../bin/test_runner.sanitize
make clean
make CC=clang++ sanitize
../bin/test_runner.sanitize
make clean
cd ..
