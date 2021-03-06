/** \file util.h
	Generic utilities library.
*/

#ifndef FISH_UTIL_H
#define FISH_UTIL_H

#include <wchar.h>
#include <stdarg.h>

/**
   Data structure for an automatically resizing dynamically allocated queue,
*/
typedef struct dyn_queue
{
	/** Start of the array */
	void **start;
	/** End of the array*/
	void **stop;
	/** Where to insert elements */
	void **put_pos;
	/** Where to remove elements */
	void **get_pos;
}
dyn_queue_t;

/**
   Internal struct used by hash_table_t.
*/
typedef struct
{
	/** Hash key*/
	const void *key;
	/** Value */
	const void *data;
}
hash_struct_t;

/**
   Data structure for the hash table implementaion. A hash table allows for
   retrieval and removal of any element in O(1), so long as a proper
   hash function is supplied.

   The hash table is implemented using a single hash function and
   element storage directly in the array. When a collision occurs, the
   hashtable iterates until a zero element is found. When the table is
   75% full, it will automatically reallocate itself. This
   reallocation takes O(n) time. The table is guaranteed to never be
   more than 75% full or less than 30% full (Unless the table is
   nearly empty). Its size is always a Mersenne number.

*/

typedef struct hash_table
{
	/** The array containing the data */
	hash_struct_t *arr;
	/** Number of elements */
	int count;
	/** Length of array */
	int size;
	/** Hash function */
	int (*hash_func)( const void *key );
	/** Comparison function */
	int (*compare_func)( const void *key1, const void *key2 );
}
hash_table_t;

/**
   Data structure for an automatically resizing dynamically allocated
   priority queue. A priority queue allows quick retrieval of the
   smallest element of a set (This implementation uses O(log n) time).
   This implementation uses a heap for storing the queue.
*/
typedef struct priority_queue
{
	/** Array contining the data */
	void **arr;
	/** Number of elements*/
	int count;
	/** Length of array */
	int size;
	/** Comparison function */
	int (*compare)(void *e1, void *e2);
}
priority_queue_t;

/**
   Array list struct.
   A dynamically growing list that supports stack operations.
*/
typedef struct array_list
{
	/** Array containing the data */
	const void **arr;
	/** Position to append elements at*/
	int pos;
	/** Length of array */
	int size;
}
array_list_t;

/**
   Linked list node.
*/
typedef struct _ll_node
{
	/** Next node */
	struct _ll_node *next, /** Previous node */ *prev;
	/** Node data */
	void *data;
}
ll_node_t;

/**
   Buffer for concatenating arbitrary data.
*/
typedef struct buffer
{
	char *buff; /**<data buffer*/
	size_t length; /**< Size of buffer */
	size_t used; /**< Size of data in buffer */
}
buffer_t;


/**
   String buffer struct.  An autoallocating buffer used for
   concatenating strings. This is really just a buffer_t.
*/
typedef buffer_t string_buffer_t;

	

/**
   Returns the larger of two ints
*/
int maxi( int a, int b );
/**
   Returns the smaller of two ints
 */
int mini( int a, int b );
/**
   Returns the larger of two floats
*/
float maxf( float a, float b );
/**
   Returns the smaller of two floats
*/
float minf( float a, float b );

/*
  All the datastuctures below autoresize. The queue, stack and
  priority queue are all impemented using an array and are guaranteed
  to never be less than 50% full. 
*/

/** 
	Initialize the queue. A queue is a FIFO buffer, i.e. the first
    element to be inserted into the buffer is the first element to be
    returned. 
*/
void q_init( dyn_queue_t *q );
/** Destroy the queue */
void q_destroy( dyn_queue_t *q );
/** Insert element into queue */
int q_put( dyn_queue_t *q, void *e );
/** Remove and return next element from queue */
void *q_get( dyn_queue_t *q);
/** Return next element from queue without removing it */
void *q_peek( dyn_queue_t *q);
/** Returns 1 if the queue is empty, 0 otherwise */
int q_empty( dyn_queue_t *q );

/**
   Initialize a hash table. The hash function must never return the value 0.
*/
void hash_init( hash_table_t *h,
				int (*hash_func)(const void *key),
				int (*compare_func)(const void *key1, const void *key2) );

/**
   Initialize a hash table. The hash function must never return the value 0.
*/
void hash_init2( hash_table_t *h,
				int (*hash_func)(const void *key),
				 int (*compare_func)(const void *key1, const void *key2),
				 size_t capacity);

/**
   Destroy the hash table and free associated memory.
*/
void hash_destroy( hash_table_t *h );
/**
   Set the key/value pair for the hashtable. 
*/
int hash_put( hash_table_t *h, 
			  const void *key,
			  const void *data );
/**
   Returns the data with the associated key, or 0 if no such key is in the hashtable
*/
const void *hash_get( hash_table_t *h,
					  const void *key );
/**
   Returns the hash tables version of the specified key
*/
const void *hash_get_key( hash_table_t *h, 
						  const void *key );

/**
   Returns the number of key/data pairs in the table.
*/
int hash_get_count( hash_table_t *h);
/**
   Remove the specified key from the hash table

   \param h The hashtable
   \param key The key
   \param old_key If not 0, a pointer to the old key will be stored at the specified address
   \param old_data If not 0, a pointer to the data will be stored at the specified address
*/
void hash_remove( hash_table_t *h, 
				  const void *key, 
				  const void **old_key,
				  const void **old_data );
/**
   Checks whether the specified key is in the hash table
*/
int hash_contains( hash_table_t *h, 
				   const void *key );

/**
   Appends all keys in the table to the specified list
*/
void hash_get_keys( hash_table_t *h,
					array_list_t *arr );
/**
   Appends all data elements in the table to the specified list
*/
void hash_get_data( hash_table_t *h,
					array_list_t *arr );
/** Call the function func for each key/data pair in the table*/
void hash_foreach( hash_table_t *h, 
				   void (*func)( const void *, const void * ) );
/** Same as hash_foreach, but the function func takes an additional
 * argument, which is provided by the caller in the variable aux */
void hash_foreach2( hash_table_t *h, void (*func)( const void *, 
												 const void *, 
												 void *), 
					void *aux );

/**
   Hash function suitable for character strings. 
*/
int hash_str_func( const void *data );
/**
   Hash comparison function suitable for character strings
*/
int hash_str_cmp( const void *a, const void *b );

/**
   Hash function suitable for wide character strings. 
*/
int hash_wcs_func( const void *data );

/**
   Hash comparison function suitable for wide character strings
*/
int hash_wcs_cmp( const void *a, const void *b );

/** Initialize the priority queue

 \param q the queue to initialize
 \param compare a comparison function that can compare two entries in the queue
*/
void pq_init( priority_queue_t *q,
			  int (*compare)(void *e1, void *e2) );
/**
   Add element to the queue

 \param q the queue
 \param e the new element
 
*/
int pq_put( priority_queue_t *q,
			 void *e );
/**
  Removes and returns the last entry in the priority queue
*/
void *pq_get( priority_queue_t *q );
/**
  Returns the last entry in the priority queue witout removing it.
*/
void *pq_peek( priority_queue_t *q );

/** 
	Returns 1 if the priority queue is empty, 0 otherwise.
*/
int pq_empty( priority_queue_t *q );

/**
   Returns the number of elements in the priority queue.
*/
int pq_get_count( priority_queue_t *q );

/** 
	Destroy the priority queue and free memory used by it.
*/
void pq_destroy(  priority_queue_t *q );

/**
   Allocate heap memory for creating a new list and initialize it
*/
array_list_t *al_new();

/** Initialize the list. */
void al_init( array_list_t *l );
/** Destroy the list and free memory used by it.*/
void al_destroy( array_list_t *l );
/**
   Append element to list

   \param l The list
   \param o The element
   \return
   \return 1 if succesfull, 0 otherwise
*/
int al_push( array_list_t *l, const void *o );
/**
   Append all elements of a list to another

   \param a The destination list
   \param b The source list
   \return 1 if succesfull, 0 otherwise
*/
int al_push_all( array_list_t *a, array_list_t *b );
/**
   Sets the element at the specified index

   \param l The array_list_t
   \param pos The index 
   \param o The element 
*/
int al_set( array_list_t *l, int pos, const void *o );
/**
   Returns the element at the specified index

   \param l The array_list_t
   \param pos The index 
   \return The element 
*/
const void *al_get( array_list_t *l, int pos );
/**
  Truncates the list to new_sz items.
*/
void al_truncate( array_list_t *l, int new_sz );
/**
  Removes and returns the last entry in the list
*/
const void *al_pop( array_list_t *l );
/**
   Returns the number of elements in the list
*/
int al_get_count( array_list_t *l );
/**
  Returns the last entry in the list witout removing it.
*/
const void *al_peek( array_list_t *l );
/** Returns 1 if the list is empty, 0 otherwise*/
int al_empty( array_list_t *l);
/** Call the function func for each entry in the list*/
void al_foreach( array_list_t *l, void (*func)(const void * ));
/** 
	Same as al_foreach, but the function func takes an additional
	argument, which is provided by the caller in the variable aux
*/
void al_foreach2( array_list_t *l, void (*func)(const void *, void *), void *aux);

/**
   Compares two wide character strings without case but with  
   a logical ordering for numbers.

   This function tries to order strings in a way which is intuitive to
   humans with regards to sorting strings containing numbers.

   Most sorting functions would sort the strings 'file1.txt'
   'file5.txt' and 'file12.txt' as:

   file1.txt
   file12.txt
   file5.txt

   This function regards any sequence of digits as a single entity
   when performing comparisons, so the output is instead:

   file1.txt
   file5.txt
   file12.txt

   Which most people would find more intuitive.

   The system breaks down if the user is using numbers of a base larger than 10.
*/
int wcsfilecmp( const wchar_t *a, const wchar_t *b );


/**
   Initialize the specified string_buffer
*/
void sb_init( string_buffer_t * );

string_buffer_t *sb_new();

/**
   Append a string to the buffer
*/
void sb_append( string_buffer_t *, const wchar_t * );

/**
   Append a part of a string to the buffer
*/
void sb_append_substring( string_buffer_t *, const wchar_t *, size_t );

/**
   Append a character to the buffer
*/
void sb_append_char( string_buffer_t *, wchar_t );


/**
   Append a null terminated list of strings to the buffer.
   Example:

   sb_append2( my_buff, L"foo", L"bar", 0 );
*/
void sb_append2( string_buffer_t *, ... );

/**
   Append formated string data to the buffer. This function internally
   relies on \c vswprintf, so any filter options supported by that
   function is also supported by this function
*/
int sb_printf( string_buffer_t *buffer, const wchar_t *format, ... );

/**
   Vararg version of sb_printf
*/
int sb_vprintf( string_buffer_t *buffer, const wchar_t *format, va_list va_orig );

/**
  Destroy the buffer and free it's memory
*/
void sb_destroy( string_buffer_t * );

/**
   Truncate the buffer.
*/
void sb_clear( string_buffer_t * );


/**
   Initialize the specified buffer_t
 */
void b_init( buffer_t *b);
/**
   Destroy the specified buffer_t
*/

void b_destroy( buffer_t *b );

/**
   Add data of the specified length to the specified buffer_t
*/
void b_append( buffer_t *b, const void *d, ssize_t len );

/**
   Get the current time in microseconds since Jan 1, 1970
*/
long long get_time();

#endif
