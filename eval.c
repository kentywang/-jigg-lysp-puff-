#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static Boolean self_evaluating(const Element);
static Boolean variable(const Element);
static Boolean application(const Element);
static Boolean lambda(const Element);
static Boolean quoted(const Element);

static Element make_procedure(const Element, const Element);
static Element apply(const Element, const Element);
static Element list_of_values(const Element, const Element);
static Element make_procedure(const Element, const Element);
static Element apply_compound(const Element, Pair *);
static Element procedure_parameters(const Element);
static Element procedure_body(const Element);
static Element procedure_environment(const Element);
static Element eval_sequence(const Element, const Element);
static Element text_of_quotation(const Element);

Element eval_dispatch(const Element exp, const Element env)
{
  if (self_evaluating(exp))
    return exp;
  if (variable(exp))
    return lookup_variable_value(exp.contents.symbol, env);
  if (quoted(exp))
    return text_of_quotation(exp);
  // if (assignment(exp))
  //   return eval_assignment(exp, env);
  // if (definition(exp))
  //   return eval_definition(exp, env);
  // if (if_expression(exp))
  //   return eval_if(exp, env);
  if (lambda(exp))
    return make_procedure(exp, env);
  // if (cond(exp))
  //   return eval_dispatch(cond_to_if(exp), env);
  // if (and(exp))
  //   return eval_and(exp, env);
  // if (or(exp))
  //   return eval_or(exp, env);
  // if (let(exp))
  //   return eval_dispatch(let_to_combination(exp), env);
  if (application(exp))
    return apply(exp, env);

  // TODO: Let print_element print to stderr. 
  fprintf(stderr, "Unknown expression type.\n");
  exit(BAD_EXPRESSION);
}

Boolean self_evaluating(const Element exp)
{
  // TODO: Check if string too.
  return exp.type_tag == NUMBER;
}

Boolean variable(const Element exp)
{
  return exp.type_tag == SYMBOL;
}

Boolean application(const Element exp)
{
  return exp.type_tag == PAIR;
}

Boolean lambda(const Element exp)
{
  return (
    exp.type_tag == PAIR &&
    car(exp).type_tag == SYMBOL &&
    strcmp(car(exp).contents.symbol, "lambda") == 0
  );
}

Boolean quoted(const Element exp)
{
  return (
    exp.type_tag == PAIR &&
    car(exp).type_tag == SYMBOL &&
    strcmp(car(exp).contents.symbol, "quote") == 0
  );
}

Element apply(const Element exp, const Element env)
{
  Element (*operator)(const Element) = &car;
  Element (*operands)(const Element) = &cdr;

  Element procedure = eval_dispatch((*operator)(exp), env);
  Pair *arguments = list_of_values((*operands)(exp), env).contents.pair_ptr;

  if (procedure.type_tag == PRIMITIVE_PROCEDURE)
    // Does this allow null args?
    return (*procedure.contents.func_ptr)(arguments);
  else if (procedure.type_tag == COMPOUND_PROCEDURE)
    return apply_compound(procedure, arguments);

  // Not a procedure. TODO: Print operator.
  fprintf(stderr, "Not a procedure.\n");
  exit(NOT_PROCEDURE);
  return procedure;
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

  // Instead of defer argument evaluation order to the C compiled to decide,
  // we can choose to explicitly set the order to left-to-right.
  e = eval_dispatch(car(operands), env);

  return make_cons(e, list_of_values(cdr(operands), env));
}

Element make_procedure(const Element exp, const Element env)
{
  Element e = {
    .type_tag = COMPOUND_PROCEDURE,
    .contents.pair_ptr = make_cons(
      car(cdr(exp)), // Lambda parameters
      make_cons(
        cdr(cdr(exp)), // Lambda body
        env
      )
    ).contents.pair_ptr
  };

  return e;
}

Element apply_compound(const Element procedure, Pair *arguments)
{
  Element e = {
    .type_tag = PAIR,
    .contents.pair_ptr = arguments
  };

  return eval_sequence(
    procedure_body(procedure),
    extend_environment(
      make_frame(procedure_parameters(procedure), e),
      procedure_environment(procedure)
    )
  );
}

// The compiler may perform tail-call optimization here. But we should
// rework evaluation into a giant while loop with a stack to explicitly
// control it.
Element eval_sequence(const Element exps, const Element env)
{
  // Are there more expressions after the head?
  if (cdr(exps).contents.pair_ptr) {
    eval_dispatch(car(exps), env);
    return eval_sequence(cdr(exps), env);
  }

  return eval_dispatch(car(exps), env);
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