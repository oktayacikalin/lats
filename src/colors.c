#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <math.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "callbacks.h"
#include "support.h"
#include "importedfuncs.h"
#include "colors.h"

GdkPixbuf extern *loaded_image ; // the actual loaded image

float extern check_color_out_cut_off_factor ;

GtkWidget extern *MainWindow ;
GtkWidget extern *PrefsWindow ;


int
get_red_from_rgb_value ( double color, int range )
{
	double farbe = color, rest ;
	
	rest = fmod ( farbe, range ) ; // blue
	farbe = ( farbe - rest ) / range ;
	rest = fmod ( farbe, range ) ; // green
	farbe = ( farbe - rest ) / range ;
	rest = fmod ( farbe, range ) ; // red
	
	farbe = rest * ( (double) 255 / (double) (range-1) ) ;

	//printf ( "co: red = %d \n", (int) farbe ) ;
	return (int) farbe ;
}


int
get_green_from_rgb_value ( double color, int range )
{
	double farbe = color, rest ;
	
	rest = fmod ( farbe, range ) ; // blue
	farbe = ( farbe - rest ) / range ;
	rest = fmod ( farbe, range ) ; // green
	
	farbe = rest * ( (double) 255 / (double) (range-1) ) ;
	
	//printf ( "co: green = %d \n", (int) farbe ) ;
	
	return (int) farbe ;
}


int
get_blue_from_rgb_value ( double color, int range )
{
	double farbe = color, rest ;
	
	rest = fmod ( farbe, range ) ; // blue
	
	farbe = rest * ( (double) 255 / (double) (range-1) ) ;
	
	//printf ( "co: blue = %d \n", (int) farbe ) ;
	
	return (int) farbe ;
}


void
show_rgb_from_value ( double color, int range ) 
{
	double farbe = color, rest ;
	
	printf ( "co: colorvalue = %d\n", (int) farbe ) ;
	
	rest = fmod ( farbe, range ) ;
	printf ( "co: %d / blue = %d\n", (int) farbe, (int) rest ) ;
	farbe = ( farbe - rest ) / range ;
	rest = fmod ( farbe, range ) ;
	printf ( "co: %d / green = %d\n", (int) farbe, (int) rest ) ;
	farbe = ( farbe - rest ) / range ;
	rest = fmod ( farbe, range ) ;
	printf ( "co: %d / red = %d\n\n", (int) farbe, (int) rest ) ;
}


guint32
scale_color ( guint8 r, guint8 g, guint8 b, guint8 a, guint8 dest, guint8 fadeout )
{
	guint32 r2, g2, b2 ;
	float a2 ;
	
	printd ("co: ");
	
	if ( a < 255 ) a = MIN ( MAX ( a * check_color_out_cut_off_factor, 0 ), 255 ) ;
		
	a2 = (float) a / (float) 255 ;
	
	r2 = MAX( r - (  ( r - dest ) * ( (float) 1 - a2 )  ) - fadeout, 0 ) ; printd (text_from_var(r2)); printd (" ");
	g2 = MAX( g - (  ( g - dest ) * ( (float) 1 - a2 )  ) - fadeout, 0 ) ; printd (text_from_var(g2)); printd (" ");
	b2 = MAX( b - (  ( b - dest ) * ( (float) 1 - a2 )  ) - fadeout, 0 ) ; printd (text_from_var(b2)); printd (" ");
	
	printd ("\n");
	
	return (guint32) (   ( r2 * 256 * 256 ) + ( g2 * 256 ) + b2  ) ;
}


int
get_check_color_a ( void ) 
{
	return (int) gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton1" ) ) );
}


int
get_check_color_b ( void ) 
{
	return (int) gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton2" ) ) );
}


int
get_check_color_out_a ( void ) 
{
	return (int) gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton3" ) ) );
}


int
get_check_color_out_b ( void ) 
{
	return (int) gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton4" ) ) );
}


int
get_check_color_auto ( void )
{
	return gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton1" )) ) ;
}


int
get_check_color_out_use ( void )
{
	return gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(lookup_widget ( PrefsWindow, "prefsimagecheckbutton2" )) ) ;
}


float
get_check_color_out_cut_off_factor ( void )
{
	return (float) gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON( lookup_widget( PrefsWindow, "spinbutton5" ) ) );
}


guint32
get_pixel_from_loaded_image ( int x, int y )
{
	GdkPixmap *gdkpixmap = NULL ;
	GdkGC *drawing_gc ;
	GdkImage *gdkimage ;
	guint32 value ;
	gdkpixmap = gdk_pixmap_new ( lookup_widget( MainWindow, "MainWindow" )->window, 1, 1, -1 ) ;
	drawing_gc = gdk_gc_new ( lookup_widget( MainWindow, "MainWindow" )->window );
	gdk_gc_set_function (drawing_gc, GDK_COPY);
	gdk_rgb_gc_set_foreground (drawing_gc, 0x000000);
	gdk_rgb_gc_set_background (drawing_gc, 0x000000);
	//printd("co: try getting topleft pixel from pixbuf...\n");
	//printf("co: width = %d ; height = %d\n", width, height ) ;
	gdk_pixbuf_render_to_drawable ( loaded_image, gdkpixmap, drawing_gc, x, y, 0, 0, 1, 1, GDK_RGB_DITHER_NONE, 0, 0 ) ;
	//printd("co: gdk_image_get gdkimage\n");
	gdkimage = gdk_image_get ( gdkpixmap, 0, 0, 1, 1 ) ;
	
	//printd("co: get pixel at "); printd ( text_from_var(x) ) ; printd ("x"); printd( text_from_var(y) ) ; printd ("\n") ;
	value = gdk_image_get_pixel ( gdkimage, 0, 0 ) ;
	
	if ( drawing_gc ) gdk_gc_unref ( drawing_gc ) ;
	if ( gdkpixmap )  gdk_pixmap_unref (gdkpixmap) ;
	if ( gdkimage ) gdk_image_destroy (gdkimage) ;
	
	return value ;
}


