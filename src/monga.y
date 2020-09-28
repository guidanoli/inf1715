%token VAR ID TYPE FUNCTION IF ELSE WHILE RETURN NUMERAL NEW AS EQ NE LE GE AND OR
%%
program : definition_list
        ;

definition_list : definition_list definition
                | definition
                ;

definition : def_variable
           | def_function
           | def_type
           ;

def_variable : VAR ID ':' type ';'
             ;

type : ID
     ;

def_type : TYPE ID '=' typedesc
         ;

typedesc : ID
         | '[' typedesc ']'
         | '{' field_list '}'
         ;

field_list : field_list field
           | field
           ;

field : ID ':' type ';'
      ;

def_function : FUNCTION ID '(' opt_parameter_list ')' opt_def_function_type block
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

parameter : ID ':' type
          ;

block : '{' def_variable_list statement_list '}'
      ;

def_variable_list : def_variable_list def_variable
                  | def_variable
                  ;

statement_list : statement_list statement
               | statement
               ;

statement : IF cond block opt_else_block
          | WHILE cond block
          | var '=' exp ';'
          | RETURN opt_exp ';'
          | call ';'
          | '@' exp ';'
          | block
          ;

opt_else_block : ELSE block
               |
               ;

opt_exp : exp
        |
        ;

var : ID
    | exp '[' exp ']'
    | exp '.' ID
    ;

exp : NUMERAL
    | var
    | '(' exp ')'
    | call
    | exp AS type
    | NEW type item_access_list
    | '-' exp
    | exp '+' exp
    | exp '-' exp
    | exp '*' exp
    | exp '/' exp
    | cond '?' exp ':' exp
    ;

item_access_list : item_access_list item_access
                 | item_access
                 ;

item_access : '[' exp ']'
            ;

cond :  '(' cond ')'
     | exp EQ exp
     | exp NE exp
     | exp LE exp
     | exp GE exp
     | exp '<' exp
     | exp '>' exp
     | '!' cond
     | cond AND cond
     | cond OR cond
     ;

call : ID '(' opt_explist ')'
     ;

opt_explist : explist
            |
            ;

explist : explist ',' exp
        | exp
        ;

%%
