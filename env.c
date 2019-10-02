#include <stdlib.h>
#include <string.h>
#include "lisp.h"

typedef enum procedure Procedure;

enum procedure {
  PRIMITIVE,
  COMPOUND
};

// (((+ 'primitive add) (- sub))
//  ((cons cons)))

void setup_environment(Element *env)
{
  // Pseudocode:
  // cons(make_frame([+, add], [-, sub]), env);
}


Element extend_environment(Element var_val_pairs, Element base_env)
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
