
#include <gnome.h>


gboolean pixbuf_to_file_as_png (GdkPixbuf *pixbuf, char *filename);

GdkPixbuf *pixbuf_copy_rotate_90(GdkPixbuf *src, gint counter_clockwise);
GdkPixbuf *pixbuf_copy_mirror(GdkPixbuf *src, gint mirror, gint flip);

