#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <sys/stat.h>

#include "interface.h"
#include "support.h"

#include "imageview.h"
#include "tasks.h"
#include "text.h"

#include "lats_slide_template.h"
#include "brokenimage.xpm"

// imported from mainwindow.c
GtkWidget extern *MainWindow ;

GtkWidget extern *image_canvas ;
GtkWidget extern *image_statusbar ;
GtkWidget extern *image_multipage_box ;
GtkWidget extern *image_multipage_progressbar ;
//

// default image background-color
int red = 242 ;
int green = 240 ;
int blue = 225 ;
//

// imported from tasks.c
int extern thread_count ; // threads
//

Mouse_Position Mouse ;

GList *Slide_list = NULL ;
GList *Slide_case_cache = NULL ;

gchar
*exchange_char ( char *name, int search_char, int dest_char )
{
	char temp_name[2048] ;
	int pos = 0 ;
	
	sprintf ( temp_name, "%s", name ) ;
	
	for ( pos = 0 ; pos < strlen(temp_name) ; pos++ )
	{
		if ( temp_name[pos] == 47 ) temp_name[pos] = 183 ;
	}
	
	return g_strdup ( temp_name ) ;
}


GdkPixbuf
*get_thumbnail ( char *filename, char *model )
{	
	GdkPixbuf *image_pixbuf ;
	GdkPixbuf *return_pixbuf ;
	gdouble pix_width, pix_height ;
	gdouble scale_x, scale_y ;
	Slide_Model slide = get_slide_model ( model ) ;
	struct stat buf ;
	gchar *thumb_filename ;
	gboolean thumb_exist = FALSE ;
	
	{
		char path[2048] ;
		sprintf ( path, "%s", gnome_vfs_expand_initial_tilde("~/.lats2") ) ;
		mkdir ( path, S_IRWXU ) ;
	}
	{
		char path[2048] ;
		sprintf ( path, "%s", gnome_vfs_expand_initial_tilde("~/.lats2/thumbnails") ) ;
		mkdir ( path, S_IRWXU ) ;
	}
	
	thumb_filename = g_strdup_printf ( "%s/thumbnails/%s.thumb_%d", 
										gnome_vfs_expand_initial_tilde("~/.lats2"),
										exchange_char(filename, 47, 183), slide.image_quality ) ;
	
	//g_print ( _("thumbnail filename is %s\n"), thumb_filename ) ;
	
	if ( stat ( thumb_filename, &buf ) == 0 )
	{
		//g_print ( _("trying to load saved thumbnail...\n") ) ;
		image_pixbuf = gdk_pixbuf_new_from_file ( thumb_filename, NULL ) ;
		if ( image_pixbuf != NULL ) // thumbnail loaded?
		{
			thumb_exist = TRUE ;
			//g_print ( _("thumbnail loaded.\n") ) ;
		} else { // not loaded.. trying to create one from original image later...
			//g_print ( _("thumbnail not loaded. trying to create one...\n") ) ;
			image_pixbuf = gdk_pixbuf_new_from_file ( filename, NULL ) ;
			thumb_exist = FALSE ;
		}
	} else { // no thumbnail found. create one from original later..
		//g_print ( _("trying to create thumbnail...\n") ) ;
		image_pixbuf = gdk_pixbuf_new_from_file ( filename, NULL ) ;
		thumb_exist = FALSE ;
	}
	
	// if the image could not be loaded we won't get anything...
	if ( image_pixbuf == NULL ) return image_pixbuf ;
	
	//g_print ( _("thumbnail loaded.\n") ) ;
	
	pix_width = gdk_pixbuf_get_width ( image_pixbuf ) ;
	pix_height = gdk_pixbuf_get_height ( image_pixbuf ) ;

	//scale_x = slide.image_width / pix_width ;
	//scale_y = slide.image_height / pix_height ;
	
	//g_print ( _("A current scale_x = %f ; scale_y = %f\n"), scale_x, scale_y ) ;
	
	scale_x = ((gdouble) MIN ( slide.image_width, slide.image_height )) / 
				((gdouble) MIN ( pix_width, pix_height )) ;
	scale_y = scale_x ;
	
	if ( pix_width * scale_x > slide.image_width )
		scale_x = slide.image_width / pix_width ;
	
	if ( pix_height * scale_y > slide.image_height )
		scale_y = slide.image_height / pix_height ;

	scale_y = scale_x = MIN ( scale_x, scale_y ) ;
	
	//g_print ( _("B current scale_x = %f ; scale_y = %f\n"), scale_x, scale_y ) ;
	
	// correct rounding errors
	if ( pix_width * scale_x >= slide.image_width - 1 && pix_width * scale_x <= slide.image_width + 1 )
		scale_x = slide.image_width / pix_width ;
	if ( pix_height * scale_y >= slide.image_height - 1 && pix_height * scale_y <= slide.image_height + 1 )
		scale_y = slide.image_height / pix_height ;
	
	return_pixbuf = gdk_pixbuf_scale_simple ( image_pixbuf, pix_width * scale_x, pix_height * scale_y, GDK_INTERP_HYPER ) ;
	
	if ( thumb_exist == FALSE )
	{
		//g_print ( _("trying to save thumbnail...\n") ) ;
		if ( ! gdk_pixbuf_save ( return_pixbuf,
								thumb_filename,
								"png",
								NULL,
								NULL ) )
			g_print ( _("thumbnail not saved!\n") ) ;
	}
	
	g_free ( thumb_filename ) ;
	
	gdk_pixbuf_unref ( image_pixbuf ) ;
	
	return return_pixbuf ;
}

void
center_image_canvas ( GnomeCanvas *canvas )
{
	double canvas_width ;
	double canvas_height ;
	double max_x = 0 ;
	double max_y = 0 ;
	
	GList *list = g_list_first ( Slide_list ) ;
	
	gboolean AGAIN = TRUE ;
	
	if ( Slide_list == NULL ) 
	{
		gnome_canvas_set_scroll_region ( canvas, 0, 0, 1, 1 ) ;
		return ;
	}
	
	gnome_canvas_c2w (	canvas,
						GTK_WIDGET(canvas)->allocation.width,
						GTK_WIDGET(canvas)->allocation.height,
						&canvas_width,
						&canvas_height
						) ;
	
	while ( AGAIN == TRUE )
	{
		GnomeCanvasItem *item ;
		double x1, y1, x2, y2 ;
		gchar *filename ;
		
		filename = list->data ;
		
		item = gtk_object_get_data ( GTK_OBJECT(image_canvas), filename ) ;

		if ( item )
		{
			gnome_canvas_item_get_bounds ( item, &x1, &y1, &x2, &y2 ) ;
			
			if ( x2 > max_x ) max_x = x2 ;
			if ( y2 > max_y ) max_y = y2 ;
		}
		
		if ( !(list = g_list_next(list)) ) AGAIN = FALSE ;
	}

	if ( max_x < canvas_width ) max_x = canvas_width ;
	if ( max_y < canvas_height ) max_y = canvas_height ;
	
	gnome_canvas_set_scroll_region ( canvas, 0, 0, max_x, max_y ) ;
}

void
move_canvas_item_thread ( GnomeCanvasItem *item )
{
	Mouse_Position old_Mouse ;
	
	int own_task ;

	if ( task_already_exists_in_queue ( "move_canvas_item", NULL ) ||
		 task_already_exists_in_queue ( "stop_move_canvas_item", NULL ) )
		return ;
		
	own_task = task_add_to_queue ( "move_canvas_item", (gpointer) item ) ;
	thread_count++ ;
	
	//g_print ( _("moving canvas item...\n") ) ;
	
	old_Mouse.x = Mouse.x ;
	old_Mouse.y = Mouse.y ;
	
	while ( task_already_exists_in_queue ( "stop_move_canvas_item", NULL ) == FALSE )
	{
		Mouse_Position diff_Mouse ;
		usleep ( 10000 ) ;
		
		if ( old_Mouse.x != Mouse.x || old_Mouse.y != Mouse.y )
		{
			diff_Mouse.x = Mouse.x - old_Mouse.x ;
			diff_Mouse.y = Mouse.y - old_Mouse.y ;
			old_Mouse.x = Mouse.x ;
			old_Mouse.y = Mouse.y ;
			
			if ( task_already_exists_in_queue ( "stop_move_canvas_item", NULL ) == FALSE )
			{
				double x1, y1, x2, y2 ;
				double diff_x, diff_y ;
				gdk_threads_enter () ;
				gnome_canvas_item_get_bounds ( item, &x1, &y1, &x2, &y2 ) ;
				
				gnome_canvas_c2w (	GNOME_CANVAS(image_canvas),
					diff_Mouse.x,
					diff_Mouse.y,
					&diff_x,
					&diff_y
					) ;

				if ( x1 + diff_x < 0 ) diff_x = x1 * -1 ;
				if ( y1 + diff_y < 0 ) diff_y = y1 * -1 ;
				
				gnome_canvas_item_move ( item, diff_x, diff_y ) ;
				center_image_canvas ( GNOME_CANVAS(image_canvas) ) ;
				gnome_canvas_update_now ( GNOME_CANVAS(image_canvas) ) ;
				
				gdk_threads_leave () ;
			}
		}
	}
	
	//g_print ( _("done moving canvas item.\n") ) ;
	
	task_remove_from_queue ( own_task ) ;
	thread_count-- ;
}

void
move_canvas_item ( GnomeCanvasItem *item )
{
	pthread_t move_item_thread ;

	int own_task = task_add_to_queue ( "stop_move_canvas_item", NULL ) ;
	
	while ( task_already_exists_in_queue ( "move_canvas_item", NULL ) ||
			task_already_exists_in_queue ( "read_dir_entries", NULL ) )
	{
		usleep ( 10000 ) ;
		refresh_screen () ;
	}
	
	task_remove_from_queue ( own_task ) ;
	
	pthread_create ( &move_item_thread, NULL, (void*)&move_canvas_item_thread, item ) ;
	pthread_detach ( move_item_thread ) ;
}

void
on_image_canvas_size_changed (	GtkWidget *widget,
								GtkAllocation *allocation,
								gpointer user_data )
{
	//g_print ( _("image_canvas_size_changed\n") ) ;
	
	center_image_canvas ( user_data ) ;
}

gboolean
on_image_canvas_button_pressed (	GtkWidget *widget,
									GdkEventButton *event,
									gpointer user_data )
{
	GnomeCanvasItem *group ;
	GnomeCanvasItem *item ;
	gdouble x, y ;
	double wx, wy ;

	int own_task = task_add_to_queue ( "stop_move_canvas_item", NULL ) ;
	
	while ( task_already_exists_in_queue ( "move_canvas_item", NULL ) )
	{
		usleep ( 10000 ) ;
	}
	
	task_remove_from_queue ( own_task ) ;

	//g_print ( _("image_canvas_button_press_event\n") ) ;
	
	x = event->x ;
	y = event->y ;
	
	//g_print ( _("button pressed at x = %f, y = %f\n"), x, y ) ;
	
	gnome_canvas_c2w (	GNOME_CANVAS(image_canvas),
					x,
					y,
					&wx,
					&wy
					) ;
	
	item = gnome_canvas_get_item_at ( GNOME_CANVAS(image_canvas), wx, wy ) ;
	
	// if there's no item to handle with...
	if ( ! item ) return TRUE ;
	
	// now that we found at item we try to handle it
	//g_print ( _("got an item at this position!\n") ) ;
	
	// get parent group of item
	group = item->parent ;
	// if we already have the group then just take it :)
	if ( ! group ) group = item ;
	
	gnome_canvas_item_raise_to_top ( group ) ;
	
	if ( event->type == GDK_3BUTTON_PRESS ) {
		return TRUE ;
	} else if ( event->type == GDK_2BUTTON_PRESS ) {
		//g_print ( _("put item on top\n") ) ;
		//gnome_canvas_item_raise_to_top ( group ) ;
		return TRUE ;
	} else
		move_canvas_item ( group ) ;
	
	return TRUE ;
}

gboolean
on_image_canvas_button_released (	GtkWidget *widget,
									GdkEventButton *event,
									gpointer user_data )
{
	int own_task = task_add_to_queue ( "stop_move_canvas_item", NULL ) ;

	gdouble x, y ;
	
	//g_print ( _("image_canvas_button_release_event\n") ) ;
	
	x = event->x ;
	y = event->y ;
	
	//g_print ( _("button released at x = %f, y = %f\n"), x, y ) ;
	
	while ( task_already_exists_in_queue ( "move_canvas_item", NULL ) )
	{
		usleep ( 10000 ) ;
	}
	
	// look if canvas boundaries have to be changed...
	center_image_canvas ( GNOME_CANVAS(image_canvas) ) ;

	task_remove_from_queue ( own_task ) ;
	return TRUE ;
}

gboolean
on_image_canvas_mouse_position_changed (	GtkWidget *widget,
											GdkEventMotion *event,
											gpointer user_data )
{
	Mouse.x = event->x ;
	Mouse.y = event->y ;
	
	//g_print ( _("current mouse position is x = %d, y = %d\n"), Mouse.x, Mouse.y ) ;
	
	return TRUE ;
}

Slide_Model
get_slide_model ( char *model )
{
	Slide_Model slide ;
	int quality = 2 ; // render quality.. thumbsize = thumbsize * quality !!

	if ( strcmp ( model, "plain" ) == 0 )
	{
		slide.image_quality = quality ;
		
		slide.image_x_offset = 35 * slide.image_quality ;
		slide.image_y_offset = 51 * slide.image_quality ;
		slide.image_width = 102 * slide.image_quality ;
		slide.image_height = 78 * slide.image_quality ;
		
		slide.name_x_offset = 10 + slide.image_width ;
		slide.name_y_offset = 30 ;
		
		slide.attrib_x_offset = 0 ;
		slide.attrib_y_offset = 0 ;
		
		/*slide.bg_red = 242 ;
		slide.bg_green = 240 ;
		slide.bg_blue = 225 ;
		slide.bg_alpha = 128*/

		slide.bg_red = 0 ;
		slide.bg_green = 0 ;
		slide.bg_blue = 0 ;
		slide.bg_alpha = 128 ;
	}
	
	return slide ;
}

GdkPixbuf
*create_slide_case ( char *model )
{
	GdkPixbuf *case_pixbuf = NULL, *image_bg_pixbuf, *pre_case_pixbuf ;
	Slide_Model slide = get_slide_model ( model ) ;

	// search for slide-case in cache...
	//g_print ( _("search for slide-case in image-cache...\n") ) ;
	if ( Slide_case_cache != NULL && g_list_find ( Slide_case_cache, model ) != NULL )
	{
		//g_print ( _("found one\n") ) ;
		case_pixbuf = gtk_object_get_data ( GTK_OBJECT(image_canvas), model ) ;
	}
	
	if ( case_pixbuf == NULL )
	{
		g_print ( _("no slide-case in cache found. creating new one...\n") ) ;
		pre_case_pixbuf = gdk_pixbuf_new_from_inline ( 81435, lats_slide_template, TRUE, NULL ) ;
		case_pixbuf = gdk_pixbuf_scale_simple (	pre_case_pixbuf,
												gdk_pixbuf_get_width ( pre_case_pixbuf ) * slide.image_quality,
												gdk_pixbuf_get_height ( pre_case_pixbuf ) * slide.image_quality,
												GDK_INTERP_BILINEAR
												) ;
		gdk_pixbuf_unref ( pre_case_pixbuf ) ;
		
		image_bg_pixbuf = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace ( case_pixbuf ),
											FALSE,
											8,
											slide.image_width,
											slide.image_height
											) ;
		gdk_pixbuf_fill (	image_bg_pixbuf, 
							GNOME_CANVAS_COLOR_A ( slide.bg_red, slide.bg_green, slide.bg_blue, 255 ) ) ;
		
		gdk_pixbuf_composite (	image_bg_pixbuf,
								case_pixbuf,
								slide.image_x_offset,
								slide.image_y_offset,
								slide.image_width,
								slide.image_height,
								slide.image_x_offset,
								slide.image_y_offset,
								1,
								1,
								GDK_INTERP_BILINEAR,
								slide.bg_alpha
								) ;
		
		gdk_pixbuf_unref ( image_bg_pixbuf ) ;

		g_print ( _("adding slide-case to cache...\n") ) ;
		Slide_case_cache = g_list_append ( Slide_case_cache, model ) ;
		gtk_object_set_data ( GTK_OBJECT(image_canvas), model, case_pixbuf ) ;
	}
	
	return gdk_pixbuf_copy(case_pixbuf) ;
}

GdkPixbuf
*create_slide ( char *filename, char *model )
{
	GdkPixbuf *image_pixbuf ;
	GdkPixbuf *slide_pixbuf ;
	gdouble pix_width, pix_height ;
	gdouble center_x_offset, center_y_offset ;
	
	Slide_Model slide ;

	slide = get_slide_model ( model ) ;

	slide_pixbuf = create_slide_case ( model ) ;
	
	image_pixbuf = get_thumbnail ( filename, model ) ;
	if ( ! image_pixbuf )
	{
		image_pixbuf = gdk_pixbuf_new_from_xpm_data( (const char**) brokenimage_xpm ) ;
		g_print ( _("!!! couldn't load %s !!!\n"), filename ) ;
		//return NULL ;
	}
	
	pix_width = gdk_pixbuf_get_width ( image_pixbuf ) ;
	pix_height = gdk_pixbuf_get_height ( image_pixbuf ) ;

	center_x_offset = ( slide.image_width - pix_width ) / 2 ;
	center_y_offset = ( slide.image_height - pix_height ) / 2 ;
		
	gdk_pixbuf_composite (	image_pixbuf,
							slide_pixbuf,
							slide.image_x_offset + center_x_offset,
							slide.image_y_offset + center_y_offset,
							pix_width,
							pix_height,
							slide.image_x_offset + center_x_offset,
							slide.image_y_offset + center_y_offset,
							1,
							1,
							GDK_INTERP_TILES,
							255
							) ;

	gdk_pixbuf_unref ( image_pixbuf ) ;
	
	return slide_pixbuf ;
}

GnomeCanvasItem
*add_new_slide ( char *filename, gdouble x, gdouble y )
{
	GnomeCanvasItem *canvasgroup, *canvasimage ;
	GdkPixbuf *slide_pixbuf ;
	gboolean free_space_found = FALSE ;
	gchar *item_filename ;
	
	gdk_threads_enter () ;
	center_image_canvas ( GNOME_CANVAS(image_canvas) ) ;
	gdk_threads_leave () ;

	slide_pixbuf = create_slide ( filename, "plain" ) ;
	
	if ( slide_pixbuf == NULL ) return NULL ;
	
	if ( x == -1 && y == -1 )
	{
		gdouble canvas_width ;
		gdouble canvas_height ;
		gdouble slide_width ;
		gdouble slide_height ;
		gdouble block ;
		gboolean item_found = TRUE ;

		gnome_canvas_c2w (	GNOME_CANVAS(image_canvas),
							image_canvas->allocation.width,
							image_canvas->allocation.height,
							&canvas_width,
							&canvas_height
							) ;
		// the following seems to be bad... :)
		/*gnome_canvas_c2w (	GNOME_CANVAS(image_canvas),
							gdk_pixbuf_get_width ( slide_pixbuf ),
							gdk_pixbuf_get_height ( slide_pixbuf ),
							&slide_width,
							&slide_height
							) ;*/
		
		// going straight forward seems to be better...
		slide_width = gdk_pixbuf_get_width ( slide_pixbuf ) ;
		slide_height = gdk_pixbuf_get_height ( slide_pixbuf ) ;
		
		block = MAX ( slide_width, slide_height ) / 2 ;
		
		x = 0 ; y = 0 ;
		
		{
			double x1, y1, x2, y2 ;
			gnome_canvas_get_scroll_region ( GNOME_CANVAS(image_canvas), &x1, &y1, &x2, &y2 ) ;
			if ( x2 > canvas_width ) canvas_width = x2 ;
			if ( y2 > canvas_height ) canvas_height = y2 ;
		}
		
		/*g_print ( _("REAL: cw = %d ; ch = %d ; sw = %d ; sh = %d\n"),
					image_canvas->allocation.width, image_canvas->allocation.height,
					gdk_pixbuf_get_width ( slide_pixbuf ), 
					gdk_pixbuf_get_height ( slide_pixbuf ) ) ;
		g_print ( _("VIRTUAL: cw = %f ; ch = %f ; sw = %f ; sh = %f ; block = %f\n"),
					canvas_width, canvas_height, slide_width, slide_height, block ) ;
		*/
		//g_print ( _("searching for free space on workspace...\n") ) ;
		
		while ( item_found == TRUE )
		{
			gdouble bx, by ;
			GnomeCanvasItem *item = NULL ;
			item_found = FALSE ;
			for ( by = 0 ; by < block && item_found == FALSE ; by = by + block/5 )
			{
				for ( bx = 0 ; bx < block && item_found == FALSE ; bx = bx + block/5 )
				{
					if ( item = gnome_canvas_get_item_at ( GNOME_CANVAS(image_canvas), x+bx, y+by ) )
						item_found = TRUE ;
				}
			}
			
			if ( item_found == TRUE )
			{
				gdouble x1, y1, x2, y2 ;
				gnome_canvas_item_get_bounds ( item, &x1, &y1, &x2, &y2 ) ;
				x = x + x2 - 10 ;
				//g_print ( _("bounds x1 = %f ; y1 = %f ; x2 = %f ; y2 = %f\n"), x1, y1, x2, y2 ) ;
				if ( x + x2 >= canvas_width + 30 )
				{
					x = 0 ;
					y = y + 10 ;
					//g_print ( _("goto new line\n") ) ;
					if ( item = gnome_canvas_get_item_at ( GNOME_CANVAS(image_canvas), x, y ) )
					{
						//gdouble x1, y1, x2, y2 ;
						gnome_canvas_item_get_bounds ( item, &x1, &y1, &x2, &y2 ) ;
						y = y + y2 ;
						//g_print ( _("found an item at new line y = %f\n"), y ) ;
					}
				}
			}
			//g_print ( _("current x = %f ; y = %f \n"), x, y ) ;
		}
		//g_print ( _("done.\n") ) ;
	} else {
		if ( x == -1 ) x = 0 ;
		if ( y == -1 ) y = 0 ;
	}
	
	gdk_threads_enter () ;
	canvasgroup = gnome_canvas_item_new ( gnome_canvas_root (GNOME_CANVAS(image_canvas)),
											gnome_canvas_group_get_type (),
											"x", x,
											"y", y,
											NULL ) ;
	
	canvasimage = gnome_canvas_item_new ( GNOME_CANVAS_GROUP(canvasgroup),
											GNOME_TYPE_CANVAS_PIXBUF,
											"pixbuf", slide_pixbuf,
											"x", 0,
											"y", 0,
											NULL ) ;
	
	item_filename = g_strdup ( filename ) ;
	
	Slide_list = g_list_append ( Slide_list, item_filename ) ;
	
	gtk_object_set_data ( GTK_OBJECT(image_canvas), item_filename, canvasgroup ) ;
	
	gdk_pixbuf_unref ( slide_pixbuf ) ;
	
	gnome_canvas_update_now ( GNOME_CANVAS(image_canvas) ) ;
	
	center_image_canvas ( GNOME_CANVAS(image_canvas) ) ;
	
	gdk_threads_leave () ;
	
	return canvasgroup ;
}

void
initialize_image_canvas ( void )
{
	Slide_Model slide = get_slide_model ( "plain" ) ;
	
	double initial_zoom = ((double) 1) / ((double) slide.image_quality) ;
	
	//g_print ( "initial zoom = %d\n", initial_zoom ) ;
	
	gnome_canvas_set_pixels_per_unit ( GNOME_CANVAS(image_canvas), initial_zoom ) ;
	gnome_canvas_set_dither ( GNOME_CANVAS(image_canvas), GDK_RGB_DITHER_MAX ) ;
	
	center_image_canvas ( GNOME_CANVAS(image_canvas) ) ;

	// setup repositioning stuff
	gtk_signal_connect ( GTK_OBJECT(image_canvas), "size_allocate",
						GTK_SIGNAL_FUNC (on_image_canvas_size_changed),
						GNOME_CANVAS(image_canvas) ) ;
	// setup mousebutton behaviour
	gtk_signal_connect ( GTK_OBJECT(image_canvas), "button_press_event",
						GTK_SIGNAL_FUNC (on_image_canvas_button_pressed),
						NULL ) ;
	gtk_signal_connect ( GTK_OBJECT(image_canvas), "button_release_event",
						GTK_SIGNAL_FUNC (on_image_canvas_button_released),
						NULL ) ;
	// setup mouse-position tracker
	gtk_signal_connect ( GTK_OBJECT(image_canvas), "motion_notify_event",
						GTK_SIGNAL_FUNC (on_image_canvas_mouse_position_changed),
						NULL ) ;

	// enable these for testing purposes
	/*{
		int count ;
		
		for ( count = 0 ; count < 2 ; count++ )
		{
			add_new_slide ( "/home/oktay/Bilder/Backgrounds/03_2_lake.jpg", -1, -1 ) ;
			add_new_slide ( "/home/oktay/Bilder/Backgrounds/sunset.jpg", -1, -1 ) ;
			add_new_slide ( "/home/oktay/Bilder/Backgrounds/RedMaple1600x1200.jpg", -1, -1 ) ;
			add_new_slide ( "/home/oktay/Bilder/Artist/kirby.png", -1, -1 ) ;
		}
	}*/

}
