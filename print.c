#include <unistd.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"
extern void setError();
extern TreeP getChild(TreeP tree, int rank);
extern bool verbose;
extern bool noCode;
extern bool debug;
extern ClassP listClass;


/* Affichage d'une expression binaire */
void pprintTree2(TreeP tree, char *op) {
    if (tree->nbChildren == 2 && getChild(tree, 0)!=NULL && getChild(tree, 1)!=NULL) {
	pprint(getChild(tree, 0));
	printf("%s", op);
	pprint(getChild(tree, 1));
    }else pprintTree1(tree,op);
	
}

/* Affichage d'une expression unaire */
void pprintTree1(TreeP tree, char *op) {
    printf("(%s(", op); /* on parenthese explicitement l'unique operande */
    pprint(getChild(tree, 0));
    printf("))");
}

/* Affichage d'un if then else */
void pprintIf(TreeP tree) {
    printf("if ");
    pprint(getChild(tree, 0));
    printf(" then ");
    pprint(getChild(tree, 1));
    printf(" else ");
    pprint(getChild(tree, 2));
}

/* Affichage d'un bloc */
void pprintBloc(TreeP tree) {
    printf(" { ");
    if (tree->nbChildren == 2){
	pprint(getChild(tree, 0));
	printf(" is ");
	pprint(getChild(tree, 1));
    }else 
	pprint(getChild(tree, 0));
    printf(" } ");
}

void pprintAff(TreeP tree) {  
    if (tree->nbChildren == 2){
	pprint(getChild(tree,0));
	printf(" := ");
	pprint(getChild(tree, 1));
	printf(";");
    }
  
}

void pprintMes(TreeP tree){
    if(tree->nbChildren == 3){
	pprint(getChild(tree, 0));
	printf(".");
	pprint(getChild(tree, 1));
	printf("(");
	if (getChild(tree, 2) != NULL)
	    pprint(getChild(tree, 2));
	printf(")");
    }
}

void pprintNew(TreeP tree){
    printf("new ");
    pprint(getChild(tree, 0));
    printf("(");
    pprint(getChild(tree, 1));
    printf(")");
}

void pprintDec(TreeP tree){
    printf("var ");
    printf("%s",tree->u.declParams->name);
    printf(" : ");
    if(tree->u.declParams->type != NULL)
	printf("%s",tree->u.declParams->type->name);
    else
	printf("Unknown");
    if (tree->u.declParams->expression != NULL) {
	printf(" := ");
	pprint(tree->u.declParams->expression);
    }
    printf(";\n");
}

void pprintListExp(TreeP tree){
    if(tree != NULL) {
	pprint(getChild(tree, 0));
	if (getChild(tree,1) != NULL){
	    printf(",");
	    pprint(getChild(tree, 1));
	}
    }
}

void pprintList(TreeP tree){
    printf("\n");
    if (getChild(tree, 1) != NULL) {
	pprint(getChild(tree, 0));
	pprint(getChild(tree, 1));
    } else 
	pprint(getChild(tree, 0));
}


void pprintBody(TreeP tree){
    if (getChild(tree,1)->op != LIST){	
	printf(" is ");
	pprint(getChild(tree,1));
    }else {
	printf(" := ");
	pprint(getChild(getChild(tree,1),0));
    }
    
}

void pprintCast(TreeP tree){
    printf("( as ");
    pprint(getChild(tree,0));
    printf(" : ");
    pprint(getChild(tree,1));
    printf(" ) ");
}


int nbrParentheses = 0;
/* Affichage recursif d'un arbre representant une expression. */
void pprint(TreeP tree) {
    if (! verbose ) return;
    if (tree == NIL(Tree)) { 
	return;
    }
    if(debug){
	++nbrParentheses;
	printf("\n");
	int i;
	for(i = 0; i < nbrParentheses; i++){
	    printf(" ");
	};
	printf("(");
    }
  
    switch (tree->op) {
    case ID:    printf("%s", tree->u.str); break;
    case IDC:
	if(tree->idc != NULL)
	    printf("%s", tree->idc->name);
	else
	    printf("Unknown");
	break;
    case CONST:   printf("%d", tree->u.val); break;
    case STRG:   printf("%s", tree->u.str); break;
    case EQ:    pprintTree2(tree, " = "); break;
    case NE:    pprintTree2(tree, " <> "); break;
    case GT:    pprintTree2(tree, " > "); break;
    case GE:    pprintTree2(tree, " >= "); break;
    case LT:    pprintTree2(tree, " < "); break;
    case LE:    pprintTree2(tree, " <= "); break;
    case PLUS:   pprintTree2(tree, " + "); break;
    case MINUS:   pprintTree2(tree, " - "); break;
    case MULT:   pprintTree2(tree, " * "); break;
    case QUO:   pprintTree2(tree, " / "); break;
    case ITE:    pprintIf(tree); break;
    case CONCAT: pprintTree2(tree, " & "); break;
    case THI: printf("this"); break;
    case ISBLOC: pprintBloc(tree); break;
    case AFFECT: pprintAff(tree); break;
    case BODY: pprintBody(tree); break;
    case LISTEXP: pprintListExp(tree); break;
    case LIST: pprintList(tree);break;
    case DECLS: pprintDec(tree); break;
    case SELEC:  pprintTree2(tree, "."); break;
    case ENVOI: pprintMes(tree); break;
    case NEWC:  pprintNew(tree); break;
    case CAST: pprintCast(tree);break;
    case RET: printf("return;");break;
    case RES: printf("result");break;
    case SUP: printf("super");break;
    default:
	/* On signale le probleme mais on ne quitte pas le programme pour autant */
	fprintf(stderr, "Erreur! pprint : etiquette d'operator inconnue: %d\n", 
		tree->op);
    }
    if(debug){
	printf(")");
	nbrParentheses--;
    }
}

/* Affichage d'un champ */
void pprintChamps (DeclParamP cc){
    DeclParamP c = cc;
    if(c != NULL){
	pprintChamps(c->next);
	printf("\n");
	printf("var ");
	if(c->type != NULL)
	    printf("%s : %s ",c->name, c->type->name);
	else
	    printf("%s : Unknown ",c->name);
	if (c->expression != NULL) {
	    printf(" := ");
	    pprint(c->expression);
	}
	printf(";");	

    }
}

/* Affichage d'un paramètre */
void pprintParam (DeclParamP pp){
    DeclParamP p = pp;
    if (p!=NULL){
	pprintParam(p->next);
	if(p->next != NULL)
	    printf(", ");	
	if(p->type != NULL)
	    printf("%s : %s",p->name, p->type->name);
	else
	    printf("%s : Unknown",p->name);
    }
  
}

/* Affichage d'un extends */
void pprintExtends (TreeP e){
    if (e!=NULL && getChild(e, 0)->op == EXT){
	printf("(");
	pprint(getChild(e,1));
	printf(")");
    }
}

/* Affichage d'une méthode */
void pprintMethods (MethodP mm){
    MethodP m = mm;
  
    if(mm != NULL){
  
  	pprintMethods(m->next);
  	printf("\n\n");
	if (m->override){
	    printf("override ");
    	}	
  	printf("def ");
  	printf("%s(",m->name);
    	pprintParam(m->params);
  	printf(") ");
	if(m->returnType != NULL) {
	    printf(" : %s", m->returnType->name);
	}
  	pprint(m->body);
    }
}

/* Affichage d'une classe */


void pprintAllClasse (ClassP c){
    if(c != NULL){
	pprintAllClasse(c->next);
	printf("class ");
	printf("%s(",c->name);
	pprintParam(c->constructorParams);
	printf(") ");
	if (c->super != NULL) {
	    printf("extends %s",c->super->name);
	    pprintExtends(c->superTree);
	}
	if(c->constructorBody != NULL) pprint(c->constructorBody);
	printf(" is { ");
	pprintChamps(c->members);
	pprintMethods(c->methods);
	printf(" }\n\n");
	  
    }
}

void pprintMain(TreeP tree) {
    if (! verbose) return;
    printf("Classes : \n\n");
    pprintAllClasse(listClass);
    printf("Expression principale : \n");
    pprint(tree);
    printf("\n");
    fflush(NULL);
}
