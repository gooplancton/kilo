// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "jsonRPC.h"
#include "LspServer.h"

// Read a message from stdin following LSP protocol
int main() {
    ForthLspServer *server = ForthLspServer__new();

    fprintf(stderr, "LSP Server started (PID: %d)\n", getpid());
    
    // Main message loop
    while (!feof(stdin)) {
        char* message = read_message();
        if (!message) {
            if (feof(stdin)) break;
            continue;
        }
        
        // Debug logging
        fprintf(stderr, "Received: %s\n", message);

        cJSON* json = cJSON_Parse(message);
        if (!json) {
            fprintf(stderr, "Failed to parse JSON\n");
            continue;
        }
        
        ForthLspServer__handle_message(server, json);

        free(message);
    }
    
    // Cleanup
    ForthLspServer__drop(server);
    fprintf(stderr, "LSP Server shutting down\n");
    
    return 0;
}