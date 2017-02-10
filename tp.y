/* attention: NEW est defini dans tp.h Utilisez un autre token */
%token IS CLASS VAR EXTENDS DEF OVERRIDE RETURN AS IF THEN ELSE AFF ADD SUB MUL DIV THIS SUPER RESULT NeW DOT AND
%token<S> Id Str Idc
%token<I> Cste
%token<C> RelOp

%type<I> redifOpt
%type<pT> extendsOpt affOpt bodyAlt returnOpt blockCons block blockOpt instrLOpt instr expr message exprLOpt  exprL
%type<pC> classHeader
%type<pM> methodLOpt methodDecl
%type<pDP> paramLOpt paramL param varLOpt varDecl

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

#include <stdio.h>

extern int yylex();
extern void yyerror(char *);
%}

%%
Prog : classLOpt block                                                 {evalMain($2);}
;

classLOpt:                                                                          
| class classLOpt                                                     
;

class: classHeader '(' paramLOpt ')' extendsOpt blockCons IS '{' varLOpt methodLOpt '}' {fillClass($1,$3,$5,$6,$9,$10);}
;

classHeader: CLASS Idc                                                                   {$$ = makeClass($2);}
;

extendsOpt:                                                                              {$$ = NIL(Tree);}
| EXTENDS Idc '(' exprLOpt ')'                                                           {$$ = makeTree(EXT,2,makeLeafStr(IDC,$2),$4);}
;

paramLOpt:                                                                               {$$ = NIL(DeclParam);}
| paramL                                                                                 {$$ = $1;}
;

paramL: param                                                                            {$$ = $1;}
| param ',' paramL                                                                       {$$ = $1; $1->next = $3;}
;

param: Id ':' Idc                                                                        {$$ = makeParam($1,$3);}
;

varLOpt:                                                                                 {$$ = NIL(DeclParam);}
| varDecl varLOpt                                                                        {$$ = $1; $1->next = $2;}
;

varDecl: VAR Id ':' Idc affOpt ';'                                                       {$$ = makeDecl($2,$4,$5);}
;

affOpt:                                                                                  {$$ = NIL(Tree);}
| AFF expr                                                                               {$$ = $2;}
;

methodLOpt:                                                                             {$$ = NIL(Method);}
| methodDecl methodLOpt                                                                 {$$ = $1; $1->next = $2;}
;

methodDecl: redifOpt DEF Id '('paramLOpt')' bodyAlt                                     {$$ = makeMethod($1,$3,$5,$7);}
;

bodyAlt: ':' Idc AFF expr                                           {$$ = makeTree(BODY,2,makeLeafStr(IDC,$2),makeTree(LIST,2,$4,makeTree(RET,0)));}
| returnOpt IS block                                                                     {$$ = makeTree(BODY,2,$1,$3);}
;

returnOpt:                                                   {$$ = NIL(Tree);}
| ':' Idc                                                    {$$ = makeLeafStr(IDC,$2);}
;

redifOpt:                                                    {$$ = 0;}
| OVERRIDE                                                   {$$ = 1;}
;

blockCons:                                                   {$$ = NIL(Tree);}
| block                                                      {$$ = $1;}
;


block: '{' blockOpt '}'                                      {$$ =$2;}
;


blockOpt: instrLOpt                                          {$$ = makeTree(ISBLOC,1,$1);} 
 | varDecl varLOpt IS instr instrLOpt                         {$1->next = $2;
                                                              TreeP t1 = makeLeafLVar(DECLS,$1);
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

expr: expr RelOp expr                                        {$$ = makeTree($2,2,$1,$3);}
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

message: expr DOT Id '(' exprLOpt ')'                        {$$ = makeTree(ENVOI,3,$1,makeLeafCallMethod($3),$5);}
;

exprLOpt:                                                    {$$ = NIL(Tree);}
|exprL                                                       {$$ = $1;}
;

exprL: expr                                                  {$$ = makeTree(LISTEXP,1,$1);} 
| expr ',' exprL                                             {$$ = makeTree(LISTEXP,2,$1,$3);}
;




















