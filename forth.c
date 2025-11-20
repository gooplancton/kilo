#include "ForthParser.h"
#include "ForthInterpreter.h"
#include <stdio.h>

void builtin_add(ForthInterpreter *in)
{
    ForthObject *n1 = ForthInterpreter__pop_arg(in, Number);
    ForthObject *n2 = ForthInterpreter__pop_arg(in, Number);

    ForthObject *res = ForthObject__new_number(n1->num + n2->num);
    ForthObject__list_push_move(in->stack, res);
}

void builtin_print(ForthInterpreter *in)
{
    for (size_t i = 0; i < in->stack->list.len; i++)
    {
        ForthObject__print(in->stack->list.data[i]);
    }
}

int main(void)
{

    ForthInterpreter *in = ForthInterpreter__new();
    ForthInterpreter__register_function(in, "add", builtin_add);
    ForthInterpreter__register_function(in, "print", builtin_print);
    ForthInterpreter__register_literal(in, "pi", ForthObject__new_number(3.1415));

    ForthParser parser = {
        .string = "[1 pi add print]"};

    ForthObject *expr = ForthParser__parse_list(&parser);

    ForthInterpreter__eval(in, expr);

    return 0;
}
