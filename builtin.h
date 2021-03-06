/** \file builtin.h
	Prototypes for functions for executing builtin functions.
*/

#ifndef FISH_BUILTIN_H
#define FISH_BUILTIN_H

#include <wchar.h>

#include "util.h"

enum
{
	COMMAND_NOT_BUILTIN,
	BUILTIN_REGULAR,
	BUILTIN_FUNCTION
}
;

#define BUILTIN_ERR_MISSING L": Expected argument"
#define BUILTIN_ERR_COMBO L": Invalid combination of options"
#define BUILTIN_ERR_GLOCAL L": Variable can only be one of universal, global and local"
#define BUILTIN_ERR_EXPUNEXP L": Variable can't be both exported and unexported"
#define BUILTIN_ERR_UNKNOWN	L": Unknown option"

/**
   Stringbuffer used to represent standard output
*/
extern string_buffer_t *sb_out;

/**
   Stringbuffer used to represent standard error
*/
extern string_buffer_t *sb_err;

/**
   Kludge. Tells builtins if output is to screen
*/
extern int builtin_out_redirect;

/**
   Kludge. Tells builtins if error is to screen
*/
extern int builtin_err_redirect;


/**
   Initialize builtin data. 
*/
void builtin_init();

/**
   Destroy builtin data.
*/
void builtin_destroy();

/**
  Is there a builtin command with the given name?
*/
int builtin_exists( wchar_t *cmd );

/**
  Execute a builtin command 

  \param argv Array containing the command and parameters 
  of the builtin.  The list is terminated by a
  null pointer. This syntax resembles the syntax 
  for exec.

  \return the exit status of the builtin command
*/
int builtin_run( wchar_t **argv );

/**
  Insert all builtin names into l. These are not copies of the strings and should not be freed after use.
*/
void builtin_get_names( array_list_t *list );

/**
   Pushes a new set of input/output to the stack. The new stdin is supplied, a new set of output string_buffer_ts is created.
*/
void builtin_push_io( int stdin_fd );

/**
   Pops a set of input/output from the stack. The output string_buffer_ts are destroued, but the input file is not closed.
*/
void builtin_pop_io();


/**
   Return a one-line description of the specified builtin
*/
const wchar_t *builtin_get_desc( const wchar_t *b );

/**
   Counts the number of non null pointers in the specified array
*/
int builtin_count_args( wchar_t **argv );

/**
   Print help for the specified builtin. If \c b is sb_err, also print the line information
*/
void builtin_print_help( wchar_t *cmd, string_buffer_t *b );


int builtin_set(wchar_t **argv);
int builtin_commandline(wchar_t **argv);
int builtin_ulimit(wchar_t **argv);

/** 
	This function works like wperror, but it prints its result into
	the sb_err string_buffer_t instead of to stderr. Used by the builtin
	commands.
*/
void builtin_wperror( const wchar_t *s);

#endif
