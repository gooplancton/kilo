#include "jsonRPC.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

// Send a JSON response back to client
void send_json_response(cJSON* json) {
    char* json_str = cJSON_PrintUnformatted(json);
    if (!json_str) return;
    
    size_t length = strlen(json_str);
    printf("Content-Length: %zu\r\n\r\n%s", length, json_str);
    fflush(stdout);

    free(json_str);
}

// Send a JSON-RPC response
void send_jsonrpc_response(int id, cJSON* result) {
    cJSON* response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "jsonrpc", "2.0");
    cJSON_AddNumberToObject(response, "id", id);
    
    if (result) {
        cJSON_AddItemToObject(response, "result", result);
    } else {
        cJSON_AddNullToObject(response, "result");
    }
    
    send_json_response(response);
    cJSON_Delete(response);
}

// Send a JSON-RPC notification
void send_notification(const char* method, cJSON* params) {
    cJSON* notification = cJSON_CreateObject();
    cJSON_AddStringToObject(notification, "jsonrpc", "2.0");
    cJSON_AddStringToObject(notification, "method", method);
    
    if (params) {
        cJSON_AddItemToObject(notification, "params", params);
    }
    
    send_json_response(notification);
    cJSON_Delete(notification);
}

// Send an error response
void send_error_response(int id, int code, const char* message) {
    cJSON* error = cJSON_CreateObject();
    cJSON_AddNumberToObject(error, "code", code);
    cJSON_AddStringToObject(error, "message", message);
    
    cJSON* response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "jsonrpc", "2.0");
    cJSON_AddNumberToObject(response, "id", id);
    cJSON_AddItemToObject(response, "error", error);
    
    send_json_response(response);
    cJSON_Delete(response);
}

char* read_message() {
    char line[256];
    int content_length = 0;
    
    // Read headers
    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "Content-Length:", 15) == 0) {
            content_length = atoi(line + 15);
        }
        
        // Empty line indicates end of headers
        if (line[0] == '\r' && line[1] == '\n') break;
        if (line[0] == '\n') break;
    }
    
    
    if (content_length <= 0) {
        return NULL;
    }
    
    // Read JSON content
    char* content = malloc(content_length + 1);
    if (!content) return NULL;
    
    size_t read = fread(content, 1, content_length, stdin);
    if (read != content_length) {
        fprintf(stderr, "error: expected %i bytes, got %i", content_length, read);
        free(content);
        return NULL;
    }
    
    content[content_length] = '\0';
    
    return content;
}
