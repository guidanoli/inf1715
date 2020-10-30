# AST typing

Typing happens in a bottom-up approach. Every expression and variable is typed.

## Predicates

There are some predicates that formalize the process of typing the tree.

### `numeric(t)`

`op t` and `t op t` are valid expressions.

### `castable(te,tc)`

`te as tc` is a valid expression.

### `assignable(tl,tr)`

`tl = tr` is a valid statement.

### `equal(t1,t2)`

`assignable(t1,t2)` and `assignable(t2,t1)`

### `sibling(t1,t2)`

`assignable(t1,t2)` or `assignable(t2,t1)`

### `parent(t1,t2)`

* `t1`, if `assignable(t1,t2)`
* `t2`, if `assignable(t2,t1)`

## Tautologies

* Given `t1` and `t2`, if `sibling(t1,t2)` then exists `tp = parent(t1,t2)`
* Given `t1` and `t2`, if `equal(t1,t2)` then `sibling(t1,t2)`
* Given `t1` and `t2`, if `equal(t1,t2)` then exists `tp = parent(t1,t2)`

## Expression binding

| expression | nodes | rules | type |
| :-: | :-: | :-: | :-: |
| integer_exp | | | `integer` |
| real_exp | | | `float` |
| null_exp | | | `null` |
| var_exp | `variable` | | `type(variable)` |
| call_exp | `call` | | `type(call.f)` |
| cast_exp | `exp`, `t` | `castable(type(exp),t)` | `t` |
| new_exp | `t` | `!numeric(t)` | `t` |
| new_exp | `t`, `exp` | `type(exp) == integer` | `array(t)` |
| negative_exp | `exp` | `numeric(type(exp))` | `type(exp)` |
| binop_exp | `exp1`, `exp2` | `numeric(type(exp1))` and `numeric(type(exp2))` and `equal(type(exp1),type(exp2))` | `type(exp1)` |
| conditional_exp | `cond`, `expt`, `expf` | `assignable(type(expt),type(expf))` or `assignable(type(expf),type(expt))` | `base(type(expt),type(expf))` |

## Condition binding

| condition | nodes | rules |
| :-: | :-: | :-: |
| exp_binop_cond | `exp1`, `exp2` | `sibling(exp1,exp2)` |
| cond_binop_cond | `cond1`, `cond2` | |
| cond_unop_cond | `cond` | |

## Variable binding

| variable | nodes | rules | type |
| :-: | :-: | :-: | :-: |
| id_var | `var_ref` | | `type(var_ref.u.def_variable)` |
| array_var | `array_exp`, `index_exp` | `type(index_exp) = integer` and `type(array_exp) = array(subtype)` | `subtype` |
| record_var | `record_exp`, `field_ref` | `type(record_exp) = record(field_list)` and `field_ref.id ∈ field_list` | `type(field_ref.u.field)`