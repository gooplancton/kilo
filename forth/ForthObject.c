#include "ForthObject.h"

ForthObject *ForthObject__rc_clone(ForthObject *obj)
{
    obj->ref_count += 1;

    return obj;
}

void ForthObject__drop(ForthObject *obj)
{
    if (!obj)
        return;
    obj->ref_count -= 1;
    if (obj->ref_count == 0)
    {
        switch (obj->type)
        {
        case List:
            for (size_t i = 0; i < obj->list.len; i++)
                ForthObject__drop(obj->list.data[i]);
            free(obj->list.data);
            break;
        case String:
        case QuotedSymbol:
        case Symbol:
            free(obj->string.chars);
            break;
        default:
            break;
        }

        free(obj);
    }
}

ForthObject *ForthObject__new_number(double num)
{
    ForthObject *obj = malloc(sizeof(*obj));
    obj->type = Number;
    obj->num = num;
    obj->ref_count = 1;

    return obj;
}

ForthObject *ForthObject__new_string(char *string, size_t len)
{
    ForthObject *obj = malloc(sizeof(*obj));
    if (!obj)
        abort();
    obj->type = String;
    obj->string.chars = malloc(len + 1);
    if (!obj->string.chars)
        abort();
    memcpy(obj->string.chars, string, len);
    obj->string.chars[len] = '\0';
    obj->string.len = len;
    obj->ref_count = 1;

    return obj;
}

ForthObject *ForthObject__new_symbol(char *string, size_t len)
{
    ForthObject *obj = malloc(sizeof(*obj));
    if (!obj)
        abort();
    obj->type = Symbol;
    obj->string.chars = malloc(len + 1);
    if (!obj->string.chars)
        abort();
    memcpy(obj->string.chars, string, len);
    obj->string.chars[len] = '\0';
    obj->string.len = len;
    obj->ref_count = 1;

    return obj;
}

ForthObject *ForthObject__new_list(void)
{
    ForthObject *obj = malloc(sizeof(*obj));
    if (!obj)
        abort();
    obj->type = List;
    obj->list.len = 0;
    obj->list.cap = DEFAULT_LIST_CAP;
    obj->list.data = malloc(sizeof(*obj->list.data) * DEFAULT_LIST_CAP);
    if (!obj->list.data)
        abort();
    obj->ref_count = 1;

    return obj;
}

void ForthObject__list_reserve_slot(ForthObject *self)
{
    size_t needed = self->list.len + 1;
    if (needed > self->list.cap)
    {
        size_t new_cap = self->list.cap ? self->list.cap * 2 : DEFAULT_LIST_CAP;
        void *tmp = realloc(self->list.data, sizeof(*self->list.data) * new_cap);
        if (!tmp)
            abort();
        self->list.data = tmp;
        self->list.cap = new_cap;
    }
}

void ForthObject__list_push_copy(ForthObject *self, ForthObject *obj)
{
    assert(self->type == List);
    ForthObject__list_reserve_slot(self);
    self->list.data[self->list.len++] = ForthObject__rc_clone(obj);
}

void ForthObject__list_push_move(ForthObject *self, ForthObject *obj)
{
    assert(self->type == List);
    ForthObject__list_reserve_slot(self);
    self->list.data[self->list.len++] = obj;
}

ForthObject *ForthObject__list_pop(ForthObject *self)
{
    assert(self->type == List);
    if (self->list.len == 0)
        return NULL;
    self->list.len -= 1;

    return self->list.data[self->list.len];
}

void ForthObject__print(ForthObject *obj)
{
    switch (obj->type)
    {
    case String:
        printf("\"%.*s\"", (int)obj->string.len, obj->string.chars);
        break;
    case QuotedSymbol:
        printf("'%.*s", (int)obj->string.len, obj->string.chars);
        break;
    case Symbol:
        printf("%.*s", (int)obj->string.len, obj->string.chars);
        break;
    case Number:
        printf("%g", obj->num);
        break;
    case List:
        printf("[");
        for (size_t i = 0; i < obj->list.len; i++)
        {
            ForthObject__print(obj->list.data[i]);
            if (i != obj->list.len - 1)
                printf(", ");
        }
        printf("]");
        break;
    default:
        break;
    }
}
