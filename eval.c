#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static Element list_of_values(Element, Element);
static Element make_procedure(Element, Element);
static Element apply(Element, Pair *);
static Element eval_sequence(Element, Element);
static Element eval_definition(Element, Element);
static Element eval_if(Element, Element);

static Boolean self_evaluating(Element);
static Boolean variable(Element);
static Boolean application(Element);
static Boolean special_form(char *, Element);

// Abstractions around car/cdr
static Element operator(Element);
static Element operands(Element);
static Element procedure_parameters(Element);
static Element procedure_body(Element);
static Element procedure_environment(Element);
static Element text_of_quotation(Element);
static Element definition_variable(Element);
static Element definition_expression(Element);
static Element predicate(Element);
static Element consequent(Element);
static Element alternative(Element);

Element eval_dispatch(const Element exp, const Element env) {
  // These cases don't allocate memory, so GC won't get triggered here, so
  // we don't need to save pointers.
  if (self_evaluating(exp))
    return exp;

  if (variable(exp))
    return lookup_variable_value(exp.data.symbol, env);

  if (special_form(QUOTE, exp))
    return text_of_quotation(exp);

  // The rest do though.
  save(exp);
  save(env);

  if (special_form(DEFINE, exp)) {
    Element result = eval_definition(exp, env);
    release(2);
    return result;

  } else if (special_form(IF, exp)) {
    release(2);  // let eval_if itself handle stack for tail call recursion.
    Element result = eval_if(exp, env);
    return result;

  } else if (special_form(LAMBDA, exp)) {
    Element result = make_procedure(exp, env);
    release(2);
    return result;

  } else if (application(exp)) {
    Element proc = eval_dispatch(operator(exp), env);
    save(proc);

    Pair *args = list_of_values(operands(exp), env).data.pair_ptr;
    release(3);

    return apply(proc, args);
  }
  // TODO: Add cond, and, or, let, assignment cases
  // Warning, we aren't release memory if these above cases aren't true.
  // Currently fine since we error out here.

  // TODO: Let print_element print to stderr.
  fprintf(stderr, "Unknown expression type.\n");
  exit(BAD_EXPRESSION);
}

Element list_of_values(const Element operands, const Element env) {
  if (!operands.data.pair_ptr) {
    return (Element){
      .type = PAIR,
      .data.pair_ptr = NULL
    };
  }

  Element first_value = eval_dispatch(car(operands), env);
  save(first_value);

  Element rest = list_of_values(cdr(operands), env);
  release(1);

  return make_cons(first_value, rest);
}

Element apply(const Element procedure, Pair *arguments) {
  if (procedure.type == PRIMITIVE_PROCEDURE)
    // TODO: Does this allow null args?
    return (*procedure.data.func_ptr)(arguments);

  if (procedure.type == COMPOUND_PROCEDURE) {
    Element params = procedure_parameters(procedure);
    Element rest = procedure_environment(procedure);
    Element proc_body = procedure_body(procedure);
    save(procedure);

    Element first_frame = make_frame(
      params,
      (Element){
        .type = PAIR,
        .data.pair_ptr = arguments
      }
    );

    Element env = make_cons(first_frame, rest);
    release(1);

    return eval_sequence(proc_body, env);
  }

  // Not a procedure. TODO: Print operator.
  fprintf(stderr, "Not a procedure.\n");
  exit(NOT_PROCEDURE);
  return procedure;
}

// The compiler may or may not perform C-level tail-call optimization,
// But on the Lisp level we can at least reuse the current stack frame by
// popping off the top in preparation for pushing a new frame.
Element eval_sequence(const Element exps, const Element env) {
  // Check if there's more expressions after the head.
  if (cdr(exps).data.pair_ptr) {
    save(exps);
    save(env);

    eval_dispatch(car(exps), env);
    release(2);

    return eval_sequence(cdr(exps), env);
  }

  return eval_dispatch(car(exps), env);
}

Element eval_if(const Element exp, const Element env) {
  save(exp);
  save(env);

  Element conditional = eval_dispatch(predicate(exp), env);
  release(2);

  return (
    is_true(conditional) ?
    eval_dispatch(consequent(exp), env) :
    eval_dispatch(alternative(exp), env)
  );
}

// Modifies the environment to insert a new binding.
Element eval_definition(const Element exp, const Element env) {
  Element def_name = definition_variable(exp);
  Element def_exp = definition_expression(exp);
  Element def_val = eval_dispatch(def_exp, env);

  Element binding = make_cons(def_name, def_val);

  // TODO: Add check to see if variable exists in first frame.
  // If we had our own version of set-car!, we could use it here.
  env.data.pair_ptr->car = make_cons(binding, first_frame(env));

  return NIL;
}

Element make_procedure(const Element exp, const Element env) {
  return (Element){
    .type = COMPOUND_PROCEDURE,
    .data.pair_ptr = make_cons(
      car(cdr(exp)),  // lambda parameters
      make_cons(cdr(cdr(exp)), env  // lambda body and env
      )
    ).data.pair_ptr
  };
}

Boolean self_evaluating(const Element exp) {
  // TODO: Check if string, bools too.
  return (
    exp.type == NUMBER ? TRUE :
    exp.type == PAIR && exp.data.pair_ptr == NULL ? TRUE :
    FALSE
  );
}

Boolean variable(const Element exp) {
  return exp.type == SYMBOL;
}

Boolean application(const Element exp) {
  return exp.type == PAIR;
}

Boolean special_form(char *symbol, const Element exp) {
  return (
    exp.type == PAIR &&
    car(exp).type == SYMBOL &&
    strcmp(car(exp).data.symbol, symbol) == 0
  );
}

Element operator(const Element exp) { return car(exp); }
Element operands(const Element exp) { return cdr(exp); }

// Gets the abc in (quote abc)
Element text_of_quotation(const Element exp) { return car(cdr(exp)); }

// Here, exp looks like this: ((x) (+ x 1) env)
Element procedure_parameters(const Element exp) { return car(exp); }
Element procedure_body(const Element exp) { return car(cdr(exp)); }
Element procedure_environment(const Element exp) { return cdr(cdr(exp)); }

// Gets the x or (+ 1 2) in (define x (+ 1 2))
Element definition_variable(const Element exp) { return car(cdr(exp)); }
Element definition_expression(const Element exp) { return car(cdr(cdr(exp))); }

// Gets the p/c/a in (if p c a)
Element predicate(const Element exp) { return car(cdr(exp)); }
Element consequent(const Element exp) { return car(cdr(cdr(exp))); }
Element alternative(const Element exp) { return car(cdr(cdr(cdr(exp)))); }
