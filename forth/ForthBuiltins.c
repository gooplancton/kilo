#include "ForthBuiltins.h"
#include "ForthInterpreter.h"
#include "ForthObject.h"


// NOTE (IMPORTANT)
// FIXME: All of these functions can leak memory if an arity error happens after a pop
// pop arg1 -> ok
// pop arg2 -> NULL => raise arity error => arg1 is never freed (arg2 is freed in ForthObject__pop_arg)
// To fix this, some notion of cleanup function must be introduced

// Macros for common patterns
#define POP_ARG(in, arg_name) \
ForthObject *arg_name = ForthInterpreter__pop_arg(in); \
if (!arg_name) return ArityError;

#define POP_ARG_TYPED(in, arg_name, type) \
ForthObject *arg_name = ForthInterpreter__pop_arg_typed(in, type); \
if (!arg_name) return ArityError;


#define BINARY_MATH_OP(op_name, operation) \
ForthEvalResult builtin_##op_name(ForthInterpreter *in) { \
    POP_ARG_TYPED(in, n2, Number); \
    POP_ARG_TYPED(in, n1, Number); \
    ForthObject *res = ForthObject__new_number(operation); \
    ForthObject__list_push_move(in->stack, res); \
    ForthObject__drop(n1); \
    ForthObject__drop(n2); \
    return Ok; \
}

#define BINARY_COMPARISON_OP(op_name, comparison) \
ForthEvalResult builtin_##op_name(ForthInterpreter *in) { \
    POP_ARG_TYPED(in, n2, Number); \
    POP_ARG_TYPED(in, n1, Number); \
    ForthObject *res = ForthObject__new_number((comparison) ? 1.0 : 0.0); \
    ForthObject__list_push_move(in->stack, res); \
    ForthObject__drop(n1); \
    ForthObject__drop(n2); \
    return Ok; \
}

#define BINARY_LOGIC_OP(op_name, operation) \
ForthEvalResult builtin_##op_name(ForthInterpreter *in) { \
    POP_ARG_TYPED(in, b2, Number); \
    POP_ARG_TYPED(in, b1, Number); \
    bool b1_val = b1->num != 0.0; \
    bool b2_val = b2->num != 0.0; \
    double result = (operation) ? 1.0 : 0.0; \
    ForthObject *res = ForthObject__new_number(result); \
    ForthObject__list_push_move(in->stack, res); \
    ForthObject__drop(b1); \
    ForthObject__drop(b2); \
    return Ok; \
}

// Math operations using macros
BINARY_MATH_OP(add, n1->num + n2->num)
BINARY_MATH_OP(sub, n1->num - n2->num)
BINARY_MATH_OP(mul, n1->num * n2->num)

ForthEvalResult builtin_div(ForthInterpreter *in)
{
    POP_ARG_TYPED(in, n2, Number)
    POP_ARG_TYPED(in, n1, Number)

    if (n2->num == 0.0) {
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
    ForthObject *n2 = ForthInterpreter__pop_arg_typed(in, Number);
    ForthObject *n1 = ForthInterpreter__pop_arg_typed(in, Number);

    if (n2->num == 0.0) {
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
BINARY_LOGIC_OP(and, b1_val && b2_val)
BINARY_LOGIC_OP(or, b1_val || b2_val)
BINARY_LOGIC_OP(xor, b1_val != b2_val)

ForthEvalResult builtin_not(ForthInterpreter *in)
{
    POP_ARG_TYPED(in, b, Number)
    double result = (b->num == 0.0) ? 1.0 : 0.0;
    ForthObject *res = ForthObject__new_number(result);
    ForthObject__list_push_move(in->stack, res);
    ForthObject__drop(b);

    return Ok;
}

// Comparison
ForthEvalResult builtin_eq(ForthInterpreter *in)
{
    POP_ARG(in, obj2)
    POP_ARG(in, obj1)

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
    POP_ARG_TYPED(in, result, Number)
    result->num = (result->num == 0.0) ? 1.0 : 0.0;
    ForthObject__list_push_move(in->stack, result);

    return Ok;
}

// Strings/Lists
ForthEvalResult builtin_contains(ForthInterpreter *in)
{
    POP_ARG(in, needle)
    POP_ARG(in, haystack)

    bool found = false;

    if (haystack->type == List) {
        for (size_t i = 0; i < haystack->list.len; i++) {
            ForthObject *item = haystack->list.data[i];
            // Use proper equality comparison instead of pointer comparison
            if (ForthObject__eq(item, needle)) {
                found = true;
                break;
            }
        }
    } else if (haystack->type == String || haystack->type == Symbol) {
        if (needle->type != String) {
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
    } else {
        fprintf(stderr, "TypeError: container must be a string or a list\n");

        return TypeError;
    }

    ForthObject *res = ForthObject__new_number(found ? 1.0 : 0.0);
    ForthObject__list_push_move(in->stack, res);

    ForthObject__drop(haystack);
    ForthObject__drop(needle);

    return Ok;
}

ForthEvalResult builtin_at(ForthInterpreter *in) {
    POP_ARG_TYPED(in, idx_obj, Number)
    POP_ARG(in, container)
    
    long idx = (long)idx_obj->num;
    ForthObject *result = NULL;

    switch (container->type) {
        case String:
        case Symbol: {
            if (idx < 0 || (size_t)idx >= container->string.len) {
                fprintf(stderr, "IndexError: string index %ld out of bounds (length: %zu)\n", 
                       idx, container->string.len);

                ForthObject__drop(idx_obj);
                ForthObject__drop(container);

                return IndexError;
            }
            // Return the character as a single-character string
            char single_char[2] = {container->string.chars[idx], '\0'};
            result = ForthObject__new_string(single_char, 1);
            break;
        }
        
        case List: {
            if (idx < 0 || (size_t)idx >= container->list.len) {
                fprintf(stderr, "IndexError: list index %ld out of bounds (length: %zu)\n", 
                       idx, container->list.len);

                ForthObject__drop(idx_obj);
                ForthObject__drop(container);

                return IndexError;
            }
            // Return a reference-counted clone of the element
            result = ForthObject__rc_clone(container->list.data[idx]);
            break;
        }
        
        case Number:
            fprintf(stderr, "TypeError: number is not indexable\n");

            ForthObject__drop(idx_obj);
            ForthObject__drop(container);

            return TypeError;
    }

    ForthObject__list_push_move(in->stack, result);
    ForthObject__drop(idx_obj);
    ForthObject__drop(container);

    return Ok;
}

// Control Flow
ForthEvalResult builtin_if(ForthInterpreter *in)
{
    POP_ARG_TYPED(in, true_branch, List)
    POP_ARG_TYPED(in, condition, Number)

    ForthEvalResult res = Ok;
    if (condition->num != 0.0) {
        res = ForthInterpreter__eval(in, true_branch);
    }

    ForthObject__drop(condition);
    ForthObject__drop(true_branch);

    return res;
}

ForthEvalResult builtin_ifelse(ForthInterpreter *in)
{
    POP_ARG_TYPED(in, false_branch, List)
    POP_ARG_TYPED(in, true_branch, List)
    POP_ARG_TYPED(in, condition, Number)

    ForthEvalResult res = Ok;
    if (condition->num != 0.0) {
        res = ForthInterpreter__eval(in, true_branch);
    } else {
        res = ForthInterpreter__eval(in, false_branch);
    }

    ForthObject__drop(condition);
    ForthObject__drop(true_branch);
    ForthObject__drop(false_branch);

    return res;
}

ForthEvalResult builtin_while(ForthInterpreter *in)
{
    POP_ARG_TYPED(in, body, List)
    POP_ARG_TYPED(in, condition, List)

    ForthEvalResult res = Ok;
    while (true) {
        ForthInterpreter__eval(in, condition);
        ForthObject *result = ForthInterpreter__pop_arg_typed(in, Number);
        if (!result) {
            fprintf(stderr, "ArityError: stack underflow in loop execution\n");
            ForthObject__drop(condition);
            ForthObject__drop(body);

            return ArityError;
        }
        
        if (result->num == 0.0) {
            ForthObject__drop(result);
            break;
        }
        ForthObject__drop(result);
        
        res = ForthInterpreter__eval(in, body);
    }

    ForthObject__drop(condition);
    ForthObject__drop(body);

    return res;
}

// Execution Stack
ForthEvalResult builtin_pop(ForthInterpreter *in)
{
    POP_ARG(in, el)
    ForthObject__drop(el);

    return Ok;
}

ForthEvalResult builtin_dup(ForthInterpreter *in)
{
    POP_ARG(in, el)
    ForthObject__list_push_copy(in->stack, el);
    ForthObject__list_push_move(in->stack, el);

    return Ok;
}


// Print
ForthEvalResult builtin_print(ForthInterpreter *in)
{
    ForthObject__print(in->stack);
    printf("\n");

    return Ok;
}

ForthEvalResult builtin_peek(ForthInterpreter *in)
{
    if (in->stack->list.len) {
        ForthObject *el = in->stack->list.data[in->stack->list.len - 1];
        ForthObject__print(el);
        printf("\n");
    }

    return Ok;
}

ForthEvalResult builtin_print_symbols(ForthInterpreter *in)
{
    for (size_t i = 0; i < in->symbols->len; i++)
    {
        printf("%s ", in->symbols->entries[i]->key);
    }
    printf("\n");

    return Ok;
}

// Eval
ForthEvalResult builtin_eval(ForthInterpreter *in)
{
    POP_ARG_TYPED(in, expr, List)
    ForthInterpreter__eval(in, expr);
    ForthObject__drop(expr);

    return Ok;
}

// Define
ForthEvalResult builtin_define(ForthInterpreter *in)
{
    POP_ARG(in, val)
    POP_ARG_TYPED(in, key, Symbol)

    if (!key->string.quoted)
    {
        fprintf(stderr, "TypeError: unexpected unquoted symbol\n");
        return TypeError;
    }

    if (val->type == List)
        ForthInterpreter__register_closure(in, key->string.chars, val);
    else
        ForthInterpreter__register_literal(in, key->string.chars, val);

    ForthObject__drop(val);
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
    ForthInterpreter__register_function(in, "contains", builtin_contains);
    ForthInterpreter__register_function(in, "at", builtin_at);

    // Control Flow
    ForthInterpreter__register_function(in, "if", builtin_if);
    ForthInterpreter__register_function(in, "ifelse", builtin_ifelse);
    ForthInterpreter__register_function(in, "while", builtin_while);

    // Execution Stack
    ForthInterpreter__register_function(in, "pop", builtin_pop);
    ForthInterpreter__register_function(in, "dup", builtin_dup);

    // Print
    ForthInterpreter__register_function(in, "print", builtin_print);
    ForthInterpreter__register_function(in, "peek", builtin_peek);
    ForthInterpreter__register_function(in, "symbols", builtin_print_symbols);

    // Eval
    ForthInterpreter__register_function(in, "eval", builtin_eval);

    // Define
    ForthInterpreter__register_function(in, "define", builtin_define);
}
