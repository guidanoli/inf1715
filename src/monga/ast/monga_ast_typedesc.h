#ifndef MONGA_AST_TYPEDESC_H
#define MONGA_AST_TYPEDESC_H

#include <stdio.h>

#include "monga_ast.h"

void monga_ast_typedesc_copy(const struct monga_ast_typedesc_t *orig, struct monga_ast_typedesc_t *dest);
void monga_ast_typedesc_make_array(struct monga_ast_typedesc_t *array_type, struct monga_ast_typedesc_t *typedesc);
void monga_ast_typedesc_write(FILE* f, struct monga_ast_typedesc_t* typedesc);

#endif