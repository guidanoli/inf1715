# AST binding

The following non-terminal symbols contains a field of type string.
Since Monga does not (yet) have string literals, that most definitely means that it holds an identifier.

## Notation

An id can either reference or be referenced.

An id definition is prefixed with a `* -->`.

An id reference is prefixed with a `{referenced-id} <--`.

## Non-terminal symbols with `string` field

```
def_variable = ( * --> id: string, {def_type.id} <-- type: string)
def_type = ( * --> id: string, typedesc)
id_typedesc = ( {def_type.id} <-- id: string)
field = ( * --> id: string, {def_type.id} <-- type: string)
def_function = ( * --> id: string, parameter*, {def_type.id} <-- type?: string, block)
parameter = ( * --> id: string, {def_type.id} <-- type: string)
id_var = ( {def_variable.id|parameter.id} <-- id: string) [1]
record_var = (exp, {field.id} <-- id: string)
cast_exp = (exp, {def_type.id} <-- type: string)
new_exp = ( {def_type.id} <-- type: string, exp?)
call = ( {def_function.id} <-- id: string, exp*)

[1] Note: functions aren't "first-class citizens" in Monga
```

## Implementation

An id definition is already implemented in the AST nodes as a struct field. Example:

```c
struct monga_ast_foo1_t
{
    char* id; /* This id can be later referenced by other nodes */
    char* type; /* Will reference another node which defines this type */
};
```

But id references aren't. How to go about doing that? Well, the basic idea is adding a new field to the structs of nodes that reference an id, which points to its definition.

This linking could be typed (that is, knowing that it references to, say, a type, and not a function). But making it generic makes dealing with references that could point to different definition types (id_var, for example) easier.

Following are some diagrams showing some options.

```
OPTION #1: Knowing which type of definition is being referenced from the referencing node beforehand.

                             +---------------+
      +--------------------->+ def_function  |
      |                      +---------------+
      |
+-----+--+
|  call  |                   +---------------+
+--------+ +---------------->+ def_variable  |
           |                 +---------------+
           |
     +----------+             +---------+
     |  id_var  +------------>+parameter|
     +----------+             +---------+

```

```
OPTION #2: Allocate a new structure that holds a tag identifying the kind of definition and a pointer (typical polymorfical C).

                             DEFINITION_STRUCT
                     +------------------------------------+
                     |  +-----------------------------+   |
                     |  |ID_DEFINITION_KIND_TAG       |   |
                     |  +-----------------------------+   |
                     |  +-----------------------------+   |
+-------------+      |  |                             |   |
|def_variable +------>  |  Unknown data               |   |
+-------------+      |  |                             |   |
                     |  |  (Please read the tag       |   |
                     |  |   to find more about        |   |
                     |  |   the contents of this      |   |
                     |  |   union)                    |   |
                     |  |                             |   |
                     |  |                             |   |
                     |  |                             |   |
                     |  |                             |   |
                     |  +-----------------------------+   |
                     +------------------------------------+

```

Since it seems far more attractive to compose an uniform interface for the concept of "definition" than to have to manage different types of definitions without any type of hygiene... we'll be opting for the second option. This might ease the maintance work later on if new types of definitions are added to the language.

Another advantage of using an uniform interface for "definition" is that when dealing with data structures used in binding, having a header which contains a tag that tells which kind of definition is being pointed to, is quite handy if not inevitable.

An elegant way to group id and reference is in a new sub-struct which contains a parsed identifier and a reference to the definition.

```c
struct monga_ast_foo2_t
{
    char* id; /* Filled by the parser */
    struct {
        char *id; /* Filled by the parser */
        struct monga_ast_reference_t *ref; /* Filled by the 'binder' */
    } type;
};
```

A premise that can be taken into account during the implementation is considering `ref` undetermined if `id` is `NULL`. That's due to the fact that the absence of an identifier implies in the absence of a reference. The other way around isn't always true, since that in an (invalid) program an identifier can be referenced but not declared.

Now, this would create a huge problem for us involved in cleaning the AST structure. Since many nodes can reference the same definition, this means that the ownership of this reference `struct` is shared... One easy fix is to make `struct monga_ast_reference_t` a static field. Now this `struct` can have an `id` too since every reference involves an identifier in the first place and we same something like so...

```c
struct monga_ast_reference_t
{
    enum { ... } tag;
    union { ... }; /* Pointers to all possible types of reference,
                      filled by the binder */
    char *id; /* Filled by the parser */
};

struct monga_ast_foo3_t
{
    char* id; /* Filled by the parser */
    struct monga_ast_reference_t type;
};
```

It has become quite apparent that it is not possible to obtain the type of some nodes simply. Records can be nested, with many repeated identifiers. These identifiers can't be simply pushed to the binding stack because they will be poped out after the type definition. This intricate structure of records needs to be accessed via the type, which is accessible by the stack. By accident, I started listing how could I extract the types of different kinds of expressions.

| expression | type |
| :-: | :-: |
| integer_exp | *built-in* int |
| real_exp | *built-in* float |
| var_exp | type(.var) |
| call_exp | type(.call) [2] |
| cast_exp | .type |
| new_exp | .type / array of .type [3] |
| negative_exp | type(.exp) |
| binop_exp | type(.exp1) and type(.exp2) [4] |
| conditional_exp | type(.true_exp) and type(.false_exp) [5] |

Some annotations:

2. Even though functions are able to not return any value, since expressions always evaluate, this value must have a type. So you cannot do something like:

    ```
    function f() {}
    function g() { var x : int; x = f(); }
    ```

    Since `f` does not return a value, `f()` cannot be assigned to `x`.

3. `.type [.exp]` has type `[.type]`.

4. If an operation is being executed on expressions of different types, it's not clear to the user which type the result will have. That's why they should be cast.

5. It cannot always be known at compile time the value of the condition, so the type of both options of a condition must be the same.

Going back to the original problem, let's see the following example:

```
a.b.c = 0;
```

This constitutes the following AST:

```
(statement =
        (variable record field="c"
                (expression variable
                        (variable record field="b"
                                (expression variable
                                        (variable "a")
                                )
                        )
                )
        )
        (expression integer "0")
)
```

In the outer-most scope, it's only known that `c` is a field of an expression (`a.b`). This expression could be of a type other than a record, for example, an integer. In order to know whether to accept this statement or not, it is needed to be known the type of this expression.

By inspecting the kind of expression (`a.b`), we see it's a record variable expression. So far, we don't even know the type of `c`, `b` or `a`, but we know that if `b` or `a` are not records, the code is incorrectly typed.

Diving deeper into the recursion, we see `a` is a record variable expression as well. It could be previously declared as an `int` and the syntax parser would accept it, since it has no knowledge of typing.

Now we are stuck with our local scenario. Which type is `a`? It could have been declared an integer or, even worse, a float! Well, if only we had a global view of our names... Except we do. Our binding stack should contain the reference that `a` is bound to.

Getting the reference node from the stack, we first check that `a` is a variable. Since every variable has a type, we check its type. If it is a record type, reference the field of id `b` in the record variable. Now, check if the type of `b` is again a record and do the same procedure for `c`. Now, check if the type of `c` is matching to the type of `0` (an integer).