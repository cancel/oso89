#include "oso89.h"
#include <stdio.h>

int
main(void) {
  oso *s = 0;
  osoputprintf(&s, "I have %d %s", 5, "cucumbers");
  puts((char *)s);
  osocat(&s, " and no carrots");
  puts((char *)s);
  osowipe(&s);
  return 0;
}
