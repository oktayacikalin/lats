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

#include "stock-zoom-1.xpm"
#include "stock-zoom-fit.xpm"
#include "stock-zoom-in.xpm"
#include "stock-zoom-out.xpm"

#include "vmbutton1.xpm"
#include "vmbutton2.xpm"
#include "vmbutton3.xpm"

GtkWidget extern *MainWindow ;

void 
add_zoom_buttons_to_toolbar ( void ) 
{
	GtkWidget *tmp_toolbar_icon;
	GtkWidget *zoombox2;
	GtkWidget *zoomm25;
	GtkWidget *zoomp25;
	GtkWidget *zoom100;
	GtkWidget *autozoom;
	GtkWidget *keepaspect;
	GtkAccelGroup *accel_group;
	
	accel_group = gtk_accel_group_new ();

	zoombox2 = lookup_widget ( MainWindow, "zoombox2" ) ;
	
	gtk_widget_destroy ( lookup_widget ( MainWindow, "zoomm25" ) ) ;
	gtk_widget_destroy ( lookup_widget ( MainWindow, "zoomp25" ) ) ;
	gtk_widget_destroy ( lookup_widget ( MainWindow, "zoom100" ) ) ;
	gtk_widget_destroy ( lookup_widget ( MainWindow, "autozoom" ) ) ;
	
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_out_xpm ) ;
	//create_pixmap (MainWindow, "look_at_the_stars/stock-zoom-out.xpm", TRUE);
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
	
	
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_in_xpm ) ;
	//create_pixmap (MainWindow, "look_at_the_stars/stock-zoom-in.xpm", TRUE);
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
	
	
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_1_xpm ) ;
	//create_pixmap (MainWindow, "look_at_the_stars/stock-zoom-1.xpm", TRUE);
	zoom100 = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("1:1"),
																	NULL, NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (zoom100);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "zoom100", zoom100,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (zoom100);
	gtk_widget_add_accelerator (zoom100, "clicked", accel_group,
												GDK_KP_Enter, 0,
												GTK_ACCEL_VISIBLE);
	
	
	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_fit_xpm ) ;
	//create_pixmap (MainWindow, "look_at_the_stars/stock-zoom-fit.xpm", TRUE);
	autozoom = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("autozoom"),
																	NULL, NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (autozoom);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "autozoom", autozoom,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (autozoom);
	gtk_widget_add_accelerator (autozoom, "clicked", accel_group,
												GDK_KP_Multiply, 0,
												GTK_ACCEL_VISIBLE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (autozoom), TRUE);
	

	tmp_toolbar_icon = gnome_pixmap_new_from_xpm_d ( stock_zoom_fit_xpm ) ;
	//create_pixmap (MainWindow, "look_at_the_stars/stock-zoom-fit.xpm", TRUE);
	keepaspect = gtk_toolbar_append_element (GTK_TOOLBAR (zoombox2),
																	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
																	NULL,
																	_("keepaspect"),
																	NULL, NULL,
																	tmp_toolbar_icon, NULL, NULL);
	gtk_widget_ref (keepaspect);
	gtk_object_set_data_full (GTK_OBJECT (MainWindow), "keepaspect", keepaspect,
											(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (keepaspect);
	gtk_widget_add_accelerator (keepaspect, "clicked", accel_group,
												GDK_KP_Divide, 0,
												GTK_ACCEL_VISIBLE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (keepaspect), FALSE);


	gtk_signal_connect (GTK_OBJECT (zoomm25), "clicked",
									GTK_SIGNAL_FUNC (on_zoomm25_clicked),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (zoomp25), "clicked",
									GTK_SIGNAL_FUNC (on_zoomp25_clicked),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (zoom100), "toggled",
									GTK_SIGNAL_FUNC (on_zoom100_toggled),
									MainWindow);
	gtk_signal_connect (GTK_OBJECT (autozoom), "toggled",
									GTK_SIGNAL_FUNC (on_autozoom_toggled),
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
