#include "monga_ast_bind_stack.h"
#include "monga_ast_reference.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static struct monga_ast_bind_stack_name_t* monga_ast_bind_stack_get_name_in_current_block(struct monga_ast_bind_stack_t* stack, char* id);
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

struct monga_ast_bind_stack_name_t* monga_ast_bind_stack_get_name_in_current_block(struct monga_ast_bind_stack_t* stack, char* id)
{
    struct monga_ast_bind_stack_name_t* name, * last_name = NULL;
    last_name = stack->blocks ? stack->blocks->start : NULL;
    for (name = stack->names; name != last_name; name = name->next) {
        if (strcmp(name->reference->id, id) == 0) {
            return name;
        }
    }
    return NULL;
}

void monga_ast_bind_stack_insert_name(struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* reference)
{
    struct monga_ast_bind_stack_name_t* name;
    if (name = monga_ast_bind_stack_get_name_in_current_block(stack, reference->id)) {
        size_t defined_line = monga_ast_reference_line(name->reference);
        size_t redefined_line = monga_ast_reference_line(reference);
        const char* kind = monga_ast_reference_kind_name(name->reference->tag);
        fprintf(stderr, "Redefined %s \"%s\" at line %zu (previously defined at line %zu)\n",
            kind, reference->id, redefined_line, defined_line);
        exit(MONGA_ERR_REDECLARATION);
    }
    name = construct(bind_stack_name);
    name->next = stack->names;
    name->reference = reference;
    stack->names = name;
}

void monga_ast_bind_stack_get_name(struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* reference, size_t line)
{
    struct monga_ast_bind_stack_name_t* name;
    for (name = stack->names; name; name = name->next) {
        if (strcmp(name->reference->id, reference->id) == 0) {
            reference->tag = name->reference->tag;
            reference->u = name->reference->u;
            return;
        }
    }
    fprintf(stderr, "Undeclared name \"%s\" referenced at line %zu\n", reference->id, line);
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

    monga_free(name->reference);
    monga_free(name);

    monga_ast_bind_stack_name_destroy(next, sentinel);
}

void monga_ast_bind_stack_destroy(struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_block_destroy(stack->blocks);
    monga_ast_bind_stack_name_destroy(stack->names, NULL);
    monga_free(stack);
}