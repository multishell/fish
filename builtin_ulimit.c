/** \file builtin_commandline.c Functions defining the commandline builtin

Functions used for implementing the commandline builtin. 

*/
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>

#include "config.h"
#include "util.h"
#include "builtin.h"
#include "common.h"
#include "wgetopt.h"

struct resource_t
{
	int resource;
	const wchar_t *desc;
	wchar_t switch_char;	
}
	;

const static struct resource_t resource_arr[] =
{
	{
		RLIMIT_CORE, L"Maximum size of core files created", L'c'
	}
	,
	{
		RLIMIT_DATA, L"Maximum size of a process’s data segment", L'd'
	}
	,
	{
		RLIMIT_FSIZE, L"Maximum size of files created by the shell", L'f'
	}
	,
	{
		RLIMIT_MEMLOCK, L"Maximum size that may be locked into memory", L'l'
	}
	,
	{
		RLIMIT_RSS, L"Maximum resident set size", L'm'
	}
	,
	{
		RLIMIT_NOFILE, L"Maximum number of open file descriptors", L'n'
	}
	,
	{
		RLIMIT_STACK, L"Maximum stack size", L's'
	}
	,
	{
		RLIMIT_CPU, L"Maximum amount of cpu time in seconds", L't'
	}
	,
	{
		RLIMIT_NPROC, L"Maximum number of processes available to a single user", L'u'
	}
	,
#if HAVE_RLIMIT_AS
	{
		RLIMIT_AS, L"Maximum amount of virtual memory available to the shell", L'v'
	}	
	,
#endif
	{
		0, 0
	}
}
	;

static int get_multiplier( int what )
{
	if( ( what == RLIMIT_NPROC ) || 
		( what == RLIMIT_NOFILE ) || 
		( what == RLIMIT_CPU ) )
	{
		return 1;
	}
	return 1024;
}


static rlim_t get( int resource, int hard )
{
	struct rlimit ls;
	
	getrlimit( resource, &ls );
	
	return hard ? ls.rlim_max:ls.rlim_cur;
}

static void print( int resource, int hard )
{
	rlim_t l = get( resource, hard );

	if( l == RLIM_INFINITY )
		sb_append( sb_out, L"unlimited\n" );
	else
		sb_printf( sb_out, L"%d\n", l );
	
}


static void print_all( int hard )
{
	int i;
	int w=0;
	
	for( i=0; resource_arr[i].desc; i++ )
	{
		w=maxi( w, my_wcswidth(resource_arr[i].desc));
	}
	
	for( i=0; resource_arr[i].desc; i++ )
	{
		struct rlimit ls;
		rlim_t l;
		getrlimit( resource_arr[i].resource, &ls );
		l = hard ? ls.rlim_max:ls.rlim_cur;

		wchar_t *unit = ((resource_arr[i].resource==RLIMIT_CPU)?L"(seconds, ":(get_multiplier(resource_arr[i].resource)==1?L"(":L"(kB, "));
		
		sb_printf( sb_out,
				   L"%-*ls %10ls-%lc) ", 
				   w, 
				   resource_arr[i].desc, 
				   unit,
				   resource_arr[i].switch_char);
		
		if( l == RLIM_INFINITY )
			sb_append( sb_out, L"unlimited\n" );
		else
			sb_printf( sb_out, L"%d\n", l );
	}
}

static const wchar_t *get_desc( int what )
{
	int i;
	
	for( i=0; resource_arr[i].desc; i++ )
	{
		if( resource_arr[i].resource == what )
		{
			return resource_arr[i].desc;
		}
	}
	return L"Not a resource";
}


static int set( int resource, int hard, int soft, rlim_t value )
{
	struct rlimit ls;
	getrlimit( resource, &ls );
	if( value != RLIM_INFINITY )
		value *= get_multiplier( resource );
	
	if( hard )
	{
		ls.rlim_max = value;
	}
	
	if( soft )
	{
		ls.rlim_cur = value;
		
		/*
		  Do not attempt to set the soft limit higher than the hard limit
		*/
		if( ( value == RLIM_INFINITY && ls.rlim_max != RLIM_INFINITY ) || 
			( value != RLIM_INFINITY && ls.rlim_max != RLIM_INFINITY && value > ls.rlim_max))
		{
			ls.rlim_cur = ls.rlim_max;
		}		
	}
	
	if( setrlimit( resource, &ls ) )
	{
		if( errno == EPERM )
			sb_printf( sb_err, L"ulimit: Permission denied when changing resource of type '%ls'\n", get_desc( resource ) );
		else
			builtin_wperror( L"ulimit" );
		return 1;
	}	
	return 0;	
}

static int set_all( int hard, int soft, rlim_t value )
{
	int i;
	int res=0;
	
	for( i=0; resource_arr[i].desc; i++ )
	{
		if( set( resource_arr[i].resource, hard, soft, value ) )
			res = 1;
	}
	return res;
}

int builtin_ulimit( wchar_t ** argv )
{
	int hard=0;
	int soft=0;
	
	int what = RLIMIT_FSIZE;
	int report_all = 0;

	int argc = builtin_count_args( argv );
	
	woptind=0;
	
	while( 1 )
	{
		const static struct woption
			long_options[] =
			{
				{
					L"all", no_argument, 0, 'a'
				}
				,
				{
					L"hard", no_argument, 0, 'H'
				}
				,
				{
					L"soft", no_argument, 0, 'S'
				}
				,
				{
					L"core-size", no_argument, 0, 'c'
				}
				,
				{
					L"data-size", no_argument, 0, 'd'
				}
				,
				{
					L"file-size", no_argument, 0, 'f'
				}
				,
				{
					L"lock-size", no_argument, 0, 'l'
				}
				,
				{
					L"resident-set-size", no_argument, 0, 'm'
				}
				,
				{
					L"file-descriptor-count", no_argument, 0, 'n'
				}
				,
				{
					L"pipe-size", no_argument, 0, 'p'
				}
				,
				{
					L"cpu-time", no_argument, 0, 't'
				}
				,
				{
					L"process-count", no_argument, 0, 'u'
				}
				,
				{
					L"virtual-memory-size", no_argument, 0, 'v'
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
								L"aHScdflmnptuv", 
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
				
			case L'a':
				report_all=1;
				break;

			case L'H':
				hard=1;
				break;

			case L'S':
				soft=1;				
				break;

			case L'c':
				what=RLIMIT_CORE;
				break;
				
			case L'd':
				what=RLIMIT_DATA;
				break;
				
			case L'f':
				what=RLIMIT_FSIZE;
				break;
				
			case L'l':
				what=RLIMIT_MEMLOCK;
				break;
				
			case L'm':
				what=RLIMIT_RSS;
				break;
				
			case L'n':
				what=RLIMIT_NOFILE;
				break;
				
			case L's':
				what=RLIMIT_STACK;
				break;
				
			case L't':
				what=RLIMIT_CPU;
				break;
				
			case L'u':
				what=RLIMIT_NPROC;
				break;
				
#if HAVE_RLIMIT_AS				
			case L'v':
				what=RLIMIT_AS;
				break;
#endif
				
			case L'?':
				builtin_print_help( argv[0], sb_err );				
				return 1;	
		}
	}		

	switch( argc - woptind )
	{
		case 0:
			/*
			  Show current limit value
			*/
			if( report_all )
			{
				print_all( hard );
			}
			else
			{
				print( what, hard );
			}
			
			break;
			
		case 1:
		{
			/*
			  Change current limit value
			*/
			rlim_t new_limit;
			wchar_t *end;
			
			/*
			  Set both hard and soft limits if nothing else was specified
			*/
			if( !(hard+soft) )
			{
				hard=soft=1;
			}
			

			if( wcscasecmp( argv[woptind], L"unlimited" )==0)
			{
				new_limit = RLIM_INFINITY;
			}
			else if( wcscasecmp( argv[woptind], L"hard" )==0)
			{
				new_limit = get( what, 1 );
			}
			else if( wcscasecmp( argv[woptind], L"soft" )==0)
			{
				new_limit = get( what, soft );
			}
			else
			{
				errno=0;				
				new_limit = wcstol( argv[woptind], &end, 10 );
				if( errno || *end )
				{
					sb_printf( sb_err, 
							   L"%ls: Invalid limit '%ls'\n", 
							   argv[0], 
							   argv[woptind] );
					builtin_print_help( argv[0], sb_err );
					return 1;
				}
			}
			
			if( report_all )
			{
				return set_all( hard, soft, new_limit );
			}
			else 
			{
				return set( what, hard, soft, new_limit );
			}
			
			
			break;
		}
		
		default:
			sb_append2( sb_err,
						argv[0],
						L": Too many arguments\n",
						(void *)0 );
			builtin_print_help( argv[0], sb_err );
			return 1;
			
			break;
	}
	return 0;	
}
