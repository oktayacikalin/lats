#include <gnome.h>

void
on_spider_toggled      (GtkToggleButton *togglebutton,
                        gpointer         user_data) ;

void
add_spider_to_toolbar ( void ) ;

void 
add_zoom_buttons_to_toolbar ( void ) ;

void
vmbutton_hide ( int button ) ;

void
vmbutton_show ( int button ) ;

void
vmbutton_disable ( int button ) ;

void
vmbutton_enable ( int button ) ;

void
add_vmbutton_series_to_toolbar ( int toolbarnum, int use_accels ) ;

