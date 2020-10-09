#ifndef MONGA_AST_H
#define MONGA_AST_H

#include "monga_utils.h"

/* Incomplete types */

struct monga_ast_def_variable_t;
struct monga_ast_block_t;
struct monga_ast_expression_t;

/* Type definitions */

struct monga_ast_expression_list_t
{
    struct monga_ast_expression_t *first;
    struct monga_ast_expression_t *last;
};

struct monga_ast_call_t
{
    char *function_id;
    struct monga_ast_expression_list_t *expressions; /* nullable */
};

struct monga_ast_condition_t
{
    enum {
        MONGA_AST_CONDITION_EQ,
        MONGA_AST_CONDITION_NE,
        MONGA_AST_CONDITION_LE,
        MONGA_AST_CONDITION_GE,
        MONGA_AST_CONDITION_LT,
        MONGA_AST_CONDITION_GT,
        MONGA_AST_CONDITION_NOT,
        MONGA_AST_CONDITION_AND,
        MONGA_AST_CONDITION_OR,
    } tag;
    union {
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } exp_binop_cond;
        struct {
            struct monga_ast_condition_t *cond1;
            struct monga_ast_condition_t *cond2;
        } cond_binop_cond;
        struct {
            struct monga_ast_condition_t *cond;
        } cond_unop_cond;
    };
};

struct monga_ast_expression_t
{
    enum {
        MONGA_AST_EXPRESSION_INTEGER,
        MONGA_AST_EXPRESSION_REAL,
        MONGA_AST_EXPRESSION_VAR,
        MONGA_AST_EXPRESSION_CALL,
        MONGA_AST_EXPRESSION_CAST,
        MONGA_AST_EXPRESSION_NEW,
        MONGA_AST_EXPRESSION_NEGATIVE,
        MONGA_AST_EXPRESSION_ADDITION,
        MONGA_AST_EXPRESSION_SUBTRACTION,
        MONGA_AST_EXPRESSION_MULTIPLICATION,
        MONGA_AST_EXPRESSION_DIVISION,
        MONGA_AST_EXPRESSION_CONDITIONAL,
    } tag;
    union {
        struct {
            int integer;
        } integer_exp;
        struct {
            double real;
        } real_exp;
        struct {
            struct monga_ast_variable_t *var;
        } var_exp;
        struct {
            struct monga_ast_call_t *call;
        } call_exp;
        struct {
            struct monga_ast_expression_t *exp;
            char *type;
        } cast_exp;
        struct {
            char *type;
            struct monga_ast_expression_t *exp; /* nullable */
        } new_exp;
        struct {
            struct monga_ast_expression_t *exp;
        } negative_exp;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } binop_exp;
        struct {
            struct monga_ast_condition_t *cond;
            struct monga_ast_expression_t *true_exp;
            struct monga_ast_expression_t *false_exp;
        } conditional_exp;
    };
    struct monga_ast_expression_t *next; /* nullable */
};

struct monga_ast_variable_t
{
    enum {
        MONGA_AST_VARIABLE_ID,
        MONGA_AST_VARIABLE_ARRAY,
        MONGA_AST_VARIABLE_RECORD,
    } tag;
    union {
        struct {
            char *id;
        } id_var;
        struct {
            struct monga_ast_expression_t *array;
            struct monga_ast_expression_t *index;
        } array_var;
        struct {
            struct monga_ast_expression_t *record;
            char *field;
        } record_var;
    };
};

struct monga_ast_statement_t
{
    enum {
        MONGA_AST_STATEMENT_IF,
        MONGA_AST_STATEMENT_WHILE,
        MONGA_AST_STATEMENT_ASSIGN,
        MONGA_AST_STATEMENT_RETURN,
        MONGA_AST_STATEMENT_CALL,
        MONGA_AST_STATEMENT_PRINT,
        MONGA_AST_STATEMENT_BLOCK,
    } tag;
    union {
        struct {
            struct monga_ast_condition_t* cond;
            struct monga_ast_block_t* then_block;
            struct monga_ast_block_t* else_block; /* nullable */
        } if_stmt;
        struct {
            struct monga_ast_condition_t* cond;
            struct monga_ast_block_t* loop;
        } while_stmt;
        struct {
            struct monga_ast_variable_t* var;
            struct monga_ast_expression_t* exp;
        } assign_stmt;
        struct {
            struct monga_ast_expression_t* exp; /* nullable */
        } return_stmt;
        struct {
            struct monga_ast_call_t* call;
        } call_stmt;
        struct {
            struct monga_ast_expression_t* exp;
        } print_stmt;
        struct {
            struct monga_ast_block_t* block;
        } block_stmt;
    };
    struct monga_ast_statement_t *next; /* nullable */
};

struct monga_ast_def_variable_list_t
{
    struct monga_ast_def_variable_t *first;
    struct monga_ast_def_variable_t *last;
};

struct monga_ast_statement_list_t
{
    struct monga_ast_statement_t *first;
    struct monga_ast_statement_t *last;
};

struct monga_ast_block_t
{
    struct monga_ast_def_variable_list_t *variables; /* nullable */
    struct monga_ast_statement_list_t *statements; /* nullable */
};

struct monga_ast_parameter_t
{
    char *id;
    char *type;
    struct monga_ast_parameter_t *next; /* nullable */
};

struct monga_ast_field_t
{
    char *id;
    char *type;
    struct monga_ast_field_t *next; /* nullable */
};

struct monga_ast_field_list_t
{
    struct monga_ast_field_t *first;
    struct monga_ast_field_t *last;
};

struct monga_ast_typedesc_t
{
    enum {
        MONGA_AST_TYPEDESC_ID,
        MONGA_AST_TYPEDESC_ARRAY,
        MONGA_AST_TYPEDESC_RECORD,
    } tag;
    union {
        char* id_typedesc;
        struct monga_ast_typedesc_t* array_typedesc;
        struct monga_ast_field_list_t* record_typedesc;
    };
};

struct monga_ast_parameter_list_t
{
    struct monga_ast_parameter_t *first;
    struct monga_ast_parameter_t *last;
};

struct monga_ast_def_function_t
{
    char *id;
    struct monga_ast_parameter_list_t *parameters; /* nullable */
    char *type; /* nullable */
    struct monga_ast_block_t *block;
};

struct monga_ast_def_type_t
{
    char *id;
    struct monga_ast_typedesc_t *typedesc;
};

struct monga_ast_def_variable_t
{
    char *id;
    char *type;
    struct monga_ast_def_variable_t *next; /* nullable */
};

struct monga_ast_definition_t
{
    enum {
        MONGA_AST_DEFINITION_VARIABLE,
        MONGA_AST_DEFINITION_TYPE,
        MONGA_AST_DEFINITION_FUNCTION,
    } tag;
    union {
        struct monga_ast_def_variable_t *def_variable;
        struct monga_ast_def_type_t *def_type;
        struct monga_ast_def_function_t *def_function;
    };
    struct monga_ast_definition_t *next; /* nullable */
};

struct monga_ast_definition_list_t
{
    struct monga_ast_definition_t *first;
    struct monga_ast_definition_t *last;
};

struct monga_ast_program_t
{
    struct monga_ast_definition_list_t *definitions; /* nullable */
};

/* Program root */

extern struct monga_ast_program_t *root;

/* Constructors */

#define construct(type) \
((struct monga_ast_ ## type ## _t *) monga_malloc(sizeof(struct monga_ast_ ## type ## _t)))

/* Destructors */

void monga_ast_program_destroy(struct monga_ast_program_t* ast);
void monga_ast_definition_destroy(struct monga_ast_definition_t* ast);
void monga_ast_def_variable_destroy(struct monga_ast_def_variable_t* ast);
void monga_ast_def_type_destroy(struct monga_ast_def_type_t* ast);
void monga_ast_def_function_destroy(struct monga_ast_def_function_t* ast);
void monga_ast_typedesc_destroy(struct monga_ast_typedesc_t* ast);
void monga_ast_field_destroy(struct monga_ast_field_t* ast);
void monga_ast_parameter_destroy(struct monga_ast_parameter_t* ast);
void monga_ast_block_destroy(struct monga_ast_block_t* ast);
void monga_ast_statement_destroy(struct monga_ast_statement_t* ast);
void monga_ast_variable_destroy(struct monga_ast_variable_t* ast);
void monga_ast_expression_destroy(struct monga_ast_expression_t* ast);
void monga_ast_condition_destroy(struct monga_ast_condition_t* ast);
void monga_ast_call_destroy(struct monga_ast_call_t* ast);

/* Print */

void monga_ast_program_print(struct monga_ast_program_t* ast);
void monga_ast_definition_print(struct monga_ast_definition_t* ast, int identation);
void monga_ast_def_variable_print(struct monga_ast_def_variable_t* ast, int identation);
void monga_ast_def_type_print(struct monga_ast_def_type_t* ast, int identation);
void monga_ast_def_function_print(struct monga_ast_def_function_t* ast, int identation);
void monga_ast_typedesc_print(struct monga_ast_typedesc_t* ast, int identation);
void monga_ast_field_print(struct monga_ast_field_t* ast, int identation);
void monga_ast_parameter_print(struct monga_ast_parameter_t* ast, int identation);
void monga_ast_block_print(struct monga_ast_block_t* ast, int identation);
void monga_ast_statement_print(struct monga_ast_statement_t* ast, int identation);
void monga_ast_variable_print(struct monga_ast_variable_t* ast, int identation);
void monga_ast_expression_print(struct monga_ast_expression_t* ast, int identation);
void monga_ast_condition_print(struct monga_ast_condition_t* ast, int identation);
void monga_ast_call_print(struct monga_ast_call_t* ast, int identation);

#endif