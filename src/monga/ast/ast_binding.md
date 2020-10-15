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
struct monga_ast_reference_t
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
struct monga_ast_reference_t
{
    char* id; /* Filled by the parser */
    struct {
        char *id; /* Filled by the parser */
        struct monga_ast_definition_t *ref; /* Filled by the 'binder' */
    } type;
};
```

Uma premissa que pode-se levar em conta na implementação é a que, caso `id` seja `NULL`, o valor que reside no campo `ref` é indeterminado. Isto porque a ausência de um identificador (que pode ser opcional, no caso de `def_function`) implica na ausência de uma referência. O contrário nem sempre é válido, por exemplo, em um programa (inválido) em que um identificador foi referenciado mas não declarado, este campo conteria lixo.