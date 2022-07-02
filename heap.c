#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#define HEAP_LIMIT 1000

static Pair heap1[HEAP_LIMIT], heap2[HEAP_LIMIT];
static Pair *free_ptr = heap1;
static Boolean on_heap1 = TRUE;

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

  if (on_heap1) {
    if (free_ptr + 1 < heap1 + HEAP_LIMIT) {
      free_ptr += 1;
      print_verbose("Space left: %ld\n", heap1 + HEAP_LIMIT - free_ptr);
    } else {
      free_ptr = heap2;
      on_heap1 = FALSE;

      // GC here.
    }
  } else {
    if (free_ptr + 1 < heap2 + HEAP_LIMIT) {
      free_ptr += 1;
      print_verbose("Space left: %ld\n", heap2 + HEAP_LIMIT - free_ptr);
    } else {
      free_ptr = heap1;
      on_heap1 = TRUE;

      // GC here.
    }
  }

  return p;
}