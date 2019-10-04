#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

#define BUFFER_SIZE 256

static char *read_word(void);
static void read_parens(Element *);
static char *create_symbol(void);
static int getch(void);
static void ungetch(int);

// For building word.
static char word_buffer[BUFFER_SIZE];
static int buffer_index = 0;

// For get/ungetch.
static char char_buffer;

void read_input(Element *e)
{
  int c;

  // Skip over leading whitespace.
  while ((c = getch()) != EOF && isspace(c))
    printf("read_input\n  space\n");

  // Do something with EOF here.

  // At start here, c is non-EOF, non-space char.
  if (c == '(') {
    printf("read_input\n  (\n");
    e->type_tag = PAIR;
    read_parens(e);

    // printf("Finished reading input,\n");
    // print_element(e);
    // printf("\n");
    return;
  }
  // Included arithmetic symbols as valid components of a word.
  else if (
    isalnum(c) ||
    c == '-'   ||
    c == '+'   ||
    c == '*'   ||
    c == '/'
  ) {
    printf("read_input\n  %c\n", c);
    word_buffer[buffer_index++] = c;
    char *s = read_word();

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

char *read_word(void)
{
  int c = getch();

  if (c == EOF)
    return create_symbol();

  if (
    isspace(c) ||
    c == '('   ||
    c == ')'
  ) {
    printf("read_word\n found ending condition\n");

    // Our job here is done. Return paren for another function to process.
    // We need to return character to buffer because the count variable must
    // be in sync.
    ungetch(c);
    return create_symbol();
  }
  printf("read_word\n  character is: %c\n", c);
  word_buffer[buffer_index++] = c;

  return read_word();
}

void read_parens(Element *e)
{
  int c;

  // Skip over leading whitespace.
  while ((c = getch()) != EOF && isspace(c)) // _Don't_ stop for newline here?
    ;

  // Do something with EOF here.

  // At start here, c is non-EOF, non-space char.
  // This should handle empty lists, since p is initialized to NULL?
  if (c == ')') {
    printf("read_parens\n  )\n");
    e->contents.pair_ptr = NULL;
    return;
  }

  Pair *p = e->contents.pair_ptr = get_next_free_ptr();
  
  if (c == '(') {
    printf("read_parens\n  (\n");
    p->car.type_tag = PAIR;
    read_parens(&p->car);
  } else {
    printf("read_parens\n  %c\n", c);

    // Almost the same as in read_input. Is there an abstraction here?
    char *s = read_word();

    if (is_integer(s)) {
      p->car.type_tag = NUMBER;
      p->car.contents.number = atoi(s);
    } else {
      p->car.type_tag = SYMBOL;
      p->car.contents.symbol = s;
    }

    // If this was the last word in the list, we know read_word put ) in the
    // buffer, so we skip over it.
    getch();
  }

  // Continue with the cdr, but no need to assign it to anything since it's
  // already been done by set_next_free_ptr.
  p->cdr.type_tag = PAIR;
  read_parens(&p->cdr);

  // printf("Finished reading between parens,\n");
  // print_pair(p);
  // printf("\n");

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
  printf("create_symbol\n  \"%s\", size: %d\n", s, buffer_index);

  // Flush word buffer.
  buffer_index = 0;

  return s;
}

int getch(void)
{
  int d = char_buffer ? char_buffer : getchar();

  char_buffer = 0;

  return d;
}

void ungetch(int c)
{
  char_buffer = c;
}