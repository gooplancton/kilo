#include "jsonRPC.h"
#include "LspServer.h"
#include "RpcHandlers.c"
#include "SymbolDefinitions.c"

ForthLspServer *ForthLspServer__new(void)
{
  ForthLspServer *server = malloc(sizeof(*server));
  server->interpreter = ForthInterpreter__new(true);
  server->handlers = HandlerTable__new();
  server->definitions = SymbolDefinitionsTable__new();
  server->requested_shutdown = false;

  return server;
}

void ForthLspServer__drop(ForthLspServer *self)
{
  ForthInterpreter__drop(self->interpreter);
  HandlerTable__drop(self->handlers);
  free(self);
}

SymbolDefinition *ForthLspServer__get_symbol_definition(ForthLspServer *self, const char *symbol_name)
{
  return SymbolDefinitionsTable__get(self->definitions, symbol_name);
}

void ForthLspServer__handle_request(ForthLspServer *self, size_t req_id, const char *method_name, cJSON *params)
{
  HandlerTableEntry *entry = HandlerTable__get(self->handlers, method_name);
  if (!entry)
  {
    send_error_response(req_id, -32501, "Method not found");
    return;
  }

  cJSON *res = entry->handler(self, params);
  send_jsonrpc_response(req_id, res);
}

void ForthLspServer__handle_notification(ForthLspServer *self, const char *method_name, cJSON *params)
{
  HandlerTableEntry *entry = HandlerTable__get(self->handlers, method_name);
  if (!entry)
    return;

  entry->handler(self, params);
}

void ForthLspServer__handle_message(ForthLspServer *self, cJSON *json)
{
    cJSON* jsonrpc = cJSON_GetObjectItemCaseSensitive(json, "jsonrpc");
    if (!jsonrpc || strcmp(cJSON_GetStringValue(jsonrpc), "2.0") != 0) {
        cJSON_Delete(json);
        return;
    }
    
    // Get message type
    cJSON* id = cJSON_GetObjectItemCaseSensitive(json, "id");
    cJSON* method = cJSON_GetObjectItemCaseSensitive(json, "method");
    cJSON* params = cJSON_GetObjectItemCaseSensitive(json, "params");

    if (!method) {
      cJSON_Delete(json);
      return;
    }

    if (id)
      ForthLspServer__handle_request(self, id->valueint, cJSON_GetStringValue(method), params);
    else
      ForthLspServer__handle_notification(self, cJSON_GetStringValue(method), params);

    cJSON_Delete(json);
}
