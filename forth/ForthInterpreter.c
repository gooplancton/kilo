#include "ForthInterpreter.h"

SymbolsTableEntry *SymbolsTableEntry__new_literal(char *key, ForthObject *val)
{
    size_t key_len = strlen(key) + 1;
    size_t alloc_size = sizeof(SymbolsTableEntry) + key_len;

    SymbolsTableEntry *self = malloc(alloc_size);
    if (!self)
        abort();

    self->type = ObjLiteral;
    self->literal = ForthObject__rc_clone(val);
    memcpy(self->key, key, key_len);

    return self;
}

SymbolsTableEntry *SymbolsTableEntry__new_function(char *key, void (*function)(ForthInterpreter *interpreter))
{
    size_t key_len = strlen(key) + 1;
    size_t alloc_size = sizeof(SymbolsTableEntry) + key_len;

    SymbolsTableEntry *self = malloc(alloc_size);
    if (!self)
        abort();

    self->type = Function;
    self->function = function;
    memcpy(self->key, key, key_len);

    return self;
}

void SymbolsTableEntry__drop(SymbolsTableEntry *self)
{
    ForthObject__drop(self->literal);
    free(self);
}

SymbolsTable *SymbolsTable__new(void)
{
    SymbolsTable *self = malloc(sizeof(*self));
    if (!self)
        abort();

    self->cap = SYMBOLS_TABLE_DEFAULT_CAPACITY;
    self->len = 0;
    self->entries = malloc(sizeof(self->entries) * self->cap);
    if (self->entries == NULL)
        abort();

    return self;
}

void SymbolsTable__drop(SymbolsTable *self)
{
    for (size_t i = 0; i < self->len; i++)
        SymbolsTableEntry__drop(self->entries[i]);

    free(self->entries);
    free(self);
}

void SymbolsTable__reserve_slot(SymbolsTable *self)
{
    if (self->cap == self->len)
    {
        self->cap *= 2;
        self->entries = realloc(self->entries, sizeof(self->entries) * self->cap);
        if (!self->entries)
            abort();
    }
}

void SymbolsTable__add_literal(SymbolsTable *self, char *key, ForthObject *literal)
{
    SymbolsTableEntry *existing_entry = SymbolsTable__get(self, key);
    if (existing_entry)
    {
        ForthObject__drop(existing_entry->literal);
        existing_entry->type = ObjLiteral;
        existing_entry->literal = ForthObject__rc_clone(literal);
        return;
    }

    SymbolsTable__reserve_slot(self);

    self->entries[self->len] = SymbolsTableEntry__new_literal(key, literal);
    self->len += 1;
}

void SymbolsTable__add_function(SymbolsTable *self, char *key, void (*function)(ForthInterpreter *interpreter))
{
    SymbolsTableEntry *existing_entry = SymbolsTable__get(self, key);
    if (existing_entry)
    {
        existing_entry->type = Function;
        existing_entry->function = function;
        return;
    }

    SymbolsTable__reserve_slot(self);

    self->entries[self->len] = SymbolsTableEntry__new_function(key, function);
    self->len += 1;
}

bool SymbolsTable__has(SymbolsTable *self, char *key)
{
    for (size_t i = 0; i < self->len; i++)
    {
        if (strcmp(self->entries[i]->key, key) == 0)
            return true;
    }

    return false;
}

SymbolsTableEntry *SymbolsTable__get(SymbolsTable *self, char *key)
{
    for (size_t i = 0; i < self->len; i++)
    {
        if (strcmp(self->entries[i]->key, key) == 0)
            return self->entries[i];
    }

    return NULL;
}

void SymbolsTable__remove(SymbolsTable *self, char *key)
{
    for (size_t i = 0; i < self->len; i++)
    {
        if (strcmp(self->entries[i]->key, key) == 0)
        {
            SymbolsTableEntry__drop(self->entries[i]);
            return;
        }
    }
}

ForthInterpreter *ForthInterpreter__new(void)
{
    ForthInterpreter *self = malloc(sizeof(*self));
    self->stack = ForthObject__new_list();
    self->symbols = SymbolsTable__new();

    return self;
}

void ForthInterpreter__drop(ForthInterpreter *self)
{
    ForthObject__drop(self->stack);
    SymbolsTable__drop(self->symbols);
    free(self);
}

void ForthInterpreter__register_literal(ForthInterpreter *self, char *key, ForthObject *literal)
{
    SymbolsTable__add_literal(self->symbols, key, literal);
}

void ForthInterpreter__register_function(ForthInterpreter *self, char *key, void (*function)(ForthInterpreter *interpreter))
{
    SymbolsTable__add_function(self->symbols, key, function);
}

void ForthInterpreter__eval(ForthInterpreter *self, ForthObject *expr)
{
    assert(expr->type == List);

    for (size_t i = 0; i < expr->list.len; i++)
    {
        if (expr->list.data[i]->type == Symbol)
        {
            char *key = expr->list.data[i]->string.chars;
            SymbolsTableEntry *entry = SymbolsTable__get(self->symbols, key);
            if (!entry)
            {
                printf("Runtime error: Unknown Symbol");
                exit(1);
            }

            switch (entry->type)
            {
            case ObjLiteral:
                ForthObject__list_push_copy(self->stack, entry->literal);
                break;
            case Function:
                entry->function(self);
                break;
            }
        }
        else
        {
            ForthObject__list_push_move(self->stack, expr->list.data[i]);
        }
    }
}

ForthObject *ForthInterpreter__pop_arg(ForthInterpreter *self, ForthObjectType type)
{
    ForthObject *arg = ForthObject__list_pop(self->stack);
    if (!arg)
    {
        printf("Runtime error: Stack underflow");
        exit(1);
    }

    if (arg->type != type)
    {
        printf("Runtime error: Type mismatch");
        exit(1);
    }

    return arg;
}
