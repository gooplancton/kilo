#include "ForthBuiltins.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    ForthInterpreter *in = ForthInterpreter__new();
    ForthInterpreter__load_builtins(in);

    // If a filename is provided as command-line argument, execute it
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            printf("Error: Could not open file '%s'\n", argv[1]);
            ForthInterpreter__drop(in);
            return 1;
        }
        
        ForthInterpreter__run_file(in, file);
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

        ForthInterpreter__parse_eval(in, line);
    }

    printf("\n");
    free(line);
    ForthInterpreter__drop(in);
    return 0;
}
