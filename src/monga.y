%{
    #include <stdio.h>

    /* Declare symbols from lex that yacc
       needs in order to compile */
    extern int yylex();
    extern int yyparse();
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
    struct {
        char* str;
        int size;
    } id;
    int integer;
    double real;
}

%%
program :

    definition_list
    {
        printf("definition_list -> program\n");
    }

definition_list :

    definition_list definition
    {
        printf("definition_list definition -> definition_list\n");
    }
    | definition 
    {
        printf("definition -> definition_list\n");
    } 

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
        printf("var \"%.*s\" -> def_variable\n", yylval.id.size, yylval.id.str);
    }

type : MONGA_TK_ID
     ;

def_type : MONGA_TK_TYPE MONGA_TK_ID '=' typedesc ';'
         ;

typedesc : MONGA_TK_ID
         | '[' typedesc ']'
         | '{' field_list '}'
         ;

field_list : field_list field
           | field
           ;

field : MONGA_TK_ID ':' type ';'
      ;

def_function : MONGA_TK_FUNCTION MONGA_TK_ID '(' opt_parameter_list ')' opt_def_function_type block
             ;

opt_def_function_type : ':' type
                      |
                      ;

opt_parameter_list : parameter_list
                   |
                   ;

parameter_list : parameter
               | parameter_list ',' parameter
               ;

parameter : MONGA_TK_ID ':' type
          ;

block : '{' def_variable_list statement_list '}'
      ;

def_variable_list : def_variable_list def_variable
                  | def_variable
                  ;

statement_list : statement_list statement
               | statement
               ;

statement : MONGA_TK_IF cond block opt_else_block
          | MONGA_TK_WHILE cond block
          | var '=' exp ';'
          | MONGA_TK_RETURN opt_exp ';'
          | call ';'
          | '@' exp ';'
          | block
          ;

opt_else_block : MONGA_TK_ELSE block
               |
               ;

opt_exp : exp
        |
        ;

var : MONGA_TK_ID
    | primary_exp '[' exp ']'
    | primary_exp '.' MONGA_TK_ID
    ;

primary_exp : MONGA_TK_INTEGER
            | MONGA_TK_REAL
            | var
            | call
            | '(' exp ')'
            ;

postfix_exp : primary_exp
            | postfix_exp MONGA_TK_AS type
            ;

new_exp : postfix_exp
        | MONGA_TK_NEW type opt_item_access
        ;

unary_exp : new_exp
          | '-' unary_exp
          ;

multiplicative_exp : unary_exp
                   | multiplicative_exp '*' unary_exp
                   | multiplicative_exp '/' unary_exp
                   ;

additive_exp : multiplicative_exp
             | additive_exp '+' multiplicative_exp
             | additive_exp '-' multiplicative_exp
             ;

conditional_exp : additive_exp
                | equality_cond '?' exp ':' conditional_exp 
                ;

exp : conditional_exp
    ;

opt_item_access : item_access
                |
                ;

item_access : '[' primary_exp ']'
            ;

cond :  '(' logical_or_cond ')'
     ;

negated_cond : cond
             | '!' '(' negated_cond ')'
             ;

relational_cond : negated_cond
                | additive_exp '<' additive_exp
                | additive_exp '>' additive_exp
                | additive_exp MONGA_TK_LE additive_exp
                | additive_exp MONGA_TK_GE additive_exp
                ;

equality_cond : relational_cond
              | additive_exp MONGA_TK_EQ additive_exp
              | additive_exp MONGA_TK_NE additive_exp
              ;

logical_and_cond : equality_cond
                 | logical_and_cond MONGA_TK_AND equality_cond
                 ;

logical_or_cond : logical_and_cond
                | logical_or_cond MONGA_TK_OR logical_and_cond
                ;

call : MONGA_TK_ID '(' opt_exp_list ')'
     ;

opt_exp_list : exp_list
             |
             ;

exp_list : exp_list ',' exp
         | exp
         ;

%%

void yyerror(const char* err)
{
    fprintf(stderr, "%s\n", err);
}

int main(int argc, char** argv)
{
    return yyparse();
}
