#ifndef FORTH_BUILTINS_H
#define FORTH_BUILTINS_H

#include "ForthInterpreter.h"

// Math
ForthEvalResult builtin_add(ForthInterpreter *in);
ForthEvalResult builtin_sub(ForthInterpreter *in);
ForthEvalResult builtin_mul(ForthInterpreter *in);
ForthEvalResult builtin_div(ForthInterpreter *in);
ForthEvalResult builtin_max(ForthInterpreter *in);
ForthEvalResult builtin_min(ForthInterpreter *in);
ForthEvalResult builtin_mod(ForthInterpreter *in);

// Logic
ForthEvalResult builtin_and(ForthInterpreter *in);
ForthEvalResult builtin_not(ForthInterpreter *in);
ForthEvalResult builtin_or(ForthInterpreter *in);
ForthEvalResult builtin_xor(ForthInterpreter *in);

// Comparison
ForthEvalResult builtin_eq(ForthInterpreter *in);
ForthEvalResult builtin_neq(ForthInterpreter *in);
ForthEvalResult builtin_gt(ForthInterpreter *in);
ForthEvalResult builtin_gte(ForthInterpreter *in);
ForthEvalResult builtin_lt(ForthInterpreter *in);
ForthEvalResult builtin_lte(ForthInterpreter *in);

// Strings/Lists
ForthEvalResult builtin_len(ForthInterpreter *in);
ForthEvalResult builtin_at(ForthInterpreter *in);
ForthEvalResult builtin_contains(ForthInterpreter *in);
// ForthEvalResult builtin_foreach(ForthInterpreter *in);
// ForthEvalResult builtin_append(ForthInterpreter *in);
// ForthEvalResult builtin_truncate(ForthInterpreter *in);

// Control Flow
ForthEvalResult builtin_if(ForthInterpreter *in);
ForthEvalResult builtin_ifelse(ForthInterpreter *in);
ForthEvalResult builtin_while(ForthInterpreter *in);
ForthEvalResult builtin_times(ForthInterpreter *in);

// Context
ForthEvalResult builtin_pop(ForthInterpreter *in);
ForthEvalResult builtin_dup(ForthInterpreter *in);
ForthEvalResult builtin_stack_len(ForthInterpreter *in);
ForthEvalResult builtin_stack(ForthInterpreter *in);
ForthEvalResult builtin_symbols(ForthInterpreter *in);

// Print
ForthEvalResult builtin_peek(ForthInterpreter *in);
ForthEvalResult builtin_print_stack(ForthInterpreter *in);
ForthEvalResult builtin_print_symbols(ForthInterpreter *in);

// Eval
ForthEvalResult builtin_eval(ForthInterpreter *in);
ForthEvalResult builtin_quote(ForthInterpreter *in);
ForthEvalResult builtin_unquote(ForthInterpreter *in);

// Define
ForthEvalResult builtin_define(ForthInterpreter *in);

// Initialization
void ForthInterpreter__load_builtins(ForthInterpreter *in);

#endif
