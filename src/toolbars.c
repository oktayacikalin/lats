#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "toolbars.h"

#include "bgtiles.xpm"

#include "stock-zoom-1.xpm"
#include "stock-zoom-fit.xpm"
#include "stock-zoom-fit-width.xpm"
#include "stock-zoom-fit-height.xpm"
#include "stock-zoom-in.xpm"
#include "stock-zoom-out.xpm"
#include "stock-zoom-aspect.xpm"

#include "vmbutton1.xpm"
#include "vmbutton2.xpm"
#include "vmbutton3.xpm"

#include "spider.xpm"

GtkWidget extern *MainWindow ;

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#include "tasks.h"
#include "imagelist.h"

void
on_spider_toggled      (GtkToggleButton *togglebutton,
                        gpointer         user_data)
{
	if ( gtk_toggle_button_get_active ( 
							GTK_TOGGLE_BUTTON(
								lookup_widget ( MainWindow, "spider" )) ) == FALSE )
	{
		// make sure the spider isn't alive anymore...
		int own_task = task_add_to_queue ( "stop_spider", NULL ), dirlistpos ;
		GtkCList *dirlist ;
		char *status ;
		
		while ( task_already_exists_in_queue ( "go_spider_go", NULL ) )
		{
			usleep ( 1000 ) ;
			refresh_screen () ;
			//printd ( "tb: spider: waiting for other spider threads to shutdown...\n" ) ;
		}
		task_remove_from_queue ( own_task ) ;

		dirlist = GTK_CLIST(lookup_widget( MainWindow, "dirlist" ));
		dirlistpos = 0 ; // go to start of dirlist
		while ( gtk_clist_get_text ( dirlist, dirlistpos, 1, &status ) )
		{
			if ( strcmp( status, "..." ) == 0 )
				gtk_clist_set_text ( dirlist, dirlistpos, 1, NULL ) ;
			dirlistpos++;
		}

	} else {
		if ( task_already_exists_in_queue ( "go_spider_go", NULL ) == FALSE &&
			 task_already_exists_in_queue ( "reload_dir_list", NULL ) == FALSE )
			go_spider_go () ;
	}
}

void
add_spider_to_toolbar ( void )
{
	GtkWidget *tmp_toolbar_icon ;
	GtkWidget *toolbar2, *spider ;

	toolbar2 = lookup_widget ( MainWindow, "toolbar2" ) ;
	
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( spider_xpm ) ;
	spider = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar2),
											GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
											NULL,
											_("spider"),
											_("may my little spider examine your subdirectories?"), NULL,
											tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (spider);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "spider", spider,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (spider);
	
	gtk_signal_connect (GTK_OBJECT (spider), "toggled",
									GTK_SIGNAL_FUNC (on_spider_toggled),
									MainWindow);
	
}


void 
add_zoom_buttons_to_toolbar ( void ) 
{
	GtkWidget *tmp_toolbar_icon;
	GtkWidget *zoombox2;
	GtkWidget *zoomm25;
	GtkWidget *zoomp25;
	GtkWidget *zoom100;
	GtkWidget *realzoom;
	GtkWidget *autozoom;
	GtkWidget *autozoomwidth;
	GtkWidget *autozoomheight;
	GtkWidget *keepaspect;
	GtkWidget *bgtiles;
	GtkWidget *toolbar5;
	GtkAccelGroup *accel_group;
	
	accel_group = gtk_accel_group_new ();


	// icon for bgtiles-button
	toolbar5 = lookup_widget ( MainWindow, "toolbar5" ) ;
	
	gtk_widget_destroy ( lookup_widget ( MainWindow, "bgtiles" ) ) ;
	
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( bgtiles_xpm ) ;
	bgtiles = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar5),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("tiles"),
																	_("view as pattern"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (bgtiles);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "bgtiles", bgtiles,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (bgtiles);
	gtk_widget_add_accelerator (bgtiles, "clicked", accel_group,
												GDK_KP_5, 0,
												GTK_ACCEL_VISIBLE);
	//gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bgtiles), TRUE);


	// zoom-buttons...
	zoombox2 = lookup_widget ( MainWindow, "zoombox2" ) ;
	
	gtk_widget_destroy ( lookup_widget ( MainWindow, "zoomm25" ) ) ;
	gtk_widget_destroy ( lookup_widget ( MainWindow, "zoomp25" ) ) ;
	gtk_widget_destroy ( lookup_widget ( MainWindow, "zoom100" ) ) ;
	gtk_widget_destroy ( lookup_widget ( MainWindow, "autozoom" ) ) ;
	
	
	// decrease zoom by 25%
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_out_xpm ) ;
	zoomm25 = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_BUTTON,
																	NULL,
																	_(" -25% "),
																	_("-25%"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (zoomm25);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "zoomm25", zoomm25,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (zoomm25);
	gtk_widget_add_accelerator (zoomm25, "clicked", accel_group,
												GDK_KP_Subtract, 0,
												GTK_ACCEL_VISIBLE);
	
	
	// increase zoom by 25%
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_in_xpm ) ;
	zoomp25 = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_BUTTON,
																	NULL,
																	_(" +25% "),
																	_("+25%"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (zoomp25);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "zoomp25", zoomp25,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (zoomp25);
	gtk_widget_add_accelerator (zoomp25, "clicked", accel_group,
												GDK_KP_Add, 0,
												GTK_ACCEL_VISIBLE);
	
	
	// zoom 100%
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_fit_xpm ) ;
	zoom100 = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("100%"),
																	_("100%"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (zoom100);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "zoom100", zoom100,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (zoom100);
	gtk_widget_add_accelerator (zoom100, "clicked", accel_group,
												GDK_KP_Enter, 0,
												GTK_ACCEL_VISIBLE);
	
	
	// zoom 1:1 - monitor (DPI) dependant!
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_1_xpm ) ;
	realzoom = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("1:1"),
																	_("1:1 ( depends on your monitor )"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (realzoom);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "realzoom", realzoom,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (realzoom);
	/*gtk_widget_add_accelerator (realzoom, "clicked", accel_group,
												GDK_KP_Enter, 0,
												GTK_ACCEL_VISIBLE);*/


	// auto-fit
	//tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_fit_xpm ) ;
	autozoom = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("autozoom"),
																	_("fit image"), NULL,
																	/*tmp_toolbar_icon*/ NULL, NULL, NULL);
	gtk_widget_ref (autozoom);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "autozoom", autozoom,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (autozoom);
	gtk_widget_hide (autozoom);
	gtk_widget_add_accelerator (autozoom, "clicked", accel_group,
												GDK_KP_Multiply, 0,
												GTK_ACCEL_VISIBLE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (autozoom), TRUE);
	

	// auto-fit width
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_fit_width_xpm ) ;
	autozoomwidth = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("fit width"),
																	_("fit image by width"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (autozoomwidth);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "autozoomwidth", autozoomwidth,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (autozoomwidth);
	/*gtk_widget_add_accelerator (autozoomwidth, "clicked", accel_group,
												GDK_KP_Multiply, 0,
												GTK_ACCEL_VISIBLE);*/
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (autozoomwidth), TRUE);


	// auto-fit height
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_fit_height_xpm ) ;
	autozoomheight = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("fit height"),
																	_("fit image by height"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (autozoomheight);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "autozoomheight", autozoomheight,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (autozoomheight);
	/*gtk_widget_add_accelerator (autozoomheight, "clicked", accel_group,
												GDK_KP_Multiply, 0,
												GTK_ACCEL_VISIBLE);*/
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (autozoomheight), TRUE);


	// keepaspect
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_aspect_xpm ) ;
	keepaspect = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("keepaspect"),
																	_("always keep aspect"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (keepaspect);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "keepaspect", keepaspect,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (keepaspect);
	gtk_widget_add_accelerator (keepaspect, "clicked", accel_group,
												GDK_KP_Divide, 0,
												GTK_ACCEL_VISIBLE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (keepaspect), FALSE);


	// now connect buttons to signals - or via versa :D
	gtk_signal_connect (GTK_OBJECT (bgtiles), "toggled",
									GTK_SIGNAL_FUNC (on_bgtiles_toggled),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (zoomm25), "clicked",
									GTK_SIGNAL_FUNC (on_zoomm25_clicked),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (zoomp25), "clicked",
									GTK_SIGNAL_FUNC (on_zoomp25_clicked),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (zoom100), "toggled",
									GTK_SIGNAL_FUNC (on_zoom100_toggled),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (realzoom), "toggled",
									GTK_SIGNAL_FUNC (on_realzoom_toggled),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (autozoom), "toggled",
									GTK_SIGNAL_FUNC (on_autozoom_toggled),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (autozoomwidth), "toggled",
									GTK_SIGNAL_FUNC (on_autozoomwidth_toggled),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (autozoomheight), "toggled",
									GTK_SIGNAL_FUNC (on_autozoomheight_toggled),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (keepaspect), "toggled",
									GTK_SIGNAL_FUNC (on_keepaspect_toggled),
									MainWindow);
	
	gtk_window_add_accel_group (GTK_WINDOW (MainWindow), accel_group);

}


void
vmbutton_hide ( int button )
{
	char button1[256], button2[256], button3[256] ;
	
	sprintf ( button1, "vmbutton%d", button ) ;
	sprintf ( button2, "vmbutton%d", button + 3 ) ;
	sprintf ( button3, "vmbutton%d", button + 8 ) ;
	
	gtk_widget_hide ( lookup_widget ( MainWindow, button1 ) ) ;
	gtk_widget_hide ( lookup_widget ( MainWindow, button2 ) ) ;
	gtk_widget_hide ( lookup_widget ( MainWindow, button3 ) ) ;
}


void
vmbutton_show ( int button )
{
	char button1[256], button2[256], button3[256] ;
	
	sprintf ( button1, "vmbutton%d", button ) ;
	sprintf ( button2, "vmbutton%d", button + 3 ) ;
	sprintf ( button3, "vmbutton%d", button + 8 ) ;
	
	gtk_widget_show ( lookup_widget ( MainWindow, button1 ) ) ;
	gtk_widget_show ( lookup_widget ( MainWindow, button2 ) ) ;
	gtk_widget_show ( lookup_widget ( MainWindow, button3 ) ) ;
}


void
vmbutton_disable ( int button )
{
	char button1[256], button2[256], button3[256] ;
	
	sprintf ( button1, "vmbutton%d", button ) ;
	sprintf ( button2, "vmbutton%d", button + 3 ) ;
	sprintf ( button3, "vmbutton%d", button + 8 ) ;
	
	gtk_widget_set_sensitive ( lookup_widget ( MainWindow, button1 ), FALSE ) ;
	gtk_widget_set_sensitive ( lookup_widget ( MainWindow, button2 ), FALSE ) ;
	gtk_widget_set_sensitive ( lookup_widget ( MainWindow, button3 ), FALSE ) ;
}


void
vmbutton_enable ( int button )
{
	char button1[256], button2[256], button3[256] ;
	
	sprintf ( button1, "vmbutton%d", button ) ;
	sprintf ( button2, "vmbutton%d", button + 3 ) ;
	sprintf ( button3, "vmbutton%d", button + 8 ) ;
	
	gtk_widget_set_sensitive ( lookup_widget ( MainWindow, button1 ), TRUE ) ;
	gtk_widget_set_sensitive ( lookup_widget ( MainWindow, button2 ), TRUE ) ;
	gtk_widget_set_sensitive ( lookup_widget ( MainWindow, button3 ), TRUE ) ;
}


void
add_vmbutton_series_to_toolbar ( int toolbarnum, int use_accels )
{
	GtkWidget *tmp_toolbar_icon, *vmbutton1, *vmbutton2, *vmbutton3, *vmtoolbar ;
	GtkAccelGroup *accel_group;
	char toolbar[256], button1[256], button2[256], button3[256] ;
	
	sprintf ( toolbar, "vmtoolbar%d", toolbarnum ) ;
	sprintf ( button1, "vmbutton%d", toolbarnum * toolbarnum ) ;
	sprintf ( button2, "vmbutton%d", toolbarnum * toolbarnum + 1 ) ;
	sprintf ( button3, "vmbutton%d", toolbarnum * toolbarnum + 2 ) ;
	
	accel_group = gtk_accel_group_new ();
	
	vmtoolbar = lookup_widget ( MainWindow, toolbar ) ;
	
	gtk_widget_destroy ( lookup_widget ( MainWindow, button1 ) ) ;
	gtk_widget_destroy ( lookup_widget ( MainWindow, button2 ) ) ;
	gtk_widget_destroy ( lookup_widget ( MainWindow, button3 ) ) ;

	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( vmbutton1_xpm ) ;
	//gnome_stock_pixmap_widget (MainWindow, GNOME_STOCK_PIXMAP_INDEX);
	vmbutton1 = gtk_toolbar_append_element (GTK_TOOLBAR (vmtoolbar),
																	GTK_TOOLBAR_CHILD_BUTTON,
																	NULL,
																	_("file view"),
																	_("file view"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (vmbutton1);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), button1, vmbutton1,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vmbutton1);
	if ( use_accels )
	{
		gtk_widget_add_accelerator (vmbutton1, "clicked", accel_group,
													GDK_F10, 0,
													GTK_ACCEL_VISIBLE);
	}
	
	
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( vmbutton2_xpm ) ;
	//gnome_stock_pixmap_widget (MainWindow, GNOME_STOCK_PIXMAP_TABLE_FILL);
	vmbutton2 = gtk_toolbar_append_element (GTK_TOOLBAR (vmtoolbar),
																	GTK_TOOLBAR_CHILD_BUTTON,
																	NULL,
																	_("splitted view"),
																	_("splitted view"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (vmbutton2);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), button2, vmbutton2,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vmbutton2);
	gtk_widget_set_sensitive ( vmbutton2, FALSE ) ;
	if ( use_accels )
	{
		gtk_widget_add_accelerator (vmbutton2, "clicked", accel_group,
													GDK_F11, 0,
													GTK_ACCEL_VISIBLE);
	}
	
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( vmbutton3_xpm ) ;
	//gnome_stock_pixmap_widget (MainWindow, GNOME_STOCK_PIXMAP_COLORSELECTOR);
	vmbutton3 = gtk_toolbar_append_element (GTK_TOOLBAR (vmtoolbar),
																	GTK_TOOLBAR_CHILD_BUTTON,
																	NULL,
																	_("image view"),
																	_("image view"), NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (vmbutton3);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), button3, vmbutton3,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vmbutton3);
	if ( use_accels )
	{
		gtk_widget_add_accelerator (vmbutton3, "clicked", accel_group,
													GDK_F12, 0,
													GTK_ACCEL_VISIBLE);
	}
	
	gtk_signal_connect (GTK_OBJECT (vmbutton1), "clicked",
									GTK_SIGNAL_FUNC (on_vmbutton1_clicked),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (vmbutton2), "clicked",
									GTK_SIGNAL_FUNC (on_vmbutton2_clicked),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (vmbutton3), "clicked",
									GTK_SIGNAL_FUNC (on_vmbutton3_clicked),
									MainWindow);

	
	gtk_window_add_accel_group (GTK_WINDOW (MainWindow), accel_group);
	
	gtk_accel_group_unref ( accel_group ) ;
	
}
