#!/bin/sh
# simple script to run all tests in all compilers
cd src
make clean
make all
../bin/test_runner
make clean
make CC=clang++
../bin/test_runner
make clean
cd ..
