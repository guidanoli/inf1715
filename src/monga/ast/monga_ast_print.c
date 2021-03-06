#include "monga_ast_print.h"
#include "monga_ast_reference.h"

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
        monga_ast_definition_print(ast->definitions->first, 1);
        printf(")\n");
    } else {
        printf("(program)\n");
    }
}

void monga_ast_definition_print(struct monga_ast_definition_t* ast, int identation)
{
    switch (ast->tag) {
        case MONGA_AST_DEFINITION_VARIABLE:
            monga_ast_def_variable_print(ast->u.def_variable, identation);
            break;
        case MONGA_AST_DEFINITION_TYPE:
            monga_ast_def_type_print(ast->u.def_type, identation);
            break;
        case MONGA_AST_DEFINITION_FUNCTION:
            monga_ast_def_function_print(ast->u.def_function, identation);
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
    printf("(def_variable id=\"%s\"\n", ast->id);
    monga_ast_reference_print(&ast->type, identation+1);
    monga_ast_print_identation(identation);
    printf(")\n");
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
    printf("(def_function id=\"%s\"\n", ast->id);
    if (ast->type.id)
        monga_ast_reference_print(&ast->type, identation+1);
    if (ast->parameters)
        monga_ast_def_variable_print(ast->parameters->first, identation+1);
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
            printf("id\n");
            monga_ast_reference_print(&ast->u.id_typedesc, identation+1);
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            printf("array\n");
            monga_ast_typedesc_print(ast->u.array_typedesc, identation+1);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
            printf("record\n");
            monga_ast_field_print(ast->u.record_typedesc.field_list->first, identation+1);
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
    printf("(field id=\"%s\"\n", ast->id);
    monga_ast_reference_print(&ast->type, identation+1);
    monga_ast_print_identation(identation);
    printf(")\n");
    if (ast->next)
        monga_ast_field_print(ast->next, identation);
}

void monga_ast_block_print(struct monga_ast_block_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    if (ast->variables || ast->statements) {
        printf("(block\n");
        if (ast->variables)
            monga_ast_def_variable_print(ast->variables->first, identation+1);
        if (ast->statements)
            monga_ast_statement_print(ast->statements->first, identation+1);
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
            monga_ast_condition_print(ast->u.if_stmt.cond, identation+1);
            monga_ast_block_print(ast->u.if_stmt.then_block, identation+1);
            if (ast->u.if_stmt.else_block)
                monga_ast_block_print(ast->u.if_stmt.else_block, identation+1);
            break;
        case MONGA_AST_STATEMENT_WHILE:
            printf("while\n");
            monga_ast_condition_print(ast->u.while_stmt.cond, identation+1);
            monga_ast_block_print(ast->u.while_stmt.loop, identation+1);
            break;
        case MONGA_AST_STATEMENT_ASSIGN:
            printf("=\n");
            monga_ast_variable_print(ast->u.assign_stmt.var, identation+1);
            monga_ast_expression_print(ast->u.assign_stmt.exp, identation+1);
            break;
        case MONGA_AST_STATEMENT_RETURN:
            if (ast->u.return_stmt.exp) {
                printf("return\n");
                monga_ast_expression_print(ast->u.return_stmt.exp, identation+1);
            } else {
                printf("return)\n");
                return;
            }
            break;
        case MONGA_AST_STATEMENT_CALL:
            printf("call\n");
            monga_ast_call_print(ast->u.call_stmt.call, identation+1);
            break;
        case MONGA_AST_STATEMENT_PRINT:
            printf("print\n");
            monga_ast_expression_print(ast->u.print_stmt.exp, identation+1);
            break;
        case MONGA_AST_STATEMENT_BLOCK:
            printf("block\n");
            monga_ast_block_print(ast->u.block_stmt.block, identation+1);
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
            printf("id\n");
            monga_ast_reference_print(&ast->u.id_var, identation+1);
            break;
        case MONGA_AST_VARIABLE_ARRAY:
            printf("array\n");
            monga_ast_expression_print(ast->u.array_var.array, identation+1);
            monga_ast_expression_print(ast->u.array_var.index, identation+1);
            break;
        case MONGA_AST_VARIABLE_RECORD:
            printf("record\n");
            monga_ast_reference_print(&ast->u.record_var.field, identation+1);
            monga_ast_expression_print(ast->u.record_var.record, identation+1);
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
            printf("integer \"%d\")\n", ast->u.integer_exp.integer);
            return;
        case MONGA_AST_EXPRESSION_REAL:
            printf("real \"%g\")\n", ast->u.real_exp.real);
            return;
        case MONGA_AST_EXPRESSION_VAR:
            printf("variable\n");
            break;
        case MONGA_AST_EXPRESSION_CALL:
            printf("call\n");
            break;
        case MONGA_AST_EXPRESSION_CAST:
            printf("cast\n");
            break;
        case MONGA_AST_EXPRESSION_NEW:
            printf("new\n");
            break;
        case MONGA_AST_EXPRESSION_NEGATIVE:
            printf("-\n");
            break;
        case MONGA_AST_EXPRESSION_ADDITION:
            printf("+\n");
            break;
        case MONGA_AST_EXPRESSION_SUBTRACTION:
            printf("-\n");
            break;
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
            printf("*\n");
            break;
        case MONGA_AST_EXPRESSION_DIVISION:
            printf("/\n");
            break;
        case MONGA_AST_EXPRESSION_CONDITIONAL:
            printf("?:\n");
            break;
        default:
            monga_unreachable();
    }
    switch (ast->tag) {
        case MONGA_AST_EXPRESSION_VAR:
            monga_ast_variable_print(ast->u.var_exp.var, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CALL:
            monga_ast_call_print(ast->u.call_exp.call, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CAST:
            monga_ast_reference_print(&ast->u.cast_exp.type, identation+1);
            monga_ast_expression_print(ast->u.cast_exp.exp, identation+1);
            break;
        case MONGA_AST_EXPRESSION_NEW:
            monga_ast_reference_print(&ast->u.new_exp.type, identation+1);
            if (ast->u.new_exp.exp)
                monga_ast_expression_print(ast->u.new_exp.exp, identation+1);
            break;
        case MONGA_AST_EXPRESSION_NEGATIVE:
            monga_ast_expression_print(ast->u.negative_exp.exp, identation+1);
            break;
        case MONGA_AST_EXPRESSION_ADDITION:
        case MONGA_AST_EXPRESSION_SUBTRACTION:
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
        case MONGA_AST_EXPRESSION_DIVISION:
            monga_ast_expression_print(ast->u.binop_exp.exp1, identation+1);
            monga_ast_expression_print(ast->u.binop_exp.exp2, identation+1);
            break;
        case MONGA_AST_EXPRESSION_CONDITIONAL:
            monga_ast_condition_print(ast->u.conditional_exp.cond, identation+1);
            monga_ast_expression_print(ast->u.conditional_exp.true_exp, identation+1);
            monga_ast_expression_print(ast->u.conditional_exp.false_exp, identation+1);
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
            break;
        case MONGA_AST_CONDITION_NE:
            printf("~=\n");
            break;
        case MONGA_AST_CONDITION_LE:
            printf("<=\n");
            break;
        case MONGA_AST_CONDITION_GE:
            printf(">=\n");
            break;
        case MONGA_AST_CONDITION_LT:
            printf("<\n");
            break;
        case MONGA_AST_CONDITION_GT:
            printf(">\n");
            break;
        case MONGA_AST_CONDITION_NOT:
            printf("!\n");
            break;
        case MONGA_AST_CONDITION_AND:
            printf("&&\n");
            break;
        case MONGA_AST_CONDITION_OR:
            printf("||\n");
            break;
        default:
            monga_unreachable();
    }
    switch (ast->tag) {
        case MONGA_AST_CONDITION_EQ:
        case MONGA_AST_CONDITION_NE:
        case MONGA_AST_CONDITION_LE:
        case MONGA_AST_CONDITION_GE:
        case MONGA_AST_CONDITION_LT:
        case MONGA_AST_CONDITION_GT:
            monga_ast_expression_print(ast->u.exp_binop_cond.exp1, identation+1);
            monga_ast_expression_print(ast->u.exp_binop_cond.exp2, identation+1);
            break;
        case MONGA_AST_CONDITION_AND:
        case MONGA_AST_CONDITION_OR:
            monga_ast_condition_print(ast->u.cond_binop_cond.cond1, identation+1);
            monga_ast_condition_print(ast->u.cond_binop_cond.cond2, identation+1);
            break;
        case MONGA_AST_CONDITION_NOT:
            monga_ast_condition_print(ast->u.cond_unop_cond.cond, identation+1);
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
    printf("(call\n");
    monga_ast_reference_print(&ast->function, identation+1);
    if (ast->expressions)
        monga_ast_expression_print(ast->expressions->first, identation+1);
    monga_ast_print_identation(identation);
    printf(")\n");
}

void monga_ast_reference_print(struct monga_ast_reference_t* ast, int identation)
{
    monga_ast_print_identation(identation);
    printf("(reference id=\"%s\" kind=\"%s\" line=%zu)\n",
        ast->id, monga_ast_reference_kind_name(ast->tag), monga_ast_reference_line(ast));
}