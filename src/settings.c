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

#include <ctype.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#include <libgnomevfs/gnome-vfs.h>

#include <math.h>

#include "callbacks.h"
#include "settings.h"
#include "support.h"
#include "tasks.h"
#include "imagelist.h"


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
char extern *LATS_RC_TEMP_DIR ;
char extern *LATS_RC_THUMB_DIR ;
char extern *LATS_THUMB_DIR ;
int extern LATS_RC_THUMB_IN_HOME ;


GtkWidget extern *MainWindow ;
GtkWidget extern *PrefsWindow ;
GtkWidget extern *BgColorSelectionDialog ;


char
*get_config_value ( char *temp_option )
{
	char *line = NULL ;
	char option[2048] ;
	
	{
		int pos ;
		int length = strlen ( temp_option ) ;
		strcpy ( option, "" ) ;
		for ( pos = 0 ; ( pos < length && length >= 1 ) ; pos++ )
		{
			char sign[2] ;
			sign[0] = toupper(temp_option[pos]) ;
			sign[1] = '\0' ;
			strcat ( option, sign ) ;
		}
	}
	
	line = malloc ( sizeof ( char ) * 2048 ) ;
	
	sprintf ( line, "%s", option ) ;

	if ( strcmp ( option, "DEBUG" ) == 0 )
	{
		if ( DEBUG )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}
	
	if ( strcmp ( option, "LAST_PATH" ) == 0 )
	{
		char path[2048] ;
		struct stat buf ;
		GtkWidget *window, *parentwindow ;
		window = lookup_widget( MainWindow, "combo_entry1" );
		parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
		strcpy ( path, gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ) ;
		if ( stat ( gnome_vfs_expand_initial_tilde(path), &buf ) == 0 )
			sprintf ( line, "%s=%s", line, path ) ;
	}
	
	if ( strcmp ( option, "LAST_FILE" ) == 0 )
	{
		char *filename = NULL ;
		GtkCList *imagestatsclist;
		struct stat buf ;
		imagestatsclist = GTK_CLIST(lookup_widget( MainWindow, "imagestatsclist" ));
		gtk_clist_get_text ( imagestatsclist, 0, 1, &filename );
		if ( stat ( filename, &buf ) == 0 )
			sprintf ( line, "%s=%s", line, filename ) ;
	}

	if ( strcmp ( option, "SPIDER_USE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "spider" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "RENDER_QUALITY" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, render_quality ) ;
	}

	if ( strcmp ( option, "PDF_AND_POSTSCRIPT_DPI" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, gtk_spin_button_get_value_as_int( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton9" ) ) ) ) ;
	}

	if ( strcmp ( option, "AUTOMATIC_ASPECT_CORRECTION" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, gtk_spin_button_get_value_as_int( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton8" ) ) ) ) ;
	}

	if ( strcmp ( option, "MONITOR_DPI_USE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimageradiobutton6" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "MONITOR_DPI_X" ) == 0 )
	{
		sprintf ( line, "%s=%f", line, gtk_spin_button_get_value_as_float( 
						GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttonx" ) ) ) ) ;
	}

	if ( strcmp ( option, "MONITOR_DPI_Y" ) == 0 )
	{
		sprintf ( line, "%s=%f", line, gtk_spin_button_get_value_as_float( 
						GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttony" ) ) ) ) ;
	}

	if ( strcmp ( option, "CHECK_COLOR_A" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, check_color_a ) ;
	}

	if ( strcmp ( option, "CHECK_COLOR_OUT_A" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, check_color_out_a ) ;
	}

	if ( strcmp ( option, "CHECK_COLOR_B" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, check_color_b ) ;
	}

	if ( strcmp ( option, "CHECK_COLOR_OUT_B" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, check_color_out_b ) ;
	}

	if ( strcmp ( option, "BG_COLOR_DETECTION_USE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagecheckbutton1" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "BG_COLOR_DETECTION_MODE" ) == 0 )
	{
		int check_color_auto_routine = 4 ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgradiobutton1" )) ) )
			check_color_auto_routine = 1 ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgradiobutton2" )) ) )
			check_color_auto_routine = 2 ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgradiobutton3" )) ) )
			check_color_auto_routine = 3 ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagebgradiobutton4" )) ) )
			check_color_auto_routine = 4 ;
		sprintf ( line, "%s=%d", line, check_color_auto_routine ) ;
	}

	if ( strcmp ( option, "BG_COLOR_DETECTION_DENSITY" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, gtk_spin_button_get_value_as_int( 
						GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton6" ) ) ) ) ;
	}

	if ( strcmp ( option, "BG_COLOR_DETECTION_ALTER_AVERAGE_VALUE" ) == 0 )
	{
		sprintf ( line, "%s=%f", line, gtk_spin_button_get_value_as_float( 
						GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton7" ) ) ) ) ;
	}

	if ( strcmp ( option, "BG_COLOR_DETECTION_CHECKERS_USE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "BG_COLOR_USE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_RED" ) == 0 )
	{
		gdouble color[4] ;
		gtk_color_selection_get_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
		sprintf ( line, "%s=%d", line, (int) ( 256 * color[0] ) ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_GREEN" ) == 0 )
	{
		gdouble color[4] ;
		gtk_color_selection_get_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
		sprintf ( line, "%s=%d", line, (int) ( 256 * color[1] ) ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_BLUE" ) == 0 )
	{
		gdouble color[4] ;
		gtk_color_selection_get_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
		sprintf ( line, "%s=%d", line, (int) ( 256 * color[2] ) ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_ALPHA" ) == 0 )
	{
		gdouble color[4] ;
		gtk_color_selection_get_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
		sprintf ( line, "%s=%d", line, (int) ( 256 * color[3] ) ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_FADE_TO_GREY_SPEED" ) == 0 )
	{
		sprintf ( line, "%s=%f", line, gtk_spin_button_get_value_as_float( 
						GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton5" ) ) ) ) ;
	}
	
	if ( strcmp ( option, "BG_TILES_USE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgtiles" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "BG_BORDER_FADE_OUT" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagecheckbutton2" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "ZOOM" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, 
					gtk_spin_button_get_value_as_int ( 
						GTK_SPIN_BUTTON(lookup_widget( MainWindow, "zoomentry" )) ) ) ;
	}

	if ( strcmp ( option, "ZOOM_REAL_USE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "realzoom" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "ZOOM_FIT_WIDTH" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "autozoomwidth" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "ZOOM_FIT_HEIGHT" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "autozoomheight" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "ZOOM_KEEP_ASPECT" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "keepaspect" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "FULLSCREENMODE_HIDE_STATUSBAR" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagecheckbutton3" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "FULLSCREENMODE_SHOW_STATUSBAR_WHILE_LOADING" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagecheckbutton4" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "THUMB_WHERE_TO_SAVE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsthumbradiobutton6" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsthumbradiobutton5" )) ) )
			sprintf ( line, "%s=%d", line, 2 ) ;
	}
	
	if ( strcmp ( option, "THUMB_DIR_RELATIVE_TO_RC_DIR" ) == 0 )
	{
		char path[2048] ;
		GtkWidget *window, *parentwindow ;
		window = lookup_widget( PrefsWindow, "prefsthumbentry1" );
		parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
		strcpy ( path, gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ) ;
		sprintf ( line, "%s=%s", line, path ) ;
	}

	if ( strcmp ( option, "THUMB_DIR_RELATIVE_TO_ACTUAL_DIR" ) == 0 )
	{
		char path[2048] ;
		GtkWidget *window, *parentwindow ;
		window = lookup_widget( PrefsWindow, "prefsthumbentry2" );
		parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
		strcpy ( path, gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ) ;
		sprintf ( line, "%s=%s", line, path ) ;
	}

	if ( strcmp ( option, "THUMB_RENDER_QUALITY" ) == 0 )
	{
		sprintf ( line, "%s=%d", line, thumb_render_quality ) ;
	}

	if ( strcmp ( option, "FILE_FILTER_USE" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton1" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}

	if ( strcmp ( option, "HIDE_LEADING_DOT_FILES" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton2" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}
	if ( strcmp ( option, "HIDE_NON_MULTIMEDIA_CONTENT" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton3" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}
	if ( strcmp ( option, "HIDE_AUDIO_FILES" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton4" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}
	if ( strcmp ( option, "HIDE_IMAGE_FILES" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton5" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}
	if ( strcmp ( option, "HIDE_MOVIE_FILES" ) == 0 )
	{
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton6" )) ) )
			sprintf ( line, "%s=%d", line, 1 ) ;
		else
			sprintf ( line, "%s=%d", line, 0 ) ;
	}
	
	if ( strcmp ( option, line ) ) {
		printd("cf: write option = '"); printd(line); printd("'\n");
	} else {
		printd("cf: write option = '"); printd(option); printd("'\n");
	}

	sprintf ( line, "%s\n", line ) ;
	
	return line ;
}


void
use_config_line ( char *option, char *value )
{
	struct stat buf ;

	if ( strcmp ( option, "DEBUG" ) == 0 )
	{
		DEBUG = atoi(value) ? TRUE : FALSE ;
	}

	if ( strcmp ( option, "LAST_PATH" ) == 0 && stat ( gnome_vfs_expand_initial_tilde(value), &buf ) == 0 )
	{
		GtkWidget *window, *parentwindow ;
		window = lookup_widget( MainWindow, "combo_entry1" );
		parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
		gtk_entry_set_text( GTK_ENTRY( parentwindow ), value );
		if ( lookup_widget( MainWindow, "imageinfo" )->allocation.width > 1 )
		{
			// load new directory-entries
			read_dir_from_combo ( FALSE, MainWindow );
		}
	}
	
	if ( strcmp ( option, "LAST_FILE" ) == 0 )
	{
		char filename[2048] ;
		GtkWidget *window, *parentwindow ;
		window = lookup_widget( MainWindow, "combo_entry1" );
		parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
		strcpy ( filename, gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ) ;
		if ( filename [ strlen(filename) - 1 ] != 47 )
			strcat ( filename, "/" ) ;
		strcat ( filename, value ) ;
		
		if ( stat ( gnome_vfs_expand_initial_tilde(filename) , &buf ) == 0 )
			task_add_to_queue ( "load_image", value ) ;
		else
			gtk_widget_activate ( lookup_widget ( MainWindow, "vmbutton1" ) ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_USE" ) == 0 )
	{
		if ( atoi(value) )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )), TRUE ) ;
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )), FALSE ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_RED" ) == 0 )
	{
		gdouble color[4] ;
		gtk_color_selection_get_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
		color[0] = (double) 1 / (double) 256 * (double) atoi(value) ;
		gtk_color_selection_set_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_GREEN" ) == 0 )
	{
		gdouble color[4] ;
		gtk_color_selection_get_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
		color[1] = (double) 1 / (double) 256 * (double) atoi(value) ;
		gtk_color_selection_set_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_BLUE" ) == 0 )
	{
		gdouble color[4] ;
		gtk_color_selection_get_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
		color[2] = (double) 1 / (double) 256 * (double) atoi(value) ;
		gtk_color_selection_set_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_ALPHA" ) == 0 )
	{
		gdouble color[4] ;
		gtk_color_selection_get_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
		color[3] = (double) 1 / (double) 256 * (double) atoi(value) ;
		gtk_color_selection_set_color	( GTK_COLOR_SELECTION(
											GTK_COLOR_SELECTION_DIALOG(
												BgColorSelectionDialog)->colorsel), color ) ;
	}
	
	if ( strcmp ( option, "BG_TILES_USE" ) == 0 )
	{
		if ( atoi(value) )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgtiles" )), TRUE ) ;
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgtiles" )), FALSE ) ;
	}

	if ( strcmp ( option, "ZOOM" ) == 0 )
	{
		if ( atoi(value) != 100 )
			gtk_spin_button_set_value( GTK_SPIN_BUTTON(lookup_widget( MainWindow, "zoomentry" )), atoi(value) );
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "zoom100" )), TRUE ) ;
	}

	if ( strcmp ( option, "ZOOM_REAL_USE" ) == 0 )
	{
		if ( atoi(value) )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "realzoom" )), TRUE ) ;
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "realzoom" )), FALSE ) ;		
	}

	if ( strcmp ( option, "ZOOM_FIT_WIDTH" ) == 0 )
	{
		if ( atoi(value) )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "autozoomwidth" )), TRUE ) ;
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "autozoomwidth" )), FALSE ) ;
	}

	if ( strcmp ( option, "ZOOM_FIT_HEIGHT" ) == 0 )
	{
		if ( atoi(value) )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "autozoomheight" )), TRUE ) ;
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "autozoomheight" )), FALSE ) ;
	}

	if ( strcmp ( option, "ZOOM_KEEP_ASPECT" ) == 0 )
	{
		if ( atoi(value) )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "keepaspect" )), TRUE ) ;
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "keepaspect" )), FALSE ) ;
	}

	if ( strcmp ( option, "SPIDER_USE" ) == 0 )
	{
		if ( atoi(value) )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "spider" )), TRUE ) ;
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "spider" )), FALSE ) ;
	}

	if ( strcmp ( option, "RENDER_QUALITY" ) == 0 )
	{
		if ( atoi(value) == 0 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimageradiobutton1" )), TRUE ) ;
		if ( atoi(value) == 1 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimageradiobutton2" )), TRUE ) ;
		if ( atoi(value) == 2 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimageradiobutton3" )), TRUE ) ;
		if ( atoi(value) == 3 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimageradiobutton4" )), TRUE ) ;
	}

	if ( strcmp ( option, "PDF_AND_POSTSCRIPT_DPI" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton9" ) ), atof(value) ) ;
	}

	if ( strcmp ( option, "AUTOMATIC_ASPECT_CORRECTION" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton8" ) ), atof(value) ) ;
	}

	if ( strcmp ( option, "MONITOR_DPI_USE" ) == 0 )
	{
		if ( atoi(value) == 0 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimageradiobutton5" )), TRUE ) ;
		else
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimageradiobutton6" )), TRUE ) ;
	}
	
	if ( strcmp ( option, "MONITOR_DPI_X" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttonx" ) ), atof(value) ) ;
	}

	if ( strcmp ( option, "MONITOR_DPI_Y" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "prefsimagedpispinbuttony" ) ), atof(value) ) ;
	}
	
	if ( strcmp ( option, "CHECK_COLOR_A" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton1" ) ), atof(value) ) ;
	}
	
	if ( strcmp ( option, "CHECK_COLOR_OUT_A" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton3" ) ), atof(value) ) ;
	}
	
	if ( strcmp ( option, "CHECK_COLOR_B" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton2" ) ), atof(value) ) ;
	}
	
	if ( strcmp ( option, "CHECK_COLOR_OUT_B" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton4" ) ), atof(value) ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_DETECTION_USE" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagecheckbutton1" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_DETECTION_MODE" ) == 0 )
	{
		if ( atoi(value) == 1 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagebgradiobutton1" )), TRUE ) ;
		if ( atoi(value) == 2 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagebgradiobutton2" )), TRUE ) ;
		if ( atoi(value) == 3 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagebgradiobutton3" )), TRUE ) ;
		if ( atoi(value) == 4 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagebgradiobutton4" )), TRUE ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_DETECTION_DENSITY" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton6" ) ), atoi(value) ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_DETECTION_ALTER_AVERAGE_VALUE" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton7" ) ), atof(value) ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_DETECTION_CHECKERS_USE" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagebgcheckbutton1" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "BG_COLOR_FADE_TO_GREY_SPEED" ) == 0 )
	{
		gtk_spin_button_set_value( 
								GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton5" ) ), atof(value) ) ;
	}
	
	if ( strcmp ( option, "BG_BORDER_FADE_OUT" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagecheckbutton2" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "FULLSCREENMODE_HIDE_STATUSBAR" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagecheckbutton3" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "FULLSCREENMODE_SHOW_STATUSBAR_WHILE_LOADING" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsimagecheckbutton4" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "THUMB_WHERE_TO_SAVE" ) == 0 )
	{
		if ( atoi(value) == 1 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsthumbradiobutton6" )), TRUE ) ;
		if ( atoi(value) == 2 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsthumbradiobutton5" )), TRUE ) ;
	}
	
	if ( strcmp ( option, "THUMB_DIR_RELATIVE_TO_RC_DIR" ) == 0 )
	{
		GtkWidget *window, *parentwindow ;
		window = lookup_widget( PrefsWindow, "prefsthumbentry1" );
		parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
		gtk_entry_set_text( GTK_ENTRY( parentwindow ), value );
	}
	
	if ( strcmp ( option, "THUMB_DIR_RELATIVE_TO_ACTUAL_DIR" ) == 0 )
	{
		GtkWidget *window, *parentwindow ;
		window = lookup_widget( PrefsWindow, "prefsthumbentry2" );
		parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
		gtk_entry_set_text( GTK_ENTRY( parentwindow ), value );
	}
	
	if ( strcmp ( option, "THUMB_RENDER_QUALITY" ) == 0 )
	{
		if ( atoi(value) == 0 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsthumbradiobutton1" )), TRUE ) ;
		if ( atoi(value) == 1 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsthumbradiobutton2" )), TRUE ) ;
		if ( atoi(value) == 2 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsthumbradiobutton3" )), TRUE ) ;
		if ( atoi(value) == 3 )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsthumbradiobutton4" )), TRUE ) ;
	}
	
	if ( strcmp ( option, "FILE_FILTER_USE" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton1" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "HIDE_LEADING_DOT_FILES" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton2" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "HIDE_NON_MULTIMEDIA_CONTENT" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton3" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "HIDE_AUDIO_FILES" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton4" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "HIDE_IMAGE_FILES" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton5" )), atoi(value) ? TRUE : FALSE ) ;
	}
	
	if ( strcmp ( option, "HIDE_MOVIE_FILES" ) == 0 )
	{
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget( PrefsWindow, "prefsfiltercheckbutton6" )), atoi(value) ? TRUE : FALSE ) ;
	}

}


void
settings_load_all ( void )
{
	FILE *stream ;
	char config_filename[2048] ;
	
	gtk_widget_realize ( MainWindow ) ;
	gtk_widget_realize ( PrefsWindow ) ;
	gtk_widget_realize ( BgColorSelectionDialog ) ;
	
	strcpy ( config_filename, LATS_RC_DIR ) ;
	if ( config_filename [ strlen(config_filename) - 1 ] != 47 )
		strcat ( config_filename, "/" ) ;	
	strcat ( config_filename, LATS_RC_FILE ) ;
	
	printd("cf: looking for config-file called '");
	printd(config_filename);
	printd("'...\n");
	
	stream = fopen ( gnome_vfs_expand_initial_tilde(config_filename), "r" ) ;
	
	if ( stream )
	{
		char *line ;
		
		line = malloc ( sizeof (char) * 2048 ) ;
		
		printd("cf: configuration found! trying to read...\n");
		
		while ( fgets ( line, 2048, stream ) )
		{
			char option[2048], value[2048], current_line[2048] ;
			int pos = 0, finish = FALSE, length ;
			strcpy ( current_line, line ) ;
			strcpy ( option, "" ) ;
			strcpy ( value, "" ) ;
			length = strlen ( current_line ) - 1 ;
			// print what's been read so far..
			//printd("config read: ");printd(current_line);printd("\n");
			// now get the variable-name and value out of this string...
			printd("cf: read option = '");
			for ( ; ( pos < length && finish == FALSE ) ; pos++ )
			{
				char sign[2] ;
				sign[0] = toupper(current_line[pos]) ;
				sign[1] = '\0' ;
				if ( strcmp( sign, "=" ) )
				{
					strcat ( option, sign ) ;
					printd ( sign ) ;
				} else finish = TRUE ;
			}
			printd("' ; value = '");
			for ( ; pos < length ; pos++ )
			{
				char sign[2] ;
				sign[0] = current_line[pos] ;
				sign[1] = '\0' ;
				strcat ( value, sign ) ;
				printd ( sign ) ;
			}
			printd("\'\n");
			
			if ( strcmp ( value, "" ) )
			{
				// now try to use this...
				use_config_line ( option, value ) ;
			}
		}
		
		free ( line ) ;
		
		printd("cf: finished reading configuration.\n");
		fclose ( stream ) ;
	} else {
		printd("cf: no configuration found. using defaults.\n");
	}
}


void
settings_save_all ( void )
{
	FILE *stream ;
	char config_filename[2048] ;
	
	gtk_widget_realize ( MainWindow ) ;
	gtk_widget_realize ( PrefsWindow ) ;
	gtk_widget_realize ( BgColorSelectionDialog ) ;
	
	strcpy ( config_filename, LATS_RC_DIR ) ;
	if ( config_filename [ strlen(config_filename) - 1 ] != 47 )
		strcat ( config_filename, "/" ) ;	
	strcat ( config_filename, LATS_RC_FILE ) ;
	
	{
		char backup[2048] ;
		sprintf ( backup, "mv -f %s %s.bak", config_filename, config_filename ) ;
		printd("cf: backingup old configuration if exists...\n");
		system ( backup ) ;
	}
	
	printd("cf: creating config-file called '");
	printd(config_filename);
	printd("'...\n");
	
	stream = fopen ( gnome_vfs_expand_initial_tilde(config_filename), "w" ) ;
	
	
	if ( stream )
	{
		char title[2048] ;
		sprintf ( title, "-- Look At The Stars ( %s / compiled at %s )", VERSION, __DATE__ ) ;
		fputs ( get_config_value ( title ), stream ) ;
		fputs ( get_config_value ( "-- auto-generated configuration" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;
		
		fputs ( get_config_value ( "- Globals..." ), stream ) ;
		fputs ( get_config_value ( "DEBUG" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;

		fputs ( get_config_value ( "- Paths and files..." ), stream ) ;
		fputs ( get_config_value ( "LAST_PATH" ), stream ) ;
		fputs ( get_config_value ( "LAST_FILE" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;
		
		fputs ( get_config_value ( "- File finder..." ), stream ) ;
		fputs ( get_config_value ( "SPIDER_USE" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;

		fputs ( get_config_value ( "- Image related stuff" ), stream ) ;
		fputs ( get_config_value ( "RENDER_QUALITY" ), stream ) ;
		fputs ( get_config_value ( "PDF_AND_POSTSCRIPT_DPI" ), stream ) ;
		fputs ( get_config_value ( "AUTOMATIC_ASPECT_CORRECTION" ), stream ) ;
		fputs ( get_config_value ( "MONITOR_DPI_USE" ), stream ) ;
		fputs ( get_config_value ( "MONITOR_DPI_X" ), stream ) ;
		fputs ( get_config_value ( "MONITOR_DPI_Y" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;
		
		fputs ( get_config_value ( "- Background related stuff" ), stream ) ;
		fputs ( get_config_value ( "CHECK_COLOR_A" ), stream ) ;
		fputs ( get_config_value ( "CHECK_COLOR_OUT_A" ), stream ) ;
		fputs ( get_config_value ( "CHECK_COLOR_B" ), stream ) ;
		fputs ( get_config_value ( "CHECK_COLOR_OUT_B" ), stream ) ;
		
		fputs ( get_config_value ( "BG_COLOR_DETECTION_USE" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_DETECTION_MODE" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_DETECTION_DENSITY" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_DETECTION_ALTER_AVERAGE_VALUE" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_DETECTION_CHECKERS_USE" ), stream ) ;

		fputs ( get_config_value ( "BG_COLOR_USE" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_RED" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_GREEN" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_BLUE" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_ALPHA" ), stream ) ;
		fputs ( get_config_value ( "BG_COLOR_FADE_TO_GREY_SPEED" ), stream ) ;

		fputs ( get_config_value ( "BG_TILES_USE" ), stream ) ;
		fputs ( get_config_value ( "BG_BORDER_FADE_OUT" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;
		
		fputs ( get_config_value ( "- Zoom related stuff..." ), stream ) ;
		fputs ( get_config_value ( "ZOOM" ), stream ) ;
		fputs ( get_config_value ( "ZOOM_REAL_USE" ), stream ) ;
		fputs ( get_config_value ( "ZOOM_FIT_WIDTH" ), stream ) ;
		fputs ( get_config_value ( "ZOOM_FIT_HEIGHT" ), stream ) ;
		fputs ( get_config_value ( "ZOOM_KEEP_ASPECT" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;

		fputs ( get_config_value ( "- Misc..." ), stream ) ;
		fputs ( get_config_value ( "FULLSCREENMODE_HIDE_STATUSBAR" ), stream ) ;
		fputs ( get_config_value ( "FULLSCREENMODE_SHOW_STATUSBAR_WHILE_LOADING" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;

		fputs ( get_config_value ( "- Thumbnails..." ), stream ) ;
		fputs ( get_config_value ( "THUMB_WHERE_TO_SAVE" ), stream ) ;
		fputs ( get_config_value ( "THUMB_DIR_RELATIVE_TO_RC_DIR" ), stream ) ;
		fputs ( get_config_value ( "THUMB_DIR_RELATIVE_TO_ACTUAL_DIR" ), stream ) ;
		fputs ( get_config_value ( "THUMB_RENDER_QUALITY" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;

		fputs ( get_config_value ( "- File filter..." ), stream ) ;
		fputs ( get_config_value ( "FILE_FILTER_USE" ), stream ) ;
		fputs ( get_config_value ( "HIDE_LEADING_DOT_FILES" ), stream ) ;
		fputs ( get_config_value ( "HIDE_NON_MULTIMEDIA_CONTENT" ), stream ) ;
		fputs ( get_config_value ( "HIDE_AUDIO_FILES" ), stream ) ;
		fputs ( get_config_value ( "HIDE_IMAGE_FILES" ), stream ) ;
		fputs ( get_config_value ( "HIDE_MOVIE_FILES" ), stream ) ;
		fputs ( get_config_value ( " " ), stream ) ;


		printd("cf: finished writing configuration.\n");
		fclose ( stream ) ;
	} else {
		printd("cf: couldn't save configuration!.\n");
	}
}


