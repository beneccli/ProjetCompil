/* attention: NEW est defini dans tp.h Utilisez un autre token */
%token IS CLASS VAR EXTENDS DEF OVERRIDE RETURN AS IF THEN ELSE AFF ADD SUB MUL DIV THIS SUPER RESULT NeW DOT AND
%token<S> Id Str Idc
%token<I> Cste
%token<C> RelOp

%right ELSE
%left RelOp
%left ADD SUB
%left MUL DIV
%nonassoc unary
%left AND
%left DOT


%{
#include "tp.h"
#include "tp_y.h"

extern int yylex();
extern void yyerror(char *);
%}

%%
Prog : classLOpt block
;

classLOpt:
| class classLOpt 
;

class: declClass blockCons IS '{' corps '}'
;

declClass: CLASS Idc '(' paramLOpt ')' extendsOpt
;

extendsOpt:
| EXTENDS Idc '(' exprLOpt ')'
;

paramLOpt:
| paramL
;

paramL: param
| param ',' paramL
;

param: Id ':' Idc
;

corps: varLOpt methodeLOpt
;

varLOpt:
| varDecl varLOpt
;

varDecl: VAR Id ':' Idc affOpt ';'
;

affOpt:
| AFF expr
;

methodeLOpt:
| methodeDecl methodeLOpt
;

methodeDecl: redifOpt DEF Id '('paramLOpt')' bodyAlt
;

bodyAlt: ':' Idc AFF expr
| returnOpt IS block
;

returnOpt:
| ':' Idc
;

redifOpt:
| OVERRIDE
;

block: '{' blockOpt '}'
;

blockCons: 
| block
;

blockOpt: instrLOpt
| varDecl varLOpt IS instr instrLOpt 
;

instrLOpt:
| instr instrLOpt
;

instr: block
| expr ';'
| RETURN ';'
| Id AFF expr ';'
| IF expr THEN instr ELSE instr
;

expr: expr RelOp expr 
| expr ADD expr
| expr SUB expr 
| expr MUL expr         
| expr DIV expr	      
| ADD expr %prec unary  
| SUB expr %prec unary
| expr AND expr
| NeW Idc '(' exprLOpt ')'  
| '(' AS Idc ':' expr ')'
| expr DOT Id
| Cste		       
| Id
| Str
| THIS
| message
| SUPER
| RESULT	
| '(' expr ')'		
;

message: expr DOT Id '(' exprLOpt ')'
;

exprLOpt:
|exprL
;

exprL: expr
| expr ',' exprL
;




















