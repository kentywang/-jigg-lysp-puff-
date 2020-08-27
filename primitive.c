#include <stdlib.h>
#include "lisp.h"

#define END_OF_BINDINGS 0

static Element add(const Pair *);
static Element multiply(const Pair *);
static Element equals(const Pair *);

Binding initial_frame[] = {
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
    "=", {
      .type_tag = PRIMITIVE_PROCEDURE,
      .contents.func_ptr = &equals
    }
  },
  {
    "cons", {
      .type_tag = PRIMITIVE_PROCEDURE,
      .contents.func_ptr = &make_cons // TODO: Why is this broken?
    }
  },
  {
    "list", {
      .type_tag = PRIMITIVE_PROCEDURE,
      .contents.func_ptr = &make_list
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

Element add(const Pair *p)
{
  // Need to check if car is actually a number!
  Element e = {
    .type_tag = NUMBER,
    .contents = p->car.contents
  };

  // Add result of adding up cdr to car, if cdr isn't null.
  if (!(p->cdr.type_tag == PAIR && !p->cdr.contents.pair_ptr))
    e.contents.number += add(p->cdr.contents.pair_ptr).contents.number;

  return e;
}

Element multiply(const Pair *p)
{
  // Need to check if car is actually a number!
  Element e = {
    .type_tag = NUMBER,
    .contents = p->car.contents
  };

  // Add result of multiplying cdr to product car, if cdr isn't null.
  if (!(p->cdr.type_tag == PAIR && !p->cdr.contents.pair_ptr))
    e.contents.number *= multiply(p->cdr.contents.pair_ptr).contents.number;

  return e;
}

Element equals(const Pair *p)
{
  // Need to check if car is actually a number!
  Element e = {
    .type_tag = BOOLEAN,
    .contents.truth = FALSE
  };

  if (
    p->car.type_tag == NUMBER &&
    p->cdr.type_tag == PAIR &&
    p->cdr.contents.pair_ptr->car.type_tag == NUMBER &&
    p->car.contents.number == p->cdr.contents.pair_ptr->car.contents.number
  )
    e.contents.truth = TRUE;

  return e;
}