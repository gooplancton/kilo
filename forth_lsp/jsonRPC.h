#ifndef JSON_RPC_H
#define JSON_RPC_H

#include "cJSON.h"

void send_json_response(cJSON* json);
void send_jsonrpc_response(int id, cJSON* result);
void send_notification(const char* method, cJSON* params);
void send_error_response(int id, int code, const char* message);
char* read_message();

#endif // JSON_RPC_H