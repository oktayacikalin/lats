/*
 * Task-stuff
 * 
 * here we try to do some semi-multitasking with some
 * sort of a queue where every task can enter its
 * name in order to find out whether it's executed more
 * than one time in which case the old ones have to
 * stop so that the new one can begin
 *
 * we need this to update the image-view correctly
 *
 * you can find all task-types in task_get_number_of_type() down there...
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <pthread.h>

#include "callbacks.h"
#include "importedfuncs.h"
#include "tasks.h"
#include "imageview.h"
#include "support.h"

GtkWidget extern *MainWindow ;
GtkWidget extern *PrefsWindow ;
GtkWidget extern *BgColorSelectionDialog ;
GtkWidget extern *FullscreenWindow ;
GtkWidget extern *ShutdownWindow ;

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
	
	printd ( "ta: clearing task-queue...\n" ) ;
	
	for ( i = 1 ; i < 256 ; i++ )
	{
		//printd ( "ta: Setting queue number " ) ; printd ( text_from_var ( i ) ) ;
		//printd ( " from '" ) ; printd ( text_from_var ( task_queue [ i ] ) ) ; printd ( "' to '" ) ;
		sprintf ( task_queue [ i ].command, "%s", "" ) ;
		sprintf ( task_queue [ i ].attr, "%s", "" ) ;
		//printd ( "ta: " ) ; printd ( text_from_var ( task_queue [ i ] ) ) ; printd ( "'\n" ) ;
	}
	
	printd ( "ta: task-queue cleared.\n" ) ;
	
	tasklist_unblock () ;
}


int
task_add_to_queue ( char *name, char *attr )
{
	int i ;

	check_tasklist_blocked () ;
	tasklist_block () ;

	for ( i = 1 ; i < 256 ; i++ )
	{
		int len = strlen ( task_queue [ i ].command ) ;
		if ( len == 0 )
		{
			printd ( "ta: set task queue number " ) ; printd ( text_from_var ( i ) ) ;
			printd ( " to '" ) ; printd ( name ) ; printd ( "' \n" ) ;
			sprintf ( task_queue [ i ].command, "%s", name ) ;
			sprintf ( task_queue [ i ].attr, "%s", attr ) ;
			tasklist_unblock () ;
			return i ;
		}
	}
	printd ( "ta: failed to insert task into queue!!\n" ) ;
	tasklist_unblock () ;
	return -1 ;
}


int
task_already_exists_in_queue ( char *name, char *attr )
{
	int i, count = 0 ;

	check_tasklist_blocked () ;
	tasklist_block () ;

	for ( i = 1 ; i < 256 ; i++ )
	{
		//printd ( "ta: " ) ; printd ( text_from_var(strlen(task_queue[i])) ) ; printd ( ": " ) ; printd ( task_queue[i] ) ; printd ( "\n" ) ;
		if ( strcmp ( task_queue [ i ].command, name ) == 0 )
			count = i ;
	}

	tasklist_unblock () ;
	return count ;
}


int
task_remove_from_queue ( int tasknum ) 
{
	//printd ( "ta: delete entry from queue\n" ) ;
	sprintf ( task_queue [ tasknum ].command, "%s", "" ) ;
	sprintf ( task_queue [ tasknum ].attr, "%s", "" ) ;
	return tasknum ;
}


int
task_remove_all_from_queue ( char *name, char *attr )
{
	int i, count = 0 ;
	
	check_tasklist_blocked () ;
	tasklist_block () ;

	for ( i = 1 ; i < 256 ; i++ )
	{
		//printf ( "ta: %s / %s / %d / %d \n", task_queue [i].command, name, strlen( task_queue[i].command ), strlen( name ) ) ;
		if ( strcmp ( task_queue [ i ].command, name ) == 0 && 
			( attr == NULL || strcmp ( task_queue [ i ].attr, attr ) == 0 ) )
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


// imageserver ( new thread !! )

void
start_image_server ( void )
{
	pthread_t image_thread ;
	
	printd("ta: is: create image server thread...\n");
	pthread_create ( &image_thread, NULL, (void*)&image_server_thread, NULL ) ;
	pthread_detach ( image_thread ) ;
	printd("ta: is: image server thread created!\n");
}


void
image_server_wait_for_final_entry ( char *name )
{
	printd ( "ta: is: image server got " ) ; printd ( name ) ; printd ( " signal!\n" ) ;
	gdk_threads_enter () ;
	while ( GTK_WIDGET_VISIBLE ( MainWindow ) != TRUE )
	{ 
		gdk_threads_leave () ;
		sleep ( 1 ) ;
		gdk_threads_enter () ;
	}
	gdk_threads_leave () ;
	while ( task_already_exists_in_queue ( name, NULL ) )
	{
		usleep ( 100000 ) ;
		task_remove_all_from_queue ( name, NULL ) ;
	}
}


void
image_server_thread ( void )
{
	//int quit = FALSE ;
	
	printd ( "ta: is: image server is running!\n" ) ;
	
	thread_count++ ;
	
	while ( task_already_exists_in_queue ( "quit", NULL ) == FALSE )
	{
		//printd ( "ta: is: image server is sleeping...\n" ) ;
		
		usleep ( 1000 ) ;
		
		//update_screen () ;
		
		if ( task_already_exists_in_queue ( "display_fullscreen", NULL ) )
		{
			image_server_wait_for_final_entry ( "display_fullscreen" ) ;
			if ( task_already_exists_in_queue ( "load_image", NULL ) == FALSE &&
				task_already_exists_in_queue ( "quit", NULL ) == FALSE )
			{
				printd("ta: is: image server display image fullscreen...\n");
				gdk_threads_enter () ;
				view_image ( "__UPDATE__", FullscreenWindow ) ;
				gdk_threads_leave () ;
			}
		}

		if ( task_already_exists_in_queue ( "display_in_window", NULL ) )
		{
			image_server_wait_for_final_entry ( "display_in_window" ) ;
			if ( task_already_exists_in_queue ( "load_image", NULL ) == FALSE &&
				task_already_exists_in_queue ( "quit", NULL ) == FALSE )
			{
				printd("ta: is: image server display image in window...\n");
				gdk_threads_enter () ;
				view_image ( "__UPDATE__", MainWindow ) ;
				gdk_threads_leave () ;
			}
		}
		
		if ( task_already_exists_in_queue ( "reload_image", NULL ) )
		{
			int entry = task_already_exists_in_queue ( "reload_image", NULL ) ;
			image_server_wait_for_final_entry ( "reload_image" ) ;
			task_remove_all_from_queue ( task_queue [entry].command, NULL ) ;
			printd("ta: is: image server reload image into memory...\n");
			gdk_threads_enter () ;
			//view_image ( filename, MainWindow ) ;
			//load_image ( filename ) ;
			if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) ) 
				view_image ( "__FORCE_RELOAD__", FullscreenWindow ) ;
			else
				view_image ( "__FORCE_RELOAD__", MainWindow ) ;
			gdk_threads_leave () ;			
		}

		if ( task_already_exists_in_queue ( "load_image", NULL ) )
		{
			int entry = task_already_exists_in_queue ( "load_image", NULL ) ;
			char filename[2048] ;			
			sprintf ( filename, "%s", task_queue [entry].attr ) ;
			image_server_wait_for_final_entry ( "load_image" ) ;
			task_remove_all_from_queue ( task_queue [entry].command, NULL ) ;
			printd("ta: is: image server load image into memory...\n");
			gdk_threads_enter () ;
			//view_image ( filename, MainWindow ) ;
			load_image ( filename ) ;
			if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) ) 
				view_image ( "__FORCE_RELOAD__", FullscreenWindow ) ;
			else
				view_image ( "__FORCE_RELOAD__", MainWindow ) ;
			gdk_threads_leave () ;
		}

	}
	
	thread_count-- ;
	printd ( "ta: is: image server terminated.\n" ) ;
}


// imageserver done.


void
wait_for_services_to_quit ( void )
{
	int max_count ;
	int old_thread_count = 0 ;
	
	task_add_to_queue ( "stop_dir_list", NULL ) ;
	task_add_to_queue ( "quit", NULL ) ;
	
	printd ( "ta: waiting for services to quit...\n" );
	
	//thread_count = thread_count + 500 ; // enable this just for testing :)
	
	max_count = thread_count ;
	
	if ( GTK_WIDGET_VISIBLE ( PrefsWindow ) ) gtk_widget_hide ( PrefsWindow ) ;
	if ( GTK_WIDGET_VISIBLE ( BgColorSelectionDialog ) ) gtk_widget_hide ( BgColorSelectionDialog ) ;
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) ) gtk_widget_hide ( FullscreenWindow ) ;
	gtk_widget_realize ( ShutdownWindow ) ;
	gtk_progress_configure ( GTK_PROGRESS(lookup_widget ( ShutdownWindow, "progressbar2" )), 0, 0, max_count ) ;
	//gtk_widget_show ( ShutdownWindow ) ;
	gtk_widget_realize ( ShutdownWindow ) ;
	refresh_screen () ;
	
	while ( thread_count > 0 )
	{
		usleep ( 1000 ) ;
		if ( old_thread_count != thread_count && thread_count > 0 )
		{
			old_thread_count = thread_count ;
			printd ( text_from_var(thread_count) ) ;
			printd ( "ta:  thread(s) left...\n" ) ;
			if ( GTK_WIDGET_VISIBLE ( ShutdownWindow ) )
				gtk_progress_set_value ( GTK_PROGRESS(lookup_widget ( ShutdownWindow, "progressbar2" )), 
											max_count - thread_count ) ;
			if ( GTK_WIDGET_VISIBLE ( ShutdownWindow ) == FALSE ) gtk_widget_show ( ShutdownWindow ) ;
		}
		refresh_screen () ;
		//update_screen () ;
		//thread_count-- ;
	} ;
	
	if ( GTK_WIDGET_VISIBLE ( MainWindow ) ) gtk_widget_hide ( MainWindow ) ;

	printd ( "ta: no more threads left. done. \n" ) ;
	if ( GTK_WIDGET_VISIBLE ( ShutdownWindow ) )
	{
		gtk_progress_set_value ( GTK_PROGRESS(lookup_widget ( ShutdownWindow, "progressbar2" )), max_count - thread_count ) ;
		refresh_screen () ;
		sleep ( 1 ) ;
		gtk_widget_hide ( ShutdownWindow ) ;
		refresh_screen () ;
	}
	
	gtk_main_quit();
}

