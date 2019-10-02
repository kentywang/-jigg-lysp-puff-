#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#define MEMORYLIMIT 1000

static Pair memory[MEMORYLIMIT];
static Pair *free_ptr = memory;

char *string_alloc(int n)
{
  return (char *) malloc(n + 1);
}

// When not full, just the next element in array. Otherwise need to garbage
// collect.
Pair *get_next_free_ptr(void)
{
  Pair *p = free_ptr;

  if (free_ptr + 1 < memory + MEMORYLIMIT) {
    free_ptr += 1;
    return p;
  }

  // WIP, need to implement garbage collection.
  printf("Uh-oh.\n");
  return p;
}