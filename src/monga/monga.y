%{
    #include <stdio.h>

    /* Declare symbols from lex that yacc
       needs in order to compile */
    extern int yylex();
    void yyerror(const char* err);
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
    char* id;
    int integer;
    double real;
}

%%
program :

    opt_definition_list
    {
        printf("opt_definition_list -> program\n");
    }

opt_definition_list :

    opt_definition_list definition
    {
        printf("opt_definition_list definition -> opt_definition_list\n");
    }
    | /* empty */
    ;

definition :

    def_variable
    {
        printf("def_variable -> definition\n");
    }
    | def_function
    {
        printf("def_function -> definition\n");
    }
    | def_type
    {
        printf("def_type -> definition\n");
    }

def_variable :

    MONGA_TK_VAR MONGA_TK_ID ':' type ';'
    {
        printf("var \"%s\" : type ; -> def_variable\n", $<id>2);
    }

type :

    MONGA_TK_ID
    {
        printf("\"%s\" -> type\n", $<id>1);
    }

def_type :

    MONGA_TK_TYPE MONGA_TK_ID '=' typedesc ';'
    {
        printf("type \"%s\" = typedesc ; -> def_type\n", $<id>2);
    }

typedesc :

    MONGA_TK_ID
    {
        printf("\"%s\" -> typedesc\n", $<id>1);
    }
    | '[' typedesc ']'
    {
        printf("[ typedesc ] -> typedesc\n");
    }
    | '{' field_list '}'
    {
        printf("{ field_list } -> typedesc\n");
    }

field_list :

    field_list field
    {
        printf("field_list field -> field_list\n");
    }
    | field
    {
        printf("field -> field_list\n");
    }

field :

    MONGA_TK_ID ':' type ';'
    {
        printf("\"%s\" : type ; -> field\n", $<id>1);
    }

def_function :

    MONGA_TK_FUNCTION MONGA_TK_ID '(' opt_parameter_list ')' opt_def_function_type block
    {
        printf("function \"%s\" ( opt_parameter_list ) opt_def_function_type block -> def_function\n", $<id>2);
    }

opt_def_function_type :

    ':' type
    {
        printf(": type -> opt_def_function_type\n");
    }
    | /* empty */
    ;

opt_parameter_list :

    parameter_list
    {
        printf("parameter_list -> opt_parameter_list\n");
    }
    | /* empty */
    ;

parameter_list :

    parameter_list ',' parameter
    {
        printf("parameter_list , parameter -> parameter_list\n");
    }
    | parameter
    {
        printf("parameter -> parameter_list\n");
    }

parameter :

    MONGA_TK_ID ':' type
    {
        printf("\"%s\" : type -> parameter\n", $<id>1);
    }

block :

    '{' opt_def_variable_list opt_statement_list '}'
    {
        printf("opt_def_variable_list opt_statement_list -> block\n");
    }

opt_def_variable_list :

    opt_def_variable_list def_variable
    {
        printf("opt_def_variable_list def_variable -> opt_def_variable_list\n");
    }
    | /* empty */
    ;

opt_statement_list :

    opt_statement_list statement
    {
        printf("opt_statement_list statement -> opt_statement_list\n");
    }
    | /* empty */
    ;

statement :

    MONGA_TK_IF cond block opt_else_block
    {
        printf("if cond block opt_else_block -> statement\n");
    }
    | MONGA_TK_WHILE cond block
    {
        printf("while cond block -> statement\n");
    }
    | var '=' exp ';'
    {
        printf("var = exp ; -> statement\n");
    }
    | MONGA_TK_RETURN opt_exp ';'
    {
        printf("return opt_exp ; -> statement\n");
    }
    | call ';'
    {
        printf("call ; -> statement\n");
    }
    | '@' exp ';'
    {
        printf("@ exp ; -> statement\n");
    }
    | block
    {
        printf("block -> statement\n");
    }

opt_else_block :

    MONGA_TK_ELSE block
    {
        printf("else block -> opt_else_block\n");
    }
    | /* empty */
    ;

opt_exp :

    exp
    {
        printf("exp -> opt_exp\n");
    }
    | /* empty */
    ;

var :

    MONGA_TK_ID
    {
        printf("\"%s\" -> var\n", $<id>1);
    }
    | primary_exp '[' exp ']'
    {
        printf("primary_exp [ exp ] -> var\n");
    }
    | primary_exp '.' MONGA_TK_ID
    {
        printf("primary_exp . \"%s\" -> var\n", $<id>3);
    }

primary_exp :

    MONGA_TK_INTEGER
    {
        printf("%d -> primary_exp\n", $<integer>1);
    }
    | MONGA_TK_REAL
    {
        printf("%g -> primary_exp\n", $<real>1);
    }
    | var
    {
        printf("var -> primary_exp\n");
    }
    | call
    {
        printf("call -> primary_exp\n");
    }
    | '(' exp ')'
    {
        printf("( exp ) -> primary_exp\n");
    }

postfix_exp :

    primary_exp
    {
        printf("primary_exp -> postfix_exp\n");
    }
    | postfix_exp MONGA_TK_AS type
    {
        printf("postfix_exp as type -> postfix_exp\n");
    }

new_exp :

    postfix_exp
    {
        printf("postfix_exp -> new_exp\n");
    }
    | MONGA_TK_NEW type opt_item_access
    {
        printf("new type opt_item_acess -> new_exp\n");
    }

unary_exp :

    new_exp
    {
        printf("new_exp -> unary_exp\n");
    }
    | '-' unary_exp
    {
        printf("- unary_exp -> unary_exp\n");
    }

multiplicative_exp :

    unary_exp
    {
        printf("unary_exp -> multiplicative_exp\n");
    }
    | multiplicative_exp '*' unary_exp
    {
        printf("multiplicative_exp * unary_exp -> multiplicative_exp\n");
    }
    | multiplicative_exp '/' unary_exp
    {
        printf("multiplicative_exp / unary_exp -> multiplicative_exp\n");
    }

additive_exp :

    multiplicative_exp
    {
        printf("multiplicative_exp -> additive_exp\n");
    }
    | additive_exp '+' multiplicative_exp
    {
        printf("additive_exp + multiplicative_exp -> additive_exp\n");
    }
    | additive_exp '-' multiplicative_exp
    {
        printf("additive_exp - multiplicative_exp -> additive_exp\n");
    }

conditional_exp :

    additive_exp
    {
        printf("additive_exp -> conditional_exp\n");
    }
    | equality_cond '?' exp ':' conditional_exp 
    {
        printf("equality_cond ? exp : conditional_exp -> conditional_exp\n");
    }

exp :

    conditional_exp
    {
        printf("conditional_exp -> exp\n");
    }

opt_item_access :

    item_access
    {
        printf("item_access -> opt_item_access\n");
    }
    | /* empty */
    ;

item_access :

    '[' primary_exp ']'
    {
        printf("[ primary_exp ] -> item_access\n");
    }

primary_cond :

    '(' cond ')'
    {
        printf("( cond ) -> primary_cond\n");
    }

negated_cond :

    primary_cond
    {
        printf("primary_cond -> negated_cond\n");
    }
    | '!' negated_cond
    {
        printf("! negated_cond -> negated_cond\n");
    }

relational_cond :

    negated_cond
    {
        printf("negated_cond -> relational_cond\n");
    }
    | additive_exp '<' additive_exp
    {
        printf("additive_exp < additive_exp -> relational_cond\n");
    }
    | additive_exp '>' additive_exp
    {
        printf("additive_exp > additive_exp -> relational_cond\n");
    }
    | additive_exp MONGA_TK_LE additive_exp
    {
        printf("additive_exp <= additive_exp -> relational_cond\n");
    }
    | additive_exp MONGA_TK_GE additive_exp
    {
        printf("additive_exp >= additive_exp -> relational_cond\n");
    }

equality_cond :

    relational_cond
    {
        printf("relational_cond -> equality_cond\n");
    }
    | additive_exp MONGA_TK_EQ additive_exp
    {
        printf("additive_exp == additive_exp -> equality_cond\n");
    }
    | additive_exp MONGA_TK_NE additive_exp
    {
        printf("additive_exp ~= additive_exp -> equality_cond\n");
    }

logical_and_cond :

    equality_cond
    {
        printf("equality_cond -> logical_and_cond\n");
    }
    | logical_and_cond MONGA_TK_AND equality_cond
    {
        printf("logical_and_cond && equality_cond -> logical_and_cond\n");
    }

logical_or_cond :

    logical_and_cond
    {
        printf("logical_and_cond -> logical_or_cond\n");
    }
    | logical_or_cond MONGA_TK_OR logical_and_cond
    {
        printf("logical_or_cond || logical_and_cond -> logical_or_cond\n");
    }

cond :

    logical_or_cond
    {
        printf("logical_or_cond -> cond\n");
    }

call :

    MONGA_TK_ID '(' opt_exp_list ')'
    {
        printf("\"%s\" ( opt_exp_list ) -> call\n", $<id>1);
    }

opt_exp_list :

    exp_list
    {
        printf("exp_list -> opt_exp_list\n");
    }
    | /* empty */
    ;

exp_list :

    exp_list ',' exp
    {
        printf("exp_list , exp -> exp_list\n");
    }
    | exp
    {
        printf("exp -> exp_list\n");
    }

%%