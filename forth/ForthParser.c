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

bool ForthParser__next_list(ForthParser *self)
{
    if (self->string[self->offset] == '[')
        return true;

    while (self->string[self->offset + 1])
    {
        self->offset += 1;
        if (self->string[self->offset] == '[')
            return true;
    }

    return false;
}

ForthObject *ForthParser__parse_string(ForthParser *self)
{
    char *shifted = self->string + self->offset;
    if (shifted[0] == '\0' || shifted[0] != '"')
        return NULL;

    size_t len = 0;
    /* find closing quote, stop at '\0' to avoid overruns */
    while (shifted[len + 1] != '"' && shifted[len + 1] != '\0')
        len++;

    if (shifted[len + 1] != '"')
    {
        fprintf(stderr, "ParsingError: Unterminated string near offset %d\n", (int)self->offset);
        return NULL;
    }

    /* consume opening quote, content and closing quote */
    self->offset += len + 2;

    return ForthObject__new_string(shifted + 1, len);
}

ForthObject *ForthParser__parse_symbol(ForthParser *self, bool quoted)
{
    char *shifted = self->string + self->offset;
    if (shifted[0] == '\0')
        return NULL;

    size_t len = 0;

    while (isalpha((unsigned char)shifted[len]) || (unsigned char)shifted[len] == '_')
        len++;

    if (!len)
    {
        fprintf(stderr, "ParsingError: Empty symbol at offset %d\n", (int)self->offset);
        return NULL;
    }

    self->offset += len;

    return ForthObject__new_symbol(shifted, len, quoted);
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

    assert(self->string[self->offset] == '[');
    self->offset += 1;

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
            return obj;
        }
        case '\'':
        {
            self->offset += 1;
            ForthObject *newObj = ForthParser__parse_symbol(self, true);
            if (newObj == NULL)
                return NULL;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        case '"':
        {
            ForthObject *newObj = ForthParser__parse_string(self);
            if (newObj == NULL)
                return NULL;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        case '[':
        {
            ForthObject *newObj = ForthParser__parse_list(self);
            if (newObj == NULL)
                return NULL;

            ForthObject__list_push_move(obj, newObj);
            self->offset += 1;
            break;
        }
        case '-':
        {
            ForthObject *newObj = ForthParser__parse_number(self);
            if (newObj == NULL)
                return NULL;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        default:
        {
            ForthObject *newObj;
            if (isdigit((unsigned char)self->string[self->offset]))
                newObj = ForthParser__parse_number(self);
            else
                newObj = ForthParser__parse_symbol(self, false);

            if (newObj == NULL)
                return NULL;

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        }
    }

    fprintf(stderr, "ParsingError: Unterminated list near offset: %d\n", (int)self->offset);
    return NULL;
}
