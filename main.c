#include <stdio.h>
#include "lisp.h"

/*
My approach will be to garbage collect on a fixed region of memory for
list-structured data, but strings/symbols will actually be stored in
dynamically-allocated memory via malloc/free. Though we could store them
in lists eventually.

I think we should use the agnostic Element type more than the Pair pointer.

Questions:
- How do you differentiate between newlines (ignored) and the enter key?
  A: Don't think you can, you need to listen to keyboard's shift.
- Do we need to manually set CDR to null because of GC?
- Should I create an abstraction layer to convert an argument list into
  a va_list for primitive functions to apply?

Tests:
  (1(  2  3  )4)
   (1 2 3)
    (1 2 (3 4 (5)) 6 7)
    (1(  -2  3a  )4)
apple
   apple

Todos:
- Handle EOF.
- Devise method of unit testing.
- Remove unneeded wrapping of Pair pointers with an Element, since they're
  initialized with PAIR type tags already. (GC might complicate this, we'll
  need to reset the type_tag and content in get_next_free_ptr! Hmm, but that
  approach might mean we'd need to manually set the CDR to the empty pair
  instead on relying on uninitialized Elements to always be empty pairs, or
  would it?)

Lessons learned:
- For mutating an object's pointer member, I can't pass the pointer into
  a function; I must pass the object.
*/

static Element exp;
// static Element env;
// void *val;
// char continue;
// void (*proc)(...);
// void *argl[];
// void *unev;

static void read_eval_print_loop(void);

int main(const int argc, const char *argv[])
{
  // setup_environment(&env);

  read_eval_print_loop();
}

void read_eval_print_loop(void)
{
  // Read
  read_input(&exp);

  // Eval
  // env = get_global_environment();
  // val = eval_dispatch(exp, env);

  // Intermediate testing
  Element exp2 = add(exp.contents.pair_ptr);
  // exp.contents.pair_ptr->car.contents.pair_ptr->car.contents.number = 5;

  // Print
  print_element(&exp2/*val*/);
  printf("\n");

  // Free memory step?

  // // Loop
  // read_eval_print_loop();
}
