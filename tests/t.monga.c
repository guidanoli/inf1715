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
			case MONGA_TK_AS:
				printf("AS\n");
				break;
			case MONGA_TK_ELSE:
				printf("ELSE\n");
				break;
			case MONGA_TK_FUNCTION:
				printf("FUNCTION\n");
				break;
			case MONGA_TK_IF:
				printf("IF\n");
				break;
			case MONGA_TK_NEW:
				printf("NEW\n");
				break;
			case MONGA_TK_RETURN:
				printf("RETURN\n");
				break;
			case MONGA_TK_VAR:
				printf("VAR\n");
				break;
			case MONGA_TK_WHILE:
				printf("WHILE\n");
				break;
			case MONGA_TK_EQ:
				printf("EQ\n");
				break;
			case MONGA_TK_NE:
				printf("NE\n");
				break;
			case MONGA_TK_LE:
				printf("LE\n");
				break;
			case MONGA_TK_GE:
				printf("GE\n");
				break;
			case MONGA_TK_AND:
				printf("AND\n");
				break;
			case MONGA_TK_OR:
				printf("OR\n");
				break;
			case MONGA_TK_TYPE:
				printf("TYPE\n");
				break;
			case MONGA_TK_CALL:
				printf("CALL\n");
				break;
			default:
				if (tk >= 0 && tk <= UCHAR_MAX)
					printf("CHAR '%c'\n", tk);
				else
					printf("UNKNOWN %d\n", tk);
				break;
		}
	}
	return MONGA_ERR_OK;
}
