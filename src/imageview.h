#include <gnome.h>

int
check_for_new_image_selection ( char *filename ) ;

void
load_image ( char *filename ) ;

void
view_image ( char *filename, gpointer user_data );

void
zoom_picture ( double value, gpointer user_data );

void
redraw_bgcolorpixmap ( gpointer user_data ) ;

