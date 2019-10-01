#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

static char *read_word(int);
static void read_parens(Element *);
static int get_number(int);
static char *create_symbol(int);
static int getch(void);
static void ungetch(int);
static void buffered_seek(int);

static int char_buffer = 0;

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
  else if (isalnum(c) || c == '-') {
    printf("read_input\n  %c\n", c);
    char *s = read_word(1);

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

char *read_word(int count)
{
  int c = getch();

  if (
    c == EOF ||
    isspace(c) ||
    c == '(' ||
    c == ')'
  ) {
    printf("read_word\n found ending condition\n");

    // Do something with EOF here.

    // Our job here is done. Return paren for another function to process.
    // We need to return character to buffer because the count variable must
    // be in sync.
    ungetch(c);
    return create_symbol(count);
  }
  printf("read_word\n  character is: %c\n", c);

  return read_word(count + 1);
}

void read_parens(Element *e)
{
  int c;

  // Skip over leading whitespace.
  while ((c = getch()) != EOF && isspace(c))
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
    char *s = read_word(1);

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

int get_number(int size)
{
  char s[size];

  buffered_seek(size);
  fgets(s, size + 1, stdin);

  return atoi(s);
}

char *create_symbol(int size)
{
  // Reserve memory size for word.
  char *s = (char *) malloc(size + 1);

  // Rewind stdin pointer to start of word.
  buffered_seek(size);

  // Copy over word from stdin to free pair's car.
  fgets(s, size + 1, stdin);
  // Will this handle empty strings?
  *(s + size) = '\0';
  printf("create_symbol\n  \"%s\", size: %d\n", s, size);

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

void buffered_seek(int n)
{
  fseek(stdin, -n - (char_buffer ? 1 : 0), SEEK_CUR);
}