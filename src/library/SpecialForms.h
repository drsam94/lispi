// (c) Sam Donow 2017
#pragma once
#include "data/Data.h"

// Includes definitions of all the Scheme special forms; these by their
// nature must be implemtned in C++, they fundamentally cannot be implemented
// within the language
// TODO: add helpers for things like Unaryfunction and such
class SpecialForms {
    // All special forms documented here; will be implemented as I get to them
    //static BuiltInFunc accessImpl;
    static BuiltInFunc caseImpl;
    //static BuiltInFunc declareImpl;
    //static BuiltInFunc defineIntegrableImpl;
    //static BuiltInFunc delayImpl;
    //static BuiltInFunc fluidLetImpl;
    //static BuiltInFunc letImpl;
    //static BuiltInFunc letSyntexImpl;
    //static BuiltInFunc localDeclarImpl;
    static BuiltInFunc orImpl;
    //static BuiltInFunc rscMacroTransformerImpl;
    //static BuiltInFunc syntaxRulesImpl;
    static BuiltInFunc andImpl;
    static BuiltInFunc condImpl;
    static BuiltInFunc defineImpl;
    //static BuiltInFunc defineStructureImpl;
    //static BuiltInFunc doImpl;
    static BuiltInFunc ifImpl;
    //static BuiltInFunc letSImpl;
    //static BuiltInFunc letrecImpl;
    static BuiltInFunc namedLambdaImpl;
    //static BuiltinFunc quasiquoteImpl;
    //static BuiltInFunc scMacroTransformer;
    //static BuiltInFunc theEnvironmentImpl;
    static BuiltInFunc beginImpl;
    //static BuiltInFunc consStreamImpl;
    //static BuiltInFunc consStreamImpl;
    //static BuiltInFunc defineSyntaxImpl;
    //static BuiltInFunc erMacroTransformerImpl;
    static BuiltInFunc lambdaImpl;
    //static BuiltInFunc letSSyntax;
    //static BuiltInFunc letrecSyntaxImpl;
    //static BuiltInFunc nonHygienicMacroTransformerImpl;
    static BuiltInFunc quoteImpl;
    //static BuiltInFunc setBangImpl;

  public:
    static void insertIntoScope(SymbolTable& st);
};
