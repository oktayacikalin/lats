#include <gnome.h>

gboolean
isimage	(char *filename);

gboolean
isdir (char *filename);

GdkPixbuf
*get_thumbnail ( char *filename, int FORCE_RELOAD );

void
read_dir_from_combo		               ( int FORCE_RELOAD, gpointer         user_data);

