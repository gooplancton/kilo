#ifndef FORTH_BUILTINS_H
#define FORTH_BUILTINS_H

#include "ForthInterpreter.h"

// Math
void builtin_add(ForthInterpreter *in);

// Print
void builtin_print(ForthInterpreter *in);
void builtin_print_symbols(ForthInterpreter *in);

// Eval
void builtin_eval(ForthInterpreter *in);

// Define
void builtin_define(ForthInterpreter *in);

// Initialization
void ForthInterpreter__load_builtins(ForthInterpreter *in);

#endif
