#include <stdio.h>
#include <stdlib.h>

#include "monga.y.h"
#include "monga_ast.h"

void yyerror(const char* err)
{
    fprintf(stderr, "%s\n", err);
}

int main(int argc, char** argv)
{
    int res = yyparse();
    if (root) {
        monga_ast_program_print(root, 0);
        monga_ast_program_destroy(root);
    }
    if (monga_get_allocated_cnt() != 0) {
        fprintf(stderr, "Memory leak detected.\n");
        exit(MONGA_ERR_LEAK);
    }
    return res;
}