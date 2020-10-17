#include "monga_ast_bind.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"

/* Function declarations */

static struct monga_ast_typedesc_t* monga_ast_typedesc_resolve_id(struct monga_ast_typedesc_t *typedesc, struct monga_ast_bind_stack_t* stack);
static bool monga_ast_typedesc_equal(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2, struct monga_ast_bind_stack_t* stack);
static bool monga_ast_field_list_equal(struct monga_ast_field_t *field1, struct monga_ast_field_t *field2, struct monga_ast_bind_stack_t* stack);
static void monga_ast_call_parameters_bind(struct monga_ast_def_function_t* def_function, struct monga_ast_parameter_t* parameter,
    struct monga_ast_expression_t* expression, struct monga_ast_bind_stack_t* stack);

/* Function definitions */

struct monga_ast_typedesc_t* monga_ast_typedesc_resolve_id(struct monga_ast_typedesc_t *typedesc, struct monga_ast_bind_stack_t* stack)
{
    if (typedesc->tag == MONGA_AST_TYPEDESC_ID) {
        struct monga_ast_reference_t* id_typedesc = &typedesc->id_typedesc;
        monga_assert(id_typedesc->tag == MONGA_AST_REFERENCE_TYPE);
        return monga_ast_typedesc_resolve_id(id_typedesc->def_type->typedesc, stack);
    }
    return typedesc;
}

bool monga_ast_typedesc_equal(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2, struct monga_ast_bind_stack_t* stack)
{
    typedesc1 = monga_ast_typedesc_resolve_id(typedesc1, stack);
    typedesc2 = monga_ast_typedesc_resolve_id(typedesc2, stack);
    if (typedesc1->tag == typedesc2->tag) {
        switch (typedesc1->tag) {
        case MONGA_AST_TYPEDESC_BUILTIN:
            return typedesc1->builtin_typedesc == typedesc2->builtin_typedesc;
        case MONGA_AST_TYPEDESC_ID:
            monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            return monga_ast_typedesc_equal(typedesc1->array_typedesc, typedesc2->array_typedesc, stack);
        case MONGA_AST_TYPEDESC_RECORD:
            return monga_ast_field_list_equal(typedesc1->record_typedesc->first, typedesc2->record_typedesc->first, stack);
        default:
            monga_unreachable();
        }
    } else {
        return false;
    }
}

bool monga_ast_field_list_equal(struct monga_ast_field_t *field1, struct monga_ast_field_t *field2, struct monga_ast_bind_stack_t* stack)
{
    if (strcmp(field1->id, field2->id) != 0)
        return false;
    
    if (!monga_ast_typedesc_equal(field1->type.def_type->typedesc, field2->type.def_type->typedesc, stack))
        return false;
    
    if (!field1->next && !field2->next)
        return true;
    else if (field1->next && field2->next)
        return monga_ast_field_list_equal(field1->next, field2->next, stack);
    else
        return false;
}

void monga_ast_program_bind(struct monga_ast_program_t* ast)
{
    if (ast->definitions) {
        struct monga_ast_bind_stack_t* stack = monga_ast_bind_stack_create();
        for (enum monga_ast_typedesc_builtin_t builtin = 0; builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT; ++builtin) {
            struct monga_ast_def_type_t* def_type = monga_ast_builtin_def_type(builtin);
            monga_ast_bind_stack_insert_name(stack, def_type->id, MONGA_AST_REFERENCE_TYPE, def_type);
        }
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
    monga_ast_bind_stack_get_typed_name(stack, &ast->type, 1, MONGA_AST_REFERENCE_TYPE);
    if (ast->next)
        monga_ast_def_variable_bind(ast->next, stack);
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
        monga_ast_bind_stack_get_typed_name(stack, &ast->type, 1, MONGA_AST_REFERENCE_TYPE);
    monga_ast_bind_stack_block_enter(stack);
    if (ast->parameters)
        monga_ast_parameter_bind(ast->parameters->first, stack);
    monga_ast_block_bind(ast->block, stack);
    monga_ast_bind_stack_block_exit(stack);
    /* TODO: check if the expressions of all the return statements have the same type as the function.
             or if all return statements DON'T have expressions if the function doesn't have a return
             value either... In order to do that, you need to inspect all the statements. */
    /* TODO: warn that all statements after return statements will be ignored */
}

void monga_ast_typedesc_bind(struct monga_ast_typedesc_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_TYPEDESC_ID:
            monga_ast_bind_stack_get_typed_name(stack, &ast->id_typedesc, 1, MONGA_AST_REFERENCE_TYPE);
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            monga_ast_typedesc_bind(ast->array_typedesc, stack);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
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
    monga_ast_bind_stack_get_typed_name(stack, &ast->type, 1, MONGA_AST_REFERENCE_TYPE);
    if (ast->next)
        monga_ast_field_bind(ast->next, stack);
}

void monga_ast_parameter_bind(struct monga_ast_parameter_t* ast, struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_insert_name(stack, ast->id, MONGA_AST_REFERENCE_PARAMETER, ast);
    monga_ast_bind_stack_get_typed_name(stack, &ast->type, 1, MONGA_AST_REFERENCE_TYPE);
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
            if (!monga_ast_typedesc_equal(&ast->assign_stmt.var->typedesc, &ast->assign_stmt.exp->typedesc, stack)) {
                fprintf(stderr, "Expected expression of type ");
                monga_ast_typedesc_write(stderr, &ast->assign_stmt.var->typedesc);
                fprintf(stderr, " instead of ");
                monga_ast_typedesc_write(stderr, &ast->assign_stmt.exp->typedesc);
                fprintf(stderr, "\n");
                exit(MONGA_ERR_TYPE);
            }
            break;
        case MONGA_AST_STATEMENT_RETURN:
            if (ast->return_stmt.exp)
                monga_ast_expression_bind(ast->return_stmt.exp, stack);
            break;
        case MONGA_AST_STATEMENT_CALL:
            monga_ast_call_bind(ast->call_stmt.call, stack);
            /* TODO: Warn that return value is being thrown away */
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
        {
            struct monga_ast_reference_t* type;
            struct monga_ast_reference_t* id_var = &ast->id_var;
            monga_ast_bind_stack_get_typed_name(stack, id_var, 2, MONGA_AST_REFERENCE_VARIABLE, MONGA_AST_REFERENCE_PARAMETER);
            switch (ast->id_var.tag) {
            case MONGA_AST_REFERENCE_VARIABLE:
            {
                struct monga_ast_def_variable_t* def_variable = id_var->def_variable;
                type = &def_variable->type;
                break;
            }
            case MONGA_AST_REFERENCE_PARAMETER:
            {
                struct monga_ast_parameter_t* parameter = id_var->parameter;
                type = &parameter->type;
                break;
            }
            default:
                monga_unreachable(); /* monga_ast_bind_stack_get_typed_name guarantees it */
            }
            monga_assert(type->tag == MONGA_AST_REFERENCE_TYPE);
            monga_ast_typedesc_copy(type->def_type->typedesc, &ast->typedesc);
            break;
        }
        case MONGA_AST_VARIABLE_ARRAY:
        {
            /* array */
            {
                struct monga_ast_typedesc_t* typedesc;
                monga_ast_expression_bind(ast->array_var.array, stack);
                typedesc = monga_ast_typedesc_resolve_id(&ast->array_var.array->typedesc, stack);
                switch (typedesc->tag) {
                case MONGA_AST_TYPEDESC_BUILTIN:
                    fprintf(stderr, "Expected expression to be of array type and not \"%s\"\n",
                        monga_ast_builtin_typedesc_id(typedesc->builtin_typedesc));
                    exit(MONGA_ERR_TYPE);
                    break;
                case MONGA_AST_TYPEDESC_ID:
                    monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                    break;
                case MONGA_AST_TYPEDESC_ARRAY:
                    typedesc = typedesc->array_typedesc;
                    break;
                case MONGA_AST_TYPEDESC_RECORD:
                    fprintf(stderr, "Expected expression to be of array type and not record\n");
                    exit(MONGA_ERR_TYPE);
                    break;
                default:
                    monga_unreachable();
                }
                monga_ast_typedesc_copy(typedesc, &ast->typedesc);
            }
            /* index */
            {
                struct monga_ast_typedesc_t* typedesc;
                monga_ast_expression_bind(ast->array_var.index, stack);
                typedesc = monga_ast_typedesc_resolve_id(&ast->array_var.index->typedesc, stack);
                switch (typedesc->tag) {
                case MONGA_AST_TYPEDESC_BUILTIN:
                    if (typedesc->builtin_typedesc != MONGA_AST_TYPEDESC_BUILTIN_INT) {
                        fprintf(stderr, "Expected index to be of int type and not \"%s\"\n",
                            monga_ast_builtin_typedesc_id(typedesc->builtin_typedesc));
                        exit(MONGA_ERR_TYPE);
                    }
                    break;
                case MONGA_AST_TYPEDESC_ID:
                    monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                    break;
                case MONGA_AST_TYPEDESC_ARRAY:
                    fprintf(stderr, "Expected index to be of int type and not array\n");
                    exit(MONGA_ERR_TYPE);
                    break;
                case MONGA_AST_TYPEDESC_RECORD:
                    fprintf(stderr, "Expected index to be of int type and not record\n");
                    exit(MONGA_ERR_TYPE);
                    break;
                default:
                    monga_unreachable();
                }
            }
            break;
        }
        case MONGA_AST_VARIABLE_RECORD:
        {
            struct monga_ast_typedesc_t* typedesc;
            monga_ast_expression_bind(ast->record_var.record, stack);
            typedesc = monga_ast_typedesc_resolve_id(&ast->record_var.record->typedesc, stack);
            switch (typedesc->tag) {
            case MONGA_AST_TYPEDESC_BUILTIN:
                fprintf(stderr, "Expected expression to be of record type and not \"%s\"\n",
                    monga_ast_builtin_typedesc_id(typedesc->builtin_typedesc));
                exit(MONGA_ERR_TYPE);
                break;
            case MONGA_AST_TYPEDESC_ID:
                monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                break;
            case MONGA_AST_TYPEDESC_ARRAY:
                fprintf(stderr, "Expected expression to be of record type and not array\n");
                exit(MONGA_ERR_TYPE);
                break;
            case MONGA_AST_TYPEDESC_RECORD:
            {
                struct monga_ast_field_t* field;
                bool found_field = false;
                for (field = typedesc->record_typedesc->first; field; field = field->next) {
                    if (strcmp(field->id, ast->record_var.field.id) == 0) {
                        struct monga_ast_reference_t* reference = &field->type;
                        ast->record_var.field.tag = MONGA_AST_REFERENCE_FIELD;
                        ast->record_var.field.field = field;
                        monga_ast_typedesc_copy(reference->def_type->typedesc, &ast->typedesc);
                        found_field = true;
                        break;
                    }
                }
                if (!found_field) {
                    fprintf(stderr, "Could not find field \"%s\" in record\n", ast->record_var.field.id);
                    exit(MONGA_ERR_UNDECLARED);
                }
                break;
            }
            default:
                monga_unreachable();
            }
            break;
        }
        default:
            monga_unreachable();
    }
}

void monga_ast_expression_bind(struct monga_ast_expression_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_EXPRESSION_INTEGER:
            monga_ast_typedesc_copy(monga_ast_builtin_typedesc(MONGA_AST_TYPEDESC_BUILTIN_INT), &ast->typedesc);
            break;
        case MONGA_AST_EXPRESSION_REAL:
            monga_ast_typedesc_copy(monga_ast_builtin_typedesc(MONGA_AST_TYPEDESC_BUILTIN_FLOAT), &ast->typedesc);
            break;
        case MONGA_AST_EXPRESSION_VAR:
            monga_ast_variable_bind(ast->var_exp.var, stack);
            monga_ast_typedesc_copy(&ast->var_exp.var->typedesc, &ast->typedesc);
            break;
        case MONGA_AST_EXPRESSION_CALL:
        {
            struct monga_ast_reference_t *reference;
            struct monga_ast_def_function_t *def_function;
            monga_ast_call_bind(ast->call_exp.call, stack);
            reference = &ast->call_exp.call->function;
            def_function = reference->def_function;
            if (def_function->type.id == NULL) {
                fprintf(stderr, "Function \"%s\" does not return a value\n", def_function->id);
                exit(MONGA_ERR_NO_RETURN);
            }
            monga_ast_typedesc_copy(def_function->type.def_type->typedesc, &ast->typedesc);
            break;
        }
        case MONGA_AST_EXPRESSION_CAST:
            monga_ast_expression_bind(ast->cast_exp.exp, stack);
            monga_ast_bind_stack_get_typed_name(stack, &ast->cast_exp.type, 1, MONGA_AST_REFERENCE_TYPE);
            monga_ast_typedesc_copy(ast->cast_exp.type.def_type->typedesc, &ast->typedesc);
            break;
        case MONGA_AST_EXPRESSION_NEW:
            monga_ast_bind_stack_get_typed_name(stack, &ast->new_exp.type, 1, MONGA_AST_REFERENCE_TYPE);
            if (ast->new_exp.exp) {
                /* exp */
                {
                    struct monga_ast_expression_t* exp = ast->new_exp.exp;
                    struct monga_ast_typedesc_t* typedesc = &exp->typedesc;
                    monga_ast_expression_bind(exp, stack);
                    if (typedesc->tag != MONGA_AST_TYPEDESC_BUILTIN && typedesc->builtin_typedesc != MONGA_AST_TYPEDESC_BUILTIN_INT) {
                        fprintf(stderr, "Expected size to be of type \"int\" instead of ");
                        monga_ast_typedesc_write(stderr, typedesc);
                        fprintf(stderr, "\n");
                        exit(MONGA_ERR_TYPE);
                    }
                }
                /* type */
                {
                    monga_ast_typedesc_make_array(ast->new_exp.type.def_type->typedesc, &ast->typedesc);
                }
            } else {
                monga_ast_typedesc_copy(ast->new_exp.type.def_type->typedesc, &ast->typedesc);
            }
            break;
        case MONGA_AST_EXPRESSION_NEGATIVE:
            monga_ast_expression_bind(ast->negative_exp.exp, stack);
            monga_ast_typedesc_copy(&ast->negative_exp.exp->typedesc, &ast->typedesc);
            break;
        case MONGA_AST_EXPRESSION_ADDITION:
        case MONGA_AST_EXPRESSION_SUBTRACTION:
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
        case MONGA_AST_EXPRESSION_DIVISION:
            monga_ast_expression_bind(ast->binop_exp.exp1, stack);
            monga_ast_expression_bind(ast->binop_exp.exp2, stack);
            if (monga_ast_typedesc_equal(&ast->binop_exp.exp1->typedesc, &ast->binop_exp.exp2->typedesc, stack)) {
                monga_ast_typedesc_copy(&ast->binop_exp.exp1->typedesc, &ast->typedesc);
            } else {
                fprintf(stderr, "Binary operation expression between ");
                monga_ast_typedesc_write(stderr, &ast->binop_exp.exp1->typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, &ast->binop_exp.exp2->typedesc);
                fprintf(stderr, "\n");
                exit(MONGA_ERR_TYPE);
            }
            break;
        case MONGA_AST_EXPRESSION_CONDITIONAL:
            monga_ast_condition_bind(ast->conditional_exp.cond, stack);
            monga_ast_expression_bind(ast->conditional_exp.true_exp, stack);
            monga_ast_expression_bind(ast->conditional_exp.false_exp, stack);
            if (monga_ast_typedesc_equal(&ast->conditional_exp.true_exp->typedesc, &ast->conditional_exp.false_exp->typedesc, stack)) {
                monga_ast_typedesc_copy(&ast->conditional_exp.true_exp->typedesc, &ast->typedesc);
            } else {
                fprintf(stderr, "Conditional expression with two possible types: ");
                monga_ast_typedesc_write(stderr, &ast->conditional_exp.true_exp->typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, &ast->conditional_exp.false_exp->typedesc);
                fprintf(stderr, "\n");
                exit(MONGA_ERR_TYPE);
            }
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_expression_bind(ast->next, stack);
}

void monga_ast_condition_bind(struct monga_ast_condition_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_CONDITION_EQ:
        case MONGA_AST_CONDITION_NE:
        case MONGA_AST_CONDITION_LE:
        case MONGA_AST_CONDITION_GE:
        case MONGA_AST_CONDITION_LT:
        case MONGA_AST_CONDITION_GT:
            monga_ast_expression_bind(ast->exp_binop_cond.exp1, stack);
            monga_ast_expression_bind(ast->exp_binop_cond.exp2, stack);
            if (!monga_ast_typedesc_equal(&ast->exp_binop_cond.exp1->typedesc, &ast->exp_binop_cond.exp2->typedesc, stack)) {
                fprintf(stderr, "Binary operation condition between ");
                monga_ast_typedesc_write(stderr, &ast->exp_binop_cond.exp1->typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, &ast->exp_binop_cond.exp2->typedesc);
                fprintf(stderr, "\n");
                exit(MONGA_ERR_TYPE);
            }
            break;
        case MONGA_AST_CONDITION_AND:
        case MONGA_AST_CONDITION_OR:
        case MONGA_AST_CONDITION_NOT:
            break;
        default:
            monga_unreachable();
    }
}

void monga_ast_call_parameters_bind(struct monga_ast_def_function_t* def_function, struct monga_ast_parameter_t* parameter,
    struct monga_ast_expression_t* expression, struct monga_ast_bind_stack_t* stack)
{
    if (parameter && expression) {
        struct monga_ast_reference_t* parameter_type = &parameter->type;
        if (!monga_ast_typedesc_equal(parameter_type->def_type->typedesc, &expression->typedesc, stack)) {
            fprintf(stderr, "Expected argument of type \"%s\" in function \"%s\"\n",
                parameter->type.def_type->id, def_function->id);
            exit(MONGA_ERR_TYPE);
        }
        monga_ast_call_parameters_bind(def_function, parameter->next, expression->next, stack);
    } else if (parameter == NULL && expression == NULL) {
        return;
    } else if (parameter == NULL && expression) {
        fprintf(stderr, "Provided too many arguments to function \"%s\"\n", def_function->id);
        exit(MONGA_ERR_SIGNATURE);
    } else { /* parameter && expression == NULL */
        fprintf(stderr, "Provided too few arguments to function \"%s\"\n", def_function->id);
        exit(MONGA_ERR_SIGNATURE);
    }
}

void monga_ast_call_bind(struct monga_ast_call_t* ast, struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_reference_t* function;
    struct monga_ast_def_function_t* def_function;
    struct monga_ast_expression_t* expressions;
    struct monga_ast_parameter_t* parameters;

    function = &ast->function;

    monga_ast_bind_stack_get_typed_name(stack, function, 1, MONGA_AST_REFERENCE_FUNCTION);

    def_function = function->def_function;

    if (ast->expressions)
        monga_ast_expression_bind(ast->expressions->first, stack);

    expressions = ast->expressions ? ast->expressions->first : NULL;
    parameters = def_function->parameters ? def_function->parameters->first : NULL;

    monga_ast_call_parameters_bind(def_function, parameters, expressions, stack);
}