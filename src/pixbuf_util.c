#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "pixbuf_util.h"

#include <png.h>	/* for png saving, below */

/*
 *-----------------------------------------------------------------------------
 * simple png save (please read comment)
 *-----------------------------------------------------------------------------
 */

/* Since I really admire John's work at gqview I naturally read through his
 * code :) Now that I wanted to save png-files too I borrowed these files :)
 *																													-Oktay
 */

/*
 * This save_pixbuf_to_file utility was copied from the nautilus 0.1 preview,
 * and judging by the message they got it from gnome-iconedit.
 *
 * I have only changed the source to match my coding style in GQview,
 * make it work in here, and rename to pixbuf_to_file_as_png
 *                                                                   -John
 *
 * === message in nautilus source:
 * utility routine for saving a pixbuf to a png file.
 * This was adapted from Iain Holmes' code in gnome-iconedit, and probably
 * should be in a utility library, possibly in gdk-pixbuf itself.
 * ===
 */
gboolean pixbuf_to_file_as_png (GdkPixbuf *pixbuf, char *filename)
{
	FILE *handle;
  	char *buffer;
	gboolean has_alpha;
	int width, height, depth, rowstride;
  	guchar *pixels;
  	png_structp png_ptr;
  	png_infop info_ptr;
  	png_text text[2];
  	int i;

	g_return_val_if_fail (pixbuf != NULL, FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (filename[0] != '\0', FALSE);

        handle = fopen (filename, "wb");
        if (handle == NULL)
		{
        	return FALSE;
		}

	png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
		{
		fclose (handle);
		return FALSE;
		}

	info_ptr = png_create_info_struct (png_ptr);
	if (info_ptr == NULL)
		{
		png_destroy_write_struct (&png_ptr, (png_infopp)NULL);
		fclose (handle);
	    	return FALSE;
		}

	if (setjmp (png_ptr->jmpbuf))
		{
		png_destroy_write_struct (&png_ptr, &info_ptr);
		fclose (handle);
		return FALSE;
		}

	png_init_io (png_ptr, handle);

        has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	depth = gdk_pixbuf_get_bits_per_sample (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);

	png_set_IHDR (png_ptr, info_ptr, width, height,
			depth, PNG_COLOR_TYPE_RGB_ALPHA,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);

	/* Some text to go with the png image */
	text[0].key = "Title";
	text[0].text = filename;
	text[0].compression = PNG_TEXT_COMPRESSION_NONE;
	text[1].key = "Software";
	text[1].text = "GQview Thumbnail";
	text[1].compression = PNG_TEXT_COMPRESSION_NONE;
	png_set_text (png_ptr, info_ptr, text, 2);

	/* Write header data */
	png_write_info (png_ptr, info_ptr);

	/* if there is no alpha in the data, allocate buffer to expand into */
	if (has_alpha)
		{
		buffer = NULL;
		}
	else
		{
		buffer = g_malloc(4 * width);
		}
	
	/* pump the raster data into libpng, one scan line at a time */	
	for (i = 0; i < height; i++)
		{
		if (has_alpha)
			{
			png_bytep row_pointer = pixels;
			png_write_row (png_ptr, row_pointer);
			}
		else
			{
			/* expand RGB to RGBA using an opaque alpha value */
			int x;
			char *buffer_ptr = buffer;
			char *source_ptr = pixels;
			for (x = 0; x < width; x++)
				{
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = 255;
				}
			png_write_row (png_ptr, (png_bytep) buffer);		
			}
		pixels += rowstride;
		}
	
	png_write_end (png_ptr, info_ptr);
	png_destroy_write_struct (&png_ptr, &info_ptr);
	
	g_free (buffer);
		
	fclose (handle);
	return TRUE;
}


/*
 *-----------------------------------------------------------------------------
 * pixbuf rotation
 *-----------------------------------------------------------------------------
 */

/*
 * Returns a copy of pixbuf src rotated 90 degrees clockwise or 90 counterclockwise
 *
 */
GdkPixbuf *pixbuf_copy_rotate_90(GdkPixbuf *src, gint counter_clockwise)
{
	GdkPixbuf *dest;
	gint has_alpha;
	gint sw, sh, srs;
	gint dw, dh, drs;
	guchar *s_pix;
        guchar *d_pix;
	guchar *sp;
        guchar *dp;
	gint i, j;
	gint a;

	if (!src) return NULL;

	sw = gdk_pixbuf_get_width(src);
	sh = gdk_pixbuf_get_height(src);
	has_alpha = gdk_pixbuf_get_has_alpha(src);
	srs = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	dw = sh;
	dh = sw;
	dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, has_alpha, 8, dw, dh);
	drs = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);

	a = (has_alpha ? 4 : 3);

	for (i = 0; i < sh; i++)
		{
		sp = s_pix + (i * srs);
		for (j = 0; j < sw; j++)
			{
			if (counter_clockwise)
				{
				dp = d_pix + ((dh - j - 1) * drs) + (i * a);
				}
			else
				{
				dp = d_pix + (j * drs) + ((dw - i - 1) * a);
				}

			*(dp++) = *(sp++);	/* r */
			*(dp++) = *(sp++);	/* g */
			*(dp++) = *(sp++);	/* b */
			if (has_alpha) *(dp) = *(sp++);	/* a */
			}
		}

	return dest;
}

/*
 * Returns a copy of pixbuf mirrored and or flipped.
 * TO do a 180 degree rotations set both mirror and flipped TRUE
 * if mirror and flip are FALSE, result is a simple copy.
 */
GdkPixbuf *pixbuf_copy_mirror(GdkPixbuf *src, gint mirror, gint flip)
{
	GdkPixbuf *dest;
	gint has_alpha;
	gint w, h, srs;
	gint drs;
	guchar *s_pix;
        guchar *d_pix;
	guchar *sp;
        guchar *dp;
	gint i, j;
	gint a;

	if (!src) return NULL;

	w = gdk_pixbuf_get_width(src);
	h = gdk_pixbuf_get_height(src);
	has_alpha = gdk_pixbuf_get_has_alpha(src);
	srs = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, has_alpha, 8, w, h);
	drs = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);

	a = has_alpha ? 4 : 3;

	for (i = 0; i < h; i++)
		{
		sp = s_pix + (i * srs);
		if (flip)
			{
			dp = d_pix + ((h - i - 1) * drs);
			}
		else
			{
			dp = d_pix + (i * drs);
			}
		if (mirror)
			{
			dp += (w - 1) * a;
			for (j = 0; j < w; j++)
				{
				*(dp++) = *(sp++);	/* r */
				*(dp++) = *(sp++);	/* g */
				*(dp++) = *(sp++);	/* b */
				if (has_alpha) *(dp) = *(sp++);	/* a */
				dp -= (a + 3);
				}
			}
		else
			{
			for (j = 0; j < w; j++)
				{
				*(dp++) = *(sp++);	/* r */
				*(dp++) = *(sp++);	/* g */
				*(dp++) = *(sp++);	/* b */
				if (has_alpha) *(dp++) = *(sp++);	/* a */
				}
			}
		}

	return dest;
}

