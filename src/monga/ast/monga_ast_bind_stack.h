#ifndef MONGA_AST_BIND_STACK_H
#define MONGA_AST_BIND_STACK_H

#include "monga_ast.h"

struct monga_ast_bind_stack_name_t {
    struct monga_ast_reference_t reference;
    struct monga_ast_bind_stack_name_t* next; /* nullable */
};

struct monga_ast_bind_stack_block_t {
    struct monga_ast_bind_stack_block_t* next; /* nullable */
    struct monga_ast_bind_stack_name_t* start; /* nullable */
};

struct monga_ast_bind_stack_t {
    struct monga_ast_bind_stack_block_t* blocks; /* nullable */
    struct monga_ast_bind_stack_name_t* names; /* nullable */
};

struct monga_ast_bind_stack_t* monga_ast_bind_stack_create();
void monga_ast_bind_stack_block_enter(struct monga_ast_bind_stack_t* stack);
void monga_ast_bind_stack_block_exit(struct monga_ast_bind_stack_t* stack);
void monga_ast_bind_stack_insert_name(struct monga_ast_bind_stack_t* stack, char* id, enum monga_ast_reference_tag_t tag, void* definition);
void monga_ast_bind_stack_get_typed_name(struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* reference, int n, ...);
void monga_ast_bind_stack_get_name(struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* reference);
void monga_ast_bind_stack_destroy(struct monga_ast_bind_stack_t* stack);

#endif