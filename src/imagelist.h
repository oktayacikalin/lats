#include <gnome.h>

gboolean
file_filter	(char *filename);

gboolean
isimage	(char *filename);

gboolean
isdir (char *filename);

gboolean
islink (char *filename);

GdkPixbuf
*get_thumbnail ( char *filename, int FORCE_RELOAD, int icon_pos );

void
read_dir_from_combo_thread ( int FORCE_RELOAD );

void
read_dir_from_combo_start_thread ( int FORCE_RELOAD );

void
read_dir_from_combo	( int FORCE_RELOAD, gpointer user_data);

void
go_to_previous_image_in_list ( void );

void
go_to_next_image_in_list ( void );

int
spider_gather_subdirinfo ( char *dirname, int depth );

void
spider_gather_dirinfo ( char *dirname, GtkCList *dirlist );

void
go_spider_thread ( void );

void
go_spider_go ( void );

void
move_to_icon ( GnomeIconList *iconlist, int pos );

void
check_for_initial_directory_read ( void );
