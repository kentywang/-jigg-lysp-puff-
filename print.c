#include <stdio.h>
#include <string.h>
#include "lisp.h"

static void print_element_dispatch(Element);
static void print_pair_data(const Pair *);

void print_element(const Element e) {
  print_element_dispatch(e);
  printf("\n");
}

void print_element_dispatch(const Element e) {
  switch (e.type) {
  case PAIR:
    if (e.data.pair_ptr)
      print_pair(e.data.pair_ptr);
    else
      printf("nil");
    break;
  case NUMBER:
    printf("%d", e.data.number);
    break;
  case BOOLEAN:
    printf("%s", e.data.truth ? TRUE_SYMBOL : FALSE_SYMBOL);
    break;
  case SYMBOL:
    printf("%s", e.data.symbol);
    break;
  case PRIMITIVE_PROCEDURE:
    // TODO: Show name. Might need to add another parent primitive type.
    printf("#<primitive-procedure>");
    break;
  case COMPOUND_PROCEDURE:
    // TODO: Show name.
    printf("#<procedure>");
    break;
  default:
    printf("Unexpected type tag: %u, %p\n", e.type, e.data.pair_ptr);
    break;
  }
}

void print_pair(const Pair *p) {
  printf("(");
  print_pair_data(p);
  printf(")");
}

void print_pair_data(const Pair *p) {
  print_element_dispatch(p->car);

  if (p->cdr.type != PAIR) {
    printf(" . ");
    print_element_dispatch(p->cdr);
  } else if (p->cdr.data.pair_ptr) {
    // If CDR is pair, we omit the dot.
    printf(" ");
    print_pair_data(p->cdr.data.pair_ptr);
  }
  // We only print the CDR if it's not the empty list.
}
