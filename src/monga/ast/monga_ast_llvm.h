#ifndef MONGA_AST_LLVM_H
#define MONGA_AST_LLVM_H

#include "monga_ast.h"

void monga_ast_program_llvm(struct monga_ast_program_t* ast);
void monga_ast_definition_llvm(struct monga_ast_definition_t* ast, size_t struct_count);
void monga_ast_def_variable_llvm(struct monga_ast_def_variable_t* ast);
void monga_ast_def_type_llvm(struct monga_ast_def_type_t* ast, size_t* struct_count_ptr);
void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast);
void monga_ast_typedesc_llvm(struct monga_ast_typedesc_t* ast, size_t* struct_count_ptr);
void monga_ast_field_llvm(struct monga_ast_field_t* ast);
void monga_ast_parameter_llvm(struct monga_ast_parameter_t* ast);
void monga_ast_block_llvm(struct monga_ast_block_t* ast, size_t* var_count_ptr, struct monga_ast_typedesc_t* ret_typedesc);
void monga_ast_statement_llvm(struct monga_ast_statement_t* ast, size_t* var_count_ptr, struct monga_ast_typedesc_t* ret_typedesc);
void monga_ast_variable_llvm(struct monga_ast_variable_t* ast, size_t* var_count_ptr);
void monga_ast_expression_llvm(struct monga_ast_expression_t* ast, size_t* var_count_ptr);
void monga_ast_condition_llvm(struct monga_ast_condition_t* ast);
void monga_ast_call_llvm(struct monga_ast_call_t* ast);
void monga_ast_reference_llvm(struct monga_ast_reference_t* ast);

#endif