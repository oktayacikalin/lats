/*
 * Anfängliche Datei main.c, erzeugt durch Glade. Bearbeiten Sie
 * Sie, wie Sie wollen. Glade wird diese Datei nicht überschreiben.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <sys/stat.h>
#include <dirent.h> 

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libgnomevfs/gnome-vfs.h>

#include <pthread.h>

#include "interface.h"
#include "support.h"
#include "callbacks.h"
#include "imagelist.h"
#include "imageview.h"
#include "toolbars.h"
#include "tasks.h"

#include "look_at_the_stars_bg.xpm"

GdkPixbuf *loaded_image = NULL ; // the actual loaded image
GdkPixbuf *loaded_scaled_image = NULL ; // the scaled image in memory ( performance+ )

int DEBUG = TRUE ;
int thumbnails = TRUE ;
int thumb_size = 100 ;
int max_thumb_size = 100 ;
int no_thumb_size = 32 ;
int thumb_render_quality = GDK_INTERP_HYPER ;
int render_quality = GDK_INTERP_BILINEAR ;
int check_size = 16 ;
guint32 check_color_a = 170 ; // will be grey! 0xaa
guint32 check_color_b = 85 ; // will be grey! 0x55
guint32 check_color_out_a = 34 ; // will be substracted from check_color_a 0x22
guint32 check_color_out_b = 34 ; // will be substracted from check_color_b 0x22
float check_color_out_cut_off_factor = 1 ;
int check_color_out_use = TRUE ;
int check_color_auto = TRUE ;
int check_color_auto_routine = 4 ;
int hide_unusable_view_buttons = TRUE ; 	// if TRUE we only see 2 usable buttons - 
																// the hidden one has no function anyway :)
int filedisplay_as_icons = FALSE ;	// show mini-icons instead of image-list
int pics_per_row_on_sidebar = 2 ; // how many pics do you want to have in a row when using da'sidebar?

int show_statusbar_in_fullscreenmode = TRUE ; // don't ask me what this does, ok? :)

char *LATS_RC_DIR 			= "~/.lats/" ;
char *LATS_RC_FILE			= "config" ;
char *LATS_RC_THUMB_DIR	 	= "thumbnails/" ;
char *LATS_THUMB_DIR		= ".thumbnails/" ;
int LATS_RC_THUMB_IN_HOME	= TRUE ;

GtkWidget *MainWindow ;
GtkWidget *PrefsWindow ;
GtkWidget *BgColorSelectionDialog ;
GtkWidget *FullscreenWindow ;
GtkWidget *ShutdownWindow ;

int
main (int argc, char *argv[])
{	
	struct stat buf ;
		
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);
#endif

	g_thread_init ( NULL ) ;
	
	gnome_init ("look_at_the_stars", VERSION, argc, argv);

	/*
	 * The following code was added by Glade to create one of each component
 	 * (except popup menus), just so that you see something after building
	 * the project. Delete any components that you don't want shown initially.
	*/
	
	stat ( gnome_vfs_expand_initial_tilde ( LATS_RC_DIR ), &buf );
	if ( ! S_ISDIR(buf.st_mode) )
	{
		printd(LATS_RC_DIR); printd(" muss erst noch erstellt werden... ");
		if ( ! mkdir ( gnome_vfs_expand_initial_tilde(LATS_RC_DIR),  S_IRWXU ) ) {
			printd("ok.\n");
		} else {
			printd("!!! "); printd(gnome_vfs_expand_initial_tilde(LATS_RC_DIR)); printd(" konnte nicht erstellt werden!!\n");
		}
	}
	
	task_clear_queue () ;  // setup tasklist for queueing..
	
	MainWindow = create_MainWindow ();
	PrefsWindow = create_prefswindow ();
	BgColorSelectionDialog = create_colorselectiondialog1 () ; 
	FullscreenWindow = create_fullscreenwindow () ;
	ShutdownWindow = create_shutdownwindow () ;
	
	gtk_color_selection_set_opacity ( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), TRUE ) ;
	gtk_widget_hide ( GTK_WIDGET(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->help_button) ) ;

	gtk_widget_hide ( lookup_widget ( MainWindow, "appbar0" ) ) ;
	
	if ( show_statusbar_in_fullscreenmode == FALSE )
		gtk_widget_hide ( lookup_widget ( FullscreenWindow, "appbarbox" ) ) ;
	
	add_zoom_buttons_to_toolbar ( ) ;
	add_vmbutton_series_to_toolbar ( 1, FALSE ) ;
	add_vmbutton_series_to_toolbar ( 2, TRUE ) ;
	add_vmbutton_series_to_toolbar ( 3, FALSE ) ;

	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_show ( 1 ) ; vmbutton_hide ( 2 ) ; vmbutton_show ( 3 ) ;
	}
	
	gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), FALSE ) ;
	gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(lookup_widget(MainWindow, "frame1nb" )), FALSE ) ;
	
	if ( filedisplay_as_icons == TRUE ) {
		gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "frame1nb" )), 1 ) ;
	} else {
		gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "frame1nb" )), 0 ) ;
	}
	
	gtk_notebook_set_homogeneous_tabs ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imageinfonotebook" )), FALSE ) ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"imagedisplay"), FALSE ) ;
	
	gtk_widget_show (MainWindow);

	gtk_widget_set_usize ( lookup_widget(MainWindow,"appbarbox"), -1, lookup_widget(MainWindow,"appbar1")->allocation.height ) ;
	gtk_widget_set_usize ( lookup_widget(FullscreenWindow,"appbarbox"), -1, lookup_widget(MainWindow,"appbar1")->allocation.height - 6 ) ;

	//refresh_screen () ;
	
	{	// let's set up the backgroundcolorselectiondialog!
		gdouble color[3] ;
		color[0] = 1 ; color[1] = 1 ; color[2] = 1 ; color[3] = 1 ;
		gtk_color_selection_set_color	( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), color);
	}
	
	//on_vmbutton1_clicked ( NULL, MainWindow ) ;

	if ( loaded_image ) gdk_pixbuf_unref( loaded_image ) ;
	loaded_image = gdk_pixbuf_new_from_xpm_data ( (const char**) look_at_the_stars_bg_xpm ) ;

	if ( argc > 1 ) // Argumente?
	{
		if ( isdir( argv[1] ) ) // ein dir als Argument?
		{
			printd ( "change to dir = " ) ; printd ( argv[1] ) ; printd ( "\n" ) ;
			chdir ( argv[1] ) ;
			gtk_entry_set_text( GTK_ENTRY( gtk_widget_get_ancestor (lookup_widget( MainWindow, "combo_entry1" ), GTK_TYPE_EDITABLE) ), argv[1] );
			//on_vmbutton2_clicked ( NULL, MainWindow ) ;
			if ( argc > 2 && isimage ( argv[2] ) ) // und noch ein Bild dazu?
			{
				printd ( "view image = " ) ; printd ( argv[2] ) ; printd ( "\n" ) ;
				on_vmbutton2_clicked ( NULL, MainWindow ) ;
				refresh_screen () ;
				//view_image ( argv[2], MainWindow ) ;
				task_add_to_queue ( "load_image", argv[2] ) ;
			} else on_vmbutton1_clicked ( NULL, MainWindow ) ;
			//refresh_screen () ;
			//read_dir_from_combo ( FALSE, MainWindow );
		} else if ( isimage ( argv[1] ) ) { // ein Bild als Argument?
			char wholefilename[2048], filename[2048], filepath[2048] ;
			int i = 0, pos = 0 ;
			strcpy ( wholefilename, argv[1] ) ;
			strcpy ( filename, "" ) ;
			strcpy ( filepath, "" ) ;
			printd("you want me to load "); printd ( wholefilename ) ; printd ( "\n" ) ;
			for ( i = 0 ; i < strlen(wholefilename) ; i++ )
			{
				if ( wholefilename[i] == 47 ) pos = i ;
			}
			strncat ( filepath, wholefilename, pos ) ; strcat ( filepath, "/" ) ;
			printd("the path is "); printd( filepath ) ; printd ( "\n" );
			printd("we'll go to this first...\n");
			chdir ( filepath ) ;
			printd("now we try to load the image ");
			strcpy ( filename, rindex ( wholefilename, 47 )+1 ) ;
			printd(filename);
			printd("...\n");
			printd ( "view image = " ) ; printd ( filename ) ; printd ( "\n" ) ;
			on_vmbutton3_clicked ( NULL, MainWindow ) ;
			
			refresh_screen() ;
			//view_image ( filename, MainWindow ) ;
			task_add_to_queue ( "load_image", filename ) ;
			//refresh_screen () ;
			//read_dir_from_combo ( FALSE, MainWindow );
		} else { // nix nützliches als Argument...
			printd ( "no usable startup arguments...\n" ) ;
			on_vmbutton2_clicked ( NULL, MainWindow ) ;
			//view_image ( "__RELOAD__", MainWindow ) ;
			refresh_screen () ;
			read_dir_from_combo ( FALSE, MainWindow );
		}
	} else { // nix nützliches als Argument...
		printd ( "no usable startup arguments...\n" ) ;
		on_vmbutton2_clicked ( NULL, MainWindow ) ;
		//view_image ( "__RELOAD__", MainWindow ) ;
		refresh_screen () ;
		read_dir_from_combo ( FALSE, MainWindow );
	}
	
//	view_image ( "__UPDATE__", MainWindow ) ;

	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"imagedisplay"), TRUE ) ;
	
	refresh_screen () ;

	start_image_server () ; // lets start our image server!
		
	gtk_main ();

	return 0;
}

