#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>


char
*get_config_value ( char *temp_option ) ;

void
use_config_line ( char *option, char *value ) ;

void
settings_load_all ( void ) ;

void
settings_save_all ( void ) ;

