#ifndef MONGA_AST_BUILTIN_H
#define MONGA_AST_BUILTIN_H

#include "monga_ast.h"

void monga_ast_builtin_init();
void monga_ast_builtin_close();

const struct monga_ast_typedesc_t* monga_ast_builtin_typedesc(enum monga_ast_typedesc_builtin_t builtin);
const char* monga_ast_builtin_typedesc_id(enum monga_ast_typedesc_builtin_t builtin);

struct monga_ast_def_type_t* monga_ast_builtin_def_type(enum monga_ast_typedesc_builtin_t builtin);

#endif