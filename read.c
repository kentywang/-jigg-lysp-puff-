#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

#define BUFFER_SIZE 100

static void read_dispatch(Element *);
static char *read_word(const char);
static void read_parens(Element *);
static char *create_symbol(void);
static char getch(void);
static void ungetch(const char);

// For building word.
static char word_buffer[BUFFER_SIZE];
static int buffer_index = 0;

// For get/ungetch.
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

void read_dispatch(Element *e)
{
  char c = getch();

  if (c == '(') {
    print_verbose("read_dispatch\n  (\n");

    // TODO: Why not move this into the function?
    e->type_tag = PAIR;
    return read_parens(e);
  }
  if (
    isalnum(c) ||
    // Included arithmetic symbols as valid beginning of a word.
    c == '+' ||
    c == '*' ||
    c == '/' ||
    c == '-'
  ) {
    print_verbose("read_dispatch\n  %c at word buffer index %d\n", c, buffer_index);

    char *s = read_word(c);

    if (is_integer(s)) {
      e->type_tag = NUMBER;
      e->contents.number = atoi(s);
      return;
    }

    e->type_tag = SYMBOL;
    e->contents.symbol = s;
    return;
  }

  // Convert single-quotes into a list beginning with "quote".
  if (c == '\'') {
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

  // Everything else.
  fprintf(stderr, "Bad identifier: %c\n", c);
  exit(BAD_IDENTIFIER);
}

char *read_word(const char prev_char)
{
  word_buffer[buffer_index++] = prev_char;

  char c = getch();

  if (
    isspace(c) ||
    c == '('   ||
    c == ')'
  ) {
    print_verbose("read_word\n found ending condition\n");

    // Our job here is done. Return paren for another function to process.
    // We need to return character to buffer because the count variable must
    // be in sync.
    ungetch(c);
    return create_symbol();
  }
  print_verbose("read_word\n  character is: %c at word buffer index %d\n", c, buffer_index);

  return read_word(c);
}

void read_parens(Element *e)
{
  print_verbose("read_parens\n  starting...\n");
  char c;

  // Skip over leading whitespace. This will also keep reading after newline
  // if we're still within a parens.
  while (isspace(c = getch()))
    ;

  // At start here, c is non-space char.
  // This should handle empty lists, since p is initialized to NULL?
  if (c == ')') {
    print_verbose("read_parens\n  )\n");
    e->contents.pair_ptr = NULL;
    return;
  }

  Pair *p = e->contents.pair_ptr = get_next_free_ptr();
  
  if (c == '(') {
    print_verbose("read_parens\n  (\n");
    p->car.type_tag = PAIR;
    read_parens(&p->car);
  } else if (c == '\'') {
    // TODO: Nearly the same as in read_input. Must abstract this.
    p->car.type_tag = PAIR;
    p->car.contents.pair_ptr = get_next_free_ptr();

    p->car.contents.pair_ptr->car.type_tag = SYMBOL;
    p->car.contents.pair_ptr->car.contents.symbol = string_alloc(QUOTE_LENGTH);
    strcpy(p->car.contents.pair_ptr->car.contents.symbol, QUOTE);

    p->car.contents.pair_ptr->cdr.type_tag = PAIR;
    p->car.contents.pair_ptr->cdr.contents.pair_ptr = get_next_free_ptr();

    read_dispatch(&p->car.contents.pair_ptr->cdr.contents.pair_ptr->car);

    // This part is different though.
    p->car.contents.pair_ptr->cdr.contents.pair_ptr->cdr.type_tag = PAIR;
    return read_parens(&p->car.contents.pair_ptr->cdr.contents.pair_ptr->cdr);

    // Relying on default initialization for empty list ending.
  } else {
    print_verbose("read_parens\n  %c at word buffer index %d\n", c, buffer_index);

    // Almost the same as in read_input. Is there an abstraction here?
    char *s = read_word(c);

    if (is_integer(s)) {
      p->car.type_tag = NUMBER;
      p->car.contents.number = atoi(s);
      print_verbose("read_parens\n  %d was an int\n", p->car.contents.number);
    } else {
      p->car.type_tag = SYMBOL;
      p->car.contents.symbol = s;
      print_verbose("read_parens\n  %s was a symbol\n", p->car.contents.symbol);
    }
  }

  // Continue with the cdr, but no need to assign it to anything since it's
  // already been done by set_next_free_ptr.
  p->cdr.type_tag = PAIR;
  read_parens(&p->cdr);

  return;
}

char *create_symbol()
{
  // Reserve memory size for word.
  char *s = string_alloc(buffer_index);

  // Copy over word from stdin buffer to free pair's car.
  strncpy(s, word_buffer, buffer_index);

  // Will this handle empty strings?
  *(s + buffer_index) = '\0';
  print_verbose("create_symbol\n  \"%s\", size: %d\n", s, buffer_index);

  // Flush word buffer.
  buffer_index = 0;

  return s;
}

char getch(void)
{
  char c;

  if (char_buffer)
    c = char_buffer;
  else {
    // TODO: Need lower-level to discern whether reading new input.
    // printf("> ");
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