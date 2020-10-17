#include "monga_ast.h"

#include "monga_ast_builtin.h"

void monga_ast_init()
{
    monga_ast_builtin_init();
}

void monga_ast_close()
{
    monga_ast_builtin_close();
}