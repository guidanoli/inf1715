#ifndef MONGA_AST_LLVM_H
#define MONGA_AST_LLVM_H

#include "monga_ast.h"

struct monga_ast_llvm_context_t {
    struct monga_ast_def_function_t* def_function; /* nullable */
    size_t struct_count;
    size_t tempvar_count;
    size_t label_count;
    bool referenced_malloc;
    bool referenced_printf;
};

void monga_ast_program_llvm(struct monga_ast_program_t* ast);
void monga_ast_definition_llvm(struct monga_ast_definition_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_def_variable_llvm(struct monga_ast_def_variable_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_def_type_llvm(struct monga_ast_def_type_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_typedesc_llvm(struct monga_ast_typedesc_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_field_llvm(struct monga_ast_field_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_block_llvm(struct monga_ast_block_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_statement_llvm(struct monga_ast_statement_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_variable_llvm(struct monga_ast_variable_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_expression_llvm(struct monga_ast_expression_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_condition_llvm(struct monga_ast_condition_t* ast, struct monga_ast_llvm_context_t* ctx, size_t true_label, size_t false_label);
void monga_ast_call_llvm(struct monga_ast_call_t* ast, struct monga_ast_llvm_context_t* ctx);
void monga_ast_reference_llvm(struct monga_ast_reference_t* ast, struct monga_ast_llvm_context_t* ctx);

#endif