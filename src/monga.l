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

SIGNAL [+-]
DIGIT [0-9]

INTEGER_DECIMAL {SIGNAL}?{DIGIT}+

REAL_DECIMAL_EXPONENT [eE]{INTEGER_DECIMAL}
REAL_DECIMAL (({SIGNAL}?{DIGIT}*"."{DIGIT}+|{INTEGER_DECIMAL}".")({REAL_DECIMAL_EXPONENT})?|{INTEGER_DECIMAL}{REAL_DECIMAL_EXPONENT})

INTEGER_HEXADECIMAL {HEXADECIMAL_0X}{HEXADECIMAL}+

REAL_HEXADECIMAL_EXPONENT [pP]{INTEGER_DECIMAL}
REAL_HEXADECIMAL_BASE {HEXADECIMAL_0X}{HEXADECIMAL}*("."{HEXADECIMAL}+|{HEXADECIMAL}(".")?)
REAL_HEXADECIMAL {REAL_HEXADECIMAL_BASE}{REAL_HEXADECIMAL_EXPONENT}[lLfF]?

%option noyywrap
%%
"as" { return MONGA_TK_AS; }
"else" { return MONGA_TK_ELSE; }
"function" { return MONGA_TK_FUNCTION; }
"if" { return MONGA_TK_IF; }
"new" { return MONGA_TK_NEW; }
"return" { return MONGA_TK_RETURN; }
"var" { return MONGA_TK_VAR; }
"while" { return MONGA_TK_WHILE; }
{ID} {
	yylval.id.size = yyleng;
	yylval.id.str = memdup(yytext, yyleng);
	return MONGA_TK_ID;
}
{INTEGER_DECIMAL} {
	yylval.integer = strtol(yytext, NULL, 10);
	return MONGA_TK_INTEGER;
}
{REAL_DECIMAL} {
	yylval.real = strtod(yytext, NULL);
	return MONGA_TK_REAL;
}
{INTEGER_HEXADECIMAL} {
	yylval.integer = strtol(yytext, NULL, 16);
	return MONGA_TK_INTEGER;
}
{REAL_HEXADECIMAL} {
	yylval.real = strtod(yytext, NULL);
	return MONGA_TK_REAL;
}
[ \n\t] ;
. { return *yytext; }
%%

void* memdup(void* mem, int size)
{
	void* new_mem = malloc(size);
	if (new_mem)
		memcpy(new_mem, mem, size);
	return new_mem;
}
