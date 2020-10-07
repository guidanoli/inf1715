#ifndef MONGA_AST_H
#define MONGA_AST_H

/* Incomplete types */
struct monga_ast_def_variable_t;
struct monga_ast_block_t;
struct monga_ast_expression_t;

struct {
    char *function_id;
    struct monga_ast_expression_t *parameters;

} monga_ast_call_t;

struct {
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
        } eq_cond;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } ne_cond;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } le_cond;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } ge_cond;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } lt_cond;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } gt_cond;
        struct {
            struct monga_ast_expression_t *exp;
        } not_cond;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } and_cond;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } or_cond;
    };
} monga_ast_condition_t;

struct {
    enum {
        MONGA_AST_EXPRESSION_INTEGER,
        MONGA_AST_EXPRESSION_REAL,
        MONGA_AST_EXPRESSION_VAR,
        MONGA_AST_EXPRESSION_CALL,
        MONGA_AST_EXPRESSION_CAST,
        MONGA_AST_EXPRESSION_NEW,
        MONGA_AST_EXPRESSION_NEGATION,
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
            struct monga_ast_expression_t *size; /* nullable */
        } new_exp;
        struct {
            struct monga_ast_expression_t *exp;
        } negation_exp;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } addition_exp;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } subtraction_exp;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } multiplication_exp;
        struct {
            struct monga_ast_expression_t *exp1;
            struct monga_ast_expression_t *exp2;
        } division_exp;
        struct {
            struct monga_ast_condition_t *cond;
            struct monga_ast_expression_t *true_exp;
            struct monga_ast_expression_t *false_exp;
        } conditional_exp;
    };
    struct monga_ast_expression_t *next; /* nullable */
} monga_ast_expression_t;

struct {
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
        };
    };
} monga_ast_variable_t;

struct {
    enum {
        MONGA_AST_STATEMENT_IF,
        MONGA_AST_STATEMENT_WHILE,
        MONGA_AST_STATEMENT_ASSIGNMENT,
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
} monga_ast_statement_t;

struct {
    struct monga_ast_def_variable_t *variables;
    struct monga_ast_statement_t *statements;
} monga_ast_block_t;

struct {
    char *id;
    char *type;
    struct monga_ast_parameter_t *next; /* nullable */
} monga_ast_parameter_t;

struct {
    char *id;
    char *type;
    struct monga_ast_field_t *next; /* nullable */
} monga_ast_field_t;

struct {
    enum {
        MONGA_AST_TYPEDESC_ID,
        MONGA_AST_TYPEDESC_ARRAY,
        MONGA_AST_TYPEDESC_RECORD,
    } tag;
    union {
        char* id_typedesc;
        struct monga_ast_typedesc_t* array_typedesc;
        struct monga_ast_field_t* record_typedesc;
    };
} monga_ast_typedesc_t;

struct {
    char *id;
    struct monga_ast_parameter_t *parameters; /* nullable */
    char *type; /* nullable */
    struct monga_ast_block_t *block;
} monga_ast_def_function_t;

struct {
    char *id;
    struct monga_ast_typedesc_t *typedesc;
} monga_ast_def_type_t;

struct {
    char *id;
    char *type;
    struct monga_ast_def_variable_t *next; /* nullable */
} monga_ast_def_variable_t;

struct {
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
} monga_ast_definition_t;

struct {
    struct monga_ast_definition_t *definitions; /* nullable */
} monga_ast_program_t;

#endif