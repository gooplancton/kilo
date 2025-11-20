#include "ForthParser.h"
#include "ForthBuiltins.h"
#include <stdio.h>
#include <stdlib.h>


int main(void)
{
    ForthInterpreter *in = ForthInterpreter__new();
    ForthInterpreter__load_builtins(in);
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
