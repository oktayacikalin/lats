#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <sys/stat.h>
#include <dirent.h>

#include "interface.h"
#include "support.h"

#include "dirlist.h"
#include "imageview.h"
#include "tasks.h"

#include "spider.xpm"


// imported from mainwindow.c
GtkWidget extern *MainWindow ;

GtkWidget extern *file_statusbar ;
GtkWidget extern *file_tree ;
GtkWidget extern *file_icons ;

GtkWidget extern *image_canvas ;
GtkWidget extern *image_statusbar ;
GtkWidget extern *image_multipage_box ;
GtkWidget extern *image_multipage_progressbar ;
//

// imported from tasks.c
int extern thread_count ; // threads
//

// imported from imageview.c
GList extern *Slide_list ;
//

// will be exported to other .c files
GtkTreeStore *file_tree_store ;
//


// returns whether if it's an image or not
gboolean
isimage ( char *filename )
{
	char mime_type[2048] ;
	//char *filename, current_path[2048] ;
	/* we might want to keep video/ too, if imlib didn't crash trying to make
	thumbnails of them... - presumably it's meant to work. */
	
	//getcwd ( current_path, 2048 ) ;
	
	//filename = g_strdup_printf ( "%s/%s", current_path, file ) ;
	
	sprintf ( mime_type, "%s", gnome_vfs_get_mime_type ( filename ) ) ;
	
	if ( strncmp ( mime_type, "image/", 6 ) == 0 )
	{
		return (TRUE);
	}
	
	return (FALSE);
}

// returns whether if it's a directory or not
int
isdir ( char *filename )
{
        DIR *dir;
        struct stat buf;

        //printd("stat\n");
        stat ( filename, &buf );

        if ( S_ISDIR (buf.st_mode) && ( dir=opendir(filename) ) && strcmp(filename,".") && strcmp(filename,"..") )
        {
                closedir(dir);
                return (TRUE);
        }
        
        return (FALSE);
}

// returns whether it's a link or not
int
islink ( char *filename )
{
	return ( readlink ( filename, NULL, 2048 ) > -1 ) ? TRUE : FALSE ;
}

// returns the path the link it pointing to or FALSE
char
*getlink ( char *filename )
{
	char real_path[2048] ;
	int count = readlink ( filename, real_path, 2048 ) ;
	if ( count > -1 )
	{
		real_path[count] = '\0' ;
		return g_strdup_printf ( "-> %s", real_path ) ;
	} else
		return FALSE ;
}

// returns TRUE if we don't want to see this file...
int
file_filter ( char *filename )
{
	if ( strncmp ( filename, ".", 1 ) == FALSE )
		return TRUE ;
	if ( strncmp ( filename, "..", 2 ) == FALSE )
		return TRUE ;
	
	return FALSE ;
}


void
remove_old_dir_entries ( void )
{
	guint statusbar_id ;
	
	if ( Slide_list == NULL ) return ;
	
	statusbar_id = gtk_statusbar_get_context_id ( GTK_STATUSBAR(image_statusbar), "dirlist" ) ;
	
	gdk_threads_enter () ;
	gtk_statusbar_push ( GTK_STATUSBAR(image_statusbar), statusbar_id, "removing old thumbnails from desktop..." ) ;
	gdk_threads_leave () ;
	
	Slide_list = g_list_first ( Slide_list ) ;
	
	while ( g_list_length(Slide_list) > 0 )
	{
		GnomeCanvasItem *item ;
		gchar *item_filename ;
		
		item_filename = Slide_list->data ;
		
		if ( item_filename != NULL )
			item = gtk_object_get_data ( GTK_OBJECT(image_canvas), item_filename ) ;
		
		if ( item != NULL && GNOME_IS_CANVAS_ITEM(item) )
		{
			gdk_threads_enter () ;
			gtk_object_destroy ( GTK_OBJECT(item) ) ;
			Slide_list = g_list_remove ( Slide_list, item_filename ) ;
			gdk_threads_leave () ;
		}
	}
	
	gdk_threads_enter () ;
	gtk_statusbar_pop ( GTK_STATUSBAR(image_statusbar), statusbar_id ) ;
	gnome_canvas_update_now ( GNOME_CANVAS(image_canvas) ) ;
	gdk_threads_leave () ;

}

void
read_dir_entries_from_path_thread ( char *dirname )
{
	struct dirent **namelist ;
	int namelist_count = 0, namelist_pos = 0 ;
	char dest_path[2048], current_path[2048], real_path[2048] ;
	int own_task ;
	guint statusbar_id, message_id ;
	
	//g_print ( _("removing old items...\n") ) ;
	remove_old_dir_entries () ;
	
	if ( dirname == NULL || isdir(dirname) == FALSE ||
		 task_already_exists_in_queue ( "stop_read_dir_entries", NULL ) )
		return ;
	
	own_task = task_add_to_queue ( "read_dir_entries", NULL ) ;
	thread_count++ ;
	
	statusbar_id = gtk_statusbar_get_context_id ( GTK_STATUSBAR(image_statusbar), "dirlist" ) ;
	
	gdk_threads_enter () ;
	message_id = gtk_statusbar_push ( GTK_STATUSBAR(image_statusbar), statusbar_id, "searching for viewable files..." ) ;
	gdk_threads_leave () ;

	//g_print ( _("reading dir entries from %s...\n"), dirname ) ;
	
	sprintf ( current_path, "%s", dirname ) ;

	namelist_count = scandir ( current_path, &namelist, 0, alphasort ) ;
	
	if ( namelist_count < 0 ||
		 task_already_exists_in_queue ( "stop_read_dir_entries", NULL ) )
	{
		//g_print ( _("nothing found.\n") ) ;
		gdk_threads_enter () ;
		gtk_statusbar_remove ( GTK_STATUSBAR(image_statusbar), statusbar_id, message_id ) ;
		gdk_threads_leave () ;
		if ( namelist ) free ( namelist ) ;
		task_remove_from_queue ( own_task ) ;
		thread_count-- ;
		return ;
	}
	
	//g_print ( _("found %d entries in current directory ( %s ).\n"), namelist_count, current_path ) ;
	
	for ( namelist_pos = 0 ; 
			namelist_pos < namelist_count && 
			task_already_exists_in_queue ( "stop_read_dir_entries", NULL ) == FALSE
			; namelist_pos++ )
	{
		char current_file[2048], current_dir[2048] ;
		
		sprintf ( current_dir, "%s", current_path ) ;
		if ( current_dir[strlen(current_dir)-1] != '/' )
			sprintf ( current_dir, "%s/", current_dir ) ;
		sprintf ( current_file, "%s%s", current_dir, namelist[namelist_pos]->d_name ) ;
		
		gdk_threads_enter () ;
		gtk_statusbar_remove ( GTK_STATUSBAR(image_statusbar), statusbar_id, message_id ) ;
		message_id = gtk_statusbar_push ( GTK_STATUSBAR(image_statusbar), statusbar_id, 
											g_strdup_printf("reading file %d of %d...", 
															namelist_pos, namelist_count) ) ;
		gdk_threads_leave () ;
		
		if ( isimage ( current_file ) == TRUE && 
				file_filter ( current_file ) == FALSE )
		{
			//g_print ( _("found an image!\n") ) ;
			//g_print ( _("put it into our database...\n") ) ;
			add_new_slide ( current_file, -1, -1 ) ;
		}
	}
	free ( namelist ) ;
	
	g_free ( dirname ) ;

	gdk_threads_enter () ;
	gtk_statusbar_remove ( GTK_STATUSBAR(image_statusbar), statusbar_id, message_id ) ;
	gdk_threads_leave () ;
	
	task_remove_from_queue ( own_task ) ;
	thread_count-- ;
}

void
read_dir_entries_from_path ( char *dirname )
{
	pthread_t read_dir_thread ;
	int own_task = task_add_to_queue ( "stop_read_dir_entries", NULL ) ;

	while ( task_already_exists_in_queue ( "read_dir_entries", NULL ) )
	{
		usleep ( 10000 ) ;
		refresh_screen () ;
	}
	
	task_remove_from_queue ( own_task ) ;
	
	pthread_create ( &read_dir_thread, NULL, (void*)&read_dir_entries_from_path_thread, dirname ) ;
	pthread_detach ( read_dir_thread ) ;
}


// this one is for the dir-tree-widget!
// they represent the columns plus the count-variable.
enum
{
	DIRNAME_COLUMN,
	ATTRIBUTE_COLUMN,
	REALPATH_COLUMN,
	N_COLUMNS
} ;

void
scroll_to_cell ( GtkTreePath *tree_path )
{
	float h_align = 0.5 ; // horizontal alignment
	float v_align = 0.3 ; // vertical alignment
	int use_alignment = TRUE ; // whether we want to use it

	gtk_tree_view_scroll_to_cell (	GTK_TREE_VIEW(file_tree),
									tree_path,
									DIRNAME_COLUMN,
									use_alignment,
									v_align,
									h_align ) ;
	
	gtk_widget_realize ( GTK_WIDGET(file_tree) ) ;
	refresh_screen () ;
}

int
check_for_subdir ( char *dirname )
{
	struct dirent **namelist ;
	int namelist_count = 0, namelist_pos = 0 ;
	char dest_path[2048], current_path[2048], real_path[2048] ;
	
	if ( dirname == NULL || isdir(dirname) == FALSE ) return FALSE ;
	
	sprintf ( current_path, "%s", dirname ) ;

	//g_print ( _("checking for subdir in %s...\n"), current_path ) ;
	
	namelist_count = scandir ( current_path, &namelist, 0, alphasort ) ;
	
	if ( namelist_count < 0 )
	{
		//g_print ( _("nothing found.\n") ) ;
		if ( namelist ) free ( namelist ) ;
		return FALSE ;
	}
	
	//g_print ( _("found %d entries in current directory ( %s ).\n"), namelist_count, current_path ) ;
	
	for ( namelist_pos = 0 ; namelist_pos < namelist_count ; namelist_pos++ )
	{
		chdir ( current_path ) ;
		if ( isdir ( namelist[namelist_pos]->d_name ) == TRUE && 
				file_filter ( namelist[namelist_pos]->d_name ) == FALSE )
		{
			free ( namelist ) ;
			return TRUE ;
		}
	}
	free ( namelist ) ;
	
	return FALSE ;
}

void
insert_dirname_into_subtree ( 	GtkTreeStore *store, 	// whole tree
								GtkTreeIter iter1, 		// new item
								GtkTreeIter iter2, 		// parent item
								char *dirname, 			// item-name
								char *attrib,			// attribute
								char *realpath )		// the real path
{
	GtkTreeIter temp_iter ;
	
	gtk_tree_store_append ( store, &iter1, &iter2 ) ;
	gtk_tree_store_set (	store, &iter1,
							DIRNAME_COLUMN, dirname,
							ATTRIBUTE_COLUMN, attrib,
							REALPATH_COLUMN, realpath,
							-1 ) ;	

	if ( check_for_subdir ( realpath ) == TRUE )
		insert_dirname_into_subtree ( store, temp_iter, iter1, "...", NULL, NULL ) ;
}

void
insert_dirname_into_tree ( 	GtkTreeStore *store, 		// whole tree
								GtkTreeIter iter, 		// new item
								char *dirname, 			// item-name
								char *attrib,			// attribute
								char *realpath )		// the real path
{
	GtkTreeIter temp_iter ;
	
	gtk_tree_store_append ( store, &iter, NULL ) ;
	gtk_tree_store_set (	store, &iter,
							DIRNAME_COLUMN, dirname,
							ATTRIBUTE_COLUMN, attrib,
							REALPATH_COLUMN, realpath,
							-1 ) ;	
	
	/*{
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data ( (const char**) spider_xpm ) ;
		gtk_tree_store_set ( store, &iter, DIRNAME_COLUMN, pixbuf, -1 ) ;
	}*/
	
	if ( check_for_subdir ( realpath ) == TRUE )
		insert_dirname_into_subtree ( store, temp_iter, iter, "...", NULL, NULL ) ;
}

GtkTreePath
*get_tree_path_by_real_path ( GtkTreeStore *store, GtkTreePath *tree_path, gchar *real_path )
{	
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(file_tree)) ;
	GtkTreeIter iter ;
	int count = 0 ;
	gboolean valid ;
	
	// FIXME: whats this??? if the following line is missing my tree won't expand
	//        correctly?!??! I have to g_print something with _(string) ! ???? bah!
	g_print ( _("") ) ;
	
	//g_print ( _("searching for dir called %s...\n"), real_path ) ;
	
	if ( gtk_tree_model_get_iter ( model, &iter, tree_path ) == FALSE )
		return tree_path ;
	
	//g_print ( _("tree_path is %s\n"), gtk_tree_path_to_string ( tree_path ) ) ;
	
	//g_print ( _("while...\n") ) ;
	while (valid)
	{
		gchar *temp_real_path ;
		//g_print ( _("current child number = %d\n"), count ) ;
		gtk_tree_model_get ( model, &iter, REALPATH_COLUMN, &temp_real_path, -1 ) ;
		if ( temp_real_path != NULL && strcmp ( temp_real_path, real_path ) == FALSE )
		{
			tree_path = gtk_tree_model_get_path ( model, &iter ) ;
			//g_print ( _("got child %d where tree_path is %s\n"), count, 
			//			gtk_tree_path_to_string ( tree_path ) ) ;
			return tree_path ;
		}
		valid = gtk_tree_model_iter_next ( model, &iter ) ;
		count++ ;
		g_free ( temp_real_path ) ;
	}
	
	return tree_path ;
}

GtkTreePath
*expand_whole_dir_path (	GtkTreeStore *store,
						char *dirname )
{
	int dirname_length = strlen (dirname) ;
	int dirname_pos = 0 ;
	char current_dir[2048] ;
	int current_dir_pos = 0 ;
	GtkTreePath *tree_path = gtk_tree_path_new_first () ;
	
	if ( dirname[dirname_length-1] != '/' )
	{
		sprintf ( dirname, "%s/", dirname ) ;
		dirname_length = strlen (dirname) ;
	}
	
	//g_print ( _("expanding dir-tree to %s\n"), dirname ) ;
	
	while ( dirname_pos < dirname_length )
	{
		char homedir[2048] ;
		
		sprintf ( homedir, "%s", gnome_vfs_expand_initial_tilde("~/") ) ;
		
		if ( strcmp ( dirname, homedir ) == FALSE )
			dirname_pos = current_dir_pos = dirname_length ;
		
		if (	( dirname[dirname_pos] == '/' && current_dir_pos > 0 ) ||
				current_dir_pos == dirname_length ||
				( dirname[0] == '/' && dirname_pos == 1 ) )
		{
			current_dir[current_dir_pos] = '\0' ;
			//g_print ( _("current dir is %s\n"), g_strndup(dirname, dirname_pos) ) ;
			current_dir_pos = 0 ;
			if ( isdir(g_strndup(dirname, dirname_pos)) )
			{
				tree_path = get_tree_path_by_real_path(store, tree_path, g_strndup(dirname, dirname_pos)) ;
				gtk_tree_view_expand_row (	GTK_TREE_VIEW(file_tree), tree_path, FALSE ) ;
				gtk_tree_path_down ( tree_path ) ;
				scroll_to_cell ( tree_path ) ;
			}
		} else {
			current_dir[current_dir_pos++] = dirname[dirname_pos] ;
		}
		
		dirname_pos++ ;
	}

	current_dir[current_dir_pos] = '\0' ;
	//g_print ( _("last current dir is %s\n"), current_dir ) ;

	gtk_tree_path_up ( tree_path ) ;
	
	scroll_to_cell ( tree_path ) ;
	
	return tree_path ;
}

void
read_subdir_contents (	GtkTreeStore *store,
						GtkTreeIter parent_iter,
						char *dirname )
{
	struct dirent **namelist ;
	int namelist_count = 0, namelist_pos = 0 ;
	char current_path[2048] ;
	GtkTreeIter iter ;
	
	if ( dirname == NULL || isdir(dirname) == FALSE ) return ;
	
	sprintf ( current_path, "%s", dirname ) ;
	
	namelist_count = scandir ( current_path, &namelist, 0, alphasort ) ;
	
	if ( namelist_count < 0 )
	{
		if ( namelist ) free ( namelist ) ;
		return ;
	}
	
	//g_print ( _("found %d entries in current directory ( %s ).\n"), namelist_count, current_path ) ;
	
	for ( namelist_pos = 0 ; namelist_pos < namelist_count ; namelist_pos++ )
	{
		char real_path[2048] ;
		chdir ( current_path ) ;
		//g_print ( _("current name in list is %s\n"), namelist[namelist_pos]->d_name ) ;
		if ( isdir ( namelist[namelist_pos]->d_name ) == TRUE && 
				file_filter ( namelist[namelist_pos]->d_name ) == FALSE )
		{
			//g_print ( _("look if it's a bad link...") ) ;
			if ( realpath(namelist[namelist_pos]->d_name, real_path) == NULL )
			{
				//g_print ( _(" yes! real_path not entered!!\n") ) ;
				strcpy ( real_path, "" ) ;
			} else 
				//g_print ( _(" no. real_path will be entered...\n") ) ;
			
			//g_print ( _("now put it into our tree...\n") ) ;
			insert_dirname_into_subtree ( store, iter, parent_iter, namelist[namelist_pos]->d_name, 
										getlink(namelist[namelist_pos]->d_name),
										real_path ) ;
		}
		if ( islink ( namelist[namelist_pos]->d_name ) && 
				file_filter ( namelist[namelist_pos]->d_name ) == FALSE )
		{
			strcpy ( real_path, realpath ( namelist[namelist_pos]->d_name, real_path ) ) ;
			//g_print ( _("%s is a link pointing at %s .\n"), namelist[namelist_pos]->d_name, real_path ) ;
		}
	}
	free ( namelist ) ;
}

void
on_file_tree_expanded ( GtkTreeView *tree_view, GtkTreeIter *iter, 
						GtkTreePath *path, gpointer *user_data )
{
	gchar *current_path = gtk_tree_path_to_string ( path ) ;
	gchar *real_path ;
	GtkTreeIter temp_iter ;
	
	gtk_tree_model_get ( gtk_tree_view_get_model(tree_view), iter, REALPATH_COLUMN, &real_path, -1 ) ;
	gtk_tree_model_get_iter ( gtk_tree_view_get_model(tree_view), &temp_iter, path ) ;
		
	if ( gtk_tree_model_iter_n_children ( gtk_tree_view_get_model(tree_view), iter ) == 1 )
	{
		GtkTreeIter child ;
		gtk_tree_model_iter_children ( gtk_tree_view_get_model(tree_view), &child, iter ) ;
		read_subdir_contents ( (GtkTreeStore*) user_data, temp_iter, real_path ) ;
		gtk_tree_store_remove ( (GtkTreeStore*) user_data, &child ) ;
	}
	
	g_free ( current_path ) ;
	g_free ( real_path ) ;
}

void
on_file_tree_collapsed ( GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path )
{
	gchar *current_path = gtk_tree_path_to_string ( path ) ;
	g_free ( current_path ) ;
}

// someone selected or unselected an item in our tree...
void
on_file_tree_selection_changed (	GtkTreeSelection *selection,
									GtkTreeModel *model,
									GtkTreePath *path,
									gboolean selected,
									gpointer user_data )
{
	gchar *real_path ;
	GtkTreeIter iter ;
	GtkTreeModel *sel_model ;
	
	int own_task ;
	
	if ( gtk_tree_selection_get_selected (	selection, &sel_model, &iter ) != TRUE ) return ;
	
	own_task = task_add_to_queue ( "stop_read_dir_entries", NULL ) ;
	
	while ( task_already_exists_in_queue ( "read_dir_entries", NULL ) )
	{
		usleep ( 10000 ) ;
		refresh_screen () ;
	}
	
	task_remove_from_queue ( own_task ) ;
	
	if ( gtk_tree_selection_get_selected (	selection, &sel_model, &iter ) != TRUE ) return ;

	gtk_tree_model_get ( sel_model, &iter, REALPATH_COLUMN, &real_path, -1 ) ;

	read_dir_entries_from_path ( real_path ) ;
}

void
create_dir_list ( void )
{
	char dest_path[2048] ;
	GtkTreeStore *store ;
	GtkTreeViewColumn *column ;
	GtkCellRenderer *renderer ;
	
	store = gtk_tree_store_new (N_COLUMNS,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING) ;
	
	
	gtk_tree_view_set_model ( GTK_TREE_VIEW(file_tree), GTK_TREE_MODEL(store) ) ;
	g_object_unref ( G_OBJECT(store) ) ;
	
	renderer = gtk_cell_renderer_text_new () ;
	column = gtk_tree_view_column_new_with_attributes (	"Dirname", renderer,
														"text", DIRNAME_COLUMN,
														NULL ) ;
	gtk_tree_view_append_column ( GTK_TREE_VIEW(file_tree), column ) ;
	
	{
		GtkCellRenderer *pixbufrenderer ;
		
		pixbufrenderer = gtk_cell_renderer_pixbuf_new () ;
		gtk_tree_view_column_pack_start ( column, pixbufrenderer, TRUE ) ;
	}
	
	renderer = gtk_cell_renderer_text_new () ;
	column = gtk_tree_view_column_new_with_attributes (	"Attributes", renderer,
														"text", ATTRIBUTE_COLUMN,
														NULL ) ;
	gtk_tree_view_append_column ( GTK_TREE_VIEW(file_tree), column ) ;
	
	renderer = gtk_cell_renderer_text_new () ;
	column = gtk_tree_view_column_new_with_attributes (	"Realpath", renderer,
														"text", REALPATH_COLUMN,
														NULL ) ;
	gtk_tree_view_append_column ( GTK_TREE_VIEW(file_tree), column ) ;
	gtk_tree_view_column_set_visible ( column, FALSE ) ;
	
	gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW(file_tree), FALSE ) ;
	
	gtk_tree_view_set_enable_search ( GTK_TREE_VIEW(file_tree), FALSE ) ;
	
	g_signal_connect ( G_OBJECT(GTK_TREE_VIEW(file_tree)), "row_expanded",
						G_CALLBACK (on_file_tree_expanded),
						store ) ;
	g_signal_connect ( G_OBJECT(GTK_TREE_VIEW(file_tree)), "row_collapsed",
						G_CALLBACK (on_file_tree_collapsed),
						NULL ) ;
	g_signal_connect ( G_OBJECT(gtk_tree_view_get_selection ( GTK_TREE_VIEW(file_tree) )), "changed",
						G_CALLBACK (on_file_tree_selection_changed),
						store ) ;
	
	getcwd ( dest_path, 2048 ) ;

	{
		GtkTreePath *tree_path ;
		GtkTreeIter my_computer_iter, my_home_iter, my_images_iter ;
		GtkTreeIter seperator1, seperator2 ;
		char homedir[2048], imagesdir[2048] ;
		
		sprintf ( homedir, "%s", gnome_vfs_expand_initial_tilde("~/") ) ;
		sprintf ( imagesdir, "%s", gnome_vfs_expand_initial_tilde("~/Bilder") ) ;
		
		// the home directory shortcut :)
		insert_dirname_into_tree ( store, my_home_iter, 
									_("My home"), 
									g_strdup_printf( _("your home directory ( %s )"), homedir ), 
									homedir ) ;

		// a seperator :)
		insert_dirname_into_tree ( store, seperator1, NULL, NULL, NULL ) ;

		// the images directory shortcut :)
		insert_dirname_into_tree ( store, my_images_iter, 
									_("My images"), 
									g_strdup_printf( _("where your images are ( %s )"), imagesdir ), 
									imagesdir ) ;
		
		// a seperator :)
		insert_dirname_into_tree ( store, seperator2, NULL, NULL, NULL ) ;
		
		// our root-tree :)
		insert_dirname_into_tree ( store, my_computer_iter, 
									_("My computer"), 
									_("the root-tree of your filesystem ( / )"), 
									"/" ) ;
		
		tree_path = expand_whole_dir_path ( store, dest_path ) ;

		//g_print ( _("select tree_path...\n") ) ;
		gtk_tree_selection_select_path (	gtk_tree_view_get_selection ( GTK_TREE_VIEW(file_tree) ),
											tree_path ) ;
	}
	
	file_tree_store = store ; // submit our newly created tree-store
}

