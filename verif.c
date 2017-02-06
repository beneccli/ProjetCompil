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
void print_env (EnvP env) {
  if (env != NULL) {
    printf("%s \n",env->name);
    print_env(env->next);
  }
}

void print_class(ClassP lc) {
  if (lc != NULL) {
    print_class(lc->next);
    printf("Classe %s:\n",lc->name);
    print_env(lc->env);
  }
}


bool verif_portee (ClassP lc) {
  fill_env(lc);
  print_class(lc);
  return TRUE;
}

void copy_param(DeclParamP vars,ClassP lc) {
  if (vars != NULL) {
    EnvP e =  NEW(1, Env);  
    e->name = vars->name;
    e->type = vars->type;
    if(lc->env != NULL)	  
      e->next = lc->env;
    lc->env = e;
    copy_param(vars->next,lc);
  }
}

void copy_env(EnvP super, ClassP lc) {
  if (super != NULL) {
    EnvP e = NEW(1, Env);
    e->name = super->name;
    e->type = super->type;
    if(lc->env != NULL)	  
      e->next = lc->env;
    lc->env = e;
    copy_env(super->next,lc);
  }
}

void copy_expr(TreeP expr, ClassP lc) {
  if (expr != NULL) {
    int i = 0;
    if (expr->op == ID) {
      EnvP e = NEW(1,Env);
      e->name = expr->u.str;
      if(lc->env != NULL)	  
        e->next = lc->env;
      lc->env = e;
    }
    else if (expr->op == DECLS) {
      DeclParamP d = expr->u.declParams;
      while (d != NULL) {
        EnvP e = NEW(1,Env);
        e->name = d->name;
        e->type = d->type;
        if(lc->env != NULL)	  
          e->next = lc->env;
        lc->env = e;
        d = d->next;
      }
    }
    for (i = 0; i < expr->nbChildren ; i++)
      copy_expr(getChild(expr,i),lc);
  } 
}

void fill_env(ClassP lc) {
  if (lc != NULL) {
    fill_env(lc->next);
    if (lc->super != NULL) {
      copy_env(lc->super->env,lc);
      copy_expr(getChild(lc->superTree,1),lc);
    }
    copy_param(lc->constructorParams, lc);
    copy_param(lc->members, lc);
    copy_expr(lc->constructorBody,lc);
    MethodP tmp = lc->methods;
    while(tmp != NULL) {
      EnvP e = NEW(1,Env);
      e->name = tmp->name;
      e->type = tmp->returnType;
      if(lc->env != NULL)	  
        e->next = lc->env;
      lc->env = e;
      tmp = tmp->next;
    }
  }
}
