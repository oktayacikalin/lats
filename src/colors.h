#include <gnome.h>

int
get_red_from_rgb_value ( double color, int range ) ;

int
get_green_from_rgb_value ( double color, int range ) ;

int
get_blue_from_rgb_value ( double color, int range ) ;

void
show_rgb_from_value ( double color, int range ) ;

guint32
scale_color ( guint8 r, guint8 g, guint8 b, guint8 a, guint8 dest, guint8 fadeout ) ;

int
get_check_color_a ( void ) ;

int
get_check_color_b ( void ) ;

int
get_check_color_out_a ( void ) ;

int
get_check_color_out_b ( void ) ;

int
get_check_color_auto ( void ) ;

int
get_check_color_out_use ( void ) ;

float
get_check_color_out_cut_off_factor ( void ) ;

guint32
get_pixel_from_loaded_image ( int x, int y ) ;

