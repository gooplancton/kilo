#ifndef FORTH_INTERPRETER_H
#define FORTH_INTERPRETER_H

#include "ForthParser.h"

#define SYMBOLS_TABLE_DEFAULT_CAPACITY 20

typedef enum SymbolsTableEntryType
{
    ObjLiteral,
    Function,
    ListClosure,
} SymbolsTableEntryType;

typedef enum ForthEvalResult {
    Ok,
    ArityError,
    TypeError,
    MathError,
    ParsingError,
    IndexError,
    UnknownSymbolError,
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

SymbolsTableEntry *SymbolsTableEntry__new_literal(char *key, ForthObject *literal);
SymbolsTableEntry *SymbolsTableEntry__new_closure(char *key, ForthObject *closure);
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

void SymbolsTable__add_literal(SymbolsTable *self, char *key, ForthObject *val);
void SymbolsTable__add_function(SymbolsTable *self, char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter));
void SymbolsTable__add_closure(SymbolsTable *self, char *key, ForthObject *closure);
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

void ForthInterpreter__register_literal(ForthInterpreter *self, char *key, ForthObject *literal);
void ForthInterpreter__register_closure(ForthInterpreter *self, char *key, ForthObject *closure);
void ForthInterpreter__register_function(ForthInterpreter *self, char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter));

ForthObject *ForthInterpreter__pop_arg(ForthInterpreter *self);
ForthObject *ForthInterpreter__pop_arg_typed(ForthInterpreter *self, ForthObjectType type);

ForthEvalResult ForthInterpreter__eval(ForthInterpreter *self, ForthObject *expr);
ForthEvalResult ForthInterpreter__parse_eval(ForthInterpreter *self, char *text);
ForthEvalResult ForthInterpreter__run_file(ForthInterpreter *self, FILE *file);

#endif
