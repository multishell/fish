/** \file io.c

Utilities for io redirection.
	
*/
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#if HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#if HAVE_TERMIO_H
#include <termio.h>
#endif

#include <term.h>

#include "util.h"
#include "wutil.h"
#include "exec.h"
#include "common.h"
#include "io.h"



void io_buffer_read( io_data_t *d )
{

	exec_close(d->param1.pipe_fd[1] );
	
	if( d->io_mode == IO_BUFFER )
	{		
		if( fcntl( d->param1.pipe_fd[0], F_SETFL, 0 ) )
		{
			wperror( L"fcntl" );
			return;
		}	
		debug( 4, L"exec_read_io_buffer: blocking read on fd %d", d->param1.pipe_fd[0] );
		while(1)
		{
			char b[4096];
			int l;
			l=read_blocked( d->param1.pipe_fd[0], b, 4096 );
			if( l==0 )
			{
				break;
			}
			else if( l<0 )
			{
				/*
				  exec_read_io_buffer is only called on jobs that have
				  exited, and will therefore never block. But a broken
				  pipe seems to cause some flags to reset, causing the
				  EOF flag to not be set. Therefore, EAGAIN is ignored
				  and we exit anyway.
				*/
				if( errno != EAGAIN )
				{
					debug( 1, 
						   L"An error occured while reading output from code block on fd %d", 
						   d->param1.pipe_fd[0] );
					wperror( L"exec_read_io_buffer" );				
				}
				
				break;				
			}
			else
			{				
				b_append( d->param2.out_buffer, b, l );				
			}
		}
	}
}


io_data_t *io_buffer_create()
{
	io_data_t *buffer_redirect = malloc( sizeof( io_data_t ));
	
	buffer_redirect->io_mode=IO_BUFFER;
	buffer_redirect->next=0;
	buffer_redirect->param2.out_buffer= malloc( sizeof(buffer_t));
	b_init( buffer_redirect->param2.out_buffer );
	buffer_redirect->fd=1;


	if( exec_pipe( buffer_redirect->param1.pipe_fd ) == -1 )
	{
		debug( 1, PIPE_ERROR );
		wperror (L"pipe");
		free(  buffer_redirect->param2.out_buffer );
		free( buffer_redirect );
		return 0;
	}
	else if( fcntl( buffer_redirect->param1.pipe_fd[0],
					F_SETFL,
					O_NONBLOCK ) )
	{
		debug( 1, PIPE_ERROR );
		wperror( L"fcntl" );
		free(  buffer_redirect->param2.out_buffer );
		free( buffer_redirect );
		return 0;
	}
	return buffer_redirect;
}

void io_buffer_destroy( io_data_t *io_buffer )
{
	
	exec_close( io_buffer->param1.pipe_fd[0] );

	/*
	  Dont free fd for writing. This should already be free'd before
	  calling exec_read_io_buffer on the buffer
	*/
	
	b_destroy( io_buffer->param2.out_buffer );
	
	free( io_buffer->param2.out_buffer );
	free( io_buffer );
}



io_data_t *io_add( io_data_t *list, io_data_t *element )
{
	io_data_t *curr = list;
	if( curr == 0 )
		return element;
	while( curr->next != 0 )
		curr = curr->next;
	curr->next = element;
	return list;
}

io_data_t *io_remove( io_data_t *list, io_data_t *element )
{
	io_data_t *curr, *prev=0;
	for( curr=list; curr; curr = curr->next )
	{		
		if( element == curr )
		{
			if( prev == 0 )
			{
				io_data_t *tmp = element->next;
				element->next = 0;
				return tmp;
			}
			else
			{
				prev->next = element->next;
				element->next = 0;
				return list;
			}
		}
		prev = curr;
	}
	return list;
}

io_data_t *io_duplicate( io_data_t *l )
{
	io_data_t *res;
	
	if( l == 0 )
		return 0;

	res = malloc( sizeof( io_data_t) );
	
	if( !res )
	{
		die_mem();
		
	}
	
	memcpy( res, l, sizeof(io_data_t ));
	res->next=io_duplicate( l->next );
	return res;	
}

io_data_t *io_get( io_data_t *io, int fd )
{
	if( io == 0 )
		return 0;
	
	io_data_t *res = io_get( io->next, fd );
	if( res )
		return res;
	
	if( io->fd == fd )
		return io;

	return 0;
}

/*
static void io_print( io_data_t *io )
{
	if( !io )
	{
		fwprintf( stderr, L"\n" );
		return;
	}
	

	fwprintf( stderr, L"IO fd %d, type ",
			  io->fd );
	switch( io->io_mode )
	{
		case IO_PIPE:
			fwprintf(stderr, L"PIPE, data %d\n", io->pipe_fd[io->fd?1:0] );
			break;
		
		case IO_FD:
			fwprintf(stderr, L"FD, copy %d\n", io->old_fd );
			break;

		case IO_BUFFER:
			fwprintf( stderr, L"BUFFER\n" );
			break;
			
		default:
			fwprintf( stderr, L"OTHER\n" );
	}

	io_print( io->next );
	
}
*/
