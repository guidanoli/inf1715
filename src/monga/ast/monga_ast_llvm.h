#ifndef MONGA_AST_LLVM_H
#define MONGA_AST_LLVM_H

#include <stdio.h>

#include "monga_ast.h"

enum monga_ast_llvm_func_t {
    MONGA_AST_LLVM_FUNC_MALLOC = 1 << 0,
    MONGA_AST_LLVM_FUNC_PRINTF = 1 << 1,
    MONGA_AST_LLVM_FUNC_CNT, /* pseudo value */
};

enum monga_ast_llvm_printf_fmt_t {
    MONGA_AST_LLVM_PRINTF_FMT_INT = 1 << 0,
    MONGA_AST_LLVM_PRINTF_FMT_FLOAT = 1 << 1,
    MONGA_AST_LLVM_PRINTF_FMT_PTR = 1 << 2,
    MONGA_AST_LLVM_PRINTF_FMT_CNT, /* pseudo value */
};

struct monga_ast_llvm_context_t {
    FILE* output_file;
    struct monga_ast_def_function_t* def_function; /* nullable */
    size_t struct_count;
    size_t tempvar_count;
    size_t label_count;
    char referenced_funcs; /* enum monga_ast_llvm_func_t */
    char referenced_printf_fmts; /* enum monga_ast_llvm_printf_fmt_t */
};

void monga_ast_program_llvm(struct monga_ast_program_t* ast, FILE* fp);
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