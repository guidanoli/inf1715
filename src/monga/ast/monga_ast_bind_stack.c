#include "monga_ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    if (stack->names != block->start) {
        monga_ast_bind_stack_name_destroy(stack->names, block->start);
        stack->names = block->start;
    }
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

void monga_ast_bind_stack_get_name(struct monga_ast_bind_stack_t* stack, char* id, enum monga_ast_reference_tag_t *tag_ptr, void** definition_ptr)
{
    struct monga_ast_bind_stack_name_t* name;
    for (name = stack->names; name; name = name->next) {
        if (strcmp(name->reference.id, id) == 0) {
            *tag_ptr = name->reference.tag;
            *definition_ptr = name->reference.generic;
            return;
        }
    }
    /* TODO: line number */
    fprintf(stderr, "Undeclared name \"%s\"\n", id);
    exit(MONGA_ERR_UNDECLARED);
}

void monga_ast_bind_stack_block_destroy(struct monga_ast_bind_stack_block_t* block)
{
    if (block->next)
        monga_ast_bind_stack_block_destroy(block->next);
    monga_free(block);
}

void monga_ast_bind_stack_name_destroy(struct monga_ast_bind_stack_name_t* name, struct monga_ast_bind_stack_name_t* sentinel)
{
    monga_free(name->reference.id);
    if (name->next != NULL && name->next != sentinel)
        monga_ast_bind_stack_name_destroy(name->next, sentinel);
    monga_free(name);
}

void monga_ast_bind_stack_destroy(struct monga_ast_bind_stack_t* stack)
{
    if (stack->blocks)
        monga_ast_bind_stack_block_destroy(stack->blocks);
    if (stack->names)
        monga_ast_bind_stack_name_destroy(stack->names, NULL);
    monga_free(stack);
}