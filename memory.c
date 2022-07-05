#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#define HEAP_LIMIT 5000
#define STACK_LIMIT 100

// Array of pair addresses (not array of pairs)
static Pair *heap1[HEAP_LIMIT];
static Pair *heap2[HEAP_LIMIT];
// An analogous array to heap to track if we should keep element at array
// index during GC.
static Boolean keep[HEAP_LIMIT] = {FALSE};
// We track addresses we've already GCed to not double free.
// static Boolean deleted[HEAP_LIMIT] = {FALSE};

static int index_h = 0; // Index of next free slot
static Pair **curr_heap = heap1;

// This is the Lisp-level stack used to store the env for each Lisp
// stack frame. The GC uses this stack to know which roots to keep.
// Array of pair addresses (not array of pairs).
static Pair *stack[STACK_LIMIT];
static int index_s = 0;  // Index of next free slot

static Boolean already_deleted(void *);
static void mark_to_keep(Pair *);
static void add_to_deleted(void *);
static void cleanup_pair(Pair *);
static void cleanup_element(Element);
static void gc(void);

// We'll use this to keep a running list of deleted addresses

struct linked_list_node {
  void *value;  // Holds pointer.
  struct linked_list_node *next;
};

typedef struct linked_list_node AddressNode;

static AddressNode *deleted = NULL;

static void reset_deleted(void);

// "Registers" that GC uses as roots to start marking.
Element curr_exp;
Element curr_val;
Element global_env;

char *string_alloc(int n)
{
  return (char *) malloc(n + 1);
}
 
Pair *get_next_free_ptr(void)
{
  Pair *p = malloc(sizeof(Pair));
  // //X printf("%p\n", p);
  if (index_h == HEAP_LIMIT)
    gc();

  if (index_h == HEAP_LIMIT) {
    fprintf(stderr, "Heap overflow: heap height: %d\n", index_h);
    exit(HEAP_OVERFLOW);
  }

  curr_heap[index_h++] = p;
  //X printf("Space left: %d\n", HEAP_LIMIT - index_h);

  return p;
}

void free_element(Element *e)
{
  return;
}

// For GC, need to traverse all pairs starting in registers and
// stack, marking each as traversable (and thus to-retain).
void gc(void)
{ 
  printf("GC START!");
  if (index_s > 0) {
    // Mark addresses referenced by stack.
    for (int i = 0; i <= index_s; i++)
      // TODO: n^4 time complexity, fix
      for (int j = 0; j < HEAP_LIMIT; j++)
        if (curr_heap[j] == stack[i])
          mark_to_keep(curr_heap[j]);
  // If no stack, we need to keep global env at least.
  } else {
    for (int i = 0; i < HEAP_LIMIT; i++)
      if (curr_heap[i] == global_env.contents.pair_ptr)
        mark_to_keep(curr_heap[i]);
  }

  // Mark addresses referenced by curr_exp.
  // TODO: Does symbol element also need to be marked? Currently we're only
  // marking pairs.
  if (curr_exp.type_tag == PAIR || curr_exp.type_tag == COMPOUND_PROCEDURE)
    for (int i = 0; i < HEAP_LIMIT; i++)
      if (curr_heap[i] == curr_exp.contents.pair_ptr) {
        mark_to_keep(curr_heap[i]);
        //X printf("keeping this curr expr: ");
        // print_pair(curr_heap[i]);
        //X printf(" whose cdr has addr: %p\n", curr_heap[i]->cdr.contents.pair_ptr);
      }
  // Disabling curr_val preservation because we don't actually need it for
  // computation.
  // if (curr_val.type_tag == PAIR || curr_val.type_tag == COMPOUND_PROCEDURE)
  //   for (int i = 0; i < HEAP_LIMIT; i++)
  //     if (curr_heap[i] == curr_val.contents.pair_ptr)
  //       mark_to_keep(curr_heap[i]);

  // //X printf("BEFORE RECURSIVE MARKING\n");
  int k = 0;
  // for (int i = 0; i < HEAP_LIMIT; i++) {
  //   //X printf("GC index: %d, keep: %d\n", i, keep[i]);
  //   print_pair(curr_heap[i]);
  //   //X printf("\n");

  //   if (keep[i]) 
  //     k += 1;
  // }
  // //X printf("k: %d\n", k);

  // Those marked currently are the roots. We'll recurse down those pairs to
  // mark all recursively referenced pairs.
  // TODO: I think this has potential to change in flight, adding more work.
  // for (int i = 0; i < HEAP_LIMIT; i++) {
  //   if (keep[i]) {
  //     //X printf("KEEP THIS ROOT: ");
  //     print_pair(curr_heap[i]);
  //     //X printf("\n");
  //     mark_to_keep(curr_heap[i]);
  //   }
  // }

  // //X printf("AFTER RECURSIVE MARKING\n");
  // k = 0;
  // for (int i = 0; i < HEAP_LIMIT; i++) {
  //   // //X printf("GC index: %d, keep: %d\n", i, keep[i]);
  //   // print_pair(curr_heap[i]);
  //   // //X printf("\n");

  //   if (keep[i])
  //     k += 1;
  // }
  // //X printf("k: %d\n", k);
  // for (int i = 0; i < HEAP_LIMIT; i++)
  //   if (!keep[i]) {
  //     //X printf("Deleting: ");
  //     print_pair(curr_heap[i]);
  //     //X printf("\n");
  //   }

  // Sweep phase
  index_h = 0;
  Pair **next_heap = curr_heap == heap1 ? heap2 : heap1;

  for (int i = 0; i < HEAP_LIMIT; i++) {
    Pair *p = curr_heap[i];
    if (keep[i]) {
      // Copy address to other heap.
      next_heap[index_h++] = p;
      //X printf("HEAP element %d, %p\n", index_h, p);
      // print_pair(p);
      //X printf("\n");
    }
    else
      cleanup_pair(p);
  }

  // Swap heaps.
  curr_heap = next_heap;
  // Reset marks.
  for (int i = 0; i < HEAP_LIMIT; i++) {
    keep[i] = FALSE;
    // deleted[i] = FALSE;
  }
  // Reset list of deleted addresses too.
  reset_deleted();
  printf("GCed, current heap size: %d\n", index_h);
  printf("last val:\n");
  print_element(curr_val);
  printf("\nCurr expr\n");
  print_element(curr_exp);
  printf("\nglobal env\n");
  print_element(global_env);
  printf("\nCurr stack height: %d\n", index_s);
  if (stack[index_s-1]) {
    int y = index_s-1;
    while (y >= 0) {
      printf("stack env at index %d: %p\n", y, stack[y]);
      print_pair(stack[y]);
      printf("\n");
      y--;
    }
  }
}

void reset_deleted(void)
{
  // TODO: O(n) time, reduce to O(1) with hash table.
  AddressNode *curr = deleted;
  AddressNode *next;

  // Traverse to last node.
  while (curr) {
    next = curr->next; // Save reference to next node.
    free(curr);
    curr = next;
  }

  deleted = NULL;
  //X printf("RESET DELETED, %p\n", deleted);
}

void mark_to_keep(Pair *p)
{
  if (p)
    for (int i = 0; i < HEAP_LIMIT; i++)
      if (curr_heap[i] == p && !keep[i]) {
        //X printf("mark_to_keep\n  ");
        // print_pair(p);
        //X printf("\n");

        keep[i] = TRUE;

        if (p->car.type_tag == PAIR || p->car.type_tag == COMPOUND_PROCEDURE) {
          //X printf("first get car\n");
          // print_pair(p->car.contents.pair_ptr);
        }
    
        if (p->cdr.type_tag == PAIR || p->cdr.type_tag == COMPOUND_PROCEDURE) {
          //X printf("then get cdr\n");
          //X p->cdr.contents.pair_ptr ? print_pair(p->cdr.contents.pair_ptr) : printf("nullptr");
        }

        // This is icky since compound procedures are still pairs underneath.
        // How can this be improved?
        if (p->car.type_tag == PAIR || p->car.type_tag == COMPOUND_PROCEDURE)
          mark_to_keep(p->car.contents.pair_ptr);
    
        if (p->cdr.type_tag == PAIR || p->cdr.type_tag == COMPOUND_PROCEDURE)
          mark_to_keep(p->cdr.contents.pair_ptr);
      }
}

Boolean already_deleted(void *ptr)
{
  // TODO: O(n) time, reduce to O(1) with hash table.
  AddressNode *curr = deleted;

  while (curr != NULL) {
    //X printf("already deleted: %p\n", curr->value);
    if (curr->value == ptr)
      return TRUE;
    curr = curr->next;
    //X printf("next ptr: %p\n", curr);
  }
  return FALSE;
}

void add_to_deleted(void *ptr)
{
  // TODO: O(n) time, reduce to O(1) with hash table.
  AddressNode *curr = deleted;

  // Create first node if no nodes yet.
  if (!deleted) {
    deleted = malloc(sizeof(AddressNode));
    deleted->value = ptr;
    deleted->next = NULL;
  } else {
    AddressNode *prev;

    // Traverse to last node.
    while (curr) {
      //X printf("addr to mark as deleted: %p\n", curr->value);
      prev = curr;
      curr = curr->next;
    }

    // Then add new address as new node
    prev->next = malloc(sizeof(AddressNode));
    prev->next->value = ptr;
    prev->next->next = NULL;
  }
}

void cleanup_pair(Pair *p)
{
  if (p) {
    //X printf("cleanup pair: \n");
    // print_pair(p);
    //X printf("\nfirst, car\n");
    cleanup_element(p->car);
    //X printf("next, cdr\n");
    cleanup_element(p->cdr);
    // Check that it hasn't been freed yet.
    if (!already_deleted(p)) {
      //X printf("finally, freeing pair at %p\n", p);
      free(p);
      add_to_deleted(p);
    }
    //X printf("finished with cleaning up pair\n");
    return;
  } else {
    //X printf("null ptr, no cleanup needed\n");
    return;
  }
}



void cleanup_element(Element e)
{
  if (e.type_tag == PAIR || e.type_tag == COMPOUND_PROCEDURE)
    return cleanup_pair(e.contents.pair_ptr);
  // if (e.type_tag == SIMPLE_PROCEDURE)
  //   return cleanup_pair(e.contents.func_ptr);
  if (e.type_tag == SYMBOL) {
    //X printf("symbol! %p\n", e.contents.symbol);
    if (!already_deleted(e.contents.symbol)) {
      //X printf("Freeing symbol %s %p\n", e.contents.symbol, e.contents.symbol);
      free(e.contents.symbol);
      add_to_deleted(e.contents.symbol);
      //X printf("Done\n");
    }
    return;
  }
  //X printf("didn't have any cleanup for this element\n");
  // No other cleanup needed except those that needed malloc.
}

void save(Pair *p)
{
  if (index_s == STACK_LIMIT) {
    fprintf(stderr, "Stack overflow: stack height: %d\n", index_s);
    exit(STACK_OVERFLOW);
  }

  stack[index_s++] = p;

  //X printf("STACK_HEIGHT: %d\n", index_s);
  // //X printf("CURR ENV: ");
  // print_pair(stack[index_s-1]);
  // //X printf("\n");
}

void forget(void)
{
  --index_s;
  stack[index_s] = NULL;
}