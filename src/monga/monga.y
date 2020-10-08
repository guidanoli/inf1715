%{
    #include <stdio.h>
    #include <stdlib.h>

    #include "monga_ast.h"

    /* Declare symbols from lex that yacc
       needs in order to compile */
    extern int yylex();
    void yyerror(const char* err);
    struct monga_ast_program_t* root = NULL;
%}

%token MONGA_TK_ID
%token MONGA_TK_INTEGER
%token MONGA_TK_REAL
%token MONGA_TK_AS
%token MONGA_TK_ELSE
%token MONGA_TK_FUNCTION
%token MONGA_TK_IF
%token MONGA_TK_NEW
%token MONGA_TK_RETURN
%token MONGA_TK_VAR
%token MONGA_TK_WHILE
%token MONGA_TK_EQ
%token MONGA_TK_NE
%token MONGA_TK_LE
%token MONGA_TK_GE
%token MONGA_TK_AND
%token MONGA_TK_OR
%token MONGA_TK_TYPE

%union {
    /* Terminals */
    char *id;
    int integer;
    double real;
    /* AST nodes */
    struct monga_ast_call_t *call;
    struct monga_ast_condition_t *condition;
    struct monga_ast_expression_t *expression;
    struct monga_ast_variable_t *variable;
    struct monga_ast_statement_t *statement;
    struct monga_ast_block_t *block;
    struct monga_ast_parameter_t *parameter;
    struct monga_ast_field_t *field;
    struct monga_ast_typedesc_t *typedesc;
    struct monga_ast_def_function_t *def_function;
    struct monga_ast_def_type_t *def_type;
    struct monga_ast_def_variable_t *def_variable;
    struct monga_ast_definition_t *definition;
    struct monga_ast_program_t *program;
}

%type <program> program
%type <definition> definition definition_list opt_definition_list
%type <def_variable> def_variable def_variable_list opt_def_variable_list
%type <id> type opt_def_function_type
%type <def_type> def_type
%type <typedesc> typedesc
%type <field> field field_list
%type <def_function> def_function
%type <parameter> parameter parameter_list opt_parameter_list
%type <block> block opt_else_block
%type <statement> statement statement_list opt_statement_list
%type <expression> exp opt_exp primary_exp postfix_exp new_exp unary_exp multiplicative_exp additive_exp conditional_exp opt_item_access item_access opt_exp_list exp_list
%type <variable> var
%type <condition> cond primary_cond negated_cond relational_cond equality_cond logical_and_cond logical_or_cond
%type <call> call

%%
program :

    opt_definition_list
    {
        $$ = construct(program);
        $$->definitions = $1;
        root = $$; /* saves program root */
    }

opt_definition_list :

    definition_list
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = NULL;
    }

definition_list:

    definition definition_list
    {
        $$ = $1;
        $$->next = $2;
    }
    | definition
    {
        $$ = $1;
    }

definition :

    def_variable
    {
        $$ = construct(definition);
        $$->tag = MONGA_AST_DEFINITION_VARIABLE;
        $$->def_variable = $1;
        $$->next = NULL;
    }
    | def_function
    {
        $$ = construct(definition);
        $$->tag = MONGA_AST_DEFINITION_FUNCTION;
        $$->def_function = $1;
        $$->next = NULL;
    }
    | def_type
    {
        $$ = construct(definition);
        $$->tag = MONGA_AST_DEFINITION_TYPE;
        $$->def_type = $1;
        $$->next = NULL;
    }

def_variable :

    MONGA_TK_VAR MONGA_TK_ID ':' type ';'
    {
        $$ = construct(def_variable);
        $$->id = $<id>2;
        $$->type = $4;
        $$->next = NULL;
    }

type :

    MONGA_TK_ID
    {
        $$ = $<id>1;
    }

def_type :

    MONGA_TK_TYPE MONGA_TK_ID '=' typedesc ';'
    {
        $$ = construct(def_type);
        $$->id = $<id>2;
        $$->typedesc = $4;
    }

typedesc :

    MONGA_TK_ID
    {
        $$ = construct(typedesc);
        $$->tag = MONGA_AST_TYPEDESC_ID;
        $$->id_typedesc = $<id>1;
    }
    | '[' typedesc ']'
    {
        $$ = construct(typedesc);
        $$->tag = MONGA_AST_TYPEDESC_ARRAY;
        $$->array_typedesc = $2;
    }
    | '{' field_list '}'
    {
        $$ = construct(typedesc);
        $$->tag = MONGA_AST_TYPEDESC_RECORD;
        $$->record_typedesc = $2;
    }

field_list :

    field field_list
    {
        $$ = $1;
        $$->next = $2;
    }
    | field
    {
        $$ = $1;
    }

field :

    MONGA_TK_ID ':' type ';'
    {
        $$ = construct(field);
        $$->id = $<id>1;
        $$->type = $3;
        $$->next = NULL;
    }

def_function :

    MONGA_TK_FUNCTION MONGA_TK_ID '(' opt_parameter_list ')' opt_def_function_type block
    {
        $$ = construct(def_function);
        $$->id = $<id>2;
        $$->parameters = $4;
        $$->type = $6;
        $$->block = $7;
    }

opt_def_function_type :

    ':' type
    {
        $$ = $2;
    }
    | /* empty */
    {
        $$ = NULL;
    }

opt_parameter_list :

    parameter_list
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = NULL;
    }

parameter_list :

    parameter ',' parameter_list
    {
        $$ = $1;
        $$->next = $3;
    }
    | parameter
    {
        $$ = $1;
    }

parameter :

    MONGA_TK_ID ':' type
    {
        $$ = construct(parameter);
        $$->id = $<id>1;
        $$->type = $3;
    }

block :

    '{' opt_def_variable_list opt_statement_list '}'
    {
        $$ = construct(block);
        $$->variables = $2;
        $$->statements = $3;
    }

opt_def_variable_list :

    def_variable_list
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = NULL;
    }

def_variable_list :

    def_variable def_variable_list
    {
        $$ = $1;
        $$->next = $2;
    }
    | def_variable
    {
        $$ = $1;
    }

opt_statement_list :

    statement_list
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = NULL;
    }

statement_list :

    statement statement_list
    {
        $$ = $1;
        $$->next = $2;
    }
    | statement
    {
        $$ = $1;
    }

statement :

    MONGA_TK_IF cond block opt_else_block
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_IF;
        $$->if_stmt.cond = $2;
        $$->if_stmt.then_block = $3;
        $$->if_stmt.else_block = $4;
        $$->next = NULL;
    }
    | MONGA_TK_WHILE cond block
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_WHILE;
        $$->while_stmt.cond = $2;
        $$->while_stmt.loop = $3;
        $$->next = NULL;
    }
    | var '=' exp ';'
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_ASSIGN;
        $$->assign_stmt.var = $1;
        $$->assign_stmt.exp = $3;
        $$->next = NULL;
    }
    | MONGA_TK_RETURN opt_exp ';'
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_RETURN;
        $$->return_stmt.exp = $2;
        $$->next = NULL;
    }
    | call ';'
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_CALL;
        $$->call_stmt.call = $1;
        $$->next = NULL;
    }
    | '@' exp ';'
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_PRINT;
        $$->print_stmt.exp = $2;
        $$->next = NULL;
    }
    | block
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_BLOCK;
        $$->block_stmt.block = $1;
        $$->next = NULL;
    }

opt_else_block :

    MONGA_TK_ELSE block
    {
        $$ = $2;
    }
    | /* empty */
    {
        $$ = NULL;
    }

opt_exp :

    exp
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = NULL;
    }

var :

    MONGA_TK_ID
    {
        $$ = construct(variable);
        $$->tag = MONGA_AST_VARIABLE_ID;
        $$->id_var.id = $<id>1;
    }
    | primary_exp '[' exp ']'
    {
        $$ = construct(variable);
        $$->tag = MONGA_AST_VARIABLE_ARRAY;
        $$->array_var.array = $1;
        $$->array_var.index = $3;
    }
    | primary_exp '.' MONGA_TK_ID
    {
        $$ = construct(variable);
        $$->tag = MONGA_AST_VARIABLE_RECORD;
        $$->record_var.record = $1;
        $$->record_var.field = $<id>3;
    }

primary_exp :

    MONGA_TK_INTEGER
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_INTEGER;
        $$->integer_exp.integer = $<integer>1;
        $$->next = NULL;
    }
    | MONGA_TK_REAL
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_REAL;
        $$->real_exp.real = $<real>1;
        $$->next = NULL;
    }
    | var
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_VAR;
        $$->var_exp.var = $1;
        $$->next = NULL;
    }
    | call
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_CALL;
        $$->call_exp.call = $1;
        $$->next = NULL;
    }
    | '(' exp ')'
    {
        $$ = $2;
    }

postfix_exp :

    primary_exp
    {
        $$ = $1;
    }
    | postfix_exp MONGA_TK_AS type
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_CAST;
        $$->cast_exp.exp = $1;
        $$->cast_exp.type = $3;
        $$->next = NULL;
    }

new_exp :

    postfix_exp
    {
        $$ = $1;
    }
    | MONGA_TK_NEW type opt_item_access
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_NEW;
        $$->new_exp.type = $2;
        $$->new_exp.exp = $3;
        $$->next = NULL;
    }

unary_exp :

    new_exp
    {
        $$ = $1;
    }
    | '-' unary_exp
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_NEGATION;
        $$->negation_exp.exp = $2;
        $$->next = NULL;
    }

multiplicative_exp :

    unary_exp
    {
        $$ = $1;
    }
    | multiplicative_exp '*' unary_exp
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_MULTIPLICATION;
        $$->multiplication_exp.exp1 = $1;
        $$->multiplication_exp.exp2 = $3;
        $$->next = NULL;
    }
    | multiplicative_exp '/' unary_exp
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_DIVISION;
        $$->division_exp.exp1 = $1;
        $$->division_exp.exp2 = $3;
        $$->next = NULL;
    }

additive_exp :

    multiplicative_exp
    {
        $$ = $1;
    }
    | additive_exp '+' multiplicative_exp
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_ADDITION;
        $$->addition_exp.exp1 = $1;
        $$->addition_exp.exp2 = $3;
        $$->next = NULL;
    }
    | additive_exp '-' multiplicative_exp
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_SUBTRACTION;
        $$->subtraction_exp.exp1 = $1;
        $$->subtraction_exp.exp2 = $3;
        $$->next = NULL;
    }

conditional_exp :

    additive_exp
    {
        $$ = $1;
    }
    | equality_cond '?' exp ':' conditional_exp 
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_CONDITIONAL;
        $$->conditional_exp.cond = $1;
        $$->conditional_exp.true_exp = $3;
        $$->conditional_exp.false_exp = $5;
        $$->next = NULL;
    }

exp :

    conditional_exp
    {
        $$ = $1;
    }

opt_item_access :

    item_access
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = NULL;
    }

item_access :

    '[' primary_exp ']'
    {
        $$ = $2;
    }

primary_cond :

    '(' cond ')'
    {
        $$ = $2;
    }

negated_cond :

    primary_cond
    {
        $$ = $1;
    }
    | '!' negated_cond
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_NOT;
        $$->not_cond.cond = $2;
    }

relational_cond :

    negated_cond
    {
        $$ = $1;
    }
    | additive_exp '<' additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_LT;
        $$->lt_cond.exp1 = $1;
        $$->lt_cond.exp2 = $3;
    }
    | additive_exp '>' additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_GT;
        $$->gt_cond.exp1 = $1;
        $$->gt_cond.exp2 = $3;
    }
    | additive_exp MONGA_TK_LE additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_LE;
        $$->le_cond.exp1 = $1;
        $$->le_cond.exp2 = $3;
    }
    | additive_exp MONGA_TK_GE additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_GE;
        $$->ge_cond.exp1 = $1;
        $$->ge_cond.exp2 = $3;
    }

equality_cond :

    relational_cond
    {
        $$ = $1;
    }
    | additive_exp MONGA_TK_EQ additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_EQ;
        $$->eq_cond.exp1 = $1;
        $$->eq_cond.exp2 = $3;
    }
    | additive_exp MONGA_TK_NE additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_NE;
        $$->ne_cond.exp1 = $1;
        $$->ne_cond.exp2 = $3;
    }

logical_and_cond :

    equality_cond
    {
        $$ = $1;
    }
    | logical_and_cond MONGA_TK_AND equality_cond
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_AND;
        $$->and_cond.cond1 = $1;
        $$->and_cond.cond2 = $3;
    }

logical_or_cond :

    logical_and_cond
    {
        $$ = $1;
    }
    | logical_or_cond MONGA_TK_OR logical_and_cond
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_OR;
        $$->or_cond.cond1 = $1;
        $$->or_cond.cond2 = $3;
    }

cond :

    logical_or_cond
    {
        $$ = $1;
    }

call :

    MONGA_TK_ID '(' opt_exp_list ')'
    {
        $$ = construct(call);
        $$->function_id = $<id>1;
        $$->expressions = $3;
    }

opt_exp_list :

    exp_list
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = NULL;
    }

exp_list :

    exp ',' exp_list
    {
        $$ = $1;
        $$->next = $3;
    }
    | exp
    {
        $$ = $1;
    }

%%