#include <stdlib.h>
#include <string.h>
#include "lisp.h"

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
  }
};

// come up with description of primitive vars/procedures
// run setup_env should load them into memory

// (((+ 'primitive add) (- sub))
//  ((cons cons)))


// void setup_environment(Element *env)
// {
//   // Pseudocode:
//   extend_environment(env, [+, add], [-, sub]);
// }

// primitive_procedures = {
//   { "+", &add },

// }

// Element extend_environment(Element var_val_pairs, Element base_env)
// {
//   return make_cons(var_val_pairs, base_env);
// }

// (foo 123) is also a valid variable-value combo.

// Rudimentary env (just add operator) at index 0.
//        0   1   2   3   4
// cars [ p3,   ,   , + ,   ]
// cdrs [ e0,   ,   ,add,   ]

// Multivariable frame of ((+ add) (- sub)).
//        0   1   2   3   4
// cars [ p2, - , p3, + , p1]
// cdrs [ e0,sub, p4,add, e0] cdr at [0] is pointer to next frame.
