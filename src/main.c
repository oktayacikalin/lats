/*
 * 
 *    Look at the stars
 * 
 *   by TheBlackLion@web.de 
 *  alias Oktay Acikalin  :)
 * 
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

#include "settings.h" // these are for our settings load/save functions!

#include "look_at_the_stars_bg.xpm"

GdkPixbuf *loaded_image = NULL ; // the actual loaded image
GdkPixbuf *loaded_scaled_image = NULL ; // the scaled image in memory ( performance+ )

int DEBUG = FALSE ;
int thumbnails = TRUE ;
int thumb_size = 100 ;
int max_thumb_size = 100 ;
int no_thumb_size = 32 ;
int thumb_border = 2 ;
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
int filedisplay_as_icons = TRUE ;	// show mini-icons instead of image-list
int pics_per_row_on_sidebar = 2 ; // how many pics do you want to have in a row when using da'sidebar?
int image_mutlipage_fast_preview = TRUE ; // show loading-picture and count pages before converting?

int show_statusbar_in_fullscreenmode = TRUE ; // don't ask me what this does, ok? :)

int file_filter_use = TRUE ; // don't show every file
int file_filter_hide_dot_files = TRUE ; // don't show hidden files
int file_filter_hide_non_multimedia_files = TRUE ; // only show multimedia files
int file_filter_hide_images = FALSE ; // don't show images
int file_filter_hide_movies = FALSE ; // don't show movies
int file_filter_hide_audio = FALSE ; // don't show audio files

char *LATS_RC_DIR 			= "~/.lats/" ;
char *LATS_RC_FILE			= "config" ;
char *LATS_RC_TEMP_DIR		= "temp/" ;
char *LATS_RC_THUMB_DIR	 	= "thumbnails/" ;
char *LATS_THUMB_DIR		= ".thumbnails/" ;
int LATS_RC_THUMB_IN_HOME	= TRUE ;

int spider_max_dir_depth = 5 ; // how deep should the dirlist-spider look?

int Mouse_x = 0 ; // current horizontal mouse position
int Mouse_y = 0 ; // the vertical one

GtkWidget *MainWindow ;
GtkWidget *PrefsWindow ;
GtkWidget *CalibrateWindow ;
GtkWidget *BgColorSelectionDialog ;
GtkWidget *FullscreenWindow ;
GtkWidget *FullscreenWindowProgressbar ;
GtkWidget *ShutdownWindow ;

int
main (int argc, char *argv[])
{	
	struct stat buf ;
	char current_dir[2048] ;
		
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);
#endif

	g_thread_init ( NULL ) ;
	
	gnome_init ("look_at_the_stars", VERSION, argc, argv);

	// now do something useful :)
	
	getcwd ( current_dir, 2048 ) ; // get current directory..
	if ( strcmp ( current_dir, gnome_vfs_expand_initial_tilde("~") ) == 0 )
		strcpy ( current_dir, "~" ) ;
	// important when loading a picture with a relative path! see far below :)
	
	// create the RC_DIR
	printd("ma: check for RC_DIR... ");
	printd(LATS_RC_DIR) ; printd(" ... ");
	if ( stat( gnome_vfs_expand_initial_tilde ( LATS_RC_DIR ), &buf ) != 0 || !S_ISDIR(buf.st_mode) )
	{
		printd("has to be created first... ");
		if ( ! mkdir ( gnome_vfs_expand_initial_tilde(LATS_RC_DIR),  S_IRWXU ) ) {
			printd("done. :)\n");
		} else {
			printd("!!! could not be created!! :(\n");
		}
	} else printd("already created. :)\n");
	// create the RC_THUMB_DIR
	{
		char dir[2048] ;
		sprintf ( dir, "%s%s", LATS_RC_DIR, LATS_RC_THUMB_DIR ) ;
		printd("ma: check for RC_THUMB_DIR... ");
		printd(dir) ; printd(" ... ");
		if ( stat( gnome_vfs_expand_initial_tilde ( dir ), &buf ) != 0 || !S_ISDIR(buf.st_mode) )
		{
			printd("has to be created first... ");
			if ( ! mkdir ( gnome_vfs_expand_initial_tilde(dir),  S_IRWXU ) ) {
				printd("done. :)\n");
			} else {
				printd("!!! could not be created!! :(\n");
			}
		} else printd("already created. :)\n");
	}
	// create the RC_TEMP_DIR
	{
		char dir[2048] ;
		sprintf ( dir, "%s%s", LATS_RC_DIR, LATS_RC_TEMP_DIR ) ;
		printd("ma: check for RC_TEMP_DIR... ");
		printd(dir) ; printd(" ... ");
		if ( stat( gnome_vfs_expand_initial_tilde ( dir ), &buf ) != 0 || !S_ISDIR(buf.st_mode) )
		{
			printd("has to be created first... ");
			if ( ! mkdir ( gnome_vfs_expand_initial_tilde(dir),  S_IRWXU ) ) {
				printd("done. :)\n");
			} else {
				printd("!!! could not be created!! :(\n");
			}
		} else printd("already created. :)\n");
	}
	
	MainWindow = create_MainWindow ();
	PrefsWindow = create_prefswindow ();
	CalibrateWindow = create_calibratewindow ();
	BgColorSelectionDialog = create_colorselectiondialog1 () ; 
	FullscreenWindow = create_fullscreenwindow () ;
	FullscreenWindowProgressbar = create_fullscreenwindowprogressbar () ;
	ShutdownWindow = create_shutdownwindow () ;

	gtk_widget_realize ( MainWindow ) ;
	gtk_widget_realize ( PrefsWindow ) ;
	gtk_widget_realize ( CalibrateWindow ) ;
	gtk_widget_realize ( FullscreenWindow ) ;
	gtk_widget_realize ( BgColorSelectionDialog ) ;
	
	task_clear_queue () ;  // setup tasklist for queueing..
	
	gtk_color_selection_set_opacity ( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), TRUE ) ;
	gtk_widget_hide ( GTK_WIDGET(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->help_button) ) ;

	//gtk_widget_hide ( lookup_widget ( MainWindow, "appbar0" ) ) ;
	
	if ( show_statusbar_in_fullscreenmode == FALSE )
		gtk_widget_hide ( lookup_widget ( FullscreenWindow, "appbarbox" ) ) ;
	
	add_zoom_buttons_to_toolbar ( ) ;
	add_vmbutton_series_to_toolbar ( 1, FALSE ) ;
	add_vmbutton_series_to_toolbar ( 2, TRUE ) ;
	add_vmbutton_series_to_toolbar ( 3, FALSE ) ;
	
	add_spider_to_toolbar ( ) ;

	if ( hide_unusable_view_buttons == TRUE )
	{
		vmbutton_show ( 1 ) ; vmbutton_hide ( 2 ) ; vmbutton_show ( 3 ) ;
	}
	
	gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imagedisplaynb" )), FALSE ) ;
	gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(lookup_widget(MainWindow, "frame1nb" )), FALSE ) ;
	
	if ( filedisplay_as_icons == TRUE ) {
		//gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "frame1nb" )), 1 ) ;
		gtk_widget_activate ( lookup_widget(MainWindow,"frame1radiobutton2") ) ;
	} else {
		//gtk_notebook_set_page ( GTK_NOTEBOOK(lookup_widget(MainWindow, "frame1nb" )), 0 ) ;
		gtk_widget_activate ( lookup_widget(MainWindow,"frame1radiobutton1") ) ;
	}
	
	gtk_notebook_set_homogeneous_tabs ( GTK_NOTEBOOK(lookup_widget(MainWindow, "imageinfonotebook" )), FALSE ) ;
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"imagedisplay"), FALSE ) ;
	
	{	// let's set up the backgroundcolorselectiondialog!
		gdouble color[3] ;
		color[0] = 1 ; color[1] = 1 ; color[2] = 1 ; color[3] = 1 ;
		gtk_color_selection_set_color	( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), color);
	}

	on_vmbutton2_clicked ( NULL, MainWindow ) ;
	
	
	// now load our settings!
	settings_load_all () ;


	// do we want verbose debug messages?
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsmaintogglebutton1" )), DEBUG ) ;
	DEBUG = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsmaintogglebutton1" )) ) ;
	
	//gtk_widget_show (MainWindow);
	
	//gtk_widget_set_usize ( lookup_widget(MainWindow,"appbarbox"), -1, lookup_widget(MainWindow,"appbar1")->allocation.height ) ;
	//gtk_widget_set_usize ( lookup_widget(FullscreenWindow,"appbarbox"), -1, lookup_widget(MainWindow,"appbar1")->allocation.height - 6 ) ;

	//refresh_screen () ;

	// let's set up some nifty cursor-pixmap when the mouse touches the image, the imagelist, the iconlist etc.
	set_cursor_for_widget ( lookup_widget ( MainWindow, "image" ), 2 ) ;
	set_cursor_for_widget ( lookup_widget ( MainWindow, "imagelist" ), 1 ) ;
	set_cursor_for_widget ( lookup_widget ( MainWindow, "iconlist" ), 1 ) ;
	set_cursor_for_widget ( lookup_widget ( MainWindow, "iconlist1" ), 1 ) ;
	set_cursor_for_widget ( lookup_widget ( MainWindow, "iconlistmp" ), 1 ) ;
	set_cursor_for_widget ( lookup_widget ( MainWindow, "dirlist" ), 1 ) ;
	
	// setup sidebar for choosing a page in a multi-page-document
	//gtk_widget_set_usize ( lookup_widget ( MainWindow, "scrolledwindowmp" ), max_thumb_size + 65, -1 ) ;
	gtk_widget_hide ( lookup_widget ( MainWindow, "multipagebox" ) ) ;	
	
	//on_vmbutton1_clicked ( NULL, MainWindow ) ;

	if ( loaded_image ) gdk_pixbuf_unref( loaded_image ) ;
	loaded_image = gdk_pixbuf_new_from_xpm_data ( (const char**) look_at_the_stars_bg_xpm ) ;

	if ( argc > 1 ) // arguments?
	{
		if ( isdir( argv[1] ) ) // argument is a dir
		{
			printd ( "ma: change to dir = " ) ; printd ( argv[1] ) ; printd ( "\n" ) ;
			chdir ( argv[1] ) ;
			gtk_entry_set_text( GTK_ENTRY( gtk_widget_get_ancestor (
								lookup_widget( MainWindow, "combo_entry1" ), 
								GTK_TYPE_EDITABLE) ), argv[1] );
			if ( argc > 2 && isimage ( argv[2] ) ) // plus an image?
			{
				printd ( "ma: view image = " ) ; printd ( argv[2] ) ; printd ( "\n" ) ;
				on_vmbutton2_clicked ( NULL, MainWindow ) ;
				refresh_screen () ;
				task_add_to_queue ( "load_image", argv[2] ) ;
			} else on_vmbutton1_clicked ( NULL, MainWindow ) ;
		} else /*if ( isimage ( argv[1] ) )*/ { // argument is a pic
			char wholefilename[2048], filename[2048], filepath[2048] ;
			int i = 0, pos = 0 ;
			strcpy ( wholefilename, argv[1] ) ;
			strcpy ( filename, "" ) ;
			strcpy ( filepath, "" ) ;
			printd("ma: you want me to load "); printd ( wholefilename ) ; printd ( "\n" ) ;
			for ( i = 0 ; i < strlen(wholefilename) ; i++ )
			{
				if ( wholefilename[i] == 47 ) pos = i ;
			}
			strncat ( filepath, wholefilename, pos ) ; 
			
			if ( filepath[0] != 47 && filepath[1] != 47 )
			{
				char temp_path[2048] ;
				sprintf ( temp_path, "%s/%s", current_dir, filepath ) ;
				strcpy ( filepath, temp_path ) ;
			}
			
			if ( strlen ( filepath ) )
			{
				printd("ma: we do have a new path...\n");
				if ( filepath[strlen(filepath)-1] != 47 ) strcat ( filepath, "/" ) ;
				printd("ma: get filename out of argument\n");
				strcpy ( filename, (rindex(wholefilename,47)) ? (rindex ( wholefilename, 47 )+1) : wholefilename ) ;
			} else {
				printd("ma: we get the current path...\n");
				getcwd ( filepath, 2048 ) ;
				printd("ma: copy filename out of argument\n");
				strcpy ( filename, wholefilename ) ;
			}
			printd("ma: the filepath is "); printd( filepath ) ; printd ( "\n" );
			printd("ma: the filename is "); printd( filename ) ; printd ( "\n" );
			printd("ma: we'll go to this first...\n");
			chdir ( gnome_vfs_expand_initial_tilde(filepath) ) ;
			printd("ma: now enter the path into the gui\n");
			gtk_entry_set_text( GTK_ENTRY( gtk_widget_get_ancestor (
								lookup_widget( MainWindow, "combo_entry1" ), 
								GTK_TYPE_EDITABLE) ), filepath );
			printd("ma: now we try to load the image \nma: ");
			printd(filename);
			printd("ma: ...\n");
			printd ( "ma: view image = " ) ; printd ( filename ) ; printd ( "\n" ) ;
			on_vmbutton3_clicked ( NULL, MainWindow ) ;
			
			refresh_screen() ;
			task_add_to_queue ( "load_image", filename ) ;
		}/* else { // got nothing useful as arguments...
			printd ( "ma: no usable startup arguments...\n" ) ;
			//on_vmbutton2_clicked ( NULL, MainWindow ) ;
			refresh_screen () ;
		}*/
	} else { // got nothing useful as arguments...
		printd ( "ma: no usable startup arguments...\n" ) ;
		//on_vmbutton2_clicked ( NULL, MainWindow ) ;
		refresh_screen () ;
		read_dir_from_combo ( FALSE, MainWindow );
	}
	
	gtk_widget_set_sensitive ( lookup_widget(MainWindow,"imagedisplay"), TRUE ) ;
	
	gtk_widget_show (MainWindow);
	
	gtk_widget_set_usize ( lookup_widget(MainWindow,"appbarbox"), -1, lookup_widget(MainWindow,"appbar1")->allocation.height ) ;
	//gtk_widget_set_usize ( lookup_widget(FullscreenWindow,"appbarbox"), -1, lookup_widget(MainWindow,"appbar1")->allocation.height - 6 ) ;

	// try to resize our fullscreenwindow to max size possible... -> fullscreen, you know? :)
	gtk_widget_set_usize ( FullscreenWindow, gdk_screen_width (), gdk_screen_height () ) ;
	gtk_widget_set_uposition ( FullscreenWindow, gdk_screen_width(), gdk_screen_height() ) ;
	gtk_widget_show (FullscreenWindow);
	gdk_window_set_transient_for ( FullscreenWindow->window, MainWindow->window ) ;
	gtk_widget_hide (FullscreenWindow);
	gtk_widget_set_uposition ( FullscreenWindow, 0, 0 ) ;
	//gtk_widget_hide ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
	gtk_widget_set_usize ( FullscreenWindowProgressbar, gdk_screen_width (), lookup_widget(MainWindow,"appbar1")->allocation.height - 6 ) ;
	gtk_widget_set_uposition ( FullscreenWindowProgressbar, 0, gdk_screen_height () - lookup_widget(MainWindow,"appbar1")->allocation.height + 6 ) ;

	refresh_screen () ;

	check_for_initial_directory_read () ;

	start_image_server () ; // lets start our image server!
		
	gtk_main ();

	return 0;
}

