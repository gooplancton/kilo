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
    self->string = string;
}

void ForthParser__drop(ForthParser *self)
{
    free(self);
}

// ForthObject *ForthParser__next_object(ForthParser *self)
bool ForthParser__next_list(ForthParser *self)
{
    char *current_char = ForthParser__char(self);
    if (*current_char == '[' || *current_char == '$')
        return true;

    while (self->string[self->offset + 1])
    {
        self->offset += 1;
        char *current_char = ForthParser__char(self);
        if (*current_char == '[' || *current_char == '$')
            return true;
    }

    return false;
}

inline char *ForthParser__char(ForthParser *self)
{
    return self->string + self->offset;
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

    bool quoted = false;
    if (*starting_char == '\'')
    {
        quoted = true;
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

    return ForthObject__new_symbol(starting_char, len, quoted);
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

ForthObject *ForthParser__next_object(ForthParser *self)
{
    if (self->string == NULL)
        return NULL;

}

ForthObject *ForthParser__parse_list(ForthParser *self)
{
    char *starting_char = ForthParser__char(self);

    assert(*starting_char == '[');

    self->offset += 1; // opening bracket

    ForthObject *obj = ForthObject__new_list(DEFAULT_LIST_CAP);

    while (self->string[self->offset])
    {
        switch (self->string[self->offset])
        {
        case '+':
        case '\t':
        case '\n':
        case ' ':
        {
            self->offset += 1;
            break;
        }
        case ']':
        {
            self->offset += 1;
            return obj;
        }
        case '\'':
        {
            ForthObject *newObj = ForthParser__parse_symbol(self);
            if (newObj == NULL)
                goto parse_fail;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        case '"':
        {
            ForthObject *newObj = ForthParser__parse_string(self);
            if (newObj == NULL)
                goto parse_fail;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        case '$':
        case '[':
        {
            ForthObject *newObj = ForthParser__parse_list(self);
            if (newObj == NULL)
                goto parse_fail;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        case '-':
        {
            ForthObject *newObj = ForthParser__parse_number(self);
            if (newObj == NULL)
                goto parse_fail;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        default:
        {
            ForthObject *newObj;
            if (isdigit(*ForthParser__char(self)))
                newObj = ForthParser__parse_number(self);
            else
                newObj = ForthParser__parse_symbol(self);

            if (newObj == NULL)
                goto parse_fail;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        }
    }

    fprintf(stderr, "ParsingError: Unterminated list near offset: %d\n", (int)self->offset);

parse_fail:
    ForthObject__drop(obj);

    return NULL;
}
