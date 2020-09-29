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
    | primary_exp '[' exp ']'
    | primary_exp '.' ID
    ;

primary_exp : NUMERAL
            | var
            | call
            | '(' exp ')'
            ;

postfix_exp : primary_exp
            | postfix_exp AS type
            ;

new_exp : postfix_exp
        | NEW type opt_item_access
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
                | additive_exp LE additive_exp
                | additive_exp GE additive_exp
                ;

equality_cond : relational_cond
              | additive_exp EQ additive_exp
              | additive_exp NE additive_exp
              ;

logical_and_cond : equality_cond
                 | logical_and_cond AND equality_cond
                 ;

logical_or_cond : logical_and_cond
                | logical_or_cond OR logical_and_cond
                ;

call : ID '(' opt_exp_list ')'
     ;

opt_exp_list : exp_list
             |
             ;

exp_list : exp_list ',' exp
         | exp
         ;

%%
