// (c) Sam Donow 2017
#pragma once
#include "Data.h"

// Includes definitions of all the Scheme special forms; these by their
// nature must be implemtned in C++, they fundamentally cannot be implemented
// within the language
struct SpecialForms {
    static BuiltInFunc lambdaImpl;
    static BuiltInFunc ifImpl;
    static BuiltInFunc defineImpl;
    static BuiltInFunc quoteImpl;

    static void insertIntoScope(SymbolTable& st);
};
