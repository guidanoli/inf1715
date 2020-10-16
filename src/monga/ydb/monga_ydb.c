#include <stdio.h>

#include "monga.y.h"
#include "monga_ast.h"

void yyerror(const char* err)
{
    fprintf(stderr, "%s (line %zu)\n", err, monga_get_lineno());
}

int main(int argc, char** argv)
{
    int res = yyparse();
    if (!res) {
        monga_ast_program_bind(root);
        monga_ast_program_print(root);
        monga_ast_program_destroy(root);
        if (monga_get_allocated_cnt() != 0) {
            fprintf(stderr, "Memory leak detected.\n");
#ifdef MONGA_DEBUG
            monga_clean_amb();
#endif
            return MONGA_ERR_LEAK;
        }
    }
    return res;
}