#ifndef MONGA_AST_TYPEDESC_H
#define MONGA_AST_TYPEDESC_H

#include <stdio.h>
#include <stdbool.h>

#include "monga_ast.h"

struct monga_ast_typedesc_t* monga_ast_typedesc_resolve_id(struct monga_ast_typedesc_t *typedesc);

void monga_ast_typedesc_write(FILE* f, struct monga_ast_typedesc_t* typedesc);
void monga_ast_typedesc_check_self_reference(struct monga_ast_typedesc_t* typedesc);

struct monga_ast_typedesc_t* monga_ast_typedesc_parent(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2);
bool monga_ast_typedesc_numeric(struct monga_ast_typedesc_t* typedesc);
bool monga_ast_typedesc_castable(struct monga_ast_typedesc_t *totypedesc, struct monga_ast_typedesc_t *fromtypedesc);
bool monga_ast_typedesc_assignable(struct monga_ast_typedesc_t *vartypedesc, struct monga_ast_typedesc_t *exptypedesc);
bool monga_ast_typedesc_equal(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2);
bool monga_ast_typedesc_sibling(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2);

#endif