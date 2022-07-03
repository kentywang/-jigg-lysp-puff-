#include <stdio.h>
#include <string.h>
#include "lisp.h"

void print_element(const Element);
void print_pair(const Pair *);
static void print_element_dispatch(const Element);
static void print_pair_contents(const Pair *);

void print_element(const Element e)
{
  print_element_dispatch(e);
  printf("\n");
}

void print_element_dispatch(const Element e)
{
  switch (e.type_tag) {
  case PAIR:
    if (e.contents.pair_ptr)
      print_pair(e.contents.pair_ptr);
    else
      printf("nil");
    break;
  case NUMBER:
    printf("%d", e.contents.number);
    break;
  case BOOLEAN:
    printf("%s", e.contents.truth ? TRUE_SYMBOL : FALSE_SYMBOL);
    break;
  case SYMBOL:
    printf("%s", e.contents.symbol);
    break;
  case PRIMITIVE_PROCEDURE:
    // TODO: Show name. Might need to add another parent primitive type.
    printf("#<primitive-procedure>");
    break;
  case COMPOUND_PROCEDURE:
    // TODO: Show name.
    printf("#<procedure>");
    break;
  }
}

void print_pair(const Pair *p)
{
  // if (
  //   p->car.type_tag == SYMBOL &&
  //   strcmp(p->car.contents.symbol, QUOTE) == 0
  // ) {
  //   printf("\'");
  //   print_pair_contents(p->cdr.contents.pair_ptr);
  // } else {
  printf("(");
  print_pair_contents(p);
  printf(")");
  // }
}

void print_pair_contents(const Pair *p)
{
  print_element_dispatch(p->car);

  if (p->cdr.type_tag != PAIR) {
    printf(" . ");
    print_element_dispatch(p->cdr);
  } else if (p->cdr.contents.pair_ptr) {
    // If CDR is pair, we omit the dot.
    printf(" ");
    print_pair_contents(p->cdr.contents.pair_ptr);
  }
  // We only print the CDR if it's not the empty list.
}
