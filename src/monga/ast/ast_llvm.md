# AST LLVM code generation

Based on the Abstract Syntactic Tree, LLVM code will be generated so that it can be compiled to assembly and then, to machine code. The development process will follow a strategic bottom up approach so that partial results can be effectively tested and validated.

## Metodology

Clang has a flag `-emit-llvm` which compiles C code to equivalent LLVM code. This will be used multiple times to check how Monga, which is a subset of C, should be compiled to LLVM.

Compiling `temp.c` to `temp.ll` is done by the following command.

```sh
$ clang -emit-llvm -S temp.c
```

Be aware that much of the LLVM code will be ignored for the sake of simplicity, which can lead to less optimal code.

## `def_variable`

A variable definition is very simple. It follows the following pattern:

```llvm
@<name> = internal global <type> <value>
```

It is `internal` because the variable is visible only for the library and works like `static` in C. Global variables, on the other hand, would be `common`.

`<type>` is a LLVM type descriptor.

For simplicity and possibly more optimal code, `<value>` will always be `undef`.

## `def_type`

There are three types of type descriptors.

### `builtin_typedesc`

The mapping between Monga and LLVM built-in types is described in the following table.

| Monga | LLVM |
| :-: | :-: |
| `int` | `i32` |
| `float` | `float` |
| `null` | `opaque` |

But `null` will never be used because it is internal.

### `id_typedesc`

This is simply an alias. The first non-alias typedescriptor will be used.

### `array_typedesc`

This will simply be the array subtype followed by an asterisk (`*`).

### `record_typedesc`

This will need to follow the structured type syntax:

```llvm
%<name> = type { [<type>] }
```

where `[<type>]` represents a list of type descriptors, joined with a `,`.

## `def_function`

Function definitions follow this basic pattern:

```llvm
define <return-type> @<function-name> ([<type> %<reg>]) {
    ret <return-type> <value>
}
```

Where `return-type` is the type descriptor of the function return type. If the function does not return a value, it is `void`, and the return statement is `ret void`.

And `[<type> %<reg>]` denotes a list of `,`-separated parameter descriptions. `type` is the parameter type descriptor and `reg` is the register label.