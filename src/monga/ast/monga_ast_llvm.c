#include "monga_ast_llvm.h"
#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"

#include <stdio.h>

void monga_ast_program_llvm(struct monga_ast_program_t* ast)
{
    if (ast->definitions)
        monga_ast_definition_llvm(ast->definitions->first);
}

void monga_ast_definition_llvm(struct monga_ast_definition_t* ast)
{
    switch (ast->tag) {
        case MONGA_AST_DEFINITION_VARIABLE:
            monga_ast_def_variable_llvm(ast->u.def_variable);
            break;
        case MONGA_AST_DEFINITION_TYPE:
            monga_ast_def_type_llvm(ast->u.def_type);
            break;
        case MONGA_AST_DEFINITION_FUNCTION:
            monga_ast_def_function_llvm(ast->u.def_function);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_definition_llvm(ast->next);
}

void monga_ast_def_variable_llvm(struct monga_ast_def_variable_t* ast) { (void) ast; }

void monga_ast_def_type_llvm(struct monga_ast_def_type_t* ast) { (void) ast; }

void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast) { (void) ast; }

void monga_ast_typedesc_llvm(struct monga_ast_typedesc_t* ast)
{
    ast = monga_ast_typedesc_resolve_id(ast);
    switch (ast->tag) {
        case MONGA_AST_TYPEDESC_BUILTIN:
            printf("%s", monga_ast_builtin_typedesc_llvm(ast->u.builtin_typedesc));
            break;
        case MONGA_AST_TYPEDESC_ID:
            monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            monga_ast_typedesc_llvm(ast->u.array_typedesc);
            putc('*', stdout);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
            // monga_ast_field_destroy(ast->u.record_typedesc->first);
            // monga_free(ast->u.record_typedesc);
            break;
        default:
            monga_unreachable();
    }
}

// void monga_ast_field_llvm(struct monga_ast_field_t* ast) {}

// void monga_ast_parameter_llvm(struct monga_ast_parameter_t* ast) {}

// void monga_ast_block_llvm(struct monga_ast_block_t* ast) {}

// void monga_ast_statement_llvm(struct monga_ast_statement_t* ast) {}

// void monga_ast_variable_llvm(struct monga_ast_variable_t* ast) {}

// void monga_ast_expression_llvm(struct monga_ast_expression_t* ast) {}

// void monga_ast_condition_llvm(struct monga_ast_condition_t* ast) {}

// void monga_ast_call_llvm(struct monga_ast_call_t* ast) {}

// void monga_ast_reference_llvm(struct monga_ast_reference_t* ast) {}
