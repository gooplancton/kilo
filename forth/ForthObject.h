#ifndef FORTH_OBJECT_H
#define FORTH_OBJECT_H

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define DEFAULT_LIST_CAP 10

typedef enum
{
    Number = 1,
    String = 2,
    Symbol = 4,
    List = 8,
} ForthObjectType;

#define Any Number | String | Symbol | List

typedef enum
{
    Unquoted,
    Quoted,
    EagerlyEvaluated,
} ForthSymbolFlag;

void ForthObjectType__format(ForthObjectType self, char *buffer, size_t buf_size);
const char *ForthObjectType__as_str(ForthObjectType self);

typedef struct ForthObject
{
    ForthObjectType type;
    size_t ref_count;
    union
    {
        double num;
        struct
        {
            size_t len;
            ForthSymbolFlag symbol_flag;
            char *chars;
        } string;
        struct
        {
            size_t cap;
            size_t len;
            bool quasiquoted;
            struct ForthObject **data;
        } list;
    };
} ForthObject;

// Memory management
ForthObject *ForthObject__rc_clone(ForthObject *obj);
ForthObject *ForthObject__deep_clone(ForthObject *obj);
void ForthObject__drop(ForthObject *obj);

// Factories
ForthObject *ForthObject__new_number(double num);
ForthObject *ForthObject__new_string(char *string, size_t len);
ForthObject *ForthObject__new_symbol(char *string, size_t len, ForthSymbolFlag symbol_flag);
ForthObject *ForthObject__new_list(size_t cap, bool quasiquoted);

// List push/pop
void ForthObject__list_push_copy(ForthObject *self, ForthObject *obj);
void ForthObject__list_push_move(ForthObject *self, ForthObject *obj);
ForthObject *ForthObject__list_pop(ForthObject *self);

// Utils
void ForthObject__fprint(FILE* fd, ForthObject *obj);
void ForthObject__print(ForthObject *obj);
bool ForthObject__eq(ForthObject *self, ForthObject *other);

#endif
