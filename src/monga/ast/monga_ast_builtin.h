#ifndef MONGA_AST_BUILTIN_H
#define MONGA_AST_BUILTIN_H

#include "monga_ast.h"

void monga_ast_builtin_typedesc_check(enum monga_ast_typedesc_builtin_t builtin);
const char* monga_ast_builtin_typedesc_id(enum monga_ast_typedesc_builtin_t builtin);
struct monga_ast_typedesc_t* monga_ast_builtin_typedesc(enum monga_ast_typedesc_builtin_t builtin);
struct monga_ast_def_type_t* monga_ast_builtin_def_type(enum monga_ast_typedesc_builtin_t builtin);
bool monga_ast_builtin_castable(enum monga_ast_typedesc_builtin_t to, enum monga_ast_typedesc_builtin_t from);
bool monga_ast_builtin_visible(enum monga_ast_typedesc_builtin_t builtin);

const char* monga_ast_builtin_typedesc_llvm(enum monga_ast_typedesc_builtin_t builtin);
const char* monga_ast_builtin_typedesc_zero_llvm(enum monga_ast_typedesc_builtin_t builtin);
const char* monga_ast_builtin_llvm_add_instruction(enum monga_ast_typedesc_builtin_t builtin);
const char* monga_ast_builtin_llvm_sub_instruction(enum monga_ast_typedesc_builtin_t builtin);
const char* monga_ast_builtin_llvm_mul_instruction(enum monga_ast_typedesc_builtin_t builtin);
const char* monga_ast_builtin_llvm_div_instruction(enum monga_ast_typedesc_builtin_t builtin);
const char* monga_ast_builtin_llvm_cast_instruction(enum monga_ast_typedesc_builtin_t to, enum monga_ast_typedesc_builtin_t from);

#endif