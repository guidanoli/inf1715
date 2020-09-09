%{
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "monga.h"

static void* memdup(void* mem, int size);
%}

ID [a-zA-Z_][a-zA-Z_0-9]+

HEXADECIMAL_0X "0"[xX]
HEXADECIMAL [0-9a-fA-F]

INTEGER_DECIMAL [+-]?(0|[1-9][0-9]*)
INTEGER_HEXADECIMAL {HEXADECIMAL_0X}{HEXADECIMAL}+

REAL_DECIMAL_EXPONENT [eE]{INTEGER_DECIMAL}
REAL_DECIMAL_BASE_0 ({INTEGER_DECIMAL})?"."[0-9]+
REAL_DECIMAL_BASE_1 {INTEGER_DECIMAL}(".")?
REAL_DECIMAL_BASE ({REAL_DECIMAL_BASE_0}|{REAL_DECIMAL_BASE_1})
REAL_DECIMAL {REAL_DECIMAL_BASE}({REAL_DECIMAL_EXPONENT})?

REAL_HEXADECIMAL_EXPONENT [pP]{INTEGER_DECIMAL}
REAL_HEXADECIMAL_BASE_0 {HEXADECIMAL_0X}{HEXADECIMAL}*"."{HEXADECIMAL}+
REAL_HEXADECIMAL_BASE_1 {HEXADECIMAL_0X}{HEXADECIMAL}+(".")?
REAL_HEXADECIMAL_BASE ({REAL_HEXADECIMAL_BASE_0}|{REAL_HEXADECIMAL_BASE_1})
REAL_HEXADECIMAL {REAL_HEXADECIMAL_BASE}({REAL_HEXADECIMAL_EXPONENT})?

REAL ({REAL_DECIMAL}|{REAL_HEXADECIMAL})

%option noyywrap
%%
{ID} {
	yylval.id.size = yyleng;
	yylval.id.str = memdup(yytext, yyleng);

	return MONGA_TK_ID;
}
{REAL_DECIMAL} {
	yylval.real = strtod(yytext, NULL);
	return MONGA_TK_REAL;
}
{INTEGER_DECIMAL} {
	yylval.integer = strtol(yytext, NULL, 10);
	return MONGA_TK_INTEGER;
}
{INTEGER_HEXADECIMAL} {
	yylval.integer = strtol(yytext, NULL, 16);
	return MONGA_TK_INTEGER;
}
{REAL_DECIMAL} {
	yylval.real = strtod(yytext, NULL);
	return MONGA_TK_REAL;
}
[ \n\t] ;
%%

void* memdup(void* mem, int size)
{
	void* new_mem = malloc(size);
	if (new_mem)
		memcpy(new_mem, mem, size);
	return new_mem;
}
