#include "monga_ast_llvm.h"
#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"

#include <stdio.h>

static void monga_ast_typedesc_reference_llvm(struct monga_ast_typedesc_t* ast);

void monga_ast_program_llvm(struct monga_ast_program_t* ast)
{
    if (ast->definitions)
        monga_ast_definition_llvm(ast->definitions->first, 0);
}

void monga_ast_definition_llvm(struct monga_ast_definition_t* ast, size_t struct_count)
{
    switch (ast->tag) {
        case MONGA_AST_DEFINITION_VARIABLE:
            monga_ast_def_variable_llvm(ast->u.def_variable);
            break;
        case MONGA_AST_DEFINITION_TYPE:
            monga_ast_def_type_llvm(ast->u.def_type, &struct_count);
            break;
        case MONGA_AST_DEFINITION_FUNCTION:
            monga_ast_def_function_llvm(ast->u.def_function);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_definition_llvm(ast->next, struct_count);
}

void monga_ast_def_variable_llvm(struct monga_ast_def_variable_t* ast) { (void) ast; }

void monga_ast_typedesc_reference_llvm(struct monga_ast_typedesc_t* ast)
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
        monga_ast_typedesc_reference_llvm(ast->u.array_typedesc);
        putc('*', stdout);
        break;
    case MONGA_AST_TYPEDESC_RECORD:
        printf("%%S%zu*", ast->u.record_typedesc.llvm_struct_id);
        break;
    default:
        monga_unreachable();
    }
}

void monga_ast_def_type_llvm(struct monga_ast_def_type_t* ast, size_t* struct_count_ptr)
{
    monga_ast_typedesc_llvm(ast->typedesc, struct_count_ptr);
}

void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast) { (void) ast; }

void monga_ast_typedesc_llvm(struct monga_ast_typedesc_t* ast, size_t* struct_count_ptr)
{
    switch (ast->tag) {
    case MONGA_AST_TYPEDESC_ID:
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
        monga_ast_typedesc_llvm(ast->u.array_typedesc, struct_count_ptr);
        break;
    case MONGA_AST_TYPEDESC_RECORD:
    {
        struct monga_ast_field_list_t* field_list;
        ast->u.record_typedesc.llvm_struct_id = *struct_count_ptr;
        printf("%%S%zu = type { ", ast->u.record_typedesc.llvm_struct_id);
        field_list = ast->u.record_typedesc.field_list;
        for (struct monga_ast_field_t* field = field_list->first; field; field = field->next) {
            monga_ast_typedesc_reference_llvm(field->type.u.def_type->typedesc);
            if (field->next)
                printf(", ");
        }
        printf(" }\n");
        *struct_count_ptr += 1;
        break;
    }
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
