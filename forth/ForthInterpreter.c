#include "ForthInterpreter.h"
#include "ForthObject.h"
#include "ForthParser.h"

SymbolsTableEntry *SymbolsTableEntry__new_object(char *key, ForthObject *obj, SymbolsTableEntryType type)
{
    size_t key_len = strlen(key) + 1;
    size_t alloc_size = sizeof(SymbolsTableEntry) + key_len;

    SymbolsTableEntry *self = malloc(alloc_size);
    if (!self)
        abort();

    self->type = type;
    self->obj = ForthObject__rc_clone(obj);
    memcpy(self->key, key, key_len);

    return self;
}

SymbolsTableEntry *SymbolsTableEntry__new_literal(char *key, ForthObject *obj)
{
    return SymbolsTableEntry__new_object(key, obj, ObjLiteral);
}

SymbolsTableEntry *SymbolsTableEntry__new_closure(char *key, ForthObject *obj)
{
    return SymbolsTableEntry__new_object(key, obj, ListClosure);
}

SymbolsTableEntry *SymbolsTableEntry__new_function(char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter))
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
    switch (self->type)
    {
    case ListClosure:
    case ObjLiteral:
        ForthObject__drop(self->obj);
        break;
    case Function:
        // Function pointers don't need to be freed
        break;
    }
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

void SymbolsTable__add_object(SymbolsTable *self, char *key, ForthObject *obj, SymbolsTableEntryType type)
{
    SymbolsTableEntry *existing_entry = SymbolsTable__get(self, key);
    if (existing_entry)
    {
        if (existing_entry->type != Function)
            ForthObject__drop(existing_entry->obj);
        existing_entry->type = type;
        existing_entry->obj = ForthObject__rc_clone(obj);
        return;
    }

    SymbolsTable__reserve_slot(self);

    self->entries[self->len] = SymbolsTableEntry__new_object(key, obj, type);
    self->len += 1;
}

void SymbolsTable__add_literal(SymbolsTable *self, char *key, ForthObject *literal)
{
    SymbolsTable__add_object(self, key, literal, ObjLiteral);
}

void SymbolsTable__add_closure(SymbolsTable *self, char *key, ForthObject *closure)
{
    SymbolsTable__add_object(self, key, closure, ListClosure);
}

void SymbolsTable__add_function(SymbolsTable *self, char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter))
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
            if (i < self->len - 1)
                self->entries[i] = self->entries[self->len - 1];

            self->len -= 1;

            return;
        }
    }
}

ForthInterpreter *ForthInterpreter__new(void)
{
    ForthInterpreter *self = malloc(sizeof(*self));
    self->stack = ForthObject__new_list(DEFAULT_LIST_CAP);
    self->symbols = SymbolsTable__new();
    self->parser = ForthParser__new();

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
    assert(literal->type != Symbol || (literal->type == Symbol && literal->string.quoted));

    SymbolsTable__add_literal(self->symbols, key, literal);
}

void ForthInterpreter__register_closure(ForthInterpreter *self, char *key, ForthObject *closure)
{
    assert(closure->type == List);

    SymbolsTable__add_closure(self->symbols, key, closure);
}

void ForthInterpreter__register_function(ForthInterpreter *self, char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter))
{
    SymbolsTable__add_function(self->symbols, key, function);
}

ForthEvalResult ForthInterpreter__eval(ForthInterpreter *self, ForthObject *expr)
{
    switch (expr->type)
    {
    case List:
    {
        for (size_t i = 0; i < expr->list.len; i++)
        {
            if (expr->list.data[i]->type == Symbol && !expr->list.data[i]->string.quoted)
            {
                char *key = expr->list.data[i]->string.chars;
                SymbolsTableEntry *entry = SymbolsTable__get(self->symbols, key);
                if (!entry)
                {
                    fprintf(stderr, "UnknownSymbolError: %s\n", key);
                    return UnknownSymbolError;
                }

                switch (entry->type)
                {
                case ObjLiteral:
                    ForthObject__list_push_copy(self->stack, entry->obj);
                    break;
                case Function:
                    entry->function(self);
                    break;
                case ListClosure:
                    ForthInterpreter__eval(self, entry->obj);
                    break;
                }
            }
            else
            {
                ForthObject__list_push_copy(self->stack, expr->list.data[i]);
            }
        }

        break;
    }
    case Symbol:
        if (expr->string.quoted)
        {
            SymbolsTableEntry *entry = SymbolsTable__get(self->symbols, expr->string.chars);
            if (entry->type == ListClosure)
                return ForthInterpreter__eval(self, entry->obj);
            break;
        }
        else
        {
            fprintf(stderr, "TypeError: eval expression must either be a list or quoted symbol\n");
            return TypeError;
        }
    default:
        fprintf(stderr, "TypeError: eval expression must either be a list or quoted symbol\n");
        return TypeError;
    }

    return Ok;
}

ForthEvalResult ForthInterpreter__pop_args(ForthInterpreter *self, size_t n, ...)
{
    ForthEvalResult res = Ok;
    va_list args;
    va_start(args, n);

    bool has_found_err = false;

    ForthObject **out_ptrs[n];
    ForthObjectType types[n];

    for (size_t i = 0; i < n; i++)
    {
        out_ptrs[i] = va_arg(args, ForthObject **);
        types[i] = va_arg(args, int);
    }
    va_end(args);

    // Pop and validate
    for (size_t i = 0; i < n; i++)
    {
        ForthObject *obj = ForthObject__list_pop(self->stack);
        if (!obj)
        {
            if (!has_found_err)
            {
                fprintf(stderr, "ArityError: expected %zu arguments, got %zu\n", n, i);
                res = ArityError;
                has_found_err = true;
            }
            continue;
        }
        if (!(obj->type & types[i]) && !has_found_err)
        {
            fprintf(stderr, "TypeError: expected argument %zu to be %d, got %d\n",
                    i, types[i], obj->type);
            res = TypeError;
            has_found_err = true;
        }
        *out_ptrs[i] = obj;
    }

    if (res != Ok)
        for (size_t i = 0; i < n; i++)
            ForthObject__drop(*out_ptrs[i]);

    return res;
}

ForthEvalResult ForthInterpreter__parse_eval(ForthInterpreter *self, char *text)
{
    // Reset parser state for new input
    ForthParser__reset(self->parser, text);

    ForthEvalResult res = Ok;
    // Parse and evaluate each complete expression found
    while (1)
    {
        bool has_next = ForthParser__next_list(self->parser);
        if (!has_next)
            break;

        ForthObject *expr = ForthParser__parse_list(self->parser);
        if (!expr)
            return ParsingError;

        res = ForthInterpreter__eval(self, expr);
        ForthObject__drop(expr);
    }

    return res;
}

ForthEvalResult ForthInterpreter__run_file(ForthInterpreter *self, FILE *file)
{
    if (!file)
    {
        fprintf(stderr, "ParsingError: invalid file pointer\n");
        return ParsingError;
    }

    // Read the file in chunks and parse/eval incrementally
    char buffer[4096];
    size_t total_bytes_read = 0;
    size_t bytes_read;

    ForthEvalResult res = Ok;

    while ((bytes_read = fread(buffer + total_bytes_read, 1,
                               sizeof(buffer) - total_bytes_read - 1, file)) > 0)
    {
        total_bytes_read += bytes_read;
        buffer[total_bytes_read] = '\0';

        // Parse and evaluate what we have so far
        res = ForthInterpreter__parse_eval(self, buffer);

        // If the parser consumed all input, reset for next chunk
        if (self->parser->offset >= total_bytes_read)
        {
            total_bytes_read = 0;
        }
        else
        {
            // Move remaining unparsed data to the beginning of buffer
            size_t remaining = total_bytes_read - self->parser->offset;
            memmove(buffer, buffer + self->parser->offset, remaining);
            total_bytes_read = remaining;

            // Reset parser for the remaining data
            ForthParser__reset(self->parser, buffer);
        }
    }

    // Handle any remaining data in buffer
    if (total_bytes_read > 0)
    {
        buffer[total_bytes_read] = '\0';
        res = ForthInterpreter__parse_eval(self, buffer);
    }

    return res;
}
