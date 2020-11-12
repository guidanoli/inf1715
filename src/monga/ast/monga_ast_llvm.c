#include "monga_ast_llvm.h"
#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"

#include <stdio.h>

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

static void monga_ast_def_type_reference_llvm(struct monga_ast_def_type_t* ast)
{
    struct monga_ast_typedesc_t* typedesc = ast->typedesc;
    typedesc = monga_ast_typedesc_resolve_id(typedesc);
    switch (typedesc->tag) {
    case MONGA_AST_TYPEDESC_BUILTIN:
        printf("%s", monga_ast_builtin_typedesc_llvm(typedesc->u.builtin_typedesc));
        break;
    case MONGA_AST_TYPEDESC_ID:
        monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
        monga_ast_def_type_reference_llvm(typedesc->u.array_typedesc->annonymous_def_type);
        putc('*', stdout);
        break;
    case MONGA_AST_TYPEDESC_RECORD:
        printf("%%S%zu*", ast->llvm_id);
        break;
    default:
        monga_unreachable();
    }
}

void monga_ast_def_type_llvm(struct monga_ast_def_type_t* ast, size_t* struct_count_ptr)
{
    switch (ast->typedesc->tag) {
    case MONGA_AST_TYPEDESC_ID:
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
    {
        struct monga_ast_typedesc_t* subtypedesc = ast->typedesc->u.array_typedesc;
        monga_ast_def_type_llvm(subtypedesc->annonymous_def_type, struct_count_ptr);
        break;
    }
    case MONGA_AST_TYPEDESC_RECORD:
    {
        struct monga_ast_field_list_t* fieldlist;
        ast->llvm_id = *struct_count_ptr;
        printf("%%S%zu = type { ", ast->llvm_id);
        fieldlist = ast->typedesc->u.record_typedesc;
        for (struct monga_ast_field_t* field = fieldlist->first; field; field = field->next) {
            monga_ast_def_type_reference_llvm(field->type.u.def_type);
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

void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast) { (void) ast; }

// void monga_ast_field_llvm(struct monga_ast_field_t* ast) {}

// void monga_ast_parameter_llvm(struct monga_ast_parameter_t* ast) {}

// void monga_ast_block_llvm(struct monga_ast_block_t* ast) {}

// void monga_ast_statement_llvm(struct monga_ast_statement_t* ast) {}

// void monga_ast_variable_llvm(struct monga_ast_variable_t* ast) {}

// void monga_ast_expression_llvm(struct monga_ast_expression_t* ast) {}

// void monga_ast_condition_llvm(struct monga_ast_condition_t* ast) {}

// void monga_ast_call_llvm(struct monga_ast_call_t* ast) {}

// void monga_ast_reference_llvm(struct monga_ast_reference_t* ast) {}
