#ifndef MONGA_AST_PRINT_H
#define MONGA_AST_PRINT_H

#include "monga_ast.h"

void monga_ast_program_print(struct monga_ast_program_t* ast);
void monga_ast_definition_print(struct monga_ast_definition_t* ast, int identation);
void monga_ast_def_variable_print(struct monga_ast_def_variable_t* ast, int identation);
void monga_ast_def_type_print(struct monga_ast_def_type_t* ast, int identation);
void monga_ast_def_function_print(struct monga_ast_def_function_t* ast, int identation);
void monga_ast_typedesc_print(struct monga_ast_typedesc_t* ast, int identation);
void monga_ast_field_print(struct monga_ast_field_t* ast, int identation);
void monga_ast_parameter_print(struct monga_ast_parameter_t* ast, int identation);
void monga_ast_block_print(struct monga_ast_block_t* ast, int identation);
void monga_ast_statement_print(struct monga_ast_statement_t* ast, int identation);
void monga_ast_variable_print(struct monga_ast_variable_t* ast, int identation);
void monga_ast_expression_print(struct monga_ast_expression_t* ast, int identation);
void monga_ast_condition_print(struct monga_ast_condition_t* ast, int identation);
void monga_ast_call_print(struct monga_ast_call_t* ast, int identation);

#endif