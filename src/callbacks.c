#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define COMPILE_WITH_DEBUG_SUPPORT	TRUE

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

#include <math.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "importedfuncs.h"
#include "imagelist.h"
#include "imageview.h"
#include "toolbars.h"
#include "tasks.h"
#include "settings.h"

GdkPixbuf extern *loaded_image ; // the actual loaded image
GdkPixbuf extern *loaded_scaled_image ; // the scaled image in memory ( performance+ )

int extern DEBUG ;
int extern thumbnails ;
int extern thumb_size ;
int extern max_thumb_size ;
int extern no_thumb_size ;
int extern thumb_border ;
int extern thumb_render_quality ;
int extern render_quality ;
int extern check_size ;
guint32 extern check_color_a ;
guint32 extern check_color_b ;
guint32 extern check_color_out_a ;
guint32 extern check_color_out_b ;
float extern check_color_out_cut_off_factor ;
int extern check_color_out_use ;
int extern hide_unusable_view_buttons ;

int extern filedisplay_as_icons ;
int extern pics_per_row_on_sidebar ;

char extern *LATS_RC_DIR ;
char extern *LATS_RC_FILE ;
char extern *LATS_RC_TEMP_DIR ;
char extern *LATS_RC_THUMB_DIR ;
char extern *LATS_THUMB_DIR ;
int extern LATS_RC_THUMB_IN_HOME ;

GtkWidget extern *MainWindow ;
GtkWidget extern *PrefsWindow ;
GtkWidget extern *CalibrateWindow ;
GtkWidget extern *BgColorSelectionDialog ;
GtkWidget extern *FullscreenWindow ;
GtkWidget extern *FullscreenWindowProgressbar ;

int MainWindow_x ;
int MainWindow_y ;
int MainWindow_client_x ;
int MainWindow_client_y ;
int MainWindow_width ;
int MainWindow_height ;
int MainWindow_maximized = FALSE ;


int extern Mouse_x ;
int extern Mouse_y ;

void
printd ( char *text )
{
#ifdef COMPILE_WITH_DEBUG_SUPPORT	
	if ( DEBUG != TRUE ) return ;
	printf("%s", text);
#endif
}

void
quit_lats ( void )
{
	printd("cb: quit...\n");
	while ( GTK_WIDGET_SENSITIVE(lookup_widget(MainWindow, "handlebox1")) == FALSE &&
			task_already_exists_in_queue ( "quit", NULL ) == FALSE )
		refresh_screen () ;
	wait_for_services_to_quit () ;
	refresh_screen () ;
	if ( loaded_image ) gdk_pixbuf_unref ( loaded_image ) ;
	//return FALSE;
}

void
refresh_screen ( void )
{
	while (gtk_events_pending () )
	{
		gtk_main_iteration () ;
	}
}

void
update_screen ( void )
{
	gdk_threads_leave () ;
	//usleep ( 1000 ) ;
	gdk_threads_enter () ;
}


void
set_cursor_for_widget ( GtkWidget *widget, int type )
{	// let's set up some nifty cursor-pixmap when the mouse touches the window
	GdkCursor *cursor = NULL ;
	
	if ( type == 0 )
	{
		static unsigned char cursor_bits[] = { 0x0 } ;
		static unsigned char cursormask_bits[] = { 0x0 } ;
		GdkPixmap *source, *mask ;
		GdkColor color = { 0, 0, 0, 0 } ; /* Black. */
		printd("cb: hide cursor of window...\n");
		source = gdk_bitmap_create_from_data ( NULL, cursor_bits, 1, 1 ) ;
		mask = gdk_bitmap_create_from_data ( NULL, cursormask_bits, 1, 1 ) ;
		cursor = gdk_cursor_new_from_pixmap (source, mask, &color, &color, 1, 1);
		gdk_pixmap_unref ( source ) ;
		gdk_pixmap_unref ( mask ) ;
	}
	
	if ( type == 1 )
	{
		printd("cb: set hand cursor for window...\n");
		cursor = gdk_cursor_new ( GDK_HAND1 ) ;
	}

	if ( type == 2 )
	{	// this is for the image-window
		printd("cb: set cursor for image-window...\n");
		cursor = gdk_cursor_new ( GDK_HAND1 ) ;
	}

	if ( type == 3 )
	{	// this is for the image-window while dragging
		printd("cb: set cursor for image-window...\n");
		cursor = gdk_cursor_new ( GDK_TCROSS ) ;
	}

	gtk_widget_realize ( widget ) ;
	gdk_window_set_cursor ( widget->window, cursor ) ;
	printd("cb: cursor set.\n");
	gdk_cursor_destroy ( cursor ) ;
}


void
on_new_file1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	quit_lats () ;
}


void
on_cut1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_copy1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_paste1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_clear1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_properties1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_widget_hide ( PrefsWindow ) ;
	gtk_widget_show ( PrefsWindow ) ;
	gdk_window_set_transient_for ( PrefsWindow->window, MainWindow->window ) ;
#ifndef COMPILE_WITH_DEBUG_SUPPORT
	gtk_widget_hide ( lookup_widget( PrefsWindow, "frame4" ) ) ;
#endif
}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *about1;
	about1 = create_about1 ();
	gtk_widget_show (about1);
	gdk_window_set_transient_for ( about1->window, MainWindow->window ) ;
}


void
on_exit0_activate                      (GtkButton       *button,
                                        gpointer         user_data)
{
	quit_lats () ;
}


gboolean
on_MainWindow_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	quit_lats () ;
	return FALSE ;
}


gboolean
on_MainWindow_destroy_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	quit_lats () ;
	return FALSE ;
}


void
on_combo_entry1_activate               (GtkEditable     *editable,
                                        gpointer         user_data)
{
	printd("cb: --- read dir from combo entry\n");
	refresh_screen ();
	read_dir_from_combo ( FALSE, user_data );
	printd("cb: --- dir from combo entry read.\n");
}


void
on_button9_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{	// reload current dir and redo thumbnails!
	printd("cb: --- refresh dir\n");
	read_dir_from_combo ( TRUE, user_data );
	printd("cb: --- dir refreshed\n");
}

void
on_dirlist_select_row                  (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	char *directory ;
	GtkWidget *window, *parentwindow ;
	char *clist_entry ;
	char cwd[2048] ;

	refresh_screen () ;
	
	window = lookup_widget( user_data, "combo_entry1" );
    parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
	
	directory = gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ;
	
	gtk_clist_get_text ( clist, row, 0, &clist_entry );
	
	printd("cb: --- change to new dir\n");
	
	//chdir ( clist_entry );
	
	//getcwd( cwd, 2048 );
	
	strcpy ( cwd, directory ) ;
	
	if ( cwd [ strlen(cwd) - 1 ] != 47 )
		strcat ( cwd, "/" ) ;

	// FIXME try to get entry back to tilde if home directory..
	if ( cwd == gnome_vfs_expand_initial_tilde ( "~/" ) )
		strcpy ( cwd, "~/" );

	//printf ( "cb: got char '%d'\n", cwd [ strlen(cwd) - 1 ] ) ;
	//printf ( "cb: cwd = '%s'\n", cwd ) ;
	//printf ( "cb: user home dir = '%s'\n", gnome_vfs_expand_initial_tilde ( "~/" ) ) ;
		
	if ( clist_entry[ strlen(clist_entry) - 1 ] == 46 && strlen(clist_entry) == 1 ) // if we clicked on '.'
	{
		printd ( "cb: reload current directory...\n");
	} else if ( cwd[0] == 126 && strlen(cwd) == 2 && clist_entry[ strlen(clist_entry) - 2 ] == 46 && strlen(clist_entry) == 2 )
	{
		int pos = strlen( gnome_vfs_expand_initial_tilde ( cwd ) ) - 2 ;
		int i ;

		printd ( "cb: change ~/ to homedir name and go one step back in dirtree...\n" ) ;
		strcpy( cwd, gnome_vfs_expand_initial_tilde ( cwd ) ) ;
		
		while ( cwd[ pos ] != 47 && pos > 0 )
			pos-- ;
		if ( cwd[ pos ] == 47 && pos > 1 ) pos-- ;
		for ( i = pos+1 ; i < strlen(cwd) ; i++ ) 
		{
			cwd[ i ] = 0 ;
		}
		//printf ("cb: cwd = %s\n", cwd ) ;
	} else if ( clist_entry[ strlen(clist_entry) - 2 ] == 46 && strlen(clist_entry) == 2 )  // if we clicked on '..'
	{
		int pos = strlen(cwd) - 1 ;
		int i ;
		
		if ( cwd[ pos ] == 47 && pos > 1 ) pos-- ;
		printd ( "cb: !!!!! switch to previous directory!!\n");
		while ( cwd[ pos ] != 47 && pos > 0 )
			pos-- ;
		//printf ("cb: got char '%d' at pos %d from %d chars\n", cwd[pos], pos, strlen(cwd) ) ;
		for ( i = pos+1 ; i < strlen(cwd) ; i++ ) 
		{
			cwd[ i ] = 0 ;
		}
		//printf ("cb: cwd = %s\n", cwd ) ;
	} else { // anything else was clicked.. hope'it was a directory :)
		strcat ( cwd, clist_entry ) ;
	}
	
	gtk_entry_set_text( GTK_ENTRY( parentwindow ), cwd );
	
	printd("cb: --- refresh dir\n");
	read_dir_from_combo ( FALSE, user_data );
	printd("cb: --- dir refreshed\n");
}


void
on_imagelist_select_row                (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if ( gtk_clist_get_pixmap ( clist, row, 0, NULL, NULL ) )
	{
		char *clist_entry, *oldfilename = NULL ;
		GtkCList *imagestatsclist;
		char old[2048], new[2048];
		
		printd("cb: lookup_widget imagestatsclist...\n");
		imagestatsclist = GTK_CLIST(lookup_widget( user_data, "imagestatsclist" ));
		printd("cb: get filename from imagestatsclist\n");
		gtk_clist_get_text ( imagestatsclist, 0, 1, &oldfilename );
		printd("cb: put in old var\n");
		sprintf ( old, "%s", oldfilename ) ;
		printd("cb: get entry from imagelist\n");
		gtk_clist_get_text ( clist, row, 1, &clist_entry );
		printd("cb: put in new var\n");
		sprintf ( new, "%s", clist_entry ) ;
		printd("cb: compare them\n");
		printd("cb: old filename = "); printd(old); printd(" has "); printd(text_from_var(strlen(old))); printd(" chars\n");
		printd("cb: new filename = "); printd(new), printd(" has "); printd(text_from_var(strlen(new)));printd(" chars\n");
		
		if ( strncmp( old, new, MAX( strlen(old), strlen(new) ) ) )
		{
			printd("cb: let's load da new picture!\n");
			//gtk_widget_set_sensitive ( lookup_widget ( MainWindow, "frame1" ), FALSE ) ;
			//refresh_screen() ;
			
			if ( GTK_WIDGET_SENSITIVE(lookup_widget(MainWindow,"handlebox1")) == TRUE )
			{
				if ( event != NULL )
				{
					gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), row ) ;
					if ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist"))->pad22 != NULL )
						move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), row ) ;
				} else {
					if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
					{
						task_add_to_queue ( "load_image", clist_entry ) ;
						//task_add_to_queue ( "display_in_window", NULL ) ;
					} else {
						task_add_to_queue ( "load_image", clist_entry ) ;
						//task_add_to_queue ( "display_fullscreen", NULL ) ;
					}
				}
				gtk_progress_set_value ( GTK_PROGRESS(lookup_widget ( MainWindow, "progressbar1" )),
											row+1 ) ;
				gtk_progress_set_value ( GTK_PROGRESS(lookup_widget ( FullscreenWindowProgressbar, "progressbar1" )),
											row+1 ) ;
			}
			
			//gtk_widget_set_sensitive ( lookup_widget ( MainWindow, "frame1" ), TRUE ) ;
			//refresh_screen();
		} else {
			printd("cb: we won't load the same file again!\n");
		}
	}
}


void
on_homebutton_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *window, *parentwindow ;

	window = lookup_widget( user_data, "combo_entry1" );
    parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
	
	printd("cb: --- change to home dir\n");
	
	gtk_entry_set_text( GTK_ENTRY( parentwindow ), "~/" );
	
	printd("cb: --- refresh dir\n");
	read_dir_from_combo ( FALSE, user_data );
	printd("cb: --- dir refreshed\n");
}


void
on_vmbutton1_clicked                   (GtkButton       *button,
                                        gpointer         user_data)				// file view
{
	int imageinfo_width = 0 ;
	int imagedisplay_width = 0 ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
	
	gtk_widget_show ( lookup_widget ( user_data, "imageinfo" ) ) ; imageinfo_width = 100 * pics_per_row_on_sidebar + 100 * 0.65 ;
	gtk_widget_show ( lookup_widget ( user_data, "imagedisplay" ) ) ; imagedisplay_width = -1 ;
	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_hide ( 1 ) ; vmbutton_show ( 2 ) ; vmbutton_show ( 3 ) ;
	}
	vmbutton_disable ( 1 ) ; vmbutton_enable ( 2 ) ; vmbutton_enable ( 3 ) ;
	
	gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), 1 ) ;
	
	if ( lookup_widget ( user_data, "imageinfo" )->allocation.width != imageinfo_width )
		gtk_widget_set_usize ( lookup_widget ( user_data, "imageinfo" ), imageinfo_width, -1 ) ;

	gtk_widget_hide ( lookup_widget ( user_data, "vmbox1" ) ) ;

	if ( GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton2" ) ) == TRUE &&
			GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton1" ) ) == FALSE ) {
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, 
								lookup_widget ( MainWindow, "findimage" ) -> allocation.height ) ;
		gtk_widget_show ( lookup_widget( MainWindow, "frame2" ) ) ;
	}

	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
	
	//refresh_screen () ;
	
	if ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist"))->selection )
		move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), 
								GPOINTER_TO_INT(GNOME_ICON_LIST(
									lookup_widget(MainWindow,"iconlist"))->selection->data) ) ;
	
	refresh_screen () ;
	
	check_for_initial_directory_read () ;
}


void
on_vmbutton2_clicked                   (GtkButton       *button,
                                        gpointer         user_data)				// splitted view
{
	int imageinfo_width = 0 ;
	int imagedisplay_width = 0 ;
	GtkProgress *progress ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
	
	gtk_widget_show ( lookup_widget ( user_data, "imageinfo" ) ) ; imageinfo_width = 100 * pics_per_row_on_sidebar + 100 * 0.65 ;
	gtk_widget_show ( lookup_widget ( user_data, "imagedisplay" ) ) ; imagedisplay_width = -1 ;
	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_show ( 1 ) ; vmbutton_hide ( 2 ) ; vmbutton_show ( 3 ) ;
	}
	vmbutton_enable ( 1 ) ; vmbutton_disable ( 2 ) ; vmbutton_enable ( 3 ) ;
	
	gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), 0 ) ;
	
	if ( lookup_widget ( user_data, "imageinfo" )->allocation.width != imageinfo_width )
		gtk_widget_set_usize ( lookup_widget ( user_data, "imageinfo" ), imageinfo_width, -1 ) ;
	
	gtk_widget_hide ( lookup_widget ( user_data, "vmbox1" ) ) ;
	
	progress = GTK_PROGRESS ( lookup_widget ( MainWindow, "progressbar1" ) ) ;
	
	if ( gtk_progress_get_percentage_from_value ( progress, 1 ) != 0 )
	{
		if ( GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton2" ) ) == TRUE &&
				GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton1" ) ) == TRUE )
		{
			gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, -1 ) ;
			gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton1" ) , TRUE ) ;
			gtk_widget_show ( lookup_widget( MainWindow, "frame1nb" ) ) ;
		} else if ( GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton2" ) ) == FALSE &&
				GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton1" ) ) == TRUE ) {
			gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, 
						lookup_widget ( MainWindow, "findimage" ) -> allocation.height - 
						lookup_widget ( MainWindow, "hbox11" ) -> allocation.height ) ;
		} else if ( GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton2" ) ) == TRUE &&
				GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton1" ) ) == FALSE ) {
			gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, 1 ) ;
			gtk_widget_hide ( lookup_widget ( MainWindow, "frame2" ) ) ;
			gtk_widget_show ( lookup_widget( MainWindow, "frame1nb" ) ) ;
		}
	} else {
		//printf("cb: vmbutton2 no images expand dirlist only!\n");
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, 
								lookup_widget ( MainWindow, "findimage" ) -> allocation.height ) ;
		gtk_widget_hide ( lookup_widget( MainWindow, "frame1" ) ) ;
		gtk_widget_show ( lookup_widget( MainWindow, "frame2" ) ) ;
	}
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
	
	//refresh_screen () ;
	
	if ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1"))->selection && 
			gtk_notebook_get_current_page ( GTK_NOTEBOOK( lookup_widget(MainWindow,"frame1nb")) ) == 1 )
		move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), 
								GPOINTER_TO_INT(GNOME_ICON_LIST(
									lookup_widget(MainWindow,"iconlist1"))->selection->data) ) ;
	
	refresh_screen () ;
	
	check_for_initial_directory_read () ;
}


void
on_vmbutton3_clicked                   (GtkButton       *button,
                                        gpointer         user_data)				// image view
{
	int imageinfo_width = 0 ;
	int imagedisplay_width = 0 ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
	
	imageinfo_width = 1 ;
	gtk_widget_show ( lookup_widget ( user_data, "imagedisplay" ) ) ; imagedisplay_width = -1 ;
	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_show ( 1 ) ; vmbutton_show ( 2 ) ; vmbutton_hide ( 3 ) ;
	}
	vmbutton_enable ( 1 ) ; vmbutton_enable ( 2 ) ; vmbutton_disable ( 3 ) ;
	
	gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), 0 ) ;
	
	gtk_widget_set_usize ( lookup_widget ( user_data, "imageinfo" ), imageinfo_width, -1 ) ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
	
	//refresh_screen () ;
}


void
on_bgcoloruse_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
	{
		task_add_to_queue ( "display_fullscreen", NULL ) ;
	} else {
		task_add_to_queue ( "display_in_window", NULL ) ;
	}
}


void
on_bgtiles_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
	{
		task_add_to_queue ( "display_fullscreen", NULL ) ;
	} else {
		task_add_to_queue ( "display_in_window", NULL ) ;
	}
}


void
on_zoomentry_changed                   (GtkEditable     *editable,
                                        gpointer         user_data)
{
	printd("cb: zoomentry changed\n");
	zoom_picture ( -1, user_data );
}


void
on_zoom100_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	printd("cb: zoom to 1x ( 100% / normal ) clicked\n");
	zoom_picture ( 100, user_data );
}


void
on_autozoom_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	/*if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )) ) )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )), TRUE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )), TRUE ) ;
	} else {
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )), FALSE ) ;
	}*/

	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )) ) == FALSE ||
		 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )) ) == FALSE )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
		//gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )), TRUE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )), TRUE ) ;
	} else if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )) ) == TRUE &&
			    gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )) ) == TRUE ) {
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
		//gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )), FALSE ) ;
	} 
}


void
on_autozoomwidth_toggled                    (GtkToggleButton *togglebutton,
											 gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )) ) )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )), FALSE ) ;
	}
	if ( loaded_image )
	{
		GtkWidget *imagedisplay ;
		//refresh_screen () ;
		imagedisplay = lookup_widget ( MainWindow, "handlebox1" ) ;
		
		if ( GTK_WIDGET_SENSITIVE ( imagedisplay ) == FALSE ) return ;

		printd( "cb: autozoom by width called - reload image...\n" );
		if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
		{
			task_add_to_queue ( "display_in_window", NULL ) ;
		} else {
			task_add_to_queue ( "display_fullscreen", NULL ) ;
		}
	}
}


void
on_autozoomheight_toggled                    (GtkToggleButton *togglebutton,
											  gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )) ) )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )), FALSE ) ;
	}
	if ( loaded_image )
	{
		GtkWidget *imagedisplay ;
		//refresh_screen () ;
		imagedisplay = lookup_widget ( MainWindow, "handlebox1" ) ;
		
		if ( GTK_WIDGET_SENSITIVE ( imagedisplay ) == FALSE ) return ;

		printd( "cb: autozoom by height called - reload image...\n" );
		if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
		{
			task_add_to_queue ( "display_in_window", NULL ) ;
		} else {
			task_add_to_queue ( "display_fullscreen", NULL ) ;
		}
	}
}


void
on_keepaspect_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	printd( "cb: keepaspect toggled - reload image...\n" );
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
	{
		task_add_to_queue ( "display_in_window", NULL ) ;
	} else {
		task_add_to_queue ( "display_fullscreen", NULL ) ;
	}
}


void
on_scrolledwindow6_size_allocate       (GtkWidget       *widget,		// this is the biiig pixmap-widget!!
                                        GtkAllocation   *allocation,
                                        gpointer         user_data)
{	
	printd( "cb: scrolledwindow6 got size_allocate...\n" );
	if ( GTK_WIDGET_IS_SENSITIVE(lookup_widget(MainWindow,"imagedisplay")) == FALSE ) return ;

	printd("cb: new allocation\n");
	
	if ( lookup_widget( MainWindow, "imagedisplay" ) && 
		gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) == 0 )
	{
		GtkWidget *imagedisplay ;
		
		imagedisplay = lookup_widget ( MainWindow, "handlebox1" ) ;
		
		task_add_to_queue ( "display_in_window", NULL ) ;
	}

}


void
on_zoomm25_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	double scale, temp_scale ;
	
	temp_scale = ( (double) MAX ( gdk_pixbuf_get_width ( loaded_scaled_image ), 
								gdk_pixbuf_get_height ( loaded_scaled_image ) ) * (double) 0.75 ) / 
						(double) MAX ( gdk_pixbuf_get_width ( loaded_image ), 
									   gdk_pixbuf_get_height ( loaded_image ) ) * (double) 25 ;
	
	scale = (double) gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(lookup_widget( user_data, "zoomentry" )) );
	
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )), FALSE ) ;
	
	//scale = ( (double) MAX(gdk_pixbuf_get_width(loaded_scaled_image), gdk_pixbuf_get_height(loaded_scaled_image)) ) * ( (double) 0.75 ) ;
	
	zoom_picture ( (double) scale - temp_scale, user_data );
}


void
on_zoomp25_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	double scale, temp_scale ;

	/*temp_scale = (double) MAX ( gdk_pixbuf_get_width ( loaded_scaled_image ), 
								gdk_pixbuf_get_height ( loaded_scaled_image ) ) / 
					(	(double) MAX ( gdk_pixbuf_get_width ( loaded_scaled_image ), 
									   gdk_pixbuf_get_height ( loaded_scaled_image ) ) * (double) 0.25 ) ;*/
	temp_scale = ( (double) MAX ( gdk_pixbuf_get_width ( loaded_scaled_image ), 
								gdk_pixbuf_get_height ( loaded_scaled_image ) ) * (double) 0.75 ) / 
						(double) MAX ( gdk_pixbuf_get_width ( loaded_image ), 
									   gdk_pixbuf_get_height ( loaded_image ) ) * (double) 25 ;
	
	scale = (double) gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(lookup_widget( user_data, "zoomentry" )) );
	
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )), FALSE ) ;

	//scale = ( (double) MAX(gdk_pixbuf_get_width(loaded_scaled_image), gdk_pixbuf_get_height(loaded_scaled_image)) ) * ( (double) 1.25 ) ;
	
	zoom_picture ( (double) scale + temp_scale, user_data );
}


void
on_zoom100_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )) ) )
	{
		int real_zoom = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )) ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )), FALSE ) ;
		printd("cb: zoom to 1x ( 100% / normal ) clicked\n");
		zoom_picture ( 100, user_data );
		if ( real_zoom ) {
			if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
			{
				task_add_to_queue ( "display_in_window", NULL ) ;
			} else {
				task_add_to_queue ( "display_fullscreen", NULL ) ;
			}
		}
	}
}


void
on_realzoom_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "realzoom" )) ) )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomwidth" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoomheight" )), FALSE ) ;
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "keepaspect" )), TRUE ) ;
		printd("cb: real monitor dependand scaling chosen. zoom to 1x ( 100% / normal ) !\n");
		zoom_picture ( 100, user_data );
	}
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
	{
		task_add_to_queue ( "display_in_window", NULL ) ;
	} else {
		task_add_to_queue ( "display_fullscreen", NULL ) ;
	}
}


void
on_prefsimagereload_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{	
	//view_image ( "__FORCE_RELOAD__", MainWindow ) ;
	task_add_to_queue ( "reload_image", NULL ) ;
	task_add_to_queue ( "display_in_window", NULL ) ;
}


void
on_prefsimageradiobutton1_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		render_quality = GDK_INTERP_NEAREST ;
}


void
on_prefsimageradiobutton2_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		render_quality = GDK_INTERP_TILES ;
}


void
on_prefsimageradiobutton3_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		render_quality = GDK_INTERP_BILINEAR ;
}


void
on_prefsimageradiobutton4_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		render_quality = GDK_INTERP_HYPER ;
}


void
on_bgcolorok_button1_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{

	redraw_bgcolorpixmap ( MainWindow ) ;

	printd ("cb: bgcolor apply button clicked!\n");
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )) ) != TRUE )
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "bgcoloruse" )), TRUE ) ;
	else
		task_add_to_queue ( "display_in_window", NULL ) ;
		//view_image ( "__UPDATE__", MainWindow );
}


gboolean
on_bgcolorvp_button_press_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	printd ("cb: show our sweet colorselector!\n");
	gtk_widget_hide ( BgColorSelectionDialog ) ;
	gtk_widget_show ( BgColorSelectionDialog ) ;
	return TRUE;
}


void
on_prefsmaintogglebutton1_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	DEBUG = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsmaintogglebutton1" )) ) ;
}


void
on_imageiconview_size_allocate         (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data)
{
	//printd( "cb: imageiconview got size_allocate...\n" );
	
}


void
on_packer1_size_allocate               (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data)
{
	printd( "cb: packer1 got size_allocate...\n" );
	
	if ( GTK_WIDGET_VISIBLE( lookup_widget( user_data, "frame1" ) ) == FALSE )
		gtk_widget_set_usize ( lookup_widget( user_data, "frame2" ), -1, lookup_widget ( user_data, "findimage" ) -> allocation.height ) ;
	
	if ( GTK_WIDGET_VISIBLE( lookup_widget( user_data, "imagedisplay" ) ) == FALSE )
		gtk_widget_set_usize ( lookup_widget( user_data, "imageinfo" ), lookup_widget ( user_data, "hboxmain" ) -> allocation.width, -1 ) ;
	else
		task_add_to_queue ( "display_in_window", NULL ) ;
		//view_image ( "__UPDATE__", user_data ) ;

}


void
on_prefsthumbradiobutton1_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		thumb_render_quality = GDK_INTERP_NEAREST ;
}


void
on_prefsthumbradiobutton2_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		thumb_render_quality = GDK_INTERP_TILES ;
}


void
on_prefsthumbradiobutton3_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		thumb_render_quality = GDK_INTERP_BILINEAR ;
}


void
on_prefsthumbradiobutton4_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		thumb_render_quality = GDK_INTERP_HYPER ;
}


void
on_prefsmainentry1_activate            (GtkEditable     *editable,
                                        gpointer         user_data)
{	// rc-dir
	LATS_RC_DIR = gtk_entry_get_text( GTK_ENTRY( gtk_widget_get_ancestor (GTK_WIDGET(lookup_widget( PrefsWindow, "prefsmainentry1" )), GTK_TYPE_EDITABLE) ) ) ;
	printd("cb: new LATS_RC_DIR = "); printd(LATS_RC_DIR); printd("\n");
}


void
on_prefsmainentry2_activate            (GtkEditable     *editable,
                                        gpointer         user_data)
{	// rc-file
	LATS_RC_FILE = gtk_entry_get_text( GTK_ENTRY( gtk_widget_get_ancestor (GTK_WIDGET(lookup_widget( PrefsWindow, "prefsmainentry2" )), GTK_TYPE_EDITABLE) ) ) ;
	printd("cb: new LATS_RC_FILE = "); printd(LATS_RC_FILE); printd("\n");
}


void
on_prefsthumbentry1_activate           (GtkEditable     *editable,
                                        gpointer         user_data)
{	// main-rc-thumb-dir
	LATS_RC_THUMB_DIR = gtk_entry_get_text( GTK_ENTRY( gtk_widget_get_ancestor (GTK_WIDGET(lookup_widget( PrefsWindow, "prefsthumbentry1" )), GTK_TYPE_EDITABLE) ) ) ;
	printd("cb: new LATS_RC_THUMB_DIR = "); printd(LATS_RC_THUMB_DIR); printd("\n");
}


void
on_prefsthumbentry2_activate           (GtkEditable     *editable,
                                        gpointer         user_data)
{	// general-thumb-dir
	LATS_THUMB_DIR = gtk_entry_get_text( GTK_ENTRY( gtk_widget_get_ancestor (GTK_WIDGET(lookup_widget( PrefsWindow, "prefsthumbentry2" )), GTK_TYPE_EDITABLE) ) ) ;
	printd("cb: new LATS_THUMB_DIR = "); printd(LATS_THUMB_DIR); printd("\n");
}


void
on_prefsthumbradiobutton5_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{	// save thumbs to dir where the pics are
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		LATS_RC_THUMB_IN_HOME = FALSE ;
	printd("cb: save thumbs to current dir\n");
}


void
on_prefsthumbradiobutton6_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{	// save thumbs to main-rc-dir
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		LATS_RC_THUMB_IN_HOME = TRUE ;
	printd("cb: save thumbs to main-rc-dir\n");
}


void
on_iconlist_select_icon                (GnomeIconList   *gnomeiconlist,
                                        gint             arg1,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	GtkCList *clist = GTK_CLIST(lookup_widget(MainWindow,"imagelist")) ;
	printd("cb: icon selected = "); printd(text_from_var(arg1)); printd("\n");
	gtk_clist_select_row ( clist, arg1, 0 ) ;
	gtk_clist_moveto ( clist, arg1, 0, 0.5, 0 ) ;
	//gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), 0 ) ;

	if ( //gnome_icon_list_get_items_per_line ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")) ) < arg1 &&
			gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 1 )
	{
		//printf("FIXME: if gnome_icon_list_moveto priv->lines != NULL\n");
		move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), arg1 ) ;
	}

	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )) ) == 1 &&
		event != NULL )
	{
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
		if ( gtk_clist_get_pixmap ( clist, arg1, 0, NULL, NULL ) )
		{
			if ( event->button.button == 1 )
			{ // left mousebutton pressed
				on_vmbutton2_clicked ( NULL, MainWindow ) ;
				//on_vmbutton1_clicked ( NULL, MainWindow ) ;
			} else if ( event->button.button == 2 ) { // middle mousebutton pressed
				gtk_widget_show ( FullscreenWindow ) ;
				//task_add_to_queue ( "display_fullscreen", NULL ) ;
			}
		}
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), arg1 ) ;
		move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), arg1 ) ;
		refresh_screen() ;
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
		//refresh_screen() ;
	} else if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )) ) == 0 &&
					GTK_WIDGET_SENSITIVE( lookup_widget(MainWindow,"handlebox1")) == TRUE ) {
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
		if ( event != NULL && event->button.button == 2 ) { // middle mousebutton pressed
			gtk_widget_show ( FullscreenWindow ) ;
			//task_add_to_queue ( "display_fullscreen", NULL ) ;
		}
		//on_vmbutton1_clicked ( NULL, MainWindow ) ;
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), arg1 ) ;
		//refresh_screen() ;
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
	}
}


void
on_imagedisplay_switch_page            (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        gint             page_num,
                                        gpointer         user_data)
{
	if ( page_num  == 1 )
	{
		GtkWidget *appbar2 = lookup_widget ( MainWindow, "appbar2");
		gtk_widget_hide ( lookup_widget ( MainWindow, "frame1" ) ) ;
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, lookup_widget ( MainWindow, "findimage" ) -> allocation.height ) ;
		if ( GTK_WIDGET_VISIBLE(appbar2) ) gnome_appbar_set_status (GNOME_APPBAR (appbar2), _(""));
	} else if ( gtk_clist_get_text ( GTK_CLIST(lookup_widget(MainWindow,"imagelist")), 0, 1, NULL ) ) {
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, -1 ) ;
		gtk_widget_show ( lookup_widget ( MainWindow, "frame1" ) ) ;
	}
}


void
on_scrolledwindow7_size_allocate       (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data)
{
	GnomeIconList *gil = GNOME_ICON_LIST ( lookup_widget ( MainWindow, "iconlist" ) ) ;
	int window_width ;
	int thumb_count ;
	int thumb_col_spacing ;
	//int thumb_width ;
	
	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) == 0 )
		return ;

	printd("cb: gnomeiconlist got size allocate!\n");

	window_width = lookup_widget(MainWindow,"iconlist")->allocation.width ;
	thumb_count = ( window_width - fmod ( window_width, max_thumb_size + thumb_border ) ) / ( max_thumb_size + thumb_border ) ;
	thumb_col_spacing = fmod ( window_width, max_thumb_size + thumb_border ) / thumb_count ;
	//thumb_width = window_width / thumb_count - thumb_col_spacing*2 ;

	//refresh_screen () ;
	
	printd("cb: change icon_width to fit space\n");
	//printd("thumb_width = "); printd(text_from_var(thumb_width)); printd("\n");
	gnome_icon_list_set_icon_width ( gil, max_thumb_size ) ;
	printd("cb: thumb_col_spacing = "); printd(text_from_var(thumb_col_spacing)); printd("\n");
	gnome_icon_list_set_col_spacing ( gil, thumb_col_spacing + thumb_border ) ;
	printd("cb: set border of thumbs\n");
	gnome_icon_list_set_icon_border ( gil, 5 ) ; // not yet supported :((
}


void
on_scrolledwindow8_size_allocate       (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data)
{
	GnomeIconList *gil = GNOME_ICON_LIST ( lookup_widget ( MainWindow, "iconlist1" ) ) ;
	int window_width ;
	int thumb_count ;
	int thumb_col_spacing ;
	//int thumb_width ;
	GList *sList;
	GtkCList *clist ;
	int rNum = 0 ;
	
	printd("cb: gnomeiconlist1 got size allocate!\n");
	
	clist = GTK_CLIST(lookup_widget(MainWindow,"imagelist")) ;
	
	sList = clist->selection;
	if ( sList ) 
	{
		rNum = (int) sList->data ;
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum ) ;
		if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 1 )
			move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum ) ;
		//return rNum ;
	} else {
		//return FALSE ;
	}
	
	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 0 )
		return ;

	window_width = lookup_widget(MainWindow,"iconlist1")->allocation.width ;
	thumb_count = ( window_width - fmod ( window_width, max_thumb_size + thumb_border ) ) / ( max_thumb_size + thumb_border ) ;
	thumb_col_spacing = fmod ( window_width, max_thumb_size + thumb_border ) / thumb_count ;
	//thumb_width = window_width / thumb_count - thumb_col_spacing*2 ;

	//refresh_screen () ;
	
	printd("cb: change icon_width to fit space\n");
	//printd("thumb_width = "); printd(text_from_var(thumb_width)); printd("\n");
	gnome_icon_list_set_icon_width ( gil, max_thumb_size ) ;
	printd("cb: thumb_col_spacing = "); printd(text_from_var(thumb_col_spacing)); printd("\n");
	gnome_icon_list_set_col_spacing ( gil, thumb_col_spacing + thumb_border ) ;
	printd("cb: set border of thumbs\n");
	gnome_icon_list_set_icon_border ( gil, 5 ) ; // not yet supported :((
}


gboolean
on_MainWindow_key_press_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
	
	printd("cb: MainWindow_key_press_event!\n");

	if ( GTK_WIDGET_IS_SENSITIVE ( lookup_widget ( MainWindow, "handlebox1" ) ) == FALSE ) return FALSE ;
	
	if ( event->keyval == GDK_Page_Up ) {
		printd("cb: cursor up!\n");
		go_to_previous_image_in_list () ;
	}
	if ( event->keyval == GDK_Page_Down ) {
		printd("cb: cursor down!\n");
		go_to_next_image_in_list () ;
	}
	return TRUE;
}


gboolean
on_MainWindow_key_release_event        (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
	printd("cb: MainWindow_key_release_event!\n");
	/*if ( event->keyval == GDK_Up )
		printd("cb: cursor up!\n");
	if ( event->keyval == GDK_Down )
		printd("cb: cursor down!\n");*/
	return TRUE;
}


void
on_frame1radiobutton1_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "frame1nb" )), 0 ) ;
}


void
on_frame1radiobutton2_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "frame1nb" )), 1 ) ;
}




gboolean
on_scrolledwindow6_button_press_event  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	printd("cb: got mousebutton click on image of mainwindow ( button ");
	printd(text_from_var(event->button)); printd(" )\n");
	
	// begin to drag the picture
	if ( event->button == 1 )
		follow_mouse () ;
	
	// switch to fullscreen-mode
	if ( event->button == 2 )
		gtk_widget_show ( FullscreenWindow ) ;

	// wheel-mouse actions
	if ( event->button == 4 )
		go_to_previous_image_in_list () ;
	if ( event->button == 5 )
		go_to_next_image_in_list () ;

	return TRUE;
}


gboolean
on_scrolledwindow6_button_release_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	printd("cb: got mousebutton release on image of mainwindow ( button ");
	printd(text_from_var(event->button)); printd(" )\n");
	
	// end dragging the picture
	if ( event->button == 1 )
		task_add_to_queue ( "stop_following_mouse", NULL ) ;
	
	return TRUE;
}


gboolean
on_fullscreenwindow_button_press_event (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	printd("cb: got mousebutton click on image of fullscreenwindow ( button ");
	printd(text_from_var(event->button)); printd(" )\n");

	// begin to drag the picture
	if ( event->button == 1 )
		follow_mouse () ;

	// switch to windowed-mode
	if ( event->button == 2 )
		gtk_widget_hide ( FullscreenWindow ) ;

	// wheel-mouse actions
	if ( event->button == 4 )
		go_to_previous_image_in_list () ;
	if ( event->button == 5 )
		go_to_next_image_in_list () ;

	return TRUE;
}


gboolean
on_fullscreenwindow_button_release_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	printd("cb: got mousebutton release on image of fullscreenwindow ( button ");
	printd(text_from_var(event->button)); printd(" )\n");
	
	// end dragging the picture
	if ( event->button == 1 )
		task_add_to_queue ( "stop_following_mouse", NULL ) ;
	
	return TRUE;
}


void
on_fullscreenwindow_hide               (GtkWidget       *widget,
                                        gpointer         user_data)
{
	view_image ( "__RELOAD__", MainWindow ) ;
	task_add_to_queue ( "display_in_window", NULL ) ;
	gtk_widget_hide ( FullscreenWindowProgressbar ) ;
	//refresh_screen () ;
}


void
on_fullscreenwindow_show               (GtkWidget       *widget,
                                        gpointer         user_data)
{
	GdkPixmap *gdkpixmap ;
	//int fswidth = gdk_screen_width () ;
	//int fsheight = gdk_screen_height () ;
	printd("cb: show picture in own window...\n");
	
	gdkpixmap = gdk_pixmap_new ( lookup_widget( FullscreenWindow, "scrolledwindow6" )->window, 1, 1, -1 ) ;
	gtk_pixmap_set ( GTK_PIXMAP(lookup_widget(FullscreenWindow, "image")) , gdkpixmap, NULL ) ;
	gdk_pixmap_unref ( gdkpixmap ) ;

	//gtk_widget_set_uposition ( FullscreenWindow, 0, 0 ) ;
	
	if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) == TRUE )
	{
		if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) == FALSE )
		{
			gtk_widget_hide ( FullscreenWindowProgressbar ) ;
			//gtk_widget_hide ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
		}
		//gtk_widget_set_usize ( FullscreenWindow, gdk_screen_width (), gdk_screen_height () ) ;
		printd("cb: set size of fullscreenwindow...\n");
		gtk_window_set_default_size ( GTK_WINDOW(FullscreenWindow), gdk_screen_width (),
				gdk_screen_height () ) ;
		//gtk_widget_set_usize ( lookup_widget(FullscreenWindow,"scrolledwindow6"),
		//		gdk_screen_width (), gdk_screen_height () ) ;
	} else /*if ( GTK_WIDGET_VISIBLE(FullscreenWindowProgressbar) == FALSE )*/ {
		//gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
		gtk_widget_hide ( FullscreenWindowProgressbar ) ;
		gtk_widget_show ( FullscreenWindowProgressbar ) ;
		printd("cb: set size of fullscreenwindow...\n");
		gtk_window_set_default_size ( GTK_WINDOW(FullscreenWindow), gdk_screen_width (),
				gdk_screen_height () - lookup_widget(MainWindow,"appbar1")->allocation.height + 6 ) ;
		gtk_widget_set_usize ( lookup_widget(FullscreenWindow,"scrolledwindow6"),
				gdk_screen_width (), gdk_screen_height () - 
				lookup_widget(MainWindow,"appbar1")->allocation.height + 6 ) ;
	}
	
	set_cursor_for_widget ( lookup_widget ( FullscreenWindow, "image" ), 2 ) ;
	
	refresh_screen () ;
	
	//view_image ( "__RELOAD__", FullscreenWindow ) ;
	//sleep ( 1 ) ;
	task_add_to_queue ( "display_fullscreen", NULL ) ;
	//task_add_to_queue ( "display_fullscreen", NULL ) ;
}


void
on_frame1movebutton1_clicked           (GtkButton       *button,
                                        gpointer         user_data)					// 'enlarge iconlist' button
{
	if ( lookup_widget ( MainWindow, "frame1" )->allocation.height <= 1 || 
		GTK_WIDGET_VISIBLE( lookup_widget ( MainWindow, "frame1nb" ) ) == FALSE )	// 50% icon- and 50% dirlist
	{
		printd ( "cb: show frame 1...\n" ) ;
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, -1 ) ;
		gtk_widget_show ( lookup_widget ( MainWindow, "frame1" ) ) ;
		gtk_widget_show ( lookup_widget( MainWindow, "frame1nb" ) ) ;
		gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton1" ) , TRUE ) ;
		gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton2" ) , TRUE ) ;
	} else if ( lookup_widget ( MainWindow, "frame2" )->allocation.height > 1 ) {	// 0% dirlist / 100% iconlist
		printd ( "cb: hide frame 2...\n" ) ;
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, 1 ) ;
		gtk_widget_hide ( lookup_widget ( MainWindow, "frame2" ) ) ;
		gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton1" ) , FALSE ) ;
		gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton2" ) , TRUE ) ;
	}
}


void
on_frame1movebutton2_clicked           (GtkButton       *button,
                                        gpointer         user_data)					// 'enlarge directory list' button
{
	if ( lookup_widget ( MainWindow, "frame2" )->allocation.height <= 1 || 
		GTK_WIDGET_VISIBLE( lookup_widget ( MainWindow, "frame2" ) ) == FALSE )		// 50% icon- and 50% dirlist
	{
		printd ( "cb: show frame 2...\n" ) ;
		gtk_widget_show ( lookup_widget ( MainWindow, "frame2" ) ) ;
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, -1 ) ;
		gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton1" ) , TRUE ) ;
		gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton2" ) , TRUE ) ;
	} else if ( lookup_widget ( MainWindow, "frame2" )->allocation.height > 1 && 
				GTK_WIDGET_VISIBLE( lookup_widget ( MainWindow, "frame1nb" ) ) == TRUE ) {	// 100% dirlist / 0% iconlist
		printd ( "cb: hide frame 1...\n" ) ;
		//gtk_widget_hide ( lookup_widget ( MainWindow, "frame1" ) ) ;
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, 
								lookup_widget ( MainWindow, "findimage" ) -> allocation.height - 
								lookup_widget ( MainWindow, "hbox11" ) -> allocation.height ) ;
		gtk_widget_hide ( lookup_widget( MainWindow, "frame1nb" ) ) ;
		gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton1" ) , TRUE ) ;
		gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton2" ) , FALSE ) ;
	}
}


void
on_prefsthumbclearbutton_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
	char command[2048] ;
	
	strcpy ( command, "rm -rvf " ) ;
	strcat ( command, gnome_vfs_expand_initial_tilde ( LATS_RC_DIR ) ) ;
	if ( command [ strlen(command) - 1 ] != 47 )
		strcat ( command, "/" ) ;
	strcat ( command, LATS_RC_THUMB_DIR ) ;
	if ( command [ strlen(command) - 1 ] != 47 )
		strcat ( command, "/" ) ;
	strcat ( command, "*" ) ;
	
	printd ( "cb: delete all thumbnails in rc-dir...\n" ) ;
	
	system ( command ) ;
	
	printd ( "cb: thumbnails in rc-dir deleted.\n" ) ;
	
	
}


void
on_prefsthumbupdatebutton_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
	// FIXME :)
}



void
on_toggle_fullscreen1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
		gtk_widget_hide ( FullscreenWindow ) ;
	else
		gtk_widget_show ( FullscreenWindow ) ;
}


void
on_toggle_desktop_view1_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	refresh_screen () ;

	if ( MainWindow_maximized == FALSE )
	{
		printd("cb: gather old window position / size and save it...\n");
		gdk_window_get_origin ( MainWindow->window, &MainWindow_x, &MainWindow_y ) ;
		gdk_window_get_geometry ( MainWindow->window, &MainWindow_client_x, &MainWindow_client_y, 
													  &MainWindow_width, &MainWindow_height, NULL ) ;

		printd("cb: now maximize the mainwindow...\n");
	
		gdk_window_move_resize (MainWindow->window, 
								-MainWindow_client_x, -MainWindow_client_y, 
								gdk_screen_width(), gdk_screen_height() );

		MainWindow_x = MainWindow_x - MainWindow_client_x ;
		MainWindow_y = MainWindow_y - MainWindow_client_y ;
		
		MainWindow_maximized = TRUE ;
	} else {
		printd("cb: restore old mainwindow position and size ...\n");
		
		gdk_window_move_resize (MainWindow->window, MainWindow_x, MainWindow_y, 
													MainWindow_width, MainWindow_height );

		MainWindow_maximized = FALSE ;
	}
}


void
on_iconlist2_select_icon               (GnomeIconList   *gnomeiconlist,
                                        gint             arg1,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	char destfile[2048], *filename ;
	GtkProgress *progress = GTK_PROGRESS(GTK_PROGRESS_BAR(lookup_widget(MainWindow,"imageprogressbar"))) ;
	int density = gtk_spin_button_get_value_as_int( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton9" ) ) ) ;
	GtkCList *imagestatsclist ;
	GdkPixbuf *temp_image ;
	
	imagestatsclist = GTK_CLIST(lookup_widget( user_data, "imagestatsclist" ));
	gtk_clist_get_text ( imagestatsclist, 0, 1, &filename );
	
	arg1++ ; // we begin to count with 001 not 000 ...

	{
		int pos ;
		char temp_name[2048] ;
		getcwd ( temp_name, 2048 ) ;
		strcat ( temp_name, "/" ) ;
		strcat ( temp_name, filename ) ;
		for ( pos = 0 ; pos < strlen(temp_name) ; pos++ )
		{
			//printd( text_from_var(  strlen(index(temp_name,47))  ) ); printd(" ");
			if ( temp_name[pos] == 47 ) temp_name[pos] = 183 ;
		}

		strcpy ( destfile, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
		strcat ( destfile, LATS_RC_TEMP_DIR ) ;
		strcat ( destfile, temp_name ) ;
		sprintf( destfile, "%s.%dDPI.%03d%s", destfile, density, arg1, ".ps.temp" ) ;
	}
		
	temp_image = gdk_pixbuf_new_from_file ( destfile ) ;
	
	if ( temp_image )
	{
		gdk_pixbuf_unref ( loaded_image ) ;
		
		gtk_progress_set_value ( progress, arg1 ) ;
		
		printd ( "cb: load page number " ) ; printd ( text_from_var ( arg1 ) ) ;
		printd ( " from file '" ) ; printd ( destfile ) ; printd ( "'\n" ) ;
		
		loaded_image = gdk_pixbuf_copy ( temp_image ) ;
		
		task_add_to_queue ( "reload_image", NULL ) ;
		
		/*if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
		{
			task_add_to_queue ( "display_in_window", NULL ) ;
		} else {
			task_add_to_queue ( "display_fullscreen", NULL ) ;
		}*/
		
		gdk_pixbuf_unref ( temp_image ) ;
	}
}


void
on_prefsmainloadbutton_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
	// now load our settings!
	settings_load_all () ;
}


void
on_prefsmainsavebutton_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
	// now save our settings!
	settings_save_all () ;
}


void
on_prefsmainreset_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	// FIXME :)
}

void
on_prefsimagecalibratebutton_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide ( CalibrateWindow ) ;
	gtk_widget_show ( CalibrateWindow ) ;
	gdk_window_set_transient_for ( CalibrateWindow->window, MainWindow->window ) ;
}


void
on_radiobuttoninch_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
	{
		double old_value, new_value ;
		// first the horizontal value...
		old_value = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonhoriz" ) ) ) ;
		new_value = old_value / (double) 25.4 ;
		gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonhoriz" ) ),
									new_value ) ;
		// and now the vertical value...
		old_value = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonvert" ) ) ) ;
		new_value = old_value / (double) 25.4 ;
		gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonvert" ) ),
									new_value ) ;
	}
}


void
on_radiobuttonmm_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( togglebutton ) )
	{
		double old_value, new_value ;
		// first the horizontal value...
		old_value = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonhoriz" ) ) ) ;
		new_value = old_value * (double) 25.4 ;
		gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonhoriz" ) ),
									new_value ) ;
		// and now the vertical value...
		old_value = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonvert" ) ) ) ;
		new_value = old_value * (double) 25.4 ;
		gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonvert" ) ),
									new_value ) ;
	}
}


void
on_calibrate_okbutton_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	double dpi_x = 0, dpi_y = 0 ;
	int x, y ;
	
	x = 500 ; // set this according to the gui!!!
	y = 400 ; // set this according to the gui!!!
	
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( CalibrateWindow, "radiobuttonmm" )) ) )
	{
		double mm_x = 0, mm_y = 0 ;
		mm_x = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonhoriz" ) ) ) ;
		mm_y = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonvert" ) ) ) ;
		dpi_x = (double) x / ( mm_x / (double) 25.4 ) ;
		dpi_y = (double) y / ( mm_y / (double) 25.4 ) ;
	} else {
		double inch_x = 0, inch_y = 0 ;
		inch_x = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonhoriz" ) ) ) ;
		inch_y = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonvert" ) ) ) ;
		dpi_x = (double) x / inch_x ;
		dpi_y = (double) y / inch_y ;
	}
	
	gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttonx" ) ),
									dpi_x ) ;
	gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttony" ) ),
									dpi_y ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimageradiobutton6" )), TRUE ) ;
}


void
on_calibrate_cancelbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
	double value_x = 0, value_y = 0 ;
	int x, y ;

	x = 500 ; // set this according to the gui!!!
	y = 400 ; // set this according to the gui!!!

	value_x = (double) gtk_spin_button_get_value_as_float ( 
						GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttonx" ) ) ) ;
	value_y = (double) gtk_spin_button_get_value_as_float ( 
						GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttony" ) ) ) ;

	value_x = (double) x / value_x ;
	value_y = (double) y / value_y ;

	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( CalibrateWindow, "radiobuttonmm" )) ) )
	{
		value_x = value_x * (double) 25.4 ;
		value_y = value_y * (double) 25.4 ;
	}
	
	gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonhoriz" ) ),
									value_x ) ;
	gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( CalibrateWindow, "spinbuttonvert" ) ),
									value_y ) ;
	
}


void
on_calibratewindow_show                (GtkWidget       *widget,
                                        gpointer         user_data)
{
	on_calibrate_cancelbutton_clicked ( NULL, user_data ) ;
}


gboolean
on_scrolledwindow6_motion_notify_event (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{
	//printd("cb: scrolledwindow6_motion_notify_event\n");
	Mouse_x = event->x_root ;
	Mouse_y = event->y_root ;
	
	return TRUE;
}


gboolean
on_fullscreenwindow_motion_notify_event
                                        (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{
	//printd("cb: fullscreenwindow_motion_notify_event\n");
	Mouse_x = event->x_root ;
	Mouse_y = event->y_root ;
	
	return TRUE;
}

