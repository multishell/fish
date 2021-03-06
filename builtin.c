/** \file builtin.c
	Functions for executing builtin functions.

	How to add a new builtin function:

	1). Create a function in builtin.c with the following signature:

	<tt>static int builtin_NAME( wchar_t ** args )</tt>
	
	where NAME is the name of the builtin, and args is a zero-terminated list of arguments. 

	2). Add a line like hash_put( &builtin, L"NAME", &builtin_NAME ); to builtin_init. This will enable the parser to find the builtin function.

	3). Add a line like hash_put( desc, L"NAME", L"Frobble the bloogle" ); to the proper part of builtin_get_desc, containing a short description of what the builtin does. This description is used by the completion system.

	4). Create a file names doc_src/NAME.txt, contining the manual for the builtin in Doxygen-format. Check the other builtin manuals for proper syntax.
	
	5). Add an entry to the BUILTIN_DOC_SRC variable of Makefile.in. Note that the entries should be sorted alpabetically!

	6). Add an entry to the manual at the builtin-overview subsection

*/

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <wctype.h>
#include <sys/time.h>
#include <time.h>

#include "config.h"
#include "util.h"
#include "wutil.h"
#include "builtin.h"
#include "function.h"
#include "complete.h"
#include "proc.h"
#include "parser.h"
#include "reader.h"
#include "env.h"
#include "expand.h"
#include "common.h"
#include "wgetopt.h"
#include "sanity.h"
#include "tokenizer.h"
#include "builtin_help.h"
#include "wildcard.h"
#include "input_common.h"
#include "input.h"
#include "intern.h"
#include "event.h"
#include "signal.h"

/**
   The default prompt for the read command
*/
#define DEFAULT_READ_PROMPT L"set_color green; echo read; set_color normal; echo \"> \""

/**
   The mode name to pass to history and input
*/

#define READ_MODE_NAME L"fish_read"
/**
   Table of all builtins
*/
static hash_table_t builtin;

int builtin_out_redirect;
int builtin_err_redirect;

/** 
	Buffers for storing the output of builtin functions
*/
string_buffer_t *sb_out=0, *sb_err=0;
/**
   Stack containing builtin I/O for recursive builtin calls.
*/
static array_list_t io_stack;

/** 
	The file from which builtin functions should attempt to read, use
	instead of stdin.
*/
static int builtin_stdin;

/**
   Table containing descriptions for all builtins
*/
static hash_table_t *desc=0;

int builtin_count_args( wchar_t **argv )
{
	int argc = 1;
	while( argv[argc] != 0 )
	{
		argc++;
	}
	return argc;
}


void builtin_wperror( const wchar_t *s)
{
	if( s != 0 )
	{
		sb_append2( sb_err, s, L": ", (void *)0 );
	}
	char *err = strerror( errno );
	wchar_t *werr = str2wcs( err );
	if( werr )
	{
		sb_append2( sb_err,  werr, L"\n", (void *)0 );	
		free( werr );
	}	
}


/*
  Here follows the definition of all builtin commands. The function
  names are all on the form builtin_NAME where NAME is the name of the
  builtin. so the function name for the builtin 'jobs' is
  'builtin_jobs'.

  Two builtins, 'command' and 'builtin' are not defined here as they
  are part of the parser. (They are not parsed as commands, instead
  they only slightly alter the parser state)

*/


/**
   Noop function. A fake function which successfully does nothing, for
   builtins which are handled by the parser, such as command and
   while.
*/
static int builtin_ignore( wchar_t **argv )
{
	return 0;
}

void builtin_print_help( wchar_t *cmd, string_buffer_t *b )
{
	const char *h;

	if( b == sb_err )
	{
		sb_append( sb_err, 
				   parser_current_line() );
	}

	h = builtin_help_get( cmd );

	if( !h )
		return;

	
	
	wchar_t *str = str2wcs(builtin_help_get( cmd ));
	if( str )
	{
		sb_append( b, str );
		free( str );
	}
}
/**
   The bind builtin, used for setting character sequences
*/
static int builtin_bind( wchar_t **argv )
{
	int i;
	int argc=builtin_count_args( argv );
	
	woptind=0;

	const static struct woption
		long_options[] =
		{
			{
				L"set-mode", required_argument, 0, 'M' 
			}
			,
			{ 
				0, 0, 0, 0 
			}
		}
	;		
		
	while( 1 )
	{
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"M:", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_printf( sb_err,
						   L"%ls%ls %ls\n",
						   argv[0],
						   BUILTIN_ERR_UNKNOWN,
						   long_options[opt_index].name );				
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
			case 'M':		
				input_set_mode( woptarg );
				break;

			case '?':
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
		
	}		

	for( i=woptind; i<argc; i++ )
	{
//		fwprintf( stderr, L"Parse binding '%ls'\n", argv[i] );
		
		input_parse_inputrc_line( argv[i] );		
	}

	return 0;
}


/**
   The builtin builtin, used for given builtins precedence over functions. Mostly handled by the parser. All this code does is some additional operational modes, such as printing a list of all builtins.
*/
static int builtin_builtin(  wchar_t **argv )
{
	int argc=builtin_count_args( argv );
	int list=0;
	
	woptind=0;

	const static struct woption
		long_options[] =
		{
			{
				L"names", no_argument, 0, 'n' 
			}
			,
			{
				L"help", no_argument, 0, 'h' 
			}
			,
			{ 
				0, 0, 0, 0 
			}
		}
	;		
		
	while( 1 )
	{
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"nh", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							BUILTIN_ERR_UNKNOWN,
							L" ",
							long_options[opt_index].name,
							L"\n",
							(void *)0);				
				builtin_print_help( argv[0], sb_err );
	
				
				return 1;
			case 'h':
				builtin_print_help( argv[0], sb_err );
				return 0;
				
			case 'n':
				list=1;
				break;
				
			case '?':
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
		
	}		

	if( list )
	{
		array_list_t names;
		wchar_t **names_arr;
		int i;
		
		al_init( &names );
		builtin_get_names( &names );
		names_arr = list_to_char_arr( &names );
		qsort( names_arr, 
			   al_get_count( &names ), 
			   sizeof(wchar_t *), 
			   (int (*)(const void *, const void *))&wcsfilecmp );
		for( i=0; i<al_get_count( &names ); i++ )
		{
			if( wcscmp( names_arr[i], L"count" ) == 0 )
				continue;
			
			sb_append2( sb_out,
						names_arr[i],
						L"\n",
						(void *)0 );			
		}
		free( names_arr );
		al_destroy( &names );			
	}	
	return 0;
}

/**
   A generic bultin that only supports showing a help message. This is
   only a placeholder that prints the help message. Useful for
   commands that live in hte parser.
*/

static int builtin_generic( wchar_t **argv )
{
	int argc=builtin_count_args( argv );
	woptind=0;

	const static struct woption
		long_options[] =
		{
			{
				L"help", no_argument, 0, 'h'
			}
			,
			{ 
				0, 0, 0, 0 
			}
		}
	;		
		
	while( 1 )
	{
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"h", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							BUILTIN_ERR_UNKNOWN,
							L" ",
							long_options[opt_index].name,
							L"\n",
							(void *)0);				
				builtin_print_help( argv[0], sb_err );
				return 1;
				
			case 'h':
				builtin_print_help( argv[0], sb_out );
				return 0;				
				
			case '?':
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
		
	}		
	return 1;
}

/**
   The exec bultin. This is only a placeholder that prints the help message. Ther actual implementation lives in exec.c. 
*/

static int builtin_exec( wchar_t **argv )
{
	int argc=builtin_count_args( argv );
	woptind=0;

	const static struct woption
		long_options[] =
		{
			{
				L"help", no_argument, 0, 'h'
			}
			,
			{ 
				0, 0, 0, 0 
			}
		}
	;		
		
	while( 1 )
	{
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"h", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							BUILTIN_ERR_UNKNOWN,
							L" ",
							long_options[opt_index].name,
							L"\n",
							(void *)0);				
				builtin_print_help( argv[0], sb_err );
				return 1;
				
			case 'h':
				builtin_print_help( argv[0], sb_out );
				return 0;				
				
			case '?':
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
		
	}		
	return 1;
}


/**
   The functions builtin, used for listing and erasing functions.
*/
static int builtin_functions( wchar_t **argv )
{
	int i;
	int erase=0;
	wchar_t *desc=0;	

	array_list_t names;
	wchar_t **names_arr;

	int argc=builtin_count_args( argv );
	int list=0;
	int show_hidden=0;	
	int res = 0;
		
	woptind=0;

	const static struct woption
		long_options[] =
		{
			{
				L"erase", no_argument, 0, 'e' 
			}
			,
			{
				L"description", required_argument, 0, 'd' 
			}
			,
			{
				L"names", no_argument, 0, 'n' 
			}
			,
			{
				L"all", no_argument, 0, 'a'
			}
			,
			{ 
				0, 0, 0, 0 
			}
		}
	;		
		
	while( 1 )
	{
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"ed:na", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							BUILTIN_ERR_UNKNOWN,
							L" ",
							long_options[opt_index].name,
							L"\n",
							(void *)0);				
				builtin_print_help( argv[0], sb_err );
	
				
				return 1;
				
			case 'e':		
				erase=1;				
				break;

			case 'd':
				desc=woptarg;
				break;				

			case 'n':
				list=1;
				break;
			
			case 'a':
				show_hidden=1;
				break;
				
			case '?':
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
		
	}		



	/*
	  Erase, desc and list are mutually exclusive
	*/
	if( (erase + (desc!=0) + list) > 1 )
	{
		sb_append2( sb_err,
					argv[0],
					L": Invalid combination of options\n",
					(void *)0);				
		builtin_print_help( argv[0], sb_err );
		
		return 1;
	}
	

	if( erase )
	{
		int i;
		for( i=woptind; i<argc; i++ )
			function_remove( argv[i] );
		return 0;
	}
	else if( desc )
	{
		wchar_t *func;
		
		if( argc-woptind != 1 )
		{
			sb_append2( sb_err,
						L"functions: Expected exactly one function name\n",
						(void *)0);				
			builtin_print_help( argv[0], sb_err );
			
			return 1;
		}
		func = argv[woptind];
		if( !function_exists( func ) )
		{
			sb_append2( sb_err,
						L"functions: Function ",
						func,
						L" does not exist\n",
						(void *)0);				
			builtin_print_help( argv[0], sb_err );
			
			return 1;			
		}
		
		function_set_desc( func, desc );		
		
		return 0;		
	}
	else if( list )
	{
		al_init( &names );
		function_get_names( &names, show_hidden );
		names_arr = list_to_char_arr( &names );
		qsort( names_arr, 
			   al_get_count( &names ), 
			   sizeof(wchar_t *), 
			   (int (*)(const void *, const void *))&wcsfilecmp );
		for( i=0; i<al_get_count( &names ); i++ )
		{
			sb_append2( sb_out,
						names_arr[i],
						L"\n",
						(void *)0 );			
		}
		free( names_arr );
		al_destroy( &names );			
		return 0;
	}
	
	
	switch( argc - woptind )
	{
		case 0:
		{
			sb_append( sb_out, L"Current function definitions are:\n\n" );		
			al_init( &names );
			function_get_names( &names, show_hidden );
			names_arr = list_to_char_arr( &names );
			qsort( names_arr, 
				   al_get_count( &names ), 
				   sizeof(wchar_t *), 
				   (int (*)(const void *, const void *))&wcsfilecmp );
			for( i=0; i<al_get_count( &names ); i++ )
			{
				sb_append2( sb_out,
							L"function ",
							names_arr[i],
							L"\n\t",
							function_get_definition(names_arr[i]),
							L"\nend\n\n",
							(void *)0);
			}
			free( names_arr );
			al_destroy( &names );			
			break;
		}
		
		default:
		{
			
			for( i=woptind; i<argc; i++ )
			{
				if( !function_exists( argv[i] ) )
					res++;
				else
				{
					sb_append2( sb_out,
								L"function ",
								argv[i],
								L"\n\t",
								function_get_definition(argv[i]),
								L"\nend\n\n",
								(void *)0);
				}
			}
			
			
			break;
		}
	}
	return res;
	
	
}

/**
   Test whether the specified string is a valid name for a keybinding
*/
static int wcsbindingname( wchar_t *str )
{
	
	
	while( *str )
	{
		if( (!iswalnum(*str)) && (*str != L'-' ) )
		{
			return 0;
		}
		str++;
	}
	return 1;
}

static void print_block_stack( block_t *b )
{
	if( !b )
		return;
	
	wprintf( L"%ls (%d)\n", parser_get_block_desc( b->type ), b->job?b->job->job_id:-1 );
	print_block_stack( b->outer );
	
}


/**
   The function builtin, used for providing subroutines.
   It calls various functions from function.c to perform any heavy lifting.
*/
static int builtin_function( wchar_t **argv )
{	
	int argc = builtin_count_args( argv );
	int res=0;
	wchar_t *desc=0;	
	int is_binding=0;
	array_list_t *events = al_new();
		
	woptind=0;

	const static struct woption
		long_options[] =
		{
			{
				L"description", required_argument, 0, 'd' 
			}
			,
			{
				L"key-binding", no_argument, 0, 'b' 
			}
			,
			{
				L"on-signal", required_argument, 0, 's' 
			}
			,
			{
				L"on-job-exit", required_argument, 0, 'j' 
			}
			,
			{
				L"on-process-exit", required_argument, 0, 'p' 
			}
			,
			{
				L"on-variable", required_argument, 0, 'v' 
			}
			,
			{ 
				0, 0, 0, 0 
			}
		}
	;		
		
	while( 1 && (!res ) )
	{
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"bd:s:j:p:v:", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							BUILTIN_ERR_UNKNOWN,
							L" ",
							long_options[opt_index].name,
							L"\n",
							(void *)0);				
				builtin_print_help( argv[0], sb_err );
				
				res = 1;
				break;
				
				
			case 'd':		
				desc=woptarg;				
				break;

			case 'b':
				is_binding=1;
				break;
				
			case 's':
			{
				int sig = wcs2sig( woptarg );
				event_t *e;
				
				if( sig < 0 )
				{
					sb_printf( sb_err, 
							   L"%ls: Unknown signal %ls\n",
							   argv[0],
							   woptarg );
					res=1;
					break;
				}
				
				e = malloc( sizeof(event_t));
				if( !e )
					die_mem();
				e->type = EVENT_SIGNAL;
				e->param1.signal = sig;
				e->function_name=0;				
				al_push( events, e );
				break;				
			}
			
			case 'v':
			{
				event_t *e;

				if( !wcsvarname( woptarg ) )
				{
					sb_printf( sb_err, 
							   L"%ls: Invalid variable name %ls\n",
							   argv[0],
							   woptarg );
					res=1;
					break;
				}
				
				e = malloc( sizeof(event_t));
				if( !e )
					die_mem();
				e->type = EVENT_VARIABLE;
				e->param1.variable = wcsdup( woptarg );
				e->function_name=0;				
				al_push( events, e );
				break;
			}
			
			case 'j':
			case 'p':
			{
				pid_t pid;
				wchar_t *end;
				event_t *e;
				
				e = malloc( sizeof(event_t));
				if( !e )
					die_mem();

				if( ( opt == 'j' ) && 
					( wcscasecmp( woptarg, L"caller" ) == 0 ) )
				{
					int job_id = -1;
					
					if( is_subshell ) 
					{
						block_t *b = current_block;
						
//						print_block_stack( b );

						while( b && (b->type != SUBST) )
							b = b->outer;
						
						if( b )
						{
							b=b->outer;
						}
						if( b->job )
						{
//							debug( 1, L"Found block, type is %ls", parser_get_block_desc( b->type ) );
							
							job_id = b->job->job_id;
						}
						else
						{	
//							debug( 1, L"Calling block is null" );
						}
						
					}
					
					if( job_id == -1 )
					{
						sb_printf( sb_err,
								   L"%ls: Cannot find calling job for event handler\n",
								   argv[0] );
						res=1;
					}
					else
					{
						e->type = EVENT_JOB_ID;
						e->param1.job_id = job_id;
					}
										
				}
				else
				{
					errno = 0;
					pid = wcstol( woptarg, &end, 10 );	
					if( errno || !end || *end )
					{
						sb_printf( sb_err,
								   L"%ls: Invalid process id %ls\n",
								   argv[0],
								   woptarg );
						res=1;
						break;
					}				
					
					
					e->type = EVENT_EXIT;
					e->param1.pid = (opt=='j'?-1:1)*abs(pid);
				}
				if( res )
				{
					free( e );
				}
				else
				{
					e->function_name=0;		
					al_push( events, e );
				}
				break;				
			}
			
			case '?':
				builtin_print_help( argv[0], sb_err );
				res = 1;				
				break;
				
		}
		
	}		
	
	if( !res )
	{		
		if( argc-woptind != 1 )
		{
			sb_printf( sb_err, 
					   L"%ls: Expected one argument, got %d\n",
					   argv[0],
					   argc-woptind );
			res=1;
		}
		else if( !(is_binding?wcsbindingname( argv[woptind] ) : wcsvarname( argv[woptind] ) ))
		{ 
			sb_append2( sb_err, 
						argv[0],
						L": illegal function name \'", 
						argv[woptind], 
						L"\'\n", 
						(void *)0 );
			
			res=1;	
		}	
		else if( parser_is_reserved(argv[woptind] ) )
		{
			
			sb_append2( sb_err,
						argv[0],
						L": the name \'",
						argv[woptind],
						L"\' is reserved,\nand can not be used as a function name\n",
						(void *)0 );
			res=1;
		}
	}
	
	if( res )
	{
		int i;
		array_list_t names;
		wchar_t **names_arr;
		int chars=0;
		
//		builtin_print_help( argv[0], sb_err );
		
		sb_append( sb_err, L"Current functions are: " );		
		chars += wcslen( L"Current functions are: " );		
		al_init( &names );
		function_get_names( &names, 0 );
		names_arr = list_to_char_arr( &names );
		qsort( names_arr, 
			   al_get_count( &names ), 
			   sizeof(wchar_t *), 
			   (int (*)(const void *, const void *))&wcsfilecmp );
		for( i=0; i<al_get_count( &names ); i++ )
		{
			wchar_t *nxt = names_arr[i];
			int l = wcslen( nxt + 2 );
			if( chars+l > common_get_width() )
			{
				chars = 0;
				sb_append(sb_err, L"\n" );
			}
			
			sb_append2( sb_err,
						nxt, L"  ", (void *)0 );			
		}
		free( names_arr );
		al_destroy( &names );
		sb_append( sb_err, L"\n" );		

		parser_push_block( FAKE );

		al_foreach( events, (void (*)(const void *))&event_free );
		al_destroy( events );
		free( events );		
	}
	else
	{
		int i;
		
		parser_push_block( FUNCTION_DEF );
		current_block->param1.function_name=wcsdup(argv[woptind]);
		current_block->param2.function_description=desc?wcsdup(desc):0;
		current_block->param3.function_is_binding = is_binding;
		current_block->param4.function_events = events;		
		for( i=0; i<al_get_count( events ); i++ )
		{
			event_t *e = (event_t *)al_get( events, i );
			e->function_name = wcsdup( current_block->param1.function_name );
		}

	}
	
	current_block->tok_pos = parser_get_pos();
	current_block->skip = 1;
	
	return 0;
	
}

/** 
	The random builtin. For generating random numbers.
*/

static int builtin_random( wchar_t **argv )
{
	static int seeded=0;	
	int argc = builtin_count_args( argv );
	
	woptind=0;
	
	const static struct woption
		long_options[] =
		{
			{
				L"help", no_argument, 0, 'h' 
			}
			,
			{ 
				0, 0, 0, 0 
			}
		}
	;		
		
	while( 1 )
	{
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"h", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							BUILTIN_ERR_UNKNOWN,
							L" ",
							long_options[opt_index].name,
							L"\n",
							(void *)0);				
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
			case 'h':		
				builtin_print_help( argv[0], sb_err );
				break;

			case '?':
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
		
	}		

	switch( argc-woptind )
	{

		case 0:
		{
			if( !seeded )
			{
				seeded=1;
				srand( time( 0 ) );
			}
			sb_printf( sb_out, L"%d\n", rand()%32767 );
			break;
		}
		
		case 1:
		{
			int foo;
			wchar_t *end=0;
			
			errno=0;
			foo = wcstol( argv[woptind], &end, 10 );
			if( errno || *end )
			{
				sb_append2( sb_err, 
							argv[0],
							L": Seed value '" , argv[woptind], L"' is not a valid number\n", (void *)0);
				
				return 1;
			}
			seeded=1;
			srand( foo );
			break;
		}
		
		default:
		{
			sb_printf( sb_err,
					   L"%ls: Expected zero or one argument, got %d\n",
					   argc-woptind );
			builtin_print_help( argv[0], sb_err );
			return 1;
		}
	}
	return 0;	
}


/**
   The read builtin. Reads from stdin and stores the values in environment variables.
*/
static int builtin_read( wchar_t **argv )
{
	wchar_t *buff=0;
	int i, argc = builtin_count_args( argv );
	wchar_t *ifs;
	int place = ENV_USER;
	wchar_t *nxt;
	wchar_t *prompt = DEFAULT_READ_PROMPT;
	wchar_t *commandline = L"";
	
	woptind=0;

	while( 1 )
	{
		const static struct woption
			long_options[] =
			{
				{
					L"export", no_argument, 0, 'x'
				}
				,
				{
					L"global", no_argument, 0, 'g'
				}
				,
				{
					L"local", no_argument, 0, 'l'
				}
				,
				{
					L"unexport", no_argument, 0, 'u'
				}
				,
				{
					L"prompt", required_argument, 0, 'p'
				}
				,
				{
					L"command", required_argument, 0, 'c'
				}
				,
				{ 
					0, 0, 0, 0 
				}
			}
		;		
		
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"xglup:c:", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err, 
							argv[0],
							BUILTIN_ERR_UNKNOWN,
							L" ",
							long_options[opt_index].name,
							L"\n",
							(void *)0 );
				builtin_print_help( argv[0], sb_err );

				return 1;
				
			case L'x':		
				place |= ENV_EXPORT;
				break;
			case L'g':		
				place |= ENV_GLOBAL;
				break;
			case L'l':		
				place |= ENV_LOCAL;
				break;
			case L'u':		
				place |= ENV_UNEXPORT;
				break;
			case L'p':
				prompt = woptarg;
				break;
			case L'c':
				commandline = woptarg;
				break;
				
			case L'?':
				builtin_print_help( argv[0], sb_err );

				
				return 1;	
		}
		
	}		

	if( ( place & ENV_UNEXPORT ) && ( place & ENV_EXPORT ) )
	{
		sb_append2( sb_err,
					argv[0],
					BUILTIN_ERR_EXPUNEXP,
					L"\n",
					parser_current_line(),
					L"\n",
					(void *)0 );
		builtin_print_help( argv[0], sb_err );		
		return 1;		
	}
	
	if( (place&ENV_LOCAL) && (place & ENV_GLOBAL) )
	{
		sb_append2( sb_err,
					argv[0],
					BUILTIN_ERR_GLOCAL,
					L"\n",
					parser_current_line(),
					L"\n",
					(void *)0 );
		builtin_print_help( argv[0], sb_err );		
		
		return 1;		
	}
	
	if( woptind == argc )
	{
		sb_append2( sb_err,
					argv[0],
					BUILTIN_ERR_MISSING,
					L"\n",
					parser_current_line(),
					L"\n",
					(void *)0 );
		builtin_print_help( argv[0], sb_err );				
		return 1;				
	}
	
	/*
	  The call to reader_readline may change woptind, so we save it away here
	*/
	i=woptind;

	ifs = env_get( L"IFS" );
	if( ifs == 0 )
		ifs = L"";

	/*
	  Check if we should read interactively using \c reader_readline()
	*/
	if( isatty(0) && builtin_stdin == 0 )
	{				
		reader_push( READ_MODE_NAME );		
		reader_set_prompt( prompt );
				
		reader_set_buffer( commandline, wcslen( commandline ) );
		buff = wcsdup(reader_readline( ));
		reader_pop();
	}
	else
	{		
		string_buffer_t sb;
		sb_init( &sb );
		while( 1 )
		{
			int eof=0;
			int finished=0;
			
			wchar_t res=0;
			static mbstate_t state;
			memset (&state, '\0', sizeof (state));
			
			while( !finished )
			{
				char b;
				int read_res = read_blocked( builtin_stdin, &b, 1 );				
				if( read_res <= 0 )
				{
					eof=1;
					break;
				}
				
				int sz = mbrtowc( &res, &b, 1, &state );
				
				switch( sz )
				{
					case -1:
						memset (&state, '\0', sizeof (state));
						break;
						
					case -2:
						break;
					case 0:
						eof=1;
						finished = 1;
						break;
						
					default:
						finished=1;
						break;
						
				}
			}
			
			if( eof )
				break;
			if( res == L'\n' )
				break;
			
			sb_append_char( &sb, res );
		}
		buff = wcsdup( (wchar_t *)sb.buff );
		sb_destroy( &sb );		
	}
	
	wchar_t *state;
	
	nxt = wcstok( buff, (i<argc-1)?ifs:L"", &state );
//	fwprintf( stderr, L"first token %ls, %d args, start at %d\n", nxt, argc, i );
	
	while( i<argc )
	{
		env_set( argv[i], nxt != 0 ? nxt: L"", place );		

		i++;
		if( nxt != 0 )
			nxt = wcstok( 0, (i<argc-1)?ifs:L"", &state);		
	}
	
	free( buff );
	return 0;
}

static int builtin_status( wchar_t **argv )
{
	enum 
	{
		NORMAL,
		SUBST,
		BLOCK,
		INTERACTIVE,
		LOGIN
	}
	;
	
	int mode = NORMAL;
	
	int argc = builtin_count_args( argv );	
	woptind=0;
	
	const static struct woption
		long_options[] =
		{
			{
				L"help", no_argument, 0, 'h' 
			}
			,
			{
				L"is-command-substitution", no_argument, 0, 'c' 
			}
			,
			{
				L"is-block", no_argument, 0, 'b' 
			}
			,
			{ 
				L"is-interactive", no_argument, 0, 'i' 
			}
			,
			{ 
				L"is-login", no_argument, 0, 'l' 
			}
			,
			{ 
				0, 0, 0, 0 
			}
		}
	;		
		
	while( 1 )
	{
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"hcbil", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							BUILTIN_ERR_UNKNOWN,
							L" ",
							long_options[opt_index].name,
							L"\n",
							(void *)0);				
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
			case 'h':		
				builtin_print_help( argv[0], sb_err );
				break;

			case 'i':		
				mode = INTERACTIVE;
				break;
				
			case 'c':		
				mode = SUBST;
				break;
				
			case 'b':		
				mode = BLOCK;
				break;

			case 'l':		
				mode = LOGIN;
				break;

			case '?':
				builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
		
	}		

	switch( mode )
	{
		case INTERACTIVE:
			return !is_interactive_session;
			
		case SUBST:
			return !is_subshell;
			
		case BLOCK:
			return !is_block;
			
		case LOGIN:
			return !is_login;
			
	}
	
	return 0;
}


/**
   The eval builtin. Concatenates the arguments and calls eval on the
   result.
*/
static int builtin_eval( wchar_t **argv )
{
	wchar_t *tot, **ptr, *next;
	int totlen=0;

	for( ptr = argv+1; *ptr; ptr++ )
	{
		totlen += wcslen( *ptr) + 1;
	}
	tot = malloc( sizeof(wchar_t)*totlen );
	if( !tot )
	{
		die_mem();
	}
	for( ptr = argv+1, next=tot; *ptr; ptr++ )
	{
		int len = wcslen( *ptr );
		wcscpy( next, *ptr );
		next+=len;
		*next++=L' ';
	}
	*(next-1)=L'\0';
	eval( tot, block_io, TOP );
	free( tot );
	return proc_get_last_status();
}

/**
   The exit builtin. Calls reader_exit to exit and returns the value specified.
*/
static int builtin_exit( wchar_t **argv )
{	
	int argc = builtin_count_args( argv );
	
	int ec=0;
	switch( argc )
	{
		case 1:
			break;
		case 2:
		{
			wchar_t *end;
			errno = 0;			
			ec = wcstol(argv[1],&end,10);
			if( errno || *end != 0)
			{
				sb_append2( sb_err, argv[0], L": Argument must be an integer '", argv[1], L"'\n", (void *)0 );
				builtin_print_help( argv[0], sb_err );				
				return 1;
			}
			break;
		}
		
		default:
			sb_append2( sb_err, argv[0], L": Too many arguments\n", (void *)0 );
			builtin_print_help( argv[0], sb_err );				
			return 1;
				
	}
	reader_exit( 1 );
	return ec;
}

/**
   Helper function for builtin_cd, used for seting the current working directory
*/
static int set_pwd(wchar_t *env)
{
	wchar_t dir_path[4096];
	wchar_t *res = wgetcwd( dir_path, 4096 );
	if( !res )
	{
		builtin_wperror( L"wgetcwd" );
		return 0;
	}
	env_set( env, dir_path, ENV_EXPORT | ENV_GLOBAL );
	return 1;
}

/**
   The cd builtin. Changes the current directory to the one specified
   or to $HOME if none is specified. If '-' is the directory specified,
   the directory is changed to the previous working directory. The
   directory can be relative to any directory in the CDPATH variable.
*/
static int builtin_cd( wchar_t **argv )
{
	wchar_t *dir_in;
	wchar_t *dir;
	int res=0;

	if( argv[1]  == 0 )
	{
		dir_in = env_get( L"HOME" );
		if( !dir_in )
		{
			sb_append2( sb_err,
						argv[0], 
						L": Could not find home directory\n",
						(void *)0 );			
			
		}		
	}	
	else 
		dir_in = argv[1];
	
	dir = parser_cdpath_get( dir_in );	
	
	if( !dir )
	{
		sb_append2( sb_err,
					argv[0],
					L": ",
					dir_in,
					L" is not a directory or you do not have permission to enter it\n",
					(void *)0 );
		sb_append2( sb_err, 
					parser_current_line(),
					(void *)0 );			
		return 1;
	}		

	if( wchdir( dir ) != 0 )
	{
		sb_append2( sb_err,
					argv[0],
					L": ",
					dir,
					L" is not a directory\n",
					(void *)0 );
		sb_append2( sb_err, 
					parser_current_line(),
					(void *)0 );
		
		free( dir );
		
		return 1;
	}

	if (!set_pwd(L"PWD"))
	{
		res=1;
		sb_append( sb_err, L"Could not set PWD variable\n" );		
	}
	
//	fwprintf( stderr, L"cd '%ls' -> '%ls', set PWD to '%ls'\n", argv[1]?argv[1]:L"-", dir, env_get( L"PWD" ) );
	
	free( dir );
	
	return res;
}

/**
   The complete builtin. Used for specifying programmable
   tab-completions. Calls the functions in complete.c for any heavy
   lifting.
*/
static int builtin_complete( wchar_t **argv )
{
	
	int argc=0;
	int result_mode=SHARED, long_mode=0;
	int cmd_type=-1;
	int remove = 0;
	int authorative = 1;
	
	wchar_t *cmd=0, short_opt=L'\0', *long_opt=L"", *comp=L"", *desc=L"", *condition=L"", *load=0;
	
	argc = builtin_count_args( argv );	
	
	woptind=0;
	
	while( 1 )
	{
		const static struct woption
			long_options[] =
			{
				{
					L"exclusive", no_argument, 0, 'x' 
				}
				,
				{
					L"no-files", no_argument, 0, 'f' 
				}
				,
				{
					L"require-parameter", no_argument, 0, 'r' 
				}
				,
				{
					L"path", required_argument, 0, 'p'
				}
				,					
				{
					L"command", required_argument, 0, 'c' 
				}
				,					
				{
					L"short-option", required_argument, 0, 's' 
				}
				,
				{
					L"long-option", required_argument, 0, 'l'				}
				,
				{
					L"old-option", required_argument, 0, 'o' 
				}
				,
				{
					L"description", required_argument, 0, 'd'
				}
				,
				{
					L"arguments", required_argument, 0, 'a'
				}
				,
				{
					L"erase", no_argument, 0, 'e'
				}
				,
				{
					L"unauthorative", no_argument, 0, 'u'
				}
				,
				{
					L"condition", required_argument, 0, 'n'
				}
				,
				{
					L"load", required_argument, 0, 'y'
				}
				,
				{ 
					0, 0, 0, 0 
				}
			}
		;		
		
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"a:c:p:s:l:o:d:frxeun:y:", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							L": Unknown option ",
							long_options[opt_index].name,
							L"\n",
							(void *)0 );
				sb_append( sb_err, 
						   parser_current_line() );
//				builtin_print_help( argv[0], sb_err );

				
				return 1;
				
				
			case 'x':					
				result_mode |= EXCLUSIVE;
				break;
					
			case 'f':					
				result_mode |= NO_FILES;
				break;
				
			case 'r':					
				result_mode |= NO_COMMON;
				break;
					
			case 'p':					
				cmd_type = PATH;
				cmd = expand_unescape( woptarg, 1);
				break;
					
			case 'c':
				cmd_type = COMMAND;
				cmd = expand_unescape( woptarg, 1);
				break;
				
			case 'd':
				desc = woptarg;
				break;
				
			case 'u':
				authorative=0;
				break;
				
			case 's':
				if( wcslen( woptarg ) > 1 )
				{
					sb_append2( sb_err,
								argv[0],
								L": Parameter too long ",
								woptarg,
								L"\n",
								(void *)0);
					sb_append( sb_err, 
							   parser_current_line() );
//				builtin_print_help( argv[0], sb_err );
					
					return 1;
				}
				
				short_opt = woptarg[0];
				break;
					
			case 'l':
				long_opt = woptarg;
				break;
				
			case 'o':
				long_mode=1;				
				long_opt = woptarg;
				break;

			case 'a':
				comp = woptarg;
				break;
				

			case 'e':
				remove = 1;
				
				break;

			case 'n':
				condition = woptarg;
				break;
				
			case 'y':
				load = woptarg;
				break;
				

			case '?':
				//	builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
		
	}
	
	if( woptind != argc )
	{
		sb_append2( sb_err, 
					argv[0],
					L": Too many arguments\n",
					(void *)0);
		sb_append( sb_err, 
				   parser_current_line() );
		//			builtin_print_help( argv[0], sb_err );

		return 1;
	}

	if( load )
	{
		complete_load( load, 1 );		
		return 0;		
	}
	

	if( cmd == 0 )
	{
		/* No arguments specified, meaning we print the definitions of
		 * all specified completions to stdout.*/
		complete_print( sb_out );		
	}
	else
	{
		if( remove )
		{
			/* Remove the specified completion */
			complete_remove( cmd, 
							 cmd_type, 
							 short_opt,
							 long_opt );
		}
		else
		{
			/* Add the specified completion */
			complete_add( cmd, 
						  cmd_type, 
						  short_opt,
						  long_opt,
						  long_mode, 
						  result_mode, 
						  authorative,
						  condition,
						  comp,
						  desc ); 
		}
		free( cmd );
		
	}	
	return 0;
}

/**
   The source builtin. Can be called through either 'source' or
   '.'. Evaluates the contents of a file. 
*/
static int builtin_source( wchar_t ** argv )
{
	int fd;
	int res;
	
/*
  if( wcsstr( argv[1], L"fish_complete" ) )
  {
  fwprintf( stderr, L"Woot\n" );
  return 0;
  }
*/

	if( (argv[1] == 0) || (argv[2]!=0) )
	{
		
		sb_append2( sb_err, argv[0], L": Expected exactly one argument\n", (void *)0 );
		builtin_print_help( argv[0], sb_err );

		return 1;
	}
	
	if( ( fd = wopen( argv[1], O_RDONLY ) ) == -1 )
	{		
		builtin_wperror( L"open" );
		res = 1;
	}
	else
	{
		reader_push_current_filename( argv[1] );

		/*
		  Push a new non-shadowwing variable scope to the stack. That
		  way one can use explicitly local variables in sourced files
		  that will die on return to the calling file.
		*/
		env_push(0);		
		res = reader_read( fd );		
		env_pop();
		if( res )
		{
			sb_printf( sb_err,
					   L"%ls : Error while reading file '%ls'\n",
					   argv[0],
					   argv[1]
				);
		}

		/*
		  Do not close fd after calling reader_read. reader_read
		  automatically closes it before calling eval.
		*/

		reader_pop_current_filename();
	}

	return res;
}


/**
   Make the specified job the first job of the job list. Moving jobs
   around in the list makes the list reflect the order in which the
   jobs were used.
*/
static void make_first( job_t *j )
{
	job_t *prev=0;
	job_t *curr;
	for( curr = first_job; curr != j; curr = curr->next )
	{
		prev=curr;
	}
	if( curr == j )
	{
		if( prev == 0 )
			return;
		else
		{
			prev->next = curr->next;
			curr->next = first_job;
			first_job = curr;
		}
	}
}


/**
   Builtin for putting a job in the foreground
*/
static int builtin_fg( wchar_t **argv )
{
	job_t *j;
	
	if( argv[1] == 0 )
	{
		/*
		  Last constructed job in the job que by default
		*/
		for( j=first_job; ((j!=0) && (!j->constructed)); j=j->next )
			;
	}
	else if( argv[2] != 0 )
	{
		/*
		  Specifying what more than one job to put to the foreground
		  is a syntax error, we still try to locate the job argv[1],
		  since we want to know if this is an ambigous job
		  specification or if this is an malformed job id
		*/
		int pid = wcstol( argv[1], 0, 10 );
		j = job_get_from_pid( pid );
		if( j != 0 )
		{
			sb_append2( sb_err, 
						argv[0],
						L": Ambiguous job\n",
						(void *)0);	
		}
		else
		{
			sb_append2( sb_err,
						argv[0], 
						L": Not a job (", 
						argv[1], 
						L")\n", (void *)0 );
		}
		builtin_print_help( argv[0], sb_err );
		
		return 1;
	}
	else
	{
		int pid = abs(wcstol( argv[1], 0, 10 ));
		j = job_get_from_pid( pid );
	}

	if( j == 0 )
	{
		sb_append2( sb_err,
					argv[0], 
					L": No suitable job\n",
					(void *)0);
		builtin_print_help( argv[0], sb_err );
		return 1;
	}
	else
	{
		if( builtin_err_redirect )
		{
			sb_printf( sb_err, 
					   L"Send job %d, '%ls' to foreground\n", 
					   j->job_id, 
					   j->command );
		}
		else
		{
			fwprintf( stderr,
					  L"Send job %d, '%ls' to foreground\n", 
					  j->job_id, 
					  j->command );
		}
	}
	
	wchar_t *ft = tok_first( j->command );
	if( ft != 0 )
		env_set( L"_", ft, ENV_EXPORT );
	free(ft);
	reader_write_title();
/*
  fwprintf( stderr, L"Send job %d, \'%ls\' to foreground\n", 
  j->job_id,
  j->command );
*/
	make_first( j );
	j->fg=1;
	
		
	job_continue( j, job_is_stopped(j) );
	return 0;
}

/**
   Helper function for builtin_bg()
*/
static void send_to_bg( job_t *j, wchar_t *name )
{
	if( j == 0 )
	{
		sb_append2( sb_err, L"bg", L": Unknown job ", name, L"\n", (void *)0 );
		builtin_print_help( L"bg", sb_err );
		return;
	}	
	else
	{
		sb_printf( sb_err, 
				   L"Send job %d '%ls' to background\n",
				   j->job_id,
				   j->command );
	}
	make_first( j );
	j->fg=0;
	job_continue( j, job_is_stopped(j) );
}


/**
   Builtin for putting a job in the background
*/
static int builtin_bg( wchar_t **argv )
{
	if( argv[1] == 0 )
	{
  		job_t *j;
		for( j=first_job; ((j!=0) && (!j->constructed) && (!job_is_stopped(j))); j=j->next )
			;
		send_to_bg( j, L"(default)");
		return 0;
	}
	for( argv++; *argv != 0; argv++ )
	{
		int pid = wcstol( *argv, 0, 10 );
		send_to_bg( job_get_from_pid( pid ), *argv);
	}
	return 0;
}


#ifdef HAVE__PROC_SELF_STAT
/**
   Calculates the cpu usage (in percent) of the specified job.
*/
static int cpu_use( job_t *j )
{
	double u=0;
	process_t *p;
	
	for( p=j->first_process; p; p=p->next )
	{
		struct timeval t;
		int jiffies;
		gettimeofday( &t, 0 );
		jiffies = proc_get_jiffies( p );
		
		double t1 = 1000000.0*p->last_time.tv_sec+p->last_time.tv_usec;
		double t2 = 1000000.0*t.tv_sec+t.tv_usec;
		
/*		fwprintf( stderr, L"t1 %f t2 %f p1 %d p2 %d\n",
  t1, t2, jiffies, p->last_jiffies );
*/	

		u += ((double)(jiffies-p->last_jiffies))/(t2-t1);
	}
	return u*1000000;
}
#endif

/**
   Builtin for printing running jobs
*/
static int builtin_jobs( wchar_t **argv )
{	

	enum
	{
		DEFAULT,
		PRINT_PID,
		PRINT_COMMAND
	}
	;
	
	
	int argc=0;
	job_t *j;
	int found=0;	
	int mode=DEFAULT;
	argc = builtin_count_args( argv );	
	
	woptind=0;

	while( 1 )
	{
		const static struct woption
			long_options[] =
			{
				{
					L"pid", no_argument, 0, 'p'
				}
				,
				{
					L"command", no_argument, 0, 'c'
				}
				,
				{ 
					0, 0, 0, 0 
				}
			}
		;		
		
		int opt_index = 0;
		
		int opt = wgetopt_long( argc,
								argv, 
								L"pc", 
								long_options, 
								&opt_index );
		if( opt == -1 )
			break;
			
		switch( opt )
		{
			case 0:
				if(long_options[opt_index].flag != 0)
					break;
				sb_append2( sb_err,
							argv[0],
							L": Unknown option ",
							long_options[opt_index].name,
							L"\n",
							(void *)0 );
				sb_append( sb_err, 
						   parser_current_line() );
//				builtin_print_help( argv[0], sb_err );

				
				return 1;
				
				
			case 'p':					
				mode=PRINT_PID;
				break;
					
			case 'c':					
				mode=PRINT_COMMAND;
				break;

			case '?':
				//	builtin_print_help( argv[0], sb_err );
				
				return 1;
				
		}
	}	
	
	if( mode==DEFAULT )
	{
		
		for( j= first_job; j; j=j->next )
		{
			/*
			  Ignore unconstructed jobs, i.e. ourself.
			*/
			if( j->constructed /*&& j->skip_notification*/ )
			{
				if( !found )
				{
					/*
					  Print table header before first job
					*/
					sb_append( sb_out, L"Job\tGroup\t");
#ifdef HAVE__PROC_SELF_STAT
					sb_append( sb_out, L"CPU\t" );
#endif
					sb_append( sb_out, L"State\tCommand\n" );
				}
			
				found = 1;
			
				sb_printf( sb_out, L"%d\t%d\t", j->job_id, j->pgid );
				
#ifdef HAVE__PROC_SELF_STAT
				sb_printf( sb_out, L"%d\t", cpu_use(j) );
#endif
				sb_append2( sb_out, job_is_stopped(j)?L"stopped\t":L"running\t", 
//							job_is_completed(j)?L"completed\t":L"unfinished\t", 
							j->command, L"\n", (void *)0 );
			
			}
		}
		if( !found )
		{
			sb_append2( sb_out, argv[0], L": There are no running jobs\n", (void *)0 );
		}
	}
	else
	{
		long pid;
		wchar_t *end;			
		job_t *j;
		
		if( woptind != argc-1 )
		{
			sb_append2( sb_err, argv[0], L": Expected exactly one argument\n", (void *)0 );
		}

			
		errno=0;
		pid=wcstol( argv[woptind], &end, 10 );
		if( errno || *end )
		{
			sb_append2( sb_err, argv[0], L": Not a process id: ", argv[woptind], L"\n", (void *)0 );
			return 1;
				
		}

		j = job_get_from_pid( pid );
		if( !j )
		{
			sb_printf( sb_err, L"%ls: No suitable job: %d\n", argv[0], pid );
			return 1;
		}
		process_t *p;
		for( p=j->first_process; p; p=p->next )
		{
			switch( mode )
			{
				case PRINT_PID:
				{
					sb_printf( sb_out, L"%d\n", p->pid );
					break;						
				}
				
				case PRINT_COMMAND:
				{
					sb_printf( sb_out, L"%ls\n", p->argv[0] );
					break;						
				}
			}
		}		
	}
	
	return 0;
}

/**
   Builtin for looping over a list
*/
static int builtin_for( wchar_t **argv )
{
	int argc = builtin_count_args( argv );
	int res=1;
	

	if( argc < 3) 
	{
		sb_append2( sb_err, 
					argv[0],
					L": Expected at least two arguments\n",
					(void *)0);				
		builtin_print_help( argv[0], sb_err );
	}
	else if ( !wcsvarname(argv[1]) )
	{
		sb_append2( sb_err, 
					argv[0],
					L": \'",
					argv[1],
					L"\' invalid variable name\n",
					(void *)0);				
		builtin_print_help( argv[0], sb_err );
	}
	else if (wcscmp( argv[2], L"in") != 0 )
	{
		sb_append2( sb_err,
					argv[0],
					L": Second argument must be \'in\'\n",
					(void *)0);				
		builtin_print_help( argv[0], sb_err );
	}
	else
	{
		res=0;
	}

	
	if( res )
	{
		parser_push_block( FAKE );
	}
	else
	{
		parser_push_block( FOR );
		al_init( &current_block->param2.for_vars);
		
		int i;
		current_block->tok_pos = parser_get_pos();
		current_block->param1.for_variable = wcsdup( argv[1] );
		
		for( i=argc-1; i>3; i-- )
		{
			al_push( &current_block->param2.for_vars, wcsdup(argv[ i ] ));
		}
		if( argc > 3 )
		{
			env_set( current_block->param1.for_variable, argv[3], ENV_LOCAL );
		}
		else
		{
			current_block->skip=1;
		}	
	}
	return res;
}

static int builtin_begin( wchar_t **argv )
{
	parser_push_block( BEGIN );
	current_block->tok_pos = parser_get_pos();
	return 0;
}
 

/**
   Builtin for ending a block of code, such as a for-loop or an if statement.

   The end command is whare a lot of the block-level magic happens. 
*/
static int builtin_end( wchar_t **argv )
{
	if( !current_block->outer ||
		current_block->type == OR ||
		current_block->type == AND )
	{
		sb_append2( sb_err,
					argv[0],
					L": Not inside of block\n",
					(void *)0);
		builtin_print_help( argv[0], sb_err );
		return 1;
	}
	else
	{
		/**
		   By default, 'end' kills the current block scope. But if we
		   are rewinding a loop, this should be set to false, so that
		   variables in the current loop scope won't die between laps.
		*/
		int kill_block = 1;
		
		switch( current_block->type )
		{
			case WHILE:
			{
				/*
				  If this is a while loop, we rewind the loop unless
				  it's the last lap, in which case we continue.
				*/
				if( !( current_block->skip && (current_block->loop_status != LOOP_CONTINUE )))
				{
					current_block->loop_status = LOOP_NORMAL;
					current_block->skip = 0;
					kill_block = 0;
					parser_set_pos( current_block->tok_pos);
					current_block->param1.while_state = WHILE_TEST_AGAIN;
				}
				
				break;
			}
			
			case IF:
			case SUBST:
			case BEGIN:
				/*
				  Nothing special happens at the end of these. The scope just ends.
				*/
				
				break;

			case FOR:
			{
				/*
				  set loop variable to next element, and rewind to the beginning of the block.
				*/
				if( current_block->loop_status == LOOP_BREAK )
				{
					while( al_get_count( &current_block->param2.for_vars ) )
					{
						free( (void *)al_pop( &current_block->param2.for_vars ) );
					}
				}
				
				if( al_get_count( &current_block->param2.for_vars ) )
				{
					wchar_t *val = (wchar_t *)al_pop( &current_block->param2.for_vars );
					env_set( current_block->param1.for_variable, val,  ENV_LOCAL);
					current_block->loop_status = LOOP_NORMAL;
					current_block->skip = 0;
					free(val);
					
					kill_block = 0;
					parser_set_pos( current_block->tok_pos );
/*					
  fwprintf( stderr, 
  L"jump to %d\n",
  current_block->tok_pos );								*/
				}
				break;
			}
		
			case FUNCTION_DEF:
			{
				/**
				   Copy the text from the beginning of the function
				   until the end command and use as the new definition
				   for the specified function
				*/
				wchar_t *def = wcsndup( parser_get_buffer()+current_block->tok_pos, 
										parser_get_job_pos()-current_block->tok_pos );
				
				//fwprintf( stderr, L"Function: %ls\n", def );
				if( !parser_test( def, 1 ) )
				{
					function_add( current_block->param1.function_name, 
								  def,
								  current_block->param2.function_description,
								  current_block->param4.function_events,
								  current_block->param3.function_is_binding );
				}				
				
				free(def);
			}
			break;
			
		}
		if( kill_block )
		{
			parser_pop_block();
		}
//		fwprintf( stderr, L"End with status %d\n", proc_get_last_status() );
		

		/*
		  If everything goes ok, return status of last command to execute.
		*/
		return proc_get_last_status();
	}	
}

/**
   Builtin for executing commands if an if statement is false
*/
static int builtin_else( wchar_t **argv )
{
	if( current_block == 0 || 
		current_block->type != IF ||
		current_block->param1.if_state != 1)
	{
		sb_append2( sb_err,
					argv[0],
					L": not inside of if block\n",
					(void *)0);
		builtin_print_help( argv[0], sb_err );
		return 1;
	}
	else
	{
		current_block->param1.if_state++;
		current_block->skip = !current_block->skip;
		env_pop();
		env_push(0);
	}

	/*
	  If everything goes ok, return status of last command to execute.
	*/
	return proc_get_last_status();
}

/**
   This function handles both the 'continue' and the 'break' builtins
   that are used for loop control.
*/
static int builtin_break_continue( wchar_t **argv )
{
	int is_break = (wcscmp(argv[0],L"break")==0);
	int argc = builtin_count_args( argv );
	
	block_t *b = current_block;
	
	if( argc != 1 )
	{
		sb_append2( sb_err, 
					argv[0], 
					L": Unknown option \'", argv[1], L"\'", (void *)0 );
		builtin_print_help( argv[0], sb_err );
		return 1;		
	}
	

	while( (b != 0) && 
		   ( b->type != WHILE) && 
		   (b->type != FOR ) )
	{
		b = b->outer;
	}
	
	if( b == 0 )
	{
		sb_append2( sb_err, 
					argv[0], 
					L": Not inside of loop\n", (void *)0 );
		builtin_print_help( argv[0], sb_err );
		return 1;
	}
	
	b = current_block;
	while( ( b->type != WHILE) && 
		   (b->type != FOR ) )
	{
		b->skip=1;
		b = b->outer;
	}
	b->skip=1;
	b->loop_status = is_break?LOOP_BREAK:LOOP_CONTINUE;
	return 0;
}

/**
   Function for handling the \c return builtin
*/
static int builtin_return( wchar_t **argv )
{
	int argc = builtin_count_args( argv );
	int status = 0;	
	
	block_t *b = current_block;
	
	switch( argc )
	{
		case 1:
			break;
		case 2:
		{
			wchar_t *end;
			errno = 0;			
			status = wcstol(argv[1],&end,10);
			if( errno || *end != 0)
			{
				sb_append2( sb_err,
							argv[0], 
							L": Argument must be an integer '", 
							argv[1],
							L"'\n", 
							(void *)0 );
				builtin_print_help( argv[0], sb_err );				
				return 1;
			}
//			fwprintf( stderr, L"Return with status %d\n", status );
			break;			
		}
		default:
			sb_append2( sb_err, 
						argv[0], 
						L": Too many arguments\n", (void *)0 );
			builtin_print_help( argv[0], sb_err );
			return 1;		
	}
	

	while( (b != 0) && 
		   ( b->type != FUNCTION_CALL)  )
	{
		b = b->outer;
	}
	
	if( b == 0 )
	{
		sb_append2( sb_err, 
					argv[0], 
					L": Not inside of function\n", (void *)0 );
		builtin_print_help( argv[0], sb_err );
		return 1;
	}
	
	b = current_block;
	while( ( b->type != FUNCTION_CALL))
	{
		b->skip=1;
		b = b->outer;
	}
	b->skip=1;
//	proc_set_last_status( status );
	
	return status;
}

/**
   Builtin for executing one of several blocks of commands depending on the value of an argument.
*/
static int builtin_switch( wchar_t **argv )
{
	int res=0;	
	int argc = builtin_count_args( argv );
	
	if( argc != 2 )
	{
		sb_printf( sb_err, 
				   L"%ls : syntax error, expected exactly one argument, got %d\n",
				   argv[0],
				   argc-1 );
		
		builtin_print_help( argv[0], sb_err );
		res=1;
		parser_push_block( FAKE );
	}
	else
	{
		parser_push_block( SWITCH );
		current_block->param1.switch_value = wcsdup( argv[1]);
		current_block->skip=1;
		current_block->param2.switch_taken=0;
	}
	
	return res;
}

/**
   Builtin used together with the switch builtin for conditional execution
*/
static int builtin_case( wchar_t **argv )
{
	int argc = builtin_count_args( argv );
	int i;
	wchar_t *unescaped=0;

	if( current_block->type != SWITCH )
	{
		sb_append2( sb_err,
					argv[0],
					L": syntax error, case command while not in switch block\n",
					(void *)0);
		builtin_print_help( L"case", sb_err );
		return 1;
	}
		
	current_block->skip = 1;

	if( current_block->param2.switch_taken )
	{
		return 0;
	}
	
	for( i=1; i<argc; i++ )
	{
		free( unescaped );
		unescaped = expand_unescape( argv[i], 1);

		if( wildcard_match( current_block->param1.switch_value, unescaped ) )
		{
			current_block->skip = 0;
			current_block->param2.switch_taken = 1;
			break;		
		}
	}
	free( unescaped );
	
	return 0;		
}


/*
  END OF BUILTIN COMMANDS
  Below are functions for handling the builtin commands
*/
void builtin_init()
{
	al_init( &io_stack );
	hash_init( &builtin, &hash_wcs_func, &hash_wcs_cmp );

	hash_put( &builtin, L"exit", (void*) &builtin_exit );	
	hash_put( &builtin, L"builtin", (void*) &builtin_builtin );	
	hash_put( &builtin, L"cd", (void*) &builtin_cd );
	hash_put( &builtin, L"function", (void*) &builtin_function );	
	hash_put( &builtin, L"functions", (void*) &builtin_functions );	
	hash_put( &builtin, L"complete", (void*) &builtin_complete );	
	hash_put( &builtin, L"end", (void*) &builtin_end );
	hash_put( &builtin, L"else", (void*) &builtin_else );
	hash_put( &builtin, L"eval", (void*) &builtin_eval );
	hash_put( &builtin, L"for", (void*) &builtin_for );
	hash_put( &builtin, L".", (void*) &builtin_source );
	hash_put( &builtin, L"set", (void*) &builtin_set );
	hash_put( &builtin, L"fg", (void*) &builtin_fg );
	hash_put( &builtin, L"bg", (void*) &builtin_bg );
	hash_put( &builtin, L"jobs", (void*) &builtin_jobs );
	hash_put( &builtin, L"read", (void*) &builtin_read );
	hash_put( &builtin, L"break", (void*) &builtin_break_continue );	
	hash_put( &builtin, L"continue", (void*) &builtin_break_continue );
	hash_put( &builtin, L"return", (void*) &builtin_return );
	hash_put( &builtin, L"commandline", (void*) &builtin_commandline );
	hash_put( &builtin, L"switch", (void*) &builtin_switch );
	hash_put( &builtin, L"case", (void*) &builtin_case );
	hash_put( &builtin, L"bind", (void*) &builtin_bind );
	hash_put( &builtin, L"random", (void*) &builtin_random );	
	hash_put( &builtin, L"status", (void*) &builtin_status );	
	hash_put( &builtin, L"ulimit", (void*) &builtin_ulimit );	
	
	/* 
	   Builtins that are handled directly by the parser. They are
	   bound to a noop function only so that they show up in the
	   listings of builtin commands, etc..
	*/
	hash_put( &builtin, L"command", (void*)  &builtin_ignore );		
	hash_put( &builtin, L"if", (void*) &builtin_ignore );	
	hash_put( &builtin, L"while", (void*) &builtin_ignore );	
	hash_put( &builtin, L"not", (void*) &builtin_generic );	
	hash_put( &builtin, L"and", (void*) &builtin_generic );	
	hash_put( &builtin, L"or", (void*) &builtin_generic );	
	hash_put( &builtin, L"exec", (void*) &builtin_exec );	
	hash_put( &builtin, L"begin", (void*) &builtin_begin );	

	/*
	  This is not a builtin, but fish handles it's help display
	  internally, to do some ugly special casing to make sure 'count
	  -h', but 'count (echo -h)' does not.
	*/
	hash_put( &builtin, L"count", (void*) &builtin_ignore );	

	intern_static( L"exit" );	
	intern_static( L"builtin" );	
	intern_static( L"cd" );
	intern_static( L"function" );	
	intern_static( L"functions" );	
	intern_static( L"complete" );	
	intern_static( L"end" );
	intern_static( L"else" );
	intern_static( L"eval" );
	intern_static( L"for" );
	intern_static( L"." );
	intern_static( L"set" );
	intern_static( L"fg" );
	intern_static( L"bg" );
	intern_static( L"jobs" );
	intern_static( L"read" );
	intern_static( L"break" );	
	intern_static( L"continue" );
	intern_static( L"return" );
	intern_static( L"commandline" );
	intern_static( L"switch" );
	intern_static( L"case" );
	intern_static( L"bind" );
	intern_static( L"random" );	
	intern_static( L"command" );		
	intern_static( L"if" );	
	intern_static( L"while" );	
	intern_static( L"exec" );	
	intern_static( L"count" );	
	intern_static( L"not" );	
	intern_static( L"and" );	
	intern_static( L"or" );	
	intern_static( L"begin" );	
	intern_static( L"status" );	
	intern_static( L"ulimit" );	
	
	builtin_help_init();
}

void builtin_destroy()
{
	if( desc )
	{
		hash_destroy( desc );	
		free( desc );
	}
	
	al_destroy( &io_stack );
	hash_destroy( &builtin );
	builtin_help_destroy();
}

int builtin_exists( wchar_t *cmd )
{
	/*
	  Count is not a builtin, but it's help is handled internally by
	  fish, so it is in the hash_table_t.
	*/
	if( wcscmp( cmd, L"count" )==0)
		return 0;
	
	return (hash_get(&builtin, cmd) != 0 );
}

/**
   Return true if the specified builtin should handle it's own help,
   false otherwise.
*/
static int internal_help( wchar_t *cmd )
{
	if( wcscmp( cmd, L"for" ) == 0 ||
		wcscmp( cmd, L"while" ) == 0 ||
		wcscmp( cmd, L"function" ) == 0 ||
		wcscmp( cmd, L"if" ) == 0 ||
		wcscmp( cmd, L"end" ) == 0 ||
		wcscmp( cmd, L"switch" ) == 0 ) 
		return 1;
	return 0;
}


int builtin_run( wchar_t **argv )
{
	int (*cmd)(wchar_t **argv)=0;
	cmd = hash_get( &builtin, argv[0] );
	
	if( argv[1] != 0 && !internal_help(argv[0]) )
	{
		if( argv[2] == 0 && (parser_is_help( argv[1], 0 ) ) )
		{
			builtin_print_help( argv[0], sb_out );
			return 0;
		}
	}
	
	if( cmd != 0 )
	{
		int status;
		
		status = cmd(argv);
//				fwprintf( stderr, L"Builtin: Set status of %ls to %d\n", argv[0], status );
		
		return status;
		
	}
	else
	{
		debug( 0, L"Unknown builtin: ", argv[0], 0 );
	}
	return 1;	
}


void builtin_get_names( array_list_t *list )
{
 	hash_get_keys( &builtin, list );
}

const wchar_t *builtin_get_desc( const wchar_t *b )
{
	
	if( !desc )
	{
		desc = malloc( sizeof( hash_table_t ) );
		if( !desc) 
			return 0;
		
		hash_init( desc, &hash_wcs_func, &hash_wcs_cmp );

		hash_put( desc, L"exit", L"Exit the shell" );	
		hash_put( desc, L"cd", L"Change working directory" );	
		hash_put( desc, L"function", L"Define a new function" );	
		hash_put( desc, L"functions", L"List or remove functions" );	
		hash_put( desc, L"complete", L"Edit command specific completions" );	
		hash_put( desc, L"end", L"End a block of commands" );
		hash_put( desc, L"else", L"Evaluate block if condition is false" );
		hash_put( desc, L"eval", L"Evaluate parameters as a command" );	
		hash_put( desc, L"for", L"Perform a set of commands multiple times" );
		hash_put( desc, L".", L"Evaluate contents of file" );
		hash_put( desc, L"set", L"Handle environment variables" );
		hash_put( desc, L"fg", L"Send job to foreground" );
		hash_put( desc, L"bg", L"Send job to background" );
		hash_put( desc, L"jobs", L"Print currently running jobs" );
		hash_put( desc, L"read", L"Read a line of input into variables" );
		hash_put( desc, L"break", L"Stop the innermost loop" );	
		hash_put( desc, L"continue", L"Skip the rest of the current lap of the innermost loop" );
		hash_put( desc, L"return", L"Stop the innermost currently evaluated function" );
		hash_put( desc, L"commandline", L"Set the commandline" );
		hash_put( desc, L"switch", L"Conditionally execute a block of commands" );	
		hash_put( desc, L"case", L"Conditionally execute a block of commands" );	
		hash_put( desc, L"builtin", L"Run a builtin command" );	
		hash_put( desc, L"command", L"Run a program" );		
		hash_put( desc, L"if", L"Conditionally execute a command" );	
		hash_put( desc, L"while", L"Perform a command multiple times" );	
		hash_put( desc, L"bind", L"Handle key bindings");
		hash_put( desc, L"random", L"Generate random number");
		hash_put( desc, L"exec", L"Run command in current process");
		hash_put( desc, L"not", L"Negate exit status of job");
		hash_put( desc, L"or", L"Execute second command if first fails");
		hash_put( desc, L"and", L"Execute second command if first suceeds");
		hash_put( desc, L"begin", L"Create a block of code" );
		hash_put( desc, L"status", L"Return status information about fish" );
		hash_put( desc, L"ulimit", L"Set or get the shells resurce usage limits" );
	}

	return hash_get( desc, b );	
}


void builtin_push_io( int in)
{
	if( builtin_stdin != -1 )
	{
		al_push( &io_stack, (void *)(long)builtin_stdin );
		al_push( &io_stack, sb_out );
		al_push( &io_stack, sb_err );		
	}
	builtin_stdin = in;
	sb_out = malloc(sizeof(string_buffer_t));
	sb_err = malloc(sizeof(string_buffer_t));
	sb_init( sb_out );
	sb_init( sb_err );	
}

void builtin_pop_io()
{
	builtin_stdin = 0;
	sb_destroy( sb_out );
	sb_destroy( sb_err );
	free( sb_out);
	free(sb_err);
	
	if( al_get_count( &io_stack ) >0 )
	{
		sb_err = (string_buffer_t *)al_pop( &io_stack );
		sb_out = (string_buffer_t *)al_pop( &io_stack );
		builtin_stdin = (int)(long)al_pop( &io_stack );
	}
	else
	{
		sb_out = sb_err = 0;
		builtin_stdin = 0;
	}
}

