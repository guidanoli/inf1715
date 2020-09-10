#include <stdio.h>
#include <stdlib.h>

#include "monga.h"

monga_token_value yylval;

int main(int argc, char** argv)
{
	int tk;
	while (tk = yylex()) {
		switch (tk) {
			case MONGA_TK_ID:
				printf("ID \"%.*s\"\n", yylval.id.size, yylval.id.str);
				free(yylval.id.str);
				break;
			case MONGA_TK_INTEGER:
				printf("INTEGER %d\n", yylval.integer);
				break;
			case MONGA_TK_REAL:
				printf("REAL %g\n", yylval.real);
				break;
			default:
				printf("UNKNOWN\n");
				break;
		}
	}
	return 0;
}
