#pragma once
/*
Heap-allocated string handling.
Inspired by antirez's sds and gingerBill's gb_string.h.


                               EXAMPLE
                              ---------

  oso *mystring = NULL;
  osoput(&mystring, "Hello World");
  puts((char *)mystring);
  osoput(&mystring, "How about some pancakes?");
  puts((char *)mystring);
  osocatprintf(&mstring, " Sure! I'd like %d.", 5);
  puts((char *)mystring);
  osofree(mystring);

  > Hello World!
  > How about some pancakes?
  > How about some pancakes? Sure! I'd like 5.


                                RULES
                               -------

1. You can use null as an empty `oso *` string. 

2. But you can't pass null `char const *` or `oso **` arguments.

3. `oso *` can always be cast to `char *`, but it might be null.

4. Don't call oso functions with arguments that have overlapping memory.

  oso *mystring = NULL;
  osolen(mystring);                // OK, Gives 0
  osocat(&mystring, "waffles");    // OK
  osolen(mystring);                // Gives 7

  osocat(&mystring, NULL);         // Bad, crashes
  osocat(NULL, "foo");             // Bad, crashes
  osocatoso(&mystring, *mystring); // Bad, no idea what happens


                                NAMES
                               -------

  osoput______ -> Replace the contents.
  osocat______ -> Append.
  ______len    -> Do it with an explicit length argument, so the C-string
                  doesn't have to be null-terminated.
  ______oso    -> Do it with a second oso string.
  ______printf -> Do it by using printf.


                            ALLOC FAILURE
                           ---------------

If an allocation fails (including failing to reallocate) the `oso *` will be
set to null. If you decide to handle memory allocation failures, you'll need
to check for that.

This means that if you run out of memory and call an oso function, you might
lose your string. But the upside is that you can more easily check for and
handle out-of-memory situations if they do happen. Because of how tedious it
traditionally is, lots of libc/UNIX C software doessn't bother trying to handle
out-of-memory situations at all.
*/

#include <stdarg.h>
#include <stddef.h>

#if (defined(__GNUC__) || defined(__clang__)) && defined(__has_attribute)
#if __has_attribute(format)
#define OSO_PRINTF(a, b) __attribute__((format(printf, a, b)))
#endif
#if __has_attribute(nonnull)
#define OSO_NONNULL(args) __attribute__((nonnull args))
#endif
#endif
#ifndef OSO_PRINTF
#define OSO_PRINTF(a, b)
#endif
#ifndef OSO_NONNULL
#define OSO_NONNULL(args)
#endif

/* clang-format off */

typedef struct oso oso;

void
osoput(oso **p, char const *cstr)
/* Copies the null-terminated string on the right side into the left, replacing
   its contents.
   
   If `p` points to a null pointer, or if there isn't enough capacity in it to
   hold its new contents, it will be reallocated. The pointed-to pointer might
   be different after the call.

   oso *color = NULL;
   osoput(&color, "red");
   puts((char *)color); "red" */
   OSO_NONNULL((1, 2));

void
osoputlen(oso **p, char const *cstr, size_t len)
/* Like `osoput()`, but you specify the length (number of non-null chars) for
   the right side instead of it scanning for a null terminator.
   having a null terminator. */
   OSO_NONNULL((1, 2));

void
osoputoso(oso **p, oso const *other)
/* Like `osoput()`, but the right side is an oso. */
   OSO_NONNULL((1));

void
osoputprintf(oso **p, char const *fmt, ...)
/* Like `osoput()`, but do it with a printf. */
   OSO_NONNULL((1, 2)) OSO_PRINTF(2, 3);

void
osoputvprintf(oso **p, char const *fmt, va_list ap)
/* Like `osoput()`, but do it with a vprintf. */
   OSO_NONNULL((1, 2)) OSO_PRINTF(2, 0);

void
osocat(oso **p, char const *cstr)
/* Appends the contents of the right side onto the left. The pointed-to pointer
   will be reallocated if necessary.

   oso *fungus = NULL;
   osocat(&fungus, "mush");
   osocat(&fungus, "room");
   puts((char *)fungus); "mushroom" */
   OSO_NONNULL((1, 2));

void
osocatlen(oso **p, char const *cstr, size_t len)
/* Like `osocat()`, but you specify the length (number of non-null chars) for
   the right side instead of it scanning for a null terminator. */
   OSO_NONNULL((1, 2));

void
osocatoso(oso **p, oso const *other)
/* Like `osocat()`, but the right side side is an oso. */
   OSO_NONNULL((1));

void
osocatprintf(oso **p, char const *fmt, ...)
/* Like `osocat()`, but do it with a pritnf. */
   OSO_NONNULL((1, 2)) OSO_PRINTF(2, 3);

void
osocatvprintf(oso **p, char const *fmt, va_list ap)
/* Like `osocat()`, but do it with a vprintf. */
   OSO_NONNULL((1, 2)) OSO_PRINTF(2, 0);

void
osoensurecap(oso **p, size_t cap)
/* Ensure that the oso has at least `cap` memory allocated for its capacity.
   The capacity is the number of characters it can hold in its allocated
   memory, not counting the null terminator. A `cap` of 5 means there is room
   for 5 characters, plus the null terminator.
  
   This function doesn't care about the position of the null terminator in the
   existing contents or the length number -- only the heap memory allocation.

   oso *drink = NULL;
   char const *cstring = "horchata";
   size_t len = strlen(cstring);
   osoensurecap(&drink, len);
   memcpy((char *)drink, string, len);
   ((char *)drink)[len] = '\0';
   osopokelen(drink, len); */
   OSO_NONNULL((1));


void
osomakeroomfor(oso **p, size_t len)
/* Ensure that the oso has enough memory allocated for an additional `len`
   characters, not counting the null terminator.
   
   If you want to prepare to `memcpy()` 5 characters onto the end of an `oso`
   that's currently 10 characters in length, call `osomakeroomfor(myoso, 5);`.

   This only adjusts the heap memory allocation, not the null terminator or
   length number.

   Both `osoensurecap()` and `osomakeroomfor()` can be used to avoid repeated
   heap reallactions by allocating it all at once before doing other
   operations, like `osocat()`.
   
   You can also thsese functions if want to to modify the string buffer
   manually in your own code, and need to create some space in the buffer.
   
   oso *dinner = NULL;
   const char *potato = "potato";
   const char *salad = " salad";
   const char *wbeans = " with beans";
   osoput(&dinner, potato);
   // Avoid an extra reallocation here
   osomakeroomfor(&dinner, strlen(salad) + strlen(wbeans));
   osocat(&dinner, salad);
   osocat(&dinner, wbeans); */
   OSO_NONNULL((1));


void
osoclear(oso **p)
/* Sets the length number to 0, and write a null terminator at position 0.
   Leaves allocated heap memory in place.
   
   Unless the oso is null, in which case it doesn't do anything at all. */
   OSO_NONNULL((1));

void
osofree(oso *s);
/* Frees the memory. Calling with null is allowed. */

void
osowipe(oso **p)
/* Frees the memory, except you give it a pointer to a pointer, and it 
   sets the pointed-to pointer to null for you when it's done. Calling it with
   null is not allowed, but calling it with the pointed-to pointer as null is
   OK.
   
   osowipe(NULL);                 // Bad, will probably crash.

   oso *potato = NULL;
   osoput(&potato, "nice spuds");
   osowipe(&potato);              // OK.
   assert(*potato == NULL);       // true
   osowipe(&potato);              // OK. */
   OSO_NONNULL((1));

void
ososwap(oso **a, oso **b)
/* Swaps the two pointers. Why bother making a function for this? In case you
   need to debug a memory management problem in the future. You can put some
   debug code in the definition. */
   OSO_NONNULL((1, 2));

void
osopokelen(oso *s, size_t len)
/* Manually updates length field. Doesn't do anything else for you. */
   OSO_NONNULL((1));

size_t
osolen(oso const *s);
/* Bytes in use by the string (not including the null terminator.) */

size_t
osocap(oso const *s);
/* Bytes allocated on heap (not including the null terminator.) */

void
osolencap(oso const *s, size_t *out_len, size_t *out_cap)
/* Get both the len and the cap in one call. */
   OSO_NONNULL((2, 3));

size_t
osoavail(oso const *s);
/* osocap(s) - osolen(s) */

void
osotrim(oso *s, char const *cut_set)
/* Remove the characters in `cut_set` from the beginning and ending of `s`. */
   OSO_NONNULL((2));

/* clang-format on */
#undef OSO_PRINTF
#undef OSO_NONNULL
