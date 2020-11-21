#include "monga_ast_llvm.h"
#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"

#include <stdio.h>
#include <string.h>

/* Type definitions */

typedef struct monga_ast_typedesc_t* (typedesc_getter)(void*);
typedef void* (next_getter)(void*);
typedef void (visiter)(void*, void*);
typedef const char* (*builtin_instruction_getter)(enum monga_ast_typedesc_builtin_t);

struct monga_ast_node_iter_t {
    typedesc_getter* get_node_typedesc;
    next_getter* get_next_node;
    visiter* visit_before; /* nullabe */
    visiter* visit_after; /* nullabe */
    void* visit_arg; /* nullabe */
};

/* Static function prototypes */

static void monga_ast_typedesc_reference_llvm(struct monga_ast_typedesc_t* ast);
static void monga_ast_typedesc_subtype_reference_llvm(struct monga_ast_typedesc_t* ast);
static void monga_ast_typedesc_reference_iter_llvm(void* node, struct monga_ast_node_iter_t* iter);
static void monga_ast_def_variable_reference_llvm(struct monga_ast_def_variable_t* def_variable);
static void monga_ast_variable_reference_llvm(struct monga_ast_variable_t* variable);
static void monga_ast_expression_reference_llvm(struct monga_ast_expression_t* expression);

static void monga_ast_struct_reference_llvm(size_t llvm_struct_id);
static size_t monga_ast_struct_new_llvm(struct monga_ast_llvm_context_t* ctx);
static size_t monga_ast_struct_new_define_llvm(struct monga_ast_llvm_context_t* ctx);

static void monga_ast_tempvar_reference_llvm(size_t llvm_var_id);
static size_t monga_ast_tempvar_new_llvm(struct monga_ast_llvm_context_t* ctx);
static size_t monga_ast_tempvar_new_assign_llvm(struct monga_ast_llvm_context_t* ctx);

static void monga_ast_label_reference_llvm(size_t llvm_label_id);
static void monga_ast_label_definition_llvm(size_t llvm_label_id);
static size_t monga_ast_label_new_llvm(struct monga_ast_llvm_context_t* ctx);

static void monga_ast_def_function_return_llvm(struct monga_ast_def_function_t* ast);
static void monga_ast_unconditional_jump_llvm(size_t llvm_label_id);

static builtin_instruction_getter monga_ast_expression_binop_instruction_getter(struct monga_ast_expression_t* ast);
static builtin_instruction_getter monga_ast_condition_binop_instruction_getter(struct monga_ast_condition_t* ast);

static enum monga_ast_llvm_printf_fmt_t monga_ast_typedesc_printf_fmt(struct monga_ast_typedesc_t* typedesc);
static const char* monga_ast_printf_fmt_constant_string(enum monga_ast_llvm_printf_fmt_t fmt);
static const char* monga_ast_printf_fmt_constant_name(enum monga_ast_llvm_printf_fmt_t fmt);
static size_t monga_ast_printf_fmt_size(enum monga_ast_llvm_printf_fmt_t fmt);

static const char* monga_ast_func_declaration(enum monga_ast_llvm_func_t func);

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
    struct monga_ast_llvm_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));

    if (ast->definitions != NULL)
        monga_ast_definition_llvm(ast->definitions->first, &ctx);

    /* Function declaration */
    {
        char flag = 1;
        while (flag < MONGA_AST_LLVM_FUNC_CNT) {
            if (ctx.referenced_funcs & flag) {
                const char* func_decl = monga_ast_func_declaration(flag);
                monga_assert(func_decl != NULL);
                printf("declare %s\n", func_decl);
            }
            flag <<= 1;
        }
    }
    
    /* Print format declaration */
    {
        char flag = 1;
        while (flag < MONGA_AST_LLVM_PRINTF_FMT_CNT) {
            if (ctx.referenced_printf_fmts & flag) {
                const char* fmt_str = monga_ast_printf_fmt_constant_string(flag);
                const char* fmt_name = monga_ast_printf_fmt_constant_name(flag);
                size_t fmt_size = monga_ast_printf_fmt_size(flag);
                monga_assert(fmt_str != NULL);
                monga_assert(fmt_name != NULL);
                printf("@%s = constant [%zu x i8] c\"%s\"\n", fmt_name, fmt_size, fmt_str);
            }
            flag <<= 1;
        }
    }
}

void monga_ast_definition_llvm(struct monga_ast_definition_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    switch (ast->tag) {
        case MONGA_AST_DEFINITION_VARIABLE:
            monga_ast_def_variable_llvm(ast->u.def_variable, ctx);
            break;
        case MONGA_AST_DEFINITION_TYPE:
            monga_ast_def_type_llvm(ast->u.def_type, ctx);
            break;
        case MONGA_AST_DEFINITION_FUNCTION:
            monga_ast_def_function_llvm(ast->u.def_function, ctx);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next != NULL)
        monga_ast_definition_llvm(ast->next, ctx);
}

void monga_ast_def_variable_llvm(struct monga_ast_def_variable_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    if (ast->is_global) {
        monga_assert(ctx->def_function == NULL);
        printf("@%s = internal global ", ast->id);
        monga_assert(ast->type.tag == MONGA_AST_REFERENCE_TYPE);
        monga_ast_typedesc_reference_llvm(ast->type.u.def_type->typedesc);
        printf(" undef\n");
    } else {
        monga_assert(ctx->def_function != NULL);
        ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
        printf("alloca ");
        monga_assert(ast->type.tag == MONGA_AST_REFERENCE_TYPE);
        monga_ast_typedesc_reference_llvm(ast->type.u.def_type->typedesc);
        printf("\n");
    }

    if (ast->next != NULL)
        monga_ast_def_variable_llvm(ast->next, ctx);
}

void monga_ast_def_type_llvm(struct monga_ast_def_type_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    monga_ast_typedesc_llvm(ast->typedesc, ctx);
}

void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    ctx->def_function = ast;
    ctx->tempvar_count = 0;
    ctx->label_count = 0;

    printf("define ");
    monga_ast_def_function_return_llvm(ast);
    printf(" @%s(", ast->id);

    if (ast->parameters != NULL) {
        struct monga_ast_node_iter_t iter = {
            .get_node_typedesc = def_variable_typedesc_getter,
            .get_next_node = def_variable_next_getter,
            .visit_after = def_variable_visit_after,
            .visit_arg = ctx,
        };

        monga_ast_typedesc_reference_iter_llvm(ast->parameters->first, &iter);
    }

    printf(") {\n");

    if (ast->parameters != NULL) {
        struct monga_ast_def_variable_t* parameter = ast->parameters->first;

        monga_ast_def_variable_llvm(ast->parameters->first, ctx);

        for (size_t llvm_var_id = 0; parameter != NULL; ++llvm_var_id, parameter = parameter->next) {
            struct monga_ast_typedesc_t* typedesc;

            monga_assert(parameter->type.tag == MONGA_AST_REFERENCE_TYPE);
            typedesc = parameter->type.u.def_type->typedesc;

            printf("\tstore ");
            monga_ast_typedesc_reference_llvm(typedesc);
            printf(" ");
            monga_ast_tempvar_reference_llvm(llvm_var_id);
            printf(", ");
            monga_ast_typedesc_reference_llvm(typedesc);
            printf("* ");
            monga_ast_def_variable_reference_llvm(parameter);
            printf("\n");
        }
    }

    if (ast->type.id != NULL)
        monga_assert(ast->type.tag == MONGA_AST_REFERENCE_TYPE);
        
    monga_ast_block_llvm(ast->block, ctx);

    printf("\tret ");
    monga_ast_def_function_return_llvm(ast);

    if (ast->type.id != NULL)
        printf(" undef");
    
    printf("\n}\n");

    ctx->def_function = NULL;
}

void monga_ast_typedesc_llvm(struct monga_ast_typedesc_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    switch (ast->tag) {
    case MONGA_AST_TYPEDESC_ID:
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
        monga_ast_typedesc_llvm(ast->u.array_typedesc, ctx);
        break;
    case MONGA_AST_TYPEDESC_RECORD:
    {
        struct monga_ast_field_t* field = ast->u.record_typedesc.field_list->first;
        struct monga_ast_node_iter_t iter = {
            .get_node_typedesc = field_typedesc_getter,
            .get_next_node = field_next_getter,
        };

        ast->u.record_typedesc.llvm_struct_id = monga_ast_struct_new_define_llvm(ctx);
        printf("{ ");
        monga_ast_typedesc_reference_iter_llvm(field, &iter);
        printf(" }\n");
        break;
    }
    default:
        monga_unreachable();
    }
}

void monga_ast_block_llvm(struct monga_ast_block_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    if (ast->variables != NULL)
        monga_ast_def_variable_llvm(ast->variables->first, ctx);
    if (ast->statements != NULL)
        monga_ast_statement_llvm(ast->statements->first, ctx);
}

void monga_ast_statement_llvm(struct monga_ast_statement_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    switch (ast->tag) {
        case MONGA_AST_STATEMENT_IF:
        {
            size_t then_label, else_label, endif_label;
            struct monga_ast_condition_t* cond = ast->u.if_stmt.cond;
            struct monga_ast_block_t* then_block, *else_block;

            then_block = ast->u.if_stmt.then_block;
            else_block = ast->u.if_stmt.else_block;

            then_label = monga_ast_label_new_llvm(ctx);
            else_label = monga_ast_label_new_llvm(ctx);
            if (else_block != NULL)
                endif_label = monga_ast_label_new_llvm(ctx);
            else
                endif_label = else_label;

            monga_ast_condition_llvm(cond, ctx, then_label, else_label);

            /* then: */
            monga_ast_label_definition_llvm(then_label);
            monga_ast_block_llvm(then_block, ctx);
            printf("\tbr label ");
            monga_ast_label_reference_llvm(endif_label);
            printf("\n");
            
            /* else: */
            if (else_block != NULL) {
                monga_ast_label_definition_llvm(else_label);
                monga_ast_block_llvm(else_block, ctx);
                printf("\tbr label ");
                monga_ast_label_reference_llvm(endif_label);
                printf("\n");
            }

            /* endif: */
            monga_ast_label_definition_llvm(endif_label);

            break;
        }
        case MONGA_AST_STATEMENT_WHILE:
        {
            size_t do_label, done_label;
            struct monga_ast_condition_t* cond;
            struct monga_ast_block_t* loop_block;

            cond = ast->u.while_stmt.cond;
            loop_block = ast->u.while_stmt.loop;

            do_label = monga_ast_label_new_llvm(ctx);
            done_label = monga_ast_label_new_llvm(ctx);

            monga_ast_condition_llvm(cond, ctx, do_label, done_label);

            /* do: */
            monga_ast_label_definition_llvm(do_label);
            monga_ast_block_llvm(loop_block, ctx);
            monga_ast_condition_llvm(cond, ctx, do_label, done_label);

            /* done: */
            monga_ast_label_definition_llvm(done_label);

            break;
        }
        case MONGA_AST_STATEMENT_ASSIGN:
        {
            struct monga_ast_variable_t* var = ast->u.assign_stmt.var;
            struct monga_ast_expression_t* exp = ast->u.assign_stmt.exp;

            monga_ast_variable_llvm(var, ctx);
            monga_ast_expression_llvm(exp, ctx);

            printf("\tstore ");
            monga_ast_typedesc_reference_llvm(var->typedesc);
            printf(" ");
            monga_ast_expression_reference_llvm(exp);
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
                monga_ast_expression_llvm(exp, ctx);

            printf("\tret ");
            monga_ast_def_function_return_llvm(ctx->def_function);
            if (exp != NULL) {
                printf(" ");
                monga_ast_expression_reference_llvm(exp);
            }
            printf("\n");
            
            break;
        }
        case MONGA_AST_STATEMENT_CALL:
            monga_ast_call_llvm(ast->u.call_stmt.call, ctx);
            break;
        case MONGA_AST_STATEMENT_PRINT:
        {
            struct monga_ast_expression_t* exp = ast->u.print_stmt.exp;
            struct monga_ast_typedesc_t* typedesc = exp->typedesc;
            enum monga_ast_llvm_printf_fmt_t fmt;
            const char* str_name;
            size_t str_size;
            size_t str_id;

            monga_ast_expression_llvm(exp, ctx);

            fmt = monga_ast_typedesc_printf_fmt(typedesc);
            str_name = monga_ast_printf_fmt_constant_name(fmt);
            str_size = monga_ast_printf_fmt_size(fmt);

            str_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("getelementptr inbounds [%zu x i8], [%zu x i8]* @%s, i64 0, i64 0\n",
                str_size, str_size, str_name);

            printf("\tcall i32 (i8*, ...) @printf(i8* ");
            monga_ast_tempvar_reference_llvm(str_id);
            printf(", ");
            monga_ast_typedesc_reference_llvm(typedesc);
            printf(" ");
            monga_ast_expression_reference_llvm(exp);
            printf(")\n");

            ctx->referenced_funcs |= MONGA_AST_LLVM_FUNC_PRINTF;
            ctx->referenced_printf_fmts |= fmt;

            break;
        }
        case MONGA_AST_STATEMENT_BLOCK:
            monga_ast_block_llvm(ast->u.block_stmt.block, ctx);
            break;
        default:
            monga_unreachable();
    }
    if (ast->next != NULL)
        monga_ast_statement_llvm(ast->next, ctx);
}

void monga_ast_variable_llvm(struct monga_ast_variable_t* ast, struct monga_ast_llvm_context_t* ctx)
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
                ast->tempvar_id = def_variable->tempvar_id;
            
            break;
        }
        case MONGA_AST_VARIABLE_ARRAY:
        {
            struct monga_ast_expression_t *array_exp, *index_exp;
            struct monga_ast_typedesc_t *typedesc;
            size_t index_i64_llvm_var_id;
            array_exp = ast->u.array_var.array;
            index_exp = ast->u.array_var.index;
            
            monga_ast_expression_llvm(array_exp, ctx);
            monga_ast_expression_llvm(index_exp, ctx);

            index_i64_llvm_var_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("sext i32 ");
            monga_ast_expression_reference_llvm(index_exp);
            printf(" to i64\n");
            
            monga_assert(array_exp->typedesc->tag == MONGA_AST_TYPEDESC_ARRAY);
            typedesc = array_exp->typedesc->u.array_typedesc;

            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("getelementptr ");
            monga_ast_typedesc_reference_llvm(typedesc);
            printf(", ");
            monga_ast_typedesc_reference_llvm(typedesc);
            printf("* ");
            monga_ast_expression_reference_llvm(array_exp);
            printf(", i64 ");
            monga_ast_tempvar_reference_llvm(index_i64_llvm_var_id);
            printf("\n");

            break;
        }
        case MONGA_AST_VARIABLE_RECORD:
        {
            struct monga_ast_expression_t *record_exp;
            struct monga_ast_field_t *record_field;
            struct monga_ast_field_list_t* field_list;
            int field_index;
            size_t struct_id;

            record_exp = ast->u.record_var.record;
            monga_ast_expression_llvm(record_exp, ctx);
            
            monga_assert(ast->u.record_var.field.tag == MONGA_AST_REFERENCE_FIELD);
            record_field = ast->u.record_var.field.u.field;
            
            monga_assert(record_exp->typedesc->tag == MONGA_AST_TYPEDESC_RECORD);
            field_list = record_exp->typedesc->u.record_typedesc.field_list;
            
            field_index = 0;
            for (struct monga_ast_field_t* f = field_list->first;
                f != NULL && f != record_field;
                f = f->next, ++field_index);

            monga_assert(record_exp->typedesc->tag == MONGA_AST_TYPEDESC_RECORD);
            struct_id = record_exp->typedesc->u.record_typedesc.llvm_struct_id;

            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("getelementptr ");
            monga_ast_struct_reference_llvm(struct_id);
            printf(", ");
            monga_ast_struct_reference_llvm(struct_id);
            printf("* ");
            monga_ast_expression_reference_llvm(record_exp);
            printf(", i32 0, i32 %d\n", field_index);

            break;
        }
        default:
            monga_unreachable();
    }
}

void monga_ast_expression_llvm(struct monga_ast_expression_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    /* TODO -- set llvm_var_id in every expression */
    switch (ast->tag) {
        case MONGA_AST_EXPRESSION_INTEGER:
        {
            int integer = ast->u.integer_exp.integer;
            const char *zero;
            
            zero = monga_ast_builtin_typedesc_zero_llvm(MONGA_AST_TYPEDESC_BUILTIN_INT);
            monga_assert(zero != NULL);

            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("add ");
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(" %d, %s\n", integer, zero);
            
            break;
        }
        case MONGA_AST_EXPRESSION_REAL:
        {
            float real = ast->u.real_exp.real;
            const char *zero;
            
            zero = monga_ast_builtin_typedesc_zero_llvm(MONGA_AST_TYPEDESC_BUILTIN_FLOAT);
            monga_assert(zero != NULL);

            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("fadd ");
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(" %f, %s\n", real, zero);

            break;
        }
        case MONGA_AST_EXPRESSION_VAR:
        {
            struct monga_ast_variable_t* var = ast->u.var_exp.var;

            monga_ast_variable_llvm(var, ctx);

            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
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

            monga_ast_call_llvm(call, ctx);

            monga_assert(call->function.tag == MONGA_AST_REFERENCE_FUNCTION);
            monga_assert(call->function.u.def_function->type.id != NULL);
            ast->tempvar_id = call->tempvar_id;

            break;
        }
        case MONGA_AST_EXPRESSION_CAST:
        {
            struct monga_ast_expression_t* exp = ast->u.cast_exp.exp;
            struct monga_ast_reference_t* type_ref = &ast->u.cast_exp.type;
            struct monga_ast_typedesc_t* typedesc;

            monga_ast_expression_llvm(exp, ctx);

            monga_assert(type_ref->tag == MONGA_AST_REFERENCE_TYPE);
            typedesc = monga_ast_typedesc_resolve_id(type_ref->u.def_type->typedesc);

            switch (typedesc->tag) {
            case MONGA_AST_TYPEDESC_BUILTIN:
            {
                enum monga_ast_typedesc_builtin_t from, to;

                monga_assert(exp->typedesc->tag == MONGA_AST_TYPEDESC_BUILTIN);
                from = exp->typedesc->u.builtin_typedesc;
                to = typedesc->u.builtin_typedesc;

                if (from == to)
                {
                    ast->tempvar_id = exp->tempvar_id; /* no conversion */
                }
                else
                {
                    const char* instruction = monga_ast_builtin_llvm_cast_instruction(to, from);
                    monga_assert(instruction != NULL);
                    ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
                    printf("%s %s ", instruction, monga_ast_builtin_typedesc_llvm(from));
                    monga_ast_expression_reference_llvm(exp);
                    printf(" to %s\n", monga_ast_builtin_typedesc_llvm(to));
                }
                break;
            }
            case MONGA_AST_TYPEDESC_ID:
                monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                break;
            case MONGA_AST_TYPEDESC_ARRAY:
                ast->tempvar_id = exp->tempvar_id; /* no conversion */
                break;
            case MONGA_AST_TYPEDESC_RECORD:
                ast->tempvar_id = exp->tempvar_id; /* no conversion */
                break;
            default:
                monga_unreachable();
            }

            break;
        }
        case MONGA_AST_EXPRESSION_NEW:
        {
            struct monga_ast_expression_t *exp = ast->u.new_exp.exp;
            struct monga_ast_typedesc_t *typedesc = ast->typedesc;
            size_t size_var_id, size_i64_var_id, ptr_var_id;

            if (exp != NULL)
                monga_ast_expression_llvm(exp, ctx);

            /* size */
            size_var_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("getelementptr ");
            monga_ast_typedesc_subtype_reference_llvm(typedesc);
            printf(", ");
            monga_ast_typedesc_subtype_reference_llvm(typedesc);
            printf("* null, i32 ");
            if (exp == NULL)
                printf("1");
            else
                monga_ast_expression_reference_llvm(exp);
            printf("\n");

            /* size i64 */
            size_i64_var_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("ptrtoint ");
            monga_ast_typedesc_subtype_reference_llvm(typedesc);
            printf("* ");
            monga_ast_tempvar_reference_llvm(size_var_id);
            printf(" to i64\n");

            /* ptr */
            ptr_var_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("call i8* @malloc(i64 ");
            monga_ast_tempvar_reference_llvm(size_i64_var_id);
            printf(")\n");
            
            /* signal reference to malloc */
            ctx->referenced_funcs |= MONGA_AST_LLVM_FUNC_MALLOC;

            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("bitcast i8* ");
            monga_ast_tempvar_reference_llvm(ptr_var_id);
            printf(" to ");
            monga_ast_typedesc_reference_llvm(typedesc);
            printf("\n");

            break;
        }
        case MONGA_AST_EXPRESSION_NEGATIVE:
        {
            struct monga_ast_expression_t* exp = ast->u.negative_exp.exp;
            struct monga_ast_typedesc_t* typedesc = exp->typedesc;
            enum monga_ast_typedesc_builtin_t builtin_typedesc;
            const char* instruction;
            const char* zero;
            
            monga_ast_expression_llvm(exp, ctx);

            typedesc = monga_ast_typedesc_resolve_id(typedesc);

            monga_assert(typedesc->tag == MONGA_AST_TYPEDESC_BUILTIN);
            builtin_typedesc = typedesc->u.builtin_typedesc;

            instruction = monga_ast_builtin_llvm_sub_instruction(builtin_typedesc);
            monga_assert(instruction != NULL);

            zero = monga_ast_builtin_typedesc_zero_llvm(builtin_typedesc);
            monga_assert(zero != NULL);

            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("%s %s %s, ", instruction, monga_ast_builtin_typedesc_llvm(builtin_typedesc), zero);
            monga_ast_expression_reference_llvm(exp);
            printf("\n");

            break;
        }
        case MONGA_AST_EXPRESSION_ADDITION:
        case MONGA_AST_EXPRESSION_SUBTRACTION:
        case MONGA_AST_EXPRESSION_MULTIPLICATION:
        case MONGA_AST_EXPRESSION_DIVISION:
        {
            struct monga_ast_expression_t* exp1 = ast->u.binop_exp.exp1;
            struct monga_ast_expression_t* exp2 = ast->u.binop_exp.exp2;
            struct monga_ast_typedesc_t* typedesc = exp1->typedesc;
            enum monga_ast_typedesc_builtin_t builtin_typedesc;
            builtin_instruction_getter instruction_getter;
            const char* instruction;
            
            monga_ast_expression_llvm(exp1, ctx);
            monga_ast_expression_llvm(exp2, ctx);

            typedesc = monga_ast_typedesc_resolve_id(typedesc);

            monga_assert(typedesc->tag == MONGA_AST_TYPEDESC_BUILTIN);
            builtin_typedesc = typedesc->u.builtin_typedesc;

            instruction_getter = monga_ast_expression_binop_instruction_getter(ast);
            monga_assert(instruction_getter != NULL);

            instruction = instruction_getter(builtin_typedesc);
            monga_assert(instruction != NULL);

            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("%s %s ", instruction, monga_ast_builtin_typedesc_llvm(builtin_typedesc));
            monga_ast_expression_reference_llvm(exp1);
            printf(", ");
            monga_ast_expression_reference_llvm(exp2);
            printf("\n");

            break;
        }
        case MONGA_AST_EXPRESSION_CONDITIONAL:
        {
            struct monga_ast_condition_t* cond = ast->u.conditional_exp.cond;
            struct monga_ast_expression_t* true_exp = ast->u.conditional_exp.true_exp;
            struct monga_ast_expression_t* false_exp = ast->u.conditional_exp.false_exp;
            size_t true_label, true_end_label, false_label, false_end_label, eval_label;

            true_label = monga_ast_label_new_llvm(ctx);
            true_end_label = monga_ast_label_new_llvm(ctx);
            false_label = monga_ast_label_new_llvm(ctx);
            false_end_label = monga_ast_label_new_llvm(ctx);
            eval_label = monga_ast_label_new_llvm(ctx);

            monga_ast_condition_llvm(cond, ctx, true_label, false_label);

            /* true: */
            monga_ast_label_definition_llvm(true_label);
            monga_ast_expression_llvm(true_exp, ctx);
            monga_ast_unconditional_jump_llvm(true_end_label);

            /* true_end: */
            monga_ast_label_definition_llvm(true_end_label);
            monga_ast_unconditional_jump_llvm(eval_label);

            /* false: */
            monga_ast_label_definition_llvm(false_label);
            monga_ast_expression_llvm(false_exp, ctx);
            monga_ast_unconditional_jump_llvm(false_end_label);

            /* false_end: */
            monga_ast_label_definition_llvm(false_end_label);
            monga_ast_unconditional_jump_llvm(eval_label);

            /* eval: */
            monga_ast_label_definition_llvm(eval_label);
            ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("phi ");
            monga_ast_typedesc_reference_llvm(true_exp->typedesc);
            printf(" [ ");
            monga_ast_expression_reference_llvm(true_exp);
            printf(", ");
            monga_ast_label_reference_llvm(true_end_label);
            printf(" ], [ ");
            monga_ast_expression_reference_llvm(false_exp);
            printf(", ");
            monga_ast_label_reference_llvm(false_end_label);
            printf(" ]\n");

            break;
        }
        default:
            monga_unreachable();
    }
    if (ast->next != NULL)
        monga_ast_expression_llvm(ast->next, ctx);
}

void monga_ast_condition_llvm(struct monga_ast_condition_t* ast, struct monga_ast_llvm_context_t* ctx, size_t true_label, size_t false_label)
{
    switch (ast->tag) {
        case MONGA_AST_CONDITION_EQ:
        case MONGA_AST_CONDITION_NE:
        case MONGA_AST_CONDITION_LE:
        case MONGA_AST_CONDITION_GE:
        case MONGA_AST_CONDITION_LT:
        case MONGA_AST_CONDITION_GT:
        {
            size_t result;
            struct monga_ast_expression_t* exp1, *exp2;
            struct monga_ast_typedesc_t* typedesc;
            enum monga_ast_typedesc_builtin_t builtin;
            builtin_instruction_getter instruction_getter;
            const char* cmp, *op;

            instruction_getter = monga_ast_condition_binop_instruction_getter(ast);
            monga_assert(instruction_getter != NULL);

            exp1 = ast->u.exp_binop_cond.exp1;
            exp2 = ast->u.exp_binop_cond.exp2;
            typedesc = exp1->typedesc;

            monga_assert(typedesc->tag == MONGA_AST_TYPEDESC_BUILTIN);
            builtin = typedesc->u.builtin_typedesc;

            cmp = monga_ast_builtin_llvm_cmp_instruction(builtin);
            monga_assert(cmp != NULL);

            op = instruction_getter(builtin);
            monga_assert(op != NULL);

            monga_ast_expression_llvm(exp1, ctx);
            monga_ast_expression_llvm(exp2, ctx);

            result = monga_ast_tempvar_new_assign_llvm(ctx);
            printf("%s %s ", cmp, op);
            monga_ast_typedesc_reference_llvm(typedesc);
            printf(" ");
            monga_ast_expression_reference_llvm(exp1);
            printf(", ");
            monga_ast_expression_reference_llvm(exp2);
            printf("\n");

            printf("\tbr i1 ");
            monga_ast_tempvar_reference_llvm(result);
            printf(", label ");
            monga_ast_label_reference_llvm(true_label);
            printf(", label ");
            monga_ast_label_reference_llvm(false_label);
            printf("\n");

            break;
        }
        case MONGA_AST_CONDITION_AND:
        {
            size_t mid_label = monga_ast_label_new_llvm(ctx);
            monga_ast_condition_llvm(ast->u.cond_binop_cond.cond1, ctx, mid_label, false_label);
            monga_ast_label_definition_llvm(mid_label);
            monga_ast_condition_llvm(ast->u.cond_binop_cond.cond2, ctx, true_label, false_label);
            break;
        }
        case MONGA_AST_CONDITION_OR:
        {
            size_t mid_label = monga_ast_label_new_llvm(ctx);
            monga_ast_condition_llvm(ast->u.cond_binop_cond.cond1, ctx, true_label, mid_label);
            monga_ast_label_definition_llvm(mid_label);
            monga_ast_condition_llvm(ast->u.cond_binop_cond.cond2, ctx, true_label, false_label);
            break;
        }
        case MONGA_AST_CONDITION_NOT:
            monga_ast_condition_llvm(ast->u.cond_unop_cond.cond, ctx, false_label, true_label);
            break;
        default:
            monga_unreachable();
    }
}

void monga_ast_call_llvm(struct monga_ast_call_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    struct monga_ast_reference_t* def_function_ref;
    struct monga_ast_def_function_t* def_function;

    def_function_ref = &ast->function;
    monga_assert(def_function_ref->tag == MONGA_AST_REFERENCE_FUNCTION);
    def_function = def_function_ref->u.def_function;
    
    if (ast->expressions != NULL)
        monga_ast_expression_llvm(ast->expressions->first, ctx);

    if (def_function->type.id != NULL)
        ast->tempvar_id = monga_ast_tempvar_new_assign_llvm(ctx);
    else
        printf("\t");
    
    printf("call ");
    monga_ast_def_function_return_llvm(def_function);
    printf(" @%s(", def_function->id);
    
    if (ast->expressions != NULL) {
        struct monga_ast_node_iter_t iter = {
            .get_node_typedesc = expression_typedesc_getter,
            .get_next_node = expression_next_getter,
            .visit_after = expression_visit_after,
        };

        monga_ast_typedesc_reference_iter_llvm(ast->expressions->first, &iter);
    }
    
    printf(")\n");
}

/* Static function definitions */

void monga_ast_def_variable_reference_llvm(struct monga_ast_def_variable_t* def_variable)
{
    if (def_variable->is_global) {
        printf("@%s", def_variable->id);
    } else {
        monga_ast_tempvar_reference_llvm(def_variable->tempvar_id);
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
    struct monga_ast_llvm_context_t* ctx = (struct monga_ast_llvm_context_t*) arg;
    def_variable_->tempvar_id = ctx->tempvar_count++;
    printf(" ");
    monga_ast_def_variable_reference_llvm(def_variable_);
}

void monga_ast_typedesc_reference_iter_llvm(void* node, struct monga_ast_node_iter_t* iter)
{
    struct monga_ast_typedesc_t* typedesc;
    while (node != NULL) {
        typedesc = iter->get_node_typedesc(node);
        
        if (iter->visit_before != NULL)
            iter->visit_before(node, iter->visit_arg);
        
        monga_ast_typedesc_reference_llvm(typedesc);

        if (iter->visit_after != NULL)
            iter->visit_after(node, iter->visit_arg);
        
        node = iter->get_next_node(node);
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
    case MONGA_AST_TYPEDESC_RECORD:
        monga_ast_typedesc_subtype_reference_llvm(ast);
        printf("*");
        break;
    default:
        monga_unreachable();
    }
}

void monga_ast_typedesc_subtype_reference_llvm(struct monga_ast_typedesc_t* ast)
{
    ast = monga_ast_typedesc_resolve_id(ast);
    switch (ast->tag) {
    case MONGA_AST_TYPEDESC_BUILTIN:
        monga_unreachable(); /* no builtin type has subtype */
        break;
    case MONGA_AST_TYPEDESC_ID:
        monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
        monga_ast_typedesc_reference_llvm(ast->u.array_typedesc);
        break;
    case MONGA_AST_TYPEDESC_RECORD:
        monga_ast_struct_reference_llvm(ast->u.record_typedesc.llvm_struct_id);
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

void monga_ast_expression_reference_llvm(struct monga_ast_expression_t* expression)
{
    monga_ast_tempvar_reference_llvm(expression->tempvar_id);
}

void monga_ast_variable_reference_llvm(struct monga_ast_variable_t* variable)
{
    switch(variable->tag) {
    case MONGA_AST_VARIABLE_ID:
    {
        struct monga_ast_reference_t* id_var = &variable->u.id_var;

        monga_assert(id_var->tag == MONGA_AST_REFERENCE_VARIABLE);
        monga_ast_def_variable_reference_llvm(id_var->u.def_variable);
        break;
    }
    case MONGA_AST_VARIABLE_ARRAY:
        monga_ast_tempvar_reference_llvm(variable->tempvar_id);
        break;
    case MONGA_AST_VARIABLE_RECORD:
        monga_ast_tempvar_reference_llvm(variable->tempvar_id);
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
    monga_ast_expression_reference_llvm(expression_);
    if (expression_->next != NULL)
        printf(", ");
}

void monga_ast_unconditional_jump_llvm(size_t llvm_label_id)
{
    printf("\tbr label ");
    monga_ast_label_reference_llvm(llvm_label_id);
    printf("\n");
}

void monga_ast_struct_reference_llvm(size_t llvm_struct_id)
{
    printf("%%s%zu", llvm_struct_id);
}

size_t monga_ast_struct_new_llvm(struct monga_ast_llvm_context_t* ctx)
{
    return ctx->struct_count++;
}

size_t monga_ast_struct_new_define_llvm(struct monga_ast_llvm_context_t* ctx)
{
    size_t llvm_struct_id = monga_ast_struct_new_llvm(ctx);
    monga_ast_struct_reference_llvm(llvm_struct_id);
    printf(" = type ");
    return llvm_struct_id;
}

void monga_ast_tempvar_reference_llvm(size_t llvm_var_id)
{
    printf("%%t%zu", llvm_var_id);
}

size_t monga_ast_tempvar_new_llvm(struct monga_ast_llvm_context_t* ctx)
{
    return ctx->tempvar_count++;
}

size_t monga_ast_tempvar_new_assign_llvm(struct monga_ast_llvm_context_t* ctx)
{
    size_t llvm_var_id = monga_ast_tempvar_new_llvm(ctx);
    printf("\t");
    monga_ast_tempvar_reference_llvm(llvm_var_id);
    printf(" = ");
    return llvm_var_id;
}

void monga_ast_label_reference_llvm(size_t llvm_label_id)
{
    printf("%%l%zu", llvm_label_id);
}

void monga_ast_label_definition_llvm(size_t llvm_label_id)
{
    printf("l%zu:\n", llvm_label_id);
}

size_t monga_ast_label_new_llvm(struct monga_ast_llvm_context_t* ctx)
{
    return ctx->label_count++;
}

builtin_instruction_getter monga_ast_expression_binop_instruction_getter(struct monga_ast_expression_t* ast)
{
    switch (ast->tag) {
    case MONGA_AST_EXPRESSION_ADDITION:
        return monga_ast_builtin_llvm_add_instruction;
    case MONGA_AST_EXPRESSION_SUBTRACTION:
        return monga_ast_builtin_llvm_sub_instruction;
    case MONGA_AST_EXPRESSION_MULTIPLICATION:
        return monga_ast_builtin_llvm_mul_instruction;
    case MONGA_AST_EXPRESSION_DIVISION:
        return monga_ast_builtin_llvm_div_instruction;
    default:
        return NULL;
    }
}

builtin_instruction_getter monga_ast_condition_binop_instruction_getter(struct monga_ast_condition_t* ast)
{
    switch (ast->tag) {
    case MONGA_AST_CONDITION_EQ:
        return monga_ast_builtin_llvm_eq_instruction;
    case MONGA_AST_CONDITION_NE:
        return monga_ast_builtin_llvm_ne_instruction;
    case MONGA_AST_CONDITION_GT:
        return monga_ast_builtin_llvm_gt_instruction;
    case MONGA_AST_CONDITION_LT:
        return monga_ast_builtin_llvm_lt_instruction;
    case MONGA_AST_CONDITION_GE:
        return monga_ast_builtin_llvm_ge_instruction;
    case MONGA_AST_CONDITION_LE:
        return monga_ast_builtin_llvm_le_instruction;
    default:
        return NULL;
    }
}

enum monga_ast_llvm_printf_fmt_t monga_ast_typedesc_printf_fmt(struct monga_ast_typedesc_t* typedesc)
{
    typedesc = monga_ast_typedesc_resolve_id(typedesc);

    switch (typedesc->tag) {
    case MONGA_AST_TYPEDESC_BUILTIN:
    {
        switch (typedesc->u.builtin_typedesc) {
        case MONGA_AST_TYPEDESC_BUILTIN_INT:
            return MONGA_AST_LLVM_PRINTF_FMT_INT;
        case MONGA_AST_TYPEDESC_BUILTIN_FLOAT:
            return MONGA_AST_LLVM_PRINTF_FMT_FLOAT;
        default:
            monga_unreachable();
        }
        break;
    }
    case MONGA_AST_TYPEDESC_ID:
        monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
    case MONGA_AST_TYPEDESC_RECORD:
        return MONGA_AST_LLVM_PRINTF_FMT_PTR;
    default:
        monga_unreachable();
    }

    return 0;
}

const char* monga_ast_printf_fmt_constant_name(enum monga_ast_llvm_printf_fmt_t fmt)
{
    switch (fmt) {
    case MONGA_AST_LLVM_PRINTF_FMT_INT:
        return ".fmt.int";
    case MONGA_AST_LLVM_PRINTF_FMT_FLOAT:
        return ".fmt.float";
    case MONGA_AST_LLVM_PRINTF_FMT_PTR:
        return ".fmt.ptr";
    default:
        monga_unreachable();
        return NULL;
    }
}

const char* monga_ast_printf_fmt_constant_string(enum monga_ast_llvm_printf_fmt_t fmt)
{
    switch (fmt) {
    case MONGA_AST_LLVM_PRINTF_FMT_INT:
        return "%d\\0A\\00";
    case MONGA_AST_LLVM_PRINTF_FMT_FLOAT:
        return "%f\\0A\\00";
    case MONGA_AST_LLVM_PRINTF_FMT_PTR:
        return "%p\\0A\\00";
    default:
        monga_unreachable();
        return NULL;
    }
}

size_t monga_ast_printf_fmt_size(enum monga_ast_llvm_printf_fmt_t monga_unused(fmt))
{
    return 4;
}

const char* monga_ast_func_declaration(enum monga_ast_llvm_func_t func)
{
    switch (func) {
    case MONGA_AST_LLVM_FUNC_MALLOC:
        return "i8* @malloc(i64)";
    case MONGA_AST_LLVM_FUNC_PRINTF:
        return "i32 @printf(i8*, ...)";
    default:
        monga_unreachable();
        return NULL;
    }
}