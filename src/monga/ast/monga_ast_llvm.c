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

    if (parameter->next)
        monga_ast_parameter_store_llvm(parameter->next, parameter_count);
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

// void monga_ast_block_llvm(struct monga_ast_block_t* ast) {}

// void monga_ast_statement_llvm(struct monga_ast_statement_t* ast) {}

// void monga_ast_variable_llvm(struct monga_ast_variable_t* ast) {}

// void monga_ast_expression_llvm(struct monga_ast_expression_t* ast) {}

// void monga_ast_condition_llvm(struct monga_ast_condition_t* ast) {}

// void monga_ast_call_llvm(struct monga_ast_call_t* ast) {}

// void monga_ast_reference_llvm(struct monga_ast_reference_t* ast) {}
