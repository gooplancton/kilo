#include "ForthBuiltins.h"
#include "ForthInterpreter.h"
#include "ForthObject.h"

// Macros for common patterns
#define BINARY_MATH_OP(op_name, operation)                                                      \
    ForthEvalResult builtin_##op_name(ForthInterpreter *in)                                     \
    {                                                                                           \
        ForthObject *n1 = NULL, *n2 = NULL;                                                     \
        ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &n1, Number, &n2, Number); \
        if (args_res != Ok)                                                                     \
            return args_res;                                                                    \
        ForthObject *res = ForthObject__new_number(operation);                                  \
        ForthObject__list_push_move(in->stack, res);                                            \
        ForthObject__drop(n1);                                                                  \
        ForthObject__drop(n2);                                                                  \
        return Ok;                                                                              \
    }

#define BINARY_COMPARISON_OP(op_name, comparison)                                               \
    ForthEvalResult builtin_##op_name(ForthInterpreter *in)                                     \
    {                                                                                           \
        ForthObject *n1 = NULL, *n2 = NULL;                                                     \
        ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &n1, Number, &n2, Number); \
        if (args_res != Ok)                                                                     \
            return args_res;                                                                    \
        ForthObject *res = ForthObject__new_number((comparison) ? 1.0 : 0.0);                   \
        ForthObject__list_push_move(in->stack, res);                                            \
        ForthObject__drop(n1);                                                                  \
        ForthObject__drop(n2);                                                                  \
        return Ok;                                                                              \
    }

#define BINARY_LOGIC_OP(op_name, operation)                                                     \
    ForthEvalResult builtin_##op_name(ForthInterpreter *in)                                     \
    {                                                                                           \
        ForthObject *b1 = NULL, *b2 = NULL;                                                     \
        ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &b1, Number, &b2, Number); \
        if (args_res != Ok)                                                                     \
            return args_res;                                                                    \
        bool b1_val = b1->num != 0.0;                                                           \
        bool b2_val = b2->num != 0.0;                                                           \
        double result = (operation) ? 1.0 : 0.0;                                                \
        ForthObject *res = ForthObject__new_number(result);                                     \
        ForthObject__list_push_move(in->stack, res);                                            \
        ForthObject__drop(b1);                                                                  \
        ForthObject__drop(b2);                                                                  \
        return Ok;                                                                              \
    }

// Math operations using macros
BINARY_MATH_OP(add, n1->num + n2->num)
BINARY_MATH_OP(sub, n1->num - n2->num)
BINARY_MATH_OP(mul, n1->num * n2->num)

ForthEvalResult builtin_div(ForthInterpreter *in)
{
    ForthObject *n1 = NULL, *n2 = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &n1, Number, &n2, Number);
    if (args_res != Ok)
        return args_res;

    if (n2->num == 0.0)
    {
        fprintf(stderr, "MathError: division by zero\n");
        ForthObject__drop(n1);
        ForthObject__drop(n2);

        return MathError;
    }

    ForthObject *res = ForthObject__new_number(n1->num / n2->num);
    ForthObject__list_push_move(in->stack, res);

    ForthObject__drop(n1);
    ForthObject__drop(n2);

    return Ok;
}

ForthEvalResult builtin_mod(ForthInterpreter *in)
{
    ForthObject *n1 = NULL, *n2 = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &n1, Number, &n2, Number);
    if (args_res != Ok)
        return args_res;

    if (n2->num == 0.0)
    {
        fprintf(stderr, "MathError: modulo by zero\n");
        ForthObject__drop(n1);
        ForthObject__drop(n2);

        return MathError;
    }

    ForthObject *res = ForthObject__new_number((size_t)n1->num % (size_t)n2->num);
    ForthObject__list_push_move(in->stack, res);

    ForthObject__drop(n1);
    ForthObject__drop(n2);

    return Ok;
}

BINARY_MATH_OP(max, (n1->num > n2->num) ? n1->num : n2->num)
BINARY_MATH_OP(min, (n1->num < n2->num) ? n1->num : n2->num)

// Comparison operations using macros
BINARY_COMPARISON_OP(gt, n1->num > n2->num)
BINARY_COMPARISON_OP(gte, n1->num >= n2->num)
BINARY_COMPARISON_OP(lt, n1->num < n2->num)
BINARY_COMPARISON_OP(lte, n1->num <= n2->num)

// Logic operations using macros
BINARY_LOGIC_OP(and, b1_val &&b2_val)
BINARY_LOGIC_OP(or, b1_val || b2_val)
BINARY_LOGIC_OP(xor, b1_val != b2_val)

ForthEvalResult builtin_not(ForthInterpreter *in)
{
    ForthObject *b = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &b, Number);
    if (args_res != Ok)
        return args_res;
    double result = (b->num == 0.0) ? 1.0 : 0.0;
    ForthObject *res = ForthObject__new_number(result);
    ForthObject__list_push_move(in->stack, res);
    ForthObject__drop(b);

    return Ok;
}

// Comparison
ForthEvalResult builtin_eq(ForthInterpreter *in)
{
    ForthObject *obj1 = NULL, *obj2 = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &obj1, Any, &obj2, Any);
    if (args_res != Ok)
        return args_res;

    bool equal = ForthObject__eq(obj1, obj2);
    ForthObject *res = ForthObject__new_number(equal ? 1.0 : 0.0);
    ForthObject__list_push_move(in->stack, res);

    ForthObject__drop(obj1);
    ForthObject__drop(obj2);

    return Ok;
}

ForthEvalResult builtin_neq(ForthInterpreter *in)
{
    builtin_eq(in);
    ForthObject *result = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &result, Number);
    if (args_res != Ok)
        return args_res;
    result->num = (result->num == 0.0) ? 1.0 : 0.0;
    ForthObject__list_push_move(in->stack, result);

    return Ok;
}

// Strings/Lists
ForthEvalResult builtin_contains(ForthInterpreter *in)
{
    ForthObject *needle = NULL, *haystack = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &needle, Any, &haystack, List | String | Symbol);
    if (args_res != Ok)
        return args_res;

    bool found = false;

    if (haystack->type == List)
    {
        for (size_t i = 0; i < haystack->list.len; i++)
        {
            ForthObject *item = haystack->list.data[i];
            if (ForthObject__eq(item, needle))
            {
                found = true;
                break;
            }
        }
    }
    else
    {
        if (needle->type != String)
        {
            fprintf(stderr, "TypeError: needle must be a string\n");

            ForthObject__drop(haystack);
            ForthObject__drop(needle);

            return TypeError;
        }

        // Use strstr for efficient substring search
        // Create null-terminated strings for strstr
        char *haystack_str = malloc(haystack->string.len + 1);
        char *needle_str = malloc(needle->string.len + 1);

        memcpy(haystack_str, haystack->string.chars, haystack->string.len);
        haystack_str[haystack->string.len] = '\0';

        memcpy(needle_str, needle->string.chars, needle->string.len);
        needle_str[needle->string.len] = '\0';

        found = (strstr(haystack_str, needle_str) != NULL);

        free(haystack_str);
        free(needle_str);
    }

    ForthObject *res = ForthObject__new_number(found ? 1.0 : 0.0);
    ForthObject__list_push_move(in->stack, res);

    ForthObject__drop(haystack);
    ForthObject__drop(needle);

    return Ok;
}

ForthEvalResult builtin_len(ForthInterpreter *in)
{
    ForthObject *container = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &container, List | String | Symbol);
    if (args_res != Ok)
        return args_res;

    size_t container_len = container->type == List ? container->list.len : container->string.len;
    ForthObject *res = ForthObject__new_number((double)container_len);
    ForthObject__list_push_move(in->stack, res);

    ForthObject__drop(container);

    return Ok;
}

ForthEvalResult builtin_at(ForthInterpreter *in)
{
    ForthObject *idx_obj = NULL, *container = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &idx_obj, Number, &container, List | String | Symbol);
    if (args_res != Ok)
        return args_res;

    long idx = (long)idx_obj->num;
    ForthObject *result = NULL;

    size_t container_len = container->type == List ? container->list.len : container->string.len;
    if (idx < 0 || (size_t)idx >= container_len)
    {
        fprintf(stderr, "IndexError: index %ld out of bounds for length: %zu\n",
                idx, container_len);

        ForthObject__drop(idx_obj);
        ForthObject__drop(container);

        return IndexError;
    }

    if (container->type == List)
        result = ForthObject__rc_clone(container->list.data[idx]);
    else
    {
        char single_char[2] = {container->string.chars[idx], '\0'};
        result = ForthObject__new_string(single_char, 1);
    }

    ForthObject__list_push_move(in->stack, result);
    ForthObject__drop(idx_obj);
    ForthObject__drop(container);

    return Ok;
}

// Control Flow
ForthEvalResult builtin_if(ForthInterpreter *in)
{
    ForthObject *true_branch = NULL, *condition = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &true_branch, List, &condition, Number);
    if (args_res != Ok)
        return args_res;

    ForthEvalResult res = Ok;
    if (condition->num != 0.0)
        res = ForthInterpreter__eval(in, true_branch);

    ForthObject__drop(condition);
    ForthObject__drop(true_branch);

    return res;
}

ForthEvalResult builtin_ifelse(ForthInterpreter *in)
{
    ForthObject *false_branch = NULL, *true_branch = NULL, *condition = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 3, &false_branch, List, &true_branch, List, &condition, Number);
    if (args_res != Ok)
        return args_res;

    ForthObject *branch = condition->num != 0.0 ? true_branch : false_branch;
    ForthEvalResult res = ForthInterpreter__eval(in, branch);

    ForthObject__drop(condition);
    ForthObject__drop(true_branch);
    ForthObject__drop(false_branch);

    return res;
}

ForthEvalResult builtin_while(ForthInterpreter *in)
{
    ForthObject *body = NULL, *condition = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &body, List, &condition, List);
    if (args_res != Ok)
        return args_res;

    ForthEvalResult res = Ok;
    while (true)
    {
        ForthInterpreter__eval(in, condition);

        ForthObject *result = NULL;
        args_res = ForthInterpreter__pop_args(in, 1, &result, Number);
        if (args_res != Ok)
        {
            res = args_res;
            break;
        }

        bool should_break = result->num == 0.0;
        ForthObject__drop(result);
        if (should_break)
            break;

        res = ForthInterpreter__eval(in, body);
    }

    ForthObject__drop(condition);
    ForthObject__drop(body);

    return res;
}

// Execution Stack
ForthEvalResult builtin_pop(ForthInterpreter *in)
{
    ForthObject *el = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &el, Any);
    if (args_res != Ok)
        return args_res;

    ForthObject__drop(el);

    return Ok;
}

ForthEvalResult builtin_dup(ForthInterpreter *in)
{
    ForthObject *el = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &el, Any);
    if (args_res != Ok)
        return args_res;

    ForthObject__list_push_copy(in->stack, el);
    ForthObject__list_push_move(in->stack, el);

    return Ok;
}

ForthEvalResult builtin_stack_len(ForthInterpreter *in)
{
    ForthObject *el = ForthObject__new_number((double)in->stack->list.len);
    ForthObject__list_push_move(in->stack, el);

    return Ok;
}

ForthEvalResult builtin_stack(ForthInterpreter *in)
{
    ForthObject *cloned = ForthObject__deep_clone(in->stack);
    ForthObject__list_push_move(in->stack, cloned);

    return Ok;
}

ForthEvalResult builtin_symbols(ForthInterpreter *in)
{
    ForthObject *res = ForthObject__new_list(in->symbols->len);
    for (size_t i = 0; i < in->symbols->len; i++)
    {
        size_t keylen = strlen(in->symbols->entries[i]->key);
        ForthObject *sym = ForthObject__new_symbol(in->symbols->entries[i]->key, keylen, true);
        ForthObject__list_push_move(res, sym);
    }

    ForthObject__list_push_move(in->stack, res);

    return Ok;
}

// Print
ForthEvalResult builtin_print_stack(ForthInterpreter *in)
{
    printf("Stack: ");
    ForthObject__print(in->stack);
    printf("\n");

    return Ok;
}

ForthEvalResult builtin_peek(ForthInterpreter *in)
{
    if (in->stack->list.len)
    {
        printf("Top of Stack: ");
        ForthObject *el = in->stack->list.data[in->stack->list.len - 1];
        ForthObject__print(el);
    }
    else
    {
        printf("Empty");
    }

    printf("\n");

    return Ok;
}

ForthEvalResult builtin_print_symbols(ForthInterpreter *in)
{
    printf("Symbols: ");
    for (size_t i = 0; i < in->symbols->len; i++)
        printf("%s ", in->symbols->entries[i]->key);

    printf("\n");

    return Ok;
}

// Eval
ForthEvalResult builtin_eval(ForthInterpreter *in)
{
    ForthObject *expr = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &expr, List | Symbol);
    if (args_res != Ok)
        return args_res;

    ForthInterpreter__eval(in, expr);
    ForthObject__drop(expr);

    return Ok;
}

ForthEvalResult builtin_quote(ForthInterpreter *in)
{
    ForthObject *string = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &string, String);
    if (args_res != Ok)
        return args_res;

    ForthObject *sym = ForthObject__new_symbol(string->string.chars, string->string.len, true);
    ForthObject__list_push_move(in->stack, sym);

    ForthObject__drop(string);

    return Ok;
}

ForthEvalResult builtin_unquote(ForthInterpreter *in)
{
    ForthObject *sym = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &sym, Symbol);
    if (args_res != Ok)
        return args_res;

    ForthInterpreter__eval(in, sym);
    ForthObject__drop(sym);

    return Ok;
}

// Define
ForthEvalResult builtin_define(ForthInterpreter *in)
{
    ForthObject *key = NULL, *val = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 2, &key, Symbol, &val, Any);
    if (args_res != Ok)
        return args_res;

    if (val->type == List)
        ForthInterpreter__register_closure(in, key->string.chars, val);
    else
        ForthInterpreter__register_literal(in, key->string.chars, val);

    ForthObject__drop(val);
    ForthObject__drop(key);

    return Ok;
}

ForthEvalResult builtin_undefine(ForthInterpreter *in)
{
    ForthObject *key = NULL;
    ForthEvalResult args_res = ForthInterpreter__pop_args(in, 1, &key, Symbol);
    if (args_res != Ok)
        return args_res;

    SymbolsTable__remove(in->symbols, key->string.chars);
    ForthObject__drop(key);

    return Ok;
}

// Initialization
void ForthInterpreter__load_builtins(ForthInterpreter *in)
{
    // Math
    ForthInterpreter__register_function(in, "add", builtin_add);
    ForthInterpreter__register_function(in, "sub", builtin_sub);
    ForthInterpreter__register_function(in, "mul", builtin_mul);
    ForthInterpreter__register_function(in, "div", builtin_div);
    ForthInterpreter__register_function(in, "mod", builtin_mod);
    ForthInterpreter__register_function(in, "max", builtin_max);
    ForthInterpreter__register_function(in, "min", builtin_min);

    // Logic
    ForthInterpreter__register_function(in, "and", builtin_and);
    ForthInterpreter__register_function(in, "not", builtin_not);
    ForthInterpreter__register_function(in, "or", builtin_or);
    ForthInterpreter__register_function(in, "xor", builtin_xor);

    // Comparison
    ForthInterpreter__register_function(in, "eq", builtin_eq);
    ForthInterpreter__register_function(in, "neq", builtin_neq);
    ForthInterpreter__register_function(in, "gt", builtin_gt);
    ForthInterpreter__register_function(in, "gte", builtin_gte);
    ForthInterpreter__register_function(in, "lt", builtin_lt);
    ForthInterpreter__register_function(in, "lte", builtin_lte);

    // Strings/Lists
    ForthInterpreter__register_function(in, "len", builtin_len);
    ForthInterpreter__register_function(in, "contains", builtin_contains);
    ForthInterpreter__register_function(in, "at", builtin_at);

    // Control Flow
    ForthInterpreter__register_function(in, "if", builtin_if);
    ForthInterpreter__register_function(in, "ifelse", builtin_ifelse);
    ForthInterpreter__register_function(in, "while", builtin_while);

    // Execution Stack
    ForthInterpreter__register_function(in, "pop", builtin_pop);
    ForthInterpreter__register_function(in, "dup", builtin_dup);
    ForthInterpreter__register_function(in, "stack", builtin_stack);
    ForthInterpreter__register_function(in, "symbols", builtin_symbols);
    ForthInterpreter__register_function(in, "stack_len", builtin_stack_len);

    // Print
    ForthInterpreter__register_function(in, "peek", builtin_peek);
    ForthInterpreter__register_function(in, "print_stack", builtin_print_stack);
    ForthInterpreter__register_function(in, "print_symbols", builtin_print_symbols);

    // Eval
    ForthInterpreter__register_function(in, "eval", builtin_eval);
    ForthInterpreter__register_function(in, "quote", builtin_quote);
    ForthInterpreter__register_function(in, "unquote", builtin_unquote);

    // Define
    ForthInterpreter__register_function(in, "define", builtin_define);
    ForthInterpreter__register_function(in, "undefine", builtin_undefine);
}
