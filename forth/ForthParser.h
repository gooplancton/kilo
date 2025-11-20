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
ForthObject *ForthParser__parse_symbol(ForthParser *self, bool quoted);
ForthObject *ForthParser__parse_number(ForthParser *self);
ForthObject *ForthParser__parse_list(ForthParser *self);

#endif
