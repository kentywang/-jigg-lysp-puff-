#include <stdio.h>
#include "lisp.h"

static void read_eval_print_loop(void);

Boolean verbosity;

int main(int argc, const char *argv[]) {
  char c;

  while (--argc && **++argv == '-')
    while ((c = *++*argv))
      switch (c) {
      case 'v':
        verbosity = TRUE;
        break;
      default:
        fprintf(stderr, "Unexpected argument: -%c\n", c);
        return UNEXPECTED_ARG;
      }

  read_eval_print_loop();
}

void read_eval_print_loop(void) {
  Element exp;
  Element val;
  Element env = setup_environment();
  save(env);  // keep the global env from being GCed

  do {
    // Read
    printf(">>>");
    read_input(&exp);

    // Eval
    val = eval_dispatch(exp, env);

    // Print
    print_element(val);

  } while (TRUE); // Loop
}
