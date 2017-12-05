#!/bin/sh
# simple script to run all tests in all compilers
cd src
make clean
make sanitize
../bin/sanitize.g++/test_runner
make clean
make CC=clang++ sanitize
../bin/sanitize.clang++/test_runner
make clean
cd ..
