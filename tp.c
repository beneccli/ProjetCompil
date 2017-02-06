#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"

extern int yyparse();
extern int yylineno;

/* Niveau de 'verbosite'.
 * Par defaut, n'imprime que le resultat et les messages d'erreur
 */
bool verbose = FALSE;

/* Generation de code ou pas. Par defaut, on produit le code */
bool noCode = FALSE;

/* Pose de points d'arret ou pas dans le code produit */
bool debug = FALSE;

/* code d'erreur a retourner */
int errorCode = NO_ERROR;

FILE *out; /* fichier de sortie pour le code engendre */

int main(int argc, char **argv) {
  int fi;
  int i, res;

  listClass = NULL;

  out = stdout;
  for(i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'd': case 'D':
	debug = TRUE; continue;
      case 'v': case 'V':
	verbose = TRUE; continue;
      case 'e': case 'E':
	noCode = TRUE; continue;
      case '?': case 'h': case 'H':
	fprintf(stderr, "Appel: tp -v -e -d -o file.out programme.txt\n");
	exit(USAGE_ERROR);
      case'o':
	  if ((out= fopen(argv[++i], "w")) == NULL) {
	    fprintf(stderr, "erreur: Cannot open %s\n", argv[i]);
	    exit(USAGE_ERROR);
	  }
	break;
      default:
	fprintf(stderr, "Option inconnue: %c\n", argv[i][1]);
	exit(USAGE_ERROR);
      }
    } else break;
  }

  if (i == argc) {
    fprintf(stderr, "Fichier programme manquant\n");
    exit(USAGE_ERROR);
  }

  if ((fi = open(argv[i++], O_RDONLY)) == -1) {
    fprintf(stderr, "erreur: Cannot open %s\n", argv[i-1]);
    exit(USAGE_ERROR);
  }

  /* redirige l'entree standard sur le fichier... */
  close(0); dup(fi); close(fi);
  makeClass("Integer");
  makeClass("String");

  res = yyparse();
  if (out != NIL(FILE) && out != stdout) fclose(out);
  return res ? SYNTAX_ERROR : errorCode;
}


void setError(int code) {
  errorCode = code;
  if (code != NO_ERROR) { noCode = TRUE; /*  abort(); */}
}


/* yyerror:  fonction importee par Bison et a fournir explicitement. Elle
 * est appelee quand Bison detecte une erreur syntaxique.
 * Ici on se contente d'un message minimal.
 */
void yyerror(char *ignore) {
  printf("erreur de syntaxe: Ligne %d\n", yylineno);
  setError(SYNTAX_ERROR);
}


/* Tronc commun pour la construction d'arbre */
TreeP makeNode(int nbChildren, short op) {
  TreeP tree = NEW(1, Tree);
  tree->isCallMethod = 0;
  tree->op = op;
  tree->idc = NULL;
  tree->nbChildren = nbChildren;
  tree->u.children = nbChildren > 0 ? NEW(nbChildren, TreeP) : NIL(TreeP);
  return(tree);
}


/* Construction d'un arbre a nbChildren branches, passees en parametres */
TreeP makeTree(short op, int nbChildren, ...) {
  va_list args;
  int i;
  TreeP tree = makeNode(nbChildren, op);
  tree->idc = NULL;
  tree->isCallMethod = 0;
  va_start(args, nbChildren);
  for (i = 0; i < nbChildren; i++) { 
    tree->u.children[i] = va_arg(args, TreeP);
  }
  va_end(args);
  return(tree);
}


/* Retourne le rankieme fils d'un arbre (de 0 a n-1) */
TreeP getChild(TreeP tree, int rank) {
  if (tree->nbChildren < rank -1) { 
    fprintf(stderr, "Incorrect rank in getChild: %d\n", rank);
    abort();
  }
  return tree->u.children[rank];
}


void setChild(TreeP tree, int rank, TreeP arg) {
  if (tree->nbChildren < rank -1) { 
    fprintf(stderr, "Incorrect rank in getChild: %d\n", rank);
    abort();
  }
  tree->u.children[rank] = arg;
}


/* Constructeur de feuille dont la valeur est une chaine de caracteres */
TreeP makeLeafStr(short op, char *str) {
  TreeP tree = makeNode(0, op);
  tree->u.str = str;
  return tree;
}

/* Constructeur de feuille dont la valeur est une chaine de caracteres */
TreeP makeLeafCallMethod(char *str) {
  TreeP tree = makeNode(0, ID);
  tree->u.str = str;
  tree->isCallMethod = 1;
  return tree;
}

/* Constructeur de feuille dont la valeur est un entier */
TreeP makeLeafInt(short op, int val) {
  TreeP tree = makeNode(0, op); 
  tree->u.val = val;
  return(tree);
}

TreeP makeLeafLVar(short op, DeclParamP lvar) {
  TreeP tree = makeNode(0, op); 
  tree->u.declParams = lvar;
  return(tree);
}

void fillClass(ClassP class, DeclParamP params, TreeP extends, TreeP constructor, DeclParamP members, MethodP methods) {
  class->constructorParams = params;
  class->superTree = extends;
  class->constructorBody = constructor;
  class->members = members;
  class->methods = methods;
}

ClassP makeClass(char *name) {
  ClassP class = NEW(1, Class);
  class->name = name;
  class->constructorBody = NULL;
  class->constructorParams = NULL;
  class->methods = NULL;
  class->members = NULL;
  class->super = NULL;
  class->superTree = NULL;
  class->next = NULL;
  if(listClass != NULL)
    class->next = listClass;
  listClass = class;
  return class;
}

MethodP makeMethod(int override, char* name, DeclParamP params, TreeP body) {
  MethodP method = NEW(1, Method);
  method->returnType = NULL;
  method->next = NULL;
  method->override = override;
  method->name = name;
  method->params = params;
  method->body = body;
  return method;
}

DeclParamP makeDecl(char* name, char* class, TreeP expression) {
  DeclParamP decl = NEW(1, DeclParam);
  decl->type = NULL;
  decl->next = NULL;
  decl->decl = 1;
  decl->name = name;
  decl->typeName = class;
  decl->expression = expression;
  return decl;
}

DeclParamP makeParam(char* name, char* class) {
  DeclParamP param = NEW(1, DeclParam);
  param->type = NULL;
  param->next = NULL;
  param->decl = 0;
  param->name = name;
  param->typeName = class;
  param->expression = NULL;
  return param;
}

MethodP getMethod(ClassP cl,char* name) {
    MethodP method = cl->methods;
    while(method != NULL) {
	if(!strcmp(method->name, name))
	    return method;
	method = method->next;
    }
    return NULL;
}

ClassP getClass(ClassP classEnv, char* name) {
    ClassP class = classEnv;
    while(class != NULL) {
	if(!strcmp(class->name, name))
	    return class;
	class = class->next;
    }
    return NULL;
}

void resolveTree(TreeP tree) {
    if(tree != NULL) {
        int i = 0;
	switch (tree->op) {
	case IDC:
	    tree->idc = getClass(listClass, tree->u.str);
	    break;
	case DECLS:
	    tree->u.declParams->type = getClass(listClass, tree->u.declParams->typeName);
	    resolveTree(tree->u.declParams->expression);
	default:
            for(i = 0; i<tree->nbChildren; i++) {		
	        resolveTree(getChild(tree, i));
	    }			
	}
    }
}
    
void resolveDeclParam(DeclParamP declParam) {
    if(declParam != NULL) {
	declParam->type = getClass(listClass, declParam->typeName);
	if(declParam->decl)
	    resolveTree(declParam->expression);
	resolveDeclParam(declParam->next);
    }
}

void resolveMethod(MethodP method) {
    if(method != NULL) {
	if(method->body) {
	    resolveTree(method->body);
	    if(method->body && getChild(method->body, 0)->op == IDC)
		method->returnType = getClass(listClass, getChild(method->body, 0)->u.str);	    
	}
	resolveDeclParam(method->params);
	resolveMethod(method->next);
    }
}

void resolveTreeMain(ClassP class) {
    if(class != NULL) {
	resolveTreeMain(class->next);
	if(class->superTree && class->superTree->op == EXT && getChild(class->superTree, 0)->op == IDC)
	    class->super = getClass(listClass, getChild(class->superTree, 0)->u.str);
	resolveTree(class->constructorBody);
	resolveDeclParam(class->constructorParams);
	resolveMethod(class->methods);
	resolveDeclParam(class->members);
	if(class->superTree)
	    resolveTree(getChild(class->superTree, 1));
    }
}
	
void evalMain(TreeP tree) {
    resolveTreeMain(listClass);
    resolveTree(tree);
    pprintMain(tree);
    printf("%d\n",verif_override(listClass));
    printf("%d\n",circuit_class(listClass));
    verif_portee(listClass);
}
