#include <stdlib.h>
#include "lisp.h"

#define END_OF_BINDINGS 0

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
    "x", {
      .type_tag = NUMBER,
      .contents.number = 322
    }
  },
  {
    END_OF_BINDINGS // Sentinel value.
  }
};

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

  // Add result of adding up cdr to car, if cdr isn't null.
  if (!(p->cdr.type_tag == PAIR && !p->cdr.contents.pair_ptr))
    e.contents.number *= multiply(p->cdr.contents.pair_ptr).contents.number;

  return e;
}
