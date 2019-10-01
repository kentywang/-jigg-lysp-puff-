#include <stdio.h>
#include "lisp.h"

#define MEMORYLIMIT 1000

static Pair memory[MEMORYLIMIT];
static Pair *free_ptr = memory;

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

// Returns _old_ free pointer, while updating current free pointer. We can
// always get the current free pointer in the global scope.
Pair *set_next_free_ptr(void)
{
  Pair *old_free_ptr = free_ptr;
  free_ptr = free_ptr->cdr.contents.pair_ptr;
  return old_free_ptr;
}
