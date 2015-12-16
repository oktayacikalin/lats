#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "dirlist.h"
#include "imageview.h"

// imported from mainwindow.c
GtkWidget extern *file_statusbar ;
GtkWidget extern *file_tree ;
GtkWidget extern *file_icons ;

GtkWidget extern *MainWindow ;

GtkWidget extern *image_canvas ;
GtkWidget extern *image_statusbar ;
GtkWidget extern *image_multipage_box ;
GtkWidget extern *image_multipage_progressbar ;
//

// imported from dirlist.c
GtkTreeStore extern *file_tree_store ;
//


void
on_home_button_clicked ( GtkButton *button, gpointer user_data )
{
	char dest_path[2048] ;
	GtkTreePath *tree_path ;
	
	sprintf ( dest_path, "%s", gnome_vfs_expand_initial_tilde("~/") ) ;
	
	g_print ( _("home dir is %s\n"), dest_path ) ;
	
	tree_path = expand_whole_dir_path ( file_tree_store, dest_path ) ;
	
	g_print ( _("tree_path is %s\n"), gtk_tree_path_to_string ( tree_path ) ) ;
	
	g_print ( _("select tree_path...\n") ) ;
	gtk_tree_selection_select_path (	gtk_tree_view_get_selection ( GTK_TREE_VIEW(file_tree) ),
										tree_path ) ;
}


void
on_zoom_smaller_button_clicked ( GtkButton *button, gpointer user_data )
{
	gchar *value ;
	int new_value ;
	
	value = gtk_editable_get_chars ( GTK_EDITABLE(user_data), 0, -1 ) ;
	
	new_value = atoi(value)-25 ;
	
	if ( new_value <= 0 )
		new_value = 1 ; // our minimum
	
	gtk_entry_set_text ( GTK_ENTRY(user_data), g_strdup_printf("%d", new_value) ) ;
	
	g_free ( value ) ;
	gtk_widget_activate ( user_data ) ;
	
	g_print ( _("zoom out!\n") ) ;
}	//	on_zoom_smaller_button_clicked

void
on_zoom_entry_changed ( GtkEntry *entry, gpointer user_data )
{
	gchar *value ;
	Slide_Model slide = get_slide_model ( "plain" ) ;
	double zoom_divider = ((double) 100) * ((double) slide.image_quality ) ;
	
	value = gtk_editable_get_chars ( GTK_EDITABLE(entry), 0, -1 ) ;
	
	g_print ( _("zoom_entry has a new value = %d !\n"), atoi(value) ) ;
	
	gnome_canvas_set_pixels_per_unit ( GNOME_CANVAS(image_canvas), (double)atoi(value) / zoom_divider ) ;
	center_image_canvas ( GNOME_CANVAS(image_canvas) ) ;
	
	g_free ( value ) ;
}	//	on_zoom_entry_changed

void
on_zoom_bigger_button_clicked ( GtkButton *button, gpointer user_data )
{
	gchar *value ;
	int new_value ;
	
	value = gtk_editable_get_chars ( GTK_EDITABLE(user_data), 0, -1 ) ;

	new_value = atoi(value)+25 ;
	
	if ( new_value >= 800 )
		new_value = 800 ; // our maximum

	gtk_entry_set_text ( GTK_ENTRY(user_data), g_strdup_printf("%d", new_value) ) ;
	
	g_free ( value ) ;
	gtk_widget_activate ( user_data ) ;
	
	g_print ( _("zoom in!\n") ) ;
}	//	on_zoom_bigger_button_clicked

void
on_zoom_100_button_clicked ( GtkButton *button, gpointer user_data )
{
	gtk_entry_set_text ( GTK_ENTRY(user_data), g_strdup_printf("%d", 100) ) ;
	
	gtk_widget_activate ( user_data ) ;
	
	g_print ( _("zoom to full size (100%) !\n") ) ;
}	//	on_zoom_100_button_clicked

void
on_zoom_fit_button_clicked ( GtkButton *button, gpointer user_data )
{
	g_print ( _("zoom to fit!\n") ) ;
}	//	on_zoom_fit_button_clicked

