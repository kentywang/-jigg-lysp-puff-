#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static Element the_empty_environment = {
  .type_tag = PAIR,
  .contents.pair_ptr = NULL
};

// Multivariable frame of ((+ add) (- sub)).
//        0   1   2   3   4
// cars [ p2, - , p3, + , p1]
// cdrs [ e0,sub, p4,add, e0] cdr at [0] is pointer to next frame.

Boolean is_empty_environment(const Element env)
{
  return env.type_tag == the_empty_environment.type_tag &&
    env.contents.pair_ptr == the_empty_environment.contents.pair_ptr;
}

Element setup_environment(void)
{
  return extend_environment(
    load_frame(initial_frame),
    the_empty_environment
  );
}

Element extend_environment(const Element frame, const Element base_env)
{
  return make_cons(frame, base_env);
}

Binding find_binding(char *var, Element env)
{
  Binding b = {
    // This will be our signal to calling function indicating no binding
    // found.
    .variable = NULL
  };

  if (is_empty_environment(env))
    return b;

  do {
    Element frame_scanner = first_frame(env);

    // We still have pairs to scan through in frame.
    while (frame_scanner.contents.pair_ptr && strcmp(var, car(car(frame_scanner)).contents.symbol) != 0) {
      frame_scanner = cdr(frame_scanner);
    }

    if (frame_scanner.contents.pair_ptr) {
      // We exited the while loop because we found the variable.
      b.variable = var; // Use same string allocated as variable in parameter.
      b.value = cdr(car(frame_scanner));
      return b;
    }

    // Otherwise, not in this frame.
  } while (!is_empty_environment(env = enclosing_environment(env)));

  return b;
}

Element lookup_variable_value(char *var, Element env)
{
  Binding b = find_binding(var, env);

  if (b.variable)
    return b.value;

  fprintf(stderr, "Unbound variable: %s\n", var);
  exit(UNBOUND_VARIABLE);
}

// (a b c) (1 2 3) => ((a . 1) (b . 2) (c . 3))
Element make_frame(const Element bindings, const Element values)
{
  // If only one of bindings or values is null, we have a parameter-argument
  // mismatch. Hopefully !s works.
  // TODO: List procedure name, number of args vs. parameters
  if (!bindings.contents.pair_ptr != !values.contents.pair_ptr) {
    fprintf(stderr, "Arity mismatch.\n");
    exit(ARITY_MISMATCH);
  }

  // Both empty.
  if (!bindings.contents.pair_ptr && !values.contents.pair_ptr) {
    Element e = {
      .type_tag = PAIR,
      .contents.pair_ptr = NULL
    };

    return e;
  }

  // Still have parameters and arguments.
  return make_cons(
    make_cons(car(bindings), car(values)),
    make_frame(cdr(bindings), cdr(values))
  );
}

Element first_frame(const Element env)
{
  return car(env);
}

Element enclosing_environment(const Element env)
{
  return cdr(env);
}