#include "LspServer.h"

#define N_BUILTIN_SYMBOLS 30 // double check

SymbolDefinitionsTable *SymbolDefinitionsTable__new(void)
{
  SymbolDefinitionsTable *definitions = malloc(sizeof(*definitions));
  definitions->entries = malloc(N_BUILTIN_SYMBOLS * sizeof(SymbolDefinition));
  definitions->cap = N_BUILTIN_SYMBOLS;
  definitions->len = 1;
  definitions->entries[0] = (SymbolDefinition){
    .symbol_name="add",
    .documentation="[n2:Number n1:Number add] -> [res:Number]\n\nPops two numbers from the stack, adds them and pushes back the result",
    .type_in=(uint8_t[]){Number, Number, 0},
    .type_out=(uint8_t[]){Number, 0},
  };

  return definitions;
}

void SymbolDefinition__drop(SymbolDefinition *self)
{
  free(self->symbol_name);
  free(self->documentation);
  if (self->type_in)
    free(self->type_in);
  if (self->type_out)
    free(self->type_out);
}

void SymbolDefinitionsTable__drop(SymbolDefinitionsTable *self)
{
  for (size_t i = 0; i < self->len; i++)
    SymbolDefinition__drop(self->entries + i);

  free(self->entries);
  free(self);
}

SymbolDefinition *SymbolDefinitionsTable__get(SymbolDefinitionsTable *self, const char *symbol_name)
{
  for (size_t i = 0; i < self->len; i++)
    if (strcmp(self->entries[i].symbol_name, symbol_name) == 0)
      return &self->entries[i];

  return NULL;
}

void SymbolDefinitionsTable__add(SymbolDefinitionsTable *self, SymbolDefinition *definition)
{
  // Check if entry already exists and replace it
  for (size_t i = 0; i < self->len; i++)
  {
    if (strcmp(self->entries[i].symbol_name, definition->symbol_name) == 0)
    {
      SymbolDefinition__drop(self->entries + i);
      self->entries[i] = *definition;
      return;
    }
  }
  
  // If not found, add new entry
  if (self->len >= self->cap)
  {
    self->cap *= 2;
    self->entries = realloc(self->entries, self->cap * sizeof(SymbolDefinition));
  }
  
  self->entries[self->len++] = *definition;
}

void SymbolDefinitionsTable__remove(SymbolDefinitionsTable *self, const char *symbol_name)
{
  for (size_t i = 0; i < self->len; i++)
    if (strcmp(self->entries[i].symbol_name, symbol_name) == 0)
    {
      SymbolDefinition__drop(self->entries + i);
      
      // Swap with last element and decrement length
      if (i < self->len - 1)
        self->entries[i] = self->entries[self->len - 1];

      self->len--;
      return;
    }
}
