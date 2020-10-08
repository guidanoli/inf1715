#include "monga_ast.h"

#include <stdlib.h>

void monga_ast_program_destroy(struct monga_ast_program_t* ast)
{
    if (ast->definitions)
        monga_ast_definition_destroy(ast->definitions);
    monga_free(ast);
}

void monga_ast_definition_destroy(struct monga_ast_definition_t* ast)
{
    switch (ast->tag) {
        case MONGA_AST_DEFINITION_VARIABLE:
            monga_ast_def_variable_destroy(ast->def_variable);
            break;
        case MONGA_AST_DEFINITION_TYPE:
            monga_ast_def_type_destroy(ast->def_type);
            break;
        case MONGA_AST_DEFINITION_FUNCTION:
            monga_ast_def_function_destroy(ast->def_function);
            break;
    }
    if (ast->next)
        monga_ast_definition_destroy(ast->next);
    monga_free(ast);
}

void monga_ast_def_variable_destroy(struct monga_ast_def_variable_t* ast)
{
    monga_free(ast->id);
    monga_free(ast->type);
    if (ast->next)
        monga_ast_def_variable_destroy(ast->next);
    monga_free(ast);
}

void monga_ast_def_type_destroy(struct monga_ast_def_type_t* ast)
{
    monga_free(ast->id);
    monga_ast_typedesc_destroy(ast->typedesc);
    monga_free(ast);
}

void monga_ast_def_function_destroy(struct monga_ast_def_function_t* ast)
{
    monga_free(ast->id);
    if (ast->parameters)
        monga_ast_parameter_destroy(ast->parameters);
    if (ast->type)
        monga_free(ast->type);
    monga_ast_block_destroy(ast->block);
    monga_free(ast);
}

void monga_ast_typedesc_destroy(struct monga_ast_typedesc_t* ast)
{
    switch (ast->tag) {
        case MONGA_AST_TYPEDESC_ID:
            monga_free(ast->id_typedesc);
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            monga_ast_typedesc_destroy(ast->array_typedesc);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
            monga_ast_field_destroy(ast->record_typedesc);
            break;
    }
    monga_free(ast);
}

void monga_ast_field_destroy(struct monga_ast_field_t* ast)
{
    monga_free(ast->id);
    monga_free(ast->type);
    if (ast->next)
        monga_ast_field_destroy(ast->next);
    monga_free(ast);
}

void monga_ast_parameter_destroy(struct monga_ast_parameter_t* ast)
{
    monga_free(ast->id);
    monga_free(ast->type);
    if (ast->next)
        monga_ast_parameter_destroy(ast->next);
    monga_free(ast);
}

void monga_ast_block_destroy(struct monga_ast_block_t* ast)
{
    if (ast->variables)
        monga_ast_def_variable_destroy(ast->variables);
    if (ast->statements)
        monga_ast_statement_destroy(ast->statements);
    monga_free(ast);
}

void monga_ast_statement_destroy(struct monga_ast_statement_t* ast)
{
    switch (ast->tag) {
        case MONGA_AST_STATEMENT_IF:
            monga_ast_condition_destroy(ast->if_stmt.cond);
            monga_ast_block_destroy(ast->if_stmt.then_block);
            if (ast->if_stmt.else_block)
                monga_ast_block_destroy(ast->if_stmt.else_block);
            break;
        case MONGA_AST_STATEMENT_WHILE:
            monga_ast_condition_destroy(ast->while_stmt.cond);
            monga_ast_block_destroy(ast->while_stmt.loop);
            break;
        case MONGA_AST_STATEMENT_ASSIGN:
            monga_ast_variable_destroy(ast->assign_stmt.var);
            monga_ast_expression_destroy(ast->assign_stmt.exp);
            break;
        case MONGA_AST_STATEMENT_RETURN:
            if (ast->return_stmt.exp)
                monga_ast_expression_destroy(ast->return_stmt.exp);
            break;
        case MONGA_AST_STATEMENT_CALL:
            monga_ast_call_destroy(ast->call_stmt.call);
            break;
        case MONGA_AST_STATEMENT_PRINT:
            monga_ast_expression_destroy(ast->print_stmt.exp);
            break;
        case MONGA_AST_STATEMENT_BLOCK:
            monga_ast_block_destroy(ast->block_stmt.block);
            break;
    }
    if (ast->next)
        monga_ast_statement_destroy(ast->next);
    monga_free(ast);
}

void monga_ast_variable_destroy(struct monga_ast_variable_t* ast)
{
    switch (ast->tag) {
        case MONGA_AST_VARIABLE_ID:
            monga_free(ast->id_var.id);
            break;
        case MONGA_AST_VARIABLE_ARRAY:
            monga_ast_expression_destroy(ast->array_var.array);
            monga_ast_expression_destroy(ast->array_var.index);
            break;
        case MONGA_AST_VARIABLE_RECORD:
            monga_ast_expression_destroy(ast->record_var.record);
            monga_free(ast->record_var.field);
            break;
    }
    monga_free(ast);
}

void monga_ast_expression_destroy(struct monga_ast_expression_t* ast)
{
    switch (ast->tag) {
        case MONGA_AST_EXPRESSION_INTEGER:
            break;
        case MONGA_AST_EXPRESSION_REAL:
            break;
        case MONGA_AST_EXPRESSION_VAR:
            monga_ast_variable_destroy(ast->var_exp.var);
            break;
        case MONGA_AST_EXPRESSION_CALL:
            monga_ast_call_destroy(ast->call_exp.call);
            break;
        case MONGA_AST_EXPRESSION_CAST:
            monga_ast_expression_destroy(ast->cast_exp.exp);
            monga_free(ast->cast_exp.type);
            break;
        case MONGA_AST_EXPRESSION_NEW:
            monga_free(ast->new_exp.type);
            if (ast->new_exp.exp)
                monga_ast_expression_destroy(ast->new_exp.exp);
            break;
        case MONGA_AST_EXPRESSION_NEGATION:
            monga_ast_expression_destroy(ast->negation_exp.exp);
            break;
        case MONGA_AST_EXPRESSION_ADDITION:
            monga_ast_expression_destroy(ast->addition_exp.exp1);
            monga_ast_expression_destroy(ast->addition_exp.exp2);
            break;
        case MONGA_AST_EXPRESSION_SUBTRACTION:
            monga_ast_expression_destroy(ast->subtraction_exp.exp1);
            monga_ast_expression_destroy(ast->subtraction_exp.exp2);
            break;
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
            monga_ast_expression_destroy(ast->multiplication_exp.exp1);
            monga_ast_expression_destroy(ast->multiplication_exp.exp2);
            break;
        case MONGA_AST_EXPRESSION_DIVISION:
            monga_ast_expression_destroy(ast->division_exp.exp1);
            monga_ast_expression_destroy(ast->division_exp.exp2);
            break;
        case MONGA_AST_EXPRESSION_CONDITIONAL:
            monga_ast_condition_destroy(ast->conditional_exp.cond);
            monga_ast_expression_destroy(ast->conditional_exp.true_exp);
            monga_ast_expression_destroy(ast->conditional_exp.false_exp);
            break;
    }
    if (ast->next)
        monga_ast_expression_destroy(ast->next);
    monga_free(ast);
}

void monga_ast_condition_destroy(struct monga_ast_condition_t* ast)
{
    switch (ast->tag) {
        case MONGA_AST_CONDITION_EQ:
            monga_ast_expression_destroy(ast->eq_cond.exp1);
            monga_ast_expression_destroy(ast->eq_cond.exp2);
            break;
        case MONGA_AST_CONDITION_NE:
            monga_ast_expression_destroy(ast->ne_cond.exp1);
            monga_ast_expression_destroy(ast->ne_cond.exp2);
            break;
        case MONGA_AST_CONDITION_LE:
            monga_ast_expression_destroy(ast->le_cond.exp1);
            monga_ast_expression_destroy(ast->le_cond.exp2);
            break;
        case MONGA_AST_CONDITION_GE:
            monga_ast_expression_destroy(ast->ge_cond.exp1);
            monga_ast_expression_destroy(ast->ge_cond.exp2);
            break;
        case MONGA_AST_CONDITION_LT:
            monga_ast_expression_destroy(ast->lt_cond.exp1);
            monga_ast_expression_destroy(ast->lt_cond.exp2);
            break;
        case MONGA_AST_CONDITION_GT:
            monga_ast_expression_destroy(ast->gt_cond.exp1);
            monga_ast_expression_destroy(ast->gt_cond.exp2);
            break;
        case MONGA_AST_CONDITION_NOT:
            monga_ast_condition_destroy(ast->not_cond.cond);
            break;
        case MONGA_AST_CONDITION_AND:
            monga_ast_condition_destroy(ast->and_cond.cond1);
            monga_ast_condition_destroy(ast->and_cond.cond2);
            break;
        case MONGA_AST_CONDITION_OR:
            monga_ast_condition_destroy(ast->or_cond.cond1);
            monga_ast_condition_destroy(ast->or_cond.cond2);
            break;
    }
    monga_free(ast);
}

void monga_ast_call_destroy(struct monga_ast_call_t* ast)
{
    monga_free(ast->function_id);
    if (ast->expressions)
        monga_ast_expression_destroy(ast->expressions);
    monga_free(ast);
}
