#include <gnome.h>

typedef struct
{
	int x ;
	int y ;
} Mouse_Position ;

typedef struct
{
	int image_quality ; // 1 for screen-quality until 4 for great zooming
	
	int image_x_offset ;
	int image_y_offset ;
	int image_width ;
	int image_height ;
	
	int name_x_offset ;
	int name_y_offset ;
	
	int attrib_x_offset ;
	int attrib_y_offset ;
	
	int bg_red ;
	int bg_green ;
	int bg_blue ;
	int bg_alpha ;
} Slide_Model ;


void
center_image_canvas ( GnomeCanvas *canvas ) ;

void
move_canvas_item_thread ( GnomeCanvasItem *item ) ;

void
move_canvas_item ( GnomeCanvasItem *item ) ;

void
on_image_canvas_size_changed (	GtkWidget *widget,
								GtkAllocation *allocation,
								gpointer user_data ) ;

gboolean
on_image_canvas_button_pressed (	GtkWidget *widget,
									GdkEventButton *event,
									gpointer user_data ) ;

gboolean
on_image_canvas_button_released (	GtkWidget *widget,
									GdkEventButton *event,
									gpointer user_data ) ;

gboolean
on_image_canvas_mouse_position_changed (	GtkWidget *widget,
											GdkEventMotion *event,
											gpointer user_data ) ;

Slide_Model
get_slide_model ( char *model );

GdkPixbuf
*create_slide_case ( char *model ) ;

GdkPixbuf
*create_slide ( char *filename, char *model ) ;

GnomeCanvasItem
*add_new_slide ( char *filename, gdouble x, gdouble y ) ;

void
initialize_image_canvas ( void ) ;
