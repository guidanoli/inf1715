#ifndef MONGA_AST_BIND_H
#define MONGA_AST_BIND_H

#include "monga_ast.h"
#include "monga_ast_bind_stack.h"

void monga_ast_program_bind(struct monga_ast_program_t* ast);
void monga_ast_definition_bind(struct monga_ast_definition_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_def_variable_bind(struct monga_ast_def_variable_t* ast, struct monga_ast_bind_stack_t* stack, bool is_global);
void monga_ast_def_type_bind(struct monga_ast_def_type_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_def_function_bind(struct monga_ast_def_function_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_typedesc_bind(struct monga_ast_typedesc_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_field_bind(struct monga_ast_field_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_parameter_bind(struct monga_ast_parameter_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_block_bind(struct monga_ast_block_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_statement_bind(struct monga_ast_statement_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_variable_bind(struct monga_ast_variable_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_expression_bind(struct monga_ast_expression_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_condition_bind(struct monga_ast_condition_t* ast, struct monga_ast_bind_stack_t* stack);
void monga_ast_call_bind(struct monga_ast_call_t* ast, struct monga_ast_bind_stack_t* stack);

#endif