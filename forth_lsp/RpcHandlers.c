#include "LspServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_HANDLERS 10

cJSON *lsp_handle_initialize(ForthLspServer *server, cJSON* params) {
    (void)server;
    (void)params;
    // TODO: Parse client capabilities from params if needed

    // Build server capabilities
    cJSON* capabilities = cJSON_CreateObject();
    
    // Text document synchronization (1 = full sync)
    cJSON_AddNumberToObject(capabilities, "textDocumentSync", 1);
    
    // Hover support
    cJSON_AddTrueToObject(capabilities, "hoverProvider");
    
    // Completion support
    cJSON* completion = cJSON_CreateObject();
    cJSON_AddTrueToObject(completion, "completionProvider");
    cJSON* trigger_chars = cJSON_CreateArray();
    cJSON_AddItemToArray(trigger_chars, cJSON_CreateString("."));
    cJSON_AddItemToObject(completion, "triggerCharacters", trigger_chars);
    cJSON_AddItemToObject(capabilities, "completionProvider", completion);
    
    // // Signature help (optional)
    cJSON_AddFalseToObject(capabilities, "signatureHelpProvider");
    
    // Declaration support (optional)
    cJSON_AddFalseToObject(capabilities, "declarationProvider");
    
    // Definition support
    cJSON_AddTrueToObject(capabilities, "definitionProvider");
    
    // References support
    cJSON_AddTrueToObject(capabilities, "referencesProvider");
    
    // Document symbols
    cJSON_AddTrueToObject(capabilities, "documentSymbolProvider");

    // Build response
    cJSON* result = cJSON_CreateObject();
    cJSON_AddItemToObject(result, "capabilities", capabilities);
    
    // Add server info
    cJSON* server_info = cJSON_CreateObject();
    cJSON_AddStringToObject(server_info, "name", "forthls");
    cJSON_AddStringToObject(server_info, "version", "0.1.0");
    cJSON_AddItemToObject(result, "serverInfo", server_info);
    
    return result;
}

cJSON *lsp_handle_shutdown(ForthLspServer *server, cJSON *params) {
    (void)params;

    server->requested_shutdown = true;
    
    fprintf(stderr, "Shutdown requested\n");

    return NULL;
}

cJSON *lsp_handle_hover(ForthLspServer *server, cJSON* params) {
    // Extract text document position
    cJSON* text_document = cJSON_GetObjectItem(params, "textDocument");
    cJSON* position = cJSON_GetObjectItem(params, "position");
    
    if (!text_document || !position) {
        return cJSON_CreateNull();
    }
    
    // Get URI and position details
    cJSON* uri = cJSON_GetObjectItem(text_document, "uri");
    cJSON* line_obj = cJSON_GetObjectItem(position, "line");
    cJSON* character_obj = cJSON_GetObjectItem(position, "character");

    int line = cJSON_GetNumberValue(line_obj);
    int character = cJSON_GetNumberValue(character_obj);
    size_t offset = ForthParser__line_col_to_offset(server->interpreter->parser, line, character);
    
    fprintf(stderr, "Hover requested at offset: %d (%d:%d)\n",
            offset,
            line,
            character);

    // Convert line:character to offset
    size_t symbol_len = 0;
    char *symbol = ForthParser__symbol_name_at(server->interpreter->parser, offset, &symbol_len);
    
    if (!symbol || symbol_len == 0) {
        return cJSON_CreateNull();
    }
    
    // Create hover result with the symbol
    cJSON* contents = cJSON_CreateArray();
    cJSON* content_item = cJSON_CreateObject();
    cJSON_AddStringToObject(content_item, "language", "forth");
    
    // Create string with symbol (need to null-terminate it)
    char symbol_str[symbol_len + 1];
    memcpy(symbol_str, symbol, symbol_len);
    symbol_str[symbol_len] = '\0';

    SymbolDefinition *def = ForthLspServer__get_symbol_definition(server, symbol_str);
    if (def && def->documentation) {
        cJSON_AddStringToObject(content_item, "value", def->documentation);
    } else {
        cJSON_AddStringToObject(content_item, "value", symbol_str);
    }
    cJSON_AddItemToArray(contents, content_item);
    
    cJSON* result = cJSON_CreateObject();
    cJSON_AddItemToObject(result, "contents", contents);
    
    return result;
}

cJSON *lsp_handle_completion(ForthLspServer *server, cJSON* params) {
    (void)params;

    // Create completion list
    cJSON* items = cJSON_CreateArray();
    
    // Add completions from interpreter symbols
    SymbolsTable* symbols = server->interpreter->symbols;
    for (size_t i = 0; i < symbols->len; i++) {
        SymbolsTableEntry* entry = symbols->entries[i];
        
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "label", entry->key);
        cJSON_AddNumberToObject(item, "kind", (entry->type == Function) ? 12 : 13);
        cJSON_AddStringToObject(item, "detail", (entry->type == Function) ? "Function" : "Variable");
        
        cJSON_AddItemToArray(items, item);
    }
    
    cJSON* result = cJSON_CreateObject();
    cJSON_AddFalseToObject(result, "isIncomplete");
    cJSON_AddItemToObject(result, "items", items);
    
    return result;
}

cJSON *lsp_handle_initialized(ForthLspServer *server, cJSON* params) {
    (void)server;
    (void)params;

    fprintf(stderr, "Client initialized notification received\n");
    
    return NULL;
}

cJSON *lsp_handle_did_open(ForthLspServer *server, cJSON* params) {
    cJSON* text_document = cJSON_GetObjectItem(params, "textDocument");
    if (!text_document) return NULL;
    
    cJSON* uri = cJSON_GetObjectItem(text_document, "uri");
    cJSON* text = cJSON_GetObjectItem(text_document, "text");

    // remove the file:// prefix from the uri to get the file path
    char *file_path = cJSON_GetStringValue(uri) + 7;
    
    fprintf(stderr, "Document opened: %s\n", cJSON_GetStringValue(uri));
    fprintf(stderr, "Text length: %zu\n", strlen(cJSON_GetStringValue(text)));

    if (server->errors)
        free(server->errors);

    server->errors = ForthInterpreter__run_file(server->interpreter, file_path);
    ForthLspServer__publish_diagnostics(server, cJSON_GetStringValue(uri));
    
    return NULL;
}

cJSON *lsp_handle_did_save(ForthLspServer *server, cJSON* params) {
    cJSON* text_document = cJSON_GetObjectItem(params, "textDocument");
    
    if (!text_document)
        return NULL;
    
    cJSON* uri = cJSON_GetObjectItem(text_document, "uri");
    // remove the file:// prefix from the uri to get the file path
    char *file_path = cJSON_GetStringValue(uri) + 7;

    if (server->errors)
        free(server->errors);
    
    server->errors = ForthInterpreter__run_file(server->interpreter, file_path);
    ForthLspServer__publish_diagnostics(server, cJSON_GetStringValue(uri));

    return NULL;
}

cJSON *lsp_handle_did_close(ForthLspServer *server, cJSON* params) {
    (void)server;
    (void)params;

    cJSON* text_document = cJSON_GetObjectItem(params, "textDocument");
    if (!text_document) return NULL;
    
    cJSON* uri = cJSON_GetObjectItem(text_document, "uri");
    fprintf(stderr, "Document closed: %s\n", cJSON_GetStringValue(uri));
    
    // TODO: Clean up document state
    return NULL;
}

cJSON *lsp_handle_exit(ForthLspServer *server, cJSON* params) {
    (void)params;

    fprintf(stderr, "Exit notification received\n");
    if (server->requested_shutdown) {
        exit(0);
    } else {
        exit(1);
    }

    return NULL;
}

cJSON *lsp_handle_symbol(ForthLspServer *server, cJSON* params) {
    cJSON* text_document = cJSON_GetObjectItem(params, "textDocument");
    if (!text_document) return NULL;
    
    cJSON* uri = cJSON_GetObjectItem(text_document, "uri");
    fprintf(stderr, "Document symbols requested for: %s\n", cJSON_GetStringValue(uri));
    
    // Create symbol list
    cJSON* result = cJSON_CreateArray();
    
    // Iterate through symbols in the interpreter
    SymbolsTable* symbols = server->interpreter->symbols;
    for (size_t i = 0; i < symbols->len; i++) {
        SymbolsTableEntry* entry = symbols->entries[i];
        
        // Create DocumentSymbol object
        cJSON* symbol = cJSON_CreateObject();
        cJSON_AddStringToObject(symbol, "name", entry->key);
        
        // Kind: 12 = Function, 13 = Variable
        int kind = (entry->type == Function) ? 12 : 13;
        cJSON_AddNumberToObject(symbol, "kind", kind);
        
        // Placeholder range (0,0) to (0,0)
        cJSON* range = cJSON_CreateObject();
        cJSON* start = cJSON_CreateObject();
        cJSON_AddNumberToObject(start, "line", 0);
        cJSON_AddNumberToObject(start, "character", 0);
        cJSON_AddItemToObject(range, "start", start);
        
        cJSON* end = cJSON_CreateObject();
        cJSON_AddNumberToObject(end, "line", 0);
        cJSON_AddNumberToObject(end, "character", 0);
        cJSON_AddItemToObject(range, "end", end);
        
        cJSON_AddItemToObject(symbol, "range", range);
        cJSON_AddItemToObject(symbol, "selectionRange", cJSON_Duplicate(range, 1));
        
        cJSON_AddItemToArray(result, symbol);
    }
    
    return result;
}

void ForthLspServer__publish_diagnostics(ForthLspServer *server, const char *uri)
{
    ForthEvalError *errors = server->errors;
    // Create diagnostics array
    cJSON* diagnostics = cJSON_CreateArray();
    
    for (size_t i = 0; errors[i].result != Ok; i++) {
        cJSON* diagnostic = cJSON_CreateObject();

        size_t line = 0, col = 0;
        ForthParser__offset_to_line_col(server->interpreter->parser, errors[i].offset, &line, &col);
        
        // For now, use placeholder range (0,0) to (0,0)
        cJSON* range = cJSON_CreateObject();
        cJSON* start = cJSON_CreateObject();
        cJSON_AddNumberToObject(start, "line", line);
        cJSON_AddNumberToObject(start, "character", col);
        cJSON_AddItemToObject(range, "start", start);
        
        cJSON* end = cJSON_CreateObject();
        cJSON_AddNumberToObject(end, "line", line);
        cJSON_AddNumberToObject(end, "character", col);
        cJSON_AddItemToObject(range, "end", end);
        
        cJSON_AddItemToObject(diagnostic, "range", range);
        
        // Severity: 1 = Error
        cJSON_AddNumberToObject(diagnostic, "severity", 1);
        
        // Message based on error type
        const char* message = "Unknown error";
        switch (errors[i].result) {
            case ArityError:
                message = "Arity error: wrong number of arguments";
                break;
            case TypeError:
                message = "Type error: invalid type";
                break;
            case MathError:
                message = "Math error: division by zero or invalid operation";
                break;
            case ParsingError:
                message = "Parsing error";
                break;
            case IndexError:
                message = "Index error: out of bounds";
                break;
            case UnknownSymbolError:
                message = "Unknown symbol";
                break;
            case FileNotFoundError:
                message = "File not found";
                break;
            default:
                break;
        }
        cJSON_AddStringToObject(diagnostic, "message", message);
        
        cJSON_AddItemToArray(diagnostics, diagnostic);
    }
    
    // Create params object
    cJSON* params = cJSON_CreateObject();
    cJSON_AddStringToObject(params, "uri", uri);
    cJSON_AddItemToObject(params, "diagnostics", diagnostics);
    
    // Send notification
    send_notification("textDocument/publishDiagnostics", params);
}


HandlerTable *HandlerTable__new(void)
{
  HandlerTable *handlers = malloc(sizeof(*handlers));
  handlers->len = N_HANDLERS;
  handlers->entries = malloc(N_HANDLERS * sizeof(HandlerTableEntry));
  handlers->entries[0] = (HandlerTableEntry){ .handler=lsp_handle_initialize, .method_name="initialize" };
  handlers->entries[1] = (HandlerTableEntry){ .handler=lsp_handle_initialized, .method_name="initialized" };
  handlers->entries[2] = (HandlerTableEntry){ .handler=lsp_handle_did_open, .method_name="textDocument/didOpen" };
  handlers->entries[3] = (HandlerTableEntry){ .handler=lsp_handle_did_close, .method_name="textDocument/didClose" };
  handlers->entries[4] = (HandlerTableEntry){ .handler=lsp_handle_did_save, .method_name="textDocument/didSave" };
  handlers->entries[5] = (HandlerTableEntry){ .handler=lsp_handle_exit, .method_name="exit" };
  handlers->entries[6] = (HandlerTableEntry){ .handler=lsp_handle_shutdown, .method_name="shutdown" };
  handlers->entries[7] = (HandlerTableEntry){ .handler=lsp_handle_hover, .method_name="textDocument/hover" };
  handlers->entries[8] = (HandlerTableEntry){ .handler=lsp_handle_completion, .method_name="textDocument/completion" };
  handlers->entries[9] = (HandlerTableEntry){ .handler=lsp_handle_symbol, .method_name="textDocument/documentSymbol" };

  return handlers;
}

HandlerTableEntry *HandlerTable__get(HandlerTable *self, const char *method_name)
{
  for (uint8_t i = 0; i < self->len; i++)
  {
    HandlerTableEntry *entry = self->entries + i;
    if (strcmp(method_name, entry->method_name) == 0)
      return entry;
  }

  return NULL;
}

void HandlerTable__drop(HandlerTable *self)
{
  for (uint8_t i = 0; i < self->len; i++)
    free(self->entries[i].method_name);

  free(self->entries);
  free(self);
}