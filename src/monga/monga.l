%{
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "monga_utils.h"
#include "monga.y.h"

/* POSIX function */
extern int fileno (FILE *stream);

static size_t monga_lineno = 1;
%}

ID [a-zA-Z_][a-zA-Z_0-9]*

HEXADECIMAL_0X "0"[xX]
HEXADECIMAL [0-9a-fA-F]

DIGIT [0-9]

INTEGER_DECIMAL {DIGIT}+

REAL_DECIMAL_EXPONENT [eE][+-]?{DIGIT}+
REAL_DECIMAL ({DIGIT}*("."{DIGIT}+|{DIGIT}".")({REAL_DECIMAL_EXPONENT})?|{DIGIT}+{REAL_DECIMAL_EXPONENT})

INTEGER_HEXADECIMAL {HEXADECIMAL_0X}{HEXADECIMAL}+

REAL_HEXADECIMAL_EXPONENT [pP][+-]?{DIGIT}+
REAL_HEXADECIMAL_BASE {HEXADECIMAL_0X}{HEXADECIMAL}*("."{HEXADECIMAL}+|{HEXADECIMAL}(".")?)
REAL_HEXADECIMAL {REAL_HEXADECIMAL_BASE}{REAL_HEXADECIMAL_EXPONENT}[lLfF]?

%option noyywrap
%%
[\n] { ++monga_lineno; }
[ \t] ;
#[^\n]* ;
"as" { return MONGA_TK_AS; }
"else" { return MONGA_TK_ELSE; }
"function" { return MONGA_TK_FUNCTION; }
"if" { return MONGA_TK_IF; }
"new" { return MONGA_TK_NEW; }
"return" { return MONGA_TK_RETURN; }
"type" { return MONGA_TK_TYPE; }
"var" { return MONGA_TK_VAR; }
"while" { return MONGA_TK_WHILE; }
"null" { return MONGA_TK_NULL; }
"==" { return MONGA_TK_EQ; }
"~=" { return MONGA_TK_NE; }
"<=" { return MONGA_TK_LE; }
">=" { return MONGA_TK_GE; }
"&&" { return MONGA_TK_AND; }
"||" { return MONGA_TK_OR; }
{ID} {
	yylval.terminal.id = monga_memdup(yytext, yyleng+1);
	yylval.terminal.line = monga_lineno;
	return MONGA_TK_ID;
}
{INTEGER_DECIMAL} {
	yylval.terminal.integer = strtol(yytext, NULL, 10);
	yylval.terminal.line = monga_lineno;
	return MONGA_TK_INTEGER;
}
{REAL_DECIMAL} {
	yylval.terminal.real = strtod(yytext, NULL);
	yylval.terminal.line = monga_lineno;
	return MONGA_TK_REAL;
}
{INTEGER_HEXADECIMAL} {
	yylval.terminal.integer = strtol(yytext, NULL, 16);
	yylval.terminal.line = monga_lineno;
	return MONGA_TK_INTEGER;
}
{REAL_HEXADECIMAL} {
	yylval.terminal.real = strtod(yytext, NULL);
	yylval.terminal.line = monga_lineno;
	return MONGA_TK_REAL;
}
. {
	yylval.terminal.line = monga_lineno;
	return *yytext;
}
%%
size_t monga_get_lineno()
{
	return monga_lineno;
}