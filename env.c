/** \file env.c
	Functions for setting and getting environment variables.
*/

#include "config.h"

#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#if HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#if HAVE_TERMIO_H
#include <termio.h>
#endif

#include <term.h>
#include <errno.h>

#include "util.h"
#include "wutil.h"
#include "proc.h"
#include "common.h"
#include "env.h"
#include "sanity.h"
#include "expand.h"
#include "history.h"
#include "reader.h"
#include "parser.h"
#include "env_universal.h"
#include "input_common.h"
#include "event.h"

/**
   Command used to start fishd
*/
#define FISHD_CMD L"if which fishd >/dev/null; fishd ^/tmp/fish.%s.log; end"

#define ENV_NULL L"\x1d"

/**
   At init, we read all the environment variables from this array 
*/
extern char **environ;


/**
   Struct representing one level in the function variable stack
*/
typedef struct env_node
{
	/** 
		Variable table 
	*/
	hash_table_t env;
	/** 
		Does this node imply a new variable scope? If yes, all
		non-global variables below this one in the stack are
		invisible. If new_scope is set for the global variable node,
		the universe will explode.
	*/
	int new_scope;
	/**
	   Does this node contain any variables which are exported to subshells
	*/
	int export;
	
	/** 
		Pointer to next level
	*/
	struct env_node *next;
}
	env_node_t;

/**
   A variable entry. Stores the value of a variable and whether it
   should be exported. Obviously, it needs to be allocated large
   enough to fit the value string.
*/
typedef struct var_entry
{
	int export; /**< Whether the variable should be exported */
	wchar_t val[0]; /**< The value of the variable */
}
	var_entry_t;

/**
   Top node on the function stack
*/
static env_node_t *top=0;

/**
   Bottom node on the function stack
*/
static env_node_t *global_env = 0;


/**
   Table for global variables
*/
static hash_table_t *global;

/**
   Table of variables that may not be set using the set command.
*/
static hash_table_t env_read_only;

/**
   Exported variable array used by execv
*/
static char **export_arr=0;

/**
   Buffer used for storing string contents for export_arr
*/
static buffer_t export_buffer;


/**
   Flag for checking if we need to regenerate the exported variable
   array
*/
static int has_changed = 1;

/**
   Free hash key and hash value
*/
static void clear_hash_entry( const void *key, const void *data )
{
	var_entry_t *entry = (var_entry_t *)data;	
	if( entry->export )
		has_changed = 1;
	
	free( (void *)key );
	free( (void *)data );
}

/**
   This stringbuffer is used to store the value of dynamically
   generated variables, such as history.
*/
static string_buffer_t dyn_var;

/**
   Variable used by env_get_names to communicate auxiliary information
   to add_key_to_hash
*/
static int get_names_show_exported;
/**
   Variable used by env_get_names to communicate auxiliary information
   to add_key_to_hash
*/
static int get_names_show_unexported;

/**
   When fishd isn't started, this function is provided to
   env_universal as a callback, it tries to start up fishd. It's
   implementation is a bit of a hack, since it evaluates a bit of
   shellscript, and it might be used at times when that might not be
   the best idea.
*/
static void start_fishd()
{
	string_buffer_t cmd;
	struct passwd *pw;
		
	sb_init( &cmd );
	pw = getpwuid(getuid());
	
	debug( 3, L"Spawning new copy of fishd" );
	
	if( !pw )
	{
		debug( 0, L"Could not get user information" );
		return;
	}
	
	sb_printf( &cmd, FISHD_CMD, pw->pw_name );
	
	eval( (wchar_t *)cmd.buff,
		  0,
		  TOP );
	sb_destroy( &cmd );
}

static void universal_callback( int type,
								const wchar_t *name, 
								const wchar_t *val )
{
	wchar_t *str=0;
	
	switch( type )
	{
		case SET:
		case SET_EXPORT:
			str=L"SET";
			break;
		case ERASE:
			str=L"ERASE";
			break;
	}
	
	if( str )
	{
		array_list_t arg;
		event_t ev;
		
		has_changed=1;
		
		ev.type=EVENT_VARIABLE;
		ev.param1.variable=name;
		ev.function_name=0;
		
		al_init( &arg );
		al_push( &arg, L"VARIABLE" );
		al_push( &arg, str );
		al_push( &arg, name );
		event_fire( &ev, &arg );		
		al_destroy( &arg );
	}
}

void env_init()
{
	char **p;

	sb_init( &dyn_var );

	b_init( &export_buffer );
	
	
	/*
	  These variables can not be altered directly by the user
	*/
	hash_init( &env_read_only, &hash_wcs_func, &hash_wcs_cmp );

	hash_put( &env_read_only, L"status", L"" );
	hash_put( &env_read_only, L"history", L"" );
	hash_put( &env_read_only, L"_", L"" );
	hash_put( &env_read_only, L"LINES", L"" );
	hash_put( &env_read_only, L"COLUMNS", L"" );
	hash_put( &env_read_only, L"PWD", L"" );
	
	/*
	  HOME should be writeable by root, since this is often a
	  convenient way to install software.
	*/
	if( getuid() != 0 )
		hash_put( &env_read_only, L"HOME", L"" );
	
	top = malloc( sizeof(env_node_t) );
	top->next = 0;
	top->new_scope = 0;
	top->export=0;
	hash_init( &top->env, &hash_wcs_func, &hash_wcs_cmp );
	global_env = top;
	global = &top->env;	

	/*
	  Import environment variables
	*/
	for( p=environ; *p; p++ )
	{
		wchar_t *key, *val;
		wchar_t *pos;
		
		key = str2wcs(*p);
		
		if( !key )
			continue;
		
		val = wcschr( key, L'=' );
		
		if( val == 0 )
			env_set( key, L"", ENV_EXPORT );
		else
		{ 
			*val = L'\0';
			val++;
			pos=val;
			//fwprintf( stderr, L"Set $%ls to %ls\n", key, val );
			while( *pos )
			{
				if( *pos == L':' )
					*pos = ARRAY_SEP;
				pos++;
			}
			
			env_set( key, val, ENV_EXPORT | ENV_GLOBAL );
		}		
		free(key);
	}		

	env_universal_init( env_get( L"FISHD_SOKET_DIR"), 
						env_get( L"USER" ),
						&start_fishd,
						&universal_callback );
	
}

void env_destroy()
{
	env_universal_destroy();
	
	sb_destroy( &dyn_var );

	b_destroy( &export_buffer );
	
	while( &top->env != global )
		env_pop();

	hash_destroy( &env_read_only );
	
	hash_foreach( global, &clear_hash_entry );
	hash_destroy( global );
	free( top );
	
	free( export_arr );
	
}

/**
   Find the scope hashtable containing the variable with the specified
   key
*/
static env_node_t *env_get_node( const wchar_t *key )
{
	var_entry_t* res;
	env_node_t *env = top;


	while( env != 0 )
	{
		res = (var_entry_t *) hash_get( &env->env, 
										key );
		if( res != 0 )
		{
			return env;
		}

		if( env->new_scope )
			env = global_env;
		else		
			env = env->next;
	}
	
	return 0;
}
	
void env_set( const wchar_t *key, 
			  const wchar_t *val, 
			  int var_mode )
{
	int free_val = 0;
	var_entry_t *entry;
	env_node_t *node;
	int has_changed_old = has_changed;
	int has_changed_new = 0;
	var_entry_t *e=0;	
	int done=0;

	event_t ev;
	array_list_t ev_list;
	int is_universal = 0;	
	
	if( (var_mode & ENV_USER ) && 
		hash_get( &env_read_only, key ) )
	{
		return;
	}
	
	if( wcscmp(key, L"LANG" )==0 )
	{
		fish_setlocale(LC_ALL,val);
	}

	if( wcscmp( key, L"umask" ) == 0)
	{
		wchar_t *end;
		int mask;
		
		if( val && wcslen(val) )
		{				
			errno=0;
			mask = wcstol( val, &end, 8 );
	
			if( !errno && !*end )
			{
				umask( mask );
			}
		}
	}
	

	/*
	  Zero element arrays are internaly not coded as null but as this placeholder string
	*/
	if( !val && ( var_mode & ENV_USER ) )
	{
		val = ENV_NULL;
	}
	
	if( var_mode & ENV_UNIVERSAL )
	{
		int export = 0;

		if( !(var_mode & ENV_EXPORT ) &&
			!(var_mode & ENV_UNEXPORT ) )
		{
			env_universal_get_export( key );
		}
		else 
			export = (var_mode & ENV_EXPORT );
		
		env_universal_set( key, val, export );
		is_universal = 1;
		
	}
	else
	{
		
		if( val == 0 )
		{
			wchar_t *prev_val;
			free_val = 1;
			prev_val = env_get( key );
			val = wcsdup( prev_val?prev_val:L"" );
		}

		node = env_get_node( key );
		if( node && &node->env != 0 )
		{
			e = (var_entry_t *) hash_get( &node->env, 
										  key );
		
			if( e->export )
				has_changed_new = 1;
		
		}

		if( (var_mode & ENV_LOCAL) || 
			(var_mode & ENV_GLOBAL) )
		{
			node = ( var_mode & ENV_GLOBAL )?global_env:top;
		}
		else
		{
			if( node )
			{
				if( !(var_mode & ENV_EXPORT ) &&
					!(var_mode & ENV_UNEXPORT ) )
				{				
					var_mode = e->export?ENV_EXPORT:0;
				}
			}
			else
			{
				if( !proc_had_barrier)
					env_universal_barrier();

				if( env_universal_get( key ) )
				{
					int export = 0;
				
					if( !(var_mode & ENV_EXPORT ) &&
						!(var_mode & ENV_UNEXPORT ) )
					{
						env_universal_get_export( key );
					}
					else 
						export = (var_mode & ENV_EXPORT );
				
					env_universal_set( key, val, export );
					is_universal = 1;
					
					done = 1;
				
				}
				else
				{
					/*
					  New variable with unspecified scope. The default scope is the innermost scope that is shadowing
					*/
					node = top;
					while( node->next && !node->new_scope )
						node = node->next;
				
				}
			}
		}
	
		if( !done )
		{
			void *k, *v;
			hash_remove( &node->env, key, (const void **)&k, (const void **)&v );
			free( k );
			free( v );

			entry = malloc( sizeof( var_entry_t ) + 
							sizeof(wchar_t )*(wcslen(val)+1));
	
			if( var_mode & ENV_EXPORT)
			{
				entry->export = 1;
				has_changed_new = 1;		
			}
			else
				entry->export = 0;

			wcscpy( entry->val, val );

			hash_put( &node->env, wcsdup(key), entry );

			if( entry->export )
			{
				node->export=1;
			}

			if( free_val )
				free((void *)val);
		
			has_changed = has_changed_old || has_changed_new;
		}
	
	}

	if( !is_universal )
	{
		ev.type=EVENT_VARIABLE;
		ev.param1.variable = key;
		ev.function_name = 0;
		
		al_init( &ev_list );
		al_push( &ev_list, L"VARIABLE" );
		al_push( &ev_list, key );
		
//	debug( 1, L"env_set: fire events on variable %ls", key );	
		event_fire( &ev, &ev_list );
//	debug( 1, L"env_set: return from event firing" );	
		al_destroy( &ev_list );	
	}
	
}

/**
   Attempt to remove/free the specified key/value pair from the
   specified hash table.
*/
static int try_remove( env_node_t *n,
					   const wchar_t *key )
{
	wchar_t *old_key, *old_val;
	if( n == 0 )
		return 0;

	hash_remove( &n->env, 
				 key,
				 (const void **)&old_key, 
				 (const void **)&old_val );
	if( old_key != 0 )
	{
		var_entry_t * v = (var_entry_t *)old_val;
		if( v->export )
		{
			has_changed = 1;
		}
		
		free(old_key);
		free(old_val);
		return 1;
	}

	if( n->new_scope )
		return try_remove( global_env, key );
	else
		return try_remove( n->next, key );
}


void env_remove( const wchar_t *key, int var_mode )
{
	if( (var_mode & ENV_USER ) && 
		hash_get( &env_read_only, key ) )
	{
		return;
	}
	
	if( !try_remove( top, key ) )
	{
		env_universal_remove( key );
	}
}


wchar_t *env_get( const wchar_t *key )
{
	var_entry_t *res;
	env_node_t *env = top;
	wchar_t *item;
	
	if( wcscmp( key, L"history" ) == 0 )
	{
		wchar_t *current;
		int i;		
		int add_current=0;
		sb_clear( &dyn_var );						
		
		current = reader_get_buffer();
		if( current && wcslen( current ) )
		{
			add_current=1;
			sb_append( &dyn_var, current );
		}
		
		for( i=add_current; i<8; i++ )
		{
			wchar_t *next = history_get( i-add_current );
			if( !next )
			{
				debug( 1, L"No history at idx %d\n", i );
				break;
			}
			
			if( i!=0)
				sb_append( &dyn_var, ARRAY_SEP_STR );
			sb_append( &dyn_var, next );
		}
		return (wchar_t *)dyn_var.buff;
	}
	else if( wcscmp( key, L"COLUMNS" )==0 )
	{
		sb_clear( &dyn_var );						
		sb_printf( &dyn_var, L"%d", common_get_width() );		
		return (wchar_t *)dyn_var.buff;		
	}	
	else if( wcscmp( key, L"LINES" )==0 )
	{
		sb_clear( &dyn_var );						
		sb_printf( &dyn_var, L"%d", common_get_height() );		
		return (wchar_t *)dyn_var.buff;
	}
	else if( wcscmp( key, L"status" )==0 )
	{
		sb_clear( &dyn_var );			
		sb_printf( &dyn_var, L"%d", proc_get_last_status() );		
		return (wchar_t *)dyn_var.buff;		
	}
	
	while( env != 0 )
	{
		res = (var_entry_t *) hash_get( &env->env, 
										key );
		if( res != 0 )
		{
			if( wcscmp( res->val, ENV_NULL )==0) 
			{
				return 0;
			}
			else
				return res->val;			
		}
		
		if( env->new_scope )
			env = global_env;
		else
			env = env->next;
	}	
	if( !proc_had_barrier)
		env_universal_barrier();
	item = env_universal_get( key );
	
	if( !item || (wcscmp( item, ENV_NULL )==0))
	{
		return 0;
	}
	else
		return item;
}

int env_exist( const wchar_t *key )
{
	var_entry_t *res;
	env_node_t *env = top;
	wchar_t *item;
	
    if( hash_get( &env_read_only, key ) )
    {
        return 1;
    }
	
	while( env != 0 )
	{
		res = (var_entry_t *) hash_get( &env->env, 
										key );
		if( res != 0 )
		{
			return 1;
		}
		
		if( env->new_scope )
			env = global_env;
		else
			env = env->next;
	}	
	if( !proc_had_barrier)
		env_universal_barrier();
	item = env_universal_get( key );
	
	return item != 0;
}

static int local_scope_exports( env_node_t *n )
{
	
	if( n==global_env )
		return 0;
	
	if( n->export )
		return 1;
	
	if( n->new_scope )
		return 0;
	
	return local_scope_exports( n->next );
}

void env_push( int new_scope )
{
	env_node_t *node = malloc( sizeof(env_node_t) );
	node->next = top;
	node->export=0;
	hash_init( &node->env, &hash_wcs_func, &hash_wcs_cmp );
	node->new_scope=new_scope;
	if( new_scope )
	{
		has_changed |= local_scope_exports(top);
	}
	top = node;	

}


void env_pop()
{
	if( &top->env != global )
	{
		env_node_t *killme = top;

		if( killme->new_scope )
		{
			has_changed |= killme->export || local_scope_exports( killme->next );
		}
		
		top = top->next;
		hash_foreach( &killme->env, &clear_hash_entry );
		hash_destroy( &killme->env );
		free( killme );
		
	}
	else
	{
		debug( 0,
			   L"Tried to pop empty environment stack." );
		sanity_lose();
	}	
}


/**
   Function used with hash_foreach to insert keys of one table into
   another
*/
static void add_key_to_hash( const void *key, 
							 const void *data,
							 void *aux )
{
	var_entry_t *e = (var_entry_t *)data;
	if( ( e->export && get_names_show_exported) || 
		( !e->export && get_names_show_unexported) )
		hash_put( (hash_table_t *)aux, key, 0 );
}

static void add_to_hash( const void *k, void *aux )
{
	hash_put( (hash_table_t *)aux,
			  k,
			  0 );
}


void env_get_names( array_list_t *l, int flags )
{
	int show_local = flags & ENV_LOCAL;
	int show_global = flags & ENV_GLOBAL;
	int show_universal = flags & ENV_UNIVERSAL;

	hash_table_t names;
	env_node_t *n=top;

	get_names_show_exported = 
		flags & ENV_EXPORT|| (!(flags & ENV_UNEXPORT));
	get_names_show_unexported = 
		flags & ENV_UNEXPORT|| (!(flags & ENV_EXPORT));

	if( !show_local && !show_global && !show_universal )
	{
		show_local =show_universal = show_global=1;
	}

	hash_init( &names, &hash_wcs_func, &hash_wcs_cmp );
	
	if( show_local )
	{
		while( n )
		{
			if( n == global_env )
				break;
			
			hash_foreach2( &n->env, 
						   add_key_to_hash,
						   &names );

			if( n->new_scope )
				break;		
			else
				n = n->next;

		}
	}
	
	if( show_global )
	{
		hash_foreach2( &global_env->env, 
					   add_key_to_hash,
					   &names );
		if( get_names_show_unexported )
		{
			al_push( l, L"history" );
			al_push( l, L"status" );
		}
		
		if( get_names_show_exported )
		{
			al_push( l, L"COLUMNS" );
			al_push( l, L"LINES" );
		}
		
	}
	
	if( show_universal )
	{
		array_list_t uni_list;
		al_init( &uni_list );
		
		env_universal_get_names( &uni_list, 
								 get_names_show_exported,
								 get_names_show_unexported );

		al_foreach2( &uni_list, &add_to_hash, &names );
		al_destroy( &uni_list );
	}
	
	hash_get_keys( &names, l );
	hash_destroy( &names );	
}

/**
   Function used by env_export_arr to iterate over hashtable of variables
*/
static void export_func1( const void *k, const void *v, void *aux )
{
	var_entry_t *val_entry = (var_entry_t *)v;
	if( val_entry->export )
	{
		hash_table_t *h = (hash_table_t *)aux;
		
		if( !hash_get( h, k ) )
			hash_put( h, k, val_entry->val );
	}
	
}

/**
   Function used by env_export_arr to iterate over hashtable of variables
*/
static void export_func2( const void *k, const void *v, void *aux )
{
	wchar_t *key = (wchar_t *)k;
	wchar_t *val = (wchar_t *)v;
	
	char *ks = wcs2str( key );
	char *vs = wcs2str( val );
	
	char *pos = vs;

	buffer_t *out = (buffer_t *)aux;

	if( !ks || !vs )
	{
		die_mem();
	}
	
	/*
	  Make arrays into colon-separated lists
	*/
	while( *pos )
	{
		if( *pos == ARRAY_SEP )
			*pos = ':';			
		pos++;
	}
	int nil = 0;
	
	b_append( out, ks, strlen(ks) );
	b_append( out, "=", 1 );
	b_append( out, vs, strlen(vs) );
	b_append( out, &nil, 1 );

	free( ks );
	free( vs );
}

char **env_export_arr( int recalc)
{
	if( recalc && !proc_had_barrier)
		env_universal_barrier();

	if( has_changed )
	{
		array_list_t uni;
		hash_table_t vals;
		env_node_t *n=top;
		int prev_was_null=1;
		int pos=0;		
		int i;

		debug( 4, L"env_export_arr() recalc" );
				
		hash_init( &vals, &hash_wcs_func, &hash_wcs_cmp );
		
		while( n )
		{
			hash_foreach2( &n->env, &export_func1, &vals );
			
			if( n->new_scope )
				n = global_env;
			else
				n = n->next;			
		}		
		
		al_init( &uni );
		env_universal_get_names( &uni, 1, 0 );
		for( i=0; i<al_get_count( &uni ); i++ )
		{
			wchar_t *key = (wchar_t *)al_get( &uni, i );
			wchar_t *val = env_universal_get( key );
			if( !hash_get( &vals, key ) )
				hash_put( &vals, key, val );
		}
		al_destroy( &uni );

		export_buffer.used=0;
		
		hash_foreach2( &vals, &export_func2, &export_buffer );
		hash_destroy( &vals );
		
		export_arr = realloc( export_arr,
							  sizeof(char *)*(hash_get_count( &vals) + 1) );
		
		for( i=0; i<export_buffer.used; i++ )
		{
			if( prev_was_null )
			{
				export_arr[pos++]= &export_buffer.buff[i];
				debug( 3, L"%s", &export_buffer.buff[i]);
			}
			prev_was_null = (export_buffer.buff[i]==0);
		}
		export_arr[pos]=0;
		has_changed=0;

	}
	return export_arr;	
}
