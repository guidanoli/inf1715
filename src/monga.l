%{
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "monga.h"

static void* memdup(void* mem, int size);
static void copy_yytext();
%}

ID [a-zA-Z_][a-zA-Z_0-9]+

INTEGER_DECIMAL [+-]?(0|[1-9][0-9]*)
INTEGER_HEXADECIMAL "0x"[0-9a-fA-F]+

REAL_EXPONENT [eE]{INTEGER_DECIMAL}
REAL_BASE_BEFORE {INTEGER_DECIMAL}
REAL_BASE_AFTER [0-9]+
REAL_BASE_11 {REAL_BASE_BEFORE}"."{REAL_BASE_AFTER}
REAL_BASE_10 {REAL_BASE_BEFORE}(".")?
REAL_BASE_01 "."{REAL_BASE_AFTER}
REAL_BASE ({REAL_BASE_01}|{REAL_BASE_11}|{REAL_BASE_10})
REAL {REAL_BASE}({REAL_EXPONENT})?

%option noyywrap
%%
{ID} {
	copy_yytext();
	return MONGA_TK_ID;
}
{REAL} {
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
[ \n\t] ;
%%

void copy_yytext()
{
	yylval.id.size = yyleng;
	yylval.id.str = memdup(yytext, yyleng);
}

void* memdup(void* mem, int size)
{
	void* new_mem = malloc(size);
	if (new_mem)
		memcpy(new_mem, mem, size);
	return new_mem;
}
