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

#include <math.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "importedfuncs.h"
#include "imagelist.h"
#include "imageview.h"
#include "toolbars.h"
#include "tasks.h"

GdkPixbuf extern *loaded_image ; // the actual loaded image
GdkPixbuf extern *loaded_scaled_image ; // the scaled image in memory ( performance+ )

int extern DEBUG ;
int extern thumbnails ;
int extern thumb_size ;
int extern max_thumb_size ;
int extern no_thumb_size ;
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
char extern *LATS_RC_THUMB_DIR ;
char extern *LATS_THUMB_DIR ;
int extern LATS_RC_THUMB_IN_HOME ;

GtkWidget extern *MainWindow ;
GtkWidget extern *PrefsWindow ;
GtkWidget extern *BgColorSelectionDialog ;
GtkWidget extern *FullscreenWindow ;

void
printd ( char *text )
{
	if ( DEBUG != TRUE ) return ;
	printf("%s", text);
}

void
quit_lats ( void )
{
	printd("beenden...\n");
	while ( GTK_WIDGET_SENSITIVE(lookup_widget(MainWindow, "handlebox1")) == FALSE )
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
}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *about1;
	about1 = create_about1 ();
	gtk_widget_show (about1);
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
	printd("--- read dir from combo entry\n");
	refresh_screen ();
	read_dir_from_combo ( FALSE, user_data );
	printd("--- dir from combo entry read.\n");
}


void
on_button9_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	/*char cmd[2048] ;
	
	strcpy ( cmd, "rm -rf ");
	if ( LATS_RC_THUMB_IN_HOME == TRUE )
		strcat ( cmd, gnome_vfs_expand_initial_tilde( LATS_RC_DIR ) ) ;
	else
		strcat ( cmd, "." ) ;
	strcat ( cmd, LATS_RC_THUMB_DIR ) ;
	system ( cmd );*/
	
	printd("--- refresh dir\n");
	read_dir_from_combo ( TRUE, user_data );
	//view_image ( "__FORCE_RELOAD__", user_data );
	printd("--- dir refreshed\n");
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
	
	gtk_clist_get_text ( clist, row, column, &clist_entry );
	
	printd("--- change to new dir\n");
	
	//chdir ( clist_entry );
	
	//getcwd( cwd, 2048 );
	
	strcpy ( cwd, directory ) ;
	
	if ( cwd [ strlen(cwd) - 1 ] != 47 )
		strcat ( cwd, "/" ) ;

	// FIXME try to get entry back to tilde if home directory..
	if ( cwd == gnome_vfs_expand_initial_tilde ( "~/" ) )
		strcpy ( cwd, "~/" );

	//printf ( "got char '%d'\n", cwd [ strlen(cwd) - 1 ] ) ;
	//printf ( "cwd = '%s'\n", cwd ) ;
	//printf ( "user home dir = '%s'\n", gnome_vfs_expand_initial_tilde ( "~/" ) ) ;
		
	if ( clist_entry[ strlen(clist_entry) - 1 ] == 46 && strlen(clist_entry) == 1 ) // if we clicked on .
	{
		printd ( "reload current directory...\n");
	} else if ( cwd[0] == 126 && strlen(cwd) == 2 && clist_entry[ strlen(clist_entry) - 2 ] == 46 && strlen(clist_entry) == 2 )
	{
		int pos = strlen( gnome_vfs_expand_initial_tilde ( cwd ) ) - 2 ;
		int i ;

		printd ( "change ~/ to homedir name and go one step back in dirtree...\n" ) ;
		strcpy( cwd, gnome_vfs_expand_initial_tilde ( cwd ) ) ;
		
		while ( cwd[ pos ] != 47 && pos > 0 )
			pos-- ;
		if ( cwd[ pos ] == 47 && pos > 1 ) pos-- ;
		for ( i = pos+1 ; i < strlen(cwd) ; i++ ) 
		{
			cwd[ i ] = 0 ;
		}
		//printf ("cwd = %s\n", cwd ) ;
	} else if ( clist_entry[ strlen(clist_entry) - 2 ] == 46 && strlen(clist_entry) == 2 )  // if we clicked on ..
	{
		int pos = strlen(cwd) - 1 ;
		int i ;
		
		if ( cwd[ pos ] == 47 && pos > 1 ) pos-- ;
		printd ( "!!!!! switch to previous directory!!\n");
		while ( cwd[ pos ] != 47 && pos > 0 )
			pos-- ;
		//printf ("got char '%d' at pos %d from %d chars\n", cwd[pos], pos, strlen(cwd) ) ;
		for ( i = pos+1 ; i < strlen(cwd) ; i++ ) 
		{
			cwd[ i ] = 0 ;
		}
		//printf ("cwd = %s\n", cwd ) ;
	} else { // anything else was clicked.. hope'it was a directory :)
		strcat ( cwd, clist_entry ) ;
	}
	
	gtk_entry_set_text( GTK_ENTRY( parentwindow ), cwd );
	
	printd("--- refresh dir\n");
	read_dir_from_combo ( FALSE, user_data );
	printd("--- dir refreshed\n");
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
		
		printd("lookup_widget imagestatsclist...\n");
		imagestatsclist = GTK_CLIST(lookup_widget( user_data, "imagestatsclist" ));
		printd("get filename from imagestatsclist\n");
		gtk_clist_get_text ( imagestatsclist, 0, 1, &oldfilename );
		printd("put in old var\n");
		sprintf ( old, "%s", oldfilename ) ;
		printd("get entry from imagelist\n");
		gtk_clist_get_text ( clist, row, 1, &clist_entry );
		printd("put in new var\n");
		sprintf ( new, "%s", clist_entry ) ;
		printd("compare them\n");
		printd("old filename = "); printd(old); printd(" has "); printd(text_from_var(strlen(old))); printd(" chars\n");
		printd("new filename = "); printd(new), printd(" has "); printd(text_from_var(strlen(new)));printd(" chars\n");
		
		if ( strncmp( old, new, MAX( strlen(old), strlen(new) ) ) )
		{
			printd("let's load da new picture!\n");
			//gtk_widget_set_sensitive ( lookup_widget ( MainWindow, "frame1" ), FALSE ) ;
			//refresh_screen() ;
			
			if ( GTK_WIDGET_SENSITIVE(lookup_widget(MainWindow,"handlebox1")) == TRUE )
			{
				if ( event != NULL )
				{
					gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), row ) ;
					if ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist"))->pad22 != NULL )
						gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), row, 0.5 ) ;
				} else 
					if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
					{
						//view_image ( clist_entry, user_data );
						task_add_to_queue ( "load_image", clist_entry ) ;
						//task_add_to_queue ( "display_in_window", NULL ) ;
					} else {
						//view_image ( clist_entry, FullscreenWindow );
						task_add_to_queue ( "load_image", clist_entry ) ;
						//task_add_to_queue ( "display_fullscreen", NULL ) ;
					}
				gtk_progress_set_value ( GTK_PROGRESS(lookup_widget ( MainWindow, "progressbar1" )),
											row+1 ) ;
				gtk_progress_set_value ( GTK_PROGRESS(lookup_widget ( FullscreenWindow, "progressbar1" )),
											row+1 ) ;
			}
			
			//gtk_widget_set_sensitive ( lookup_widget ( MainWindow, "frame1" ), TRUE ) ;
			//refresh_screen();
		} else {
			printd("we won't load the same file again!\n");
		}
	}
}


void
on_homebutton_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	//char *directory ;
	GtkWidget *window, *parentwindow ;
	//char cwd[2048] ;

	//refresh_screen () ;
	
	window = lookup_widget( user_data, "combo_entry1" );
    parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
	
	//directory = gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ;
	
	printd("--- change to home dir\n");
	
	/*chdir ( getenv("HOME") );
	
	getcwd(cwd, 2048) ;
	
	printd("new working dir = "); printd(cwd); printd("\n");*/
	
	gtk_entry_set_text( GTK_ENTRY( parentwindow ), "~/" );
	
	printd("--- refresh dir\n");
	read_dir_from_combo ( FALSE, user_data );
	printd("--- dir refreshed\n");
}


/*void
on_vmbutton1_clicked                   (GtkButton       *button,
                                        gpointer         user_data)				// file view
{
	int imageinfo_width = 0 ;
	int imagedisplay_width = 0 ;
	
	gtk_widget_show ( lookup_widget ( user_data, "imageinfo" ) ) ; imageinfo_width = lookup_widget(user_data,"hboxmain")->allocation.width ;
	gtk_widget_hide ( lookup_widget ( user_data, "imagedisplay" ) ) ; imagedisplay_width = 1 ;
	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_hide ( 1 ) ; vmbutton_show ( 2 ) ; vmbutton_show ( 3 ) ;
	}
	vmbutton_disable ( 1 ) ; vmbutton_enable ( 2 ) ; vmbutton_enable ( 3 ) ;
	
	gtk_widget_set_usize ( lookup_widget ( user_data, "imageinfo" ), imageinfo_width, -1 ) ;
	//gtk_widget_set_usize ( lookup_widget ( user_data, "imagedisplay"), imagedisplay_width, -1 ) ;

	gtk_widget_show ( lookup_widget ( user_data, "vmbox1" ) ) ;
}*/


void
on_vmbutton1_clicked                   (GtkButton       *button,
                                        gpointer         user_data)				// file view
{
	int imageinfo_width = 0 ;
	int imagedisplay_width = 0 ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
	
	gtk_widget_show ( lookup_widget ( user_data, "imageinfo" ) ) ; imageinfo_width = thumb_size * pics_per_row_on_sidebar + thumb_size * 0.65 ;
	gtk_widget_show ( lookup_widget ( user_data, "imagedisplay" ) ) ; imagedisplay_width = -1 ;
	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_hide ( 1 ) ; vmbutton_show ( 2 ) ; vmbutton_show ( 3 ) ;
	}
	vmbutton_disable ( 1 ) ; vmbutton_enable ( 2 ) ; vmbutton_enable ( 3 ) ;
	
	gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), 1 ) ;
	
	gtk_widget_set_usize ( lookup_widget ( user_data, "imageinfo" ), imageinfo_width, -1 ) ;
	//gtk_widget_set_usize ( lookup_widget ( user_data, "imagelist" ), imageinfo_width, -1 ) ;
	//gtk_widget_set_usize ( lookup_widget ( user_data, "imagedisplay"), imagedisplay_width, -1 ) ;

	gtk_widget_hide ( lookup_widget ( user_data, "vmbox1" ) ) ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
	
	//refresh_screen () ;
	
	if ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist"))->selection )
	gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), 
								GPOINTER_TO_INT(GNOME_ICON_LIST(
									lookup_widget(MainWindow,"iconlist"))->selection->data), 0.5 ) ;
}


void
on_vmbutton2_clicked                   (GtkButton       *button,
                                        gpointer         user_data)				// splitted view
{
	int imageinfo_width = 0 ;
	int imagedisplay_width = 0 ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
	
	gtk_widget_show ( lookup_widget ( user_data, "imageinfo" ) ) ; imageinfo_width = thumb_size * pics_per_row_on_sidebar + thumb_size * 0.65 ;
	gtk_widget_show ( lookup_widget ( user_data, "imagedisplay" ) ) ; imagedisplay_width = -1 ;
	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_show ( 1 ) ; vmbutton_hide ( 2 ) ; vmbutton_show ( 3 ) ;
	}
	vmbutton_enable ( 1 ) ; vmbutton_disable ( 2 ) ; vmbutton_enable ( 3 ) ;
	
	gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), 0 ) ;
	
	gtk_widget_set_usize ( lookup_widget ( user_data, "imageinfo" ), imageinfo_width, -1 ) ;
	//gtk_widget_set_usize ( lookup_widget ( user_data, "imagedisplay"), imagedisplay_width, -1 ) ;
	
	gtk_widget_hide ( lookup_widget ( user_data, "vmbox1" ) ) ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
	
	//refresh_screen () ;
	
	if ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1"))->selection && 
			gtk_notebook_get_current_page ( GTK_NOTEBOOK( lookup_widget(MainWindow,"frame1nb")) ) == 1 )
	gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), 
								GPOINTER_TO_INT(GNOME_ICON_LIST(
									lookup_widget(MainWindow,"iconlist1"))->selection->data), 0.5 ) ;
}


void
on_vmbutton3_clicked                   (GtkButton       *button,
                                        gpointer         user_data)				// image view
{
	int imageinfo_width = 0 ;
	int imagedisplay_width = 0 ;
	
	//printf("vmbutton3_clicked begin\n");
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
	
	/*gtk_widget_hide ( lookup_widget ( user_data, "imageinfo" ) ) ;*/ imageinfo_width = 1 ;
	gtk_widget_show ( lookup_widget ( user_data, "imagedisplay" ) ) ; imagedisplay_width = -1 ;
	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_show ( 1 ) ; vmbutton_show ( 2 ) ; vmbutton_hide ( 3 ) ;
	}
	vmbutton_enable ( 1 ) ; vmbutton_enable ( 2 ) ; vmbutton_disable ( 3 ) ;
	
	gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), 0 ) ;
	
	gtk_widget_set_usize ( lookup_widget ( user_data, "imageinfo" ), imageinfo_width, -1 ) ;
	//gtk_widget_set_usize ( lookup_widget ( user_data, "imagedisplay"), imagedisplay_width, -1 ) ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
	
	//refresh_screen () ;
	
	//printf("vmbutton3_clicked end\n");
}


void
on_bgcoloruse_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
	{
		//view_image ( "__UPDATE__", FullscreenWindow );
		task_add_to_queue ( "display_fullscreen", NULL ) ;
	} else {
		//view_image ( "__UPDATE__", user_data );
		task_add_to_queue ( "display_in_window", NULL ) ;
	}
}


void
on_zoomentry_changed                   (GtkEditable     *editable,
                                        gpointer         user_data)
{
	//double scale ;
	printd("zoomentry changed\n");
	//refresh_screen () ;
	//scale = (double) gtk_spin_button_get_value_as_float( GTK_OBJECT(lookup_widget( user_data, "zoomentry" )) );
	zoom_picture ( -1, user_data );
}


void
on_zoom100_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	printd("zoom to 1x ( 100% / normal ) clicked\n");
	zoom_picture ( 100, user_data );
}


void
on_autozoom_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )) ) )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
		if ( loaded_image )
		{
			GtkWidget *imagedisplay ;
			//refresh_screen () ;
			imagedisplay = lookup_widget ( MainWindow, "handlebox1" ) ;
			
			if ( GTK_WIDGET_SENSITIVE ( imagedisplay ) == FALSE ) return ;

			printd( "autozoom called - reload image...\n" );
			if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
			{
				//view_image ( "__UPDATE__", user_data );
				task_add_to_queue ( "display_in_window", NULL ) ;
			} else {
				//view_image ( "__UPDATE__", FullscreenWindow );
				task_add_to_queue ( "display_fullscreen", NULL ) ;
			}
		}
	}
}


void
on_keepaspect_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	printd( "keepaspect toggled - reload image...\n" );
	if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
	{
		//view_image ( "__UPDATE__", user_data );
		task_add_to_queue ( "display_in_window", NULL ) ;
	} else {
		//view_image ( "__UPDATE__", FullscreenWindow );
		task_add_to_queue ( "display_fullscreen", NULL ) ;
	}
}


void
on_scrolledwindow6_size_allocate       (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data)
{	
	//int width = lookup_widget ( user_data, "scrolledwindow6" )->allocation.width ;
	//int height = lookup_widget ( user_data, "scrolledwindow6" )->allocation.height ;
	printd( "scrolledwindow6 got size_allocate...\n" );
	//gtk_widget_queue_resize ( lookup_widget( user_data, "scrolledwindow6" ) ) ;
	if ( GTK_WIDGET_IS_SENSITIVE(lookup_widget(MainWindow,"imagedisplay")) == FALSE ) return ;
	//if ( GTK_WIDGET_IS_SENSITIVE(lookup_widget(MainWindow,"handlebox1")) == FALSE ) return ;
	//if ( width == allocation->width && height == allocation->height ) return ;

	printd("new allocation\n");
	//return;
	
	if ( lookup_widget( MainWindow, "imagedisplay" ) && 
		gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) == 0 )
	{
		GtkWidget *imagedisplay ;
		
		//gtk_widget_show_now ( lookup_widget ( user_data, "MainWindow" ) ) ;
		//refresh_screen () ;	
		
		imagedisplay = lookup_widget ( MainWindow, "handlebox1" ) ;
		
		//if ( GTK_WIDGET_SENSITIVE ( imagedisplay ) == FALSE ) return ;
		
		//printd( "reload image...\n" );
		//return;
		//if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) == FALSE )
			//view_image ( "__RELOAD__", user_data );
			task_add_to_queue ( "display_in_window", NULL ) ;
		//else
		//	view_image ( "__RELOAD__", FullscreenWindow );
		//gtk_widget_draw_default ( lookup_widget( user_data, "scrolledwindow6" ) ) ;
	}

}


void
on_zoomm25_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	double scale ;
	
	scale = (double) gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(lookup_widget( user_data, "zoomentry" )) );
	
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )), FALSE ) ;
	
	//scale = ( (double) MAX(gdk_pixbuf_get_width(loaded_scaled_image), gdk_pixbuf_get_height(loaded_scaled_image)) ) * ( (double) 0.75 ) ;
	
	zoom_picture ( (double) scale * (double) 0.75, user_data );
}


void
on_zoomp25_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	double scale ;
	
	scale = (double) gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(lookup_widget( user_data, "zoomentry" )) );
	
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )), FALSE ) ;
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )), FALSE ) ;

	//scale = ( (double) MAX(gdk_pixbuf_get_width(loaded_scaled_image), gdk_pixbuf_get_height(loaded_scaled_image)) ) * ( (double) 1.25 ) ;
	
	zoom_picture ( (double) scale * (double) 1.25, user_data );
}


void
on_zoom100_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "zoom100" )) ) )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( user_data, "autozoom" )), FALSE ) ;
		printd("zoom to 1x ( 100% / normal ) clicked\n");
		zoom_picture ( 100, user_data );
	}
}


void
on_prefsimagereload_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{	
	//view_image ( "__FORCE_RELOAD__", MainWindow ) ;
	task_add_to_queue ( "reload", NULL ) ;
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

	printd ("bgcolor apply button clicked!\n");
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
	printd ("show our sweet colorselector!\n");
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
	//printd( "imageiconview got size_allocate...\n" );
	
}


void
on_packer1_size_allocate               (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data)
{
	printd( "packer1 got size_allocate...\n" );
	
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
	printd("new LATS_RC_DIR = "); printd(LATS_RC_DIR); printd("\n");
}


void
on_prefsmainentry2_activate            (GtkEditable     *editable,
                                        gpointer         user_data)
{	// rc-file
	LATS_RC_FILE = gtk_entry_get_text( GTK_ENTRY( gtk_widget_get_ancestor (GTK_WIDGET(lookup_widget( PrefsWindow, "prefsmainentry2" )), GTK_TYPE_EDITABLE) ) ) ;
	printd("new LATS_RC_FILE = "); printd(LATS_RC_FILE); printd("\n");
}


void
on_prefsthumbentry1_activate           (GtkEditable     *editable,
                                        gpointer         user_data)
{	// main-rc-thumb-dir
	LATS_RC_THUMB_DIR = gtk_entry_get_text( GTK_ENTRY( gtk_widget_get_ancestor (GTK_WIDGET(lookup_widget( PrefsWindow, "prefsthumbentry1" )), GTK_TYPE_EDITABLE) ) ) ;
	printd("new LATS_RC_THUMB_DIR = "); printd(LATS_RC_THUMB_DIR); printd("\n");
}


void
on_prefsthumbentry2_activate           (GtkEditable     *editable,
                                        gpointer         user_data)
{	// general-thumb-dir
	LATS_THUMB_DIR = gtk_entry_get_text( GTK_ENTRY( gtk_widget_get_ancestor (GTK_WIDGET(lookup_widget( PrefsWindow, "prefsthumbentry2" )), GTK_TYPE_EDITABLE) ) ) ;
	printd("new LATS_THUMB_DIR = "); printd(LATS_THUMB_DIR); printd("\n");
}


void
on_prefsthumbradiobutton5_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{	// save thumbs to dir where the pics are
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		LATS_RC_THUMB_IN_HOME = FALSE ;
	printd("save thumbs to current dir\n");
}


void
on_prefsthumbradiobutton6_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{	// save thumbs to main-rc-dir
	if ( gtk_toggle_button_get_active ( togglebutton ) )
		LATS_RC_THUMB_IN_HOME = TRUE ;
	printd("save thumbs to main-rc-dir\n");
}


void
on_iconlist_select_icon                (GnomeIconList   *gnomeiconlist,
                                        gint             arg1,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	GtkCList *clist = GTK_CLIST(lookup_widget(MainWindow,"imagelist")) ;
	printd("icon selected = "); printd(text_from_var(arg1)); printd("\n");
	gtk_clist_select_row ( clist, arg1, 0 ) ;
	gtk_clist_moveto ( clist, arg1, 0, 0.5, 0 ) ;
	//gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), 0 ) ;

	if ( //gnome_icon_list_get_items_per_line ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")) ) < arg1 &&
			gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 1 )
	{
		//printf("FIXME: if gnome_icon_list_moveto priv->lines != NULL\n");
		gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), arg1, 0.5 ) ;
	}

	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )) ) == 1 &&
		event != NULL )
	{
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
		if ( gtk_clist_get_pixmap ( clist, arg1, 0, NULL, NULL ) )
		{
			//gtk_widget_show ( FullscreenWindow ) ;
			on_vmbutton2_clicked ( NULL, MainWindow ) ;
			//on_vmbutton1_clicked ( NULL, MainWindow ) ;
		}
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), arg1 ) ;
		gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), arg1, 0.5 ) ;
		refresh_screen() ;
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), TRUE ) ;
		//refresh_screen() ;
	} else if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )) ) == 0 &&
					GTK_WIDGET_SENSITIVE( lookup_widget(MainWindow,"handlebox1")) == TRUE ) {
		gtk_widget_set_sensitive ( lookup_widget(MainWindow,"handlebox1"), FALSE ) ;
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
	} else if ( gtk_clist_get_pixmap ( GTK_CLIST(lookup_widget(MainWindow,"imagelist")), 0, 0, NULL, NULL ) ) {
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

	printd("gnomeiconlist got size allocate!\n");

	window_width = lookup_widget(MainWindow,"iconlist")->allocation.width ;
	thumb_count = ( window_width - fmod ( window_width, max_thumb_size ) ) / max_thumb_size ;
	thumb_col_spacing = fmod ( window_width, max_thumb_size ) / thumb_count ;
	//thumb_width = window_width / thumb_count - thumb_col_spacing*2 ;

	//refresh_screen () ;
	
	printd("icon_width an thumbs anpassen\n");
	//printd("thumb_width = "); printd(text_from_var(thumb_width)); printd("\n");
	gnome_icon_list_set_icon_width ( gil, max_thumb_size ) ;
	printd("thumb_col_spacing = "); printd(text_from_var(thumb_col_spacing)); printd("\n");
	gnome_icon_list_set_col_spacing ( gil, thumb_col_spacing ) ;
	printd("set border of thumbs\n");
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
	
	printd("gnomeiconlist1 got size allocate!\n");
	
	clist = GTK_CLIST(lookup_widget(MainWindow,"imagelist")) ;
	
	sList = clist->selection;
	if ( sList ) 
	{
		rNum = (int) sList->data ;
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum ) ;
		if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 1 )
			gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum, 0.5 ) ;
		//return rNum ;
	} else {
		//return FALSE ;
	}
	
	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 0 )
		return ;

	window_width = lookup_widget(MainWindow,"iconlist1")->allocation.width ;
	thumb_count = ( window_width - fmod ( window_width, max_thumb_size ) ) / max_thumb_size ;
	thumb_col_spacing = fmod ( window_width, max_thumb_size ) / thumb_count ;
	//thumb_width = window_width / thumb_count - thumb_col_spacing*2 ;

	//refresh_screen () ;
	
	printd("icon_width an thumbs anpassen\n");
	//printd("thumb_width = "); printd(text_from_var(thumb_width)); printd("\n");
	gnome_icon_list_set_icon_width ( gil, max_thumb_size ) ;
	printd("thumb_col_spacing = "); printd(text_from_var(thumb_col_spacing)); printd("\n");
	gnome_icon_list_set_col_spacing ( gil, thumb_col_spacing ) ;
	printd("set border of thumbs\n");
	gnome_icon_list_set_icon_border ( gil, 5 ) ; // not yet supported :((
}

gboolean
on_MainWindow_key_press_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
	GList *sList;
	GtkCList *clist ;
	int rNum = 0 ;
	
	clist = GTK_CLIST(lookup_widget(MainWindow,"imagelist")) ;
	
	sList = clist->selection;
	if ( sList ) 
	{
		rNum = (int) sList->data ;
		//return rNum ;
	} else {
		rNum = -1 ;
		//return FALSE ;
	}
	
	printd("MainWindow_key_press_event!\n");

	if ( GTK_WIDGET_IS_SENSITIVE ( lookup_widget ( MainWindow, "handlebox1" ) ) == FALSE ) return FALSE ;
	
	printd("selected row in imagelist = "); printd(text_from_var(rNum)); printd("\n");
	if ( event->keyval == GDK_Up ) {
		printd("cursor up!\n");
		if ( rNum < 1 ) rNum = clist->rows ;
		if ( rNum > 0 )
		gtk_clist_select_row ( clist, rNum-1, 0 ) ;
		gtk_clist_moveto ( clist, rNum-1, 0, 0.5, 0 ) ;
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), rNum-1 ) ;
		if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) == 1 )
			gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), rNum-1, 0.5 ) ;
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum-1 ) ;
		if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 1 )
			gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum-1, 0.5 ) ;
	}
	if ( event->keyval == GDK_Down ) {
		printd("cursor down!\n");
		if ( rNum >= clist->rows-1 ) rNum = -1 ;
		gtk_clist_select_row ( clist, rNum+1, 0 ) ;
		gtk_clist_moveto ( clist, rNum+1, 0, 0.5, 0 ) ;
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), rNum+1 ) ;
		if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) == 1 )
			gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), rNum+1, 0.5 ) ;
		gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum+1 ) ;
		if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 1 )
			gnome_icon_list_moveto ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum+1, 0.5 ) ;
	}
	return TRUE;
}


gboolean
on_MainWindow_key_release_event        (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
	printd("MainWindow_key_release_event!\n");
	/*if ( event->keyval == GDK_Up )
		printd("cursor up!\n");
	if ( event->keyval == GDK_Down )
		printd("cursor down!\n");*/
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
	gtk_widget_show ( FullscreenWindow ) ;

	return TRUE;
}


void
on_fullscreenwindow_hide               (GtkWidget       *widget,
                                        gpointer         user_data)
{
	//view_image ( "__RELOAD__", MainWindow ) ;
	task_add_to_queue ( "display_in_window", NULL ) ;
	//refresh_screen () ;
}


void
on_fullscreenwindow_show               (GtkWidget       *widget,
                                        gpointer         user_data)
{
	GdkPixmap *gdkpixmap ;
	int fswidth = gdk_screen_width () ;
	int fsheight = gdk_screen_height () ;
	printd("show picture in own window...");
	
	gdkpixmap = gdk_pixmap_new ( lookup_widget( FullscreenWindow, "scrolledwindow6" )->window, 1, 1, -1 ) ;
	gtk_pixmap_set ( GTK_PIXMAP(lookup_widget(FullscreenWindow, "image")) , gdkpixmap, NULL ) ;
	gdk_pixmap_unref ( gdkpixmap ) ;
	
	gtk_widget_set_usize ( FullscreenWindow, fswidth, fsheight ) ;
	gtk_widget_set_uposition ( FullscreenWindow, 0, 0 ) ;
	
	if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) )
	{
		if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) == FALSE )
			gtk_widget_hide ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
	} else if ( GTK_WIDGET_VISIBLE ( lookup_widget(FullscreenWindow, "appbarbox") ) == FALSE ) {
		gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
	}
	
	refresh_screen () ;
	
	//view_image ( "__RELOAD__", FullscreenWindow ) ;
	task_add_to_queue ( "display_fullscreen", NULL ) ;
	task_add_to_queue ( "display_fullscreen", NULL ) ;
}

