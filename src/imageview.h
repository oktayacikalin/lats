#include <gnome.h>

void
image_progressbar_thread ( void ) ;

void
stop_image_progressbar ( void ) ;

void
start_image_progressbar ( void ) ;

void
mp_update_progressbar_thread ( void ) ;

void
mp_update_progressbar ( void ) ;

void
mp_fill_sidebar_thread ( char *filename ) ;

void
mp_fill_sidebar ( char *filename ) ;

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

void
follow_mouse_thread ( void ) ;

void
follow_mouse ( void ) ;
