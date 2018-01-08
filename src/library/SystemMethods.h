// (c) Sam Donow 2017
#pragma once
#include "data/Data.h"

class SystemMethods {
    static BuiltInFunc add;
    static BuiltInFunc sub;
    static BuiltInFunc mul;
    static BuiltInFunc div;

    static Datum quotient(Number, Number);
    static Datum remainder(Number, Number);
    static Datum modulo(Number, Number);

    static Datum inc(Number);
    static Datum dec(Number);
    static Datum abs(Number);

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

