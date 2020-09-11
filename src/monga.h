#ifndef MONGA_H
#define MONGA_H

#include <limits.h>

/* Token type */
typedef enum
{
	MONGA_TK_ID = UCHAR_MAX + 1,
	MONGA_TK_INTEGER,
	MONGA_TK_REAL,
	MONGA_TK_AS,
	MONGA_TK_ELSE,
	MONGA_TK_FUNCTION,
	MONGA_TK_IF,
	MONGA_TK_NEW,
	MONGA_TK_RETURN,
	MONGA_TK_VAR,
	MONGA_TK_WHILE,
	MONGA_TK_EQ,
	MONGA_TK_NE,
	MONGA_TK_LE,
	MONGA_TK_GE,
	MONGA_TK_AND,
	MONGA_TK_OR,
	MONGA_TK_TYPE,
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
