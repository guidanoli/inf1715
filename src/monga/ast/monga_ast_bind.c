#include "monga_ast_bind.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"
#include "monga_ast_reference.h"

/* Function declarations */

static void monga_ast_repeated_field_check(struct monga_ast_field_t* field);
static void monga_ast_typedesc_check_self_reference(struct monga_ast_typedesc_t* typedesc, struct monga_ast_bind_stack_t* stack);
static struct monga_ast_typedesc_t* monga_ast_typedesc_resolve_id(struct monga_ast_typedesc_t *typedesc, struct monga_ast_bind_stack_t* stack);
static bool monga_ast_typedesc_can_cast(struct monga_ast_typedesc_t *base, struct monga_ast_typedesc_t *target, struct monga_ast_bind_stack_t* stack);
static bool monga_ast_typedesc_equal(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2, struct monga_ast_bind_stack_t* stack);
static void monga_ast_call_parameters_bind(struct monga_ast_call_t* call, struct monga_ast_parameter_t* parameter, struct monga_ast_expression_t* expression, struct monga_ast_bind_stack_t* stack);
static void monga_ast_check_function_statements(struct monga_ast_statement_t* statement, struct monga_ast_typedesc_t* typedesc, struct monga_ast_bind_stack_t* stack, struct monga_ast_reference_t* function);

/* Function definitions */

struct monga_ast_typedesc_t* monga_ast_typedesc_resolve_id(struct monga_ast_typedesc_t *typedesc, struct monga_ast_bind_stack_t* stack)
{
    while (typedesc->tag == MONGA_AST_TYPEDESC_ID) {
        struct monga_ast_reference_t* id_typedesc = &typedesc->id_typedesc;
        monga_assert(id_typedesc->tag == MONGA_AST_REFERENCE_TYPE);
        typedesc = id_typedesc->u.def_type->typedesc;
    }
    return typedesc;
}

static void monga_ast_typedesc_check_self_reference(struct monga_ast_typedesc_t* typedesc, struct monga_ast_bind_stack_t* stack)
{
    struct monga_ast_def_type_t* def_type = NULL; /* definition of self-referecing type */

    switch (typedesc->tag) {
    case MONGA_AST_TYPEDESC_BUILTIN:
        break; /* built-in types never reference each other */
    case MONGA_AST_TYPEDESC_ID:
    {
        struct monga_ast_reference_t* reference = &typedesc->id_typedesc;
        monga_assert(reference->tag == MONGA_AST_REFERENCE_TYPE);

        def_type = reference->u.def_type;
        break;
    }
    case MONGA_AST_TYPEDESC_ARRAY:
    {
        struct monga_ast_typedesc_t* haystack = typedesc;
        
        while (haystack->tag != MONGA_AST_TYPEDESC_ARRAY) {
            haystack = haystack->array_typedesc;
        }
        
        if (haystack->tag == MONGA_AST_TYPEDESC_ID) {
            struct monga_ast_reference_t* reference = &typedesc->id_typedesc;
            monga_assert(reference->tag == MONGA_AST_REFERENCE_TYPE);
            def_type = reference->u.def_type;
        }
        break;
    }
    case MONGA_AST_TYPEDESC_RECORD:
        break; /* self reference in records is allowed */
    default:
        monga_unreachable();
    }

    if (def_type != NULL && def_type->typedesc == typedesc) {
        fprintf(stderr, "Type \"%s\" references itself (line %zu)\n", def_type->id, def_type->line);
        exit(MONGA_ERR_REDECLARATION);
    }
}

static bool monga_ast_typedesc_can_cast(struct monga_ast_typedesc_t *base, struct monga_ast_typedesc_t *target, struct monga_ast_bind_stack_t* stack)
{
    base = monga_ast_typedesc_resolve_id(base, stack);
    target = monga_ast_typedesc_resolve_id(target, stack);
    if (base->tag == target->tag) {
        switch (base->tag) {
        case MONGA_AST_TYPEDESC_BUILTIN:
            return true; /* float <-> int */
        case MONGA_AST_TYPEDESC_ID:
            monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            return monga_ast_typedesc_equal(base->array_typedesc, target->array_typedesc, stack); /* sizeof(float) != sizeof(int) */
        case MONGA_AST_TYPEDESC_RECORD:
            return base == target; /* same type definition */
        default:
            monga_unreachable();
        }
    } else {
        return false;
    }
}

bool monga_ast_typedesc_equal(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2, struct monga_ast_bind_stack_t* stack)
{
    typedesc1 = monga_ast_typedesc_resolve_id(typedesc1, stack);
    typedesc2 = monga_ast_typedesc_resolve_id(typedesc2, stack);
    if (typedesc1->tag == typedesc2->tag) {
        switch (typedesc1->tag) {
        case MONGA_AST_TYPEDESC_BUILTIN:
            return typedesc1->builtin_typedesc == typedesc2->builtin_typedesc; /* same built-in type */
        case MONGA_AST_TYPEDESC_ID:
            monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            return monga_ast_typedesc_equal(typedesc1->array_typedesc, typedesc2->array_typedesc, stack); /* propagate to subtype */
        case MONGA_AST_TYPEDESC_RECORD:
            return typedesc1 == typedesc2; /* same type definition */
        default:
            monga_unreachable();
        }
    } else {
        return false;
    }
}

void monga_ast_program_bind(struct monga_ast_program_t* ast)
{
    if (ast->definitions) {
        struct monga_ast_bind_stack_t* stack = monga_ast_bind_stack_create();
        for (enum monga_ast_typedesc_builtin_t builtin = 0; builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT; ++builtin) {
            struct monga_ast_def_type_t* def_type = monga_ast_builtin_def_type(builtin);
            struct monga_ast_reference_t* reference = construct(reference);
            reference->tag = MONGA_AST_REFERENCE_TYPE;
            reference->u.def_type = def_type;
            reference->id = def_type->id;
            monga_ast_bind_stack_insert_name(stack, reference);
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
    monga_ast_typedesc_bind(ast->typedesc, stack);
    monga_ast_typedesc_check_self_reference(ast->typedesc, stack);
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
        struct monga_ast_block_t* then_block = statement->if_stmt.then_block;
        struct monga_ast_block_t* else_block = statement->if_stmt.else_block;
        if (then_block->statements)
            monga_ast_check_function_statements(then_block->statements->first, typedesc, stack, function);
        if (else_block && else_block->statements)
            monga_ast_check_function_statements(else_block->statements->first, typedesc, stack, function);
        break;
    }
    case MONGA_AST_STATEMENT_WHILE:
    {
        struct monga_ast_block_t* loop = statement->while_stmt.loop;
        if (loop->statements)
            monga_ast_check_function_statements(loop->statements->first, typedesc, stack, function);
        break;
    }
    case MONGA_AST_STATEMENT_ASSIGN:
        break;
    case MONGA_AST_STATEMENT_RETURN:
    {
        struct monga_ast_expression_t* exp = statement->return_stmt.exp;
        if (exp) {
            if (typedesc) {
                if (!monga_ast_typedesc_equal(exp->typedesc, typedesc, stack)) {
                    fprintf(stderr, "Returning expression of type ");
                    monga_ast_typedesc_write(stderr, exp->typedesc);
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
        struct monga_ast_block_t* block = statement->block_stmt.block;
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

void monga_ast_typedesc_bind(struct monga_ast_typedesc_t* ast, struct monga_ast_bind_stack_t* stack)
{
    switch (ast->tag) {
        case MONGA_AST_TYPEDESC_ID:
            monga_ast_bind_stack_get_name(stack, &ast->id_typedesc, ast->line);
            monga_ast_reference_check_kind(&ast->id_typedesc, MONGA_AST_REFERENCE_TYPE, ast->line);
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            monga_ast_typedesc_bind(ast->array_typedesc, stack);
            break;
        case MONGA_AST_TYPEDESC_RECORD:
            monga_ast_bind_stack_block_enter(stack);
            monga_ast_repeated_field_check(ast->record_typedesc->first);
            monga_ast_field_bind(ast->record_typedesc->first, stack);
            monga_ast_bind_stack_block_exit(stack);
            break;
        default:
            monga_unreachable();
    }
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
            if (!monga_ast_typedesc_equal(ast->assign_stmt.var->typedesc, ast->assign_stmt.exp->typedesc, stack)) {
                fprintf(stderr, "Expected expression type ");
                monga_ast_typedesc_write(stderr, ast->assign_stmt.var->typedesc);
                fprintf(stderr, " instead of ");
                monga_ast_typedesc_write(stderr, ast->assign_stmt.exp->typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            break;
        case MONGA_AST_STATEMENT_RETURN:
            if (ast->return_stmt.exp)
                monga_ast_expression_bind(ast->return_stmt.exp, stack);
            break;
        case MONGA_AST_STATEMENT_CALL:
        {
            struct monga_ast_call_t* call = ast->call_stmt.call;
            struct monga_ast_reference_t* function = &call->function;
            struct monga_ast_def_function_t* def_function;
            monga_ast_call_bind(call, stack);
            monga_assert(function->tag == MONGA_AST_REFERENCE_FUNCTION);
            def_function = function->u.def_function;
            break;
        }
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
            monga_ast_bind_stack_get_name(stack, id_var, ast->line);
            switch (ast->id_var.tag) {
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
                monga_ast_reference_check_kind(&ast->id_var, MONGA_AST_REFERENCE_VARIABLE, ast->line);
            }
            monga_assert(type->tag == MONGA_AST_REFERENCE_TYPE);
            ast->typedesc = type->u.def_type->typedesc;
            break;
        }
        case MONGA_AST_VARIABLE_ARRAY:
        {
            /* array */
            {
                struct monga_ast_typedesc_t* typedesc;
                monga_ast_expression_bind(ast->array_var.array, stack);
                typedesc = monga_ast_typedesc_resolve_id(ast->array_var.array->typedesc, stack);
                switch (typedesc->tag) {
                case MONGA_AST_TYPEDESC_BUILTIN:
                    fprintf(stderr, "Expected expression to be of array type and not \"%s\" (line %zu)\n",
                        monga_ast_builtin_typedesc_id(typedesc->builtin_typedesc), ast->line);
                    exit(MONGA_ERR_TYPE);
                    break;
                case MONGA_AST_TYPEDESC_ID:
                    monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                    break;
                case MONGA_AST_TYPEDESC_ARRAY:
                    typedesc = typedesc->array_typedesc;
                    break;
                case MONGA_AST_TYPEDESC_RECORD:
                    fprintf(stderr, "Expected expression to be of array type and not record (line %zu)\n",
                        ast->line);
                    exit(MONGA_ERR_TYPE);
                    break;
                default:
                    monga_unreachable();
                }
                ast->typedesc = typedesc;
            }
            /* index */
            {
                struct monga_ast_typedesc_t* typedesc;
                monga_ast_expression_bind(ast->array_var.index, stack);
                typedesc = monga_ast_typedesc_resolve_id(ast->array_var.index->typedesc, stack);
                switch (typedesc->tag) {
                case MONGA_AST_TYPEDESC_BUILTIN:
                    if (typedesc->builtin_typedesc != MONGA_AST_TYPEDESC_BUILTIN_INT) {
                        fprintf(stderr, "Expected index to be of \"int\" type and not \"%s\" (line %zu)\n",
                            monga_ast_builtin_typedesc_id(typedesc->builtin_typedesc), ast->line);
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
            monga_ast_expression_bind(ast->record_var.record, stack);
            typedesc = monga_ast_typedesc_resolve_id(ast->record_var.record->typedesc, stack);
            switch (typedesc->tag) {
            case MONGA_AST_TYPEDESC_BUILTIN:
                fprintf(stderr, "Expected expression to be of record type and not \"%s\" (line %zu)\n",
                    monga_ast_builtin_typedesc_id(typedesc->builtin_typedesc), ast->line);
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
                for (field = typedesc->record_typedesc->first; field; field = field->next) {
                    if (strcmp(field->id, ast->record_var.field.id) == 0) {
                        struct monga_ast_reference_t* reference = &field->type;
                        ast->record_var.field.tag = MONGA_AST_REFERENCE_FIELD;
                        ast->record_var.field.u.field = field;
                        ast->typedesc = reference->u.def_type->typedesc;
                        found_field = true;
                        break;
                    }
                }
                if (!found_field) {
                    fprintf(stderr, "Could not find field \"%s\" in record (line %zu)\n",
                        ast->record_var.field.id, ast->line);
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
            ast->typedesc = monga_ast_builtin_typedesc(MONGA_AST_TYPEDESC_BUILTIN_INT);
            break;
        case MONGA_AST_EXPRESSION_REAL:
            ast->typedesc = monga_ast_builtin_typedesc(MONGA_AST_TYPEDESC_BUILTIN_FLOAT);
            break;
        case MONGA_AST_EXPRESSION_VAR:
            monga_ast_variable_bind(ast->var_exp.var, stack);
            ast->typedesc = ast->var_exp.var->typedesc;
            break;
        case MONGA_AST_EXPRESSION_CALL:
        {
            struct monga_ast_reference_t *reference;
            struct monga_ast_def_function_t *def_function;
            monga_ast_call_bind(ast->call_exp.call, stack);
            reference = &ast->call_exp.call->function;
            def_function = reference->u.def_function;
            if (def_function->type.id == NULL) {
                fprintf(stderr, "Cannot use call to function \"%s\" as an expression, "
                                "because it does not return a value (line %zu)\n",
                    def_function->id, ast->line);
                exit(MONGA_ERR_NO_RETURN);
            }
            ast->typedesc = def_function->type.u.def_type->typedesc;
            break;
        }
        case MONGA_AST_EXPRESSION_CAST:
        {
            struct monga_ast_expression_t* exp = ast->cast_exp.exp;
            struct monga_ast_typedesc_t* cast_typedesc;
            monga_ast_expression_bind(exp, stack);
            monga_ast_bind_stack_get_name(stack, &ast->cast_exp.type, ast->line);
            monga_ast_reference_check_kind(&ast->cast_exp.type, MONGA_AST_REFERENCE_TYPE, ast->line);
            cast_typedesc = ast->cast_exp.type.u.def_type->typedesc;
            if (!monga_ast_typedesc_can_cast(exp->typedesc, cast_typedesc, stack)) {
                fprintf(stderr, "Cannot cast expression of type ");
                monga_ast_typedesc_write(stderr, exp->typedesc);
                fprintf(stderr, " to type ");
                monga_ast_typedesc_write(stderr, cast_typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            ast->typedesc = cast_typedesc;
            break;
        }
        case MONGA_AST_EXPRESSION_NEW:
            monga_ast_bind_stack_get_name(stack, &ast->new_exp.type, ast->line);
            monga_ast_reference_check_kind(&ast->new_exp.type, MONGA_AST_REFERENCE_TYPE, ast->line);
            if (ast->new_exp.exp) {
                /* exp */
                {
                    struct monga_ast_expression_t* exp = ast->new_exp.exp;
                    struct monga_ast_typedesc_t* typedesc;
                    monga_ast_expression_bind(exp, stack);
                    typedesc = exp->typedesc;
                    if (typedesc->tag != MONGA_AST_TYPEDESC_BUILTIN ||
                        typedesc->builtin_typedesc != MONGA_AST_TYPEDESC_BUILTIN_INT) {
                        fprintf(stderr, "Expected size to be of type \"int\" instead of ");
                        monga_ast_typedesc_write(stderr, typedesc);
                        fprintf(stderr, " (line %zu)\n", ast->line);
                        exit(MONGA_ERR_TYPE);
                    }
                }
                /* type */
                {
                    ast->typedesc = construct(typedesc);
                    monga_ast_typedesc_make_array(ast->new_exp.type.u.def_type->typedesc, ast->typedesc);
                }
            } else {
                ast->typedesc = ast->new_exp.type.u.def_type->typedesc;
            }
            break;
        case MONGA_AST_EXPRESSION_NEGATIVE:
            monga_ast_expression_bind(ast->negative_exp.exp, stack);
            ast->typedesc = ast->negative_exp.exp->typedesc;
            break;
        case MONGA_AST_EXPRESSION_ADDITION:
        case MONGA_AST_EXPRESSION_SUBTRACTION:
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
        case MONGA_AST_EXPRESSION_DIVISION:
            monga_ast_expression_bind(ast->binop_exp.exp1, stack);
            monga_ast_expression_bind(ast->binop_exp.exp2, stack);
            if (monga_ast_typedesc_equal(ast->binop_exp.exp1->typedesc,
                                         ast->binop_exp.exp2->typedesc, stack)) {
                ast->typedesc = ast->binop_exp.exp1->typedesc;
            } else {
                fprintf(stderr, "Binary operation expression between ");
                monga_ast_typedesc_write(stderr, ast->binop_exp.exp1->typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, ast->binop_exp.exp2->typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            break;
        case MONGA_AST_EXPRESSION_CONDITIONAL:
            monga_ast_condition_bind(ast->conditional_exp.cond, stack);
            monga_ast_expression_bind(ast->conditional_exp.true_exp, stack);
            monga_ast_expression_bind(ast->conditional_exp.false_exp, stack);
            if (monga_ast_typedesc_equal(ast->conditional_exp.true_exp->typedesc,
                                         ast->conditional_exp.false_exp->typedesc, stack)) {
                ast->typedesc = ast->conditional_exp.true_exp->typedesc;
            } else {
                fprintf(stderr, "Conditional expression with two possible types: ");
                monga_ast_typedesc_write(stderr, ast->conditional_exp.true_exp->typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, ast->conditional_exp.false_exp->typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
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
            if (!monga_ast_typedesc_equal(ast->exp_binop_cond.exp1->typedesc,
                                          ast->exp_binop_cond.exp2->typedesc, stack)) {
                fprintf(stderr, "Binary operation condition between ");
                monga_ast_typedesc_write(stderr, ast->exp_binop_cond.exp1->typedesc);
                fprintf(stderr, " and ");
                monga_ast_typedesc_write(stderr, ast->exp_binop_cond.exp2->typedesc);
                fprintf(stderr, " (line %zu)\n", ast->line);
                exit(MONGA_ERR_TYPE);
            }
            break;
        case MONGA_AST_CONDITION_AND:
        case MONGA_AST_CONDITION_OR:
            monga_ast_condition_bind(ast->cond_binop_cond.cond1, stack);
            monga_ast_condition_bind(ast->cond_binop_cond.cond2, stack);
            break;
        case MONGA_AST_CONDITION_NOT:
            monga_ast_condition_bind(ast->cond_unop_cond.cond, stack);
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
        if (!monga_ast_typedesc_equal(parameter_type->u.def_type->typedesc, expression->typedesc, stack)) {
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