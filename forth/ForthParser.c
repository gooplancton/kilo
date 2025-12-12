#include "ForthParser.h"

ForthParser *ForthParser__new(void)
{
    ForthParser *self = malloc(sizeof(*self));
    self->offset = 0;
    self->string = NULL;

    return self;
}

void ForthParser__reset(ForthParser *self, char *string)
{
    self->offset = 0;
    if (self->string)
        free(self->string);
    self->string = string;
}

void ForthParser__drop(ForthParser *self)
{
    free(self);
}

inline char *ForthParser__char(ForthParser *self)
{
    return self->string + self->offset;
}

char *ForthParser__symbol_name_at(ForthParser *self, size_t offset, size_t *out_len)
{
    fprintf(stderr, "debug: parser string: %s\n", self->string);
    if (self->string == NULL || offset >= strlen(self->string))
        return NULL;

    // Find the start of the word by going left
    int start = offset;
    while (start > 0 && (isalpha(self->string[start - 1]) || self->string[start - 1] == '_'))
        start--;

    // Find the end of the word by going right
    int end = offset;
    while (self->string[end] != '\0' && (isalpha(self->string[end]) || self->string[end] == '_'))
        end++;

    *out_len = end - start;
    return self->string + start;
}

size_t ForthParser__line_col_to_offset(ForthParser *self, size_t line, size_t col)
{
    size_t offset = 0;
    int current_line = 0;
    int current_char = 0;
    
    for (size_t i = 0; self->string[i] != '\0'; i++) {
        if (current_line == line && current_char == col) {
            offset = i;
            break;
        }
        
        if (self->string[i] == '\n') {
            current_line++;
            current_char = 0;
        } else {
            current_char++;
        }
    }

    return offset;
}

void ForthParser__offset_to_line_col(ForthParser *self, size_t offset, size_t *out_line, size_t *out_col)
{
    size_t current_line = 0;
    size_t current_char = 0;
    
    for (size_t i = 0; i < offset && self->string[i] != '\0'; i++) {
        if (self->string[i] == '\n') {
            current_line++;
            current_char = 0;
        } else {
            current_char++;
        }
    }

    *out_line = current_line;
    *out_col = current_char;
}

ForthObject *ForthParser__parse_string(ForthParser *self)
{
    char *starting_char = ForthParser__char(self);
    if (*starting_char == '\0' || *starting_char != '"')
        return NULL;

    self->offset += 1; // opening quote

    size_t len = 0;
    while (true)
    {
        char *next_char = ForthParser__char(self);
        if (*next_char != '"' && *next_char != '\0')
        {
            self->offset += 1;
            len += 1;
        }
        else
            break;
    }

    if (*ForthParser__char(self) != '"')
    {
        fprintf(stderr, "ParsingError: Unterminated string near offset %d\n", (int)self->offset);
        return NULL;
    }

    self->offset += 1; // closing quote

    return ForthObject__new_string(starting_char + 1, len);
}

ForthObject *ForthParser__parse_symbol(ForthParser *self)
{
    char *starting_char = ForthParser__char(self);
    if (*starting_char == '\0')
        return NULL;

    ForthSymbolFlag symbol_flag = Unquoted;
    if (*starting_char == ',')
    {
        symbol_flag = EagerlyEvaluated;
        self->offset += 1; // comma
        starting_char += 1;
    }
    else if (*starting_char == '\'')
    {
        symbol_flag = Quoted;
        self->offset += 1; // tick
        starting_char += 1;
    }

    size_t len = 0;

    while (true)
    {
        char *next_char = ForthParser__char(self);
        if (isalpha(*next_char) || *next_char == '_')
        {
            self->offset += 1;
            len += 1;
        }
        else
            break;
    }

    if (!len)
    {
        fprintf(stderr, "ParsingError: Empty symbol at offset %d\n", (int)self->offset);
        return NULL;
    }

    return ForthObject__new_symbol(starting_char, len, symbol_flag);
}

ForthObject *ForthParser__parse_number(ForthParser *self)
{
    if (self->string == NULL)
        return NULL;

    char *start = &self->string[self->offset];
    char *endptr = NULL;
    double num = strtod(start, &endptr);

    /* no characters consumed => parse failure */
    if (endptr == start)
    {
        fprintf(stderr, "ParsingError: Empty number at offset %d\n", (int)self->offset);
        return NULL;
    }

    /* update caller's offset to point after the parsed number */
    self->offset = (size_t)(endptr - self->string);

    return ForthObject__new_number(num);
}

ForthObject *ForthParser__parse_list(ForthParser *self)
{
    if (self->string == NULL)
        return NULL;

    bool quasiquoted = false;
    char *starting_char = ForthParser__char(self);
    if (*starting_char == '$')
    {
        quasiquoted = true;
        self->offset += 1; // dollar sign
        starting_char += 1;
    }

    assert(*starting_char == '[');

    self->offset += 1; // opening bracket

    ForthObject *obj = ForthObject__new_list(DEFAULT_LIST_CAP, quasiquoted);

    while (true)
    {
        ForthObject *el = ForthParser__parse_object(self);
        if (!el) {
            if (self->string[self->offset] == ']') {
                self->offset += 1;
                return obj;
            } else 
                goto parse_fail;
        }

        ForthObject__list_push_copy(obj, el);
    }


parse_fail:
    ForthObject__drop(obj);

    return NULL;
}

ForthObject *ForthParser__parse_object(ForthParser *self) {
    while (self->string[self->offset])
    {
        switch (self->string[self->offset])
        {
        case '#':
        {
            while (self->string[self->offset] && self->string[self->offset] != '\n')
                self->offset += 1;

            break;
        }
        case '+':
        case '\t':
        case '\n':
        case ' ':
        {
            self->offset += 1;
            break;
        }
        case ']':
            return NULL;
        case '\'':
            return ForthParser__parse_symbol(self);
        case '"':
            return ForthParser__parse_string(self);
        case '$':
        case '[':
            return ForthParser__parse_list(self);
        case '-':
            return ForthParser__parse_number(self);
        default:
            if (isdigit(*ForthParser__char(self)))
                return ForthParser__parse_number(self);
            else
                return ForthParser__parse_symbol(self);
        }
    }

    return NULL;
}
