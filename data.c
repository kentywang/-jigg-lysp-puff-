#include <string.h>
#include "lisp.h"

static Element clone(const Element);

/*
Should return Element for closure property.

If the elements to cons are pairs, we want to the cons to hold their
references so if their contents change, the cons does too. But if the
elements are atoms (strings, symbols, numbers, boolean), we want a copy
of them.
*/
Element make_cons(const Element x, const Element y)
{
  Element e;

  Pair *p = get_next_free_ptr();

  p->car = clone(x);
  p->cdr = clone(y);

  e.type_tag = PAIR;
  e.contents.pair_ptr = p;

  return e;
}

Element clone(const Element x)
{
  Element y = {
    .type_tag = PAIR,
    .contents.pair_ptr = NULL
  };

  switch (x.type_tag) {
  // C's pass-by-value ensures numbers are always copied.
  case NUMBER:
  case BOOLEAN:
  // We copy the reference, which is what we want with non-atomic elements.
  case PAIR:
  // Primitives procedures are stored by the address of their functions.
  case PRIMITIVE_PROCEDURE:
  // Compounds procedures are same as Pairs.
  case COMPOUND_PROCEDURE:
    return x;
  // Since we store strings and symbol elements as pointers to manually
  // allocated memory, in order to truly copy them, we'll need to allocate
  // new memory.
  case SYMBOL:
    y.type_tag = x.type_tag;
    y.contents.symbol = string_alloc(strlen(x.contents.symbol));
    strcpy(y.contents.symbol, x.contents.symbol);
    return y;
  }
}

Element car(const Element x)
{
  // Add check?
  return x.contents.pair_ptr->car;
}

Element cdr(const Element x)
{
  return x.contents.pair_ptr->cdr;
}

Boolean is_true(const Element x)
{
  // Anything but a strict true is false.
  return x.type_tag == BOOLEAN && x.contents.truth;
}