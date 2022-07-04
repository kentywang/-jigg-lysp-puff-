#include <stdio.h>
#include "lisp.h"

static void read_eval_print_loop(void);

Boolean verbosity;

int main(int argc, const char *argv[])
{
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

void read_eval_print_loop(void)
{
  global_env = setup_environment();
  // print_element(global_env);
  while (TRUE) {
    // Read
    //X printf("\nglobal env\n");
    // print_element(global_env);
    printf("λ 》");
    read_input(&curr_exp);

    // //X printf("READ:\n");
    // print_element(curr_exp);

    // Eval
    curr_val = eval_dispatch(curr_exp, global_env);

    // Print
    print_element(curr_val);

    // //X printf("ENV:\n");
    // print_element(env);

    // Free memory step?
  }
}
