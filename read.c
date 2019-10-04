#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

#define BUFFER_SIZE 100
#define print_verbose verbosity && printf

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

static Boolean verbosity;

void read_input(Element *e, const Boolean use_verbose)
{
  verbosity = use_verbose;
  int c;

  // Skip over leading whitespace.
  while (isspace(c = getch()))
    print_verbose("read_input\n  space\n");

  // At start here, c is non-space char.
  if (c == '(') {
    print_verbose("read_input\n  (\n");

    e->type_tag = PAIR;
    read_parens(e);

    return;
  }
  if (
    isalnum(c) ||
    // Included arithmetic symbols as valid beginning of a word.
    c == '+'   ||
    c == '*'   ||
    c == '/'   ||
    c == '-'
  ) {
    print_verbose("read_input\n  %c at word buffer index %d\n", c, buffer_index);

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

  // Everything else.
  fprintf(stderr, "Bad identifier: %c\n", c);
  exit(BAD_IDENTIFIER);
}

char *read_word(const char prev_char)
{
  word_buffer[buffer_index++] = prev_char;

  int c = getch();

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
  int c;

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

  // Copy over word from stdin to free pair's car.
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