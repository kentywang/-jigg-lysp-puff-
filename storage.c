#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#define MEMORYLIMIT 100

static Pair memory1[MEMORYLIMIT], memory2[MEMORYLIMIT];
static Pair *free_ptr = memory1;

// For GC, need to load pointers to Elements in all registers into the
// working memory in a list structure that will be traversed.

// Which registers need to be preserved though?

char *string_alloc(int n)
{
  return (char *) malloc(n + 1);
}

// When not full, just the next element in array. Otherwise need to garbage
// collect.
Pair *get_next_free_ptr(void)
{
  Pair *p = free_ptr;

  if (free_ptr + 1 < memory1 + MEMORYLIMIT) {
    free_ptr += 1;
    return p;
  }

  // WIP, need to implement garbage collection.
  printf("Uh-oh.\n");
  return p;
}