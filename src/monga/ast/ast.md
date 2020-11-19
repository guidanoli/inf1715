# Abstract Syntax Tree

```
program = (definition*)

definition : def_variable
           | def_function
           | def_type

def_variable = (id: string, type: string)

def_type = (id: string, typedesc)

typedesc : id_typedesc
         | array_typedesc
         | record_typedesc

id_typedesc = (id: string)

array_typedesc = (typedesc)

record_typedesc = (field+)

field = (id: string, type: string)

def_function = (id: string, parameter*, type?: string, block)

parameter = (id: string, type: string)

block = (def_variable*, statement*)

statement : if_statement
          | while_statement
          | assign_statement
          | return_statement
          | call_statement
          | print_statement
          | block_statement

if_statement = (cond, block, block?)

while_statement = (cond, block)

assign_statement = (var, exp)

return_statement = (exp?)

call_statement = (call)

print_statement = (exp)

block_statement = (block)

var : id_var | array_var | record_var

id_var = (id: string)

array_var = (exp, exp)

record_var = (exp, id: string)

exp : integer_exp
    | real_exp
    | var_exp
    | call_exp
    | cast_exp
    | new_exp
    | negative_exp
    | addition_exp
    | subtraction_exp
    | multiplication_exp
    | division_exp
    | conditional_exp

integer_exp = (val: integer)

real_exp = (val: real)

var_exp = (var)

call_exp = (call)

cast_exp = (exp, type: string)

new_exp = (type: string, exp?)

negative_exp = (exp)

addition_exp = (exp, exp)

subtraction_exp = (exp, exp)

multiplication_exp = (exp, exp)

division_exp = (exp, exp)

conditional_exp = (cond, exp, exp)

cond : equal_cond
     | not_equal_cond
     | less_or_eq_cond
     | greater_or_eq_cond
     | less_than_cond
     | greater_than_cond
     | not_cond
     | and_cond
     | or_cond

equal_cond = (exp, exp)

not_equal_cond = (exp, exp)

less_or_eq_cond = (exp, exp)

greater_or_eq_cond = (exp, exp)

less_than_cond = (exp, exp)

greater_than_cond = (exp, exp)

not_cond = (exp)

and_cond = (exp, exp)

or_cond = (exp, exp)

call = (id: string, exp*)
```

## Legend

```
x = (a: y, b: z) - x is defined as a of type y and b of type z

x : a | b - x is defined as a or b

x = (a*, c : y) - x is defined as a list of a and c of type y

x = (a+, c : y) - x is defined as a non-empty list of a and c of type y

x = (a?, d : w) - x is defined as an optional a and d of type w
```