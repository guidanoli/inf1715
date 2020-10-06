#include <stdio.h>

#include "monga.y.h"

void yyerror(const char* err)
{
    fprintf(stderr, "%s\n", err);
}

int main(int argc, char** argv)
{
    return yyparse();
}