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
enum {LIST, PLUS, MINUS, MULT, QUO, NE, EQ, LT, LE, GT, GE, CONCAT, ID, IDC, ITE, NEWC, ENVOI, CAST, SELEC, CONST, STRG, THI, SUP, RES, RET, AFFECT, ISBLOC, OVRD, EXT, BODY,DECLS};

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

struct _Param;
typedef struct _Param Param, *ParamP;

struct _Method;
typedef struct _Method Method, *MethodP;

struct _DeclParam;
typedef struct _DeclParam DeclParam, *DeclParamP;

/* la structure d'un arbre (noeud ou feuille) */
struct _Tree {
    short op;         /* etiquette de l'operateur courant */
    short nbChildren; /* nombre de sous-arbres */
    union {
	char *str;      /* valeur de la feuille si op = Id ou STR */
	int val;        /* valeur de la feuille si op = Cste */
	DeclParamP declParams;
	struct _Tree **children; /* tableau des sous-arbres */
    } u;
};

struct _Method {
    char* name;
    DeclParamP params;
    TreeP body;
    ClassP return_type;
    int override;
    struct _Method *next;
};

struct _Class {
    char* name;
    TreeP constructorBody;
    DeclParamP constructorParams;
    MethodP methods;
    DeclParamP members;
    struct _Class *super;
    TreeP superTree;
    struct _Class *next;
};

struct _DeclParam {
    char* name;
    ClassP type;
    char* typeName;
    TreeP expression;
    int decl;
    struct _DeclParam *next;
};

typedef union
{
    char *S;
    char C;
    int I;
    TreeP pT;
    MethodP pM;
    ClassP pC;
    DeclParamP pDP;
} YYSTYPE;

#define YYSTYPE YYSTYPE

TreeP makeLeafStr(short op, char *str);       
TreeP makeLeafInt(short op, int val);	      
TreeP makeTree(short op, int nbChildren, ...);
TreeP makeLeafLVar(short op, DeclParamP lvar);
void fillClass(ClassP class, DeclParamP params, TreeP extends, TreeP constructor, DeclParamP members, MethodP methods);
ClassP makeClass(char *name);
MethodP makeMethod(int override, char* name, DeclParamP params, TreeP body);
DeclParamP makeDecl(char* name, char* class, TreeP expression);
DeclParamP makeParam(char* name, char* class);
void evalMain(TreeP tree);
