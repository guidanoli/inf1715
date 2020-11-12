#include "monga_ast_bind.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"
#include "monga_ast_reference.h"

/* Function declarations */

static struct monga_ast_def_type_t* monga_ast_construct_annonymous_def_type(struct monga_ast_typedesc_t* typedesc);
static struct monga_ast_typedesc_t* monga_ast_construct_annonymous_array_typedesc(struct monga_ast_typedesc_t* typedesc);
static void monga_ast_repeated_field_check(struct monga_ast_field_t* field);
static void monga_ast_call_parameters_bind(struct monga_ast_call_t* call, struct monga_ast_parameter_t* parameter, struct monga_ast_expression_t* expression, struct monga_ast_bind_stack_t* stack);
static void monga_ast_check_function_statements(struct monga_ast_statement_t* statement, struct monga_ast_typedesc_t* typedesc, struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* function);

/* Function definitions */

void monga_ast_program_bind(struct monga_ast_program_t* ast)
{
    if (ast->definitions) {
        struct monga_ast_bind_stack_t* stack = monga_ast_bind_stack_create();
        for (enum monga_ast_typedesc_builtin_t builtin = 0; builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT; ++builtin) {
            if (monga_ast_builtin_visible(builtin)) {
                struct monga_ast_def_type_t* def_type = monga_ast_builtin_def_type(builtin);
                struct monga_ast_reference_t* reference = construct(reference);
                reference->tag = MONGA_AST_REFERENCE_TYPE;
                reference->u.def_type = def_type;
                reference->id = def_type->id;
                monga_ast_bind_stack_insert_name(stack, reference);
            }
        }
        monga_ast_definition_bind(ast->definitions->first, stack);
        monga_ast_bind_stack_destroy(stack);
    }
}

void monga_ast_definition_bind(struct monga_ast_definition_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_DEFINITION_VARIABLE:
            monga_ast_def_variable_bind(ast->u.def_variable, stack);
            break;
        case MONGA_AST_DEFINITION_TYPE:
            monga_ast_def_type_bind(ast->u.def_type, stack);
            break;
        case MONGA_AST_DEFINITION_FUNCTION:
            monga_ast_def_function_bind(ast->u.def_function, stack);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_definition_bind(ast->next, stack);
}

void monga_ast_def_variable_bind(struct monga_ast_def_variable_t* ast, struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_reference_t* reference = construct(reference);
    reference->tag = MONGA_AST_REFERENCE_VARIABLE;
    reference->u.def_variable = ast;
    reference->id = ast->id;
    monga_ast_bind_stack_insert_name(stack, reference);
    monga_ast_bind_stack_get_name(stack, &ast->type, ast->line);
    monga_ast_reference_check_kind(&ast->type, MONGA_AST_REFERENCE_TYPE, ast->line);
    if (ast->next)
        monga_ast_def_variable_bind(ast->next, stack);
}

void monga_ast_def_type_bind(struct monga_ast_def_type_t* ast, struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_reference_t* reference = construct(reference);
    reference->tag = MONGA_AST_REFERENCE_TYPE;
    reference->u.def_type = ast;
    reference->id = ast->id;
    monga_ast_bind_stack_insert_name(stack, reference);
    monga_ast_typedesc_bind(ast->typedesc, stack, false);
    monga_ast_typedesc_check_self_reference(ast->typedesc);
}

/* typedesc can be nullable if function doesn't return value */
void monga_ast_check_function_statements(struct monga_ast_statement_t* statement, struct monga_ast_typedesc_t* typedesc,
    struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* function)
{
    const char* function_name = function->id;
    size_t stmt_line = statement->line;
    switch (statement->tag) {
    case MONGA_AST_STATEMENT_IF:
    {
        struct monga_ast_block_t* then_block = statement->u.if_stmt.then_block;
        struct monga_ast_block_t* else_block = statement->u.if_stmt.else_block;
        if (then_block->statements)
            monga_ast_check_function_statements(then_block->statements->first, typedesc, stack, function);
        if (else_block && else_block->statements)
            monga_ast_check_function_statements(else_block->statements->first, typedesc, stack, function);
        break;
    }
    case MONGA_AST_STATEMENT_WHILE:
    {
        struct monga_ast_block_t* loop = statement->u.while_stmt.loop;
        if (loop->statements)
            monga_ast_check_function_statements(loop->statements->first, typedesc, stack, function);
        break;
    }
    case MONGA_AST_STATEMENT_ASSIGN:
        break;
    case MONGA_AST_STATEMENT_RETURN:
    {
        struct monga_ast_expression_t* exp = statement->u.return_stmt.exp;
        if (exp) {
            if (typedesc) {
                /* implicit assignment to eax register in assembler language */
                if (!monga_ast_typedesc_assignable(typedesc, exp->def_type->typedesc)) {
                    fprintf(stderr, "Returning expression of type ");
                    monga_ast_typedesc_write(stderr, exp->def_type->typedesc);
                    fprintf(stderr, " in function \"%s\" of return type ", function_name);
                    monga_ast_typedesc_write(stderr, typedesc);
                    fprintf(stderr, " (line %zu)\n", stmt_line);
                    exit(MONGA_ERR_TYPE);
                }
            } else {
                fprintf(stderr, "Returning expression in non-returning function \"%s\" (line %zu)\n",
                    function_name, stmt_line);
                exit(MONGA_ERR_TYPE);
            }
        } else {
            if (typedesc) {
                fprintf(stderr, "Expected expression of type ");
                monga_ast_typedesc_write(stderr, typedesc);
                fprintf(stderr, " in function \"%s\" (line %zu)\n", function_name, stmt_line);
                exit(MONGA_ERR_TYPE);
            }
        }
        break;
    }
    case MONGA_AST_STATEMENT_CALL:
        break;
    case MONGA_AST_STATEMENT_PRINT:
        break;
    case MONGA_AST_STATEMENT_BLOCK:
    {
        struct monga_ast_block_t* block = statement->u.block_stmt.block;
        if (block->statements)
            monga_ast_check_function_statements(block->statements->first, typedesc, stack, function);
        break;
    }
    default:
        monga_unreachable();
    }
    if (statement->next)
        monga_ast_check_function_statements(statement->next, typedesc, stack, function);
}

void monga_ast_def_function_bind(struct monga_ast_def_function_t* ast, struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_reference_t* reference = construct(reference);
    reference->tag = MONGA_AST_REFERENCE_FUNCTION;
    reference->u.def_function = ast;
    reference->id = ast->id;
    monga_ast_bind_stack_insert_name(stack, reference);
    if (ast->type.id) {
        monga_ast_bind_stack_get_name(stack, &ast->type, ast->line);
        monga_ast_reference_check_kind(&ast->type, MONGA_AST_REFERENCE_TYPE, ast->line);
    }
    monga_ast_bind_stack_block_enter(stack);
    if (ast->parameters)
        monga_ast_parameter_bind(ast->parameters->first, stack);
    monga_ast_block_bind(ast->block, stack);
    if (ast->block->statements) {
        struct monga_ast_typedesc_t* typedesc = ast->type.id ? ast->type.u.def_type->typedesc : NULL;
        monga_ast_check_function_statements(ast->block->statements->first, typedesc, stack, reference);
    }
    monga_ast_bind_stack_block_exit(stack);
}

void monga_ast_repeated_field_check(struct monga_ast_field_t* field)
{
    for (struct monga_ast_field_t* first = field; first; first = first->next) {
        for (struct monga_ast_field_t* second = first->next; second; second = second->next) {
            if (strcmp(first->id, second->id) == 0) {
                fprintf(stderr, "Duplicate field \"%s\" at line %zu\n", second->id, second->line);
                exit(MONGA_ERR_REDECLARATION);
            }
        }
    }
}

void monga_ast_typedesc_bind(struct monga_ast_typedesc_t* ast, struct monga_ast_bind_stack_t* stack, bool annonymous)
{
    switch (ast->tag) {
        case MONGA_AST_TYPEDESC_ID:
            monga_ast_bind_stack_get_name(stack, &ast->u.id_typedesc, ast->line);
            monga_ast_reference_check_kind(&ast->u.id_typedesc, MONGA_AST_REFERENCE_TYPE, ast->line);
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            monga_ast_typedesc_bind(ast->u.array_typedesc, stack, true);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_repeated_field_check(ast->u.record_typedesc->first);
            monga_ast_field_bind(ast->u.record_typedesc->first, stack);
            monga_ast_bind_stack_block_exit(stack);
            break;
        default:
            monga_unreachable();
    }
    if (annonymous)
        ast->annonymous_def_type = monga_ast_construct_annonymous_def_type(ast);
}

void monga_ast_field_bind(struct monga_ast_field_t* ast, struct monga_ast_bind_stack_t* stack)
{
    monga_ast_bind_stack_get_name(stack, &ast->type, ast->line);
    monga_ast_reference_check_kind(&ast->type, MONGA_AST_REFERENCE_TYPE, ast->line);
    if (ast->next)
        monga_ast_field_bind(ast->next, stack);
}

void monga_ast_parameter_bind(struct monga_ast_parameter_t* ast, struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_reference_t* reference = construct(reference);
    reference->tag = MONGA_AST_REFERENCE_PARAMETER;
    reference->u.parameter = ast;
    reference->id = ast->id;
    monga_ast_bind_stack_insert_name(stack, reference);
    monga_ast_bind_stack_get_name(stack, &ast->type, ast->line);
    monga_ast_reference_check_kind(&ast->type, MONGA_AST_REFERENCE_TYPE, ast->line);
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
            monga_ast_condition_bind(ast->u.if_stmt.cond, stack);
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_block_bind(ast->u.if_stmt.then_block, stack);
            monga_ast_bind_stack_block_exit(stack);
            if (ast->u.if_stmt.else_block) {
                monga_ast_bind_stack_block_enter(stack);
                monga_ast_block_bind(ast->u.if_stmt.else_block, stack);
                monga_ast_bind_stack_block_exit(stack);
            }
            break;
        case MONGA_AST_STATEMENT_WHILE:
            monga_ast_condition_bind(ast->u.while_stmt.cond, stack);
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_block_bind(ast->u.while_stmt.loop, stack);
            monga_ast_bind_stack_block_exit(stack);
            break;
        case MONGA_AST_STATEMENT_ASSIGN:
            monga_ast_variable_bind(ast->u.assign_stmt.var, stack);
            monga_ast_expression_bind(ast->u.assign_stmt.exp, stack);
            if (!monga_ast_typedesc_assignable(ast->u.assign_stmt.var->def_type->typedesc, ast->u.assign_stmt.exp->def_type->typedesc)) {
                fprintf(stderr, "Expected expression type ");
                monga_ast_typedesc_write(stderr, ast->u.assign_stmt.var->def_type->typedesc);
                fprintf(stderr, " instead of ");
                monga_ast_typedesc_write(stderr, ast->u.assign_stmt.exp->def_type->typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            break;
        case MONGA_AST_STATEMENT_RETURN:
            if (ast->u.return_stmt.exp)
                monga_ast_expression_bind(ast->u.return_stmt.exp, stack);
            break;
        case MONGA_AST_STATEMENT_CALL:
        {
            struct monga_ast_call_t* call = ast->u.call_stmt.call;
            struct monga_ast_reference_t* function = &call->function;
            monga_ast_call_bind(call, stack);
            monga_assert(function->tag == MONGA_AST_REFERENCE_FUNCTION);
            break;
        }
        case MONGA_AST_STATEMENT_PRINT:
            monga_ast_expression_bind(ast->u.print_stmt.exp, stack);
            break;
        case MONGA_AST_STATEMENT_BLOCK:
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_block_bind(ast->u.block_stmt.block, stack);
            monga_ast_bind_stack_block_exit(stack);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_statement_bind(ast->next, stack);
}

struct monga_ast_def_type_t* monga_ast_construct_annonymous_def_type(struct monga_ast_typedesc_t* typedesc)
{
    struct monga_ast_def_type_t *type = construct(def_type);
    char const* id;
    switch (typedesc->tag) {
    case MONGA_AST_TYPEDESC_BUILTIN:
        id = monga_ast_builtin_typedesc_id(typedesc->u.builtin_typedesc);
        break;
    case MONGA_AST_TYPEDESC_ID:
        id = typedesc->u.id_typedesc.id;
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
        id = "(annonymous array)";
        break;
    case MONGA_AST_TYPEDESC_RECORD:
        id = "(annonymous record)";
        break;
    default:
        monga_unreachable();
    }
    type->id = monga_memdup(id, strlen(id)+1);
    type->typedesc = typedesc;
    type->line = typedesc->line;
    return type;
}

struct monga_ast_typedesc_t* monga_ast_construct_annonymous_array_typedesc(struct monga_ast_typedesc_t* typedesc)
{
    struct monga_ast_typedesc_t* array_typedesc = construct(typedesc);
    array_typedesc->tag = MONGA_AST_TYPEDESC_ARRAY;
    array_typedesc->line = typedesc->line;
    array_typedesc->u.array_typedesc = typedesc;
    return array_typedesc;
}

void monga_ast_variable_bind(struct monga_ast_variable_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_VARIABLE_ID:
        {
            struct monga_ast_reference_t* type = NULL;
            struct monga_ast_reference_t* id_var = &ast->u.id_var;
            monga_ast_bind_stack_get_name(stack, id_var, ast->line);
            switch (ast->u.id_var.tag) {
            case MONGA_AST_REFERENCE_VARIABLE:
            {
                struct monga_ast_def_variable_t* def_variable = id_var->u.def_variable;
                type = &def_variable->type;
                break;
            }
            case MONGA_AST_REFERENCE_PARAMETER:
            {
                struct monga_ast_parameter_t* parameter = id_var->u.parameter;
                type = &parameter->type;
                break;
            }
            default:
                /* For the user, parameters are just local variables */
                monga_ast_reference_check_kind(&ast->u.id_var, MONGA_AST_REFERENCE_VARIABLE, ast->line);
                monga_unreachable();
            }
            monga_assert(type->tag == MONGA_AST_REFERENCE_TYPE);
            ast->def_type = type->u.def_type;
            break;
        }
        case MONGA_AST_VARIABLE_ARRAY:
        {
            /* array */
            {
                struct monga_ast_typedesc_t* typedesc;
                monga_ast_expression_bind(ast->u.array_var.array, stack);
                typedesc = monga_ast_typedesc_resolve_id(ast->u.array_var.array->def_type->typedesc);
                switch (typedesc->tag) {
                case MONGA_AST_TYPEDESC_BUILTIN:
                    fprintf(stderr, "Expected expression to be of array type and not \"%s\" (line %zu)\n",
                        monga_ast_builtin_typedesc_id(typedesc->u.builtin_typedesc), ast->line);
                    exit(MONGA_ERR_TYPE);
                    break;
                case MONGA_AST_TYPEDESC_ID:
                    monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                    break;
                case MONGA_AST_TYPEDESC_ARRAY:
                    ast->def_type = typedesc->u.array_typedesc->annonymous_def_type;
                    break;
                case MONGA_AST_TYPEDESC_RECORD:
                    fprintf(stderr, "Expected expression to be of array type and not record (line %zu)\n",
                        ast->line);
                    exit(MONGA_ERR_TYPE);
                    break;
                default:
                    monga_unreachable();
                }
            }
            /* index */
            {
                struct monga_ast_typedesc_t* typedesc;
                monga_ast_expression_bind(ast->u.array_var.index, stack);
                typedesc = monga_ast_typedesc_resolve_id(ast->u.array_var.index->def_type->typedesc);
                switch (typedesc->tag) {
                case MONGA_AST_TYPEDESC_BUILTIN:
                    if (typedesc->u.builtin_typedesc != MONGA_AST_TYPEDESC_BUILTIN_INT) {
                        fprintf(stderr, "Expected index to be of \"int\" type and not \"%s\" (line %zu)\n",
                            monga_ast_builtin_typedesc_id(typedesc->u.builtin_typedesc), ast->line);
                        exit(MONGA_ERR_TYPE);
                    }
                    break;
                case MONGA_AST_TYPEDESC_ID:
                    monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                    break;
                case MONGA_AST_TYPEDESC_ARRAY:
                    fprintf(stderr, "Expected index to be of \"int\" type and not array (line %zu)\n",
                        ast->line);
                    exit(MONGA_ERR_TYPE);
                    break;
                case MONGA_AST_TYPEDESC_RECORD:
                    fprintf(stderr, "Expected index to be of \"int\" type and not record (line %zu)\n",
                        ast->line);
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
            monga_ast_expression_bind(ast->u.record_var.record, stack);
            typedesc = monga_ast_typedesc_resolve_id(ast->u.record_var.record->def_type->typedesc);
            switch (typedesc->tag) {
            case MONGA_AST_TYPEDESC_BUILTIN:
                fprintf(stderr, "Expected expression to be of record type and not \"%s\" (line %zu)\n",
                    monga_ast_builtin_typedesc_id(typedesc->u.builtin_typedesc), ast->line);
                exit(MONGA_ERR_TYPE);
                break;
            case MONGA_AST_TYPEDESC_ID:
                monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                break;
            case MONGA_AST_TYPEDESC_ARRAY:
                fprintf(stderr, "Expected expression to be of record type and not array (line %zu)\n",
                    ast->line);
                exit(MONGA_ERR_TYPE);
                break;
            case MONGA_AST_TYPEDESC_RECORD:
            {
                struct monga_ast_field_t* field;
                bool found_field = false;
                for (field = typedesc->u.record_typedesc->first; field; field = field->next) {
                    if (strcmp(field->id, ast->u.record_var.field.id) == 0) {
                        struct monga_ast_reference_t* reference = &field->type;
                        ast->u.record_var.field.tag = MONGA_AST_REFERENCE_FIELD;
                        ast->u.record_var.field.u.field = field;
                        ast->def_type = reference->u.def_type;
                        found_field = true;
                        break;
                    }
                }
                if (!found_field) {
                    fprintf(stderr, "Could not find field \"%s\" in record (line %zu)\n",
                        ast->u.record_var.field.id, ast->line);
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
            ast->def_type = monga_ast_builtin_def_type(MONGA_AST_TYPEDESC_BUILTIN_INT);
            break;
        case MONGA_AST_EXPRESSION_REAL:
            ast->def_type = monga_ast_builtin_def_type(MONGA_AST_TYPEDESC_BUILTIN_FLOAT);
            break;
        case MONGA_AST_EXPRESSION_NULL:
            ast->def_type = monga_ast_builtin_def_type(MONGA_AST_TYPEDESC_BUILTIN_NULL);
            break;
        case MONGA_AST_EXPRESSION_VAR:
            monga_ast_variable_bind(ast->u.var_exp.var, stack);
            ast->def_type = ast->u.var_exp.var->def_type;
            break;
        case MONGA_AST_EXPRESSION_CALL:
        {
            struct monga_ast_reference_t *reference;
            struct monga_ast_def_function_t *def_function;
            monga_ast_call_bind(ast->u.call_exp.call, stack);
            reference = &ast->u.call_exp.call->function;
            def_function = reference->u.def_function;
            if (def_function->type.id == NULL) {
                fprintf(stderr, "Cannot use call to function \"%s\" as an expression, "
                                "because it does not return a value (line %zu)\n",
                    def_function->id, ast->line);
                exit(MONGA_ERR_NO_RETURN);
            }
            ast->def_type = def_function->type.u.def_type;
            break;
        }
        case MONGA_AST_EXPRESSION_CAST:
        {
            struct monga_ast_expression_t* exp = ast->u.cast_exp.exp;
            struct monga_ast_def_type_t* cast_def_type, *exp_def_type;
            monga_ast_expression_bind(exp, stack);
            monga_ast_bind_stack_get_name(stack, &ast->u.cast_exp.type, ast->line);
            monga_ast_reference_check_kind(&ast->u.cast_exp.type, MONGA_AST_REFERENCE_TYPE, ast->line);
            cast_def_type = ast->u.cast_exp.type.u.def_type;
            exp_def_type = exp->def_type;
            if (!monga_ast_typedesc_castable(cast_def_type->typedesc, exp_def_type->typedesc)) {
                fprintf(stderr, "Cannot cast expression of type ");
                monga_ast_typedesc_write(stderr, exp_def_type->typedesc);
                fprintf(stderr, " to type ");
                monga_ast_typedesc_write(stderr, cast_def_type->typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            ast->def_type = cast_def_type;
            break;
        }
        case MONGA_AST_EXPRESSION_NEW:
        {
            struct monga_ast_def_type_t* new_def_type;
            monga_ast_bind_stack_get_name(stack, &ast->u.new_exp.type, ast->line);
            monga_ast_reference_check_kind(&ast->u.new_exp.type, MONGA_AST_REFERENCE_TYPE, ast->line);
            new_def_type = ast->u.new_exp.type.u.def_type;
            if (ast->u.new_exp.exp) {
                /* exp */
                {
                    struct monga_ast_expression_t* exp = ast->u.new_exp.exp;
                    struct monga_ast_typedesc_t* typedesc;
                    monga_ast_expression_bind(exp, stack);
                    typedesc = exp->def_type->typedesc;
                    if (typedesc->tag != MONGA_AST_TYPEDESC_BUILTIN ||
                        typedesc->u.builtin_typedesc != MONGA_AST_TYPEDESC_BUILTIN_INT) {
                        fprintf(stderr, "Expected size to be of type \"int\" instead of ");
                        monga_ast_typedesc_write(stderr, typedesc);
                        fprintf(stderr, " (line %zu)\n", ast->line);
                        exit(MONGA_ERR_TYPE);
                    }
                }
                /* type */
                {
                    struct monga_ast_typedesc_t* typedesc;
                    typedesc = monga_ast_construct_annonymous_array_typedesc(new_def_type->typedesc);
                    ast->def_type = monga_ast_construct_annonymous_def_type(typedesc);
                }
            } else {
                ast->def_type = new_def_type;
            }
            break;
        }
        case MONGA_AST_EXPRESSION_NEGATIVE:
        {
            struct monga_ast_expression_t* exp = ast->u.negative_exp.exp;
            monga_ast_expression_bind(exp, stack);
            if (!monga_ast_typedesc_numeric(exp->def_type->typedesc)) {
                fprintf(stderr, "Type ");
                monga_ast_typedesc_write(stderr, exp->def_type->typedesc);
                fprintf(stderr, " is not numeric (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            ast->def_type = exp->def_type;
            break;
        }
        case MONGA_AST_EXPRESSION_ADDITION:
        case MONGA_AST_EXPRESSION_SUBTRACTION:
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
        case MONGA_AST_EXPRESSION_DIVISION:
        {
            struct monga_ast_expression_t* exp1 = ast->u.binop_exp.exp1;
            struct monga_ast_expression_t* exp2 = ast->u.binop_exp.exp2;
            monga_ast_expression_bind(exp1, stack);
            monga_ast_expression_bind(exp2, stack);
            if (!monga_ast_typedesc_numeric(exp1->def_type->typedesc)) {
                fprintf(stderr, "Left operand is of type ");
                monga_ast_typedesc_write(stderr, exp1->def_type->typedesc);
                fprintf(stderr, " which is not numeric (line %zu)\n", exp1->line);
                exit(MONGA_ERR_TYPE);
            }
            if (!monga_ast_typedesc_numeric(exp2->def_type->typedesc)) {
                fprintf(stderr, "Right operand is of type ");
                monga_ast_typedesc_write(stderr, exp2->def_type->typedesc);
                fprintf(stderr, " which is not numeric (line %zu)\n", exp2->line);
                exit(MONGA_ERR_TYPE);
            }
            if (monga_ast_typedesc_equal(exp1->def_type->typedesc,
                                         exp2->def_type->typedesc)) {
                ast->def_type = exp1->def_type;
            } else {
                fprintf(stderr, "Binary operation expression between ");
                monga_ast_typedesc_write(stderr, exp1->def_type->typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, exp2->def_type->typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            break;
        }
        case MONGA_AST_EXPRESSION_CONDITIONAL:
        {
            struct monga_ast_typedesc_t* true_typedesc;
            struct monga_ast_typedesc_t* false_typedesc;
            struct monga_ast_typedesc_t* parent_typedesc;

            monga_ast_condition_bind(ast->u.conditional_exp.cond, stack);
            monga_ast_expression_bind(ast->u.conditional_exp.true_exp, stack);
            monga_ast_expression_bind(ast->u.conditional_exp.false_exp, stack);
            
            true_typedesc = ast->u.conditional_exp.true_exp->def_type->typedesc;
            false_typedesc = ast->u.conditional_exp.false_exp->def_type->typedesc;
            parent_typedesc = monga_ast_typedesc_parent(true_typedesc, false_typedesc);
            
            if (parent_typedesc != NULL) {
                if (parent_typedesc == true_typedesc) {
                    ast->def_type = ast->u.conditional_exp.true_exp->def_type;
                } else if (parent_typedesc == false_typedesc) {
                    ast->def_type = ast->u.conditional_exp.false_exp->def_type;
                } else {
                    ast->def_type = monga_ast_construct_annonymous_def_type(parent_typedesc);
                }
            } else {
                fprintf(stderr, "Conditional expression with two possible types: ");
                monga_ast_typedesc_write(stderr, true_typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, false_typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            break;
        }
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
        {
            struct monga_ast_expression_t* exp1 = ast->u.exp_binop_cond.exp1;
            struct monga_ast_expression_t* exp2 = ast->u.exp_binop_cond.exp2;

            monga_ast_expression_bind(exp1, stack);
            monga_ast_expression_bind(exp2, stack);
            
            if (!monga_ast_typedesc_sibling(exp1->def_type->typedesc, exp2->def_type->typedesc)) {
                fprintf(stderr, "Binary operation condition between ");
                monga_ast_typedesc_write(stderr, exp1->def_type->typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, exp2->def_type->typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            break;
        }
        case MONGA_AST_CONDITION_AND:
        case MONGA_AST_CONDITION_OR:
            monga_ast_condition_bind(ast->u.cond_binop_cond.cond1, stack);
            monga_ast_condition_bind(ast->u.cond_binop_cond.cond2, stack);
            break;
        case MONGA_AST_CONDITION_NOT:
            monga_ast_condition_bind(ast->u.cond_unop_cond.cond, stack);
            break;
        default:
            monga_unreachable();
    }
}

void monga_ast_call_parameters_bind(struct monga_ast_call_t* call, struct monga_ast_parameter_t* parameter,
    struct monga_ast_expression_t* expression, struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_def_function_t* def_function = call->function.u.def_function;
    if (parameter && expression) {
        struct monga_ast_reference_t* parameter_type = &parameter->type;
        if (!monga_ast_typedesc_assignable(parameter_type->u.def_type->typedesc, expression->def_type->typedesc)) {
            fprintf(stderr, "Expected argument \"%s\" to be of type \"%s\" in call to function \"%s\" (line %zu)\n",
                parameter->id, parameter->type.u.def_type->id, def_function->id, expression->line);
            exit(MONGA_ERR_TYPE);
        }
        monga_ast_call_parameters_bind(call, parameter->next, expression->next, stack);
    } else if (parameter == NULL && expression == NULL) {
        return; /* end of parameter list */
    } else if (parameter == NULL && expression) {
        fprintf(stderr, "Provided too many arguments in call to function \"%s\" (line %zu)\n",
            def_function->id, call->line);
        exit(MONGA_ERR_SIGNATURE);
    } else { /* parameter && expression == NULL */
        fprintf(stderr, "Provided too few arguments in call to function \"%s\" (line %zu)\n",
            def_function->id, call->line);
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

    monga_ast_bind_stack_get_name(stack, function, ast->line);
    monga_ast_reference_check_kind(function, MONGA_AST_REFERENCE_FUNCTION, ast->line);

    def_function = function->u.def_function;

    if (ast->expressions)
        monga_ast_expression_bind(ast->expressions->first, stack);

    expressions = ast->expressions ? ast->expressions->first : NULL;
    parameters = def_function->parameters ? def_function->parameters->first : NULL;

    monga_ast_call_parameters_bind(ast, parameters, expressions, stack);
}