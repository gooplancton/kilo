#include "ForthObject.h"

void ForthObjectType__format(ForthObjectType self, char *buffer, size_t buf_size)
{
    buffer[0] = '\0';
    bool first = true;

    if (self & Number)
    {
        strncat(buffer, "Number", buf_size - strlen(buffer) - 1);
        first = false;
    }
    if (self & String)
    {
        if (!first)
            strncat(buffer, ", ", buf_size - strlen(buffer) - 1);
        strncat(buffer, "String", buf_size - strlen(buffer) - 1);
        first = false;
    }
    if (self & Symbol)
    {
        if (!first)
            strncat(buffer, ", ", buf_size - strlen(buffer) - 1);
        strncat(buffer, "Symbol", buf_size - strlen(buffer) - 1);
        first = false;
    }
    if (self & List)
    {
        if (!first)
            strncat(buffer, ", ", buf_size - strlen(buffer) - 1);
        strncat(buffer, "List", buf_size - strlen(buffer) - 1);
    }
}

const char *ForthObjectType__as_str(ForthObjectType self)
{
    switch (self)
    {
    case Number:
        return "Number";
    case String:
        return "String";
    case Symbol:
        return "Symbol";
    case List:
        return "List";
    default:
        return "Invalid";
    }
}

ForthObject *ForthObject__rc_clone(ForthObject *self)
{
    self->ref_count += 1;

    return self;
}

ForthObject *ForthObject__deep_clone(ForthObject *self)
{
    switch (self->type)
    {
    case Number:
        return ForthObject__new_number(self->num);
    case String:
        return ForthObject__new_string(self->string.chars, self->string.len);
    case Symbol:
        return ForthObject__new_symbol(self->string.chars, self->string.len, self->string.quoted);
    case List:
    {
        ForthObject *res = ForthObject__new_list(self->list.len);
        for (size_t i = 0; i < self->list.len; i++)
            ForthObject__list_push_move(res, ForthObject__deep_clone(self->list.data[i]));

        return res;
    }
    }
}

void ForthObject__drop(ForthObject *self)
{
    if (!self)
        return;
    self->ref_count -= 1;
    if (self->ref_count == 0)
    {
        switch (self->type)
        {
        case List:
            for (size_t i = 0; i < self->list.len; i++)
                ForthObject__drop(self->list.data[i]);
            free(self->list.data);
            break;
        case String:
        case Symbol:
            free(self->string.chars);
            break;
        default:
            break;
        }

        free(self);
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
    obj->string.quoted = false;
    obj->string.len = len;
    obj->ref_count = 1;

    return obj;
}

ForthObject *ForthObject__new_symbol(char *string, size_t len, bool quoted)
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
    obj->string.quoted = quoted;
    obj->ref_count = 1;

    return obj;
}

ForthObject *ForthObject__new_list(size_t cap)
{
    ForthObject *obj = malloc(sizeof(*obj));
    if (!obj)
        abort();
    obj->type = List;
    obj->list.len = 0;
    obj->list.cap = cap;
    obj->list.data = malloc(sizeof(*obj->list.data) * cap);
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
    case Symbol:
        if (obj->string.quoted)
            printf("'%.*s", (int)obj->string.len, obj->string.chars);
        else
            printf("%.*s", (int)obj->string.len, obj->string.chars);
        break;
    case String:
        printf("\"%.*s\"", (int)obj->string.len, obj->string.chars);
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
                printf(" ");
        }
        printf("]");
        break;
    default:
        break;
    }
}

bool ForthObject__eq(ForthObject *self, ForthObject *other)
{
    if (self->type != other->type)
        return false;

    switch (self->type)
    {
    case Number:
        return self->num == other->num;
    case String:
    case Symbol:
        return self->string.len == other->string.len &&
               memcmp(self->string.chars, other->string.chars, self->string.len) == 0;
    case List:
    {
        if (self->list.len != other->list.len)
            return false;

        for (size_t i = 0; i < self->list.len; i++)
        {
            if (!ForthObject__eq(self->list.data[i], other->list.data[i]))
                return false;
        }

        return true;
    }
    }

    return false;
}
