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
    Symbol = 3,
    List = 4,
} ForthObjectType;

# define Any Number | String | Symbol | List

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
            bool quoted;
            char *chars;
        } string;
        struct
        {
            size_t len;
            size_t cap;
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
ForthObject *ForthObject__new_symbol(char *string, size_t len, bool quoted);
ForthObject *ForthObject__new_list(size_t cap);

// List push/pop
void ForthObject__list_push_copy(ForthObject *self, ForthObject *obj);
void ForthObject__list_push_move(ForthObject *self, ForthObject *obj);
ForthObject *ForthObject__list_pop(ForthObject *self);

// Utils
void ForthObject__print(ForthObject *obj);
bool ForthObject__eq(ForthObject *self, ForthObject *other);

#endif
