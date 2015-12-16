#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "interface.h"
#include "support.h"

#include "mainwindow.h"
#include "callbacks.h"

#include "look_at_the_stars_icon.xpm"

// the following variables can be exported into other .c files
GtkWidget *MainWindow ;

GtkWidget *file_statusbar ;
GtkWidget *file_tree ;
GtkWidget *file_icons ;

GtkWidget *image_canvas ;
GtkWidget *image_statusbar ;
GtkWidget *image_multipage_box ;
GtkWidget *image_multipage_progressbar ;
//

// imported from dirlist.c
GtkTreeStore extern *file_tree_store ;
//


static GnomeUIInfo file_menu_uiinfo[] =
{
  /*GNOMEUIINFO_MENU_NEW_ITEM (N_("_New File"), NULL, on_new_file1_activate, NULL),
  GNOMEUIINFO_MENU_OPEN_ITEM (on_open1_activate, NULL),
  GNOMEUIINFO_MENU_SAVE_ITEM (on_save1_activate, NULL),
  GNOMEUIINFO_MENU_SAVE_AS_ITEM (on_save_as1_activate, NULL),
  GNOMEUIINFO_SEPARATOR,*/
  GNOMEUIINFO_MENU_EXIT_ITEM (quit_lats, NULL),
  GNOMEUIINFO_END
};


static GnomeUIInfo menubar1_uiinfo[] =
{
  GNOMEUIINFO_MENU_FILE_TREE (file_menu_uiinfo),
  /*GNOMEUIINFO_MENU_EDIT_TREE (edit_menu_uiinfo),
  GNOMEUIINFO_MENU_VIEW_TREE (view_menu_uiinfo),
  GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu_uiinfo),
  GNOMEUIINFO_MENU_HELP_TREE (help_menu_uiinfo),*/
  GNOMEUIINFO_END
};


void
create_mainwindow ( void )
{
	GtkWidget *file_table ;
	GtkWidget *image_table ;
	GtkWidget *tmp_toolbar_icon ;
	GtkTooltips *tooltips ;
	GtkWidget *hpane ;
	
	tooltips = gtk_tooltips_new () ;
	
	// first our mainwindow-framework :)
	MainWindow = gnome_app_new ( "lats-2", g_strdup_printf(_("Look at the stars %s"), VERSION) ) ;
	
	gnome_app_enable_layout_config ( GNOME_APP(MainWindow), TRUE ) ;
	
	gtk_window_set_position ( GTK_WINDOW(MainWindow), GTK_WIN_POS_CENTER ) ;
	gtk_window_set_default_size ( GTK_WINDOW(MainWindow), 640, 480 ) ;
	
	gtk_widget_realize ( MainWindow ) ;

	gnome_app_create_menus (GNOME_APP (MainWindow), menubar1_uiinfo);
	
	gtk_signal_connect (GTK_OBJECT (MainWindow), "delete_event",
						GTK_SIGNAL_FUNC (quit_lats),
						NULL);
	gtk_signal_connect (GTK_OBJECT (MainWindow), "destroy_event",
						GTK_SIGNAL_FUNC (quit_lats),
						NULL);
	
	// now we'll devide it using a table 
	//    'file_table'     and    'image-table'
	// 
	// +------------------+ | +---------------------+---+
	// | file-toolbar   1 | | | image-toolbar         2 |
	// +------------------+ | +---------------------+---+
	// |                  | | |                     |   |
	// | directory      3 | | | our image         4 | 4 | <- 4b) multi-page
	// | entries          | | |                     | b |        display
	// |                  | | |                     |   |
	// |                  | | |                     |   |
	// |                  | | |                     |   |
	// |                  | | |                     |   |
	// +------------------+ | +---------------------+---+
	// | file-statusbar 5 | | | image-statusbar       6 |
	// +------------------+ | +---------------------+---+
	//
	// both will be combined by a horizontal pane

	// first we put our hpane into our gnome-app
	hpane = gtk_hpaned_new () ;
	gtk_widget_ref ( hpane ) ;
	gtk_object_set_data_full ( GTK_OBJECT(MainWindow), 
								"hpane", 
								hpane, 
								(GtkDestroyNotify) gtk_widget_unref ) ;
	gtk_widget_show ( hpane ) ;
	gnome_app_set_contents ( GNOME_APP(MainWindow), hpane ) ;
	
	// now we put our file_table into the hpaned on the left
	file_table = gtk_table_new ( 2, 1, FALSE ) ;
	gtk_widget_ref ( file_table ) ;
	gtk_paned_pack1 ( GTK_PANED(hpane), file_table, TRUE, TRUE ) ;
	gtk_widget_show ( file_table ) ;

	// and now the file_table into the hpaned on the right
	image_table = gtk_table_new ( 2, 2, FALSE ) ;
	gtk_widget_ref ( image_table ) ;
	gtk_paned_pack2 ( GTK_PANED(hpane), image_table, TRUE, TRUE ) ;
	gtk_widget_show ( image_table ) ;
	
	// now try to get the left pane a little bit smaller for the beginning...
	gtk_paned_set_position ( GTK_PANED(hpane), 250 ) ;

	// * * * * *
	
	{	// now we'll create the file-toolbar (1)
		GtkWidget *toolbar ;
		
		toolbar = gtk_toolbar_new () ;
		gtk_widget_ref ( toolbar ) ;
		gtk_object_set_data_full ( GTK_OBJECT(MainWindow),
									"file_toolbar",
									toolbar,
									(GtkDestroyNotify) gtk_widget_unref ) ;
		gtk_widget_show ( toolbar ) ;
		gtk_toolbar_set_style ( GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS ) ;
		
		gtk_table_attach ( GTK_TABLE(file_table), toolbar, 0, 1, 0, 1,
							(GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
							(GtkAttachOptions) (0), 0, 0 ) ;
		
		{	// put the previous button into it
			GtkWidget *previous_button ;
		
			tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-go-back", 
									gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
			previous_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
															GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
															NULL, // widget
															_("previous file"), // label
															_("go to previous file"), // tooltip
															NULL, // private tooltip
															tmp_toolbar_icon,
															NULL, // callback
															NULL, // user_data
															-1 // position; -1 = end
															) ;
			gtk_widget_show ( previous_button ) ;
		}	// previous-button
		{	// put the next button into it
			GtkWidget *next_button ;
		
			tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-go-forward", 
									gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
			next_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
															GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
															NULL, // widget
															_("next file"), // label
															_("go to next file"), // tooltip
															NULL, // private tooltip
															tmp_toolbar_icon,
															NULL, // callback
															NULL, // user_data
															-1 // position; -1 = end
															) ;
			gtk_widget_show ( next_button ) ;
		}	// next-button
		{	// put the up button into it
			GtkWidget *up_button ;
		
			tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-go-up", 
									gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
			up_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
															GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
															NULL, // widget
															_("parent dir"), // label
															_("go up one directory in tree"), // tooltip
															NULL, // private tooltip
															tmp_toolbar_icon,
															NULL, // callback
															NULL, // user_data
															-1 // position; -1 = end
															) ;
			gtk_widget_show ( up_button ) ;
		}	// up-button
		{	// put the home button into it
			GtkWidget *home_button ;
		
			tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-home", 
									gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
			home_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
															GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
															NULL, // widget
															_("home dir"), // label
															_("go to your home directory"), // tooltip
															NULL, // private tooltip
															tmp_toolbar_icon,
															NULL, // callback
															NULL, // user_data
															-1 // position; -1 = end
															) ;
			gtk_widget_show ( home_button ) ;
			gtk_signal_connect ( GTK_OBJECT(home_button), "clicked",
						GTK_SIGNAL_FUNC (on_home_button_clicked),
						NULL ) ;
		}	// home-button
		{	// put the reload button into it
			GtkWidget *reload_button ;
		
			tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-refresh", 
									gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
			reload_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
															GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
															NULL, // widget
															_("reload dir"), // label
															_("reload current dir and thumbnails"), // tooltip
															NULL, // private tooltip
															tmp_toolbar_icon,
															NULL, // callback
															NULL, // user_data
															-1 // position; -1 = end
															) ;
			gtk_widget_show ( reload_button ) ;
		}	// reload-button
		{	// put the stop button into it
			GtkWidget *stop_button ;
		
			tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-cancel", 
									gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
			stop_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
															GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
															NULL, // widget
															_("stop reading dir"), // label
															_("stop reading current dir and thumbnails"), // tooltip
															NULL, // private tooltip
															tmp_toolbar_icon,
															NULL, // callback
															NULL, // user_data
															-1 // position; -1 = end
															) ;
			gtk_widget_show ( stop_button ) ;
			gtk_widget_set_sensitive ( stop_button, FALSE ) ;
		}	// stop-button
	}	// file-toolbar
	
	// * * * * *
	
	{	// now we'll create the image-toolbar (2)
		GtkWidget *toolbar ;
		
		toolbar = gtk_toolbar_new () ;
		gtk_widget_ref ( toolbar ) ;
		gtk_object_set_data_full ( GTK_OBJECT(MainWindow),
									"image_toolbar",
									toolbar,
									(GtkDestroyNotify) gtk_widget_unref ) ;
		gtk_widget_show ( toolbar ) ;
		gtk_toolbar_set_style ( GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS ) ;
		
		gtk_table_attach ( GTK_TABLE(image_table), toolbar, 0, 2, 0, 1,
							(GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
							(GtkAttachOptions) (0), 0, 0 ) ;
		
		{	// put the reload button into it
			GtkWidget *reload_button ;
		
			tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-refresh", 
									gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
			reload_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
															GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
															NULL, // widget
															_("reload image"), // label
															_("reload current image"), // tooltip
															NULL, // private tooltip
															tmp_toolbar_icon,
															NULL, // callback
															NULL, // user_data
															-1 // position; -1 = end
															) ;
			gtk_widget_show ( reload_button ) ;
		}	// reload-button
		{	// put the stop button into it
			GtkWidget *stop_button ;
		
			tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-cancel", 
									gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
			stop_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
															GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
															NULL, // widget
															_("stop image-processing"), // label
															_("stop loading or processing an image"), // tooltip
															NULL, // private tooltip
															tmp_toolbar_icon,
															NULL, // callback
															NULL, // user_data
															-1 // position; -1 = end
															) ;
			gtk_widget_show ( stop_button ) ;
			gtk_widget_set_sensitive ( stop_button, FALSE ) ;
		}	// stop-button
		{	// a seperator
			gtk_toolbar_append_space ( GTK_TOOLBAR(toolbar) ) ;
		}	// a seperator
		{	// some zoom-buttons
			GtkWidget *zoom_entry = gtk_entry_new () ; // has to be prepared first!
					
			{	// put the zoom-smaller-button into it
				GtkWidget *zoom_smaller_button ;
			
				tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-zoom-out", 
										gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
				zoom_smaller_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
																GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
																NULL, // widget
																_("display smaller"), // label
																_("display image a bit smaller"), // tooltip
																NULL, // private tooltip
																tmp_toolbar_icon,
																NULL, // callback
																NULL, // user_data
																-1 // position; -1 = end
																) ;
				gtk_widget_show ( zoom_smaller_button ) ;
				gtk_signal_connect ( GTK_OBJECT(zoom_smaller_button), "clicked",
										GTK_SIGNAL_FUNC (on_zoom_smaller_button_clicked),
										GTK_OBJECT(zoom_entry) ) ;
			}	// zoom-smaller-button
			{	// put the page zoom into it
				gtk_widget_show ( zoom_entry ) ;
				gtk_toolbar_insert_widget ( GTK_TOOLBAR(toolbar), // toolbar
											zoom_entry, // widget
											_("enter zoom in percent here \n( 100 = normal ; 50 = half-size ; \n200 = double-size ; 1 = min ; \n800 = max ; 1:1 = true to scale )"), // tooltip
											NULL, // private tooltip
											-1 // position; -1 = end
											) ;
				gtk_entry_set_width_chars ( GTK_ENTRY(zoom_entry), 4 ) ;
				gtk_entry_set_text ( GTK_ENTRY(zoom_entry), "100" ) ;
				gtk_signal_connect ( GTK_OBJECT(zoom_entry), "activate",
										GTK_SIGNAL_FUNC (on_zoom_entry_changed),
										NULL ) ;
			}
			{	// put the zoom-bigger-button into it
				GtkWidget *zoom_bigger_button ;
			
				tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-zoom-in", 
										gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
				zoom_bigger_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
																GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
																NULL, // widget
																_("display bigger"), // label
																_("display image a bit bigger"), // tooltip
																NULL, // private tooltip
																tmp_toolbar_icon,
																NULL, // callback
																NULL, // user_data
																-1 // position; -1 = end
																) ;
				gtk_widget_show ( zoom_bigger_button ) ;
				gtk_signal_connect ( GTK_OBJECT(zoom_bigger_button), "clicked",
										GTK_SIGNAL_FUNC (on_zoom_bigger_button_clicked),
										GTK_OBJECT(zoom_entry) ) ;
			}	// zoom-bigger-button
			{	// put the zoom-100%-button into it
				GtkWidget *zoom_100_button ;
			
				tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-zoom-100", 
										gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
				zoom_100_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
																GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
																NULL, // widget
																_("display at 100%"), // label
																_("display image at full size"), // tooltip
																NULL, // private tooltip
																tmp_toolbar_icon,
																NULL, // callback
																NULL, // user_data
																-1 // position; -1 = end
																) ;
				gtk_widget_show ( zoom_100_button ) ;
				gtk_signal_connect ( GTK_OBJECT(zoom_100_button), "clicked",
										GTK_SIGNAL_FUNC (on_zoom_100_button_clicked),
										GTK_OBJECT(zoom_entry) ) ;
			}	// zoom-100%-button
			{	// put the zoom-fit-button into it
				GtkWidget *zoom_fit_button ;
			
				tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-zoom-fit", 
										gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
				zoom_fit_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
																GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
																NULL, // widget
																_("fit to display"), // label
																_("fit image to display-size"), // tooltip
																NULL, // private tooltip
																tmp_toolbar_icon,
																NULL, // callback
																NULL, // user_data
																-1 // position; -1 = end
																) ;
				gtk_widget_show ( zoom_fit_button ) ;
				gtk_signal_connect ( GTK_OBJECT(zoom_fit_button), "clicked",
										GTK_SIGNAL_FUNC (on_zoom_fit_button_clicked),
										GTK_OBJECT(zoom_entry) ) ;
			}	// zoom-fit-button
		} // some zoom-buttons
	}	// image-toolbar
	
	// * * * * *
	
	{	// directory entries (3)
		GtkWidget *notebook ;
		
		// init the notebook-widget
		notebook = gtk_notebook_new () ;
		gtk_widget_ref ( notebook ) ;
		gtk_object_set_data_full ( GTK_OBJECT(notebook),
									"file_notebook",
									notebook,
									(GtkDestroyNotify) gtk_widget_unref ) ;
		gtk_widget_show ( notebook ) ;
		gtk_table_attach ( GTK_TABLE(file_table), notebook, 0, 1, 1, 2,
							(GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
							(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0 ) ;
		
		{	// page 1 - directory-listing
			GtkWidget *page ;
			GtkWidget *label, *label_image, *label_text ;

			// setup page-frame
			page = gtk_vpaned_new () ;
			gtk_widget_show ( page ) ;
			gtk_container_add ( GTK_CONTAINER(notebook), page ) ;
			
			// setup page-label
			label = gtk_hbox_new ( FALSE, 0 ) ;
			label_image = gtk_image_new_from_stock ( "gnome-stock-multiple-file",
								GTK_ICON_SIZE_BUTTON ) ;
			label_text = gtk_label_new ( _("files") ) ;
			gtk_container_add ( GTK_CONTAINER(label), label_image ) ;
			gtk_container_add ( GTK_CONTAINER(label), label_text ) ;
			gtk_misc_set_padding ( GTK_MISC(label_text), 5, 0 ) ;
			gtk_widget_show ( label ) ;
			gtk_widget_show ( label_image ) ;
			gtk_widget_show ( label_text ) ;
			gtk_notebook_set_tab_label ( GTK_NOTEBOOK(notebook), 
						gtk_notebook_get_nth_page ( GTK_NOTEBOOK(notebook), 0 ),
						label ) ;
			gtk_label_set_justify ( GTK_LABEL(label_text), GTK_JUSTIFY_LEFT ) ;
			
			{	// now the page itself...
				GtkWidget *dir_tree_frame, *dir_tree_window, *dir_tree ;
				GtkWidget *icon_list_frame, *icon_list_window, *icon_list ;
				
				// dir-tree
				dir_tree_frame = gtk_frame_new ( _("directory tree") ) ;
				gtk_widget_show ( dir_tree_frame ) ;
				dir_tree_window = gtk_scrolled_window_new ( NULL, NULL ) ;
				gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW(dir_tree_window), 
													GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC ) ;
				gtk_widget_show ( dir_tree_window ) ;
				dir_tree = gtk_tree_view_new () ;
				gtk_widget_show ( dir_tree ) ;
				gtk_container_add ( GTK_CONTAINER(dir_tree_window), dir_tree ) ;
				gtk_container_add ( GTK_CONTAINER(dir_tree_frame), dir_tree_window ) ;
				//gtk_container_add ( GTK_CONTAINER(page), dir_tree_frame ) ;
				gtk_paned_pack1 ( GTK_PANED(page), dir_tree_frame, TRUE, TRUE ) ;
				gtk_container_set_border_width ( GTK_CONTAINER(dir_tree_window), 5 ) ;
				gtk_container_set_border_width ( GTK_CONTAINER(dir_tree_frame), 5 ) ;
				GTK_WIDGET_SET_FLAGS ( dir_tree, GTK_CAN_DEFAULT ) ;
				gtk_widget_grab_focus ( dir_tree ) ;
				gtk_widget_grab_default ( dir_tree ) ;
				
				// icon-list
				icon_list_frame = gtk_frame_new ( _("viewable files") ) ;
				gtk_widget_show ( icon_list_frame ) ;
				icon_list_window = gtk_scrolled_window_new ( NULL, NULL ) ;
				gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW(icon_list_window), 
													GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC ) ;
				gtk_widget_show ( icon_list_window ) ;
				icon_list = gnome_icon_list_new ( 78, NULL, 0 ) ;
				gtk_widget_show ( icon_list ) ;
				gtk_container_add ( GTK_CONTAINER(icon_list_window), icon_list ) ;
				gtk_container_add ( GTK_CONTAINER(icon_list_frame), icon_list_window ) ;
				//gtk_container_add ( GTK_CONTAINER(page), icon_list_frame ) ;
				gtk_paned_pack2 ( GTK_PANED(page), icon_list_frame, TRUE, TRUE ) ;
				gtk_container_set_border_width ( GTK_CONTAINER(icon_list_window), 5 ) ;
				gtk_container_set_border_width ( GTK_CONTAINER(icon_list_frame), 5 ) ;
				
				file_tree = dir_tree ;
				file_icons = icon_list ;
			}
		}	// page 1 - directory-listing
		{	// page 2 - image properties
			GtkWidget *page ;
			GtkWidget *label, *label_image, *label_text ;

			// setup page-frame
			page = gtk_vbox_new ( FALSE, 0 ) ;
			gtk_widget_show ( page ) ;
			gtk_container_add ( GTK_CONTAINER(notebook), page ) ;
			
			// setup page-label
			label = gtk_hbox_new ( FALSE, 0 ) ;
			label_image = gtk_image_new_from_stock ( "gtk-properties",
								GTK_ICON_SIZE_BUTTON ) ;
			label_text = gtk_label_new ( _("properties") ) ;
			gtk_container_add ( GTK_CONTAINER(label), label_image ) ;
			gtk_container_add ( GTK_CONTAINER(label), label_text ) ;
			gtk_misc_set_padding ( GTK_MISC(label_text), 5, 0 ) ;
			gtk_widget_show ( label ) ;
			gtk_widget_show ( label_image ) ;
			gtk_widget_show ( label_text ) ;
			gtk_notebook_set_tab_label ( GTK_NOTEBOOK(notebook), 
						gtk_notebook_get_nth_page ( GTK_NOTEBOOK(notebook), 1 ),
						label ) ;
			gtk_label_set_justify ( GTK_LABEL(label_text), GTK_JUSTIFY_LEFT ) ;
			
			{	// now the page itself...
				
			}
		}	// page 2 - image properties
		{	// page 3 - image manipulation
			GtkWidget *page ;
			GtkWidget *label, *label_image, *label_text ;

			// setup page-frame
			page = gtk_vbox_new ( FALSE, 0 ) ;
			gtk_widget_show ( page ) ;
			gtk_container_add ( GTK_CONTAINER(notebook), page ) ;
			
			// setup page-label
			label = gtk_hbox_new ( FALSE, 0 ) ;
			label_image = gtk_image_new_from_stock ( "gtk-select-color",
								GTK_ICON_SIZE_BUTTON ) ;
			label_text = gtk_label_new ( _("image \nmanipulation") ) ;
			gtk_container_add ( GTK_CONTAINER(label), label_image ) ;
			//gtk_container_add ( GTK_CONTAINER(label), label_text ) ;
			gtk_misc_set_padding ( GTK_MISC(label_text), 5, 0 ) ;
			gtk_widget_show ( label ) ;
			gtk_widget_show ( label_image ) ;
			gtk_widget_show ( label_text ) ;
			gtk_notebook_set_tab_label ( GTK_NOTEBOOK(notebook), 
						gtk_notebook_get_nth_page ( GTK_NOTEBOOK(notebook), 2 ),
						label ) ;
			gtk_label_set_justify ( GTK_LABEL(label_text), GTK_JUSTIFY_LEFT ) ;
			
			{	// now the page itself...
				
			}
		}	// page 3 - image manipulation
	}	// directory entries (3)
	
	// * * * * *
	
	{	// image (4)
		GtkWidget *image_window, *image ;
		
		image_window = gtk_scrolled_window_new ( NULL, NULL ) ;
		gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW(image_window), 
											GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC ) ;
		gtk_widget_show ( image_window ) ;

        gdk_rgb_init ( ) ;
        gtk_widget_push_visual ( gdk_rgb_get_visual ( ) ) ;
        gtk_widget_push_colormap ( gdk_rgb_get_cmap ( ) ) ;
        //image = gnome_canvas_new ( ) ;
		image = gnome_canvas_new_aa ( ) ;
        gtk_widget_pop_colormap ( ) ;
        gtk_widget_pop_visual ( ) ;
        gtk_widget_ref ( image ) ;
        gtk_object_set_data_full ( GTK_OBJECT (MainWindow), 
									"image", image, (GtkDestroyNotify) gtk_widget_unref ) ;
		gtk_widget_show ( image ) ;
		
		gtk_container_add ( GTK_CONTAINER(image_window), image ) ;
		gnome_canvas_set_scroll_region ( GNOME_CANVAS(image), 0, 0, 100, 100 ) ;
		gtk_table_attach ( GTK_TABLE(image_table), image_window, 0, 1, 1, 2,
							(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
							(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0 ) ;
		
		image_canvas = image ;
	}	// image (4)
	
	// * * * * *
	
	{	// multi-page display (4b)
		GtkWidget *mp_vbox, *mp_window, *mp, *mp_progressbar ;
		
		mp_vbox = gtk_vbox_new ( FALSE, 0 ) ;
		gtk_widget_show ( mp_vbox ) ;
		gtk_table_attach ( GTK_TABLE(image_table), mp_vbox, 1, 2, 1, 2,
							(GtkAttachOptions) (GTK_FILL),
							(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0 ) ;
		
		{	// now we'll create the multipage-toolbar
			GtkWidget *toolbar ;
			
			toolbar = gtk_toolbar_new () ;
			gtk_widget_ref ( toolbar ) ;
			gtk_object_set_data_full ( GTK_OBJECT(MainWindow),
										"multi_page_toolbar",
										toolbar,
										(GtkDestroyNotify) gtk_widget_unref ) ;
			gtk_widget_show ( toolbar ) ;
			gtk_toolbar_set_style ( GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS ) ;
			
			gtk_box_pack_start ( GTK_BOX(mp_vbox), toolbar, FALSE, TRUE, 0 ) ;
			
			{	// put the previous button into it
				GtkWidget *previous_button ;
			
				tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-go-back", 
										gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
				previous_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
																GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
																NULL, // widget
																_("previous page"), // label
																_("go to previous page"), // tooltip
																NULL, // private tooltip
																tmp_toolbar_icon,
																NULL, // callback
																NULL, // user_data
																-1 // position; -1 = end
																) ;
				gtk_widget_show ( previous_button ) ;
			}	// previous-button
			{	// put the page counter into it
				GtkWidget *counter_entry ;
				
				counter_entry = gtk_entry_new () ;
				gtk_widget_show ( counter_entry ) ;
				gtk_toolbar_insert_widget ( GTK_TOOLBAR(toolbar), // toolbar
											counter_entry, // widget
											_("enter pagenumber to display"), // tooltip
											NULL, // private tooltip
											-1 // position; -1 = end
											) ;
				gtk_entry_set_width_chars ( GTK_ENTRY(counter_entry), 4 ) ;
				gtk_entry_set_text ( GTK_ENTRY(counter_entry), "1" ) ;
			}
			{	// put the next button into it
				GtkWidget *next_button ;
			
				tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-go-forward", 
										gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
				next_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
																GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
																NULL, // widget
																_("next page"), // label
																_("go to next page"), // tooltip
																NULL, // private tooltip
																tmp_toolbar_icon,
																NULL, // callback
																NULL, // user_data
																-1 // position; -1 = end
																) ;
				gtk_widget_show ( next_button ) ;
			}	// next-button
			{	// a seperator
				gtk_toolbar_append_space ( GTK_TOOLBAR(toolbar) ) ;
			}	// a seperator
			{	// put the reload button into it
				GtkWidget *reload_button ;
			
				tmp_toolbar_icon = gtk_image_new_from_stock ( "gtk-refresh", 
										gtk_toolbar_get_icon_size ( GTK_TOOLBAR(toolbar) ) ) ;
				reload_button = gtk_toolbar_insert_element ( GTK_TOOLBAR(toolbar), // toolbar
																GTK_TOOLBAR_CHILD_BUTTON, // childbutton-type
																NULL, // widget
																_("recreate"), // label
																_("recreate pages"), // tooltip
																NULL, // private tooltip
																tmp_toolbar_icon,
																NULL, // callback
																NULL, // user_data
																-1 // position; -1 = end
																) ;
				gtk_widget_show ( reload_button ) ;
			}	// reload-button
		}	// multi-page-toolbar
		
		mp_window = gtk_scrolled_window_new ( NULL, NULL ) ;
		gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW(mp_window), 
											GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC ) ;
		gtk_widget_show ( mp_window ) ;
		mp = gnome_icon_list_new ( 78, NULL, 0 ) ;
		gtk_widget_show ( mp ) ;
		gtk_container_add ( GTK_CONTAINER(mp_window), mp ) ;
		gtk_box_pack_start ( GTK_BOX(mp_vbox), mp_window, TRUE, TRUE, 0 ) ;
		
		mp_progressbar = gtk_progress_bar_new () ;
		gtk_widget_show ( mp_progressbar ) ;
		gtk_box_pack_start ( GTK_BOX(mp_vbox), mp_progressbar, FALSE, TRUE, 0 ) ;
		
		gtk_widget_hide ( mp_vbox ) ; // initially hide the page-switcher!
		
		image_multipage_box = mp_vbox ;
		image_multipage_progressbar = mp_progressbar ;
	}	// multi-page display (4b)
	
	// * * * * *
	
	{	// file-statusbar (5)
		GtkWidget *statusbar ;
		
		statusbar = gtk_statusbar_new () ;
		gtk_widget_ref ( statusbar ) ;
		gtk_object_set_data_full ( GTK_OBJECT(statusbar),
									"file_statusbar",
									statusbar,
									(GtkDestroyNotify) gtk_widget_unref ) ;
		gtk_statusbar_set_has_resize_grip ( GTK_STATUSBAR(statusbar), FALSE ) ;
		gtk_widget_show ( statusbar ) ;
		gtk_table_attach ( GTK_TABLE(file_table), statusbar, 0, 1, 2, 3,
							(GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
							(GtkAttachOptions) (0), 0, 0 ) ;
		
		file_statusbar = statusbar ;
	}	// file-statusbar (5)
	
	// * * * * *
	
	{	// image-statusbar (6)
		GtkWidget *statusbar ;
		
		statusbar = gtk_statusbar_new () ;
		gtk_widget_ref ( statusbar ) ;
		gtk_object_set_data_full ( GTK_OBJECT(statusbar),
									"image_statusbar",
									statusbar,
									(GtkDestroyNotify) gtk_widget_unref ) ;
		gtk_widget_show ( statusbar ) ;
		gtk_table_attach ( GTK_TABLE(image_table), statusbar, 0, 2, 2, 3,
							(GtkAttachOptions) (GTK_FILL),
							(GtkAttachOptions) (0), 0, 0 ) ;
		
		image_statusbar = statusbar ;
	}	// image-statusbar (6)
	
	gtk_tooltips_enable ( tooltips ) ;
}	// create mainwindow


void
display_mainwindow ( void )
{
	gtk_widget_show ( MainWindow ) ;
	refresh_screen () ;
}


void
quit_lats ( void )
{
	gtk_main_quit () ;
}

