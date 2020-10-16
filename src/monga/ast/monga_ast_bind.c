#include "monga_ast.h"

#include <stdlib.h>

#define sizeofv(v) (sizeof(v)/sizeof(*v))

/* Built-in types */

static struct monga_ast_typedesc_t monga_ast_builtin_int_typedesc = {
    MONGA_AST_TYPEDESC_BUILTIN, MONGA_AST_TYPEDESC_BUILTIN_INT };

static struct monga_ast_typedesc_t monga_ast_builtin_float_typedesc = {
    MONGA_AST_TYPEDESC_BUILTIN, MONGA_AST_TYPEDESC_BUILTIN_FLOAT };

static struct monga_ast_def_type_t monga_ast_builtin_def_types[] = {
    { "int", &monga_ast_builtin_int_typedesc },
    { "float", &monga_ast_builtin_float_typedesc },
};

/* Function definitions */

static void monga_ast_builtin_def_types_bind(struct monga_ast_bind_stack_t* stack)
{
    for (size_t i = 0; i < sizeofv(monga_ast_builtin_def_types); ++i) {
        struct monga_ast_def_type_t* def_type = &monga_ast_builtin_def_types[i];
        monga_ast_bind_stack_insert_name(stack, def_type->id, MONGA_AST_REFERENCE_TYPE, def_type);
    }
}

void monga_ast_program_bind(struct monga_ast_program_t* ast)
{
    if (ast->definitions) {
        struct monga_ast_bind_stack_t* stack = monga_ast_bind_stack_create();
        monga_ast_builtin_def_types_bind(stack);
        monga_ast_definition_bind(ast->definitions->first, stack);
        monga_ast_bind_stack_destroy(stack);
    }
}

void monga_ast_definition_bind(struct monga_ast_definition_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_DEFINITION_VARIABLE:
            monga_ast_def_variable_bind(ast->def_variable, stack);
            break;
        case MONGA_AST_DEFINITION_TYPE:
            monga_ast_def_type_bind(ast->def_type, stack);
            break;
        case MONGA_AST_DEFINITION_FUNCTION:
            monga_ast_def_function_bind(ast->def_function, stack);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_definition_bind(ast->next, stack);
}

void monga_ast_def_variable_bind(struct monga_ast_def_variable_t* ast, struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_insert_name(stack, ast->id, MONGA_AST_REFERENCE_VARIABLE, ast);
    monga_ast_bind_stack_get_name(stack, ast->type.id, &ast->type.tag, &ast->type.generic);
    /* TODO: check if kind of reference matches expected kind */
    if (ast->next)
        monga_ast_def_variable_destroy(ast->next);
}

void monga_ast_def_type_bind(struct monga_ast_def_type_t* ast, struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_insert_name(stack, ast->id, MONGA_AST_REFERENCE_TYPE, ast);
    monga_ast_typedesc_bind(ast->typedesc, stack);
}

void monga_ast_def_function_bind(struct monga_ast_def_function_t* ast, struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_insert_name(stack, ast->id, MONGA_AST_REFERENCE_FUNCTION, ast);
    if (ast->type.id)
        monga_ast_bind_stack_get_name(stack, ast->type.id, &ast->type.tag, &ast->type.generic);
    monga_ast_bind_stack_block_enter(stack);
    if (ast->parameters)
        monga_ast_parameter_bind(ast->parameters->first, stack);
    monga_ast_block_bind(ast->block, stack);
    monga_ast_bind_stack_block_exit(stack);
}

void monga_ast_typedesc_bind(struct monga_ast_typedesc_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_TYPEDESC_ID:
            monga_ast_bind_stack_get_name(stack, ast->id_typedesc.id, &ast->id_typedesc.tag, &ast->id_typedesc.generic);
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            monga_ast_typedesc_bind(ast->array_typedesc, stack);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
            /* TODO: test adding the same identifier in different levels of a record */
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_field_bind(ast->record_typedesc->first, stack);
            monga_ast_bind_stack_block_exit(stack);
            break;
        default:
            monga_unreachable();
    }
}

void monga_ast_field_bind(struct monga_ast_field_t* ast, struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_get_name(stack, ast->type.id, &ast->type.tag, &ast->type.generic);
    if (ast->next)
        monga_ast_field_bind(ast->next, stack);
}

void monga_ast_parameter_bind(struct monga_ast_parameter_t* ast, struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_insert_name(stack, ast->id, MONGA_AST_REFERENCE_PARAMETER, ast);
    monga_ast_bind_stack_get_name(stack, ast->type.id, &ast->type.tag, &ast->type.generic);
    if (ast->next)
        monga_ast_parameter_bind(ast->next, stack);
}

void monga_ast_block_bind(struct monga_ast_block_t* ast, struct monga_ast_bind_stack_t* stack)
{
    if (ast->variables)
        monga_ast_def_variable_bind(ast->variables->first, stack);
    if (ast->statements)
        monga_ast_statement_bind(ast->statements->first, stack);
}

void monga_ast_statement_bind(struct monga_ast_statement_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_STATEMENT_IF:
            monga_ast_condition_bind(ast->if_stmt.cond, stack);
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_block_bind(ast->if_stmt.then_block, stack);
            monga_ast_bind_stack_block_exit(stack);
            if (ast->if_stmt.else_block) {
                monga_ast_bind_stack_block_enter(stack);
                monga_ast_block_bind(ast->if_stmt.else_block, stack);
                monga_ast_bind_stack_block_exit(stack);
            }
            break;
        case MONGA_AST_STATEMENT_WHILE:
            monga_ast_condition_bind(ast->while_stmt.cond, stack);
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_block_bind(ast->while_stmt.loop, stack);
            monga_ast_bind_stack_block_exit(stack);
            break;
        case MONGA_AST_STATEMENT_ASSIGN:
            monga_ast_variable_bind(ast->assign_stmt.var, stack);
            monga_ast_expression_bind(ast->assign_stmt.exp, stack);
            break;
        case MONGA_AST_STATEMENT_RETURN:
            if (ast->return_stmt.exp)
                monga_ast_expression_bind(ast->return_stmt.exp, stack);
            break;
        case MONGA_AST_STATEMENT_CALL:
            monga_ast_call_bind(ast->call_stmt.call, stack);
            break;
        case MONGA_AST_STATEMENT_PRINT:
            monga_ast_expression_bind(ast->print_stmt.exp, stack);
            break;
        case MONGA_AST_STATEMENT_BLOCK:
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_block_bind(ast->block_stmt.block, stack);
            monga_ast_bind_stack_block_exit(stack);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_statement_bind(ast->next, stack);
}

void monga_ast_variable_bind(struct monga_ast_variable_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_VARIABLE_ID:
            monga_ast_bind_stack_get_name(stack, ast->id_var.id, &ast->id_var.tag, &ast->id_var.generic);
            break;
        case MONGA_AST_VARIABLE_ARRAY:
            monga_ast_expression_bind(ast->array_var.array, stack);
            monga_ast_expression_bind(ast->array_var.index, stack);
            break;
        case MONGA_AST_VARIABLE_RECORD:
            monga_ast_expression_bind(ast->record_var.record, stack);
            /* How to set ast->record_var.field reference?
               1- Get the type of ast->record_var.record (expression)
               2- Make sure it's of record type
               3- Iterate through the field list until matching id is found
               4- Link to matching field
            */
            break;
        default:
            monga_unreachable();
    }
}

void monga_ast_expression_bind(struct monga_ast_expression_t* ast, struct monga_ast_bind_stack_t* stack) {}
void monga_ast_condition_bind(struct monga_ast_condition_t* ast, struct monga_ast_bind_stack_t* stack) {}
void monga_ast_call_bind(struct monga_ast_call_t* ast, struct monga_ast_bind_stack_t* stack) {}