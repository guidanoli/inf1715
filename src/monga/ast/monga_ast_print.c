#include "monga_ast.h"

#include <stdio.h>

static void monga_ast_print_identation(int identation)
{
    while (identation--)
        putc('\t', stdout);
}

void monga_ast_program_print(struct monga_ast_program_t* ast, int identation)
{
    monga_ast_definition_print(ast->definitions, identation);
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
    }
    if (ast->next)
        monga_ast_definition_print(ast->next, identation);
}

void monga_ast_def_variable_print(struct monga_ast_def_variable_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("Defined variable \"%s\" of type \"%s\"\n", ast->id, ast->type);
}

void monga_ast_def_type_print(struct monga_ast_def_type_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("Defined type \"%s\" described as...\n", ast->id);
    monga_ast_typedesc_print(ast->typedesc, identation+1);
}

void monga_ast_def_function_print(struct monga_ast_def_function_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("Defined function \"%s\"", ast->id);
    if (ast->type)
        printf(" of return type \"%s\"", ast->type);
    if (ast->parameters) {
        printf(" with parameters...\n");
        monga_ast_parameter_print(ast->parameters, identation+1);
    }
    monga_ast_block_print(ast->block, identation+1);
}

void monga_ast_typedesc_print(struct monga_ast_typedesc_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    switch (ast->tag) {
        case MONGA_AST_TYPEDESC_ID:
            printf("%s\n", ast->id_typedesc);
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            printf("array of...\n");
            monga_ast_typedesc_print(ast->array_typedesc, identation+1);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
            printf("record of...\n");
            monga_ast_field_print(ast->record_typedesc, identation+1);
            break;
    }
}

void monga_ast_field_print(struct monga_ast_field_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("%s, named \"%s\"\n", ast->type, ast->id);
    if (ast->next)
        monga_ast_field_print(ast->next, identation);
}

void monga_ast_parameter_print(struct monga_ast_parameter_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("\"%s\", of type %s\n", ast->id, ast->type);
    if (ast->next)
        monga_ast_parameter_print(ast->next, identation);
}

void monga_ast_block_print(struct monga_ast_block_t* ast, int identation)
{
    if (ast->variables) {
        monga_ast_print_identation(identation);
        printf("Defined variables...\n");
        monga_ast_def_variable_print(ast->variables, identation+1);
    }
    if (ast->statements) {
        monga_ast_print_identation(identation);
        printf("With statements...\n");
        monga_ast_statement_print(ast->statements, identation+1);
    }
}

void monga_ast_statement_print(struct monga_ast_statement_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    switch (ast->tag) {
        case MONGA_AST_STATEMENT_IF:
            printf("If statement with condition...\n");
            monga_ast_condition_print(ast->if_stmt.cond, identation+1);
            monga_ast_print_identation(identation);
            printf("with then block...\n");
            monga_ast_block_print(ast->if_stmt.then_block, identation+1);
            if (ast->if_stmt.else_block) {
                monga_ast_print_identation(identation);
                printf("with else block...\n");
                monga_ast_block_print(ast->if_stmt.else_block, identation+1);
            }
            break;
        case MONGA_AST_STATEMENT_WHILE:
            printf("While statement with condition...\n");
            monga_ast_condition_print(ast->while_stmt.cond, identation+1);
            monga_ast_print_identation(identation);
            printf("with loop block...\n");
            monga_ast_block_print(ast->while_stmt.loop, identation+1);
            break;
        case MONGA_AST_STATEMENT_ASSIGN:
            printf("Assignment statement with variable...\n");
            monga_ast_variable_print(ast->assign_stmt.var, identation+1);
            monga_ast_print_identation(identation);
            printf("with expression...\n");
            monga_ast_expression_print(ast->assign_stmt.exp, identation+1);
            break;
        case MONGA_AST_STATEMENT_RETURN:
            if (ast->return_stmt.exp) {
                printf("Return statement with expression...\n");
                monga_ast_expression_print(ast->return_stmt.exp, identation+1);
            } else {
                printf("Return statement\n");
            }
            break;
        case MONGA_AST_STATEMENT_CALL:
            printf("Call statement with call...\n");
            monga_ast_call_print(ast->call_stmt.call, identation+1);
            break;
        case MONGA_AST_STATEMENT_PRINT:
            printf("Print statement with expression...\n");
            monga_ast_expression_print(ast->print_stmt.exp, identation+1);
            break;
        case MONGA_AST_STATEMENT_BLOCK:
            printf("Block statement with block...\n");
            monga_ast_block_print(ast->block_stmt.block, identation+1);
            break;
    }
    if (ast->next)
        monga_ast_statement_print(ast->next, identation+1);
}

void monga_ast_variable_print(struct monga_ast_variable_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    switch (ast->tag) {
        case MONGA_AST_VARIABLE_ID:
            printf("Variable \"%s\"\n", ast->id_var.id);
            break;
        case MONGA_AST_VARIABLE_ARRAY:
            printf("Variable of array expression...\n");
            monga_ast_expression_print(ast->array_var.array, identation+1);
            monga_ast_print_identation(identation);
            printf("with indexed expression...\n");
            monga_ast_expression_print(ast->array_var.index, identation+1);
            break;
        case MONGA_AST_VARIABLE_RECORD:
            printf("Variable of field \"%s\" and record expression...\n", ast->record_var.field);
            monga_ast_expression_print(ast->record_var.record, identation+1);
            break;
    }
}

void monga_ast_expression_print(struct monga_ast_expression_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    switch (ast->tag) {
        case MONGA_AST_EXPRESSION_INTEGER:
            printf("Integer expression \"%d\"\n", ast->integer_exp.integer);
            break;
        case MONGA_AST_EXPRESSION_REAL:
            printf("Real expression \"%g\"\n", ast->real_exp.real);
            break;
        case MONGA_AST_EXPRESSION_VAR:
            printf("Variable expression of variable...\n");
            monga_ast_variable_print(ast->var_exp.var, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CALL:
            printf("Call expression of call...\n");
            monga_ast_call_print(ast->call_exp.call, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CAST:
            printf("Cast expression to type \"%s\" of expression...\n", ast->cast_exp.type);
            monga_ast_expression_print(ast->cast_exp.exp, identation+1);
            break;
        case MONGA_AST_EXPRESSION_NEW:
            if (ast->new_exp.exp) {
                printf("New expression of type \"%s\" and size given by expression...\n", ast->new_exp.type);
                monga_ast_expression_print(ast->new_exp.exp, identation+1);
            } else {
                printf("New expression of type \"%s\"\n", ast->new_exp.type);
            }
            break;
        case MONGA_AST_EXPRESSION_NEGATION:
            printf("Negation expression of expression...\n");
            monga_ast_expression_print(ast->negation_exp.exp, identation+1);
            break;
        case MONGA_AST_EXPRESSION_ADDITION:
            printf("Addition expression of expressions...\n");
            monga_ast_expression_print(ast->addition_exp.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->addition_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_SUBTRACTION:
            printf("Subtraction expression of expressions...\n");
            monga_ast_expression_print(ast->subtraction_exp.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->subtraction_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
            printf("Multiplication expression of expressions...\n");
            monga_ast_expression_print(ast->multiplication_exp.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->multiplication_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_DIVISION:
            printf("Division expression of expressions...\n");
            monga_ast_expression_print(ast->division_exp.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->division_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CONDITIONAL:
            printf("Conditional expression of condition...\n");
            monga_ast_condition_print(ast->conditional_exp.cond, identation+1);
            monga_ast_print_identation(identation);
            printf("with true expression...\n");
            monga_ast_expression_print(ast->conditional_exp.true_exp, identation+1);
            monga_ast_print_identation(identation);
            printf("with false expression...\n");
            monga_ast_expression_print(ast->conditional_exp.false_exp, identation+1);
            break;
    }
    if (ast->next)
        monga_ast_expression_print(ast->next, identation+1);
}

void monga_ast_condition_print(struct monga_ast_condition_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    switch (ast->tag) {
        case MONGA_AST_CONDITION_EQ:
            printf("Equality condition of expressions...\n");
            monga_ast_expression_print(ast->eq_cond.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->eq_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_NE:
            printf("Inequality condition of expressions...\n");
            monga_ast_expression_print(ast->ne_cond.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->ne_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_LE:
            printf("Less or equal condition of expressions...\n");
            monga_ast_expression_print(ast->le_cond.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->le_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_GE:
            printf("Greater or equal condition of expressions...\n");
            monga_ast_expression_print(ast->ge_cond.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->ge_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_LT:
            printf("Less than condition of expressions...\n");
            monga_ast_expression_print(ast->lt_cond.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->lt_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_GT:
            printf("Greater than condition of expressions...\n");
            monga_ast_expression_print(ast->gt_cond.exp1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_expression_print(ast->gt_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_NOT:
            printf("Negation condition of condition...\n");
            monga_ast_condition_print(ast->not_cond.cond, identation+1);
            break;
        case MONGA_AST_CONDITION_AND:
            printf("Logical 'and' condition of conditions...\n");
            monga_ast_condition_print(ast->and_cond.cond1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_condition_print(ast->and_cond.cond2, identation+1);
            break;
        case MONGA_AST_CONDITION_OR:
            printf("Logical 'or' condition of conditions...\n");
            monga_ast_condition_print(ast->or_cond.cond1, identation+1);
            monga_ast_print_identation(identation);
            printf("and...\n");
            monga_ast_condition_print(ast->or_cond.cond2, identation+1);
            break;
    }
}

void monga_ast_call_print(struct monga_ast_call_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    if (ast->expressions) {
        printf("Call to function \"%s\" with expressions...\n", ast->function_id);
        monga_ast_expression_print(ast->expressions, identation+1);
    } else {
        printf("Call to function \"%s\"\n", ast->function_id);
    }
}
