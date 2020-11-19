#include "monga_ast_llvm.h"
#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"

#include <stdio.h>

typedef struct monga_ast_typedesc_t* (typedesc_getter)(void*);
typedef void* (next_getter)(void*);
typedef void (visiter)(void*, void*);

static void monga_ast_typedesc_reference_llvm(struct monga_ast_typedesc_t* ast);
static void monga_ast_typedesc_reference_list_llvm(void* node, typedesc_getter get_node_typedesc,
    next_getter get_next_node, visiter visit_before, visiter visit_after, void* visit_arg);

static struct monga_ast_typedesc_t* field_typedesc_getter(void* field);
static void* field_next_getter(void *field);

static struct monga_ast_typedesc_t* parameter_typedesc_getter(void* parameter);
static void* parameter_next_getter(void *parameter);
static void parameter_visit_after(void* parameter, void* arg);

static void monga_ast_parameter_allocation_llvm(struct monga_ast_parameter_t* parameter, size_t* var_count_ptr);
static void monga_ast_parameter_store_llvm(struct monga_ast_parameter_t* parameter, size_t parameter_count);

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

void monga_ast_def_variable_llvm(struct monga_ast_def_variable_t* ast)
{
    printf("@%s = internal global ", ast->id);
    monga_ast_typedesc_reference_llvm(ast->type.u.def_type->typedesc);
    printf(" undef\n");
}

void monga_ast_typedesc_reference_list_llvm(void* node, typedesc_getter get_node_typedesc,
    next_getter get_next_node, visiter visit_before, visiter visit_after, void* visit_arg)
{
    struct monga_ast_typedesc_t* typedesc;
    while (node != NULL) {
        typedesc = get_node_typedesc(node);
        
        if (visit_before != NULL)
            visit_before(node, visit_arg);
        
        monga_ast_typedesc_reference_llvm(typedesc);

        if (visit_after != NULL)
            visit_after(node, visit_arg);
        
        node = get_next_node(node);
        if (node != NULL)
            printf(", ");
    }
}

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

void monga_ast_def_function_return_llvm(struct monga_ast_def_function_t* ast)
{
    if (ast->type.id != NULL) {
        monga_ast_typedesc_reference_llvm(ast->type.u.def_type->typedesc);
    } else {
        printf("void");
    }
}

void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast)
{
    size_t var_count = 0, parameter_count;

    printf("define ");
    monga_ast_def_function_return_llvm(ast);
    printf(" @%s(", ast->id);

    if (ast->parameters != NULL) {
        monga_ast_typedesc_reference_list_llvm(ast->parameters->first,
            parameter_typedesc_getter, parameter_next_getter, NULL,
            parameter_visit_after, &var_count);
    }

    /* monga_ast_typedesc_reference_list_llvm increased var_count
       for every parameter in the parameter list */
    parameter_count = var_count;

    printf(") {\n");

    if (ast->parameters != NULL) {
        monga_ast_parameter_allocation_llvm(ast->parameters->first, &var_count);
        monga_ast_parameter_store_llvm(ast->parameters->first, parameter_count);
    }

    monga_ast_block_llvm(ast->block, &var_count, ast->type.u.def_type->typedesc);

    printf("\tret ");
    monga_ast_def_function_return_llvm(ast);

    if (ast->type.id)
        printf(" undef");
    
    printf("\n}\n");
}

void monga_ast_parameter_allocation_llvm(struct monga_ast_parameter_t* parameter, size_t* var_count_ptr)
{
    struct monga_ast_typedesc_t* typedesc = parameter_typedesc_getter(parameter);

    printf("\t%%t%zu = alloca ", *var_count_ptr);
    monga_ast_typedesc_reference_llvm(typedesc);
    putc('\n', stdout);

    *var_count_ptr += 1;

    if (parameter->next)
        monga_ast_parameter_allocation_llvm(parameter->next, var_count_ptr);
}

void monga_ast_parameter_store_llvm(struct monga_ast_parameter_t* parameter, size_t parameter_count)
{
    struct monga_ast_typedesc_t* typedesc = parameter_typedesc_getter(parameter);

    printf("\tstore ");
    monga_ast_typedesc_reference_llvm(typedesc);
    printf(" %%t%zu, ", parameter->llvm_var_id);
    monga_ast_typedesc_reference_llvm(typedesc);
    printf("* %%t%zu\n", parameter->llvm_var_id + parameter_count);

    parameter->llvm_var_id += parameter_count; /* update id */

    if (parameter->next)
        monga_ast_parameter_store_llvm(parameter->next, parameter_count);
}

void monga_ast_def_variable_allocation_llvm(struct monga_ast_def_variable_t* def_variable, size_t* var_count_ptr)
{
    struct monga_ast_typedesc_t* typedesc = def_variable->type.u.def_type->typedesc;

    printf("\t%%t%zu = alloca ", *var_count_ptr);
    monga_ast_typedesc_reference_llvm(typedesc);
    putc('\n', stdout);

    def_variable->llvm_var_id = *var_count_ptr;
    *var_count_ptr += 1;

    if (def_variable->next)
        monga_ast_def_variable_allocation_llvm(def_variable->next, var_count_ptr);
}

struct monga_ast_typedesc_t* field_typedesc_getter(void* field)
{
    struct monga_ast_field_t* field_ = (struct monga_ast_field_t*) field;
    return field_->type.u.def_type->typedesc;
}

void* field_next_getter(void *field)
{
    struct monga_ast_field_t* field_ = (struct monga_ast_field_t*) field;
    return field_->next;
}

struct monga_ast_typedesc_t* parameter_typedesc_getter(void* parameter)
{
    struct monga_ast_parameter_t* parameter_ = (struct monga_ast_parameter_t*) parameter;
    return parameter_->type.u.def_type->typedesc;
}

void* parameter_next_getter(void *parameter)
{
    struct monga_ast_parameter_t* parameter_ = (struct monga_ast_parameter_t*) parameter;
    return parameter_->next;
}

void parameter_visit_after(void* parameter, void* arg)
{
    struct monga_ast_parameter_t* parameter_ = (struct monga_ast_parameter_t*) parameter;
    size_t* var_count = (size_t*) arg;
    parameter_->llvm_var_id = *var_count;
    printf(" %%t%zu", parameter_->llvm_var_id);
    *var_count += 1;
}

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
        struct monga_ast_field_t* field;
        ast->u.record_typedesc.llvm_struct_id = *struct_count_ptr;
        printf("%%S%zu = type { ", ast->u.record_typedesc.llvm_struct_id);
        field = ast->u.record_typedesc.field_list->first;
        monga_ast_typedesc_reference_list_llvm(field,
            field_typedesc_getter, field_next_getter, NULL, NULL, NULL);
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

void monga_ast_block_llvm(struct monga_ast_block_t* ast, size_t* var_count_ptr, struct monga_ast_typedesc_t* ret_typedesc)
{
    if (ast->variables)
        monga_ast_def_variable_allocation_llvm(ast->variables->first, var_count_ptr);
    if (ast->statements)
        monga_ast_statement_llvm(ast->statements->first, var_count_ptr, ret_typedesc);
}

void monga_ast_statement_llvm(struct monga_ast_statement_t* ast, size_t* var_count_ptr, struct monga_ast_typedesc_t* ret_typedesc)
{
    switch (ast->tag) {
        case MONGA_AST_STATEMENT_IF:
            break;
        case MONGA_AST_STATEMENT_WHILE:
            break;
        case MONGA_AST_STATEMENT_ASSIGN:
        {
            struct monga_ast_variable_t* var = ast->u.assign_stmt.var;
            struct monga_ast_expression_t* exp = ast->u.assign_stmt.exp;

            monga_ast_variable_llvm(var, var_count_ptr);
            monga_ast_expression_llvm(exp, var_count_ptr);

            printf("store ");
            monga_ast_typedesc_reference_llvm(var->typedesc);
            printf(", ");
            monga_ast_typedesc_reference_llvm(var->typedesc);
            printf("* ");

            break;
        }
        case MONGA_AST_STATEMENT_RETURN:
        {
            struct monga_ast_expression_t* exp;
            exp = ast->u.return_stmt.exp;
            if (exp != NULL) {
                monga_ast_expression_llvm(ast->u.return_stmt.exp, var_count_ptr);
                printf("\tret ");
                monga_ast_typedesc_reference_llvm(ret_typedesc);
                printf(" %%t%zu\n", exp->llvm_var_id);
            } else {
                printf("\tret void\n");
            }
            break;
        }
        case MONGA_AST_STATEMENT_CALL:
            break;
        case MONGA_AST_STATEMENT_PRINT:
            break;
        case MONGA_AST_STATEMENT_BLOCK:
            monga_ast_block_llvm(ast->u.block_stmt.block, var_count_ptr, ret_typedesc);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_statement_llvm(ast->next, var_count_ptr, ret_typedesc);
}

size_t monga_ast_get_field_index(struct monga_ast_field_list_t* field_list, struct monga_ast_field_t* field)
{
    size_t index = 0;

    for (struct monga_ast_field_t* f = field_list->first;
        f != NULL && f != field;
        f = f->next, ++index);

    return index;
}

void monga_ast_variable_llvm(struct monga_ast_variable_t* ast, size_t* var_count_ptr)
{
    switch (ast->tag) {
        case MONGA_AST_VARIABLE_ID:
        {
            struct monga_ast_reference_t* id_reference;
            id_reference = &ast->u.id_var;
            switch (id_reference->tag) {
            case MONGA_AST_REFERENCE_VARIABLE:
            {
                struct monga_ast_def_variable_t* def_variable;
                struct monga_ast_typedesc_t* typedesc;

                def_variable = id_reference->u.def_variable;
                typedesc = def_variable->type.u.def_type->typedesc;
                
                if (def_variable->is_global) {
                    printf("\t%%t%zu = getelementptr ", *var_count_ptr);
                    monga_ast_typedesc_reference_llvm(typedesc);
                    printf(", ");
                    monga_ast_typedesc_reference_llvm(typedesc);
                    printf("* @%s, i32 0\n", def_variable->id);
                    ast->llvm_var_id = *var_count_ptr;
                    *var_count_ptr += 1;
                } else {
                    ast->llvm_var_id = def_variable->llvm_var_id;
                }
                break;
            }
            case MONGA_AST_REFERENCE_PARAMETER:
            {
                struct monga_ast_parameter_t* parameter;
                parameter = id_reference->u.parameter;
                ast->llvm_var_id = parameter->llvm_var_id;
                break;
            }
            default:
                monga_unreachable();
            }
            break;
        }
        case MONGA_AST_VARIABLE_ARRAY:
        {
            struct monga_ast_expression_t *array_exp, *index_exp;
            array_exp = ast->u.array_var.array;
            index_exp = ast->u.array_var.index;
            
            monga_ast_expression_llvm(array_exp, var_count_ptr);
            monga_ast_expression_llvm(index_exp, var_count_ptr);

            monga_assert(index_exp->typedesc->tag == MONGA_AST_TYPEDESC_BUILTIN &&
                index_exp->typedesc->u.builtin_typedesc == MONGA_AST_TYPEDESC_BUILTIN_INT);
            printf("\t%%t%zu = sext i32 %%t%zu to i64\n", *var_count_ptr, index_exp->llvm_var_id);
            size_t index_i64_llvm_var_id = *var_count_ptr;
            *var_count_ptr += 1;

            printf("\t%%t%zu = getelementptr ", *var_count_ptr);
            monga_assert(array_exp->typedesc->tag == MONGA_AST_TYPEDESC_ARRAY);
            monga_ast_typedesc_reference_llvm(array_exp->typedesc->u.array_typedesc);
            printf(", ");
            monga_ast_typedesc_reference_llvm(array_exp->typedesc->u.array_typedesc);
            printf("* %%t%zu, i64 %%t%zu\n", array_exp->llvm_var_id, index_i64_llvm_var_id);
            *var_count_ptr += 1;
            break;
        }
        case MONGA_AST_VARIABLE_RECORD:
        {
            struct monga_ast_expression_t *record_exp;
            struct monga_ast_field_t *record_field;
            struct monga_ast_field_list_t* field_list;
            size_t field_index;

            record_exp = ast->u.record_var.record;
            monga_ast_expression_llvm(record_exp, var_count_ptr);
            
            monga_assert(ast->u.record_var.field.tag == MONGA_AST_REFERENCE_FIELD);
            record_field = ast->u.record_var.field.u.field;
            
            monga_assert(record_exp->typedesc->tag == MONGA_AST_TYPEDESC_RECORD);
            field_list = record_exp->typedesc->u.record_typedesc.field_list;
            
            field_index = monga_ast_get_field_index(field_list, record_field);

            monga_assert(record_exp->typedesc->tag == MONGA_AST_TYPEDESC_RECORD);
            printf("%%t%zu = getelementptr ", *var_count_ptr);
            monga_ast_typedesc_reference_llvm(record_exp->typedesc);
            printf(", ");
            monga_ast_typedesc_reference_llvm(record_exp->typedesc);
            printf("* %%t%zu, i64 0, i64 %zu\n", record_exp->llvm_var_id, field_index);
            *var_count_ptr += 1;
            break;
        }
        default:
            monga_unreachable();
    }
}

void monga_ast_expression_llvm(struct monga_ast_expression_t* ast, size_t* var_count_ptr)
{
    /* TODO -- set llvm_var_id in every expression */
    switch (ast->tag) {
        case MONGA_AST_EXPRESSION_INTEGER:
            printf("\t%%t%zu = add ", *var_count_ptr);
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(" %d, 0\n", ast->u.integer_exp.integer);
            *var_count_ptr += 1;
            break;
        case MONGA_AST_EXPRESSION_REAL:
            printf("\t%%t%zu = fadd ", *var_count_ptr);
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(" %.6f, 0.0\n", ast->u.real_exp.real);
            *var_count_ptr += 1;
            break;
        case MONGA_AST_EXPRESSION_VAR:
        {
            struct monga_ast_variable_t* var = ast->u.var_exp.var;
            monga_ast_variable_llvm(var, var_count_ptr);
            printf("\t%%t%zu = load ", *var_count_ptr);
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(", ");
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf("* %%t%zu\n", var->llvm_var_id);
            *var_count_ptr += 1;
            break;
        }
        case MONGA_AST_EXPRESSION_CALL:
            break;
        case MONGA_AST_EXPRESSION_CAST:
            break;
        case MONGA_AST_EXPRESSION_NEW:
            break;
        case MONGA_AST_EXPRESSION_NEGATIVE:
            break;
        case MONGA_AST_EXPRESSION_ADDITION:
        case MONGA_AST_EXPRESSION_SUBTRACTION:
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
        case MONGA_AST_EXPRESSION_DIVISION:
            break;
        case MONGA_AST_EXPRESSION_CONDITIONAL:
            break;
        default:
            monga_unreachable();
    }
    if (ast->next)
        monga_ast_expression_llvm(ast->next, var_count_ptr);
}

// void monga_ast_condition_llvm(struct monga_ast_condition_t* ast) {}

// void monga_ast_call_llvm(struct monga_ast_call_t* ast) {}

// void monga_ast_reference_llvm(struct monga_ast_reference_t* ast) {}
