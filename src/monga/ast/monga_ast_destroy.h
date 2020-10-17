#ifndef MONGA_AST_DESTROY_H
#define MONGA_AST_DESTROY_H

#include "monga_ast.h"

void monga_ast_program_destroy(struct monga_ast_program_t* ast);
void monga_ast_definition_destroy(struct monga_ast_definition_t* ast);
void monga_ast_def_variable_destroy(struct monga_ast_def_variable_t* ast);
void monga_ast_def_type_destroy(struct monga_ast_def_type_t* ast);
void monga_ast_def_function_destroy(struct monga_ast_def_function_t* ast);
void monga_ast_typedesc_destroy(struct monga_ast_typedesc_t* ast);
void monga_ast_field_destroy(struct monga_ast_field_t* ast);
void monga_ast_parameter_destroy(struct monga_ast_parameter_t* ast);
void monga_ast_block_destroy(struct monga_ast_block_t* ast);
void monga_ast_statement_destroy(struct monga_ast_statement_t* ast);
void monga_ast_variable_destroy(struct monga_ast_variable_t* ast);
void monga_ast_expression_destroy(struct monga_ast_expression_t* ast);
void monga_ast_condition_destroy(struct monga_ast_condition_t* ast);
void monga_ast_call_destroy(struct monga_ast_call_t* ast);

#endif