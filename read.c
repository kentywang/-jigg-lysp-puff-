#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

#define BUFFER_SIZE 100

static void read_dispatch(Element *);
static void read_word(const char, Element *);
static void fill_word_buffer(const char);
static void read_parenthesized(Element *);
static void read_quoted(Element *);
static char getch(void);
static void ungetch(const char);
static Boolean is_integer(char *s, const int end_index);

// For building word.
static char word_buffer[BUFFER_SIZE];
static int buffer_index = 0; // this is the next free index

// For get/ungetch. char_buffer holds either one new char from input or
// one "regurgitated" input char.
static char char_buffer;

void read_input(Element *e)
{
  char c;

  // Skip over leading whitespace.
  while (isspace(c = getch()))
    print_verbose("read_input\n  space\n");

  ungetch(c);

  // At start here, next char is non-space.
  read_dispatch(e);
}

// Mutates element to become pair with malloc string as the car, representing
// the symbol "quote".
void read_quoted(Element *e)
{
  e->type_tag = PAIR;
  e->contents.pair_ptr = get_next_free_ptr();

  e->contents.pair_ptr->car.type_tag = SYMBOL;
  e->contents.pair_ptr->car.contents.symbol = string_alloc(QUOTE_LENGTH);

  strcpy(e->contents.pair_ptr->car.contents.symbol, QUOTE);

  e->contents.pair_ptr->cdr.type_tag = PAIR;
  e->contents.pair_ptr->cdr.contents.pair_ptr = get_next_free_ptr();

  return read_dispatch(&e->contents.pair_ptr->cdr.contents.pair_ptr->car);
  // Relying on default initialization for empty list ending.
}

void read_dispatch(Element *e)
{
  char c = getch();

  if (c == '(') {
    print_verbose("read_dispatch\n  (\n");
    read_parenthesized(e);
    return;
  } else if (c == '\'') {
    read_quoted(e);
    return;
  } else {
    print_verbose("read_dispatch\n  %c at word buffer index %d\n", c, buffer_index);
    read_word(c, e);
    return;
  }

  // Everything else.
  fprintf(stderr, "Bad identifier: %c\n", c);
  exit(BAD_IDENTIFIER);
}

// Mutates element to become either number or malloc string.
void read_word(const char c, Element *e)
{
  fill_word_buffer(c);

  if (is_integer(word_buffer, buffer_index)) {
    e->type_tag = NUMBER;
    e->contents.number = atoi(word_buffer);
  } else {
    // If not a number, then it's a string that we must allocate for.
    e->type_tag = SYMBOL;
    e->contents.symbol = string_alloc(buffer_index);

    // Copy over word from stdin buffer to free pair's car.
    strncpy(e->contents.symbol, word_buffer, buffer_index);

    // TODO: Will this handle empty strings?
    *(e->contents.symbol + buffer_index) = '\0';
  }

  // Flush word buffer.
  buffer_index = 0;
}

void fill_word_buffer(const char prev_char)
{
  word_buffer[buffer_index++] = prev_char;

  char c = getch();

  if (
    isspace(c) ||
    c == '(' ||
    c == ')'
  ) {
    print_verbose("fill_word_buffer\n found ending condition\n");

    // Our job here is done. Return paren for another function to process.
    // We need to return character to buffer because the count variable must
    // be in sync.
    ungetch(c);
    return;
  }
  print_verbose("fill_word_buffer\n  character is: %c at word buffer index %d\n", c, buffer_index);

  fill_word_buffer(c);
}

void read_parenthesized(Element *e)
{
  print_verbose("read_parenthesized\n  starting...\n");

  e->type_tag = PAIR;
  char c;

  // Skip over leading whitespace. This will also keep reading after newline
  // if we're still within a parens.
  while (isspace(c = getch()))
    ;

  // Here, c is non-space char.
  if (c == ')') {
    print_verbose("read_parenthesized\n  )\n");
    // Explicity set ptr to NULL in case default initialization didn't.
    e->contents.pair_ptr = NULL;
    return;
  }

  Pair *p = e->contents.pair_ptr = get_next_free_ptr();
  
  ungetch(c); // for read_dispatch to get
  read_dispatch(&p->car);

  // Continue with the cdr, but no need to assign it to anything since it's
  // already been done by set_next_free_ptr.
  read_parenthesized(&p->cdr);
}

char getch(void)
{
  char c;

  if (char_buffer)
    c = char_buffer;
  else {
    // TODO: Need lower-level to discern whether reading new input.
    c = getchar();
  }

  if (char_buffer && c == char_buffer)
    print_verbose("getch\n  got %c from char buffer\n", c);

  char_buffer = 0;

  return c;
}

void ungetch(const char c)
{
  print_verbose("ungetch\n  %c now in char buffer\n", c);
  char_buffer = c;
}

Boolean is_integer(char *s, const int end_index)
{
  if (end_index == 0)
    return FALSE;

  int curr_index = 0;

  if (s[curr_index] == '-')
    curr_index++;

  if (curr_index == end_index)
    return FALSE;  // since the only character was "-"

  while (curr_index < end_index)
    if (!isdigit(s[curr_index++]))
      return FALSE;

  return TRUE;
}