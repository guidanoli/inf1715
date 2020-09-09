%{
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "monga.h"

static void* memdup(void* mem, int size);
static void copy_yytext();
%}

ID [a-zA-Z_][a-zA-Z_0-9]+

%option noyywrap
%%
{ID} {
	copy_yytext();
	return MONGA_TK_ID;
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
