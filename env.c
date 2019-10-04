#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

#define END_OF_BINDINGS 0

static Binding initial_frame[] = {
  {
    "+", {
      .type_tag = PRIMITIVE_PROCEDURE,
      .contents.func_ptr = &add
    }
  },
  {
    "*", {
      .type_tag = PRIMITIVE_PROCEDURE,
      .contents.func_ptr = &multiply
    }
  },
  {
    "x", {
      .type_tag = NUMBER,
      .contents.number = 322
    }
  },
  {
    END_OF_BINDINGS // Sentinel value.
  }
};

static Element load_frame(const Binding *);
static Element first_frame(const Element);
static Element enclosing_environment(const Element);

static Element the_empty_environment = {
  .type_tag = PAIR,
  .contents.pair_ptr = NULL
};

Boolean is_empty_environment(const Element env)
{
  return env.type_tag == the_empty_environment.type_tag &&
    env.contents.pair_ptr == the_empty_environment.contents.pair_ptr;
}

Element setup_environment(void)
{
  return extend_environment(load_frame(initial_frame), the_empty_environment);
}

/*
We'll be generating two pairs at a time, one pair as the backbone of the
list, and the other as the actual variable-and-value pair.
  p
 /\
2 /\
 5 /\
  8  null
*/
Element load_frame(const Binding *b)
{
  Element frame_head = {
    .type_tag = PAIR,
    .contents.pair_ptr = NULL
  };

  // We need to handle the first element a bit differently since we set the
  // return element to point to the first backbone.
  if (b->variable) {
    Pair *curr_backbone = get_next_free_ptr();
    Pair *p = get_next_free_ptr();

    frame_head.contents.pair_ptr = curr_backbone;

    p->car.type_tag = SYMBOL;
    // We could also copy the string into GCed memory.
    p->car.contents.symbol = b->variable;
    p->cdr = b->value;

    // Wrapping Pair pointer in Element is optional, since the default
    // initialization gives it the PAIR type tag.
    curr_backbone->car.contents.pair_ptr = p;

    // Similar to above.
    while ((++b)->variable) { // Stop when we encounter END_OF_BINDINGS.
      curr_backbone->cdr.contents.pair_ptr = get_next_free_ptr();
      curr_backbone = curr_backbone->cdr.contents.pair_ptr;

      p = get_next_free_ptr();
      p->car.type_tag = SYMBOL;
      p->car.contents.symbol = b->variable;
      p->cdr = b->value;

      curr_backbone->car.contents.pair_ptr = p;
    }
  }

  return frame_head;
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

Element first_frame(const Element env)
{
  return car(env);
}

Element enclosing_environment(const Element env)
{
  return cdr(env);
}
// (define (find-binding var env)
//   (define (scan vars vals)
//     (cond ((null? vars)
//            (find-binding var (enclosing-environment env)))
//           ((eq? var (car vars))
//            (cons vars vals))
//           (else (scan (cdr vars)
//                       (cdr vals)))))
//   (if (eq? env the-empty-environment)
//       false
//       (let ((frame (first-frame env)))
//         (scan (frame-variables frame)
//               (frame-values frame)))))

// (foo 123) is also a valid variable-value combo.

// Rudimentary env (just add operator) at index 0.
//        0   1   2   3   4
// cars [ p3,   ,   , + ,   ]
// cdrs [ e0,   ,   ,add,   ]

// Multivariable frame of ((+ add) (- sub)).
//        0   1   2   3   4
// cars [ p2, - , p3, + , p1]
// cdrs [ e0,sub, p4,add, e0] cdr at [0] is pointer to next frame.
