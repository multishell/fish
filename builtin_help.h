/** \file builtin_help.h 

    Prototypes for functions for printing usage information of builtin commands. The
	corresponding .c file is automatically generated by combining the
	builtin_help.hdr file with doxygen output.
*/

#ifndef FISH_BUILTIN_HELP_H
#define FISH_BUILTIN_HELP_H

#include <wchar.h>

/**
   Return the help text for the specified builtin command. Use
   non-wide characters since wide characters have some issues with
   string formating escape sequences sometimes.

   \param cmd The command for which to obtain help text
*/
char *builtin_help_get( wchar_t *cmd );

/**
   Initialize builtin help data
*/
void builtin_help_init();

/**
   Destory builtin help data
*/
void builtin_help_destroy();

#endif
