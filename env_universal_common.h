#ifndef FISH_ENV_UNIVERSAL_COMMON_H
#define FISH_ENV_UNIVERSAL_COMMON_H

#include <wchar.h>

#include "util.h"

/**
   The set command
*/
#define SET_STR L"SET"

/**
   The set_export command
*/
#define SET_EXPORT_STR L"SET_EXPORT"

/**
   The erase command
*/
#define ERASE_STR L"ERASE"

/**
   The barrier command
*/
#define BARRIER_STR L"BARRIER"

/**
   The barrier_reply command
*/
#define BARRIER_REPLY_STR L"BARRIER_REPLY"


/**
   The filename to use for univeral variables. The username is appended
*/
#define SOCK_FILENAME "fishd.socket."

/**
   The different types of commands that can be sent between client/server
*/
enum
{
	SET,
	SET_EXPORT,
	ERASE,
	BARRIER,
	BARRIER_REPLY,
}
	;

/**
   This struct represents a connection between a universal variable server/client
*/
typedef struct connection
{
	/**
	   The file descriptor this socket lives on
	*/
	int fd;
	/**
	   Queue of onsent messages
	*/
	dyn_queue_t unsent;
	/**
	   Set to one when this connection should be killed
	*/
	int killme;
	/**
	   The state used for character conversions
	*/
	mbstate_t wstate;
	/**
	   The input string. Input from the socket goes here. When a
	   newline is encountered, the buffer is parsed and cleared.
	*/
	string_buffer_t input;
	
	/**
	   Link to the next connection
	*/
	struct connection *next;
}
	connection_t;

/**
   A struct representing a message to be sent between client and server
*/
typedef struct 
{
	int count;
	char body[0];
}
	message_t;

/**
   Read all available messages on this connection
*/
void read_message( connection_t * );

/**
   Send as many messages as possible without blocking to the connection
*/
void try_send_all( connection_t *c );

/**
   Create a messge with the specified properties
*/
message_t *create_message( int type, const wchar_t *key, const wchar_t *val );

/**
   Init the library
*/
void env_universal_common_init(void (*cb)(int type, const wchar_t *key, const wchar_t *val ) );

/**
   Destroy library data
*/
void env_universal_common_destroy();

/**
   Add all variable names to the specified list
*/
void env_universal_common_get_names( array_list_t *l,
									 int show_exported,
									 int show_unexported );

/**
   Get the value of the variable with the specified name
*/
wchar_t *env_universal_common_get( const wchar_t *name );

/**
   Get the export flag of the variable with the specified
   name. Returns 0 if the variable doesn't exist.
*/
int env_universal_common_get_export( const wchar_t *name );

void enqueue_all( connection_t *c );


#endif
