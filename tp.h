#include <stdlib.h>

#define TRUE 1
#define FALSE 0

typedef unsigned char bool;

#define NEW(howmany, type) (type *) calloc((unsigned) howmany, sizeof(type))
#define NIL(type) (type *) 0

/* Etiquettes additionnelles pour les arbres de syntaxe abstraite.
 * Certains tokens servent directement d'etiquette. Attention ici a ne pas
 * donner des valeurs identiques a celles des tokens.
 */
enum {LIST, PLUS, MINUS, MULT, QUO, NE, EQ, LT, LE, GT, GE, CONCAT, ID, IDC, ITE, NEWC, ENVOI, CAST, SELEC, CONST, STRG, THI, SUP, RES, RET, AFFECT, ISBLOC, OVRD};

/* Codes d'erreurs */
#define NO_ERROR	0
#define USAGE_ERROR	1
#define LEXICAL_ERROR	2
#define SYNTAX_ERROR    3
#define CONTEXT_ERROR	40	/* default value for this stage */
#define DECL_ERROR	41	/* more precise information */
#define TYPE_ERROR	42
#define EVAL_ERROR	50
#define UNEXPECTED	10O

struct _Tree;
typedef struct _Tree Tree, *TreeP;

struct _Class;
typedef struct _Class Class, *ClassP;

struct _Arg;
typedef struct _Arg Arg, *ArgP;

struct _Method;
typedef struct _Method Method, *MethodP;

struct _Decl;
typedef struct _Decl Decl, *DeclP;

struct _Block;
typedef struct _Block Block, *BlockP;


/* la structure d'un arbre (noeud ou feuille) */
struct _Tree {
	short op;         /* etiquette de l'operateur courant */
	short nbChildren; /* nombre de sous-arbres */
	union {
		char *str;      /* valeur de la feuille si op = Id ou STR */
		int val;        /* valeur de la feuille si op = Cste */
		BlockP block;
		struct _Tree **children; /* tableau des sous-arbres */
	} u;
};

struct _Block {
	DeclP declarations;
	TreeP body;
};

struct _Arg {
	char* name;
	ClassP type;
	struct _Arg *next;
};

struct _Method {
	char* name;
	ArgP args;
	BlockP body;
	ClassP return_type;
	struct _Method *next;
};

struct _Class {
	char* name;
	MethodP constructor;
	MethodP methods;
	DeclP members;
	struct _Class *super;
	DeclP declarations;
	struct _Class *next;
};

struct _Decl {
	char* name;
	ClassP type;
	TreeP expression;
	struct _Decl *next;
};

typedef union
{ char *S;
	char C;
	int I;
	TreeP pT;
} YYSTYPE;

#define YYSTYPE YYSTYPE
