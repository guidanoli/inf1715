#include "monga_ast_llvm.h"
#include "monga_ast_builtin.h"
#include "monga_ast_typedesc.h"

#include <stdio.h>

/* Type definitions */

typedef struct monga_ast_typedesc_t* (typedesc_getter)(void*);
typedef void* (next_getter)(void*);
typedef void (visiter)(void*, void*);
typedef const char* (*builtin_instruction_getter)(enum monga_ast_typedesc_builtin_t);

/* Static mappings */

static builtin_instruction_getter exp_binop_instruction_getters[MONGA_AST_EXPRESSION_CNT] = {
    [MONGA_AST_EXPRESSION_ADDITION] = monga_ast_builtin_llvm_add_instruction,
    [MONGA_AST_EXPRESSION_SUBTRACTION] = monga_ast_builtin_llvm_sub_instruction,
    [MONGA_AST_EXPRESSION_MULTIPLICATION] = monga_ast_builtin_llvm_mul_instruction,
    [MONGA_AST_EXPRESSION_DIVISION] = monga_ast_builtin_llvm_div_instruction
};

static builtin_instruction_getter cond_binop_instruction_getters[MONGA_AST_CONDITION_CNT] = {
    [MONGA_AST_CONDITION_EQ] = monga_ast_builtin_llvm_eq_instruction,
    [MONGA_AST_CONDITION_NE] = monga_ast_builtin_llvm_ne_instruction,
    [MONGA_AST_CONDITION_GT] = monga_ast_builtin_llvm_gt_instruction,
    [MONGA_AST_CONDITION_LT] = monga_ast_builtin_llvm_lt_instruction,
    [MONGA_AST_CONDITION_GE] = monga_ast_builtin_llvm_ge_instruction,
    [MONGA_AST_CONDITION_LE] = monga_ast_builtin_llvm_le_instruction,
};

/* Static function prototypes */

static void monga_ast_typedesc_reference_llvm(struct monga_ast_typedesc_t* ast);
static void monga_ast_typedesc_reference_list_llvm(void* node, typedesc_getter get_node_typedesc, next_getter get_next_node, visiter visit_before, visiter visit_after, void* visit_arg);

static void monga_ast_def_variable_store_llvm(struct monga_ast_def_variable_t* def_variable, size_t value_id);
static void monga_ast_def_variable_list_allocation_llvm(struct monga_ast_def_variable_list_t* def_variable_list, struct monga_ast_llvm_context_t* ctx);
static void monga_ast_def_variable_reference_llvm(struct monga_ast_def_variable_t* def_variable);

static void monga_ast_variable_reference_llvm(struct monga_ast_variable_t* variable);

static void monga_ast_def_function_return_llvm(struct monga_ast_def_function_t* ast);

static int monga_ast_get_field_index(struct monga_ast_field_list_t* field_list, struct monga_ast_field_t* field);

static void monga_ast_expression_value_reference_llvm(struct monga_ast_expression_t* expression);

/* todo: uniform temporary, struct and label creation and referencing */
static void monga_ast_struct_reference_llvm(size_t struct_id);
static size_t monga_ast_new_struct_llvm(struct monga_ast_llvm_context_t* ctx);

static void monga_ast_temporary_reference_llvm(size_t var_id);
static size_t monga_ast_new_temporary_llvm(struct monga_ast_llvm_context_t* ctx);

static void monga_ast_unconditional_jump_llvm(size_t label_id);

static void monga_ast_label_tag_llvm(size_t label_id);
static void monga_ast_label_reference_llvm(size_t label_id);
static size_t monga_ast_new_label_llvm(struct monga_ast_llvm_context_t* ctx);

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
    if (ast->definitions != NULL) {
        struct monga_ast_llvm_context_t ctx;

        ctx.def_function = NULL;
        ctx.struct_count = 0;
        ctx.variable_count = 0;
        ctx.label_count = 0;

        monga_ast_definition_llvm(ast->definitions->first, &ctx);
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

void monga_ast_def_variable_llvm(struct monga_ast_def_variable_t* ast, struct monga_ast_llvm_context_t* monga_unused(ctx))
{
    printf("@%s = internal global ", ast->id);
    monga_assert(ast->type.tag == MONGA_AST_REFERENCE_TYPE);
    monga_ast_typedesc_reference_llvm(ast->type.u.def_type->typedesc);
    printf(" undef\n");
}

void monga_ast_def_type_llvm(struct monga_ast_def_type_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    monga_ast_typedesc_llvm(ast->typedesc, ctx);
}

void monga_ast_def_function_llvm(struct monga_ast_def_function_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    ctx->def_function = ast;
    ctx->variable_count = 0;
    ctx->label_count = 0;

    printf("define ");
    monga_ast_def_function_return_llvm(ast);
    printf(" @%s(", ast->id);

    if (ast->parameters != NULL) {
        monga_ast_typedesc_reference_list_llvm(ast->parameters->first,
            def_variable_typedesc_getter, def_variable_next_getter, NULL,
            def_variable_visit_after, ctx);
    }

    printf(") {\n");

    if (ast->parameters != NULL) {
        struct monga_ast_def_variable_t* parameter = ast->parameters->first;
        size_t parameter_id = 0;

        monga_ast_def_variable_list_allocation_llvm(ast->parameters, ctx);
        for (; parameter != NULL; ++parameter_id, parameter = parameter->next)
            monga_ast_def_variable_store_llvm(parameter, parameter_id);
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
        ast->u.record_typedesc.llvm_struct_id = monga_ast_new_struct_llvm(ctx);
        printf("{ ");
        monga_ast_typedesc_reference_list_llvm(ast->u.record_typedesc.field_list->first,
            field_typedesc_getter, field_next_getter, NULL, NULL, NULL);
        printf(" }\n");
        break;
    default:
        monga_unreachable();
    }
}

void monga_ast_block_llvm(struct monga_ast_block_t* ast, struct monga_ast_llvm_context_t* ctx)
{
    if (ast->variables != NULL)
        monga_ast_def_variable_list_allocation_llvm(ast->variables, ctx);
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

            then_label = monga_ast_new_label_llvm(ctx);
            else_label = monga_ast_new_label_llvm(ctx);
            if (else_block != NULL)
                endif_label = monga_ast_new_label_llvm(ctx);
            else
                endif_label = else_label;

            monga_ast_condition_llvm(cond, ctx, then_label, else_label);

            /* then: */
            monga_ast_label_tag_llvm(then_label);
            monga_ast_block_llvm(then_block, ctx);
            printf("\tbr label ");
            monga_ast_label_reference_llvm(endif_label);
            printf("\n");
            
            /* else: */
            if (else_block != NULL) {
                monga_ast_label_tag_llvm(else_label);
                monga_ast_block_llvm(else_block, ctx);
                printf("\tbr label ");
                monga_ast_label_reference_llvm(endif_label);
                printf("\n");
            }

            /* endif: */
            monga_ast_label_tag_llvm(endif_label);

            break;
        }
        case MONGA_AST_STATEMENT_WHILE:
        {
            size_t do_label, done_label;
            struct monga_ast_condition_t* cond;
            struct monga_ast_block_t* loop_block;

            cond = ast->u.while_stmt.cond;
            loop_block = ast->u.while_stmt.loop;

            do_label = monga_ast_new_label_llvm(ctx);
            done_label = monga_ast_new_label_llvm(ctx);

            monga_ast_condition_llvm(cond, ctx, do_label, done_label);

            /* do: */
            monga_ast_label_tag_llvm(do_label);
            monga_ast_block_llvm(loop_block, ctx);
            monga_ast_condition_llvm(cond, ctx, do_label, done_label);

            /* done: */
            monga_ast_label_tag_llvm(done_label);

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
                monga_ast_expression_llvm(exp, ctx);

            printf("\tret ");
            monga_ast_def_function_return_llvm(ctx->def_function);
            if (exp != NULL) {
                printf(" ");
                monga_ast_expression_value_reference_llvm(exp);
            }
            printf("\n");
            
            break;
        }
        case MONGA_AST_STATEMENT_CALL:
            monga_ast_call_llvm(ast->u.call_stmt.call, ctx);
            break;
        case MONGA_AST_STATEMENT_PRINT:
            /* TODO -- create calls to a printing function */
            break;
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
            
            monga_ast_expression_llvm(array_exp, ctx);
            monga_ast_expression_llvm(index_exp, ctx);

            index_i64_llvm_var_id = monga_ast_new_temporary_llvm(ctx);
            printf("sext i32 ");
            monga_ast_expression_value_reference_llvm(index_exp);
            printf(" to i64\n");
            
            monga_assert(array_exp->typedesc->tag == MONGA_AST_TYPEDESC_ARRAY);
            typedesc = array_exp->typedesc->u.array_typedesc;

            ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
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
            int field_index;
            size_t struct_id;

            record_exp = ast->u.record_var.record;
            monga_ast_expression_llvm(record_exp, ctx);
            
            monga_assert(ast->u.record_var.field.tag == MONGA_AST_REFERENCE_FIELD);
            record_field = ast->u.record_var.field.u.field;
            
            monga_assert(record_exp->typedesc->tag == MONGA_AST_TYPEDESC_RECORD);
            field_list = record_exp->typedesc->u.record_typedesc.field_list;
            
            field_index = monga_ast_get_field_index(field_list, record_field);

            monga_assert(record_exp->typedesc->tag == MONGA_AST_TYPEDESC_RECORD);
            struct_id = record_exp->typedesc->u.record_typedesc.llvm_struct_id;

            ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
            printf("getelementptr ");
            monga_ast_struct_reference_llvm(struct_id);
            printf(", ");
            monga_ast_struct_reference_llvm(struct_id);
            printf("* ");
            monga_ast_expression_value_reference_llvm(record_exp);
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

            ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
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

            ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
            printf("fadd ");
            monga_ast_typedesc_reference_llvm(ast->typedesc);
            printf(" %f, %s\n", real, zero);

            break;
        }
        case MONGA_AST_EXPRESSION_VAR:
        {
            struct monga_ast_variable_t* var = ast->u.var_exp.var;

            monga_ast_variable_llvm(var, ctx);

            ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
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
            ast->llvm_var_id = call->llvm_var_id;

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
                    ast->llvm_var_id = exp->llvm_var_id; /* no conversion */
                }
                else
                {
                    const char* instruction = monga_ast_builtin_llvm_cast_instruction(to, from);
                    monga_assert(instruction != NULL);
                    ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
                    printf("%s %s ", instruction, monga_ast_builtin_typedesc_llvm(from));
                    monga_ast_expression_value_reference_llvm(exp);
                    printf(" to %s\n", monga_ast_builtin_typedesc_llvm(to));
                }
                break;
            }
            case MONGA_AST_TYPEDESC_ID:
                monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
                break;
            case MONGA_AST_TYPEDESC_ARRAY:
                ast->llvm_var_id = exp->llvm_var_id; /* no conversion */
                break;
            case MONGA_AST_TYPEDESC_RECORD:
                ast->llvm_var_id = exp->llvm_var_id; /* no conversion */
                break;
            default:
                monga_unreachable();
            }

            break;
        }
        case MONGA_AST_EXPRESSION_NEW:
            /* TODO -- call memory allocation function */
            break;
        case MONGA_AST_EXPRESSION_NEGATIVE:
        {
            struct monga_ast_expression_t* exp = ast->u.negative_exp.exp;
            struct monga_ast_typedesc_t* typedesc = exp->typedesc;
            enum monga_ast_typedesc_builtin_t builtin_typedesc;
            builtin_instruction_getter instruction_getter;
            const char* instruction;
            const char* zero;
            
            monga_ast_expression_llvm(exp, ctx);

            typedesc = monga_ast_typedesc_resolve_id(typedesc);

            monga_assert(typedesc->tag == MONGA_AST_TYPEDESC_BUILTIN);
            builtin_typedesc = typedesc->u.builtin_typedesc;

            instruction_getter = exp_binop_instruction_getters[MONGA_AST_EXPRESSION_SUBTRACTION];
            monga_assert(instruction_getter != NULL);

            instruction = instruction_getter(builtin_typedesc);
            monga_assert(instruction != NULL);

            zero = monga_ast_builtin_typedesc_zero_llvm(builtin_typedesc);
            monga_assert(zero != NULL);

            ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
            printf("%s %s %s, ", instruction, monga_ast_builtin_typedesc_llvm(builtin_typedesc), zero);
            monga_ast_expression_value_reference_llvm(exp);
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

            instruction_getter = exp_binop_instruction_getters[ast->tag];
            monga_assert(instruction_getter != NULL);

            instruction = instruction_getter(builtin_typedesc);
            monga_assert(instruction != NULL);

            ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
            printf("%s %s ", instruction, monga_ast_builtin_typedesc_llvm(builtin_typedesc));
            monga_ast_expression_value_reference_llvm(exp1);
            printf(", ");
            monga_ast_expression_value_reference_llvm(exp2);
            printf("\n");

            break;
        }
        case MONGA_AST_EXPRESSION_CONDITIONAL:
        {
            struct monga_ast_condition_t* cond = ast->u.conditional_exp.cond;
            struct monga_ast_expression_t* true_exp = ast->u.conditional_exp.true_exp;
            struct monga_ast_expression_t* false_exp = ast->u.conditional_exp.false_exp;
            size_t true_label, true_end_label, false_label, false_end_label, eval_label;

            true_label = monga_ast_new_label_llvm(ctx);
            true_end_label = monga_ast_new_label_llvm(ctx);
            false_label = monga_ast_new_label_llvm(ctx);
            false_end_label = monga_ast_new_label_llvm(ctx);
            eval_label = monga_ast_new_label_llvm(ctx);

            monga_ast_condition_llvm(cond, ctx, true_label, false_label);

            /* true: */
            monga_ast_label_tag_llvm(true_label);
            monga_ast_expression_llvm(true_exp, ctx);
            monga_ast_unconditional_jump_llvm(true_end_label);

            /* true_end: */
            monga_ast_label_tag_llvm(true_end_label);
            monga_ast_unconditional_jump_llvm(eval_label);

            /* false: */
            monga_ast_label_tag_llvm(false_label);
            monga_ast_expression_llvm(false_exp, ctx);
            monga_ast_unconditional_jump_llvm(false_end_label);

            /* false_end: */
            monga_ast_label_tag_llvm(false_end_label);
            monga_ast_unconditional_jump_llvm(eval_label);

            /* eval: */
            monga_ast_label_tag_llvm(eval_label);
            ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
            printf("phi ");
            monga_ast_typedesc_reference_llvm(true_exp->typedesc);
            printf(" [ ");
            monga_ast_expression_value_reference_llvm(true_exp);
            printf(", ");
            monga_ast_label_reference_llvm(true_end_label);
            printf(" ], [ ");
            monga_ast_expression_value_reference_llvm(false_exp);
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

            instruction_getter = cond_binop_instruction_getters[ast->tag];
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

            result = monga_ast_new_temporary_llvm(ctx);
            printf("%s %s ", cmp, op);
            monga_ast_typedesc_reference_llvm(typedesc);
            printf(" ");
            monga_ast_expression_value_reference_llvm(exp1);
            printf(", ");
            monga_ast_expression_value_reference_llvm(exp2);
            printf("\n");

            printf("\tbr i1 ");
            monga_ast_temporary_reference_llvm(result);
            printf(", label ");
            monga_ast_label_reference_llvm(true_label);
            printf(", label ");
            monga_ast_label_reference_llvm(false_label);
            printf("\n");

            break;
        }
        case MONGA_AST_CONDITION_AND:
        {
            size_t mid_label = monga_ast_new_label_llvm(ctx);
            monga_ast_condition_llvm(ast->u.cond_binop_cond.cond1, ctx, mid_label, false_label);
            monga_ast_label_tag_llvm(mid_label);
            monga_ast_condition_llvm(ast->u.cond_binop_cond.cond2, ctx, true_label, false_label);
            break;
        }
        case MONGA_AST_CONDITION_OR:
        {
            size_t mid_label = monga_ast_new_label_llvm(ctx);
            monga_ast_condition_llvm(ast->u.cond_binop_cond.cond1, ctx, true_label, mid_label);
            monga_ast_label_tag_llvm(mid_label);
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
        ast->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
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

void monga_ast_def_variable_list_allocation_llvm(struct monga_ast_def_variable_list_t* def_variable_list, struct monga_ast_llvm_context_t* ctx)
{
    struct monga_ast_def_variable_t* def_variable;

    for (def_variable = def_variable_list->first;
        def_variable != NULL;
        def_variable = def_variable->next) {
        
        struct monga_ast_typedesc_t* typedesc;

        monga_assert(def_variable->type.tag == MONGA_AST_REFERENCE_TYPE);
        typedesc = def_variable->type.u.def_type->typedesc;

        def_variable->llvm_var_id = monga_ast_new_temporary_llvm(ctx);
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
    struct monga_ast_llvm_context_t* ctx = (struct monga_ast_llvm_context_t*) arg;
    def_variable_->llvm_var_id = ctx->variable_count;
    ctx->variable_count += 1;
    printf(" ");
    monga_ast_def_variable_reference_llvm(def_variable_);
}

void monga_ast_typedesc_reference_list_llvm(void* node, typedesc_getter get_node_typedesc, next_getter get_next_node, visiter visit_before, visiter visit_after, void* visit_arg)
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
        monga_ast_struct_reference_llvm(ast->u.record_typedesc.llvm_struct_id);
        printf("*");
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

int monga_ast_get_field_index(struct monga_ast_field_list_t* field_list, struct monga_ast_field_t* field)
{
    int index = 0;

    for (struct monga_ast_field_t* f = field_list->first;
        f != NULL && f != field;
        f = f->next, ++index);

    return index;
}

void monga_ast_expression_value_reference_llvm(struct monga_ast_expression_t* expression)
{
    monga_ast_temporary_reference_llvm(expression->llvm_var_id);
}

size_t monga_ast_new_temporary_llvm(struct monga_ast_llvm_context_t* ctx)
{
    size_t var_count = ctx->variable_count;
    printf("\t");
    monga_ast_temporary_reference_llvm(var_count);
    printf(" = ");
    ctx->variable_count += 1;
    return var_count;
}

void monga_ast_temporary_reference_llvm(size_t var_id)
{
    printf("%%t%zu", var_id);
}

size_t monga_ast_new_struct_llvm(struct monga_ast_llvm_context_t* ctx)
{
    size_t struct_count = ctx->struct_count;
    monga_ast_struct_reference_llvm(struct_count);
    printf(" = type ");
    ctx->struct_count += 1;
    return struct_count;
}

void monga_ast_struct_reference_llvm(size_t struct_id)
{
    printf("%%S%zu", struct_id);
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

void monga_ast_label_tag_llvm(size_t label_id)
{
    printf("l%zu:\n", label_id);
}

void monga_ast_label_reference_llvm(size_t label_id)
{
    printf("%%l%zu", label_id);
}

size_t monga_ast_new_label_llvm(struct monga_ast_llvm_context_t* ctx)
{
    size_t label_id = ctx->label_count;
    ctx->label_count += 1;
    return label_id;
}

void monga_ast_unconditional_jump_llvm(size_t label_id)
{
    printf("\tbr label ");
    monga_ast_label_reference_llvm(label_id);
    printf("\n");
}