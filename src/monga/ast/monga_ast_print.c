#include "monga_ast.h"

#include <stdio.h>

static void monga_ast_print_identation(int identation)
{
    while (identation--)
        putc('\t', stdout);
}

void monga_ast_program_print(struct monga_ast_program_t* ast)
{
    if (ast->definitions) {
        printf("(program\n");
        monga_ast_definition_print(ast->definitions, 1);
        printf(")\n");
    } else {
        printf("(program)\n");
    }
}

void monga_ast_definition_print(struct monga_ast_definition_t* ast, int identation)
{
    switch (ast->tag) {
        case MONGA_AST_DEFINITION_VARIABLE:
            monga_ast_def_variable_print(ast->def_variable, identation);
            break;
        case MONGA_AST_DEFINITION_TYPE:
            monga_ast_def_type_print(ast->def_type, identation);
            break;
        case MONGA_AST_DEFINITION_FUNCTION:
            monga_ast_def_function_print(ast->def_function, identation);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_definition_print(ast->next, identation);
}

void monga_ast_def_variable_print(struct monga_ast_def_variable_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(def_variable id=\"%s\" type=\"%s\")\n", ast->id, ast->type);
    if (ast->next)
        monga_ast_def_variable_print(ast->next, identation);
}

void monga_ast_def_type_print(struct monga_ast_def_type_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(def_type id=\"%s\"\n", ast->id);
    monga_ast_typedesc_print(ast->typedesc, identation+1);
    monga_ast_print_identation(identation);
    printf(")\n");
}

void monga_ast_def_function_print(struct monga_ast_def_function_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(def_function id=\"%s\"", ast->id);
    if (ast->type)
        printf(" type=\"%s\"", ast->type);
    printf("\n");
    if (ast->parameters)
        monga_ast_parameter_print(ast->parameters, identation+1);
    monga_ast_block_print(ast->block, identation+1);
    monga_ast_print_identation(identation);
    printf(")\n");
}

void monga_ast_typedesc_print(struct monga_ast_typedesc_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(typedesc ");
    switch (ast->tag) {
        case MONGA_AST_TYPEDESC_ID:
            printf("\"%s\")\n", ast->id_typedesc);
            return;
        case MONGA_AST_TYPEDESC_ARRAY:
            printf("array\n");
            monga_ast_typedesc_print(ast->array_typedesc, identation+1);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
            printf("record\n");
            monga_ast_field_print(ast->record_typedesc, identation+1);
            break;
        default:
            monga_unreachable();
    }
    monga_ast_print_identation(identation);
    printf(")\n");
}

void monga_ast_field_print(struct monga_ast_field_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(field id=\"%s\" type=\"%s\")\n", ast->id, ast->type);
    if (ast->next)
        monga_ast_field_print(ast->next, identation);
}

void monga_ast_parameter_print(struct monga_ast_parameter_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(parameter id=\"%s\" type=\"%s\")\n", ast->id, ast->type);
    if (ast->next)
        monga_ast_parameter_print(ast->next, identation);
}

void monga_ast_block_print(struct monga_ast_block_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    if (ast->variables || ast->statements) {
        printf("(block\n");
        if (ast->variables)
            monga_ast_def_variable_print(ast->variables, identation+1);
        if (ast->statements)
            monga_ast_statement_print(ast->statements, identation+1);
        monga_ast_print_identation(identation);
        printf(")\n");
    } else {
        printf("(block)\n");
    }
}

void monga_ast_statement_print(struct monga_ast_statement_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(statement ");
    switch (ast->tag) {
        case MONGA_AST_STATEMENT_IF:
            printf("if\n");
            monga_ast_condition_print(ast->if_stmt.cond, identation+1);
            monga_ast_block_print(ast->if_stmt.then_block, identation+1);
            if (ast->if_stmt.else_block)
                monga_ast_block_print(ast->if_stmt.else_block, identation+1);
            break;
        case MONGA_AST_STATEMENT_WHILE:
            printf("while\n");
            monga_ast_condition_print(ast->while_stmt.cond, identation+1);
            monga_ast_block_print(ast->while_stmt.loop, identation+1);
            break;
        case MONGA_AST_STATEMENT_ASSIGN:
            printf("=\n");
            monga_ast_variable_print(ast->assign_stmt.var, identation+1);
            monga_ast_expression_print(ast->assign_stmt.exp, identation+1);
            break;
        case MONGA_AST_STATEMENT_RETURN:
            if (ast->return_stmt.exp) {
                printf("return\n");
                monga_ast_expression_print(ast->return_stmt.exp, identation+1);
            } else {
                printf("return)\n");
                return;
            }
            break;
        case MONGA_AST_STATEMENT_CALL:
            printf("call\n");
            monga_ast_call_print(ast->call_stmt.call, identation+1);
            break;
        case MONGA_AST_STATEMENT_PRINT:
            printf("print\n");
            monga_ast_expression_print(ast->print_stmt.exp, identation+1);
            break;
        case MONGA_AST_STATEMENT_BLOCK:
            printf("block\n");
            monga_ast_block_print(ast->block_stmt.block, identation+1);
            break;
        default:
            monga_unreachable();
    }
    monga_ast_print_identation(identation);
    printf(")\n");
    if (ast->next)
        monga_ast_statement_print(ast->next, identation);
}

void monga_ast_variable_print(struct monga_ast_variable_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(variable ");
    switch (ast->tag) {
        case MONGA_AST_VARIABLE_ID:
            printf("\"%s\")\n", ast->id_var.id);
            return;
        case MONGA_AST_VARIABLE_ARRAY:
            printf("array\n");
            monga_ast_expression_print(ast->array_var.array, identation+1);
            monga_ast_expression_print(ast->array_var.index, identation+1);
            break;
        case MONGA_AST_VARIABLE_RECORD:
            printf("record field=\"%s\"\n", ast->record_var.field);
            monga_ast_expression_print(ast->record_var.record, identation+1);
            break;
        default:
            monga_unreachable();
    }
    monga_ast_print_identation(identation);
    printf(")\n");
}

void monga_ast_expression_print(struct monga_ast_expression_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(expression ");
    switch (ast->tag) {
        case MONGA_AST_EXPRESSION_INTEGER:
            printf("integer \"%d\")\n", ast->integer_exp.integer);
            return;
        case MONGA_AST_EXPRESSION_REAL:
            printf("real \"%g\")\n", ast->real_exp.real);
            return;
        case MONGA_AST_EXPRESSION_VAR:
            printf("variable\n");
            monga_ast_variable_print(ast->var_exp.var, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CALL:
            printf("call\n");
            monga_ast_call_print(ast->call_exp.call, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CAST:
            printf("cast type=\"%s\"\n", ast->cast_exp.type);
            monga_ast_expression_print(ast->cast_exp.exp, identation+1);
            break;
        case MONGA_AST_EXPRESSION_NEW:
            if (ast->new_exp.exp) {
                printf("new type=\"%s\"\n", ast->new_exp.type);
                monga_ast_expression_print(ast->new_exp.exp, identation+1);
            } else {
                printf("new type=\"%s\")\n", ast->new_exp.type);
                return;
            }
            break;
        case MONGA_AST_EXPRESSION_NEGATIVE:
            printf("-\n");
            monga_ast_expression_print(ast->negative_exp.exp, identation+1);
            break;
        case MONGA_AST_EXPRESSION_ADDITION:
            printf("+\n");
            monga_ast_expression_print(ast->addition_exp.exp1, identation+1);
            monga_ast_expression_print(ast->addition_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_SUBTRACTION:
            printf("-\n");
            monga_ast_expression_print(ast->subtraction_exp.exp1, identation+1);
            monga_ast_expression_print(ast->subtraction_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
            printf("*\n");
            monga_ast_expression_print(ast->multiplication_exp.exp1, identation+1);
            monga_ast_expression_print(ast->multiplication_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_DIVISION:
            printf("/\n");
            monga_ast_expression_print(ast->division_exp.exp1, identation+1);
            monga_ast_expression_print(ast->division_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CONDITIONAL:
            printf("?:\n");
            monga_ast_condition_print(ast->conditional_exp.cond, identation+1);
            monga_ast_expression_print(ast->conditional_exp.true_exp, identation+1);
            monga_ast_expression_print(ast->conditional_exp.false_exp, identation+1);
            break;
        default:
            monga_unreachable();
    }
    monga_ast_print_identation(identation);
    printf(")\n");
    if (ast->next)
        monga_ast_expression_print(ast->next, identation);
}

void monga_ast_condition_print(struct monga_ast_condition_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(condition ");
    switch (ast->tag) {
        case MONGA_AST_CONDITION_EQ:
            printf("==\n");
            monga_ast_expression_print(ast->eq_cond.exp1, identation+1);
            monga_ast_expression_print(ast->eq_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_NE:
            printf("~=\n");
            monga_ast_expression_print(ast->ne_cond.exp1, identation+1);
            monga_ast_expression_print(ast->ne_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_LE:
            printf("<=\n");
            monga_ast_expression_print(ast->le_cond.exp1, identation+1);
            monga_ast_expression_print(ast->le_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_GE:
            printf(">=\n");
            monga_ast_expression_print(ast->ge_cond.exp1, identation+1);
            monga_ast_expression_print(ast->ge_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_LT:
            printf("<\n");
            monga_ast_expression_print(ast->lt_cond.exp1, identation+1);
            monga_ast_expression_print(ast->lt_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_GT:
            printf(">\n");
            monga_ast_expression_print(ast->gt_cond.exp1, identation+1);
            monga_ast_expression_print(ast->gt_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_NOT:
            printf("!\n");
            monga_ast_condition_print(ast->not_cond.cond, identation+1);
            break;
        case MONGA_AST_CONDITION_AND:
            printf("&&\n");
            monga_ast_condition_print(ast->and_cond.cond1, identation+1);
            monga_ast_condition_print(ast->and_cond.cond2, identation+1);
            break;
        case MONGA_AST_CONDITION_OR:
            printf("||\n");
            monga_ast_condition_print(ast->or_cond.cond1, identation+1);
            monga_ast_condition_print(ast->or_cond.cond2, identation+1);
            break;
        default:
            monga_unreachable();
    }
    monga_ast_print_identation(identation);
    printf(")\n");
}

void monga_ast_call_print(struct monga_ast_call_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    if (ast->expressions) {
        printf("(call function=\"%s\"\n", ast->function_id);
        monga_ast_expression_print(ast->expressions, identation+1);
        monga_ast_print_identation(identation);
        printf(")\n");
    } else {
        printf("(call function=\"%s\")\n", ast->function_id);
    }
}
