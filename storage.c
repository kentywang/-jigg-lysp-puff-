#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#define MEMORYLIMIT 1000

static Pair memory1[MEMORYLIMIT], memory2[MEMORYLIMIT];
static Pair *free_ptr = memory1;
static Boolean on_memory1 = TRUE;

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

  if (on_memory1) {
    if (free_ptr + 1 < memory1 + MEMORYLIMIT) {
      free_ptr += 1;
      printf("Space left: %ld\n", memory1 + MEMORYLIMIT - free_ptr);
    } else {
      free_ptr = memory2;
      on_memory1 = FALSE;

      // GC here.
    }
  } else {
    if (free_ptr + 1 < memory2 + MEMORYLIMIT) {
      free_ptr += 1;
      printf("Space left: %ld\n", memory2 + MEMORYLIMIT - free_ptr);
    } else {
      free_ptr = memory1;
      on_memory1 = TRUE;

      // GC here.
    }
  }

  return p;
}