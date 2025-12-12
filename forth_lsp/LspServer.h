#ifndef LSP_SERVER_H
#define LSP_SERVER_H

#include "ForthInterpreter.h"
#include "cJSON.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct ForthLspServer ForthLspServer;

typedef struct SymbolDefinition {
    char *symbol_name;
    char *documentation;
    uint8_t *type_in; 
    uint8_t *type_out;
} SymbolDefinition;

typedef struct SymbolDefinitionsTable {
    size_t len;
    size_t cap;
    SymbolDefinition *entries;
} SymbolDefinitionsTable;

typedef struct HandlerTableEntry {
    cJSON *(*handler)(ForthLspServer* server, cJSON *params);
    char *method_name;
} HandlerTableEntry;

typedef struct HandlerTable {
    size_t len;
    HandlerTableEntry *entries;
} HandlerTable;

typedef struct ForthLspServer {
    ForthInterpreter *interpreter;
    HandlerTable *handlers;
    SymbolDefinitionsTable *definitions;
    ForthEvalError *errors;
    bool requested_shutdown;
} ForthLspServer;


ForthLspServer *ForthLspServer__new(void);
void ForthLspServer__drop(ForthLspServer *self);

void ForthLspServer__define_symbol(ForthLspServer *self, SymbolsTableEntry *entry);
SymbolDefinition *ForthLspServer__get_symbol_definition(ForthLspServer *self, const char *symbol_name);
void ForthLspServer__undefine_symbol(ForthLspServer *self, const char *symbol_name);

void ForthLspServer__handle_message(ForthLspServer *self, cJSON *json);

#endif // LSP_SERVER_H