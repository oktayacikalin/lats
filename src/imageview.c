#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <sys/types.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <dirent.h> 

#include <unistd.h>

#include <string.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#include <libgnomevfs/gnome-vfs.h>

#include <pthread.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "importedfuncs.h"
#include "imagelist.h"
#include "imageview.h"
#include "colors.h"
#include "tasks.h"

// borrowed from gqview :)
#include "pixbuf_util.h"

#include "loading.xpm"

GdkPixbuf extern *loaded_image ; // the actual loaded image
GdkPixbuf extern *loaded_scaled_image ; // the scaled image in memory ( performance+ )

int extern render_quality ;
int extern check_size ;
guint32 extern check_color_a ;
guint32 extern check_color_b ;
guint32 extern check_color_out_a ;
guint32 extern check_color_out_b ;
float extern check_color_out_cut_off_factor ;
int extern check_color_out_use ;
int extern check_color_auto ;
int extern check_color_auto_routine ;

int extern thumbnails ;
int extern thumb_size ;
int extern max_thumb_size ;
int extern no_thumb_size ;
int extern thumb_render_quality ;

int extern filedisplay_as_icons ;

int extern image_mutlipage_fast_preview ;

char extern *LATS_RC_DIR ;
char extern *LATS_RC_TEMP_DIR ;

int extern Mouse_x ;
int extern Mouse_y ;

GtkWidget extern *MainWindow ;
GtkWidget extern *PrefsWindow ;
GtkWidget extern *BgColorSelectionDialog ;
GtkWidget extern *FullscreenWindow ;
GtkWidget extern *FullscreenWindowProgressbar ;

int extern thread_count ;


void
image_progressbar_thread ( void )
{
	int value = 0 ;
	GtkProgress *progress ;
	int own_task = task_add_to_queue ( "create_imageprogressbar", NULL ) ;
	thread_count++ ;
	
	gdk_threads_enter () ;
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow) == FALSE ) {
		if ( GNOME_APPBAR_HAS_PROGRESS ( GNOME_APPBAR (lookup_widget ( MainWindow, "appbar2" )) ) )
			progress = gnome_appbar_get_progress (GNOME_APPBAR (lookup_widget ( MainWindow, "appbar2" )));
		else
			progress = GTK_PROGRESS(GTK_PROGRESS_BAR(lookup_widget(MainWindow,"imageprogressbar"))) ;
	} else
		progress = gnome_appbar_get_progress (GNOME_APPBAR (lookup_widget ( FullscreenWindowProgressbar, "appbar2" )));
	gtk_progress_set_activity_mode ( progress, TRUE ) ;
	//gtk_widget_show ( lookup_widget( MainWindow, "imageprogressbar" ) ) ;
	gdk_threads_leave () ;
	
	while ( task_already_exists_in_queue ( "stop_imageprogressbar", NULL ) == FALSE &&
			task_already_exists_in_queue ( "quit", NULL ) == FALSE )
	{
		usleep ( 50000 ) ;
		gdk_threads_enter () ;
		
		if ( GTK_WIDGET_VISIBLE ( MainWindow ) )
			value = gtk_progress_get_value ( GTK_PROGRESS( progress ) ) ;
		
		if ( value < 100 )
			value++ ;
		else if ( value == 100 )
			value = 0 ;
		
		if ( GTK_WIDGET_VISIBLE ( MainWindow ) )
			gtk_progress_set_value ( GTK_PROGRESS( progress ), value ) ;
		
		gdk_threads_leave () ;
	}

	task_remove_from_queue ( own_task ) ;
	thread_count-- ;
}


void
stop_image_progressbar ( void )
{
	GtkProgress *progress ;
	int own_task = task_add_to_queue ( "stop_imageprogressbar", NULL ) ;
	
	while ( task_already_exists_in_queue ( "create_imageprogressbar", NULL ) )
	{
		usleep ( 10000 ) ;
		gdk_threads_enter () ;
		gdk_threads_leave () ;
	}
	
	gdk_threads_enter () ;
	
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow) == FALSE ) {
		if ( GNOME_APPBAR_HAS_PROGRESS ( GNOME_APPBAR (lookup_widget ( MainWindow, "appbar2" )) ) )
			progress = gnome_appbar_get_progress (GNOME_APPBAR (lookup_widget ( MainWindow, "appbar2" )));
		else
			progress = GTK_PROGRESS(GTK_PROGRESS_BAR(lookup_widget(MainWindow,"imageprogressbar"))) ;
	} else
		progress = gnome_appbar_get_progress (GNOME_APPBAR (lookup_widget ( FullscreenWindowProgressbar, "appbar2" )));
	gtk_progress_set_activity_mode ( progress, FALSE ) ;
	if ( GTK_WIDGET_VISIBLE ( MainWindow ) )
		gtk_progress_set_value ( GTK_PROGRESS( progress ), 0 ) ;
	
	//gtk_widget_hide ( lookup_widget( MainWindow, "imageprogressbar" ) ) ;
	gdk_threads_leave () ;

	task_remove_from_queue ( own_task ) ;
	task_remove_all_from_queue ( "stop_imageprogressbar", NULL ) ;
}


void
start_image_progressbar ( void )
{
	pthread_t imageprogressbar_thread ;

	/*if ( GNOME_APPBAR_HAS_PROGRESS ( lookup_widget ( MainWindow, "appbar2") ) )
		return ;*/
	
	stop_image_progressbar () ;
	
	pthread_create ( &imageprogressbar_thread, NULL, (void*)&image_progressbar_thread, NULL ) ;
	pthread_detach ( imageprogressbar_thread ) ;
}


void
mp_update_progressbar_thread ( void )
{
	int value = 0 ;

	int own_task = task_add_to_queue ( "create_progressbarmp", NULL ) ;
	thread_count++ ;
	
	gdk_threads_enter () ;
	gtk_widget_show ( lookup_widget( MainWindow, "progressbarmp" ) ) ;
	gdk_threads_leave () ;
	
	while ( task_already_exists_in_queue ( "stop_progressbarmp", NULL ) == FALSE &&
			task_already_exists_in_queue ( "quit", NULL ) == FALSE )
	{
		usleep ( 50000 ) ;
		gdk_threads_enter () ;
		
		value = gtk_progress_get_value ( GTK_PROGRESS( lookup_widget ( MainWindow, "progressbarmp" ) ) ) ;
		
		if ( value < 100 )
			value++ ;
		else if ( value == 100 )
			value = 0 ;
		
		gtk_progress_set_value ( GTK_PROGRESS( lookup_widget ( MainWindow, "progressbarmp" ) ), value ) ;
		
		gdk_threads_leave () ;
	}

	gdk_threads_enter () ;
	gtk_widget_hide ( lookup_widget( MainWindow, "progressbarmp" ) ) ;
	gdk_threads_leave () ;
	
	task_remove_from_queue ( own_task ) ;
	thread_count-- ;
}


void
mp_update_progressbar ( void )
{
	pthread_t progressbarmp_thread ;
	int own_task = task_add_to_queue ( "stop_progressbarmp", NULL ) ;
	
	while ( task_already_exists_in_queue ( "create_progressbarmp", NULL ) )
	{
		usleep ( 10000 ) ;
		gdk_threads_enter () ;
		gdk_threads_leave () ;
	}
	task_remove_from_queue ( own_task ) ;
	task_remove_all_from_queue ( "stop_progressbarmp", NULL ) ;
	
	pthread_create ( &progressbarmp_thread, NULL, (void*)&mp_update_progressbar_thread, NULL ) ;
	pthread_detach ( progressbarmp_thread ) ;	
}


void
mp_fill_sidebar_thread ( char *filename )
{
	GnomeIconList *iconlist ;
	struct stat buf2 ;
	int finish = FALSE ;
	char cmd[2048] ;
	int count, density, sum ;
	GtkProgress *progress ;
	//GtkCList *imagestatsclist ;
	char *temp_real_filename = NULL ;
	char real_filename[2048] ;

	int own_task = task_add_to_queue ( "create_page_list", NULL ) ;
	
	GdkImlibImage *loadpic = gdk_imlib_create_image_from_xpm_data( (char**) loading_xpm ) ;
	
	//imagestatsclist = GTK_CLIST(lookup_widget( MainWindow, "imagestatsclist" ));
	
	//gtk_clist_get_text ( imagestatsclist, 0, 1, &temp_real_filename );
	//strcpy ( real_filename, temp_real_filename ) ;
	strcpy ( real_filename, filename ) ;
	free ( temp_real_filename ) ;

	thread_count++ ;

	mp_update_progressbar () ; // progressbar activity..
	
	gdk_threads_enter ();

	progress = GTK_PROGRESS(GTK_PROGRESS_BAR(lookup_widget(MainWindow,"imageprogressbar"))) ;
	gtk_progress_set_format_string ( progress, "counting: %U" ) ;
	gtk_progress_set_show_text ( progress, TRUE ) ;
	gtk_progress_set_value ( progress, 1 ) ;
	
	iconlist = GNOME_ICON_LIST(lookup_widget(MainWindow, "iconlistmp")) ;
	gnome_icon_list_clear ( iconlist ) ;
	
	density = gtk_spin_button_get_value_as_int( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton9" ) ) ) ;
	
	count = 1 ; // reset page-counter

	gdk_threads_leave () ;
	
	//printf("iv: mp: start sidebar_thread with filename = %s\n", filename ) ;
	
	printd("iv: mp: start to fill the page-list...\n");
		
	if ( image_mutlipage_fast_preview )
	{
		while ( finish == FALSE && task_already_exists_in_queue ( "quit", NULL ) == FALSE &&
				task_already_exists_in_queue ( "stop_page_list", NULL ) == FALSE )
		{
			char to_file[2048], dir[2048] ;
			char destfile[2048], mini_file[2048] ;
	
			strcpy ( dir, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
			{
				char name[2048] ;
				int pos ;
				getcwd ( name, 2048 ) ;
				strcat ( name, "/" ) ;
				strcat ( name, real_filename ) ;
				for ( pos = 0 ; pos < strlen(name) ; pos++ )
				{
					//printd( text_from_var(  strlen(index(name,47))  ) ); printd(" ");
					if ( name[pos] == 47 ) name[pos] = 183 ;
				}

				if ( !rindex ( real_filename, 47 ) )
					sprintf ( to_file, "%s%s%s.%dDPI.%03d.ps", dir, LATS_RC_TEMP_DIR, name, density, count ) ;
				else
					sprintf ( to_file, "%s.%dDPI.%03d.ps", real_filename, density, count ) ;
			}
			sprintf ( cmd, "acroread -toPostScript -binary -fast -start %d -end %d -pairs \'%s\' \'%s\'", count, count, real_filename, to_file ) ;
			printd("iv: mp: "); printd ( cmd ) ; printd ( "\n" ) ;

			sprintf( destfile, "%s.temp", to_file ) ;
			sprintf( mini_file, "%s.mini", destfile ) ;

			//if ( stat ( to_file, &buf2 ) == 0 ) unlink ( to_file ) ;
			if ( stat ( destfile, &buf2 ) != 0 && stat ( to_file, &buf2 ) != 0 ) system(cmd) ;
			if ( stat ( destfile, &buf2 ) == 0 || 
					( stat ( to_file, &buf2 ) == 0 && buf2.st_size > 0 ) )
			{
				char page_name[2048] ;
	
				gdk_threads_enter () ;
							
				if ( count == 2 )
				{	// more than one page viewable - display sidebar
					if ( ! GTK_WIDGET_VISIBLE ( GTK_WIDGET ( progress ) ) )
						gtk_widget_show ( GTK_WIDGET ( progress ) ) ;
					gtk_widget_set_usize ( GTK_WIDGET(lookup_widget(MainWindow,"multipagebox")), 
											max_thumb_size + (max_thumb_size*0.20), -1 ) ;
					if ( !GTK_WIDGET_VISIBLE ( lookup_widget ( MainWindow, "multipagebox" ) ) )
						gtk_widget_show ( lookup_widget ( MainWindow, "multipagebox" ) ) ;
					if ( !GTK_WIDGET_VISIBLE ( lookup_widget ( MainWindow, "progressbarmp" ) ) )
						gtk_widget_show ( lookup_widget ( MainWindow, "progressbarmp" ) ) ;
				}
				
				sprintf ( page_name, "page %d", count ) ;
				//printf("iv: mp: append page number %d from %s...\n", count, destfile ) ;
				gnome_icon_list_append_imlib ( iconlist, loadpic, page_name ) ;
				
				gtk_progress_configure ( progress, gtk_progress_get_value(progress), 1, count ) ;
				
				gdk_threads_leave () ;
				
				count++ ;
			} else {
				finish = TRUE ;
				count-- ;
			}
		} // while
	
		sum = count ; // get thumbnail count
		
		// do something with this...
		printd ( "iv: mp: counted ") ; printd ( text_from_var(sum) ) ; printd ( " thumbnails!\n" ) ;
		
		// go insert those thumbnails!
		
		count = 1 ; // reset page-counter
		finish = FALSE ;
	}

	gdk_threads_enter () ;
	gtk_progress_set_format_string ( progress, "page %V of %U" ) ;
	gdk_threads_leave () ;

	while ( finish == FALSE && task_already_exists_in_queue ( "quit", NULL ) == FALSE &&
			task_already_exists_in_queue ( "stop_page_list", NULL ) == FALSE )
	{
		char to_file[2048], dir[2048] ;
		char destfile[2048], mini_file[2048] ;

		strcpy ( dir, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
		{
			char name[2048] ;
			int pos ;
			getcwd ( name, 2048 ) ;
			strcat ( name, "/" ) ;
			strcat ( name, real_filename ) ;
			for ( pos = 0 ; pos < strlen(name) ; pos++ )
			{
				if ( name[pos] == 47 ) name[pos] = 183 ;
			}

			if ( !rindex ( real_filename, 47 ) )
				sprintf ( to_file, "%s%s%s.%dDPI.%03d.ps", dir, LATS_RC_TEMP_DIR, name, density, count ) ;
			else
				sprintf ( to_file, "%s.%dDPI.%03d.ps", real_filename, density, count ) ;
		}
		sprintf ( cmd, "acroread -toPostScript -binary -fast -transQuality 5 -start %d -end %d -pairs '%s' '%s'", count, count, real_filename, to_file ) ;
		printd ( "iv: mp: " ) ; printd ( cmd ) ; printd ( "\n" ) ;
		
		sprintf( destfile, "%s.temp", to_file ) ;
		sprintf( mini_file, "%s.mini", destfile ) ;

		//if ( stat ( to_file, &buf2 ) == 0 ) unlink ( to_file ) ;
		if ( stat ( destfile, &buf2 ) != 0 && stat ( to_file, &buf2 ) != 0 ) system(cmd) ;
		if ( stat ( destfile, &buf2 ) == 0 || 
				( stat ( to_file, &buf2 ) == 0 && buf2.st_size > 0 ) )
		{
			int thumb_exist = FALSE ;
			if ( stat ( destfile, &buf2 ) != 0 )
			{
				printd("iv: mp: convert page "); printd(text_from_var(count)); printd(" to an image...\n");
				thumb_exist = FALSE ;
			} else
				thumb_exist = TRUE ;
			strcpy ( cmd, "gs -sDEVICE=jpeg -r" ) ;
			strcat ( cmd, text_from_var ( density ) ) ;
			strcat ( cmd, " -q -dSAFER -dNOPAUSE -sOutputFile='" ) ;
			strcat ( cmd, destfile ) ;
			strcat ( cmd, "' -- '" ) ;
			strcat ( cmd, to_file ) ;
			strcat ( cmd, "'" ) ;
			printd("iv: mp: "); printd(cmd); printd("\n");
			if ( (thumb_exist == FALSE && system(cmd) == 0) || thumb_exist == TRUE )
			{
				char page_name[2048] ;

				if ( stat ( mini_file, &buf2 ) !=0 )
				{
					GdkPixbuf *unscaled_image = NULL, *scaled_image = NULL ;
					int width = 1, height = 1, scaled_width = 1, scaled_height = 1 ;
					double scale ;

					unscaled_image = gdk_pixbuf_new_from_file ( destfile ) ;
	
					if ( ! unscaled_image )
					{
						char cmd2[2048] ;
						printd ( "iv: mp: failed to read thumbnail! trying to recreate...\n" ) ;
						sprintf ( cmd2, "acroread -toPostScript -binary -fast -transQuality 5 -start %d -end %d -pairs '%s' '%s'", 
									count, count, real_filename, to_file ) ;
						if ( system(cmd2) == 0 && system(cmd) == 0 )
							unscaled_image = gdk_pixbuf_new_from_file ( destfile ) ;
						if ( unscaled_image )
							printd ( "iv: mp: thumbnail loaded...\n" ) ;
						else
							printd ( "iv: mp: recreation failed!\n" ) ;
					}
					
					if ( unscaled_image )
					{
						width = gdk_pixbuf_get_width ( unscaled_image ) ;
						height = gdk_pixbuf_get_height ( unscaled_image ) ;
		
						scale = ( (double) max_thumb_size ) /
								( (double) MAX ( width, height ) ) ;
						
						scaled_width = ( (double) width ) * scale ;
						scaled_height = ( (double) height ) * scale ;
						
						//printf ( "iv: mp: width = %d ; height = %d ; scale = %f\n", width, height, scale ) ;
						//printf ( "iv: mp: scaled_width = %d ; scaled_height = %d \n", scaled_width, scaled_height ) ;
		
						scaled_image = gdk_pixbuf_scale_simple ( unscaled_image, 
																 scaled_width, 
																 scaled_height,
																 thumb_render_quality ) ;
						
						if ( scaled_image )
						{
							pixbuf_to_file_as_png ( scaled_image, mini_file ) ;
							gdk_pixbuf_unref ( scaled_image ) ;
						}
						
						gdk_pixbuf_unref ( unscaled_image ) ;
					}
				}

				gdk_threads_enter () ;
				
				if ( count == 2 )
				{	// more than one page viewable - display sidebar
					if ( ! GTK_WIDGET_VISIBLE ( GTK_WIDGET ( progress ) ) )
						gtk_widget_show ( GTK_WIDGET ( progress ) ) ;
					gtk_widget_set_usize ( GTK_WIDGET(lookup_widget(MainWindow,"multipagebox")), 
											max_thumb_size + (max_thumb_size*0.20), -1 ) ;
					if ( !GTK_WIDGET_VISIBLE ( lookup_widget ( MainWindow, "multipagebox" ) ) )
						gtk_widget_show ( lookup_widget ( MainWindow, "multipagebox" ) ) ;
					if ( !GTK_WIDGET_VISIBLE ( lookup_widget ( MainWindow, "progressbarmp" ) ) )
						gtk_widget_show ( lookup_widget ( MainWindow, "progressbarmp" ) ) ;
				}
				
				sprintf ( page_name, "page %d", count ) ;
				
				printd("iv: mp: append page number "); 
				printd(text_from_var(count)); printd(" from "); printd(destfile) ;
				printd("...\n");
				
				if ( image_mutlipage_fast_preview ) gnome_icon_list_remove ( iconlist, count - 1 ) ;
				
				if ( stat ( mini_file, &buf2 ) == 0 )
				{
					gnome_icon_list_insert ( iconlist, count - 1, mini_file, page_name ) ;
				} else {
					gnome_icon_list_insert_imlib ( iconlist, count - 1, gdk_imlib_create_image_from_xpm_data( (char**) loading_xpm ), page_name ) ;
				}

				gtk_progress_configure ( progress, gtk_progress_get_value(progress), 1, count ) ;

				gdk_threads_leave () ;
				
				count++ ;
			} else {
				printd("iv: mp: convert failed. image not loaded.\n");
				finish = TRUE ;
				count-- ;
			}
		} else {
			finish = TRUE ;
			count-- ;
		}
		unlink ( to_file ) ;
	} // while

	if ( count < 2 || task_already_exists_in_queue ( "quit", NULL ) == TRUE ||
			task_already_exists_in_queue ( "stop_page_list", NULL ) == TRUE )
	{	// if there's only one page to show - hide sidebar 
		gdk_threads_enter () ;
		if ( GTK_WIDGET_VISIBLE ( GTK_WIDGET ( progress ) ) )
			gtk_widget_hide ( GTK_WIDGET ( progress ) ) ;
		if ( GTK_WIDGET_VISIBLE ( lookup_widget ( MainWindow, "multipagebox" ) ) )
			gtk_widget_hide ( lookup_widget ( MainWindow, "multipagebox" ) ) ;
		gdk_threads_leave () ;
	}
	if ( loadpic ) gdk_imlib_destroy_image ( loadpic ) ;
	
	task_add_to_queue ( "stop_progressbarmp", NULL ) ;
	
	task_remove_from_queue ( own_task ) ;
	thread_count-- ;

}


void
mp_fill_sidebar ( char *filename )
{
	pthread_t pagelist_thread ;
	int own_task = task_add_to_queue ( "stop_page_list", NULL ) ;
	
	//printf("iv: mp: initialize sidebar_thread with filename = %s\n", filename ) ;
	
	while ( task_already_exists_in_queue ( "create_page_list", NULL ) )
	{
		usleep ( 1000 ) ;
		gdk_threads_enter () ;
		gdk_threads_leave () ;
		//printd ( "iv: mp: waiting for other page-list threads to shutdown...\n" ) ;
	}
	task_remove_from_queue ( own_task ) ;
	
	pthread_create ( &pagelist_thread, NULL, (void*)&mp_fill_sidebar_thread, filename ) ;
	pthread_detach ( pagelist_thread ) ;
}


int
check_for_new_image_selection ( char *filename )
{
	GList *sList;
	GtkCList *clist ;
	int rNum;
	
	clist = GTK_CLIST(lookup_widget(MainWindow,"imagelist")) ;
	
	sList = clist->selection;
	if ( sList && filename != NULL && filename != "__RELOAD__" ) 
	{
		rNum = (int) sList->data ;
		return rNum ;
	} else {
		return FALSE ;
	}
}


void
load_image ( char *filename )
{
	GdkPixbuf *new_image = NULL ;
	char dimension[2048] ;
	char filetype[2048] ;
	GtkCList *imagestatsclist;
	gchar *imagestatsclist_entry[1];
	struct stat buf ;
	GtkWidget *appbar2 ;
	//GtkProgress *progress ;
	int density = gtk_spin_button_get_value_as_int( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton9" ) ) ) ;
	char temp_filename[2048] ;
	char multifile[2048] = { "" } ;
	int force_pdf_load = FALSE ; // force pdf loading before postscript?
	
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == TRUE )
	{
		appbar2 = lookup_widget ( FullscreenWindowProgressbar, "appbar2");
		if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) == FALSE ||
				gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) )
			gtk_widget_show ( FullscreenWindowProgressbar ) ;
	} else
		appbar2 = lookup_widget ( MainWindow, "appbar2");
	//progress = gnome_appbar_get_progress (GNOME_APPBAR (appbar2));

	//update_screen () ;
	{
		char text[2048] ;
		//gtk_widget_set_redraw_on_allocate ( FullscreenWindow, FALSE ) ; // API 2.0
		//gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
		sprintf ( text, "Trying to load %s...", filename ) ;
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _(text));
	}

	{
		GtkProgress *progress = GTK_PROGRESS(GTK_PROGRESS_BAR(lookup_widget(MainWindow,"imageprogressbar"))) ;
		if ( GTK_WIDGET_VISIBLE ( GTK_WIDGET ( progress ) ) )
			gtk_widget_hide ( GTK_WIDGET ( progress ) ) ;
	}

	gdk_threads_leave () ;

	start_image_progressbar () ;
	
	strcpy ( temp_filename, filename ) ;
	if ( !strcmp( gnome_mime_type(temp_filename), "application/postscript" ) )
	{
		char cmd[2048], destfile[2048] ;
		int pos = 0 ;
		struct stat buf2 ;
		
		getcwd ( destfile, 2048 ) ;
		strcat ( destfile, "/" ) ;
		strcat ( destfile, temp_filename ) ;
		for ( pos = 0 ; pos < strlen(destfile) ; pos++ )
		{
			//printd( text_from_var(  strlen(index(destfile,47))  ) ); printd(" ");
			if ( destfile[pos] == 47 ) destfile[pos] = 183 ;
		}
		strcpy ( temp_filename, destfile ) ;
		temp_filename[strlen(temp_filename)] = '\0' ;
		
		printd("iv: converted temporary filename = "); printd(temp_filename); printd("\n");
		
		strcpy ( destfile, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
		strcat ( destfile, LATS_RC_TEMP_DIR ) ;
		strcat ( destfile, temp_filename ) ;

		//sprintf ( cmd, "ps2pdfwr '%s' '%s'", filename, temp_filename ) ;
		sprintf ( cmd, "gs -dSAFER -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile='%s' -dSAFER -c .setpdfwrite -f '%s'", destfile, filename ) ;
		printd ( cmd ) ; printd ( "\n" ) ;
		if ( stat ( destfile, &buf2 ) != 0 && system(cmd) != 0 )
			printd("iv: convert from postscript to pdf failed...\n");
		else
			force_pdf_load = TRUE ;
		strcpy ( temp_filename, destfile ) ;
	}

	if ( !strcmp( gnome_mime_type(temp_filename), "application/pdf" ) || force_pdf_load ) {
		char cmd[2048] ;
		int acroread_available = FALSE ;
		int count = 1, finish = FALSE ;
		struct stat buf2 ;
		
		printd("iv: load a pdf-document...\n");
		printd("iv: check for Adobe Acrobat Reader ( acroread ) available in path... ");
		if ( system("which acroread &> /dev/null") == 0 )
		{
			printd("found!\n");
			acroread_available = TRUE ;
		} else {
			printd("not found!\n");
			acroread_available = FALSE ;
		}
		if ( acroread_available ) 
		{
			char to_file[2048], *dir ;
			char destfile[2048] ;

			printd("iv: using acroread for converting pdf-document to postscript...\n");
	
			dir = gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ;
			{
				char name[2048] ;
				int pos ;
				getcwd ( name, 2048 ) ;
				strcat ( name, "/" ) ;
				strcat ( name, temp_filename ) ;
				for ( pos = 0 ; pos < strlen(name) ; pos++ )
				{
					//printd( text_from_var(  strlen(index(name,47))  ) ); printd(" ");
					if ( name[pos] == 47 ) name[pos] = 183 ;
				}
				
				if ( !rindex ( temp_filename, 47 ) )
					sprintf ( to_file, "%s%s%s.%dDPI.%03d.ps", dir, LATS_RC_TEMP_DIR, name, density, count ) ;
				else
					sprintf ( to_file, "%s.%dDPI.%03d.ps", temp_filename, density, count ) ;
			}
			sprintf ( cmd, "acroread -toPostScript -binary -fast -transQuality 5 -start %d -end %d -pairs '%s' '%s'", count, count, temp_filename, to_file ) ;
			printd( "iv: " ) ; printd ( cmd ) ; printd ( "\n" ) ;
	
			strcpy ( destfile, to_file ) ;
			strcat ( destfile, ".temp" ) ;
			
			if ( stat ( to_file, &buf2 ) == 0 ) unlink ( to_file ) ;
			if ( stat ( destfile, &buf2 ) == 0 || 
					( stat ( to_file, &buf2 ) != 0 && system(cmd) == 0 && 
					  stat ( to_file, &buf2 ) == 0 && buf2.st_size > 0 ) )
			{
				if ( stat ( destfile, &buf2 ) != 0 )
				{
					printd("iv: now convert it to an image...\n");
					strcpy ( cmd, "gs -sDEVICE=jpeg -r" ) ;
					strcat ( cmd, text_from_var ( density ) ) ;
					strcat ( cmd, " -q -dSAFER -dNOPAUSE -sOutputFile='" ) ;
					strcat ( cmd, destfile ) ;
					strcat ( cmd, "' -- '" ) ;
					strcat ( cmd, to_file ) ;
					strcat ( cmd, "'" ) ;
				} else strcpy ( cmd, "" ) ;
				
				printd("iv: "); printd(cmd); printd("\n");
				
				if ( (strcmp(cmd,"") && system(cmd) == 0) || !strcmp(cmd,"") )
				{
					strcpy ( multifile, temp_filename ) ;
					printd("iv: now load first image...\n");
					strcpy ( cmd, "" ) ;
					if ( !rindex ( temp_filename, 47 ) )
					{
						char name[2048] ;
						int pos ;
						strcat ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
						strcat ( cmd, LATS_RC_TEMP_DIR ) ;
						
						getcwd ( name, 2048 ) ;
						strcat ( name, "/" ) ;
						for ( pos = 0 ; pos < strlen(name) ; pos++ )
						{
							if ( name[pos] == 47 ) name[pos] = 183 ;
						}
						
						strcat ( cmd, name ) ;
					}
					strcat ( cmd, temp_filename ) ;
					sprintf( cmd, "%s.%dDPI.001.ps.temp", cmd, density ) ;
					printd("iv: ");printd(cmd);printd("\n");
					new_image = gdk_pixbuf_new_from_file ( cmd );
					if ( ! new_image )
					{
						gdk_threads_enter();
						gtk_widget_hide ( lookup_widget ( MainWindow, "multipagebox" ) ) ;
						gdk_threads_leave();
					}
					//gtk_widget_set_sensitive ( lookup_widget ( MainWindow, "realzoom" ), TRUE ) ;
					count++ ;
				} else {
					printd("iv: convert failed. image not loaded.\n");
					finish = TRUE ;
					count-- ;
				}
			} else {
				finish = TRUE ;
				count-- ;
			}
			unlink ( to_file ) ;
		}
		if ( acroread_available == FALSE ) {
			char destfile[2048];
			char name[2048] ;
			int pos ;
			getcwd ( name, 2048 ) ;
			strcat ( name, "/" ) ;
			strcat ( name, temp_filename ) ;
			for ( pos = 0 ; pos < strlen(name) ; pos++ )
			{
				if ( name[pos] == 47 ) name[pos] = 183 ;
			}
			strcpy ( destfile, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
			strcat ( destfile, LATS_RC_TEMP_DIR ) ;
			strcat ( destfile, name ) ;
			sprintf( destfile, "%s.%dDPI.temp", destfile, density ) ;
			printd("iv: using convert for converting pdf-document to an image...\n");
			printd("iv: now convert it to an image...\n");
			strcpy ( cmd, "convert -density " ) ;
			strcat ( cmd, text_from_var ( density ) ) ;
			strcat ( cmd, " '" ) ;
			strcat ( cmd, filename ) ;
			strcat ( cmd, "' 'PNG:" ) ;
			strcat ( cmd, destfile ) ;
			strcat ( cmd, "'" ) ;
			if ( stat(destfile, &buf2) == 0 || system(cmd) == 0 )
			{
				printd("iv: now load this image...\n");
				gdk_threads_enter();
				if ( GTK_WIDGET_VISIBLE ( lookup_widget ( MainWindow, "multipagebox" ) ) )
					gtk_widget_hide ( lookup_widget ( MainWindow, "multipagebox" ) ) ;
				gdk_threads_leave();
				new_image = gdk_pixbuf_new_from_file ( destfile );
				//gtk_widget_set_sensitive ( lookup_widget ( MainWindow, "realzoom" ), TRUE ) ;
			} else
				printd("iv: convert failed. image not loaded.\n");
		}
		printd("iv: now delete the temp-file...\n");
		strcpy ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
		strcat ( cmd, LATS_RC_TEMP_DIR ) ;
		strcat ( cmd, temp_filename ) ;
		sprintf( cmd, "%s.%dDPI.ps", cmd, density ) ;
		unlink ( cmd ) ;
	} else if ( !strcmp( gnome_mime_type(filename), "application/postscript" ) ) {
		char cmd[2048], destfile[2048], uncompdestfile[2048] ;
		struct stat buf2 ;
		// first delete our temporary file...
		unlink ( temp_filename ) ;
		strcpy ( destfile, "" ) ;
		//strcpy ( destfile, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
		//strcat ( destfile, LATS_RC_TEMP_DIR ) ;
		strcat ( destfile, temp_filename ) ;
        sprintf( destfile, "%s.%dDPI.temp", destfile, density ) ;
		sprintf( uncompdestfile, "%s.uncomp", destfile ) ;
		printd("iv: load a postscript-document...\n");
		strcpy ( cmd, "gs -sDEVICE=jpeg -r" ) ;
		strcat ( cmd, text_from_var ( density ) ) ;
		strcat ( cmd, " -q -dSAFER -dNOPAUSE -sOutputFile='" ) ;
		strcat ( cmd, uncompdestfile ) ;
		strcat ( cmd, "' -- '" ) ;
		strcat ( cmd, filename ) ;
		strcat ( cmd, "'" ) ;
		printd("iv: "); printd(cmd); printd("\n");
		if ( stat ( destfile, &buf2 ) == 0 || system(cmd) >= 0 )
		{
			printd("iv: recompress temp file...\n");
			sprintf ( cmd, "jpegtran -optimize -progressive -outfile '%s' '%s.uncomp' &> /dev/null", destfile, destfile ) ;
			if ( stat ( uncompdestfile, &buf2 ) == 0 && system(cmd) < 0 )
			{
				sprintf( cmd, "%s.uncomp", destfile ) ;
				unlink ( destfile ) ;
				rename ( cmd, destfile ) ;
			} else {
				sprintf( cmd, "%s.uncomp", destfile ) ;
				unlink ( cmd ) ;
			}
			gdk_threads_enter();
			if ( GTK_WIDGET_VISIBLE ( lookup_widget ( MainWindow, "multipagebox" ) ) )
				gtk_widget_hide ( lookup_widget ( MainWindow, "multipagebox" ) ) ;
			gdk_threads_leave();
			printd("now load this image...\n");
			new_image = gdk_pixbuf_new_from_file ( destfile );
			//gtk_widget_set_sensitive ( lookup_widget ( MainWindow, "realzoom" ), TRUE ) ;
		}
	} else {
		gdk_threads_enter();
		//gtk_widget_set_sensitive ( lookup_widget ( MainWindow, "realzoom" ), FALSE ) ;
		if ( GTK_WIDGET_VISIBLE ( lookup_widget ( MainWindow, "multipagebox" ) ) )
			gtk_widget_hide ( lookup_widget ( MainWindow, "multipagebox" ) ) ;
		gdk_threads_leave();
		printd("iv: load regular image...\n");
		new_image = gdk_pixbuf_new_from_file ( filename );
	}
	
	if ( loaded_image && new_image )
	{
		char file_name[2048] ;
		//gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("image loaded."));
		printd ("iv: free old image in memory\n");
		gdk_pixbuf_unref ( loaded_image ) ;
		printd ("iv: copy image "); printd (filename); printd (" into memory...\n");
		loaded_image = gdk_pixbuf_copy ( new_image ) ;
		printd ("iv: free temporary image in memory...\n");
		gdk_pixbuf_unref ( new_image ) ;
		
		printd ("iv: now udpate the status display...\n");
		gdk_threads_enter () ;
		
		sprintf ( file_name, "%s", filename ) ;
		printd("iv: file_name = "); printd(file_name); printd("\n");
		
		printd ("iv: get imagestatsclist widget...\n");
		imagestatsclist = GTK_CLIST(lookup_widget( MainWindow, "imagestatsclist" ));
		printd("iv:  - update iamgestatsclist -\n");
		gtk_clist_clear (imagestatsclist);
		gtk_clist_set_column_visibility ( imagestatsclist, 0, TRUE ); // show fieldnames?
		gtk_clist_set_column_justification ( imagestatsclist, 0, GTK_JUSTIFY_LEFT );
		gtk_clist_set_column_justification ( imagestatsclist, 1, GTK_JUSTIFY_LEFT );
		gtk_clist_set_column_auto_resize ( imagestatsclist, 0, TRUE );
		gtk_clist_set_column_auto_resize ( imagestatsclist, 1, TRUE );
		stat ( filename, &buf );
		printd("iv: imagestatsclist_entry[1] = "); printd(file_name); printd("\n");
		imagestatsclist_entry[0] = "filename" ; imagestatsclist_entry[1] = strdup( file_name ) ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		printd("iv: appended.\n");
		imagestatsclist_entry[0] = "size" ; imagestatsclist_entry[1] = text_from_size(buf.st_size) ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		imagestatsclist_entry[0] = "date" ; imagestatsclist_entry[1] = text_from_time(buf.st_mtime) ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		sprintf ( dimension, "%dx%d", gdk_pixbuf_get_width( loaded_image ),
					gdk_pixbuf_get_height( loaded_image ) ) ;
		imagestatsclist_entry[0] = "dimensions" ; imagestatsclist_entry[1] = dimension ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		imagestatsclist_entry[0] = "transparency?" ; 
		if ( gdk_pixbuf_get_has_alpha( loaded_image ) )
			imagestatsclist_entry[1] = "yes" ;
		else
			imagestatsclist_entry[1] = "no" ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		sprintf ( filetype, "%s", gnome_mime_type(filename) ) ;
		imagestatsclist_entry[0] = "filetype" ; imagestatsclist_entry[1] = filetype ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		imagestatsclist_entry[0] = "colorspace" ; imagestatsclist_entry[1] = text_from_var(gdk_pixbuf_get_colorspace(loaded_image)) ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		imagestatsclist_entry[0] = "n channels" ; imagestatsclist_entry[1] = text_from_var(gdk_pixbuf_get_n_channels(loaded_image)) ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		imagestatsclist_entry[0] = "bits per sample" ; imagestatsclist_entry[1] = text_from_var(gdk_pixbuf_get_bits_per_sample(loaded_image)) ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		imagestatsclist_entry[0] = "rowstride" ; imagestatsclist_entry[1] = text_from_var(gdk_pixbuf_get_rowstride(loaded_image)) ;
		gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
		printd("iv: imagestatsclist updated.\n");
		
		gtk_widget_realize ( GTK_WIDGET( imagestatsclist ) ) ;
		
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"frame3"), TRUE );
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"imagestats"), TRUE );
		/*if ( ! oldimage )
			on_vmbutton2_clicked ( NULL, user_data ) ;*/
	} else {
		gdk_threads_enter () ;
	}
	
	gdk_threads_leave () ;
	stop_image_progressbar () ;
	gdk_threads_enter () ;
	gdk_threads_leave () ;
	
	if ( multifile ) 
	{
		int pos, count = 1 ;
		char name[2048], dir[2048], to_file[2048] ;
		struct stat buf2 ;
		
		strcpy ( dir, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
		getcwd ( name, 2048 ) ;
		strcat ( name, "/" ) ;
		strcat ( name, multifile ) ;
		for ( pos = 0 ; pos < strlen(name) ; pos++ )
		{
			if ( name[pos] == 47 ) name[pos] = 183 ;
		}

		if ( !rindex ( multifile, 47 ) )
			sprintf ( to_file, "%s%s%s.%dDPI.%03d.ps.temp", dir, LATS_RC_TEMP_DIR, name, density, count ) ;
		else
			sprintf ( to_file, "%s.%dDPI.%03d.ps.temp", multifile, density, count ) ;
		
		strcpy ( name, to_file ) ;
		
		printd("iv: destfile for multipage = ") ; printd(name) ; printd("\n") ;
		if ( stat(name, &buf2) == 0 ) 
			mp_fill_sidebar ( multifile ) ;
	} else { // kill all page-list threads...
		int own_task = task_add_to_queue ( "stop_page_list", NULL ) ;
		
		while ( task_already_exists_in_queue ( "create_page_list", NULL ) )
		{
			usleep ( 1000 ) ;
			gdk_threads_enter () ;
			gdk_threads_leave () ;
			//printd ( "iv: mp: waiting for other page-list threads to shutdown...\n" ) ;
		}
		task_remove_from_queue ( own_task ) ;
	}
	gdk_threads_enter () ;
	
	//update_screen () ;
}

void
view_image ( char *filename, gpointer user_data )
{
	GtkWidget *imagedisplay, *appbar2 ;
	GtkProgress *progress = NULL ;
	GtkCList *imagestatsclist;
	int reload = FALSE ;
	int force_reload = FALSE ;
	int aspect_correction ;
	int gnome_appbar_has_progress = FALSE ;
	
	usleep ( 1000 ) ;

	gdk_threads_leave () ;
	stop_image_progressbar () ;
	gdk_threads_enter () ;
	
	if ( task_already_exists_in_queue ( "quit", NULL ) ) return ;
	
	aspect_correction = gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "keepaspect" )) ) ;

	if ( GTK_WIDGET_VISIBLE ( user_data ) == FALSE ) return ;
	
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == TRUE )
		appbar2 = lookup_widget ( FullscreenWindowProgressbar, "appbar2");
	else
		appbar2 = lookup_widget ( MainWindow, "appbar2");
	gnome_appbar_has_progress = GNOME_APPBAR_HAS_PROGRESS ( appbar2 ) ;
	if ( gnome_appbar_has_progress )
		progress = gnome_appbar_get_progress (GNOME_APPBAR (appbar2));
	else {
		gnome_appbar_has_progress = TRUE ;
		progress = GTK_PROGRESS(GTK_PROGRESS_BAR(lookup_widget(MainWindow,"imageprogressbar"))) ;
	}
	
	imagedisplay = lookup_widget ( MainWindow, "handlebox1" ) ;

	if ( strcmp ( filename, "__FORCE_RELOAD__" ) == 0 )
	{
		filename = "__RELOAD__" ;
		force_reload = TRUE ;
	} 
	
	if ( strcmp ( filename, "__UPDATE__" ) == 0 )
	{
		filename = "__RELOAD__" ;
	} //else if ( gtk_events_pending () ) return ;
	
	if ( GTK_WIDGET_SENSITIVE ( imagedisplay ) == FALSE && 
		strcmp ( filename, "__RELOAD__" ) == 0 && 
		loaded_image == NULL
		) return ;
	
	gtk_widget_set_sensitive ( imagedisplay, FALSE );
	//gtk_widget_set_sensitive ( lookup_widget ( user_data, "frame1" ), FALSE );
	//gtk_widget_set_sensitive ( lookup_widget(user_data, "scrolledwindow6"), FALSE ) ;

/*	if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) &&
			gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) )
		gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;*/
	
	//update_screen ();

	imagestatsclist = GTK_CLIST(lookup_widget( MainWindow, "imagestatsclist" ));

	if ( strcmp ( filename, "__RELOAD__" ) == 0 )
	{
		//printd("iv:  imagestatsclist get text\n");
		gtk_clist_get_text ( imagestatsclist, 0, 1, &filename );
		//printd("iv: "); printd(filename); printd("\n");
		reload = TRUE ;
	} 

	//printd("iv: Bild "); printd(filename);
	
	//if ( gnome_appbar_has_progress )
	//	gtk_progress_set_value (progress, 0 );

	if ( reload == FALSE || loaded_image == NULL || force_reload == TRUE ) 
	{
		int fold_out = FALSE ; // if we need to switch to splitted view... 
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("load image from file..."));
		update_screen () ;
		if ( GTK_WIDGET_VISIBLE(lookup_widget(MainWindow, "imagedisplay")) == FALSE && loaded_image == NULL )
			fold_out = TRUE ; // so we didn't load an image yet...
		//load_image ( filename );
		if ( loaded_image ) 
		{
			// so it worked? :)
			if ( fold_out == TRUE ) // do we need to switch to splitted view mode before displaying da image?
				on_vmbutton2_clicked ( NULL, MainWindow ) ;
			gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("image loaded... calculate dimensions..."));
			if ( gnome_appbar_has_progress )
				gtk_progress_set_value (progress, 10 );
			update_screen () ;
		}
	} else {
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("load image from cache..."));
		if ( gnome_appbar_has_progress )
			gtk_progress_set_value (progress, 10 );
		update_screen () ;
		printd("iv: image from memory");
	}

	update_screen () ;
	
	if ( loaded_image && task_already_exists_in_queue ( "quit", NULL ) == FALSE )
	{
		long double imagewidth, imageheight, scale, real_scale, scale_x = 1, scale_y = 1 ;
		int canvaswidth = 0 , canvasheight = 0, imagex = 0, imagey = 0 ;
		GtkAdjustment *hadjust, *vadjust ;
		GtkWidget *gtkpixmap ;
		GdkPixmap *gdkpixmap = NULL ;
		GdkGC *drawing_gc ;
		GdkPixbuf *dest = NULL, *dummy = NULL ;
		guint32 checkcolora = ( check_color_a * 256 * 256 ) + ( check_color_a * 256 ) + check_color_a ;
		guint32 checkcolorb = ( check_color_b * 256 * 256 ) + ( check_color_b * 256 ) + check_color_b ;
		guint32 checkcolorouta = 0 ;
		guint32 checkcoloroutb = 0 ;
		int checksize = check_size ;
		int force_auto_zoom = FALSE ;
		int show_tiled_image = FALSE ;
		int use_real_zooming = FALSE ;
		double density = ( gtk_spin_button_get_value_as_float( 
									  GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton9" ) ) ) / (double) 10 ) ;
		// now let's see how big our display is :)
		double mm_x = gdk_screen_width_mm(), mm_y = gdk_screen_height_mm() ;
		double x = gdk_screen_width(), y = gdk_screen_height() ;
		double dpi_x, dpi_y, scale_dpi_x, scale_dpi_y ;

		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimageradiobutton5" )) ) )
		{
			dpi_x = x / ( mm_x / (double) 2.54 ) ;
			dpi_y = y / ( mm_y / (double) 2.54 ) ;
		} else {
			dpi_x = gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttonx" ) ) ) / (double) 10 ;
			dpi_y = gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttony" ) ) ) / (double) 10 ;
		}
		scale_dpi_x = (double) 1 / density * dpi_x ;
		scale_dpi_y = (double) 1 / density * dpi_y ;
		
		printd(" loaded\n");
		
		gdk_threads_leave () ;
		start_image_progressbar () ;
		gdk_threads_enter () ;

		printd("iv: your displays' resolution is ");
		printd(text_from_var( (int) ( dpi_x * 10 ) ));
		printd("x");
		printd(text_from_var( (int) ( dpi_y * 10 ) ));
		printd(" DPI and ");
		printd(text_from_var((int)mm_x));
		printd("x");
		printd(text_from_var((int)mm_y));
		printd(" mm. \n");
		printd("iv: scale_dpi_x = "); printd(text_from_var( (int)(scale_dpi_x*(double)100) )) ; printd(" ; ");
		printd("scale_dpi_y = "); printd(text_from_var( (int)(scale_dpi_y*(double)100) )) ; printd("\n");
		// get actual settings
		check_color_a = get_check_color_a () ;
		check_color_b = get_check_color_b () ;
		check_color_out_a = get_check_color_out_a () ;
		check_color_out_b = get_check_color_out_b () ;
		check_color_auto = get_check_color_auto () ;
		check_color_out_use = get_check_color_out_use () ;
		check_color_out_cut_off_factor = get_check_color_out_cut_off_factor () ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgradiobutton1" )) ) )
			check_color_auto_routine = 1 ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgradiobutton2" )) ) )
			check_color_auto_routine = 2 ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgradiobutton3" )) ) )
			check_color_auto_routine = 3 ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgradiobutton4" )) ) )
			check_color_auto_routine = 4 ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "bgtiles" )) ) )
			show_tiled_image = TRUE ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "realzoom" )) ) )
			use_real_zooming = TRUE ;

		/*if ( reload == FALSE )
		{
			char dimension[2048] ;
			struct stat buf ;
			gchar *imagestatsclist_entry[1] ;
			printd("iv:  - iamgestatsclist updaten -");
			gtk_clist_clear (imagestatsclist);
			gtk_clist_set_column_visibility ( imagestatsclist, 0, TRUE ); // show fieldnames?
			gtk_clist_set_column_justification ( imagestatsclist, 0, GTK_JUSTIFY_LEFT );
			gtk_clist_set_column_justification ( imagestatsclist, 1, GTK_JUSTIFY_LEFT );
			gtk_clist_set_column_auto_resize ( imagestatsclist, 0, TRUE );
			gtk_clist_set_column_auto_resize ( imagestatsclist, 1, TRUE );
			stat ( filename, &buf );
			imagestatsclist_entry[0] = "filename" ; imagestatsclist_entry[1] = filename ;
			gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
			imagestatsclist_entry[0] = "size" ; imagestatsclist_entry[1] = text_from_size(buf.st_size) ;
			gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
			imagestatsclist_entry[0] = "date" ; imagestatsclist_entry[1] = text_from_time(buf.st_mtime) ;
			gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
			sprintf ( dimension, "%dx%d", gdk_pixbuf_get_width( loaded_image ),
						gdk_pixbuf_get_height( loaded_image ) ) ;
			imagestatsclist_entry[0] = "dimensions" ; imagestatsclist_entry[1] = dimension ;
			gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
			imagestatsclist_entry[0] = "transparency?" ; 
			if ( gdk_pixbuf_get_has_alpha( loaded_image ) )
				imagestatsclist_entry[1] = "yes" ;
			else
				imagestatsclist_entry[1] = "no" ;
			gtk_clist_append ( imagestatsclist, imagestatsclist_entry );
			gtk_widget_set_sensitive ( lookup_widget(MainWindow,"frame3"), TRUE );
			gtk_widget_set_sensitive ( lookup_widget(MainWindow,"imagestats"), TRUE );
			//if ( ! oldimage )
				on_vmbutton2_clicked ( NULL, user_data ) ;//
		}*/

		imagewidth = ( gdk_pixbuf_get_width ( loaded_image ) ) ;//* scale ;
		imageheight = ( gdk_pixbuf_get_height ( loaded_image ) ) ;//* scale ;

		gtkpixmap = lookup_widget ( user_data, "image") ;
		//gtk_pixmap_set ( GTK_PIXMAP(gtkpixmap) , gdk_pixmap_new ( lookup_widget( user_data, "MainWindow" )->window, 1, 1, -1 ), NULL ) ;
		// Now we need the size of the view-area
		/*hadjust = gtk_scrolled_window_get_hadjustment ( GTK_SCROLLED_WINDOW( lookup_widget ( user_data, "scrolledwindow6" ) ) ) ;
		vadjust = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW( lookup_widget ( user_data, "scrolledwindow6" ) ) ) ;
		canvaswidth = (int) hadjust->page_size ; canvasheight = (int) vadjust->page_size ;
		printd("iv: change widget size\n");
		gtk_widget_set_usize ( gtkpixmap, canvaswidth, canvasheight );
		update_screen () ;*/
		//gtk_widget_realize ( lookup_widget ( user_data, "image" ) ) ;
		hadjust = gtk_scrolled_window_get_hadjustment ( GTK_SCROLLED_WINDOW( lookup_widget ( user_data, "scrolledwindow6" ) ) ) ;
		vadjust = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW( lookup_widget ( user_data, "scrolledwindow6" ) ) ) ;
		canvaswidth = (int) hadjust->page_size ; canvasheight = (int) vadjust->page_size ;

		//canvaswidth = lookup_widget( user_data, "scrolledwindow6" )->allocation.width ;
		//canvasheight = lookup_widget( user_data, "scrolledwindow6" )->allocation.height ;

		/*if ( gtk_toggle_button_get_active ( 
									GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) &&
			GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
		{	
			if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) )
				gtk_widget_set_usize ( lookup_widget ( FullscreenWindow, "image" ), canvaswidth, canvasheight ) ;
			//canvaswidth = lookup_widget ( FullscreenWindow, "fullscreenbox" )->allocation.width ;
			//canvasheight = lookup_widget ( FullscreenWindow, "fullscreenbox" )->allocation.height ;
			canvaswidth = gdk_screen_width () ;
			canvasheight = gdk_screen_height () ;
		}*/
		
		/*if ( lookup_widget( user_data, "scrolledwindow6" )->allocation.width > canvaswidth &&
				gtkpixmap->allocation.width < lookup_widget( user_data, "scrolledwindow6" )->allocation.width )
			canvaswidth = lookup_widget( user_data, "scrolledwindow6" )->allocation.width ;
		if ( lookup_widget( user_data, "scrolledwindow6" )->allocation.height > canvasheight &&
				gtkpixmap->allocation.height < lookup_widget( user_data, "scrolledwindow6" )->allocation.height )
			canvasheight = lookup_widget( user_data, "scrolledwindow6" )->allocation.height ;
		*/
		
		{
			int scaling = gtk_spin_button_get_value_as_int( 
								GTK_SPIN_BUTTON( lookup_widget( MainWindow, "zoomentry" ) ) ) / 100 ;
			int width = imagewidth * scaling ;
			int height = imageheight * scaling ;
			
			if ( width > 4000 || height > 4000 )
				force_auto_zoom = TRUE ;
		}
		
		if ( ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoomwidth" )) ) &&
				gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoomheight" )) ) )
				|| force_auto_zoom == TRUE )
		{
			//gtk_widget_set_sensitive ( lookup_widget(user_data,"zoombox2"), FALSE );
			if ( canvaswidth > imagewidth )
			{
				printd ( "iv: scale1 canvaswidth > imagewidth\n" ) ;
				scale = (double) canvaswidth / (double) imagewidth ;
			} else {
				printd ( "iv: scale2 canvaswidth <= imagewidth\n" ) ;
				scale = (double) canvasheight / (double) imageheight ;
			}
			if ( imageheight * scale > canvasheight )
			{
				printd ( "iv: scale3 imageheight*scale > canvasheight\n" ) ;
				scale = (double) canvasheight / (double) imageheight ;
			}
			if ( imagewidth * scale > canvaswidth )
			{
				printd ( "iv: scale4 imagewidth*scale > canvaswidth\n" ) ;
				scale = ((double) MAX (canvaswidth, canvasheight)) / 
							((double) MAX (imagewidth, imageheight));
				if ( imagewidth * scale > canvaswidth )
				{
					printd ( "iv: scale4.1 imagewidth*scale > canvaswidth\n" ) ;
					scale = (double) canvaswidth / (double) imagewidth  ;
				} else if ( imagewidth * scale == canvaswidth )
					printd ( "iv: scale4.2 imagewidth*scale < canvaswidth\n" ) ;
				else if ( imagewidth * scale < canvaswidth )
				{
					printd ( "iv: scale4.3 imagewdith*scale < canvaswidth \niv: image width greater than canvas width *FIXME* \n" ) ;
					scale = (double) canvaswidth / (double) imagewidth ;
				}
			}
			/*if ( imageheight * scale > canvasheight )
			{
				printd ( "iv: scale5\n" ) ;
				scale = (double) canvasheight / (double) imageheight ;
			}*/
			
			// we don't want any autozoom above this factor...
			if ( scale > 10 ) scale = 10 ;
				
			gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( MainWindow, "zoomentry" ) ), ( (int) ( scale*100) ) );
		} else if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoomwidth" )) ) ) {
			scale = (double) canvaswidth / (double) imagewidth ;
		} else if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoomheight" )) ) ) {
			scale = (double) canvasheight / (double) imageheight ;
		} else {
			//gtk_widget_set_sensitive ( lookup_widget(user_data,"zoombox2"), TRUE );
			scale = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( MainWindow, "zoomentry" ) ) );
			scale = scale / 100 ;
			//imagewidth = imagewidth ; imageheight = imageheight ;
		}
		
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "keepaspect" )) ) == FALSE &&
			 ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoomwidth" )) ) == TRUE &&
			   gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoomheight" )) ) == TRUE ) )
		{
			double factor = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton8" ) ) ) / ( (double) 100 ) ;
			scale_x = scale ; scale_y = scale ;
			if ( imagewidth * scale < canvaswidth ) scale_x = (double) canvaswidth / (double) imagewidth ;
			if ( imageheight * scale < canvasheight ) scale_y = (double) canvasheight / (double) imageheight ;
			
			if ( canvaswidth-(imagewidth*scale) > (double)canvaswidth*factor ) scale_x = scale ;
			if ( canvasheight-(imageheight*scale) > (double)canvasheight*factor ) scale_y = scale ;
		} else {
			scale_x = scale ; scale_y = scale ;
		}

		if ( scale_x == scale && scale_y == scale )
			aspect_correction = TRUE ;
		else
			aspect_correction = FALSE ;
		
		real_scale = scale ;
		//scale = ( (double) (  (int) ( scale * ( (double) 100 ) )  ) ) / (  (double) 100  ) ; 
		//scale_x = ( (double) (  (int) ( scale_x * ( (double) 100 ) )  ) ) / (  (double) 100  ) ; 
		//scale_y = ( (double) (  (int) ( scale_y * ( (double) 100 ) )  ) ) / (  (double) 100  ) ; 
		printd("iv: scale = "); printd(text_from_var(scale*100)); printd("\n");
		printd("iv: scale_x = "); printd(text_from_var(scale_x*100)); printd("\n");
		printd("iv: scale_y = "); printd(text_from_var(scale_y*100)); printd("\n");
		if ( use_real_zooming )
		{
			printd("iv: you want monitor dependent scaling...\n");
			printd("iv: scale after correction:\n");
			scale_x = scale_x * scale_dpi_x ;
			scale_y = scale_y * scale_dpi_y ;
		} else
			printd("iv: you want monitor independent scaling...\n");
		printd("iv: scale_x = "); printd(text_from_var(scale_x*100)); printd("\n");
		printd("iv: scale_y = "); printd(text_from_var(scale_y*100)); printd("\n");

		//printf("iv: scale_dpi_x = %f ; scale_dpi_y = %f\n", (double) scale_dpi_x, (double) scale_dpi_y );
		//printf("iv: scale_x = %f ; scale_y = %f\n", (double) scale_x, (double) scale_y );
		
		imagewidth = imagewidth * scale_x ; imageheight = imageheight * scale_y ;
		imagewidth = (int) imagewidth ; imageheight = (int) imageheight ;
		//printf("iv: %f x %f\n", imagewidth, imageheight );
		
		// our prog will crash if we've got these set below 1 !
		if ( imagewidth < 1 ) imagewidth = 1 ;
		if ( imageheight < 1 ) imageheight = 1 ;
		if ( canvaswidth < 1 ) canvaswidth = 1 ;
		if ( canvasheight < 1 ) canvasheight = 1 ;
		
		if ( scale == 0 ) 
		{
			printd("iv: scale = 0 - no image.\n");
			if ( gdkpixmap ) gdk_pixmap_unref (gdkpixmap) ;
			if ( dest ) gdk_pixbuf_unref ( dest ) ;
			gdk_threads_leave () ;
			stop_image_progressbar () ;
			gdk_threads_enter () ;
			return ;
		}
		
		if ( imagewidth > canvaswidth) canvaswidth = imagewidth ;
		if ( imageheight > canvasheight ) canvasheight = imageheight ;

		printd("iv: canvaswidth = "); printd(text_from_var(canvaswidth));
		printd(" ; canvasheight = "); printd(text_from_var(canvasheight)); printd(" \n");
				
		printd("iv: imagewidth = "); printd(text_from_var(imagewidth));
		printd(" ; imageheight = "); printd(text_from_var(imageheight)); printd(" \n");
		
		if ( imagewidth < canvaswidth )
			imagex = ( canvaswidth - imagewidth ) / 2 ;
		else
			imagex = 0 ;
		if ( imageheight < canvasheight )
			imagey = ( canvasheight - imageheight ) / 2 ;
		else
			imagey = 0 ;
		
		printd("iv: imagex = "); printd(text_from_var(imagex));
		printd(" ; imagey = "); printd(text_from_var(imagey)); printd(" \n");
		
		printd("iv: use background coloration on alpha-channel? ");
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )) ) == TRUE ) // background coloration
		{
			guint8 r, g, b, a ;
			
			printd("YES! lets see...\n");
			{
				gdouble color[4] ;
				printd("iv: get color from colorselectiondialog...\n");
				gtk_color_selection_get_color	( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), color);
				
				printd ("iv: show color...\n");
				printd ( "iv: r = " ) ; printd ( text_from_var ( color[0] * 100 ) ) ;
				printd ( ", g = " ) ; printd ( text_from_var ( color[1] * 100 ) ) ;
				printd ( ", b = " ) ; printd ( text_from_var ( color[2] * 100 ) ) ;
				printd ( ", trans = " ) ; printd ( text_from_var ( color[3] * 100 ) ) ;
				printd ( "\n" ) ;

				r = color[0] * 255 ; g = color[1] * 255 ; b = color[2] * 255 ; a = color[3] * 255 ;
			}			
			// get every first color
			checkcolora = scale_color ( r, g, b, a, check_color_a, 0 ) ;
			
			printd("iv: "); printd (text_from_var(r)); printd(" "); printd (text_from_var(g)); printd(" "); printd (text_from_var(b)); printd(" "); printd (text_from_var(a)); printd(" \n"); 
			printd("iv: "); printd (text_from_var(checkcolora)); printd ("\n");
			
			// get every second color
			checkcolorb = scale_color ( r, g, b, a, check_color_b, 0 ) ;
			
			printd("iv: "); printd (text_from_var(r)); printd(" "); printd (text_from_var(g)); printd(" "); printd (text_from_var(b)); printd(" "); printd (text_from_var(a)); printd(" \n"); 
			printd("iv: "); printd (text_from_var(checkcolorb)); printd ("\n");
		} else { // no background coloration - use automatic color-detection?
			if ( check_color_auto == TRUE )
			{
				guint32 value_r = 0, value_g = 0, value_b = 0, prevalue[3] ;
				int width = gdk_pixbuf_get_width ( loaded_image ), height = gdk_pixbuf_get_height ( loaded_image ) ;
				int range = 16 ;
				
				if ( width < 2 ) width = 1 ;
				if ( height < 2 ) height = 1 ;
					
				printd("NO! but we try to get those from the picture...\n");
				
				printd("iv: gathering pixel colors...\n");
								
				prevalue[0] = get_pixel_from_loaded_image ( 1, 1 ) ;
				prevalue[1] = get_pixel_from_loaded_image ( width - 1, 1 ) ;
				prevalue[2] = get_pixel_from_loaded_image ( width - 1, height - 1 ) ;
				prevalue[3] = get_pixel_from_loaded_image ( 1, height - 1 ) ;
				
				printd("iv: pixels gathered. now analyse them...\n");
				
				if ( check_color_auto_routine == 1 ) {
					
					value_r = ( get_red_from_rgb_value ( prevalue[0], range ) +
									get_red_from_rgb_value ( prevalue[1], range ) +
									get_red_from_rgb_value ( prevalue[2], range ) +
									get_red_from_rgb_value ( prevalue[3], range ) ) / 4 ;
					//printf ( "iv: main red = %d\n", value_r ) ;
					value_g = ( get_green_from_rgb_value ( prevalue[0], range ) +
									get_green_from_rgb_value ( prevalue[1], range ) +
									get_green_from_rgb_value ( prevalue[2], range ) +
									get_green_from_rgb_value ( prevalue[3], range ) ) / 4 ;
					//printf ( "iv: main green = %d\n", value_g ) ;
					value_b = ( get_blue_from_rgb_value ( prevalue[0], range ) +
									get_blue_from_rgb_value ( prevalue[1], range ) +
									get_blue_from_rgb_value ( prevalue[2], range ) +
									get_blue_from_rgb_value ( prevalue[3], range ) ) / 4 ;
					//printf ( "iv: main blue = %d\n", value_b ) ;
					
					checkcolora = ( value_r * 256 * 256 ) + ( MIN( value_g, value_b ) * 256 ) + MAX( value_b, value_g ) ;
					checkcolorb = checkcolora ;
					
				} else if ( check_color_auto_routine == 2 ) {
					
					value_r = ( MIN ( get_red_from_rgb_value ( prevalue[0], range ),
									get_red_from_rgb_value ( prevalue[1], range ) ) +
									MIN ( get_red_from_rgb_value ( prevalue[2], range ),
									get_red_from_rgb_value ( prevalue[3], range ) ) ) / 2 ;
					//printf ( "iv: main red = %d\n", value_r ) ;
					value_g = ( MIN ( get_green_from_rgb_value ( prevalue[0], range ),
									get_green_from_rgb_value ( prevalue[1], range ) ) +
									MIN ( get_green_from_rgb_value ( prevalue[2], range ),
									get_green_from_rgb_value ( prevalue[3], range ) ) ) / 2 ;
					//printf ( "iv: main green = %d\n", value_g ) ;
					value_b = ( MIN ( get_blue_from_rgb_value ( prevalue[0], range ),
									get_blue_from_rgb_value ( prevalue[1], range ) ) +
									MIN ( get_blue_from_rgb_value ( prevalue[2], range ),
									get_blue_from_rgb_value ( prevalue[3], range ) ) ) / 2 ;
					//printf ( "iv: main blue = %d\n", value_b ) ;
					
					checkcolora = ( value_r * 256 * 256 ) + ( MIN( value_g, value_b ) * 256 ) + MAX( value_b, value_g ) ;
					checkcolorb = checkcolora ;
					
				} else if ( check_color_auto_routine == 3 ) {
					
					guint32 lum_a[3], lum_a_work[3], lum_b[3] ;
					
					// get brightness of all 4 points
					lum_a[0] = lum_a_work[0] = ( get_red_from_rgb_value ( prevalue[0], range ) + get_green_from_rgb_value ( prevalue[0], range ) + get_blue_from_rgb_value ( prevalue[0], range ) ) / 3 ;
					lum_a[1] = lum_a_work[1] = ( get_red_from_rgb_value ( prevalue[1], range ) + get_green_from_rgb_value ( prevalue[1], range ) + get_blue_from_rgb_value ( prevalue[1], range ) ) / 3 ;
					lum_a[2] = lum_a_work[2] = ( get_red_from_rgb_value ( prevalue[2], range ) + get_green_from_rgb_value ( prevalue[2], range ) + get_blue_from_rgb_value ( prevalue[2], range ) ) / 3 ;
					lum_a[3] = lum_a_work[3] = ( get_red_from_rgb_value ( prevalue[3], range ) + get_green_from_rgb_value ( prevalue[3], range ) + get_blue_from_rgb_value ( prevalue[3], range ) ) / 3 ;
					
					// which one is the brightest one?
					lum_b[0] = MAX ( MAX ( lum_a_work[0], lum_a_work[1] ), MAX ( lum_a_work[2], lum_a_work[3] ) ) ;
					
					// which one is the darkest one?
					lum_b[3] = MIN ( MIN ( lum_a_work[0], lum_a_work[1] ), MIN ( lum_a_work[2], lum_a_work[3] ) ) ;
					
					// now we blend out the chosen ones from above
					if ( lum_a_work[0] == lum_b[0] ) 
						lum_a_work[0] = 0 ;
					else if ( lum_a_work[1] == lum_b[0] ) 
						lum_a_work[1] = 0 ;
					else if ( lum_a_work[2] == lum_b[0] ) 
						lum_a_work[2] = 0 ;
					else if ( lum_a_work[3] == lum_b[0] ) 
						lum_a_work[3] = 0 ;
					
					if ( lum_a_work[0] == lum_b[3] ) 
						lum_a_work[0] = 0 ;
					else if ( lum_a_work[1] == lum_b[3] ) 
						lum_a_work[1] = 0 ;
					else if ( lum_a_work[2] == lum_b[3] ) 
						lum_a_work[2] = 0 ;
					else if ( lum_a_work[3] == lum_b[3] ) 
						lum_a_work[3] = 0 ;
					
					// which one is the brightest one?
					lum_b[1] = MAX ( MAX ( lum_a_work[0], lum_a_work[1] ), MAX ( lum_a_work[2], lum_a_work[3] ) ) ;
					
					// blend this one out too!
					if ( lum_a_work[0] == lum_b[1] ) 
						lum_a_work[0] = 0 ;
					else if ( lum_a_work[1] == lum_b[1] ) 
						lum_a_work[1] = 0 ;
					else if ( lum_a_work[2] == lum_b[1] ) 
						lum_a_work[2] = 0 ;
					else if ( lum_a_work[3] == lum_b[1] ) 
						lum_a_work[3] = 0 ;
					
					// now we need the forth one...
					lum_b[2] = MAX ( MAX ( lum_a_work[0], lum_a_work[1] ), MAX ( lum_a_work[2], lum_a_work[3] ) ) ;
					
					// for now lets just choose the darkest one..
					if ( lum_a[0] == lum_b[3] ) {
						value_r = get_red_from_rgb_value ( prevalue[0], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[0], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[0], range ) ;
					} else if ( lum_a[1] == lum_b[3] ) {
						value_r = get_red_from_rgb_value ( prevalue[1], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[1], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[1], range ) ;
					} else if ( lum_a[2] == lum_b[3] ) {
						value_r = get_red_from_rgb_value ( prevalue[2], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[2], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[2], range ) ;
					} else if ( lum_a[3] == lum_b[3] ) {
						value_r = get_red_from_rgb_value ( prevalue[3], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[3], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[3], range ) ;
					}
					// and convert it to a 32bit integer usable as hex -> 0xFFFFFF
					checkcolora = ( value_r * 256 * 256 ) + ( value_g * 256 ) + value_b ;
					
					// for now lets choose the 2nd darkest one..
					if ( lum_a[0] == lum_b[2] ) {
						value_r = get_red_from_rgb_value ( prevalue[0], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[0], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[0], range ) ;
					} else if ( lum_a[1] == lum_b[2] ) {
						value_r = get_red_from_rgb_value ( prevalue[1], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[1], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[1], range ) ;
					} else if ( lum_a[2] == lum_b[2] ) {
						value_r = get_red_from_rgb_value ( prevalue[2], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[2], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[2], range ) ;
					} else if ( lum_a[3] == lum_b[2] ) {
						value_r = get_red_from_rgb_value ( prevalue[3], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[3], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[3], range ) ;
					}
					// and convert it to a 32bit integer usable as hex -> 0xFFFFFF
					checkcolorb = ( value_r * 256 * 256 ) + ( value_g * 256 ) + value_b ;

					if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgcheckbutton1" )) ) == FALSE )
						checkcolorb = checkcolora ;
				} else if ( check_color_auto_routine == 4 ) {
					
					guint32 lum_a[3], lum_a_work[3], lum_b[3], value_r2 = 0, value_g2 = 0, value_b2 = 0 ;
					
					// get brightness of all 4 points
					lum_a[0] = lum_a_work[0] = ( get_red_from_rgb_value ( prevalue[0], range ) + get_green_from_rgb_value ( prevalue[0], range ) + get_blue_from_rgb_value ( prevalue[0], range ) ) / 3 ;
					lum_a[1] = lum_a_work[1] = ( get_red_from_rgb_value ( prevalue[1], range ) + get_green_from_rgb_value ( prevalue[1], range ) + get_blue_from_rgb_value ( prevalue[1], range ) ) / 3 ;
					lum_a[2] = lum_a_work[2] = ( get_red_from_rgb_value ( prevalue[2], range ) + get_green_from_rgb_value ( prevalue[2], range ) + get_blue_from_rgb_value ( prevalue[2], range ) ) / 3 ;
					lum_a[3] = lum_a_work[3] = ( get_red_from_rgb_value ( prevalue[3], range ) + get_green_from_rgb_value ( prevalue[3], range ) + get_blue_from_rgb_value ( prevalue[3], range ) ) / 3 ;
					
					// which one is the brightest one?
					lum_b[0] = MAX ( MAX ( lum_a_work[0], lum_a_work[1] ), MAX ( lum_a_work[2], lum_a_work[3] ) ) ;
					
					// which one is the darkest one?
					lum_b[3] = MIN ( MIN ( lum_a_work[0], lum_a_work[1] ), MIN ( lum_a_work[2], lum_a_work[3] ) ) ;
					
					// now we blend out the chosen ones from above
					if ( lum_a_work[0] == lum_b[0] ) 
						lum_a_work[0] = 0 ;
					else if ( lum_a_work[1] == lum_b[0] ) 
						lum_a_work[1] = 0 ;
					else if ( lum_a_work[2] == lum_b[0] ) 
						lum_a_work[2] = 0 ;
					else if ( lum_a_work[3] == lum_b[0] ) 
						lum_a_work[3] = 0 ;
					
					if ( lum_a_work[0] == lum_b[3] ) 
						lum_a_work[0] = 0 ;
					else if ( lum_a_work[1] == lum_b[3] ) 
						lum_a_work[1] = 0 ;
					else if ( lum_a_work[2] == lum_b[3] ) 
						lum_a_work[2] = 0 ;
					else if ( lum_a_work[3] == lum_b[3] ) 
						lum_a_work[3] = 0 ;
					
					// which one is the brightest one?
					lum_b[1] = MAX ( MAX ( lum_a_work[0], lum_a_work[1] ), MAX ( lum_a_work[2], lum_a_work[3] ) ) ;
					
					// blend this one out too!
					if ( lum_a_work[0] == lum_b[1] ) 
						lum_a_work[0] = 0 ;
					else if ( lum_a_work[1] == lum_b[1] ) 
						lum_a_work[1] = 0 ;
					else if ( lum_a_work[2] == lum_b[1] ) 
						lum_a_work[2] = 0 ;
					else if ( lum_a_work[3] == lum_b[1] ) 
						lum_a_work[3] = 0 ;
					
					// now we need the forth one...
					lum_b[2] = MAX ( MAX ( lum_a_work[0], lum_a_work[1] ), MAX ( lum_a_work[2], lum_a_work[3] ) ) ;
					
					// for now lets just choose the darkest one..
					if ( lum_a[0] == lum_b[3] ) {
						value_r = get_red_from_rgb_value ( prevalue[0], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[0], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[0], range ) ;
					} else if ( lum_a[1] == lum_b[3] ) {
						value_r = get_red_from_rgb_value ( prevalue[1], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[1], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[1], range ) ;
					} else if ( lum_a[2] == lum_b[3] ) {
						value_r = get_red_from_rgb_value ( prevalue[2], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[2], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[2], range ) ;
					} else if ( lum_a[3] == lum_b[3] ) {
						value_r = get_red_from_rgb_value ( prevalue[3], range ) ;
						value_g = get_green_from_rgb_value ( prevalue[3], range ) ;
						value_b = get_blue_from_rgb_value ( prevalue[3], range ) ;
					}
					// and convert it to a 32bit integer usable as hex -> 0xFFFFFF
					//checkcolora = ( value_r * 256 * 256 ) + ( value_g * 256 ) + value_b ;
					
					// for now lets choose the 2nd darkest one..
					if ( lum_a[0] == lum_b[2] ) {
						value_r2 = get_red_from_rgb_value ( prevalue[0], range ) ;
						value_g2 = get_green_from_rgb_value ( prevalue[0], range ) ;
						value_b2 = get_blue_from_rgb_value ( prevalue[0], range ) ;
					} else if ( lum_a[1] == lum_b[2] ) {
						value_r2 = get_red_from_rgb_value ( prevalue[1], range ) ;
						value_g2 = get_green_from_rgb_value ( prevalue[1], range ) ;
						value_b2 = get_blue_from_rgb_value ( prevalue[1], range ) ;
					} else if ( lum_a[2] == lum_b[2] ) {
						value_r2 = get_red_from_rgb_value ( prevalue[2], range ) ;
						value_g2 = get_green_from_rgb_value ( prevalue[2], range ) ;
						value_b2 = get_blue_from_rgb_value ( prevalue[2], range ) ;
					} else if ( lum_a[3] == lum_b[2] ) {
						value_r2 = get_red_from_rgb_value ( prevalue[3], range ) ;
						value_g2 = get_green_from_rgb_value ( prevalue[3], range ) ;
						value_b2 = get_blue_from_rgb_value ( prevalue[3], range ) ;
					}
					// and convert it to a 32bit integer usable as hex -> 0xFFFFFF
					//checkcolorb = ( value_r2 * 256 * 256 ) + ( value_g2 * 256 ) + value_b2 ;
					checkcolorb = ( value_r2 * 256 * 256 ) + ( MIN( value_g2, value_b2 ) * 256 ) + MAX( value_b2, value_g2 ) ;

					// now we combine those 2...
					value_r = MIN ( value_r, value_r2 ) / 2 ;
					//printf ( "iv: main red = %d\n", value_r ) ;
					value_g = MIN ( value_g, value_g2 ) / 2 ;
					//printf ( "iv: main green = %d\n", value_g ) ;
					value_b = MIN ( value_b, value_b2 ) / 2 ;
					//printf ( "iv: main blue = %d\n", value_b ) ;
					
					// and convert it to a 32bit integer usable as hex -> 0xFFFFFF
					//checkcolora = ( value_r * 256 * 256 ) + ( value_g * 256 ) + value_b ;
					checkcolora = ( value_r * 256 * 256 ) + ( MIN( value_g, value_b ) * 256 ) + MAX( value_b, value_g ) ;
					
					
					{	// scan the picture for brightness
						guint32 pixel_brightness, current_pixel, darkest_pixel = 255, brightest_pixel = 0, average_pixel = 0, count = 0, value = 0 ;
						int x = 0, y = 0 ;
						int density = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton6" ) ) ) ;
						// later we scan every 'density'th points for its brightness
						
						//if ( width < 2 || height < 2 ) density = 1 ;
						
						printd("iv: scanning picture for brightness analysis...\n");
						
						for ( y = 1 ; y < height - 1 ; y = y + (height/density)  )
						{
							for ( x = 1 ; x < width - 1 ; x = x + (width/density) )
							{
								current_pixel = get_pixel_from_loaded_image ( x, y ) ;
								
								pixel_brightness = ( get_red_from_rgb_value ( current_pixel, range ) + 
																get_green_from_rgb_value ( current_pixel, range ) + 
																get_blue_from_rgb_value ( current_pixel, range ) ) / 3 ;
								
								if ( pixel_brightness > brightest_pixel ) brightest_pixel = pixel_brightness ;
								if ( pixel_brightness < darkest_pixel ) darkest_pixel = pixel_brightness ;
								
								value = value + pixel_brightness ;
								count++ ;
							}
						}
						
						printd("iv: scanning done. making conclusions...\n");
												
						if ( value > 0 && count > 0 ) 
							value = value / count ;
						else
							value = 127 ;
						printd ( "iv: the overall picture brightness is about " ) ; printd ( text_from_var( value ) ) ; printd ( "\n" ) ;

						// overide automatics
						darkest_pixel = 0 ; brightest_pixel = 255 ;
						printd ( "iv: the brightest pixel is about " ) ; printd ( text_from_var( brightest_pixel ) ) ; printd ( "\n" ) ;
						printd ( "iv: the darkest pixel is about " ) ; printd ( text_from_var( darkest_pixel ) ) ; printd ( "\n" ) ;

						average_pixel = ( brightest_pixel + darkest_pixel ) / 2 ;
						average_pixel = average_pixel * gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton7" ) ) ) ;
						printd ( "iv: the average pixel is about " ) ; printd ( text_from_var( average_pixel ) ) ; printd ( "\n" ) ;
						
						if ( value >= average_pixel ) 
						{
							guint32 color_a, color_b ;
							printd ( "iv: we've got a bright picture!\n");
							
							color_a = ( get_red_from_rgb_value ( checkcolora, 256 ) + 
												get_green_from_rgb_value ( checkcolora, 256 ) + 
												get_blue_from_rgb_value ( checkcolora, 256 ) ) / 3 ;
							color_b = ( get_red_from_rgb_value ( checkcolorb, 256 ) + 
												get_green_from_rgb_value ( checkcolorb, 256 ) + 
												get_blue_from_rgb_value ( checkcolorb, 256 ) ) / 3 ;
							if ( color_a < color_b )
								checkcolora = checkcolorb ;
							
						} else {
							guint32 color_a, color_b ;
							printd ( "iv: we've got a dark picture!\n") ;
							
							color_a = ( get_red_from_rgb_value ( checkcolora, 256 ) + 
												get_green_from_rgb_value ( checkcolora, 256 ) + 
												get_blue_from_rgb_value ( checkcolora, 256 ) ) / 3 ;
							color_b = ( get_red_from_rgb_value ( checkcolorb, 256 ) + 
												get_green_from_rgb_value ( checkcolorb, 256 ) + 
												get_blue_from_rgb_value ( checkcolorb, 256 ) ) / 3 ;
							if ( color_a > color_b )
								checkcolora = checkcolorb ;
							
						}
						
					}
					
					if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgcheckbutton1" )) ) == FALSE )
						checkcolorb = checkcolora ;
				}
				
				// the following lines is no longer of use for us... see above!
				//checkcolora = ( value_r * 256 * 256 ) + ( MIN( value_g, value_b ) * 256 ) + MAX( value_b, value_g ) ;
				//checkcolorb = checkcolora ;
				
				printd ( "iv: " ) ; printd ( text_from_var(checkcolora) ) ; printd ( "\n" ) ;
				
				//printf ( "iv: %06X \n", checkcolora ) ;
				
			} else {
				printd("NO! using standard grey checkerboard as background...\n");
			}
		}
		
		printd("iv: now the fadeout for the background... if wanted :) \n");
		if ( check_color_out_use == TRUE ) // do we want some fadeout on outer-image-fields? (border)
		{
			if ( check_color_auto == TRUE && gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )) ) == FALSE )
			{ // we don't want a specific background color and it should be automatically gained from background above
				guint32 r, g, b ;
				
				/*a = (int) checkcolora - ( ( (int) check_color_out_a * 256 * 256 ) + ( (int) check_color_out_a * 256 ) + (int) check_color_out_a ) ;
				b = (int) checkcolorb - ( ( (int) check_color_out_b * 256 * 256 ) + ( (int) check_color_out_b * 256 ) + (int) check_color_out_b ) ;
				if ( (int) a < 0 ) checkcolorouta = 0 ; else checkcolorouta = a ;
				if ( (int) b < 0 ) checkcoloroutb = 0 ; else checkcoloroutb = b ;*/
				
				r = get_red_from_rgb_value ( checkcolora, 256 ) - check_color_out_a ;
				g = get_green_from_rgb_value ( checkcolora, 256 ) - check_color_out_a ;
				b = get_blue_from_rgb_value ( checkcolora, 256 ) - check_color_out_a ;
				
				if ( r > get_red_from_rgb_value ( checkcolora, 256 ) || r < 0 ) r = 0 ;
				if ( g > get_blue_from_rgb_value ( checkcolora, 256 ) || g < 0 ) g = 0 ;
				if ( b > get_blue_from_rgb_value ( checkcolora, 256 ) || b < 0 ) b = 0 ;
					
				checkcolorouta = ( r * 256 * 256 ) + ( g * 256 ) + b ;
				
				r = get_red_from_rgb_value ( checkcolorb, 256 ) - check_color_out_b ;
				g = get_green_from_rgb_value ( checkcolorb, 256 ) - check_color_out_b ;
				b = get_blue_from_rgb_value ( checkcolorb, 256 ) - check_color_out_b ;
				
				if ( r > get_red_from_rgb_value ( checkcolorb, 256 ) || r < 0 ) r = 0 ;
				if ( g > get_green_from_rgb_value ( checkcolorb, 256 ) || g < 0 ) g = 0 ;
				if ( b > get_blue_from_rgb_value ( checkcolorb, 256 ) || b < 0 ) b = 0 ;
					
				checkcolorouta = ( r * 256 * 256 ) + ( g * 256 ) + b ;
				
				if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgcheckbutton1" )) ) == FALSE )
					checkcoloroutb = checkcolorouta ;
				
				printd ( "iv: " ) ; printd ( text_from_var( checkcolorouta ) ) ; printd ( " " ) ; printd ( text_from_var( checkcoloroutb ) ) ; printd ( "\n" ) ;
			} else { // we don't want automatic color detection....
				guint8 r, g, b, a = 0 ;
				
				if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )) ) == TRUE )
				{
					{
						gdouble color[3] ;
						printd("iv: get color from colorselectiondialog...\n");
						gtk_color_selection_get_color	( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), color);
						
						printd ("iv: show color...\n");
						printd ( "iv: r = " ) ; printd ( text_from_var ( color[0] * 100 ) ) ;
						printd ( ", g = " ) ; printd ( text_from_var ( color[1] * 100 ) ) ;
						printd ( ", b = " ) ; printd ( text_from_var ( color[2] * 100 ) ) ;
						printd ( ", trans = " ) ; printd ( text_from_var ( color[3] * 100 ) ) ;
						printd ( "\n" ) ;
						
						r = color[0] * 255 ; g = color[1] * 255 ; b = color[2] * 255 ; a = color[3] * 255 ;
					}			
				} else {
					r = check_color_a ; g = check_color_a ; b = check_color_a ;
				}
				
				// get every first color
				checkcolorouta = scale_color ( r, g, b, a, check_color_a, check_color_out_a ) ;
				
				// get every second color
				checkcoloroutb = scale_color ( r, g, b, a, check_color_b, check_color_out_b ) ;
			}
		} else {
			checkcolorouta = checkcolora ;
			checkcoloroutb = checkcolorb ;
		}
		
		printd("iv: now check for images in memory etc. \n");
		
		if ( loaded_scaled_image != NULL &&
				gdk_pixbuf_get_width ( loaded_scaled_image ) == canvaswidth && 
				gdk_pixbuf_get_height ( loaded_scaled_image ) == canvasheight )
		{
			if ( loaded_scaled_image != NULL &&
				(double) gdk_pixbuf_get_width( loaded_scaled_image ) == (double) ( (int) ( (double) gdk_pixbuf_get_width( loaded_image ) * scale_x ) ) &&
				(double) gdk_pixbuf_get_height( loaded_scaled_image ) == (double) ( (int) ( (double) gdk_pixbuf_get_height( loaded_image )* scale_y ) ) &&
				gdk_pixbuf_get_has_alpha( loaded_scaled_image ) == gdk_pixbuf_get_has_alpha( loaded_image ) &&
				gdk_pixbuf_get_has_alpha( loaded_scaled_image ) == FALSE && reload == TRUE && force_reload == FALSE &&
				gtkpixmap->allocation.width == gdk_pixbuf_get_width( loaded_scaled_image ) &&
				gtkpixmap->allocation.height == gdk_pixbuf_get_height( loaded_scaled_image ) )
			{	// image seems to be the same size and consistency as before and we don't need any new background...
				printd("iv: nothing to be done - get out of here....\n");
				
				gdk_threads_leave () ;
				stop_image_progressbar () ;
				gdk_threads_enter () ;
				if ( gdkpixmap ) { printd("iv: free pixmap\n"); gdk_pixmap_unref (gdkpixmap) ; }
				if ( dest ) { printd("iv: free pixbuf\n"); gdk_pixbuf_unref ( dest ) ; }
				//if ( drawing_gc ) { printd("free gc\n"); gdk_gc_unref ( drawing_gc ) ; }
				//gdk_pixbuf_unref ( dummy ) ;
				gtk_widget_set_sensitive ( imagedisplay, TRUE );
				if ( gnome_appbar_has_progress )
					gtk_progress_set_value (progress, 0 );
				gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Image displayed."));
				//if ( gnome_appbar_has_progress )
				//	gtk_progress_set_value (progress, 100 );
				//gtk_widget_set_sensitive ( lookup_widget ( user_data, "frame1" ), TRUE );
				//update_screen () ;
				//if ( gnome_appbar_has_progress )
				//	gtk_progress_set_value (progress, 0 );
				//update_screen () ;
				usleep ( 10000 ) ;
				gdk_threads_leave () ;
				stop_image_progressbar () ;
				gdk_threads_enter () ;
				return ;
			} else {
				if ( gtk_toggle_button_get_active ( 
											GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) &&
						gtk_toggle_button_get_active ( 
											GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) )
					{
						//gtk_widget_set_redraw_on_allocate ( FullscreenWindow, FALSE ) ; // API 2.0
						//gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
						if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == TRUE )
							gtk_widget_show ( FullscreenWindowProgressbar ) ;
					}
				if ( gnome_appbar_has_progress )
					gtk_progress_set_value (progress, 20 );
				gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Create matrix..."));
				printd("iv: create matrix...\n");
				update_screen () ;
				
				printd("iv: gdk_pixbuf_new ()\n");
				dummy = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( loaded_image ), gdk_pixbuf_get_has_alpha ( loaded_image ), 
															gdk_pixbuf_get_bits_per_sample ( loaded_image ),
															canvaswidth, canvasheight ) ;
				printd("iv: copying dummy on dest... ( we don't need checkers on background )\n");
				dest = gdk_pixbuf_copy ( dummy ) ;
				gdk_pixbuf_unref ( dummy ) ;
			}
		} else {
			if ( gtk_toggle_button_get_active ( 
										GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) &&
					gtk_toggle_button_get_active ( 
										GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) )
				{
					//gtk_widget_set_redraw_on_allocate ( FullscreenWindow, FALSE ) ; // API 2.0
					//gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
					if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == TRUE )
						gtk_widget_show ( FullscreenWindowProgressbar ) ;
				}
			if ( gnome_appbar_has_progress )
				gtk_progress_set_value (progress, 20 );
			gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Create matrix..."));
			update_screen () ;
			
			printd("iv: gdk_pixbuf_new ()\n");
			dummy = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( loaded_image ), gdk_pixbuf_get_has_alpha ( loaded_image ), 
														gdk_pixbuf_get_bits_per_sample ( loaded_image ),
														canvaswidth, canvasheight ) ;
			//printd("iv: gdk_pixbuf_scale_simple()\n");
			//dest = gdk_pixbuf_scale_simple ( im, imagewidth, imageheight, GDK_INTERP_BILINEAR );
			printd("iv: painting some checkers on background... gdk_pixbuf_composite_color_simple()\n");
			dest = gdk_pixbuf_composite_color_simple ( dummy, canvaswidth, canvasheight, GDK_INTERP_NEAREST, 0, 
																				checksize, checkcolorouta, checkcoloroutb ) ;
			gdk_pixbuf_unref ( dummy ) ;
		}
		
		if ( ! gdk_pixbuf_get_has_alpha ( loaded_image ) ) 
		{
			printd("iv: we don't need an alpha channel for this pic\n");
			printd("iv: so we don't need any pattern behind it...\n");
			checkcolora = 0x000000 ;
			checkcolorb = 0x000000 ;
			//checksize = 0 ;
		}
		
		if ( gnome_appbar_has_progress )
			gtk_progress_set_value (progress, 40 );
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Scaling and placing image..."));
		update_screen () ;
		
		gdk_threads_leave () ;

		/*if ( loaded_scaled_image )
		{
			printf ( "iv: %f x %f x %d == %f x %f x %d \n",	(double) gdk_pixbuf_get_width( loaded_scaled_image ),
																					(double) gdk_pixbuf_get_height( loaded_scaled_image ),
																					gdk_pixbuf_get_has_alpha( loaded_scaled_image ),
																					(double) ( (int) ( (double) gdk_pixbuf_get_width( loaded_image ) * scale ) ),
																					(double) ( (int) ( (double) gdk_pixbuf_get_height( loaded_image )* scale ) ),
																					gdk_pixbuf_get_has_alpha( loaded_image ) ) ;
		}*/

		if ( loaded_scaled_image != NULL &&
			(double) gdk_pixbuf_get_width( loaded_scaled_image ) == (double) ( (int) ( (double) gdk_pixbuf_get_width( loaded_image ) * scale_x ) ) &&
			(double) gdk_pixbuf_get_height( loaded_scaled_image ) == (double) ( (int) ( (double) gdk_pixbuf_get_height( loaded_image )* scale_y ) ) &&
			gdk_pixbuf_get_has_alpha( loaded_scaled_image ) == gdk_pixbuf_get_has_alpha( loaded_image ) &&
			gdk_pixbuf_get_has_alpha( loaded_scaled_image ) == FALSE && reload == TRUE && force_reload == FALSE )
		{
			printd ( "iv: using cached image data...\n");
			//gdk_pixbuf_copy ( loaded_scaled_image ) ;
		} else {
			printd ("iv: scaling image data...\n");
			if ( loaded_scaled_image ) gdk_pixbuf_unref ( loaded_scaled_image ) ;
			loaded_scaled_image = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( loaded_image ), 
														gdk_pixbuf_get_has_alpha(loaded_image), 
														gdk_pixbuf_get_bits_per_sample ( loaded_image ),
														imagewidth, imageheight ) ;
			
			// workaround for not messing up the pattern
			// resizing a transparent image does produce messy
			// output so we use the broken routine only for normal display...
			// hopefully they fix the gdk_pixbuf soon..
			if ( show_tiled_image == TRUE ) 
			{
				guint32 first_color = checkcolora, second_color = first_color /*checkcolorb*/ ;
				printd("iv: gdk_pixbuf_composite_color ( -> loaded_scaled_image)\n");
				gdk_pixbuf_composite_color      (	loaded_image,
													loaded_scaled_image,
													0,
													0,
													imagewidth,
													imageheight,
													0,
													0,
													scale_x,
													scale_y,
													render_quality,
													255,
													imagex,
													imagey,
													checksize,
													first_color,
													second_color );
			} else {
				printd("iv: gdk_pixbuf_composite ( -> loaded_scaled_image)\n");
				gdk_pixbuf_composite/*_color*/      (	loaded_image,
													loaded_scaled_image,
													0,
													0,
													imagewidth,
													imageheight,
													0,
													0,
													scale_x,
													scale_y,
													render_quality,
													255/*,
													imagex,
													imagey,
													checksize,
													first_color,
													second_color*/ );
			}
		}
		
		printd("iv: image-scaling done..\n");
		
		gdk_threads_enter () ;
		
		if ( gnome_appbar_has_progress )
			gtk_progress_set_value (progress, 45 );
		update_screen () ;
		
		gdk_threads_leave () ;
		
		if ( gdk_pixbuf_get_width ( loaded_scaled_image ) == canvaswidth && gdk_pixbuf_get_height ( loaded_scaled_image ) == canvasheight )
		{
			printd("iv: copying loaded_scaled_image on dest... gdk_pixbuf_copy()\n");
			if ( dest != NULL ) gdk_pixbuf_unref ( dest ) ;
			dest = gdk_pixbuf_copy ( loaded_scaled_image ) ;
		} else {
			if ( show_tiled_image == FALSE )
			{	// we only want a normal view of the image - no tiling...
				printd("iv: render loaded_scaled_image on dest... gdk_pixbuf_composite_color ( -> dest)\n");
				gdk_pixbuf_composite_color      (	loaded_scaled_image,
													dest,
													imagex,
													imagey,
													imagewidth,
													imageheight,
													imagex,
													imagey,
													1,
													1,
													render_quality,
													255,
													imagex,
													imagey,
													checksize,
													checkcolora,
													checkcolorb );
			} else {	// we want a tiled view of the image... useful for patterns
				int x, y, count_x = -1, count_y = -1, pos_x = imagex, pos_y = imagey ;
				printd("iv: preparing tiling...\n");
				while ( pos_x + imagewidth < canvaswidth )
				{
					pos_x = pos_x + imagewidth ;
					count_x++ ;
				}
				while ( pos_x + imagewidth > 0 )
				{
					pos_x = pos_x - imagewidth ;
					count_x++ ;
				}
				while ( pos_y + imageheight < canvasheight )
				{
					pos_y = pos_y + imageheight ;
					count_y++ ;
				}
				while ( pos_y + imageheight > 0 )
				{
					pos_y = pos_y - imageheight ;
					count_y++ ;
				}
				if ( count_x < 1 ) count_x = 1 ;
				if ( count_y < 1 ) count_y = 1 ;
				printd( "iv: count_x = " ); printd( text_from_var(count_x) ); printd( "\n" );
				printd( "iv: count_y = " ); printd( text_from_var(count_y) ); printd( "\n" );
				
				pos_x = pos_x + imagewidth ;
				pos_y = pos_y + imageheight ;
				
				printd( "iv: counting finished... drawing pattern...\n" ) ;
				
				for ( y = 0 ; y < count_y ; y++ )
				{
					int current_y = pos_y + ( imageheight * y ) ;
					for ( x = 0 ; x < count_x ; x++ )
					{
						guint32 checker_a = checkcolora ;
						guint32 checker_b = checkcolorb ;
						int current_x = pos_x + ( imagewidth * x ) ;
						int current_width = imagewidth - 
									( current_x+imagewidth>canvaswidth ? current_x+imagewidth-canvaswidth : 0 ) +
									( current_x<0 ? current_x : 0 ) ;
						int current_height = imageheight - 
									( current_y+imageheight>canvasheight ? current_y+imageheight-canvasheight : 0 ) +
									( current_y<0 ? current_y : 0 ) ;
						int threshold = ( (current_x!=imagex || current_y!=imagey) && check_color_out_use ) ? 128 : 255 ;
						if ( threshold < 255 )
						{
							checker_a = checkcolorouta ;
							checker_b = checkcoloroutb ;
						}
						current_x = ( current_width<imagewidth && current_x<0 ) ? 0 : current_x ;
						current_y = ( current_height<imageheight && current_y<0 ) ? 0 : current_y ;
						printd( "iv: count_x = " ); printd( text_from_var(current_x) ); printd( "\n" );
						printd( "iv: count_y = " ); printd( text_from_var(current_y) ); printd( "\n" );
						printd( "iv: imagewidth = " ); printd( text_from_var(current_width) ); printd( "\n" );
						printd( "iv: imageheight = " ); printd( text_from_var(current_height) ); printd( "\n" );
						printd("iv: render loaded_scaled_image on dest... gdk_pixbuf_composite_color ( -> dest)\n");
						gdk_pixbuf_composite_color      (	loaded_scaled_image,
															dest,
															current_x,
															current_y,
															current_width,
															current_height,
															pos_x + ( imagewidth * x ),
															pos_y + ( imageheight * y ),
															1,
															1,
															render_quality,
															threshold,
															current_x,
															current_y,
															checksize,
															checker_a,
															checker_b );
					}
				}
			}
		}
		
		gdk_threads_enter () ;
		printd("iv: display image!\n"); if ( gnome_appbar_has_progress ) gtk_progress_set_value (progress, 50 );
		gdkpixmap = gdk_pixmap_new ( lookup_widget( user_data, "scrolledwindow6" )->window, canvaswidth, canvasheight, -1 ) ;
		printd("iv: gdk_gc_new()\n"); if ( gnome_appbar_has_progress ) gtk_progress_set_value (progress, 55 );
		drawing_gc = gdk_gc_new ( lookup_widget( user_data, "scrolledwindow6" )->window );
		printd("iv: gdk_gc_set_function\n"); if ( gnome_appbar_has_progress ) gtk_progress_set_value (progress, 60 );
		gdk_gc_set_function (drawing_gc, GDK_COPY);
		printd("iv: gdk_rgb_gc_set_foreground\n"); if ( gnome_appbar_has_progress ) gtk_progress_set_value (progress, 70 );
		gdk_rgb_gc_set_foreground (drawing_gc, 0x000000);
		printd("iv: gdk_rgb_gc_set_background\n"); if ( gnome_appbar_has_progress ) gtk_progress_set_value (progress, 75 );
		gdk_rgb_gc_set_background (drawing_gc, 0x000000);

		if ( gnome_appbar_has_progress )
			gtk_progress_set_value (progress, 80 );
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Rendering image..."));
		update_screen () ;
		
		printd("iv: gdk_pixbuf_render_to_drawable\n");
		gdk_pixbuf_render_to_drawable ( dest, gdkpixmap, drawing_gc, 0, 0, 0, 0, canvaswidth, canvasheight, GDK_RGB_DITHER_MAX, 0, 0 ) ;

		if ( gnome_appbar_has_progress )
			gtk_progress_set_value (progress, 90 );
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Show image..."));
		//update_screen () ;

		printd("iv: gtk_pixmap_set\n");
		gtk_pixmap_set ( GTK_PIXMAP(gtkpixmap) , gdkpixmap, NULL ) ;
		printd("iv: rendered.\n");

		//printd("iv: change widget size\n");
		//gtk_widget_set_usize ( gtkpixmap, canvaswidth, canvasheight );

		if ( gdkpixmap ) { printd("iv: free pixmap\n"); gdk_pixmap_unref (gdkpixmap) ; }
		if ( dest ) { printd("iv: free pixbuf\n"); gdk_pixbuf_unref ( dest ) ; }
		if ( drawing_gc ) { printd("iv: free gc\n"); gdk_gc_unref ( drawing_gc ) ; }
		
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Image displayed."));
		if ( force_auto_zoom == TRUE )
		{
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoomwidth" )),TRUE ) ;
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoomheight" )),TRUE ) ;
		}
		
		redraw_bgcolorpixmap ( MainWindow ) ;
	} else {
		printd("iv:  not loaded!\n");
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("No image loaded."));
	}
	if ( gnome_appbar_has_progress )
		gtk_progress_set_value (progress, 100 );
	
	// now check, if user already moved to another image! :)
	// I know that this is something like a hack but it works somehow :)
	if ( check_for_new_image_selection ( filename ) )
		gtk_clist_select_row ( GTK_CLIST(lookup_widget(MainWindow,"imagelist")), check_for_new_image_selection ( filename ), 0 ) ;
	//
	
	//gtk_widget_set_sensitive ( lookup_widget ( user_data, "frame1" ), TRUE );
	//update_screen () ;
	if ( gnome_appbar_has_progress )
		gtk_progress_set_value (progress, 0 );
	
	if ( loaded_image )
	{
		char text[2048], titletext[2048], aspect_text[256] ;
		double scale ;
		
		if ( strcmp ( filename, "__RELOAD__" ) == 0 )
			filename = "it's logo" ;
		
		if ( aspect_correction == TRUE )
			strcpy ( aspect_text, "aspect corrected" ) ;
		else
			strcpy ( aspect_text, "aspect ignored" ) ;
		
		scale = (double) MAX( gdk_pixbuf_get_width(loaded_scaled_image), gdk_pixbuf_get_height(loaded_scaled_image) ) /
				(double) MAX( gdk_pixbuf_get_width(loaded_image), gdk_pixbuf_get_height(loaded_image) ) ;
		
		sprintf ( text, "%s ( %dx%d, zoomed at %d%%, %s )",
					filename, gdk_pixbuf_get_width ( loaded_image ),
					gdk_pixbuf_get_height ( loaded_image ),
					(int) ( scale * (double) 100 ),
					aspect_text ) ;
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _(text));
		sprintf ( titletext, "Look at the stars shows %s ( %d%% )",
					filename, (int) ( scale * (double) 100 ) ) ;
		gtk_window_set_title ( GTK_WINDOW(lookup_widget( MainWindow, "MainWindow" )), titletext ) ;
	}

	if ( loaded_image == NULL )
	{
		gtk_widget_set_sensitive ( imagedisplay, FALSE );
		gtk_widget_set_sensitive ( lookup_widget( PrefsWindow, "prefsimagereload" ), FALSE ) ;
	} else {
		gtk_widget_set_sensitive ( imagedisplay, TRUE );
		gtk_widget_set_sensitive ( lookup_widget( PrefsWindow, "prefsimagereload" ), TRUE ) ;
	}
	
	if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) )
	{
		if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) &&
			GTK_WIDGET_VISIBLE ( FullscreenWindowProgressbar ) )
			gtk_widget_hide ( FullscreenWindowProgressbar ) ;
			//gtk_widget_hide ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
	} else if ( GTK_WIDGET_VISIBLE ( FullscreenWindowProgressbar ) == FALSE &&
				GTK_WIDGET_VISIBLE ( FullscreenWindow ) == TRUE ) {
		//gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
		gtk_widget_show ( FullscreenWindowProgressbar ) ;
	}

	//update_screen () ;
	
	gdk_threads_leave () ;
	usleep ( 100000 ) ;
	stop_image_progressbar () ;
	gdk_threads_enter () ;
	
}


void
zoom_picture ( double value, gpointer user_data )
{
	double scale = 1;
	GtkWidget *imagedisplay ;

	imagedisplay = lookup_widget ( MainWindow, "handlebox1" ) ;
	
	if ( GTK_WIDGET_SENSITIVE ( imagedisplay ) == FALSE ) return ;
	
	printd("iv: zoom picture\n");
	//update_screen();
	
	gtk_widget_set_sensitive ( imagedisplay, FALSE ) ;

	printd("iv: get current zoom level \n");
	scale = (double) gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(lookup_widget( MainWindow, "zoomentry" )) );

	if ( scale != value )
	{
		printd("iv: zoom changed!\n");
		if ( loaded_image )
		{
			printd("iv: setup values\n");
			if ( (double) gtk_spin_button_get_value_as_float ( GTK_SPIN_BUTTON(lookup_widget( MainWindow, "zoomentry" )) ) != value )
			{
				printd("iv: change value in combo entry\n");
				if ( value == -1 ) value = scale ;
				gtk_spin_button_set_value( GTK_SPIN_BUTTON(lookup_widget( MainWindow, "zoomentry" )), value );
				if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
					view_image ( "__UPDATE__", FullscreenWindow ) ;
				else
					view_image ( "__UPDATE__", user_data ) ;
			}
			printd("iv: picture zoomed.\n");
		} else {
			printd("iv: image not loaded yet - not zoomed.\n");
		}
	} else { printd("iv: zoom not changed\n"); }
	
	gtk_widget_set_sensitive ( imagedisplay, TRUE ) ;
}


void
redraw_bgcolorpixmap ( gpointer user_data )
{
	GdkPixmap *gdkpixmap ;
	GdkGC *drawing_gc ;
	GdkPixbuf *dest = NULL, *dummy = NULL ;
	int width = 39 ;
	int height = 30 ;
	guint32 checkcolora = ( check_color_a * 256 * 256 ) + ( check_color_a * 256 ) + check_color_a ;
	guint32 checkcolorb = ( check_color_b * 256 * 256 ) + ( check_color_b * 256 ) + check_color_b ;
	int checksize = check_size / 2 ;
	
	{
		guint8 r, g, b, a ;
		
		printd("iv: redraw bgcolorbuttonpixmap... lets see...\n");
		{
			gdouble color[4] ;
			printd("iv: get color from colorselectiondialog...\n");
			gtk_color_selection_get_color	( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), color);
			
			printd ("iv: show color...\n");
			printd ( "iv: r = " ) ; printd ( text_from_var ( color[0] * 100 ) ) ;
			printd ( ", g = " ) ; printd ( text_from_var ( color[1] * 100 ) ) ;
			printd ( ", b = " ) ; printd ( text_from_var ( color[2] * 100 ) ) ;
			printd ( ", trans = " ) ; printd ( text_from_var ( color[3] * 100 ) ) ;
			printd ( "\n" ) ;
			r = color[0] * 255 ; g = color[1] * 255 ; b = color[2] * 255 ; a = color[3] * 255 ;
		}			
		
		// get every first color
		checkcolora = scale_color ( r, g, b, a, check_color_a, 0 ) ;
		
		printd("iv: "); printd (text_from_var(r)); printd(" "); printd (text_from_var(g)); printd(" "); printd (text_from_var(b)); printd(" "); printd (text_from_var(a)); printd(" \n"); 
		printd("iv: "); printd (text_from_var(checkcolora)); printd ("\n");
		
		// get every second color
		checkcolorb = scale_color ( r, g, b, a, check_color_b, 0 ) ;
		
		printd("iv: "); printd (text_from_var(r)); printd(" "); printd (text_from_var(g)); printd(" "); printd (text_from_var(b)); printd(" "); printd (text_from_var(a)); printd(" \n"); 
		printd("iv: "); printd (text_from_var(checkcolorb)); printd ("\n");
	}
	
	// render this colormess onto our preview-pixmap!
	printd("iv: gdk_pixmap_new\n");
	gdkpixmap = gdk_pixmap_new ( lookup_widget( MainWindow, "MainWindow" )->window, width, height, -1 ) ;
	printd("iv: gdk_gc_new\n");
	drawing_gc = gdk_gc_new ( lookup_widget( MainWindow, "MainWindow" )->window );
	printd("iv: gdk_gc_set_function\n");
	gdk_gc_set_function (drawing_gc, GDK_COPY);
	printd("iv: gdk_rgb_gc_set_foreground\n");
	gdk_rgb_gc_set_foreground (drawing_gc, 0x000000);
	printd("iv: gdk_rgb_gc_set_background\n");
	gdk_rgb_gc_set_background (drawing_gc, 0 );
	printd("iv: gdk_pixbuf_new\n");
	dummy = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( loaded_image ), TRUE, 
											gdk_pixbuf_get_bits_per_sample ( loaded_image ),
											width, height ) ;
	printd("iv: gdk_pixbuf_composite_color_simple\n");
	dest = gdk_pixbuf_composite_color_simple ( dummy, width, height, GDK_INTERP_NEAREST, 0, 
																	checksize, checkcolora, checkcolorb ) ;
	printd("iv: unref dummy\n");
	gdk_pixbuf_unref ( dummy ) ;
	printd("iv: render to drawable\n");
	gdk_pixbuf_render_to_drawable ( dest, gdkpixmap, drawing_gc, 0, 0, 0, 0, width, height, GDK_RGB_DITHER_MAX, 0, 0 ) ;
	printd("iv: set pixmap\n");
	gtk_pixmap_set ( GTK_PIXMAP(lookup_widget(MainWindow,"bgcolorpixmap")) , gdkpixmap, NULL ) ;
	//update_screen();
	printd("iv: now unref used resources...\n");
	if ( gdkpixmap ) gdk_pixmap_unref (gdkpixmap) ;
	if ( dest ) gdk_pixbuf_unref ( dest ) ;
	if ( drawing_gc ) gdk_gc_unref ( drawing_gc ) ;
	printd("iv: preview pixmap refreshed.\n");
}


void
follow_mouse_thread ( void )
{
	GtkWidget *window = NULL ;
	GdkCursor *cursor = gdk_cursor_new ( GDK_TCROSS ) ;
	guint32 timestamp = GDK_CURRENT_TIME ;
	int old_Mouse_x = Mouse_x, old_Mouse_y = Mouse_y ; // old mouse position
	int own_task ;
	
	if ( task_already_exists_in_queue ( "follow_mouse", NULL ) ) return ;
	
	own_task = task_add_to_queue ( "follow_mouse", NULL ) ;
	thread_count++ ;
	
	gdk_threads_enter () ;
	
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
		window = FullscreenWindow ;
	else
		window = MainWindow ;
	
	printd("iv: grab the mouse...\n");
	gdk_pointer_grab ( 	lookup_widget(window,"scrolledwindow6")->window,
						TRUE,
						GDK_POINTER_MOTION_MASK & GDK_BUTTON_PRESS_MASK & GDK_BUTTON_RELEASE_MASK,
						lookup_widget(window,"image")->window,
						cursor,
						timestamp ) ;
	
	gdk_threads_leave () ;
	
	printd("iv: follow mouse from now on...\n");
	
	while ( task_already_exists_in_queue ( "stop_following_mouse", NULL ) == FALSE &&
			task_already_exists_in_queue ( "quit", NULL ) == FALSE )
	{
		usleep ( 5000 ) ;
		if ( old_Mouse_x != Mouse_x || old_Mouse_y != Mouse_y )
		{
			GtkAdjustment *horiz_adjust, *vert_adjust ;
			int diff_Mouse_x = old_Mouse_x - Mouse_x ; // differences against
			int diff_Mouse_y = old_Mouse_y - Mouse_y ; // last mouse position
			double new_x_value = 0, new_y_value = 0 ;
			
			/*printd("iv: current mouse position: ");
			printd("x = "); printd( text_from_var(Mouse_x) );
			printd(" ; y = "); printd( text_from_var(Mouse_y) ) ;
			printd(" ; diff_x = "); printd( text_from_var(diff_Mouse_x) );
			printd(" ; diff_y = "); printd( text_from_var(diff_Mouse_y) );
			printd("\n");*/
			old_Mouse_x = Mouse_x ;
			old_Mouse_y = Mouse_y ;
			
			gdk_threads_enter () ;
			
			// get current scrollbar-adjustments
			horiz_adjust = gtk_scrolled_window_get_hadjustment ( GTK_SCROLLED_WINDOW(lookup_widget(window,"scrolledwindow6")) ) ;
			vert_adjust = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW(lookup_widget(window,"scrolledwindow6")) ) ;
			
			//printf( "iv: horiz_min = %f ; horiz_max = %f ; horiz_value = %f\n", horiz_adjust->lower, horiz_adjust->upper, horiz_adjust->value ) ;
			//printf( "iv: vert_min = %f ; vert_max = %f ; vert_value = %f\n", vert_adjust->lower, vert_adjust->upper, vert_adjust->value ) ;
			
			new_x_value = horiz_adjust->value + diff_Mouse_x ;
			new_y_value = vert_adjust->value + diff_Mouse_y ;

			// make sure our values fit...
			if ( new_x_value < horiz_adjust->lower ) 
				new_x_value = horiz_adjust->lower ;
			if ( new_x_value > horiz_adjust->upper - horiz_adjust->page_size ) 
				new_x_value = horiz_adjust->upper - horiz_adjust->page_size ;
			if ( new_y_value < vert_adjust->lower ) 
				new_y_value = vert_adjust->lower ;
			if ( new_y_value > vert_adjust->upper - vert_adjust->page_size ) 
				new_y_value = vert_adjust->upper - vert_adjust->page_size ;			
			
			// set new values...
			gtk_adjustment_set_value ( horiz_adjust, new_x_value ) ;
			gtk_adjustment_set_value ( vert_adjust, new_y_value ) ;
			
			gdk_threads_leave () ;
		}
	}

	printd("iv: following mouse finished.\n");
	
	gdk_threads_enter () ;
	
	printd("iv: ungrab mouse...\n");
	gdk_pointer_ungrab ( timestamp ) ;
	
	gdk_cursor_destroy ( cursor ) ;
	
	gdk_threads_leave () ;
	
	task_remove_from_queue ( own_task ) ;
	thread_count-- ;
}


void
follow_mouse ( void )
{
	pthread_t pagelist_thread ;
	/*int own_task = */task_add_to_queue ( "stop_following_mouse", NULL ) ;
	
	//printf("iv: initialize follow_mouse_thread...\n" ) ;
	
	while ( task_already_exists_in_queue ( "follow_mouse", NULL ) )
	{
		usleep ( 5000 ) ;
		//printd ( "iv: waiting for other follow_mouse threads to shutdown...\n" ) ;
	}
	//task_remove_from_queue ( own_task ) ;
	task_remove_all_from_queue ( "stop_following_mouse", NULL ) ;
	
	pthread_create ( &pagelist_thread, NULL, (void*)&follow_mouse_thread, NULL ) ;
	pthread_detach ( pagelist_thread ) ;
}

