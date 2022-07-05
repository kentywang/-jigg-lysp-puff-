#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static Boolean self_evaluating(const Element);
static Boolean variable(const Element);
static Boolean application(const Element);
static Boolean special_form(char *, const Element);

static Element list_of_values(const Element, const Element);
static Element make_procedure(const Element, const Element);
static Element apply(const Element, Pair *);
static Element eval_sequence(const Element, const Element);
static Element eval_definition(const Element, const Element);
static Element eval_if(const Element, const Element);

static Element operator(const Element);
static Element operands(const Element);
static Element procedure_parameters(const Element);
static Element procedure_body(const Element);
static Element procedure_environment(const Element);
static Element text_of_quotation(const Element);
static Element definition_variable(const Element);
static Element definition_value(const Element);

Element eval_dispatch(const Element exp, const Element env)
{
  printf("EVAL:\n");
  print_element(exp);

  Element result;

  if (self_evaluating(exp)) {
    result = exp;
    return result;
  }
  if (variable(exp)) {
    result = lookup_variable_value(exp.contents.symbol, env);
    return result;
  }
  if (special_form(QUOTE, exp)) {
    result = text_of_quotation(exp);
    return result;
  }
  // if (assignment(exp)) {
  //   result = eval_assignment(exp, env);
  //   forget();
  //   return result;
  // }
  if (special_form(DEFINE, exp)) {
    result = eval_definition(exp, env);
    return result;
  }
  if (special_form(IF, exp)) {
    result = eval_if(exp, env);
    return result;
  }
  if (special_form(LAMBDA, exp)) {
    result = make_procedure(exp, env);
    return result;
  }
  // if (cond(exp)) {
  //   result = eval_dispatch(cond_to_if(exp), env);
  //   forget();
  //   return result;
  // }
  // if (and(exp)) {
  //   result = eval_and(exp, env);
  //   forget();
  //   return result;
  // }
  // if (or(exp)) {
  //   result = eval_or(exp, env);
  //   forget();
  //   return result;
  // }
  // if (let(exp)) {
  //   result = eval_dispatch(let_to_combination(exp), env);
  //   forget();
  //   return result;
  // }
  if (application(exp)) {
    result = apply(
      eval_dispatch(operator(exp), env), // procedure
      list_of_values(operands(exp), env).contents.pair_ptr // arguments //k needs exp, env
    );
    return result;
  }

  // TODO: Let print_element print to stderr. 
  fprintf(stderr, "Unknown expression type.\n");
  print_element(exp);
  printf("\n");
  exit(BAD_EXPRESSION);
}

Boolean self_evaluating(const Element exp)
{
  // TODO: Check if string, bools too.
  if (exp.type_tag == NUMBER)
    return TRUE;
  if (exp.type_tag == PAIR && exp.contents.pair_ptr == NULL)
    return TRUE;
  return FALSE;
}

Boolean variable(const Element exp)
{
  return exp.type_tag == SYMBOL;
}

Boolean application(const Element exp)
{
  return exp.type_tag == PAIR;
}

Boolean special_form(char *symbol, const Element exp)
{
  return (
    exp.type_tag == PAIR &&
    car(exp).type_tag == SYMBOL &&
    strcmp(car(exp).contents.symbol, symbol) == 0
  );
}

Element list_of_values(const Element operands, const Element env)
{
  Element e = {
    .type_tag = PAIR,
    .contents.pair_ptr = NULL
  };

  if (!operands.contents.pair_ptr) {
    return e;
  }

  save(&operands);
  save(&env);

  e = make_cons(
    eval_dispatch(car(operands), env),
    list_of_values(cdr(operands), env) //k needs operands, env
  );

  forget();
  forget();

  printf("LIST_OF_VALUES result: ");
  print_element(e);
  printf("\n");
  return e;
}

Element make_procedure(const Element exp, const Element env)
{
  save(&exp);

  Element proc = {
    .type_tag = COMPOUND_PROCEDURE,
    .contents.pair_ptr = make_cons(
      car(cdr(exp)), // lambda parameters
      make_cons(
        cdr(cdr(exp)), // lambda body //k needs exp, env
        env //k needs env
      )
    ).contents.pair_ptr
  };

  forget();

  return proc;
}

Element apply(const Element procedure, Pair *arguments)
{
  if (procedure.type_tag == PRIMITIVE_PROCEDURE)
    // Does this allow null args?
    return (*procedure.contents.func_ptr)(arguments);

  if (procedure.type_tag == COMPOUND_PROCEDURE) {
    Element values = {
      .type_tag = PAIR,
      .contents.pair_ptr = arguments
    };

    printf("proc addr: %p\n", procedure.contents.pair_ptr);
    printf("proc: \n");
    print_element(procedure);
    printf("\n");

    save(&procedure);

    Element env = make_cons(
      make_frame(procedure_parameters(procedure), values),
      procedure_environment(procedure) //k needs procedure
    );

    forget();

    return eval_sequence(
      procedure_body(procedure),
      env
    );
  }

  // Not a procedure. TODO: Print operator.
  fprintf(stderr, "Not a procedure.\n");
  exit(NOT_PROCEDURE);
  return procedure;
}

// The compiler may or may not perform C-level tail-call optimization here.
// But on the Lisp level we can at least reuse the current stack frame by
// popping off the top in preparation for pushing a new frame.
Element eval_sequence(const Element exps, const Element env)
{
  // Check if there more expressions after the head.
  if (cdr(exps).contents.pair_ptr) {
    eval_dispatch(car(exps), env);
    return eval_sequence(cdr(exps), env); //k needs exprs, env
  }

  return eval_dispatch(car(exps), env);
}

Element eval_definition(const Element exp, const Element env)
{
  save(&env);
  save(&exp);

  Element e = make_cons(
    definition_variable(exp),
    eval_dispatch(definition_value(exp), env)
  );

  forget();
  forget();

  // TODO: Add check to see if variable exists in first frame.
  // If we had our own version of set-car!, we could use it here.
  env.contents.pair_ptr->car = make_cons(
    e,
    first_frame(env) //k needs env
  );

  // Return not important.
  Element throwaway = {
    .type_tag = PAIR,
    .contents.pair_ptr = NULL
  };

  return throwaway;
}

Element eval_if(const Element exp, const Element env)
{
  if (is_true(eval_dispatch(car(cdr(exp)), env))) {
    return eval_dispatch(car(cdr(cdr(exp))), env); //k needs exp, env
  }

  return eval_dispatch(car(cdr(cdr(cdr(exp)))), env); //k needs exp, env
}

Element operator(const Element exp)
{
  return car(exp);
}

Element operands(const Element exp)
{
  return cdr(exp);
}

Element text_of_quotation(const Element exp)
{
  return car(cdr(exp));
}

Element procedure_parameters(const Element exp)
{
  return car(exp);
}

Element procedure_body(const Element exp)
{
  return car(cdr(exp));
}

Element procedure_environment(const Element exp)
{
  return cdr(cdr(exp));
}

// Gets the x in (define x 1) 
Element definition_variable(const Element exp)
{
  return car(cdr(exp));
}

// Gets the (+ 1 2) in (define x (+ 1 2)) 
Element definition_value(const Element exp)
{
  return car(cdr(cdr(exp)));
}