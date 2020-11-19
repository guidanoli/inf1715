#include "monga_ast_llvm.h"
#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"

#include <stdio.h>

/* Type definitions */

typedef struct monga_ast_typedesc_t* (typedesc_getter)(void*);
typedef void* (next_getter)(void*);
typedef void (visiter)(void*, void*);

/* Static function prototypes */

static void monga_ast_typedesc_reference_llvm(struct monga_ast_typedesc_t* ast);
static void monga_ast_typedesc_reference_list_llvm(void* node, typedesc_getter get_node_typedesc, next_getter get_next_node, visiter visit_before, visiter visit_after, void* visit_arg);

static void monga_ast_def_variable_store_llvm(struct monga_ast_def_variable_t* def_variable, size_t value_id);
static void monga_ast_def_variable_list_allocation_llvm(struct monga_ast_def_variable_list_t* def_variable_list, size_t* var_count_ptr);
static void monga_ast_def_variable_reference_llvm(struct monga_ast_def_variable_t* def_variable);

static void monga_ast_variable_reference_llvm(struct monga_ast_variable_t* variable);

static void monga_ast_def_function_return_llvm(struct monga_ast_def_function_t* ast);

static size_t monga_ast_get_field_index(struct monga_ast_field_list_t* field_list, struct monga_ast_field_t* field);

static void monga_ast_expression_value_reference_llvm(struct monga_ast_expression_t* expression);

static void monga_ast_temporary_reference_llvm(size_t llvm_id);
static size_t monga_ast_new_temporary_llvm(size_t* var_count_ptr);

/* Callbacks */

static struct monga_ast_typedesc_t* field_typedesc_getter(void* field);
static void* field_next_getter(void *field);

static struct monga_ast_typedesc_t* def_variable_typedesc_getter(void* def_variable);
static void* def_variable_next_getter(void *def_variable);
static void def_variable_visit_after(void* def_variable, void* arg);

static struct monga_ast_typedesc_t* expression_typedesc_getter(void* expression);
static void* expression_next_getter(void* expression);
static void expression_visit_after(void* expression, void* arg);

/* Extern functions definitions */

void monga_ast_program_llvm(struct monga_ast_program_t* ast)
{
    if (ast->definitions != NULL)
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
    if (ast->next != NULL)
        monga_ast_definition_llvm(ast->next, struct_count);
}

void monga_ast_def_variable_llvm(struct monga_ast_def_variable_t* ast)
{
    printf("@%s = internal global ", ast->id);
    monga_assert(ast->type.tag == MONGA_AST_REFERENCE_TYPE);
    monga_ast_typedesc_reference_llvm(ast->type.u.def_type->typedesc);
    printf(" undef\n");
}

void monga_ast_def_type_llvm(struct monga_ast_def_type_t* ast, size_t* struct_count_ptr)
{
    monga_ast_typedesc_llvm(ast->typedesc, struct_count_ptr);
}

void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast)
{
    size_t var_count = 0;

    printf("define ");
    monga_ast_def_function_return_llvm(ast);
    printf(" @%s(", ast->id);

    if (ast->parameters != NULL) {
        monga_ast_typedesc_reference_list_llvm(ast->parameters->first,
            def_variable_typedesc_getter, def_variable_next_getter, NULL,
            def_variable_visit_after, &var_count);
    }

    printf(") {\n");

    if (ast->parameters != NULL) {
        struct monga_ast_def_variable_t* parameter = ast->parameters->first;
        size_t parameter_id = 0;

        monga_ast_def_variable_list_allocation_llvm(ast->parameters, &var_count);
        for (; parameter != NULL; ++parameter_id, parameter = parameter->next)
            monga_ast_def_variable_store_llvm(parameter, parameter_id);
    }

    if (ast->type.id != NULL)
        monga_assert(ast->type.tag == MONGA_AST_REFERENCE_TYPE);
        
    monga_ast_block_llvm(ast->block, &var_count, ast);

    printf("\tret ");
    monga_ast_def_function_return_llvm(ast);

    if (ast->type.id != NULL)
        printf(" undef");
    
    printf("\n}\n");
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

void monga_ast_block_llvm(struct monga_ast_block_t* ast, size_t* var_count_ptr, struct monga_ast_def_function_t* def_function)
{
    if (ast->variables != NULL)
        monga_ast_def_variable_list_allocation_llvm(ast->variables, var_count_ptr);
    if (ast->statements != NULL)
        monga_ast_statement_llvm(ast->statements->first, var_count_ptr, def_function);
}

void monga_ast_statement_llvm(struct monga_ast_statement_t* ast, size_t* var_count_ptr, struct monga_ast_def_function_t* def_function)
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

            printf("\tstore ");
            monga_ast_typedesc_reference_llvm(var->typedesc);
            printf(" ");
            monga_ast_expression_value_reference_llvm(exp);
            printf(", ");
            monga_ast_typedesc_reference_llvm(var->typedesc);
            printf("* ");
            monga_ast_variable_reference_llvm(var);
            printf("\n");

            break;
        }
        case MONGA_AST_STATEMENT_RETURN:
        {
            struct monga_ast_expression_t* exp = ast->u.return_stmt.exp;

            if (exp != NULL)
                monga_ast_expression_llvm(exp, var_count_ptr);

            printf("\tret ");
            monga_ast_def_function_return_llvm(def_function);
            if (exp != NULL) {
                printf(" ");
                monga_ast_expression_value_reference_llvm(exp);
            }
            printf("\n");
            
            break;
        }
        case MONGA_AST_STATEMENT_CALL:
            monga_ast_call_llvm(ast->u.call_stmt.call, var_count_ptr);
            break;
        case MONGA_AST_STATEMENT_PRINT:
            break;
        case MONGA_AST_STATEMENT_BLOCK:
            monga_ast_block_llvm(ast->u.block_stmt.block, var_count_ptr, def_function);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next != NULL)
        monga_ast_statement_llvm(ast->next, var_count_ptr, def_function);
}

void monga_ast_variable_llvm(struct monga_ast_variable_t* ast, size_t* var_count_ptr)
{
    switch (ast->tag) {
        case MONGA_AST_VARIABLE_ID:
        {
            struct monga_ast_reference_t* id_var;
            struct monga_ast_def_variable_t* def_variable;

            id_var = &ast->u.id_var;

            monga_assert(id_var->tag == MONGA_AST_REFERENCE_VARIABLE);
            def_variable = id_var->u.def_variable;

            /* If the variable is global, its id can already be used
               to track the global variable definition */
            
            if (!def_variable->is_global)
                ast->llvm_var_id = def_variable->llvm_var_id;
            
            break;
        }
        case MONGA_AST_VARIABLE_ARRAY:
        {
            struct monga_ast_expression_t *array_exp, *index_exp;
            struct monga_ast_typedesc_t *typedesc;
            size_t index_i64_llvm_var_id;
            array_exp = ast->u.array_var.array;
            index_exp = ast->u.array_var.index;
            
            monga_ast_expression_llvm(array_exp, var_count_ptr);
            monga_ast_expression_llvm(index_exp, var_count_ptr);

            index_i64_llvm_var_id = monga_ast_new_temporary_llvm(var_count_ptr);
            printf("sext i32 ");
            monga_ast_expression_value_reference_llvm(index_exp);
            printf(" to i64\n");
            
            monga_assert(array_exp->typedesc->tag == MONGA_AST_TYPEDESC_ARRAY);
            typedesc = array_exp->typedesc->u.array_typedesc;

            ast->llvm_var_id = monga_ast_new_temporary_llvm(var_count_ptr);
            printf("getelementptr ");
            monga_ast_typedesc_reference_llvm(typedesc);
            printf(", ");
            monga_ast_typedesc_reference_llvm(typedesc);
            printf("* ");
            monga_ast_expression_value_reference_llvm(array_exp);
            printf(", i64 ");
            monga_ast_temporary_reference_llvm(index_i64_llvm_var_id);
            printf("\n");

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

            ast->llvm_var_id = monga_ast_new_temporary_llvm(var_count_ptr);
            printf("getelementptr ");
            monga_ast_typedesc_reference_llvm(record_exp->typedesc);
            printf(", ");
            monga_ast_typedesc_reference_llvm(record_exp->typedesc);
            printf("* ");
            monga_ast_expression_value_reference_llvm(record_exp);
            printf(", i64 0, i64 %zu\n", field_index);

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
            ast->llvm_var_id = monga_ast_new_temporary_llvm(var_count_ptr);
            printf("add ");
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(" %d, 0\n", ast->u.integer_exp.integer);
            break;
        case MONGA_AST_EXPRESSION_REAL:
            ast->llvm_var_id = monga_ast_new_temporary_llvm(var_count_ptr);
            printf("fadd ");
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(" %.6f, 0.0\n", ast->u.real_exp.real);
            break;
        case MONGA_AST_EXPRESSION_VAR:
        {
            struct monga_ast_variable_t* var = ast->u.var_exp.var;
            monga_ast_variable_llvm(var, var_count_ptr);

            ast->llvm_var_id = monga_ast_new_temporary_llvm(var_count_ptr);
            printf("load ");
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(", ");
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf("* ");
            monga_ast_variable_reference_llvm(var);
            printf("\n");
            
            break;
        }
        case MONGA_AST_EXPRESSION_CALL:
        {
            struct monga_ast_call_t* call = ast->u.call_exp.call;
            monga_ast_call_llvm(call, var_count_ptr);

            monga_assert(call->function.tag == MONGA_AST_REFERENCE_FUNCTION);
            monga_assert(call->function.u.def_function->type.id != NULL);
            ast->llvm_var_id = call->llvm_var_id;

            break;
        }
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
    if (ast->next != NULL)
        monga_ast_expression_llvm(ast->next, var_count_ptr);
}

// void monga_ast_condition_llvm(struct monga_ast_condition_t* ast) {}

void monga_ast_call_llvm(struct monga_ast_call_t* ast, size_t* var_count_ptr)
{
    struct monga_ast_reference_t* def_function_ref;
    struct monga_ast_def_function_t* def_function;

    def_function_ref = &ast->function;
    monga_assert(def_function_ref->tag == MONGA_AST_REFERENCE_FUNCTION);
    def_function = def_function_ref->u.def_function;
    
    if (ast->expressions != NULL)
        monga_ast_expression_llvm(ast->expressions->first, var_count_ptr);

    if (def_function->type.id != NULL)
        ast->llvm_var_id = monga_ast_new_temporary_llvm(var_count_ptr);
    else
        printf("\t");
    
    printf("call ");
    monga_ast_def_function_return_llvm(def_function);
    printf(" @%s(", def_function->id);
    
    if (ast->expressions != NULL) {
        monga_ast_typedesc_reference_list_llvm(ast->expressions->first,
            expression_typedesc_getter, expression_next_getter, NULL, expression_visit_after, NULL);
    }
    
    printf(")\n");
}

// void monga_ast_reference_llvm(struct monga_ast_reference_t* ast) {}

/* Static function definitions */

void monga_ast_def_variable_store_llvm(struct monga_ast_def_variable_t* def_variable, size_t value_id)
{
    struct monga_ast_typedesc_t* typedesc;

    monga_assert(def_variable->type.tag == MONGA_AST_REFERENCE_TYPE);
    typedesc = def_variable->type.u.def_type->typedesc;

    printf("\tstore ");
    monga_ast_typedesc_reference_llvm(typedesc);
    printf(" ");
    monga_ast_temporary_reference_llvm(value_id);
    printf(", ");
    monga_ast_typedesc_reference_llvm(typedesc);
    printf("* ");
    monga_ast_def_variable_reference_llvm(def_variable);
    printf("\n");
}

void monga_ast_def_variable_list_allocation_llvm(struct monga_ast_def_variable_list_t* def_variable_list, size_t* var_count_ptr)
{
    struct monga_ast_def_variable_t* def_variable;

    for (def_variable = def_variable_list->first;
        def_variable != NULL;
        def_variable = def_variable->next) {
        
        struct monga_ast_typedesc_t* typedesc;

        monga_assert(def_variable->type.tag == MONGA_AST_REFERENCE_TYPE);
        typedesc = def_variable->type.u.def_type->typedesc;

        def_variable->llvm_var_id = monga_ast_new_temporary_llvm(var_count_ptr);
        printf("alloca ");
        monga_ast_typedesc_reference_llvm(typedesc);
        printf("\n");
    }
}

void monga_ast_def_variable_reference_llvm(struct monga_ast_def_variable_t* def_variable)
{
    if (def_variable->is_global) {
        printf("@%s", def_variable->id);
    } else {
        printf("%%t%zu", def_variable->llvm_var_id);
    }
}

struct monga_ast_typedesc_t* field_typedesc_getter(void* field)
{
    struct monga_ast_field_t* field_ = (struct monga_ast_field_t*) field;
    monga_assert(field_->type.tag == MONGA_AST_REFERENCE_TYPE);
    return field_->type.u.def_type->typedesc;
}

void* field_next_getter(void *field)
{
    struct monga_ast_field_t* field_ = (struct monga_ast_field_t*) field;
    return field_->next;
}

struct monga_ast_typedesc_t* def_variable_typedesc_getter(void* def_variable)
{
    struct monga_ast_def_variable_t* def_variable_ = (struct monga_ast_def_variable_t*) def_variable;
    monga_assert(def_variable_->type.tag == MONGA_AST_REFERENCE_TYPE);
    return def_variable_->type.u.def_type->typedesc;
}

void* def_variable_next_getter(void *def_variable)
{
    struct monga_ast_def_variable_t* def_variable_ = (struct monga_ast_def_variable_t*) def_variable;
    return def_variable_->next;
}

void def_variable_visit_after(void* def_variable, void* arg)
{
    struct monga_ast_def_variable_t* def_variable_ = (struct monga_ast_def_variable_t*) def_variable;
    size_t* var_count = (size_t*) arg;
    def_variable_->llvm_var_id = *var_count;
    *var_count += 1;
    printf(" ");
    monga_ast_def_variable_reference_llvm(def_variable_);
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
        printf("*");
        break;
    case MONGA_AST_TYPEDESC_RECORD:
        printf("%%S%zu*", ast->u.record_typedesc.llvm_struct_id);
        break;
    default:
        monga_unreachable();
    }
}

void monga_ast_def_function_return_llvm(struct monga_ast_def_function_t* ast)
{
    if (ast->type.id != NULL) {
        monga_assert(ast->type.tag == MONGA_AST_REFERENCE_TYPE);
        monga_ast_typedesc_reference_llvm(ast->type.u.def_type->typedesc);
    } else {
        printf("void");
    }
}

size_t monga_ast_get_field_index(struct monga_ast_field_list_t* field_list, struct monga_ast_field_t* field)
{
    size_t index = 0;

    for (struct monga_ast_field_t* f = field_list->first;
        f != NULL && f != field;
        f = f->next, ++index);

    return index;
}

void monga_ast_expression_value_reference_llvm(struct monga_ast_expression_t* expression)
{
    monga_ast_temporary_reference_llvm(expression->llvm_var_id);
}

size_t monga_ast_new_temporary_llvm(size_t* var_count_ptr)
{
    size_t var_count = *var_count_ptr;
    printf("\t");
    monga_ast_temporary_reference_llvm(var_count);
    printf(" = ");
    *var_count_ptr += 1;
    return var_count;
}

void monga_ast_temporary_reference_llvm(size_t llvm_id)
{
    printf("%%t%zu", llvm_id);
}

void monga_ast_variable_reference_llvm(struct monga_ast_variable_t* variable)
{
    switch(variable->tag) {
    case MONGA_AST_VARIABLE_ID:
    {
        struct monga_ast_reference_t* id_var = &variable->u.id_var;
        monga_assert(id_var->tag == MONGA_AST_REFERENCE_VARIABLE);

        /* variable id can be local or global */
        monga_ast_def_variable_reference_llvm(id_var->u.def_variable);
        break;
    }
    case MONGA_AST_VARIABLE_ARRAY:
        monga_ast_temporary_reference_llvm(variable->llvm_var_id);
        break;
    case MONGA_AST_VARIABLE_RECORD:
        monga_ast_temporary_reference_llvm(variable->llvm_var_id);
        break;
    default:
        monga_unreachable();
    }
}

struct monga_ast_typedesc_t* expression_typedesc_getter(void* expression)
{
    struct monga_ast_expression_t* expression_ = (struct monga_ast_expression_t*) expression;
    return expression_->typedesc;
}

void* expression_next_getter(void* expression)
{
    struct monga_ast_expression_t* expression_ = (struct monga_ast_expression_t*) expression;
    return expression_->next;
}

void expression_visit_after(void* expression, void* monga_unused(arg))
{
    struct monga_ast_expression_t* expression_ = (struct monga_ast_expression_t*) expression;
    printf(" ");
    monga_ast_expression_value_reference_llvm(expression_);
    if (expression_->next != NULL)
        printf(", ");
}
