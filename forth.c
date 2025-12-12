#include "ForthBuiltins.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    ForthInterpreter *in = ForthInterpreter__new(false);

    // If a filename is provided as command-line argument, execute it
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        
        ForthEvalError *errors = ForthInterpreter__run_file(in, argv[1]);
        for (size_t i = 0; errors[i].result != Ok; i++)
            printf("Error: %d at offset %d\n", errors[i].result, (int)errors[i].offset);

        free(errors);
        fclose(file);
        ForthInterpreter__drop(in);

        return 0;
    }

    // Otherwise, enter REPL mode
    printf("Forth Interpreter (Ctrl+D to exit)\n");
    
    char *line = NULL;
    size_t len = 0;
    size_t read;

    while (printf("> ") && (int)(read = getline(&line, &len, stdin)) != -1)
    {
        // Remove trailing newline
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        // Skip empty lines
        if (line[0] == '\0') {
            continue;
        }

        ForthEvalError *errors = ForthInterpreter__parse_eval(in, line);
        for (size_t i = 0; errors[i].result != Ok; i++)
            printf("Error: %d at offset %d\n", errors[i].result, (int)errors[i].offset);

        free(errors);
    }

    printf("\n");
    free(line);
    ForthInterpreter__drop(in);
    return 0;
}
