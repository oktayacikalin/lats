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

#include <pthread.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "importedfuncs.h"
#include "imageview.h"
#include "colors.h"
#include "tasks.h"

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

int extern filedisplay_as_icons ;

GtkWidget extern *MainWindow ;
GtkWidget extern *PrefsWindow ;
GtkWidget extern *BgColorSelectionDialog ;
GtkWidget extern *FullscreenWindow ;


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
	GdkPixbuf *new_image ;
	char dimension[2048] ;
	GtkCList *imagestatsclist;
	gchar *imagestatsclist_entry[1];
	struct stat buf ;

	//update_screen () ;
	gdk_threads_leave () ;
	new_image = gdk_pixbuf_new_from_file ( filename );
	if ( loaded_image && new_image )
	{
		printd ("altes Bild aus dem Speicher freigeben\n");
		gdk_pixbuf_unref ( loaded_image ) ;
		printd ("lade Bild "); printd (filename); printd ("...\n");
		loaded_image = gdk_pixbuf_copy ( new_image ) ;
		gdk_pixbuf_unref ( new_image ) ;

		gdk_threads_enter () ;
	
		imagestatsclist = GTK_CLIST(lookup_widget( MainWindow, "imagestatsclist" ));
		printd(" - iamgestatsclist updaten -");
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
		/*if ( ! oldimage )
			on_vmbutton2_clicked ( NULL, user_data ) ;*/
	} else {
		gdk_threads_enter () ;
	}
	
	//update_screen () ;
}

void
view_image ( char *filename, gpointer user_data )
{
	GtkWidget *imagedisplay, *appbar2 ;
	GtkProgress *progress ;
	GtkCList *imagestatsclist;
	int reload = FALSE ;
	int force_reload = FALSE ;
	int aspect_correction = gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "keepaspect" )) ) ;

	if ( GTK_WIDGET_VISIBLE ( user_data ) == FALSE ) return ;
	
	appbar2 = lookup_widget ( user_data, "appbar2");
	progress = gnome_appbar_get_progress (GNOME_APPBAR (appbar2));
	
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
		//printd(" imagestatsclist get text\n");
		gtk_clist_get_text ( imagestatsclist, 0, 1, &filename );
		//printd(filename); printd("\n");
		reload = TRUE ;
	} 

	//printd("Bild "); printd(filename);
	
	//gtk_progress_set_value (progress, 0 );

	if ( reload == FALSE || loaded_image == NULL || force_reload == TRUE ) 
	{
		int fold_out = FALSE ; // if we need to switch to splitted view... 
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Lade Bild aus Datei..."));
		update_screen () ;
		if ( GTK_WIDGET_VISIBLE(lookup_widget(MainWindow, "imagedisplay")) == FALSE && loaded_image == NULL )
			fold_out = TRUE ; // so we didn't load an image yet...
		//load_image ( filename );
		if ( loaded_image ) 
		{
			// so it worked? :)
			if ( fold_out == TRUE ) // do we need to switch to splitted view mode before displaying da image?
				on_vmbutton2_clicked ( NULL, MainWindow ) ;
			gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Bild geladen... berechne Dimensionen..."));
			gtk_progress_set_value (progress, 10 );
			update_screen () ;
		}
	} else {
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Lade Bild aus Cache..."));
		gtk_progress_set_value (progress, 10 );
		update_screen () ;
		printd("Bild aus dem Speicher");
	}

	update_screen () ;
	
	if ( loaded_image )
	{
		double imagewidth, imageheight, scale, real_scale, scale_x = 1, scale_y = 1 ;
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

		/*if ( reload == FALSE )
		{
			char dimension[2048] ;
			struct stat buf ;
			gchar *imagestatsclist_entry[1] ;
			printd(" - iamgestatsclist updaten -");
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
		printd(" geladen\n");

		imagewidth = ( gdk_pixbuf_get_width ( loaded_image ) ) ;//* scale ;
		imageheight = ( gdk_pixbuf_get_height ( loaded_image ) ) ;//* scale ;

		gtkpixmap = lookup_widget ( user_data, "image") ;
		//gtk_pixmap_set ( GTK_PIXMAP(gtkpixmap) , gdk_pixmap_new ( lookup_widget( user_data, "MainWindow" )->window, 1, 1, -1 ), NULL ) ;
		// Now we need the size of the view-area
		/*hadjust = gtk_scrolled_window_get_hadjustment ( GTK_SCROLLED_WINDOW( lookup_widget ( user_data, "scrolledwindow6" ) ) ) ;
		vadjust = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW( lookup_widget ( user_data, "scrolledwindow6" ) ) ) ;
		canvaswidth = (int) hadjust->page_size ; canvasheight = (int) vadjust->page_size ;
		printd("change widget size\n");
		gtk_widget_set_usize ( gtkpixmap, canvaswidth, canvasheight );
		update_screen () ;*/
		hadjust = gtk_scrolled_window_get_hadjustment ( GTK_SCROLLED_WINDOW( lookup_widget ( user_data, "scrolledwindow6" ) ) ) ;
		vadjust = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW( lookup_widget ( user_data, "scrolledwindow6" ) ) ) ;
		canvaswidth = (int) hadjust->page_size ; canvasheight = (int) vadjust->page_size ;

		//canvaswidth = lookup_widget( user_data, "scrolledwindow6" )->allocation.width ;
		//canvasheight = lookup_widget( user_data, "scrolledwindow6" )->allocation.height ;

		if ( gtk_toggle_button_get_active ( 
									GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) &&
			GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
		{	
			/*GdkPixmap *dummypix ;
			dummypix = gdk_pixmap_new ( lookup_widget( FullscreenWindow, "scrolledwindow6" )->window, canvaswidth, canvasheight, -1 ) ;
			gtk_pixmap_set ( GTK_PIXMAP(lookup_widget(FullscreenWindow, "image")) , dummypix, NULL ) ;
			gdk_pixmap_unref ( dummypix ) ;*/
			if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) )
				gtk_widget_set_usize ( lookup_widget ( FullscreenWindow, "image" ), canvaswidth, canvasheight ) ;
			canvaswidth = lookup_widget ( FullscreenWindow, "fullscreenbox" )->allocation.width ;
			canvasheight = lookup_widget ( FullscreenWindow, "fullscreenbox" )->allocation.height ;
		}
		
		/*if ( lookup_widget( user_data, "scrolledwindow6" )->allocation.width > canvaswidth &&
				gtkpixmap->allocation.width < lookup_widget( user_data, "scrolledwindow6" )->allocation.width )
			canvaswidth = lookup_widget( user_data, "scrolledwindow6" )->allocation.width ;
		if ( lookup_widget( user_data, "scrolledwindow6" )->allocation.height > canvasheight &&
				gtkpixmap->allocation.height < lookup_widget( user_data, "scrolledwindow6" )->allocation.height )
			canvasheight = lookup_widget( user_data, "scrolledwindow6" )->allocation.height ;
		*/
		{
			double mem_size ;
			
			scale = (double) gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON( lookup_widget( MainWindow, "zoomentry" ) ) );
			scale = scale / 100 ;

			mem_size = (   (   (imagewidth*scale)*(imageheight*scale)*16   ) / 1024   ) / 1024 ;
			
			if ( mem_size > (double) 500 ) {
				printd("DANGER! Scaled picture will probably take more than ");
				if ( (int) mem_size > 1024*1024 )
				{
					mem_size = mem_size / 1024 / 1024 ; 
					printd( text_from_var( (int) mem_size ) ) ;
					printd (" terrabytes" ) ;
				} else if ( (int) mem_size > 1024 ) {
					mem_size = mem_size / 1024 ; 
					printd( text_from_var( (int) mem_size ) ) ;
					printd (" gigabytes" ) ;
				} else {
					printd( text_from_var( (int) mem_size ) ) ;
					printd (" megabytes" ) ;
				}
				printd(" of memory!\n");
				force_auto_zoom = TRUE ;
			} else if ( mem_size > (double) 200 ) {
				printd("DANGER! Scaled picture will probably take more than ");
				printd( text_from_var( (int) mem_size ) );
				printd(" MB of memory!\n");
			}
		}
		
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoom" )) ) || force_auto_zoom == TRUE )
		{
			//gtk_widget_set_sensitive ( lookup_widget(user_data,"zoombox2"), FALSE );
			if ( canvaswidth > imagewidth )
			{
				printd ( "scale1 canvaswidth > imagewidth\n" ) ;
				scale = (double) canvaswidth / (double) imagewidth ;
			} else {
				printd ( "scale2 canvaswidth <= imagewidth\n" ) ;
				scale = (double) canvasheight / (double) imageheight ;
			}
			if ( imageheight * scale > canvasheight )
			{
				printd ( "scale3 imageheight*scale > canvasheight\n" ) ;
				scale = (double) canvasheight / (double) imageheight ;
			}
			if ( imagewidth * scale > canvaswidth )
			{
				printd ( "scale4 imagewidth*scale > canvaswidth\n" ) ;
				scale = ((double) MAX (canvaswidth, canvasheight)) / 
							((double) MAX (imagewidth, imageheight));
				if ( imagewidth * scale > canvaswidth )
				{
					printd ( "scale4.1 imagewidth*scale > canvaswidth\n" ) ;
					scale = (double) canvaswidth / (double) imagewidth  ;
				} else if ( imagewidth * scale == canvaswidth )
					printd ( "scale4.2 imagewidth*scale < canvaswidth\n" ) ;
				else if ( imagewidth * scale < canvaswidth )
				{
					printd ( "scale4.3 imagewdith*scale < canvaswidth \nimage width greater than canvas width *FIXME* \n" ) ;
					scale = (double) canvaswidth / (double) imagewidth ;
				}
			}
			/*if ( imageheight * scale > canvasheight )
			{
				printd ( "scale5\n" ) ;
				scale = (double) canvasheight / (double) imageheight ;
			}*/
			
			// bitte keinen Autozoom über dem 5-Fachen der Normalgröße!
			if ( scale > 10 ) scale = 10 ;
				
			gtk_spin_button_set_value ( GTK_SPIN_BUTTON( lookup_widget( MainWindow, "zoomentry" ) ), ( (int) ( scale*100) ) );
		} else {
			//gtk_widget_set_sensitive ( lookup_widget(user_data,"zoombox2"), TRUE );
			scale = (double) gtk_spin_button_get_value_as_float( 
								GTK_SPIN_BUTTON( lookup_widget( MainWindow, "zoomentry" ) ) );
			scale = scale / 100 ;
			//imagewidth = imagewidth ; imageheight = imageheight ;
		}
		
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "keepaspect" )) ) == FALSE &&
			gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoom" )) ) == TRUE )
		{
			scale_x = scale ; scale_y = scale ;
			if ( imagewidth * scale < canvaswidth ) scale_x = (double) canvaswidth / (double) imagewidth ;
			if ( imageheight * scale < canvasheight ) scale_y = (double) canvasheight / (double) imageheight ;
			
			if ( canvaswidth-(imagewidth*scale) > (double)canvaswidth*0.1 ) scale_x = scale ;
			if ( canvasheight-(imageheight*scale) > (double)canvasheight*0.1 ) scale_y = scale ;
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
		printd("scale = "); printd(text_from_var(scale*100)); printd("\n");
		printd("scale_x = "); printd(text_from_var(scale_x*100)); printd("\n");
		printd("scale_y = "); printd(text_from_var(scale_y*100)); printd("\n");
		
		imagewidth = imagewidth*scale_x ; imageheight = imageheight*scale_y ;
		imagewidth = (int) imagewidth ; imageheight = (int) imageheight ;
		//printf("%f x %f\n", imagewidth, imageheight );
		
		if ( scale == 0 ) 
		{
			printd("scale = 0 - kein Bild.\n");
			if ( gdkpixmap ) gdk_pixmap_unref (gdkpixmap) ;
			if ( dest ) gdk_pixbuf_unref ( dest ) ;
			return ;
		}
		
		if ( imagewidth > canvaswidth) canvaswidth = imagewidth ;
		if ( imageheight > canvasheight ) canvasheight = imageheight ;

		printd("canvaswidth = "); printd(text_from_var(canvaswidth));
		printd(" ; canvasheight = "); printd(text_from_var(canvasheight)); printd(" \n");
				
		printd("imagewidth = "); printd(text_from_var(imagewidth));
		printd(" ; imageheight = "); printd(text_from_var(imageheight)); printd(" \n");
		
		if ( imagewidth < canvaswidth )
			imagex = ( canvaswidth - imagewidth ) / 2 ;
		else
			imagex = 0 ;
		if ( imageheight < canvasheight )
			imagey = ( canvasheight - imageheight ) / 2 ;
		else
			imagey = 0 ;
		
		printd("imagex = "); printd(text_from_var(imagex));
		printd(" ; imagey = "); printd(text_from_var(imagey)); printd(" \n");
		
		printd("use background coloration on alpha-channel? ");
		if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )) ) == TRUE ) // background coloration
		{
			guint8 r, g, b, a ;
			
			printd("YES! lets see...\n");
			{
				gdouble color[4] ;
				printd("get color from colorselectiondialog...\n");
				gtk_color_selection_get_color	( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), color);
				
				printd ("show color...\n");
				printd ( " r = " ) ; printd ( text_from_var ( color[0] * 100 ) ) ;
				printd ( ", g = " ) ; printd ( text_from_var ( color[1] * 100 ) ) ;
				printd ( ", b = " ) ; printd ( text_from_var ( color[2] * 100 ) ) ;
				printd ( ", trans = " ) ; printd ( text_from_var ( color[3] * 100 ) ) ;
				printd ( "\n" ) ;

				r = color[0] * 255 ; g = color[1] * 255 ; b = color[2] * 255 ; a = color[3] * 255 ;
			}			
			// get every first color
			checkcolora = scale_color ( r, g, b, a, check_color_a, 0 ) ;
			
			printd (text_from_var(r)); printd(" "); printd (text_from_var(g)); printd(" "); printd (text_from_var(b)); printd(" "); printd (text_from_var(a)); printd(" \n"); 
			printd (text_from_var(checkcolora)); printd ("\n");
			
			// get every second color
			checkcolorb = scale_color ( r, g, b, a, check_color_b, 0 ) ;
			
			printd (text_from_var(r)); printd(" "); printd (text_from_var(g)); printd(" "); printd (text_from_var(b)); printd(" "); printd (text_from_var(a)); printd(" \n"); 
			printd (text_from_var(checkcolorb)); printd ("\n");
		} else { // no background coloration - use automatic color-detection?
			if ( check_color_auto == TRUE )
			{
				guint32 value_r = 0, value_g = 0, value_b = 0, prevalue[3] ;
				int width = gdk_pixbuf_get_width ( loaded_image ), height = gdk_pixbuf_get_height ( loaded_image ) ;
				int range = 16 ;
				printd("NO! but we try to get those from the picture...\n");
				
				prevalue[0] = get_pixel_from_loaded_image ( 1, 1 ) ;
				prevalue[1] = get_pixel_from_loaded_image ( width - 1, 1 ) ;
				prevalue[2] = get_pixel_from_loaded_image ( width - 1, height - 1 ) ;
				prevalue[3] = get_pixel_from_loaded_image ( 1, height - 1 ) ;
				
				if ( check_color_auto_routine == 1 ) {
					
					value_r = ( get_red_from_rgb_value ( prevalue[0], range ) +
									get_red_from_rgb_value ( prevalue[1], range ) +
									get_red_from_rgb_value ( prevalue[2], range ) +
									get_red_from_rgb_value ( prevalue[3], range ) ) / 4 ;
					//printf ( "main red = %d\n", value_r ) ;
					value_g = ( get_green_from_rgb_value ( prevalue[0], range ) +
									get_green_from_rgb_value ( prevalue[1], range ) +
									get_green_from_rgb_value ( prevalue[2], range ) +
									get_green_from_rgb_value ( prevalue[3], range ) ) / 4 ;
					//printf ( "main green = %d\n", value_g ) ;
					value_b = ( get_blue_from_rgb_value ( prevalue[0], range ) +
									get_blue_from_rgb_value ( prevalue[1], range ) +
									get_blue_from_rgb_value ( prevalue[2], range ) +
									get_blue_from_rgb_value ( prevalue[3], range ) ) / 4 ;
					//printf ( "main blue = %d\n", value_b ) ;
					
					checkcolora = ( value_r * 256 * 256 ) + ( MIN( value_g, value_b ) * 256 ) + MAX( value_b, value_g ) ;
					checkcolorb = checkcolora ;
					
				} else if ( check_color_auto_routine == 2 ) {
					
					value_r = ( MIN ( get_red_from_rgb_value ( prevalue[0], range ),
									get_red_from_rgb_value ( prevalue[1], range ) ) +
									MIN ( get_red_from_rgb_value ( prevalue[2], range ),
									get_red_from_rgb_value ( prevalue[3], range ) ) ) / 2 ;
					//printf ( "main red = %d\n", value_r ) ;
					value_g = ( MIN ( get_green_from_rgb_value ( prevalue[0], range ),
									get_green_from_rgb_value ( prevalue[1], range ) ) +
									MIN ( get_green_from_rgb_value ( prevalue[2], range ),
									get_green_from_rgb_value ( prevalue[3], range ) ) ) / 2 ;
					//printf ( "main green = %d\n", value_g ) ;
					value_b = ( MIN ( get_blue_from_rgb_value ( prevalue[0], range ),
									get_blue_from_rgb_value ( prevalue[1], range ) ) +
									MIN ( get_blue_from_rgb_value ( prevalue[2], range ),
									get_blue_from_rgb_value ( prevalue[3], range ) ) ) / 2 ;
					//printf ( "main blue = %d\n", value_b ) ;
					
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
					//printf ( "main red = %d\n", value_r ) ;
					value_g = MIN ( value_g, value_g2 ) / 2 ;
					//printf ( "main green = %d\n", value_g ) ;
					value_b = MIN ( value_b, value_b2 ) / 2 ;
					//printf ( "main blue = %d\n", value_b ) ;
					
					// and convert it to a 32bit integer usable as hex -> 0xFFFFFF
					//checkcolora = ( value_r * 256 * 256 ) + ( value_g * 256 ) + value_b ;
					checkcolora = ( value_r * 256 * 256 ) + ( MIN( value_g, value_b ) * 256 ) + MAX( value_b, value_g ) ;
					
					
					{	// scan the picture for brightness
						guint32 pixel_brightness, current_pixel, darkest_pixel = 255, brightest_pixel = 0, average_pixel = 0, count = 0, value = 0 ;
						int x = 0, y = 0 ;
						int density = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton6" ) ) ) ;
						// later we scan every 'density'th points for its brightness
						
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
						
						// overide automatics
						darkest_pixel = 0 ; brightest_pixel = 255 ;
						
						value = value / count ;
						average_pixel = ( brightest_pixel + darkest_pixel ) / 2 ;
						
						average_pixel = average_pixel * gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton7" ) ) ) ;
						
						printd ( "the overall picture brightness is about " ) ; printd ( text_from_var( value ) ) ; printd ( "\n" ) ;
						printd ( "the brightest pixel is about " ) ; printd ( text_from_var( brightest_pixel ) ) ; printd ( "\n" ) ;
						printd ( "the darkest pixel is about " ) ; printd ( text_from_var( darkest_pixel ) ) ; printd ( "\n" ) ;
						printd ( "the average pixel is about " ) ; printd ( text_from_var( average_pixel ) ) ; printd ( "\n" ) ;
						
						if ( value >= average_pixel ) 
						{
							guint32 color_a, color_b ;
							printd ( "we've got a bright picture!\n");
							
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
							printd ( "we've got a dark picture!\n") ;
							
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
				
				printd ( text_from_var(checkcolora) ) ; printd ( "\n" ) ;
				
				//printf ( "%06X \n", checkcolora ) ;
				
			} else {
				printd("NO! using standard grey checkerboard as background...\n");
			}
		}
		
		printd("now the fadeout for the background... if wanted :) \n");
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
				
				printd ( text_from_var( checkcolorouta ) ) ; printd ( " " ) ; printd ( text_from_var( checkcoloroutb ) ) ; printd ( "\n" ) ;
			} else { // we don't want automatic color detection....
				guint8 r, g, b, a = 0 ;
				
				if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget( MainWindow, "bgcoloruse" )) ) == TRUE )
				{
					{
						gdouble color[3] ;
						printd("get color from colorselectiondialog...\n");
						gtk_color_selection_get_color	( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), color);
						
						printd ("show color...\n");
						printd ( " r = " ) ; printd ( text_from_var ( color[0] * 100 ) ) ;
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
		
		printd("now check for images in memory etc. \n");
		
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
				printd("nothing to be done - get out of here....\n");
				
				if ( gdkpixmap ) { printd("free pixmap\n"); gdk_pixmap_unref (gdkpixmap) ; }
				if ( dest ) { printd("free pixbuf\n"); gdk_pixbuf_unref ( dest ) ; }
				//if ( drawing_gc ) { printd("free gc\n"); gdk_gc_unref ( drawing_gc ) ; }
				//gdk_pixbuf_unref ( dummy ) ;
				gtk_widget_set_sensitive ( imagedisplay, TRUE );
				gtk_progress_set_value (progress, 0 );
				gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Bild dargestellt."));
				//gtk_progress_set_value (progress, 100 );
				//gtk_widget_set_sensitive ( lookup_widget ( user_data, "frame1" ), TRUE );
				//update_screen () ;
				//gtk_progress_set_value (progress, 0 );
				//update_screen () ;
				return ;
			} else {
				if ( gtk_toggle_button_get_active ( 
											GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton3" )) ) &&
						gtk_toggle_button_get_active ( 
											GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton4" )) ) )
					{
						//gtk_widget_set_redraw_on_allocate ( FullscreenWindow, FALSE ) ; // API 2.0
						gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
					}
				gtk_progress_set_value (progress, 20 );
				gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Erstelle Matrix..."));
				printd("create matrix...\n");
				update_screen () ;
				
				printd("gdk_pixbuf_new ()\n");
				dummy = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( loaded_image ), TRUE, 
															gdk_pixbuf_get_bits_per_sample ( loaded_image ),
															canvaswidth, canvasheight ) ;
				printd("copying dummy on dest... ( we don't need checkers on background )\n");
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
					gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
				}
			gtk_progress_set_value (progress, 20 );
			gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Erstelle Matrix..."));
			update_screen () ;
			
			printd("gdk_pixbuf_new ()\n");
			dummy = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( loaded_image ), TRUE, 
														gdk_pixbuf_get_bits_per_sample ( loaded_image ),
														canvaswidth, canvasheight ) ;
			//printd("gdk_pixbuf_scale_simple()\n");
			//dest = gdk_pixbuf_scale_simple ( im, imagewidth, imageheight, GDK_INTERP_BILINEAR );
			printd("painting some checkers on background... gdk_pixbuf_composite_color_simple()\n");
			dest = gdk_pixbuf_composite_color_simple ( dummy, canvaswidth, canvasheight, GDK_INTERP_NEAREST, 0, 
																				checksize, checkcolorouta, checkcoloroutb ) ;
			gdk_pixbuf_unref ( dummy ) ;
		}
		
		if ( ! gdk_pixbuf_get_has_alpha ( loaded_image ) ) 
		{
			printd("we don't need an alpha channel for this pic\n");
			printd("so we don't need any pattern behind it...\n");
			checkcolora = 0x000000 ;
			checkcolorb = 0x000000 ;
			//checksize = 0 ;
		}
		
		gtk_progress_set_value (progress, 40 );
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Scaliere und platziere Bild..."));
		update_screen () ;
		
		gdk_threads_leave () ;

		/*if ( loaded_scaled_image )
		{
			printf ( "%f x %f x %d == %f x %f x %d \n",	(double) gdk_pixbuf_get_width( loaded_scaled_image ),
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
			printd ( "using cached image data...\n");
			//gdk_pixbuf_copy ( loaded_scaled_image ) ;
		} else {
			printd ("scaling image data...\n");
			if ( loaded_scaled_image ) gdk_pixbuf_unref ( loaded_scaled_image ) ;
			loaded_scaled_image = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( loaded_image ), 
														gdk_pixbuf_get_has_alpha(loaded_image), 
														gdk_pixbuf_get_bits_per_sample ( loaded_image ),
														imagewidth, imageheight ) ;
			
			printd("gdk_pixbuf_composite_color ( -> loaded_scaled_image)\n");
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
																checkcolora,
																checkcolorb );
		}
		
		gdk_threads_enter () ;
		
		gtk_progress_set_value (progress, 45 );
		//gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Scaliere und platziere Bild..."));
		update_screen () ;
		
		gdk_threads_leave () ;
		
		if ( gdk_pixbuf_get_width ( loaded_scaled_image ) == canvaswidth && gdk_pixbuf_get_height ( loaded_scaled_image ) == canvasheight )
		{
			printd("copying loaded_scaled_image on dest... gdk_pixbuf_copy()\n");
			if ( dest != NULL ) gdk_pixbuf_unref ( dest ) ;
			dest = gdk_pixbuf_copy ( loaded_scaled_image ) ;
		} else {
			printd("render loaded_scaled_image on dest... gdk_pixbuf_composite_color ( -> dest)\n");
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
		}
		
		gdk_threads_enter () ;
		printd("display image!\n"); gtk_progress_set_value (progress, 50 );
		gdkpixmap = gdk_pixmap_new ( lookup_widget( user_data, "scrolledwindow6" )->window, canvaswidth, canvasheight, -1 ) ;
		printd("gdk_gc_new()\n"); gtk_progress_set_value (progress, 55 );
		drawing_gc = gdk_gc_new ( lookup_widget( user_data, "scrolledwindow6" )->window );
		printd("gdk_gc_set_function\n"); gtk_progress_set_value (progress, 60 );
		gdk_gc_set_function (drawing_gc, GDK_COPY);
		printd("gdk_rgb_gc_set_foreground\n"); gtk_progress_set_value (progress, 70 );
		gdk_rgb_gc_set_foreground (drawing_gc, 0x000000);
		printd("gdk_rgb_gc_set_background\n"); gtk_progress_set_value (progress, 75 );
		gdk_rgb_gc_set_background (drawing_gc, 0x000000);

		gtk_progress_set_value (progress, 80 );
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Rendere Bild..."));
		update_screen () ;
		
		printd("gdk_pixbuf_render_to_drawable\n");
		gdk_pixbuf_render_to_drawable ( dest, gdkpixmap, drawing_gc, 0, 0, 0, 0, canvaswidth, canvasheight, GDK_RGB_DITHER_MAX, 0, 0 ) ;

		gtk_progress_set_value (progress, 90 );
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Bild anzeigen..."));
		//update_screen () ;

		printd("gtk_pixmap_set\n");
		gtk_pixmap_set ( GTK_PIXMAP(gtkpixmap) , gdkpixmap, NULL ) ;
		printd("rendered.\n");

		//printd("change widget size\n");
		//gtk_widget_set_usize ( gtkpixmap, canvaswidth, canvasheight );

		if ( gdkpixmap ) { printd("free pixmap\n"); gdk_pixmap_unref (gdkpixmap) ; }
		if ( dest ) { printd("free pixbuf\n"); gdk_pixbuf_unref ( dest ) ; }
		if ( drawing_gc ) { printd("free gc\n"); gdk_gc_unref ( drawing_gc ) ; }
		
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Bild dargestellt."));
		if ( force_auto_zoom == TRUE )
			gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(lookup_widget ( MainWindow, "autozoom" )),TRUE ) ;
		
		redraw_bgcolorpixmap ( MainWindow ) ;
	} else {
		printd(" nicht geladen!\n");
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _("Kein Bild geladen."));
	}
	gtk_progress_set_value (progress, 100 );
	
	// now check, if user already moved to another image! :)
	// I know that this is something like a hack but it works somehow :)
	if ( check_for_new_image_selection ( filename ) )
		gtk_clist_select_row ( GTK_CLIST(lookup_widget(MainWindow,"imagelist")), check_for_new_image_selection ( filename ), 0 ) ;
	//
	
	//gtk_widget_set_sensitive ( lookup_widget ( user_data, "frame1" ), TRUE );
	//update_screen () ;
	gtk_progress_set_value (progress, 0 );
	
	if ( loaded_image )
	{
		char text[2048], titletext[2048], aspect_text[256] ;
		double scale ;
		
		if ( strcmp ( filename, "__RELOAD__" ) == 0 )
			filename = "den Startbildschirm" ;
		
		if ( aspect_correction == TRUE )
			strcpy ( aspect_text, "mit Aspekt-Korrektur" ) ;
		else
			strcpy ( aspect_text, "ohne Aspekt-Korrektur" ) ;
		
		scale = (double) MAX( gdk_pixbuf_get_width(loaded_scaled_image), gdk_pixbuf_get_height(loaded_scaled_image) ) /
				(double) MAX( gdk_pixbuf_get_width(loaded_image), gdk_pixbuf_get_height(loaded_image) ) ;
		
		sprintf ( text, "Zeige %s mit einer Größe von %dx%d Pixel ( %s ), vergrößert auf %d%% des Originals.",
					filename, gdk_pixbuf_get_width ( loaded_image ),
					gdk_pixbuf_get_height ( loaded_image ),
					aspect_text,
					(int) ( scale * (double) 100 ) ) ;
		gnome_appbar_set_status (GNOME_APPBAR (appbar2), _(text));
		sprintf ( titletext, "Look at the stars zeigt %s ( %d%% )",
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
			GTK_WIDGET_VISIBLE ( lookup_widget(FullscreenWindow, "appbarbox") ) )
			gtk_widget_hide ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
	} else if ( GTK_WIDGET_VISIBLE ( lookup_widget(FullscreenWindow, "appbarbox") ) == FALSE ) {
		gtk_widget_show ( lookup_widget( FullscreenWindow, "appbarbox" ) ) ;
	}

	//update_screen () ;
}


void
zoom_picture ( double value, gpointer user_data )
{
	double scale = 1;
	GtkWidget *imagedisplay ;

	imagedisplay = lookup_widget ( MainWindow, "handlebox1" ) ;
	
	if ( GTK_WIDGET_SENSITIVE ( imagedisplay ) == FALSE ) return ;
	
	printd("zoom picture\n");
	//update_screen();
	
	gtk_widget_set_sensitive ( imagedisplay, FALSE ) ;

	printd("get current zoom level \n");
	scale = (double) gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(lookup_widget( MainWindow, "zoomentry" )) );

	if ( scale != value )
	{
		printd("zoom changed!\n");
		if ( loaded_image )
		{
			printd("setup values\n");
			if ( gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(lookup_widget( MainWindow, "zoomentry" )) ) != ( (int) value) )
			{
				printd("change value in combo entry\n");
				if ( value == -1 ) value = scale ;
				//value = ( (double) ( (int) ( value * 100 ) ) ) / (double) 100 ;
				gtk_spin_button_set_value( GTK_SPIN_BUTTON(lookup_widget( MainWindow, "zoomentry" )), value );
				if ( GTK_WIDGET_VISIBLE ( FullscreenWindow ) )
					view_image ( "__UPDATE__", FullscreenWindow ) ;
				else
					view_image ( "__UPDATE__", user_data ) ;
			}
			printd("picture zoomed.\n");
		} else {
			printd("image not loaded yet - not zoomed.\n");
		}
	} else { printd("zoom not changed\n"); }
	
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
		
		printd("redraw bgcolorbuttonpixmap... lets see...\n");
		{
			gdouble color[4] ;
			printd("get color from colorselectiondialog...\n");
			gtk_color_selection_get_color	( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(BgColorSelectionDialog)->colorsel), color);
			
			printd ("show color...\n");
			printd ( " r = " ) ; printd ( text_from_var ( color[0] * 100 ) ) ;
			printd ( ", g = " ) ; printd ( text_from_var ( color[1] * 100 ) ) ;
			printd ( ", b = " ) ; printd ( text_from_var ( color[2] * 100 ) ) ;
			printd ( ", trans = " ) ; printd ( text_from_var ( color[3] * 100 ) ) ;
			printd ( "\n" ) ;
			r = color[0] * 255 ; g = color[1] * 255 ; b = color[2] * 255 ; a = color[3] * 255 ;
		}			
		
		// get every first color
		checkcolora = scale_color ( r, g, b, a, check_color_a, 0 ) ;
		
		printd (text_from_var(r)); printd(" "); printd (text_from_var(g)); printd(" "); printd (text_from_var(b)); printd(" "); printd (text_from_var(a)); printd(" \n"); 
		printd (text_from_var(checkcolora)); printd ("\n");
		
		// get every second color
		checkcolorb = scale_color ( r, g, b, a, check_color_b, 0 ) ;
		
		printd (text_from_var(r)); printd(" "); printd (text_from_var(g)); printd(" "); printd (text_from_var(b)); printd(" "); printd (text_from_var(a)); printd(" \n"); 
		printd (text_from_var(checkcolorb)); printd ("\n");
	}
	
	// render this colormess onto our preview-pixmap!
	printd("gdk_pixmap_new\n");
	gdkpixmap = gdk_pixmap_new ( lookup_widget( MainWindow, "MainWindow" )->window, width, height, -1 ) ;
	printd("gdk_gc_new\n");
	drawing_gc = gdk_gc_new ( lookup_widget( MainWindow, "MainWindow" )->window );
	printd("gdk_gc_set_function\n");
	gdk_gc_set_function (drawing_gc, GDK_COPY);
	printd("gdk_rgb_gc_set_foreground\n");
	gdk_rgb_gc_set_foreground (drawing_gc, 0x000000);
	printd("gdk_rgb_gc_set_background\n");
	gdk_rgb_gc_set_background (drawing_gc, 0 );
	printd("gdk_pixbuf_new\n");
	dummy = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( loaded_image ), TRUE, 
											gdk_pixbuf_get_bits_per_sample ( loaded_image ),
											width, height ) ;
	printd("gdk_pixbuf_composite_color_simple\n");
	dest = gdk_pixbuf_composite_color_simple ( dummy, width, height, GDK_INTERP_NEAREST, 0, 
																	checksize, checkcolora, checkcolorb ) ;
	printd("unref dummy\n");
	gdk_pixbuf_unref ( dummy ) ;
	printd("render to drawable\n");
	gdk_pixbuf_render_to_drawable ( dest, gdkpixmap, drawing_gc, 0, 0, 0, 0, width, height, GDK_RGB_DITHER_MAX, 0, 0 ) ;
	printd("set pixmap\n");
	gtk_pixmap_set ( GTK_PIXMAP(lookup_widget(MainWindow,"bgcolorpixmap")) , gdkpixmap, NULL ) ;
	//update_screen();
	printd("now unref used resources...\n");
	if ( gdkpixmap ) gdk_pixmap_unref (gdkpixmap) ;
	if ( dest ) gdk_pixbuf_unref ( dest ) ;
	if ( drawing_gc ) gdk_gc_unref ( drawing_gc ) ;
	printd("preview pixmap refreshed.\n");
}
