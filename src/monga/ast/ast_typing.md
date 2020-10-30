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

### `base(t1,t2)`

* `t1`, if `assignable(t1,t2)`
* `t2`, if `assignable(t2,t1)`


## Expression typing

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