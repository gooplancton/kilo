#include "ForthInterpreter.h"
#include "ForthObject.h"
#include "ForthParser.h"
#include "ForthBuiltins.h"

SymbolsTableEntry *SymbolsTableEntry__new_object(char *key, ForthObject *obj)
{
    size_t key_len = strlen(key) + 1;
    size_t alloc_size = sizeof(SymbolsTableEntry) + key_len;

    SymbolsTableEntry *self = malloc(alloc_size);
    if (!self)
        abort();

    self->type = Object;
    self->obj = ForthObject__rc_clone(obj);
    memcpy(self->key, key, key_len);

    return self;
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
    if (self->type == Object)
        ForthObject__drop(self->obj);

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

void SymbolsTable__add_object(SymbolsTable *self, char *key, ForthObject *obj)
{
    SymbolsTableEntry *existing_entry = SymbolsTable__get(self, key);
    if (existing_entry)
    {
        if (existing_entry->type != Function)
            ForthObject__drop(existing_entry->obj);
        existing_entry->type = Object;
        existing_entry->obj = ForthObject__rc_clone(obj);
        return;
    }

    SymbolsTable__reserve_slot(self);

    self->entries[self->len] = SymbolsTableEntry__new_object(key, obj);
    self->len += 1;
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

ForthInterpreter *ForthInterpreter__new(bool is_sandboxed)
{
    ForthInterpreter *self = malloc(sizeof(*self));
    self->stack = ForthObject__new_list(DEFAULT_LIST_CAP, false);
    self->symbols = SymbolsTable__new();
    self->parser = ForthParser__new();
    self->is_sandboxed = is_sandboxed;
    ForthInterpreter__load_builtins(self);

    return self;
}

void ForthInterpreter__drop(ForthInterpreter *self)
{
    ForthObject__drop(self->stack);
    SymbolsTable__drop(self->symbols);
    free(self);
}

void ForthInterpreter__register_object(ForthInterpreter *self, char *key, ForthObject *obj)
{
    SymbolsTable__add_object(self->symbols, key, obj);
}

void ForthInterpreter__register_function(ForthInterpreter *self, char *key, ForthEvalResult (*function)(ForthInterpreter *interpreter))
{
    SymbolsTable__add_function(self->symbols, key, function);
}

ForthObject *ForthInterpreter__resolve_template(ForthInterpreter *self, ForthObject *template)
{
    assert(template->type == List && template->list.quasiquoted);
    ForthObject *resolved = ForthObject__new_list(template->list.len, false);
    for (size_t i = 0; i < template->list.len; i++)
    {
        ForthObject *el = template->list.data[i];

        if (el->type == List && el->list.quasiquoted)
        {
            ForthObject *nested_resolved = ForthInterpreter__resolve_template(self, el);
            ForthObject__list_push_move(resolved, nested_resolved);
            continue;
        }

        if (el->type != Symbol || el->string.symbol_flag != EagerlyEvaluated)
        {
            ForthObject__list_push_copy(resolved, el);
            continue;
        }
        SymbolsTableEntry *evaluated = SymbolsTable__get(self->symbols, el->string.chars);
        if (!evaluated)
        {
            fprintf(stderr, "UnknownSymbolError: %s\n", el->string.chars);
            continue;
        }

        if (evaluated->type == Object)
            ForthObject__list_push_copy(resolved, evaluated->obj);
        else
            evaluated->function(self);
    }

    return resolved;
}

ForthEvalResult ForthInterpreter__eval(ForthInterpreter *self, ForthObject *expr)
{
    ForthEvalResult res = Ok;

    switch (expr->type)
    {
    case List:
    {
        if (expr->list.quasiquoted)
        {
            ForthObject *resolved = ForthInterpreter__resolve_template(self, expr);
            res = ForthInterpreter__eval(self, resolved);

            ForthObject__drop(resolved);
        }
        else
            ForthObject__list_push_copy(self->stack, expr);

        break;
    }
    case Symbol:
    {
        if (expr->string.symbol_flag != Quoted)
        {
            char *key = expr->string.chars;
            SymbolsTableEntry *entry = SymbolsTable__get(self->symbols, key);
            if (!entry)
            {
                fprintf(stderr, "UnknownSymbolError: %s\n", key);
                res = UnknownSymbolError;
                break;
            }

            res = entry->type == Object ? ForthInterpreter__eval_every(self, entry->obj) : entry->function(self);
        }
        else
            ForthObject__list_push_copy(self->stack, expr);

        break;
    }
    default:
        ForthObject__list_push_copy(self->stack, expr);
    }

    if (res != Ok)
    {
        fprintf(stderr, "   while evaluating ");
        ForthObject__fprint(stderr, expr);
        fprintf(stderr, "\n");
    }

    return res;
}

ForthEvalResult ForthInterpreter__eval_every(ForthInterpreter *self, ForthObject *expr)
{
    ForthEvalResult res = Ok;

    if (expr->type == List)
        for (size_t i = 0; i < expr->list.len; i++)
            res |= ForthInterpreter__eval(self, expr->list.data[i]);
    else
        res = ForthInterpreter__eval(self, expr);

    if (res != Ok)
    {
        fprintf(stderr, "   while evaluating ");
        ForthObject__fprint(stderr, expr);
        fprintf(stderr, "\n");
    }

    return res;
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
            char expected[128];
            ForthObjectType__format(types[i], expected, sizeof(expected));

            fprintf(stderr, "TypeError: expected argument %zu to be one of (%s), got %s\n",
                    i, expected, ForthObjectType__as_str(obj->type));
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
        ForthObject *obj = ForthParser__parse_object(self->parser);
        if (!obj)
            break;

        res = ForthInterpreter__eval(self, obj);
        ForthObject__drop(obj);
    }

    return res;
}

ForthEvalResult ForthInterpreter__run_file(ForthInterpreter *self, char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (!file)
        return FileNotFoundError;

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(file_size + 1);

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    ForthEvalResult res = ForthInterpreter__parse_eval(self, buffer);
    fclose(file);

    return res;
}
