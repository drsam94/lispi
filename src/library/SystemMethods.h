// (c) Sam Donow 2017
#pragma once
#include "data/Data.h"

class SystemMethods {
    static BuiltInFunc add;
    static BuiltInFunc sub;
    static BuiltInFunc mul;
    static BuiltInFunc div;

    // TODO: pass params by const reference
    static Number quotient(Number, Number);
    static Number remainder(Number, Number);
    static Number modulo(Number, Number);

    static Number inc(Number);
    static Number dec(Number);
    static Number abs(Number);

    static Number stringLength(std::string);
    static char stringRef(std::string, Number idx);
    static bool stringEq(std::string, std::string);
    static bool stringCIEq(std::string, std::string);

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

