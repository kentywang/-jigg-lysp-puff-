#include "lisp.h"

#define STACK_SIZE 100

static Pair *stack[STACK_SIZE];
static int index = 0;

void save(Pair *p)
{
  stack[index++] = p;
}

void forget(void)
{
  --index;
}