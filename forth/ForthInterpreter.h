#ifndef FORTH_INTERPRETER_H
#define FORTH_INTERPRETER_H

#include "ForthObject.h"

#define SYMBOLS_TABLE_DEFAULT_CAPACITY 20

typedef enum SymbolsTableEntryType
{
    ObjLiteral,
    Function,
} SymbolsTableEntryType;

// NOTE: forward-declaration for FunctionEntry type
typedef struct ForthInterpreter ForthInterpreter;

typedef struct SymbolsTableEntry
{
    SymbolsTableEntryType type;
    union
    {
        ForthObject *literal;
        void (*function)(ForthInterpreter *interpreter);
    };
    char key[];
} SymbolsTableEntry;

SymbolsTableEntry *SymbolsTableEntry__new_literal(char *key, ForthObject *literal);
SymbolsTableEntry *SymbolsTableEntry__new_function(char *key, void (*function)(ForthInterpreter *interpreter));
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
void SymbolsTable__add_function(SymbolsTable *self, char *key, void (*function)(ForthInterpreter *interpreter));
bool SymbolsTable__has(SymbolsTable *self, char *key);
SymbolsTableEntry *SymbolsTable__get(SymbolsTable *self, char *key);
void SymbolsTable__remove(SymbolsTable *self, char *key);

typedef struct ForthInterpreter
{
    ForthObject *stack;
    SymbolsTable *symbols;
} ForthInterpreter;

ForthInterpreter *ForthInterpreter__new(void);
void ForthInterpreter__drop(ForthInterpreter *self);

void ForthInterpreter__register_literal(ForthInterpreter *self, char *key, ForthObject *val);
void ForthInterpreter__register_function(ForthInterpreter *self, char *key, void (*function)(ForthInterpreter *interpreter));

void ForthInterpreter__eval(ForthInterpreter *self, ForthObject *expr);
ForthObject *ForthInterpreter__pop_arg(ForthInterpreter *self, ForthObjectType type);

#endif
