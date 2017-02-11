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
  if(e2 == NULL)
    return e1;
  EnvP itEnv = e1;
  while(itEnv->next != NULL)
    itEnv = itEnv->next;
  itEnv->next = e2;
  return e1;
}

bool verif_scope(ClassP c, TreeP main) {
  envSClass(c);
  envSTree(main);
  envClass(c);
  envClassProp(c);
  envTree(main, NULL);

  return TRUE;
}

void envClass(ClassP c) {
  if(c != NULL && c->envSet == 0) {
    envClass2(c);
    envClass(c->next);
  }
}

void envClass2(ClassP c) {
  if(c->super != NULL) {
    if(c->super->envSet == 0)
      envClass2(c->super);

    c->env = concatEnv(copyEnv(c->super->envS), copyEnv(c->super->env));
  }

  c->envSet = 1;
}

void envClassProp(ClassP c) {
  if(c != NULL) {
    EnvP env = concatEnv(copyEnv(c->envS), copyEnv(c->env));
    EnvP this = NEW(1, Env);
    this->name = "this";
    this->type = c;
    this->isMethod = 0;
    env = concatEnv(this, env);
    if(c->super) {
      EnvP super = NEW(1, Env);
      super->name = "super";
      super->type = c->super;
      super->isMethod = 0;
      env = concatEnv(super, env); 
    }
    if(c->constructorParams != NULL)
	envTree(c->constructorBody, concatEnv(copyEnv(c->constructorParams->envS), env));
    else
	envTree(c->constructorBody, env);
    envMethods(c->methods, env);
    envDeclParam(c->members, c->env);
    if(c->constructorParams != NULL)
	envTree(c->superTree, copyEnv(c->constructorParams->envS));
    else
	envTree(c->superTree, NULL);
    envClassProp(c->next);
  }
}

void envMethods(MethodP m, EnvP envH) {
  if(m != NULL) {
    m->env = envH;
    EnvP env;
    if(m->params)
      env = concatEnv(copyEnv(m->params->envS), copyEnv(m->env));
    else
      env = copyEnv(m->env);
    if(m->returnType) {
      EnvP result = NEW(1, Env);
      result->name = "result";
      result->type = m->returnType;
      result->isMethod = 0;
      env = concatEnv(result, env);
    }
    envTree(m->body, env);

    envMethods(m->next, m->env);
  }  
}

void envDeclParam(DeclParamP d, EnvP envH) {
  if(d != NULL) {
    if(d->decl == 1) {
      d->env = envH;
      envTree(d->expression, copyEnv(d->env));
    }

    envDeclParam(d->next, concatEnv(copyEnv(d->envS), copyEnv(d->env)));
  }
}

EnvP inEnv(EnvP env, char* name) {
  if(env != NULL && env->name != NULL) {
    if(!strcmp(env->name, name) && env->isMethod == 0)
      return env;
    return inEnv(env->next, name);
  }
  return NULL;
}

EnvP inEnvMethod(EnvP env, char* name) {
  if(env != NULL && env->name != NULL) {
    if(!strcmp(env->name, name) && env->isMethod == 1)
      return env;
    return inEnvMethod(env->next, name);
  }
  return NULL;
}

void envTree(TreeP t, EnvP envH) {
  if(t != NULL) {
    envSExpr(t, envH);
    envTree2(t);
  }
}

void envTree2(TreeP t) {
  if(t != NULL) {
    int i;
    ClassP type = NULL;
    switch (t->op) {
    case DECLS:
      envDeclParam(t->u.declParams, t->env);
      break;
    case ID:      
      if(t->isCallMethod == 1 && !inEnvMethod(t->env, t->u.str))
	printf("Not in env func : %s\n", t->u.str);
      if(t->isCallMethod == 0 && !inEnv(t->env, t->u.str))
	printf("Not in env : %s\n", t->u.str);
      break;
    case THI:
      if(!inEnv(t->env, "this"))
	printf("Not in env : this\n");
      break;
    case SUP:
      if(!inEnv(t->env, "super"))
	printf("Not in env : super\n");
      break;
    case RES:
      if(!inEnv(t->env, "result"))
	printf("Not in env : result\n");
      break;
    case CAST:
    case NEWC:
      type = getChild(t, 0)->idc;
      if(type == NULL) {
	printf("Error cast or new\n");
	break;
      }
      envTree2(getChild(t, 1));
      break;    
    default:
      for(i = 0; i<t->nbChildren; i++)
	envTree2(getChild(t, i));
      break;
    }
  }
}

EnvP envSExpr(TreeP t, EnvP envH) {
  if(t == NULL)
    return NULL;
  t->env = envH;
  ClassP type = NULL;
  if(t->op == ID) {
    char* nameType = t->u.str;
    EnvP env;
    if(t->isCallMethod == 1)
      env = inEnvMethod(t->env, nameType);
    else
      env = inEnv(t->env, nameType);
    if(env)
      type = env->type;
    if(type == NULL)
      return NULL;
    t->envS = concatEnv(copyEnv(type->envS), copyEnv(type->env));
    return t->envS;
  }
  else if(t->op == STRG) {
    ClassP type = getClass(listClass, "String");
    t->envS = copyEnv(type->envS);
    return t->envS;
  }
  else if(t->op == THI) {
    EnvP env = inEnv(t->env, "this");    
    ClassP type = NULL;
    if(env)
      type = env->type;
    if(!type)
      return NULL;
    t->envS = concatEnv(copyEnv(type->envS), copyEnv(type->env));
    return t->envS;
  }
  else if(t->op == SUP) {
    EnvP env = inEnv(t->env, "super");   
    ClassP type = NULL;
    if(env)
      type = env->type;
    if(!type)
      return NULL;
    t->envS = concatEnv(copyEnv(type->envS), copyEnv(type->env));
    return t->envS;
  }
  else if(t->op == SELEC || t->op == ENVOI) {    
    t->envS = copyEnv(envSExpr(getChild(t, 0), copyEnv(t->env)));
    if(t->op == ENVOI)
      envSExpr(getChild(t, 2), copyEnv(t->env));
    return envSExpr(getChild(t, 1), copyEnv(t->envS));
  }
  else if(t->op == CAST || t->op == NEWC) {
    type = getChild(t, 0)->idc;
    if(type == NULL) {
      return NULL;
    }
    envSExpr(getChild(t, 1), copyEnv(t->env));
    t->envS = concatEnv(copyEnv(type->envS), copyEnv(type->env));
    return t->envS;
  }
  else if(t->op == CONCAT) {
    ClassP type = getClass(listClass, "String");
    t->envS = copyEnv(type->envS);
    envSExpr(getChild(t, 0), copyEnv(t->env));
    envSExpr(getChild(t, 1), copyEnv(t->env));
    return t->envS;
  }
  else {
    int i;
    for(i = 0; i < t->nbChildren; i++) {
      envSExpr(getChild(t, i), concatEnv(copyEnv(t->envS), copyEnv(t->env)));
    }
    return NULL;
  }
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
      return t->envS;;          
    case ITE:
    case BODY:
    case LIST:
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
