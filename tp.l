Chiffre [0-9]
Entier {Chiffre}
Lower	[a-z]
Upper	[A-Z]
Lettre	[a-zA-Z]
LC 	({Lettre}|{Chiffre})
Delim   [ \t\n\r]
Symbole [&();:,\.{}]
BS      \\.
PasBSG [^\\"]
Guill		"\""
Star  		"*"
NotStar		[^*]
NotStarPar      [^*/]
EscGuill [^(\\(^"))]
Str	\"({BS}|{EscGuill})*\"


%{
#include <string.h>
#include <strings.h>
#include "tp.h"
#include "tp_y.h"

extern YYSTYPE yylval;
extern int yylineno;
extern int yylex();

int keywordOrId() {
  if (! strcmp(yytext, "if")) return IF;
  if (! strcmp(yytext, "new")) return NeW;
  if (! strcmp(yytext, "then")) return THEN;
  if (! strcmp(yytext, "else")) return ELSE;
  if (! strcmp(yytext, "class")) return CLASS;
  if (! strcmp(yytext, "return")) return RETURN;
  if (! strcmp(yytext, "is")) return IS;
  if (! strcmp(yytext, "as")) return AS;
  if (! strcmp(yytext, "var")) return VAR;
  if (! strcmp(yytext, "def")) return DEF;
  if (! strcmp(yytext, "extends")) return EXTENDS;
  if (! strcmp(yytext, "override")) return OVERRIDE;
  if (! strcmp(yytext, "this")) return THIS;
  if (! strcmp(yytext, "super")) return SUPER;
  if (! strcmp(yytext, "result")) return RESULT;
  
  yylval.S = strdup(yytext);
  
  return(Id);
}


void lexError(char c) {
  fprintf(stderr, "Error: Skipping invalid character '%c' on line %d\n",
          c, yylineno);
  setError(LEXICAL_ERROR);
}
%}

%%
{Delim}+	{}
{Entier}+ 	{ yylval.I = atoi(yytext); return(Cste); }
{Str}		{ yylval.S = strdup(yytext); return(Str); }
{Lettre}{LC}*	{ return keywordOrId(); }
{Symbole}	{ return(yytext[0]); }
":="		{ return(AFF); }
"="		{ yylval.C = EQ; return(RelOp); }
"<>"		{ yylval.C = NE; return(RelOp); }
"<="		{ yylval.C = LE; return(RelOp); }
">="		{ yylval.C = GE; return(RelOp); }
">"		{ yylval.C = GT; return(RelOp); }
"<"		{ yylval.C = LT; return(RelOp); }
"+"		{ return(ADD); }
"-"		{ return(SUB); }
"/"		{ return(DIV); }
"*"		{ return(MUL); }
%%
