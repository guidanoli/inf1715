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
    struct {
        union {
            char *id;
            int integer;
            double real;
        };
        size_t line;
    } terminal;
    /* AST nodes */
    struct monga_ast_call_t *call;
    struct monga_ast_condition_t *condition;
    struct monga_ast_expression_t *expression;
    struct monga_ast_expression_list_t *expression_list;
    struct monga_ast_variable_t *variable;
    struct monga_ast_statement_t *statement;
    struct monga_ast_statement_list_t *statement_list;
    struct monga_ast_block_t *block;
    struct monga_ast_parameter_list_t *parameter_list;
    struct monga_ast_field_t *field;
    struct monga_ast_field_list_t *field_list;
    struct monga_ast_typedesc_t *typedesc;
    struct monga_ast_def_function_t *def_function;
    struct monga_ast_def_type_t *def_type;
    struct monga_ast_def_variable_t *def_variable;
    struct monga_ast_def_variable_list_t *def_variable_list;
    struct monga_ast_definition_t *definition;
    struct monga_ast_definition_list_t *definition_list;
    struct monga_ast_program_t *program;
}

%type <program> program
%type <definition_list> definition_list opt_definition_list
%type <definition> definition
%type <def_variable_list> def_variable_list opt_def_variable_list parameter_list opt_parameter_list
%type <def_variable> def_variable parameter
%type <terminal> type opt_def_function_type
%type <def_type> def_type
%type <typedesc> typedesc
%type <field_list> field_list
%type <field> field
%type <def_function> def_function
%type <block> block opt_else_block
%type <statement_list> statement_list opt_statement_list
%type <statement> statement
%type <expression_list> opt_exp_list exp_list
%type <expression> exp opt_exp primary_exp postfix_exp new_exp unary_exp multiplicative_exp additive_exp conditional_exp opt_item_access item_access
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

    definition_list definition
    {
        $$ = $1;
        $$->last->next = $2;
        $$->last = $2;
    }
    | definition
    {
        $$ = construct(definition_list);
        $$->first = $$->last = $1;
    }

definition :

    def_variable
    {
        $$ = construct(definition);
        $$->tag = MONGA_AST_DEFINITION_VARIABLE;
        $$->u.def_variable = $1;
        $$->next = NULL;
    }
    | def_function
    {
        $$ = construct(definition);
        $$->tag = MONGA_AST_DEFINITION_FUNCTION;
        $$->u.def_function = $1;
        $$->next = NULL;
    }
    | def_type
    {
        $$ = construct(definition);
        $$->tag = MONGA_AST_DEFINITION_TYPE;
        $$->u.def_type = $1;
        $$->next = NULL;
    }

def_variable :

    MONGA_TK_VAR MONGA_TK_ID ':' type ';'
    {
        $$ = construct(def_variable);
        $$->id = $<terminal.id>2;
        $$->type.id = $4.id;
        $$->line = $<terminal.line>2;
        $$->next = NULL;
    }

type :

    MONGA_TK_ID
    {
        $$.id = $<terminal.id>1;
        $$.line = $<terminal.line>1;
    }

def_type :

    MONGA_TK_TYPE MONGA_TK_ID '=' typedesc ';'
    {
        $$ = construct(def_type);
        $$->id = $<terminal.id>2;
        $$->typedesc = $4;
        $$->line = $<terminal.line>2;
    }

typedesc :

    MONGA_TK_ID
    {
        $$ = construct(typedesc);
        $$->tag = MONGA_AST_TYPEDESC_ID;
        $$->u.id_typedesc.id = $<terminal.id>1;
        $$->line = $<terminal.line>1;
    }
    | '[' typedesc ']'
    {
        $$ = construct(typedesc);
        $$->tag = MONGA_AST_TYPEDESC_ARRAY;
        $$->u.array_typedesc = $2;
        $$->line = $<terminal.line>1;
    }
    | '{' field_list '}'
    {
        $$ = construct(typedesc);
        $$->tag = MONGA_AST_TYPEDESC_RECORD;
        $$->u.record_typedesc.field_list = $2;
        $$->line = $<terminal.line>1;
    }

field_list :

    field_list field
    {
        $$ = $1;
        $$->last->next = $2;
        $$->last = $2;
    }
    | field
    {
        $$ = construct(field_list);
        $$->first = $$->last = $1;
    }

field :

    MONGA_TK_ID ':' type ';'
    {
        $$ = construct(field);
        $$->id = $<terminal.id>1;
        $$->type.id = $3.id;
        $$->line = $<terminal.line>1;
        $$->next = NULL;
    }

def_function :

    MONGA_TK_FUNCTION MONGA_TK_ID '(' opt_parameter_list ')' opt_def_function_type block
    {
        $$ = construct(def_function);
        $$->id = $<terminal.id>2;
        $$->parameters = $4;
        $$->type.id = $6.id;
        $$->block = $7;
        $$->line = $<terminal.line>2;
    }

opt_def_function_type :

    ':' type
    {
        $$.id = $2.id;
        $$.line = $2.line;
    }
    | /* empty */
    {
        $$.id = NULL;
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

    parameter_list ',' parameter
    {
        $$ = $1;
        $$->last->next = $3;
        $$->last = $3;
    }
    | parameter
    {
        $$ = construct(def_variable_list);
        $$->first = $$->last = $1;
    }

parameter :

    MONGA_TK_ID ':' type
    {
        $$ = construct(def_variable);
        $$->id = $<terminal.id>1;
        $$->type.id = $3.id;
        $$->line = $<terminal.line>1;
        $$->next = NULL;
    }

block :

    '{' opt_def_variable_list opt_statement_list '}'
    {
        $$ = construct(block);
        $$->variables = $2;
        $$->statements = $3;
        $$->line = $<terminal.line>1;
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

    def_variable_list def_variable
    {
        $$ = $1;
        $$->last->next = $2;
        $$->last = $2;
    }
    | def_variable
    {
        $$ = construct(def_variable_list);
        $$->first = $$->last = $1;
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

    statement_list statement
    {
        $$ = $1;
        $$->last->next = $2;
        $$->last = $2;
    }
    | statement
    {
        $$ = construct(statement_list);
        $$->first = $$->last = $1;
    }

statement :

    MONGA_TK_IF cond block opt_else_block
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_IF;
        $$->u.if_stmt.cond = $2;
        $$->u.if_stmt.then_block = $3;
        $$->u.if_stmt.else_block = $4;
        $$->line = $2->line;
        $$->next = NULL;
    }
    | MONGA_TK_WHILE cond block
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_WHILE;
        $$->u.while_stmt.cond = $2;
        $$->u.while_stmt.loop = $3;
        $$->line = $2->line;
        $$->next = NULL;
    }
    | var '=' exp ';'
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_ASSIGN;
        $$->u.assign_stmt.var = $1;
        $$->u.assign_stmt.exp = $3;
        $$->line = $1->line;
        $$->next = NULL;
    }
    | MONGA_TK_RETURN opt_exp ';'
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_RETURN;
        $$->u.return_stmt.exp = $2;
        $$->line = $<terminal.line>3;
        $$->next = NULL;
    }
    | call ';'
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_CALL;
        $$->u.call_stmt.call = $1;
        $$->line = $1->line;
        $$->next = NULL;
    }
    | '@' exp ';'
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_PRINT;
        $$->u.print_stmt.exp = $2;
        $$->line = $<terminal.line>1;
        $$->next = NULL;
    }
    | block
    {
        $$ = construct(statement);
        $$->tag = MONGA_AST_STATEMENT_BLOCK;
        $$->u.block_stmt.block = $1;
        $$->line = $1->line;
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
        $$->u.id_var.id = $<terminal.id>1;
        $$->line = $<terminal.line>1;
    }
    | primary_exp '[' exp ']'
    {
        $$ = construct(variable);
        $$->tag = MONGA_AST_VARIABLE_ARRAY;
        $$->u.array_var.array = $1;
        $$->u.array_var.index = $3;
        $$->line = $1->line;
    }
    | primary_exp '.' MONGA_TK_ID
    {
        $$ = construct(variable);
        $$->tag = MONGA_AST_VARIABLE_RECORD;
        $$->u.record_var.record = $1;
        $$->u.record_var.field.id = $<terminal.id>3;
        $$->line = $1->line;
    }

primary_exp :

    MONGA_TK_INTEGER
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_INTEGER;
        $$->u.integer_exp.integer = $<terminal.integer>1;
        $$->line = $<terminal.line>1;
        $$->next = NULL;
    }
    | MONGA_TK_REAL
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_REAL;
        $$->u.real_exp.real = $<terminal.real>1;
        $$->line = $<terminal.line>1;
        $$->next = NULL;
    }
    | var
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_VAR;
        $$->u.var_exp.var = $1;
        $$->line = $1->line;
        $$->next = NULL;
    }
    | call
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_CALL;
        $$->u.call_exp.call = $1;
        $$->line = $1->line;
        $$->next = NULL;
    }
    | '(' exp ')'
    {
        $$ = $2;
        $$->line = $<terminal.line>1;
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
        $$->u.cast_exp.exp = $1;
        $$->u.cast_exp.type.id = $3.id;
        $$->line = $1->line;
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
        $$->u.new_exp.type.id = $2.id;
        $$->u.new_exp.exp = $3;
        $$->line = $2.line;
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
        $$->tag = MONGA_AST_EXPRESSION_NEGATIVE;
        $$->u.negative_exp.exp = $2;
        $$->line = $<terminal.line>1;
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
        $$->u.binop_exp.exp1 = $1;
        $$->u.binop_exp.exp2 = $3;
        $$->line = $1->line;
        $$->next = NULL;
    }
    | multiplicative_exp '/' unary_exp
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_DIVISION;
        $$->u.binop_exp.exp1 = $1;
        $$->u.binop_exp.exp2 = $3;
        $$->line = $1->line;
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
        $$->u.binop_exp.exp1 = $1;
        $$->u.binop_exp.exp2 = $3;
        $$->line = $1->line;
        $$->next = NULL;
    }
    | additive_exp '-' multiplicative_exp
    {
        $$ = construct(expression);
        $$->tag = MONGA_AST_EXPRESSION_SUBTRACTION;
        $$->u.binop_exp.exp1 = $1;
        $$->u.binop_exp.exp2 = $3;
        $$->line = $1->line;
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
        $$->u.conditional_exp.cond = $1;
        $$->u.conditional_exp.true_exp = $3;
        $$->u.conditional_exp.false_exp = $5;
        $$->line = $1->line;
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
        $$->line = $<terminal.line>1;
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
        $$->u.cond_unop_cond.cond = $2;
        $$->line = $<terminal.line>1;
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
        $$->u.exp_binop_cond.exp1 = $1;
        $$->u.exp_binop_cond.exp2 = $3;
        $$->line = $1->line;
    }
    | additive_exp '>' additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_GT;
        $$->u.exp_binop_cond.exp1 = $1;
        $$->u.exp_binop_cond.exp2 = $3;
        $$->line = $1->line;
    }
    | additive_exp MONGA_TK_LE additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_LE;
        $$->u.exp_binop_cond.exp1 = $1;
        $$->u.exp_binop_cond.exp2 = $3;
        $$->line = $1->line;
    }
    | additive_exp MONGA_TK_GE additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_GE;
        $$->u.exp_binop_cond.exp1 = $1;
        $$->u.exp_binop_cond.exp2 = $3;
        $$->line = $1->line;
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
        $$->u.exp_binop_cond.exp1 = $1;
        $$->u.exp_binop_cond.exp2 = $3;
        $$->line = $1->line;
    }
    | additive_exp MONGA_TK_NE additive_exp
    {
        $$ = construct(condition);
        $$->tag = MONGA_AST_CONDITION_NE;
        $$->u.exp_binop_cond.exp1 = $1;
        $$->u.exp_binop_cond.exp2 = $3;
        $$->line = $1->line;
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
        $$->u.cond_binop_cond.cond1 = $1;
        $$->u.cond_binop_cond.cond2 = $3;
        $$->line = $1->line;
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
        $$->u.cond_binop_cond.cond1 = $1;
        $$->u.cond_binop_cond.cond2 = $3;
        $$->line = $1->line;
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
        $$->function.id = $<terminal.id>1;
        $$->expressions = $3;
        $$->line = $<terminal.line>1;
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

    exp_list ',' exp
    {
        $$ = $1;
        $$->last->next = $3;
        $$->last = $3;
    }
    | exp
    {
        $$ = construct(expression_list);
        $$->first = $$->last = $1;
    }

%%