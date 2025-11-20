#include "ForthParser.h"

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
        return NULL; /* unterminated string */

    /* consume opening quote, content and closing quote */
    self->offset += len + 2;

    return ForthObject__new_string(shifted + 1, len);
}

ForthObject *ForthParser__parse_symbol(ForthParser *self)
{
    char *shifted = self->string + self->offset;
    if (shifted[0] == '\0')
        return NULL;

    size_t len = 0;

    while (isalpha((unsigned char)shifted[len]))
        len++;

    if (len == 0)
        return NULL; /* nothing parsed */

    self->offset += len;

    return ForthObject__new_symbol(shifted, len);
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
        return NULL;

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

    ForthObject *obj = ForthObject__new_list();

    while (self->string[self->offset])
    {
        switch (self->string[self->offset])
        {
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
        case '"':
        {
            ForthObject *newObj = ForthParser__parse_string(self);
            if (newObj == NULL)
                exit(1);

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        case '[':
        {
            ForthObject *newObj = ForthParser__parse_list(self);
            if (newObj == NULL)
                exit(1);

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        case '-':
        {
            ForthObject *newObj = ForthParser__parse_number(self);
            if (newObj == NULL)
                exit(1);

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        default:
        {
            ForthObject *newObj;
            if (isdigit((unsigned char)self->string[self->offset]))
                newObj = ForthParser__parse_number(self);
            else
                newObj = ForthParser__parse_symbol(self);

            if (newObj == NULL)
                exit(1);

            ForthObject__list_push_move(obj, newObj);
            break;
        }
        }
    }

    // error: unterminated list
    return NULL;
}
