// (c) Sam Donow 2017
#pragma once
#include "data/Data.h"

class SystemMethods {
    static BuiltInFunc add;
    static BuiltInFunc sub;
    static BuiltInFunc mul;
    static BuiltInFunc div;

    static BuiltInFunc car;
    static BuiltInFunc cdr;
    static BuiltInFunc cons;

    static BuiltInFunc eqQ;
    static BuiltInFunc nullQ;
    static BuiltInFunc list;
    static BuiltInFunc display;

  public:

    static void insertIntoScope(SymbolTable& st);
};

