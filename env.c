#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static Element load_frame(const Binding *);

static Element binding_name(Element);
static Element binding_value(Element);
static Element first_binding(Element);
static Element rest_bindings(Element);

static Element the_empty_environment = {
  .type = PAIR,
  .data.pair_ptr = NULL
};

Boolean is_empty_environment(const Element env) {
  return env.type == the_empty_environment.type &&
    env.data.pair_ptr == the_empty_environment.data.pair_ptr;
}

// Environment is a list of frame, where each frame is a list of bindings.
Element setup_environment(void) {
  return make_cons(
    load_frame(initial_frame),
    the_empty_environment
  );
}

Binding find_binding(char *var, Element env) {
  Binding b = {
    // This will be our signal to calling function indicating no binding
    // found.
    .variable = NULL
  };

  if (is_empty_environment(env))
    return b;

  do {
    Element curr_frame = first_frame(env);

    // We still have pairs to scan through in frame.
    while (
      curr_frame.data.pair_ptr &&
      strcmp(
        var,
        binding_name(first_binding(curr_frame)).data.symbol
      ) != 0
    ) {
      curr_frame = rest_bindings(curr_frame);
    }

    if (curr_frame.data.pair_ptr) {
      // We exited the while loop because we found the variable.
      b.variable = var; // Use same string allocated as variable in parameter.
      b.value = binding_value(first_binding(curr_frame));
      return b;
    }

    // Otherwise, not in this frame.
  } while (!is_empty_environment(env = enclosing_environment(env)));

  return b;
}

Element lookup_variable_value(char *var, Element env) {
  Binding b = find_binding(var, env);

  if (b.variable)
    return b.value;

  fprintf(stderr, "Unbound variable: %s\n", var);

  // TODO: we shouldn't really return NIL, we should just exit. But to keep the REPL going, we'll try and avoid exiting, so we need to return some value.
  return NIL;
  //exit(UNBOUND_VARIABLE);
}

/*
We'll be generating two pairs at a time, one pair as the backbone of the
list, and the other as the actual variable-and-value pair.

          full env
             /\
 this ______/  \   
 frame     /\   \
          /  \   \
         /\   \   \
        x  1  /\   \
             /\ \   \
            y  2 \   \
                nil  /\
                    /  \
                next    \
                frame   nil
*/
Element load_frame(const Binding *b) {
  Element frame_head = {
    .type = PAIR,
    .data.pair_ptr = NULL
  };

  // We need to handle the first element a bit differently since we set the
  // return element to point to the first backbone.
  if (b->variable) {
    Pair *curr_backbone = get_next_free_ptr();
    Pair *p = get_next_free_ptr();

    frame_head.data.pair_ptr = curr_backbone;

    p->car.type = SYMBOL;
    // We could also copy the string into GCed memory.
    p->car.data.symbol = b->variable;
    p->cdr = b->value;

    // Wrapping Pair pointer in Element is optional, since the default
    // initialization gives it the PAIR type tag.
    curr_backbone->car.data.pair_ptr = p;

    // Similar to above.
    while ((++b)->variable) { // Stop when we encounter END_OF_BINDINGS.
      curr_backbone->cdr.data.pair_ptr = get_next_free_ptr();
      curr_backbone = curr_backbone->cdr.data.pair_ptr;

      p = get_next_free_ptr();
      p->car.type = SYMBOL;
      p->car.data.symbol = b->variable;
      p->cdr = b->value;

      curr_backbone->car.data.pair_ptr = p;
    }
  }

  return frame_head;
}

// (a b c) (1 2 3) => ((a . 1) (b . 2) (c . 3))
Element make_frame(const Element vars, const Element vals) {
  // If only one of vars or vals is null, we have a parameter-argument
  // mismatch. Hopefully !s works.
  // TODO: List procedure name, number of args vs. parameters
  if (!vars.data.pair_ptr != !vals.data.pair_ptr) {
    fprintf(stderr, "Arity mismatch during make_frame.\n");
    exit(ARITY_MISMATCH);
  }

  // Both empty.
  if (!vars.data.pair_ptr && !vals.data.pair_ptr) {
    Element empty_frame = {
      .type = PAIR,
      .data.pair_ptr = NULL
    };

    return empty_frame;
  }

  save(vars);
  save(vals);

  Element first_binding = make_cons(car(vars), car(vals));
  save(first_binding);

  Element rest = make_frame(cdr(vars), cdr(vals));
  release(3);

  return make_cons(first_binding, rest);
}

Element first_frame(const Element env) { return car(env); }

Element enclosing_environment(const Element env) { return cdr(env); }

Element first_binding(const Element frame) { return car(frame); }
Element rest_bindings(const Element frame) { return cdr(frame); }
Element binding_name(const Element binding) { return car(binding); }
Element binding_value(const Element binding) { return cdr(binding); }
