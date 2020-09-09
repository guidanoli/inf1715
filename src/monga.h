#ifndef MONGA_H
#define MONGA_H

#include <limits.h>

/* Token type */
typedef enum
{
	MONGA_TK_ID = UCHAR_MAX + 1,
	MONGA_TK_INTEGER,
	MONGA_TK_REAL,
}
monga_token_t;

/* Token value */
typedef union
{
	struct {
		char* str;
		int size;
	} id;
	int integer;
	double real;
}
monga_token_value;

/* Current token value */
extern monga_token_value yylval;

/* Lex scanner */
extern int yylex();

#endif
