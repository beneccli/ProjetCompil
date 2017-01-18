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

returnOpt:                                                   {$$ = NIL(Tree);}
| ':' Idc                                                    {$$ = makeLeafStr(IDC,$2);
;

redifOpt:                                                    {$$ = NIL(Tree);}
| OVERRIDE                                                   {$$ = makeTree(OVRD,0);}
;

blockCons:                                                   {$$ = NIL(Tree);}
| block                                                      {$$ = $1;}
;


block: '{' blockOpt '}'                                      {$$ =$2;}
;


blockOpt: instrLOpt                                          {$$ = $1;}
| varDecl varLOpt IS instr instrLOpt                         {TreeP t1 = makeTree(LIST,2,$1,$2);
                                                              TreeP t2 = makeTree(LIST,2,$4,$5);
                                                              $$ = makeTree(ISBLOC,2,t1,t2);} 
;

instrLOpt:                                                   {$$ = NIL(Tree);}
| instr instrLOpt                                            {$$ = makeTree(LIST,2,$1,$2);}
;

instr: block                                                 {$$ = $1;}
| expr ';'                                                   {$$ = $1;}
| RETURN ';'                                                 {$$= makeTree(RET,0);}
| expr AFF expr ';'                                          {$$ = makeTree(AFFECT,2,$1,$3);}
| IF expr THEN instr ELSE instr                              {$$ = makeTree(ITE,3,$2,$4,$6);}
;

expr: expr RelOp expr                                        {$$ = makeTree($2.C,2,$1,$3);}
| expr ADD expr                                              {$$ = makeTree(PLUS,2,$1,$3);}
| expr SUB expr                                              {$$ = makeTree(MINUS,2,$1,$3);}
| expr MUL expr                                              {$$ = makeTree(MULT,2,$1,$3);}
| expr DIV expr	                                             {$$ = makeTree(QUO,2,$1,$3);}
| ADD expr %prec unary                                       {$$ = makeTree(PLUS,1,$2);}
| SUB expr %prec unary                                       {$$ = makeTree(MINUS,1,$2);}
| expr AND expr                                              {$$ = makeTree(CONCAT,2,$1,$3);}
| NeW Idc '(' exprLOpt ')'                                   {$$ = makeTree(NEWC,2,makeLeafStr(IDC,$2),$4);}
| '(' AS Idc ':' expr ')'                                    {$$ = makeTree(CAST,2,makeLeafStr(IDC,$3),$5);}
| expr DOT Id                                                {$$ = makeTree(SELEC,2,$1,makeLeafStr(ID,$3));}
| Cste		                                             {$$ = makeLeafInt(CONST,$1);}
| Id                                                         {$$ = makeLeafStr(ID,$1);}  
| Str                                                        {$$ = makeLeafStr(STRG,$1);}
| THIS                                                       {$$ = makeTree(THI,0);}
| message                                                    {$$ = $1;}
| SUPER                                                      {$$ = makeTree(SUP,0);}
| RESULT	                                             {$$ = makeTree(RES,0);}
| '(' expr ')'		                                     {$$ = $2;}
;

message: expr DOT Id '(' exprLOpt ')'                        {$$ = makeTree(ENVOI,3,$1,makeLeafStr(ID,$3),$5);}
;

exprLOpt:                                                    {$$ = NIL(Tree);}
|exprL                                                       {$$ = $1;}
;

exprL: expr                                                  {$$ = makeTree(LIST,2,$1,NIL(Tree));}
| expr ',' exprL                                             {$$ = makeTree(LIST,2,$1,$3);}
;




















