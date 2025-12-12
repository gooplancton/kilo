#ifndef FORTH_PARSER_H
#define FORTH_PARSER_H

#include "ForthObject.h"

typedef struct ForthParser
{
    char *string;
    size_t offset;
} ForthParser;

//  Parsers
ForthObject *ForthParser__parse_string(ForthParser *self);
ForthObject *ForthParser__parse_symbol(ForthParser *self);
ForthObject *ForthParser__parse_number(ForthParser *self);
ForthObject *ForthParser__parse_list(ForthParser *self);
ForthObject *ForthParser__parse_object(ForthParser *self);

// Utils
char *ForthParser__char(ForthParser *self);
char *ForthParser__symbol_name_at(ForthParser *self, size_t offset, size_t *out_len);
size_t ForthParser__line_col_to_offset(ForthParser *self, size_t line, size_t col);

// Factory
ForthParser *ForthParser__new(void);

// Reset
void ForthParser__reset(ForthParser *self, char *string);

// Drop
void ForthParser__drop(ForthParser *self);

#endif
