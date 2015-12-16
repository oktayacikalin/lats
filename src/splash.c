#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <pthread.h>

#include "interface.h"
#include "support.h"

#include "splash.h"

#include "look_at_the_stars_bg.xpm"

GtkWidget *splash_screen ;
GtkWidget *splash_screen_statusbar ;

int extern thread_count ;

void
refresh_screen ( void )
{
        while (gtk_events_pending () )
        {
                gtk_main_iteration () ;
        }
}


void
update_screen ( void )
{
        gdk_threads_leave () ;
        //usleep ( 1000 ) ;
        gdk_threads_enter () ;
}


void
show_splash_screen ( void )
{
	GtkWidget *splash_box ;
	GtkWidget *image ;
	GtkWidget *button ;
	
	splash_screen = gtk_window_new ( GTK_WINDOW_POPUP ) ;
	splash_box = gtk_vbox_new ( FALSE, 0 ) ;
	
	gtk_container_add ( GTK_CONTAINER(splash_screen), splash_box ) ;
	
	gtk_window_set_title ( GTK_WINDOW(splash_screen), _("Loading Look at the stars..." )) ;
	gtk_window_set_default_size ( GTK_WINDOW(splash_screen), 480, 211 ) ;
	gtk_window_set_position ( GTK_WINDOW(splash_screen), GTK_WIN_POS_CENTER ) ;
	gtk_window_set_decorated ( GTK_WINDOW(splash_screen), FALSE ) ;

	// splash-logo
	button = gtk_button_new () ;
	gtk_container_add ( GTK_CONTAINER(splash_box), button ) ;

	image = gnome_pixmap_new_from_xpm_d ( (const char **) look_at_the_stars_bg_xpm ) ;
	if ( ! image ) 
	{
		g_print ( _("logo could not be converted for splash-screen! :(\n") ) ;
		image = gtk_label_new ( _("Loading Look at the stars...") ) ;
	}
	gtk_container_add ( GTK_CONTAINER(button), image ) ;
	
	// splash_screen_statusbar
	splash_screen_statusbar = gnome_appbar_new ( FALSE, TRUE, GNOME_PREFERENCES_NEVER ) ;
	gtk_widget_ref ( splash_screen_statusbar ) ;
	gtk_object_set_data_full ( GTK_OBJECT(splash_screen_statusbar),
								"splash_screen_statusbar",
								splash_screen_statusbar,
								(GtkDestroyNotify) gtk_widget_unref ) ;
	gtk_container_add ( GTK_CONTAINER(splash_box), splash_screen_statusbar ) ;

	// display them :)
	gtk_widget_show_all ( splash_screen ) ;
	
	refresh_screen () ;
}


void
show_splash_message ( char *message )
{
	gnome_appbar_set_status ( GNOME_APPBAR(splash_screen_statusbar), message ) ;
	refresh_screen () ;
}


void
hide_splash_screen_thread ( void )
{
	sleep ( 1 ) ;
	gdk_threads_enter () ;
	gtk_widget_hide ( splash_screen ) ;
	gdk_threads_leave () ;
}


void
hide_splash_screen ( void )
{
	pthread_t hide_splash_thread ;
	
	pthread_create ( &hide_splash_thread, NULL, (void*)&hide_splash_screen_thread, NULL ) ;
	pthread_detach ( hide_splash_thread ) ;
}

