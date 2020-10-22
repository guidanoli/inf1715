#include "monga_ast_bind_stack.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static void monga_ast_bind_stack_name_destroy(struct monga_ast_bind_stack_name_t* name, struct monga_ast_bind_stack_name_t* sentinel);
static void monga_ast_bind_stack_block_destroy(struct monga_ast_bind_stack_block_t* block);

struct monga_ast_bind_stack_t* monga_ast_bind_stack_create()
{
    struct monga_ast_bind_stack_t* stack = construct(bind_stack);
    stack->blocks = NULL;
    stack->names = NULL;
    return stack;
}

void monga_ast_bind_stack_block_enter(struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_bind_stack_block_t* block = construct(bind_stack_block);
    block->start = stack->names;
    block->next = stack->blocks;
    stack->blocks = block;
}

void monga_ast_bind_stack_block_exit(struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_bind_stack_block_t* block = stack->blocks;
    if (block == NULL)
        monga_unreachable();
    stack->blocks = block->next;
    monga_ast_bind_stack_name_destroy(stack->names, block->start);
    stack->names = block->start;
    monga_free(block);
}

bool monga_ast_bind_stack_check_name_in_current_block(struct monga_ast_bind_stack_t* stack, char* id)
{
    struct monga_ast_bind_stack_name_t* name, * last_name = NULL;
    last_name = stack->blocks ? stack->blocks->start : NULL;
    for (name = stack->names; name != last_name; name = name->next) {
        if (strcmp(name->reference.id, id) == 0) {
            return true;
        }
    }
    return false;
}

void monga_ast_bind_stack_insert_name(struct monga_ast_bind_stack_t* stack, char* id, enum monga_ast_reference_tag_t tag, void* definition)
{
    struct monga_ast_bind_stack_name_t* name;
    if (monga_ast_bind_stack_check_name_in_current_block(stack, id)) {
        /* TODO: line number, type of declaration, previously declared name */
        fprintf(stderr, "Redeclaration of \"%s\"\n", id);
        exit(MONGA_ERR_REDECLARATION);
    }
    name = construct(bind_stack_name);
    name->next = stack->names;
    name->reference.id = monga_memdup(id, strlen(id)+1);
    name->reference.tag = tag;
    name->reference.generic = definition;
    stack->names = name;
}

void monga_ast_bind_stack_get_typed_name(struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* reference, int n, ...)
{
    va_list va;
    bool matches_tag;
    enum monga_ast_reference_tag_t tag;
    monga_ast_bind_stack_get_name(stack, reference);
    va_start(va, n);
    matches_tag = false;
    for (int i = 0; i < n; ++i) {
        tag = va_arg(va, enum monga_ast_reference_tag_t);
        if (tag == reference->tag) {
            matches_tag = true;
            break;
        }
    }
    va_end(va);
    if (!matches_tag && n > 0) {
        fprintf(stderr, "Expected \"%s\" to be a reference", reference->id);
        va_start(va, n);
        for (int i = 0; i < n; ++i) {
            const char* reference_name;
            tag = va_arg(va, enum monga_ast_reference_tag_t);
            switch (tag) {
            case MONGA_AST_REFERENCE_VARIABLE:
                reference_name = "variable";
                break;
            case MONGA_AST_REFERENCE_TYPE:
                reference_name = "type";
                break;
            case MONGA_AST_REFERENCE_FUNCTION:
                reference_name = "function";
                break;
            case MONGA_AST_REFERENCE_PARAMETER:
                reference_name = "parameter";
                break;
            case MONGA_AST_REFERENCE_FIELD:
                reference_name = "field";
                break;
            default:
                monga_unreachable();
            }
            fprintf(stderr, " to a %s", reference_name);
            if (i < n-1)
                fprintf(stderr, " or");
        }
        fprintf(stderr, "\n");
        va_end(va);
        exit(MONGA_ERR_REFERENCE_KIND);
    }
}

void monga_ast_bind_stack_get_name(struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* reference)
{
    struct monga_ast_bind_stack_name_t* name;
    for (name = stack->names; name; name = name->next) {
        if (strcmp(name->reference.id, reference->id) == 0) {
            reference->tag = name->reference.tag;
            reference->generic = name->reference.generic;
            return;
        }
    }
    /* TODO: line number */
    fprintf(stderr, "Undeclared name \"%s\"\n", reference->id);
    exit(MONGA_ERR_UNDECLARED);
}

void monga_ast_bind_stack_block_destroy(struct monga_ast_bind_stack_block_t* block)
{
    struct monga_ast_bind_stack_block_t* next;

    if (block == NULL)
        return;

    next = block->next;

    monga_free(block);

    monga_ast_bind_stack_block_destroy(next);
}

void monga_ast_bind_stack_name_destroy(struct monga_ast_bind_stack_name_t* name, struct monga_ast_bind_stack_name_t* sentinel)
{
    struct monga_ast_bind_stack_name_t* next;

    if (name == NULL || name == sentinel)
        return;
    
    next = name->next;

    monga_free(name->reference.id);
    monga_free(name);

    monga_ast_bind_stack_name_destroy(next, sentinel);
}

void monga_ast_bind_stack_destroy(struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_block_destroy(stack->blocks);
    monga_ast_bind_stack_name_destroy(stack->names, NULL);
    monga_free(stack);
}