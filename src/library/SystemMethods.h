// (c) Sam Donow 2017
#pragma once
#include "data/Data.h"

class SystemMethods {
    static BuiltInFunc add;
    static BuiltInFunc sub;
    static BuiltInFunc mul;
    static BuiltInFunc exactQ;
    static BuiltInFunc inexactQ;
    static BuiltInFunc eq;
    static BuiltInFunc lt;
    static BuiltInFunc gt;
    static BuiltInFunc le;
    static BuiltInFunc ge;
    static BuiltInFunc zeroQ;
    static BuiltInFunc positiveQ;
    static BuiltInFunc negativeQ;
    static BuiltInFunc oddQ;
    static BuiltInFunc evenQ;

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

