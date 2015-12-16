#include <gnome.h>

typedef struct
{
	char command[256] ;
	gpointer *attr ;
} task_entry ;

void
check_tasklist_blocked ( void ) ;

void
tasklist_block ( void ) ;

void
tasklist_unblock ( void ) ;

void
task_clear_queue ( void ) ;

int
task_add_to_queue ( char *name, gpointer *attr ) ;

int
task_already_exists_in_queue ( char *name, gpointer *attr ) ;

int
task_remove_from_queue ( int tasknum ) ;

int
task_remove_all_from_queue ( char *name, gpointer *attr ) ;

int
task_get_last_in_queue ( int tasknum ) ;

int
task_how_many_in_queue ( int tasknum ) ;

void
wait_for_services_to_quit ( void ) ;

