#include <stdio.h>
#include "lisp.h"

/*
I think we either implement a fixed-memory system with manual garbage
collection (as directed in SICP) or a dynamically-growing-memory system using
malloc/free.

EDIT: Actually, my approach will be to garbage collect on a fixed region of
memory for list-structured data, but strings/symbols will actually be stored
in dynamically-allocated memory via malloc/free.

Make this into a folder and split the file!

I think we should use the agnostic Element type more than the Pair pointer.

Questions:
- How do you differentiate between newlines (ignored) and the enter key?
- I moved cdr assignment out from create_symbol (and create_pair), does it
  still work? I'd think so, because it probably chained numbers outside of
  parens, even when we only wanted that to happens within a set of
  parenthesis.
- Do we need to manually set CDR to null because of GC?
- We don't store symbols/numbers not in a list in the list struct, right?

Tests:
  (1(  2  3  )4)
   (1 2 3)
    (1 2 (3 4 (5)) 6 7)
    (1(  -2  3a  )4)

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

int main(int argc, char *argv[])
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
  
  // Print
  print_element(&exp/*val*/);
  printf("\n");

  // Free memory step?

  // // Loop
  // read_eval_print_loop();
}
