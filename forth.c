#include "ForthParser.h"
#include "ForthInterpreter.h"
#include <stdio.h>
#include <stdlib.h>

void builtin_add(ForthInterpreter *in)
{
    ForthObject *n2 = ForthInterpreter__pop_arg_typed(in, Number);
    ForthObject *n1 = ForthInterpreter__pop_arg_typed(in, Number);

    ForthObject *res = ForthObject__new_number(n1->num + n2->num);
    ForthObject__list_push_move(in->stack, res);
}

void builtin_print(ForthInterpreter *in)
{
    ForthObject__print(in->stack);

    printf("\n");
}

void builtin_eval(ForthInterpreter *in)
{
    ForthObject *expr = ForthInterpreter__pop_arg_typed(in, List);

    ForthInterpreter__eval(in, expr);
}

void builtin_unquote(ForthInterpreter *in)
{
    ForthObject *sym = ForthInterpreter__pop_arg_typed(in, Symbol);
    sym->string.quoted = false;

    ForthObject__list_push_move(in->stack, sym);
}

void builtin_define(ForthInterpreter *in)
{
    ForthObject *val = ForthInterpreter__pop_arg(in);
    ForthObject *key = ForthInterpreter__pop_arg_typed(in, Symbol);

    if (!key->string.quoted)
    {
        printf("Runtime error: unexpected unquoted symbol");
        exit(1);
    }

    ForthInterpreter__register_literal(in, key->string.chars, val);
}

void builtin_print_symbols(ForthInterpreter *in)
{
    for (size_t i = 0; i < in->symbols->len; i++)
    {
        printf("%s ", in->symbols->entries[i]->key);
    }

    printf("\n");
}

int main(void)
{
    ForthInterpreter *in = ForthInterpreter__new();
    ForthInterpreter__register_function(in, "add", builtin_add);
    ForthInterpreter__register_function(in, "print", builtin_print);
    ForthInterpreter__register_function(in, "eval", builtin_eval);
    ForthInterpreter__register_function(in, "unquote", builtin_unquote);
    ForthInterpreter__register_function(in, "define", builtin_define);
    ForthInterpreter__register_function(in, "symbols", builtin_print_symbols);
    ForthInterpreter__register_literal(in, "pi", ForthObject__new_number(3.1415));

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, stdin)) != -1)
    {
        ForthParser parser = {
            .string = line};

        ForthObject *expr = ForthParser__parse_list(&parser);
        if (expr)
        {
            ForthInterpreter__eval(in, expr);
            ForthObject__drop(expr);
        }
    }

    free(line);
    return 0;
}
