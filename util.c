#include <ctype.h>
#include "lisp.h"

Boolean is_integer(char *s)
{
  if (*s == '-')
    s++;

  while (*s)
    if (!isdigit(*s++))
      return FALSE;
  return TRUE;
}