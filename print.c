#include <stdio.h>
#include "lisp.h"

static void print_pair(Pair *);

void print_element(Element *e)
{
  if (e) {
    switch (e->type_tag) {
    case PAIR:
      if (e->contents.pair_ptr)
        print_pair(e->contents.pair_ptr);
      else
        printf("nil");
      break;
    case NUMBER:
      printf("%d", e->contents.number);
      break;
    case SYMBOL:
      printf("%s", e->contents.symbol);
      break;
    case PRIMITIVE_PROCEDURE:
      // TODO: Show name. Probably will need to lookup in primitive table.
      printf("#<primitive-procedure>\n");
      break;
    case COMPOUND_PROCEDURE:
      // TODO: Show name.
      printf("#<compound-procedure>\n");
      break;
    }
    return;
  } else
    // We shouldn't get here unless GC.
    printf("Uh-oh.\n");
}

void print_pair(Pair *p)
{
  void print_element(Element *);

  printf("(");
  print_element(&p->car);
  printf(" . ");
  print_element(&p->cdr);
  printf(")");
}
