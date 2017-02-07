#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(const char*);
extern TreeP getChild(TreeP tree, int rank);

extern void setError(int code);

/*classes circulaires*/
bool circuit_class(ClassP lc) {
  if (lc != NULL) {
    if(!isupper(lc->name[0]))
      return FALSE;
    if (getClass(lc->next,lc->name) != NULL)
      return FALSE;
    if (lc->superTree != NULL)
      if (getClass(lc->next,getChild(lc->superTree,0)->u.str) == NULL)
	return FALSE;
    return circuit_class(lc->next);
  } 
  else return TRUE;
}

/*override*/
bool same_params (DeclParamP p1, DeclParamP p2) {
  if (p1 != NULL && p2 != NULL) {
    if (p1->decl != 0 || p2->decl != 0 || strcmp(p1->typeName,p2->typeName) )
      return FALSE;
    return same_params(p1->next,p2->next);
  } 
  else if ( p1 != NULL || p2 != NULL)
    return FALSE;
  else return TRUE;
}


bool override_method(MethodP m, MethodP r) {
  if (m->override == 0 && r->override == 1 && strcmp(m->name,r->name) == 0) {
    if (same_params(m->params,r->params)) {
      if (getChild(m->body,0) == NULL && getChild(r->body,0) == NULL)
        return TRUE;
      else if (strcmp(getChild(m->body,0)->u.str,getChild(r->body,0)->u.str) == 0)
        return TRUE;
    }
  }
  return FALSE;
}


MethodP return_override_methods(MethodP methods) {
  MethodP res = NULL;
  MethodP res2 = NULL;
  MethodP m = methods;
  while (m != NULL) {
    if (m->override) {
      res = m;
      res->next = res2;
      res2 = res;
    }
    m = m->next;
  }
  return res2;
}

bool verif_override(ClassP lc) {
  if (lc != NULL) {
    if (lc->superTree == NULL && return_override_methods(lc->methods) != NULL)
      return FALSE;
    if (lc->superTree != NULL) {
      MethodP methods = return_override_methods(lc->methods);
      ClassP class = getClass(lc,getChild(lc->superTree,0)->u.str);
      MethodP tmp = NULL;
      while (methods != NULL) {
        if ((tmp=getMethod(class,methods->name)) == NULL)
          return FALSE;
        if (!override_method(tmp,methods))
          return FALSE;
        methods = methods->next;
      }
    } 
    return verif_override(lc->next);
  }
  else return TRUE; 
}

/*Portee*/
EnvP copyEnv(EnvP e) {
  EnvP itEnv = e;
  EnvP startNewEnv = NULL;
  EnvP env = NULL;
  EnvP envA = NULL;
  while(itEnv != NULL) {
    envA = env;
    env = NEW(1, Env);
    env->isMethod = itEnv->isMethod;
    env->name = itEnv->name;
    env->type = itEnv->type;
    env->next = NULL;

    if(startNewEnv == NULL)
      startNewEnv = env;
    else {
      envA->next = env;
      env = envA->next;
    }

    itEnv = itEnv->next;
  }
  return startNewEnv;
}

EnvP concatEnv(EnvP e1, EnvP e2) {
  if(e1 == NULL)
    return e2;
  EnvP itEnv = e1;
  while(itEnv->next != NULL)
    itEnv = itEnv->next;
  itEnv->next = e2;
  return e1;
}

bool verif_scope(ClassP c, TreeP main) {
  envSClass(c);
  envSTree(main);
  
  return TRUE;
}

EnvP envSClass(ClassP c) {
  if(c != NULL) {
    envSTree(c->constructorBody);
    envSDeclParam(c->constructorParams);
    envSTree(c->superTree);

    c->envS = concatEnv(copyEnv(envSDeclParam(c->members)),
			copyEnv(envSMethod(c->methods)));

    envSClass(c->next);

    return c->envS;
  }
  return NULL;
}

EnvP envSDeclParam(DeclParamP d) {
  if(d != NULL) {
    envSTree(d->expression);
    
    EnvP env = NEW(1, Env);
    env->isMethod = 0;
    env->name = d->name;
    env->type = d->type;
    env->next = NULL;
    
    d->envS =  concatEnv(env, copyEnv(envSDeclParam(d->next)));

    return d->envS;
  }
  return NULL;
}

EnvP envSMethod(MethodP m) {
  if(m != NULL) {
    envSDeclParam(m->params);
    envSTree(m->body);
    
    EnvP env = NEW(1, Env);
    env->isMethod = 1;
    env->name = m->name;
    env->type = m->returnType;
    env->next = NULL;
    
    m->envS =  concatEnv(env, copyEnv(envSMethod(m->next)));

    return m->envS;
  }
  return NULL;
}

EnvP envSTree(TreeP t) {
  int i;
  if(t != NULL) {
    switch (t->op) {
    case DECLS:
      t->envS = copyEnv(envSDeclParam(t->u.declParams));
      return t->envS;
    case ID:
    case IDC:
    case CONST:
    case STRG:
    case ENVOI:
      return NULL;      
    case EQ:
    case NE:
    case GT:
    case GE:
    case LT:
    case LE:
    case PLUS:
    case MINUS:
    case MULT:
    case QUO:
    case ITE:
    case CONCAT:
    case THI:
    case AFFECT:
    case BODY:
    case LISTEXP:
    case LIST:
    case SELEC:
    case NEWC:
    case CAST:
    case RET:
    case RES:
    case SUP:
    case EXT:
    case OVRD:
      t->envS = copyEnv(envSTree(getChild(t, 0)));
      for(i = 1; i<t->nbChildren; t++)
	t->envS = concatEnv(t->envS, copyEnv(envSTree(getChild(t, i))));   
      return t->envS;
    case ISBLOC:
      t->envS = copyEnv(envSTree(getChild(t, 0)));
      for(i = 1; i<t->nbChildren; t++)
	t->envS = concatEnv(t->envS, copyEnv(envSTree(getChild(t, i))));
      return NULL;
    default:
      return NULL;      
    }
  }
  return NULL;
}
