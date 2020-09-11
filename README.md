# Monga

Monga (*not to be mistaken with [mango](https://en.wikipedia.org/wiki/Mango), a tropical fruit*)
is a subset of the [C programming language](https://en.wikipedia.org/wiki/C_(programming_language)).
The goal of the project is to build a compiler for it, making use of some common tools such as
[Lex](http://dinosaur.compilertools.net/lex/index.html),
[Yacc](http://dinosaur.compilertools.net/yacc/index.html) and
[LLVM-C](http://llvm.org/doxygen/group__LLVMC.html).

## Dependencies

* A good C99 compliant compiler
* [Lex](http://dinosaur.compilertools.net/lex/index.html)
* [Yacc](http://dinosaur.compilertools.net/yacc/index.html)
* GNU Make
* Bash

## Building

Run `make` on the repository root.

## Testing

Run `make tests` on the repository root.

### Adding test cases

0. Figure out what exactly is that you want to test.
1. Add a new test case by running `tests/add` (opens vi).
2. Write the new test case and save it (`:x`).
3. Generate the output by running `tests/generate` (Take note of the test number, #).
4. Compare the output with the input by running `tests/compare #` (# is the test number).
5. Check if the obtained output matches the expected output.
   a. If the output matches the expected, approve it (y).
   b. If it doesn't, try fix the bugs, run `make` and go back to step 3.
6. Run `make tests` one more time to check if all tests pass.

## External links (*in Portuguese*)

* [INF1715 home page](http://www.inf.puc-rio.br/~roberto/comp/)
* [About the language](http://www.inf.puc-rio.br/~roberto/comp/lang.html)
