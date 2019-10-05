#include <stdio.h>
#include "lisp.h"

static void read_eval_print_loop(const Boolean);

int main(int argc, const char *argv[])
{
  char c;
  Boolean verbose = FALSE;

  while (--argc && **++argv == '-')
    while ((c = *++*argv))
      switch (c) {
      case 'v':
        verbose = TRUE;
        break;
      default:
        fprintf(stderr, "Unexpected argument: -%c\n", c);
        return UNEXPECTED_ARG;
      }

  read_eval_print_loop(verbose);
}

void read_eval_print_loop(const Boolean verbose)
{
  Element exp;
  Element val;
  Element env = setup_environment();

  while (TRUE) {
    // Read
    read_input(&exp, verbose);

    // Eval
    val = eval_dispatch(exp, env);

    // Print
    print_element(val);
    printf("\n");

    // Free memory step?
  }
}
