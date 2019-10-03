#include <stdlib.h>
#include <string.h>
#include "lisp.h"

#define END_OF_BINDINGS 0

typedef struct binding Binding;

/*
Possible types for value:
- number
- pointer to pair
- symbol/string
- primitive procedure
- compound procedure
*/

// Just a single-use object to hold variables to load into memory in
// proper list-structure.
struct binding {
  char *variable;
  Element contents;
};

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
    END_OF_BINDINGS // Sentinel value.
  }
};

static Element load_frame(Binding *);


// come up with description of primitive vars/procedures
// run setup_env should load them into memory

Element setup_environment(Element *env)
{
  Element the_empty_environment = {
    .type_tag = PAIR,
    .contents.pair_ptr = NULL
  };

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
Element load_frame(Binding *b)
{
  Pair *frame_head = get_next_free_ptr();

  Element frame = {
    .type_tag = PAIR,
    .contents.pair_ptr = frame_head
  };

  for (
    Pair *curr_backbone = frame.contents.pair_ptr,
    *p = get_next_free_ptr();
    b->variable; // Stop when we encounter END_OF_BINDINGS.
    ++b,
    curr_backbone->cdr.contents.pair_ptr = get_next_free_ptr(),
    curr_backbone = curr_backbone->cdr.contents.pair_ptr,
    p = get_next_free_ptr()
  ) {

    p->car.type_tag = SYMBOL;
    // We could also copy the string into GCed memory.
    p->car.contents.symbol = b->variable;
    p->cdr = b->contents;

    // Wrapping Pair pointer in Element is optional, since the default
    // initialization gives it the PAIR type tag.
    curr_backbone->car.contents.pair_ptr = p;
  }

  return frame;
}


Element extend_environment(Element frame, Element base_env)
{
  return make_cons(var_val_pairs, base_env);
}

// (foo 123) is also a valid variable-value combo.

// Rudimentary env (just add operator) at index 0.
//        0   1   2   3   4
// cars [ p3,   ,   , + ,   ]
// cdrs [ e0,   ,   ,add,   ]

// Multivariable frame of ((+ add) (- sub)).
//        0   1   2   3   4
// cars [ p2, - , p3, + , p1]
// cdrs [ e0,sub, p4,add, e0] cdr at [0] is pointer to next frame.
