/** \file intern.h

    Library for pooling common strings

*/

#ifndef FISH_INTERN_H
#define FISH_INTERN_H

#include <wchar.h>

/**
   Return an identical copy of the specified string from a pool of unique strings. If the string was not in the pool, add a copy.

   \param The string to return an interned copy of
*/
const wchar_t *intern( const wchar_t *in );

/**
   Insert the specified string literal into the pool of unique
   strings. The string will not first be copied, and it will not be
   free'd on exit.
*/
const wchar_t *intern_static( const wchar_t *in );

/**
   Free all interned strings
*/
void intern_free_all();

#endif
