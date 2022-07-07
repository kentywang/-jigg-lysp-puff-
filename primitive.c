#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#define END_OF_BINDINGS 0

static Element add(const Pair *);
static Element multiply(const Pair *);
static Element equals(const Pair *);
static Element primitive_make_cons(const Pair *);
static Element primitive_car(const Pair *);
static Element primitive_cdr(const Pair *);
static Element make_list(const Pair *);

Element NIL = {
  .type = PAIR,
  .data.pair_ptr = NULL
};

Binding initial_frame[] = {
  {
    "+", {
      .type = PRIMITIVE_PROCEDURE,
      .data.func_ptr = &add
    }
  },
  {
    "*", {
      .type = PRIMITIVE_PROCEDURE,
      .data.func_ptr = &multiply
    }
  },
  {
    "=", {
      .type = PRIMITIVE_PROCEDURE,
      .data.func_ptr = &equals
    }
  },
  {
    "cons", {
      .type = PRIMITIVE_PROCEDURE,
      .data.func_ptr = &primitive_make_cons
    }
  },
  {
    "car", {
      .type = PRIMITIVE_PROCEDURE,
      .data.func_ptr = &primitive_car
    }
  },
  {
    "cdr", {
      .type = PRIMITIVE_PROCEDURE,
      .data.func_ptr = &primitive_cdr
    }
  },
  {
    "list", {
      .type = PRIMITIVE_PROCEDURE,
      .data.func_ptr = &make_list
    }
  },
  {
    "nil", {  // TODO: reuse NIL constant here, somehow
      .type = PAIR,
      .data.pair_ptr = NULL
    }
  },
  {
    END_OF_BINDINGS // Sentinel value.
  }
};

Element add(const Pair *p) {
  // Need to check if car is actually a number!
  Element e = {
    .type = NUMBER,
    .data = p->car.data
  };

  // Add result of adding up cdr to car, if cdr isn't null.
  if (!(p->cdr.type == PAIR && !p->cdr.data.pair_ptr))
    e.data.number += add(p->cdr.data.pair_ptr).data.number;

  return e;
}

Element multiply(const Pair *p) {
  // Need to check if car is actually a number!
  Element e = {
    .type = NUMBER,
    .data = p->car.data
  };

  // Add result of multiplying cdr to product car, if cdr isn't null.
  if (!(p->cdr.type == PAIR && !p->cdr.data.pair_ptr))
    e.data.number *= multiply(p->cdr.data.pair_ptr).data.number;

  return e;
}

Element equals(const Pair *p) {
  // Need to check if car is actually a number!
  Element e = {
    .type = BOOLEAN,
    .data.truth = FALSE
  };

  if (
    p->car.type == NUMBER &&
    p->cdr.type == PAIR &&
    p->cdr.data.pair_ptr->car.type == NUMBER &&
    p->car.data.number == p->cdr.data.pair_ptr->car.data.number
  )
    e.data.truth = TRUE;

  return e;
}

// This is different from the make_cons function that is used internally.
// TODO: Can we just use one function for both purposes?
Element primitive_make_cons(const Pair *p) {
  if (
    // If only one argument...
    (
      p->cdr.type == PAIR &&
      !p->cdr.data.pair_ptr
    ) ||
    // Or if more than two arguments...
    (
      p->cdr.type == PAIR &&
      p->cdr.data.pair_ptr->cdr.type == PAIR &&
      p->cdr.data.pair_ptr->cdr.data.pair_ptr
    )
  ) {
    fprintf(stderr, "Arity mismatch.\n");
    exit(ARITY_MISMATCH);
  }

  return make_cons(p->car, p->cdr.data.pair_ptr->car);
}

/*
How a pair (1 . 2) looks like:
    p
   / \
  /\  null
 1  2
*/
Element primitive_car(const Pair *p) {
  return p->car.data.pair_ptr->car;
}

Element primitive_cdr(const Pair *p) {
  return p->car.data.pair_ptr->cdr;
}

/*
I think we want to treat the parsed input as a regular AST.

  p
 /\
2 /\
 5 /\
  8  null

Since arguments are already in a list, let's make use of them when
applying primitive procedures.

Note this is essentially call-by-reference since the argument list points
to an existing list in memory (We do this to allow for easy passing of
variable argument lengths to functions). But we musn't mutate the arg.

C doesn't allow variable-argument functions to be called with no arguments,
but I don't see a use case for it here, so I'm won't try adding support
for it.

We use Pair arg type because this is a purely internal-usage function
that we can guarantee is always a proper list.

Even if it's just one element like (list 1), the arg list will be (1) and
thus always has a car and cdr.
*/
Element make_list(const Pair *p) {
  if (p->cdr.type == PAIR && !p->cdr.data.pair_ptr) {
    Element e = {
      .type = PAIR,
      .data.pair_ptr = NULL
    };

    // Any empty list is interchangeable with another.
    return make_cons(p->car, e);
  }

  return make_cons(p->car, make_list(p->cdr.data.pair_ptr));
}
