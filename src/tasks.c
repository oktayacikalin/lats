/*
 * Task-stuff
 * 
 * here we try to do some semi-multitasking with some
 * sort of a queue where every task can enter its
 * name in order to find out whether it's executed more
 * than one time in which case the old ones have to
 * stop so that the new one can begin
 *
 * you can find all task-types in task_get_number_of_type() down there...
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "tasks.h"
#include "support.h"

//int extern task_queue [] ; // every task enters its name here
//int extern thread_count ; // threads
task_entry task_queue [256] ; // every task enters its name here - take a look at tasks.c !
int thread_count = 0 ; // how many threads do we need to be aware of ?!
int tasklist_blocked = FALSE ; // say if it's blocked right now


void
check_tasklist_blocked ( void )
{
	while ( tasklist_blocked == TRUE )
		usleep ( 1000 ) ;
}


void
tasklist_block ( void )
{
	tasklist_blocked = TRUE ;
}


void
tasklist_unblock ( void )
{
	tasklist_blocked = FALSE ;
}


void
task_clear_queue ( void )
{
	int i ;
	
	check_tasklist_blocked () ;
	tasklist_block () ;
	
	for ( i = 1 ; i < 256 ; i++ )
	{
		sprintf ( task_queue [ i ].command, "%s", "" ) ;
		task_queue [ i ].attr = NULL ;
	}
	
	tasklist_unblock () ;
}


int
task_add_to_queue ( char *name, gpointer *attr )
{
	int i ;

	check_tasklist_blocked () ;
	tasklist_block () ;

	for ( i = 1 ; i < 256 ; i++ )
	{
		int len = strlen ( task_queue [ i ].command ) ;
		if ( len == 0 )
		{
			sprintf ( task_queue [ i ].command, "%s", name ) ;
			task_queue [ i ].attr = attr ;
			tasklist_unblock () ;
			return i ;
		}
	}
	tasklist_unblock () ;
	return -1 ;
}


int
task_already_exists_in_queue ( char *name, gpointer *attr )
{
	int i, count = 0 ;

	check_tasklist_blocked () ;
	tasklist_block () ;

	for ( i = 1 ; i < 256 ; i++ )
	{
		if ( strcmp ( task_queue [ i ].command, name ) == 0 ||
			 ( attr != NULL && task_queue [ i ].attr == attr ) )
			count = i ;
		
	}

	tasklist_unblock () ;
	return count ;
}


int
task_remove_from_queue ( int tasknum ) 
{
	sprintf ( task_queue [ tasknum ].command, "%s", "" ) ;
	task_queue [ tasknum ].attr = NULL ;
	return tasknum ;
}


int
task_remove_all_from_queue ( char *name, gpointer *attr )
{
	int i, count = 0 ;
	
	check_tasklist_blocked () ;
	tasklist_block () ;

	for ( i = 1 ; i < 256 ; i++ )
	{
		if ( strcmp ( task_queue [ i ].command, name ) == 0 && 
			( attr == NULL || task_queue [ i ].attr == attr ) )
		{
			count++ ;
			task_remove_from_queue ( i ) ;
		}
	}

	tasklist_unblock () ;
	return count ;
}


int
task_get_last_in_queue ( int tasknum )
{
	int i, num = 0 ;

	check_tasklist_blocked () ;
	tasklist_block () ;

	for ( i = 1 ; i < 256 ; i++ )
	{
		if ( strcmp ( task_queue [ i ].command, task_queue [ tasknum ].command ) == 0 ) num = i ;
	}

	tasklist_unblock () ;
	return num ;
}


int
task_how_many_in_queue ( int tasknum )
{
	int i, num = 0 ;

	check_tasklist_blocked () ;
	tasklist_block () ;

	for ( i = 1 ; i < 256 ; i++ )
	{
		if ( strcmp ( task_queue [ i ].command, task_queue [ tasknum ].command ) == 0 ) num++ ;
	}

	tasklist_unblock () ;
	return num ;
}


void
wait_for_services_to_quit ( void )
{
	int max_count ;
	int old_thread_count = 0 ;
	
	task_add_to_queue ( "stop_dir_list", NULL ) ;
	task_add_to_queue ( "quit", NULL ) ;
	
	//thread_count = thread_count + 500 ; // enable this just for testing :)
	
	max_count = thread_count ;
	
	while ( thread_count > 0 )
	{
		usleep ( 1000 ) ;
		if ( old_thread_count != thread_count && thread_count > 0 )
		{
			old_thread_count = thread_count ;
		}
		//thread_count-- ;
	}
}

