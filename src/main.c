#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <pthread.h>

#include "interface.h"
#include "support.h"

#include "splash.h"
#include "mainwindow.h"
#include "prefswindow.h"
#include "fullscreenwindow.h"
#include "dirlist.h"
#include "imageview.h"


int
main (int argc, char *argv[])
{
	
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	
	g_thread_init ( NULL ) ; // init our thread-thingy... :)
	gdk_threads_init () ;

	gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                      argc, argv,
                      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
                      NULL);

	g_print ( _("Loading Look at the stars %s..."), VERSION ) ;
	g_print ( "\n\n" ) ;
	
	show_splash_screen () ;
	show_splash_message (  g_strdup_printf ( _("Loading Look at the stars %s..."), VERSION )  ) ;
	
	create_mainwindow () ;
	
	display_mainwindow () ;
	refresh_screen () ;

	initialize_image_canvas () ;

	show_splash_message (  g_strdup_printf ( _("Loading Look at the stars %s... building directory-tree..."), VERSION )  ) ;
	create_dir_list () ;
	
	show_splash_message (  g_strdup_printf ( _("Done. Now you may touch the main-window ;-)"), VERSION )  ) ;
	hide_splash_screen () ;
	refresh_screen () ;
		
	gtk_main ();
	
	g_print ( _("exiting...\n") ) ;
	return 0;
}

