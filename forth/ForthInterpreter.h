#ifndef FORTH_INTERPRETER_H
#define FORTH_INTERPRETER_H

#include "ForthParser.h"
#include <stdarg.h>

#define SYMBOLS_TABLE_DEFAULT_CAPACITY 20

typedef enum SymbolsTableEntryType
{
    Object,
    Function,
} SymbolsTableEntryType;

typedef enum ForthEvalResult {
    Ok,
    ArityError,
    TypeError,
    MathError,
    ParsingError,
    IndexError,
    UnknownSymbolError,
    FileNotFoundError
} ForthEvalResult;

// NOTE: forward-declaration for FunctionEntry type
typedef struct ForthInterpreter ForthInterpreter;

typedef struct SymbolsTableEntry
{
    SymbolsTableEntryType type;
    union
    {
        ForthObject *obj;
        ForthEvalResult (*function)(ForthInterpreter *interpreter);
    };
    char key[];
} SymbolsTableEntry;

SymbolsTableEntry *SymbolsTableEntry__new_object(char *key, ForthObject *ob);
SymbolsTableEntry *SymbolsTableEntry__new_function(char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter));
void SymbolsTableEntry__drop(SymbolsTableEntry *self);

typedef struct SymbolsTable
{
    SymbolsTableEntry **entries;
    size_t len;
    size_t cap;
} SymbolsTable;

SymbolsTable *SymbolsTable__new(void);
void SymbolsTable__drop(SymbolsTable *self);

void SymbolsTable__add_object(SymbolsTable *self, char *key, ForthObject *word);
void SymbolsTable__add_function(SymbolsTable *self, char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter));
bool SymbolsTable__has(SymbolsTable *self, char *key);
SymbolsTableEntry *SymbolsTable__get(SymbolsTable *self, char *key);
void SymbolsTable__remove(SymbolsTable *self, char *key);

typedef struct ForthInterpreter
{
    ForthObject *stack;
    SymbolsTable *symbols;
    ForthParser *parser;
} ForthInterpreter;

ForthInterpreter *ForthInterpreter__new(void);
void ForthInterpreter__drop(ForthInterpreter *self);

void ForthInterpreter__register_object(ForthInterpreter *self, char *key, ForthObject *obj);
void ForthInterpreter__register_function(ForthInterpreter *self, char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter));

/*
This function expects a list of n (obj pointer, type) as its variadic arguments
e.g.:
    ForhtObject *n1, *arg = NULL;
    ForthEval args_result = ForthInterpreter__pop__args(in, 2, n1, Number, n2, Any);
    if (args_result != Ok)
        return args_result;

If the result of calling this function is anything other than Ok, the function guarantees
that ALL the objects passed to it are dropped.
*/
ForthEvalResult ForthInterpreter__pop_args(ForthInterpreter *self, size_t n, ...);

ForthEvalResult ForthInterpreter__eval(ForthInterpreter *self, ForthObject *expr);
ForthEvalResult ForthInterpreter__eval_every(ForthInterpreter *self, ForthObject *expr);
ForthEvalResult ForthInterpreter__parse_eval(ForthInterpreter *self, char *text);
ForthEvalResult ForthInterpreter__run_file(ForthInterpreter *self, FILE *file);

#endif
