#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <sys/types.h>
#include <fcntl.h>

#include <stdlib.h>

#include <sys/stat.h>
#include <dirent.h> 

#include <unistd.h>

#include <string.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#include <libgnomevfs/gnome-vfs.h>

#include <math.h>

#include <pthread.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "importedfuncs.h"
#include "imagelist.h"
#include "tasks.h"

// borrowed from gqview :)
#include "pixbuf_util.h"

#include "loading.xpm"
#include "brokenimage.xpm"

int extern check_size ;
guint32 extern check_color_a ;
guint32 extern check_color_b ;

int extern thumbnails ;
int extern thumb_size ;
int extern max_thumb_size ;
int extern no_thumb_size ;
int extern thumb_render_quality ;

int extern filedisplay_as_icons ;

char dirname [2048] ;
char filename [2048] ;

int extern file_filter_use ; // don't show every file
int extern file_filter_hide_dot_files ; // don't show hidden files
int extern file_filter_hide_non_multimedia_files ; // only show multimedia files
int extern file_filter_hide_images ; // don't show images
int extern file_filter_hide_movies ; // don't show movies
int extern file_filter_hide_audio ; // don't show audio files

char extern *LATS_RC_DIR ;
char extern *LATS_RC_TEMP_DIR ;
char extern *LATS_RC_THUMB_DIR ;
char extern *LATS_THUMB_DIR ;
int extern LATS_RC_THUMB_IN_HOME ;

GtkWidget extern *MainWindow ;
GtkWidget extern *PrefsWindow ;
GtkWidget extern *FullscreenWindow ;
GtkWidget extern *FullscreenWindowProgressbar ;

int extern thread_count ;

int extern spider_max_dir_depth ; // how deep should the dirlist-spider look?


gboolean
file_filter (char *filename)
{
	// first filter non-files!
	if ( ( strlen(filename) == 1 && !strncmp(filename,  ".", 1) ) ||
		 ( strlen(filename) == 2 && !strncmp(filename, "..", 2) ) )
		return (FALSE) ;
	
	// now.. do we want to take a closer look?
	if ( file_filter_use == FALSE ) return (TRUE) ; // show every file!
	
	// now we do :)
	if ( file_filter_hide_dot_files == TRUE && !strncmp(filename, ".", 1) )
		return (FALSE);
	
	// show non multimedia content?
	if ( file_filter_hide_non_multimedia_files == TRUE &&
		 ( !strncmp (gnome_mime_type (filename), "audio/", 6) ||
	       !strncmp (gnome_mime_type (filename), "image/", 6) ||
	       !strncmp (gnome_mime_type (filename), "video/", 6) ||
		   !strcmp (gnome_mime_type (filename), "application/pdf") ||
		   !strcmp (gnome_mime_type (filename), "application/postscript") ||
	       isdir (filename) ) == FALSE )
		return (FALSE);
	
	// hide audio files?
	if ( file_filter_hide_audio && !strncmp (gnome_mime_type (filename), "audio/", 6) )
		return (FALSE);
	
	// hide image files? doesn't make sense :)
	if ( file_filter_hide_images && !strncmp (gnome_mime_type (filename), "image/", 6) )
		return (FALSE);
	
	// hide movie files?
	if ( file_filter_hide_movies && !strncmp (gnome_mime_type (filename), "video/", 6) )
		return (FALSE);
	
	return (TRUE);
}


gboolean
isimage (char *filename)
{
  /* we might want to keep video/ too, if imlib didn't crash trying to make
     thumbnails of them... - presumably it's meant to work. */

  if ( !strncmp (gnome_mime_type (filename), "image/", 6) )
    {
      return (TRUE);
    }

  return (FALSE);
}


gboolean
isdir (char *filename)
{
	DIR *imagedir;
	struct stat buf;

	//printd("stat\n");
	stat ( filename, &buf );

	if ( S_ISDIR (buf.st_mode) && ( imagedir=opendir(filename) ) && strcmp(filename,".") && strcmp(filename,"..") )
	{
		closedir(imagedir);
		return (TRUE);
	}
	
	return (FALSE);
}


gboolean
islink (char *filename)
{
	char cwd[2048];
	
	{
		struct stat buf;
		//printd("stat "); printd(filename);printd("\n");
		stat ( filename, &buf );
		
		if ( S_ISLNK (buf.st_mode) )
			return (TRUE);
	}

	strcpy ( cwd, filename ) ;
	
	{
		int pos = strlen(cwd) - 1 ;
		int i ;
		
		if ( cwd[ pos ] == 47 && pos > 1 ) pos-- ;
		while ( cwd[ pos ] != 47 && pos > 0 )
			pos-- ;
		//printf ("got char '%d' at pos %d from %d chars\n", cwd[pos], pos, strlen(cwd) ) ;
		for ( i = pos+1 ; i < strlen(cwd) ; i++ ) 
		{
			cwd[ i-pos-1 ] = cwd[ i ] ;
		}
		for ( ; i < strlen(cwd)+pos+1 ; i++ )
			cwd[ i-pos-1 ] = '\0' ;
		
		//printf ("cwd = %s\n", cwd ) ;
	}
	
	if ( file_filter_hide_dot_files == TRUE && ( !strncmp(cwd, ".", 1) || !strcmp(cwd, ".") ) )
		return (TRUE);
	
	return (FALSE);
}


GdkPixbuf
*get_thumbnail ( char *filename, int FORCE_RELOAD, int icon_pos )
{
	GdkPixbuf *image = NULL ;
	double scale = 1 ;
	char thumbfilename[2048];
	int isthumb = FALSE ;
	int supported = TRUE ;
	int not_loaded = FALSE ;
	struct stat buf, buf2 ;
	guint32 checkcolora = 0xFFFFFF ; //( check_color_a * 256 * 256 ) + ( check_color_a * 256 ) + check_color_a ;
	guint32 checkcolorb = 0xFFFFFF ; //( check_color_b * 256 * 256 ) + ( check_color_b * 256 ) + check_color_b ;
	// let's get the right backgroundcolors for the list...
	GtkWidget *imagelist = lookup_widget ( MainWindow, "imagelist" ) ;
	gushort bg_red = imagelist->style->bg->red / 256 ;
	gushort bg_green = imagelist->style->bg->green / 256 ;
	gushort bg_blue = imagelist->style->bg->blue / 256 ;
	guint32 bg_pixel = ( bg_red * 256 * 256 ) + ( bg_green * 256 ) + bg_blue ;
	
	//printf ( "showing background color of clist...\n" ) ;
	//printf ( "max should be %d=%06X\n", ( 255 * 256 * 256 ) + ( 255 * 256 ) + 255, ( 255 * 256 * 256 ) + ( 255 * 256 ) + 255 ) ;
	//printf ( "%d=%06X - %d=%06X %d=%06X %d=%06X\n", bg_pixel, bg_pixel, bg_red, bg_red, bg_green, bg_green, bg_blue, bg_blue ) ;
	
	//checkcolora = ( bg_red * 256 * 256 ) + ( bg_green * 256 ) + bg_blue ;
	//checkcolorb = ( bg_red * 256 * 256 ) + ( bg_green * 256 ) + bg_blue ;
	
	checkcolora = bg_pixel ;
	checkcolorb = bg_pixel ;
	
	gdk_threads_leave () ;
	
	if ( LATS_RC_THUMB_IN_HOME == TRUE ) {
		int pos = 0 ;
		char cwd[2048] ;
		getcwd ( cwd, 2048 ) ;
		strcat ( cwd, "/" ) ;
		printd("il: current working directory = ") ; printd(cwd) ; printd("\n");
		for ( pos = 0 ; pos < strlen(cwd) ; pos++ )
		{
			//printd( text_from_var(  strlen(index(cwd,47))  ) ); printd(" ");
			if ( cwd[pos] == 47 ) cwd[pos] = 183 ;
		}
		strcat ( cwd, filename ) ;
		printd("il: "); printd(cwd); printd("\n");
		printd("il: put thumb into homedir\n");
		sprintf( thumbfilename, "%s%s%s.png", gnome_vfs_expand_initial_tilde ( LATS_RC_DIR ), LATS_RC_THUMB_DIR, cwd ) ;
	} else {
		printd("il: put thumb into local dir\n");
		sprintf( thumbfilename, "%s%s.png", LATS_THUMB_DIR, filename ) ;
	}
	
	printd("il: ");printd(thumbfilename);printd("\n");
	
	stat ( thumbfilename, &buf );
	stat ( filename, &buf2 );
	
	/*if ( strstr ( filename, ".xcf" ) != NULL ) { // not yet supported :-/
		printd(".xcf nicht unterstützt.\n");
		supported = FALSE ;
	} else */if ( ( image = gdk_pixbuf_new_from_file(thumbfilename) ) && 
					buf.st_mtime >= buf2.st_mtime && 	// thumbnail ? newer than image?
					FORCE_RELOAD == FALSE  ) {
		printd("il: thumbnail exists!\n");
		isthumb = TRUE ;
	} else /*if ( buf.st_size != 0 )*/ { // no thumbnail or image newer than thumbnail
		if ( FORCE_RELOAD )
		{
			printd("il: we are being forced to reload all thumbnails... "); printd(text_from_var(FORCE_RELOAD)); printd("\n");
		}
		printd("il: load image...\n");
		if ( !strcmp( gnome_mime_type(filename), "application/pdf" ) ) {
			char cmd[2048], destfile[2048] ;
			int acroread_available = FALSE ;
			printd("il: load a pdf-document...\n");
			printd("il: check for Adobe Acrobat Reader ( acroread ) available in path... ");
			if ( system("which acroread &> /dev/null") == 0 )
			{
				printd("il: found!\n");
				acroread_available = TRUE ;
			} else {
				printd("il: not found!\n");
				acroread_available = FALSE ;
			}
			if ( acroread_available == TRUE )
			{
				strcpy ( destfile, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
				strcat ( destfile, LATS_RC_TEMP_DIR ) ;
				strcat ( destfile, filename ) ;
				strcat ( destfile, ".lats-temp.ps" ) ;
				printd("il: using acroread for converting pdf-document to postscript...\n");
				strcpy ( cmd, "acroread -toPostScript -binary -start 1 -end 1 -fast -pairs '" ) ;
				strcat ( cmd, filename ) ;
				strcat ( cmd, "' '" ) ;
				strcat ( cmd, destfile ) ;
				strcat ( cmd, "'" ) ;
			}
			if ( acroread_available == TRUE && system(cmd) == 0 && 
				 task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE )
			{
				printd("il: now convert it to an image...\n");
				strcpy ( cmd, "gs -sDEVICE=tiff24nc -r37 -q -dSAFER -dNOPAUSE -sOutputFile='" ) ;
				strcat ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
				strcat ( cmd, LATS_RC_TEMP_DIR ) ;
				strcat ( cmd, filename ) ;
				strcat ( cmd, ".lats-temp.ps.tiff' -- '" ) ;
				strcat ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
				strcat ( cmd, LATS_RC_TEMP_DIR ) ;
				strcat ( cmd, filename ) ;
				strcat ( cmd, ".lats-temp.ps'" ) ;
				if ( stat ( destfile, &buf ) == 0 && system(cmd) == 0 && 
					 task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE )
				{
					printd("il: now load this image...\n");
					strcpy ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
					strcat ( cmd, LATS_RC_TEMP_DIR ) ;
					strcat ( cmd, filename ) ;
					strcat ( cmd, ".lats-temp.ps.tiff" ) ;
					if ( ( image = gdk_pixbuf_new_from_file(cmd) ) == FALSE ) not_loaded = TRUE ;
				} else
					printd("il: convert failed. image not loaded.\n");
			} else if ( acroread_available == FALSE && task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE ) {
				printd("il: using convert for converting pdf-document to a tiff-image...\n");
				printd("il: now convert it to an image...\n");
				strcpy ( cmd, "convert -density 37 '" ) ;
				strcat ( cmd, filename ) ;
				strcat ( cmd, "' '" ) ;
				strcat ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
				strcat ( cmd, LATS_RC_TEMP_DIR ) ;
				strcat ( cmd, filename ) ;
				strcat ( cmd, ".lats-temp.tiff'" ) ;
				if ( system(cmd) == 0 && task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE )
				{
					printd("il: now load this image...\n");
					strcpy ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
					strcat ( cmd, LATS_RC_TEMP_DIR ) ;
					strcat ( cmd, filename ) ;
					strcat ( cmd, ".lats-temp.tiff" ) ;
					if ( ( image = gdk_pixbuf_new_from_file(cmd) ) == FALSE ) not_loaded = TRUE ;
				} else
					printd("il: convert failed. image not loaded.\n");
				printd("il: now delete the temp-file...\n");
				strcpy ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
				strcat ( cmd, LATS_RC_TEMP_DIR ) ;
				strcat ( cmd, filename ) ;
				strcat ( cmd, ".lats-temp.tiff" ) ;
				unlink ( cmd ) ;
			}
			printd("il: now delete the temp-files...\n");
			strcpy ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
			strcat ( cmd, LATS_RC_TEMP_DIR ) ;
			strcat ( cmd, filename ) ;
			strcat ( cmd, ".lats-temp.ps.tiff" ) ;
			unlink ( cmd ) ;
			strcpy ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
			strcat ( cmd, LATS_RC_TEMP_DIR ) ;
			strcat ( cmd, filename ) ;
			strcat ( cmd, ".lats-temp.ps" ) ;
			unlink ( cmd ) ;
		} else if ( !strcmp( gnome_mime_type(filename), "application/postscript" ) ) {
			char cmd[2048] ;
			printd("il: load a postscript-document...\n");
			strcpy ( cmd, "gs -sDEVICE=tiff24nc -r37 -q -dSAFER -dNOPAUSE -sOutputFile='" ) ;
			strcat ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
			strcat ( cmd, LATS_RC_TEMP_DIR ) ;
			strcat ( cmd, filename ) ;
			strcat ( cmd, ".lats-temp.tiff' -- '" ) ;
			strcat ( cmd, filename ) ;
			strcat ( cmd, "'" ) ;
			if ( system(cmd) >= 0 && task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE )
			{
				printd("il: now load this image...\n");
				strcpy ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
				strcat ( cmd, LATS_RC_TEMP_DIR ) ;
				strcat ( cmd, filename ) ;
				strcat ( cmd, ".lats-temp.tiff" ) ;
				if ( ( image = gdk_pixbuf_new_from_file(cmd) ) == FALSE ) not_loaded = TRUE ;
			}
			printd("il: now delete the temp-files...\n");
			strcpy ( cmd, gnome_vfs_expand_initial_tilde (LATS_RC_DIR) ) ;
			strcat ( cmd, LATS_RC_TEMP_DIR ) ;
			strcat ( cmd, filename ) ;
			strcat ( cmd, ".lats-temp.tiff" ) ;
			unlink ( cmd ) ;
		} else {
			printd("il: load regular image...\n");
			if ( ( image = gdk_pixbuf_new_from_file(filename) ) == FALSE ) not_loaded = TRUE ;
		}
		isthumb = FALSE ;
	} 
	
	if ( ! image || not_loaded )
	{
		image = gdk_pixbuf_new_from_xpm_data ( (const char**) brokenimage_xpm ) ;
		not_loaded = TRUE ;
	}
	
	gdk_threads_enter () ;
	
	if ( image && supported == TRUE )
	{
		GnomeIconList *gil = GNOME_ICON_LIST(lookup_widget(MainWindow, "iconlist")) ;
		GnomeIconList *gil2 = GNOME_ICON_LIST(lookup_widget(MainWindow, "iconlist1")) ;
		int thumb_width ;
		int thumb_height ;
		GdkPixbuf *tnimage = NULL, *output_image ;
		int temp_thumb_render_quality = thumb_render_quality ; // gather user-defined interpolation mode
		gdk_threads_leave () ;
		printd("il: thumbnail loaded\n");
		if ( isthumb == FALSE )
		{
			scale = ((double) thumb_size) / 
					((double) MAX (gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image)));
			unlink ( thumbfilename ) ;
		} else if ( isthumb == TRUE && gdk_pixbuf_get_width(image) <= max_thumb_size && gdk_pixbuf_get_height(image) <= max_thumb_size ) {
			scale = 1 ;
			temp_thumb_render_quality = GDK_INTERP_NEAREST ; // disable interpolation if image fits!
		} else {
			scale = ((double) max_thumb_size) / 
					((double) MAX (gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image)));
		}
		//printd("thumbnail scale = "); printd(text_from_var(scale)); printd("\n");
		printd("il: thumbnail scale = "); printd(text_from_var(scale*100)); printd("\n");
		
		thumb_width = ( gdk_pixbuf_get_width(image) ) * scale ;
		thumb_height = ( gdk_pixbuf_get_height(image) ) * scale ;
		
		if ( thumb_width < 2 ) thumb_width = 1 ;
		if ( thumb_height < 2 ) thumb_height = 1 ;
		
		//gdk_imlib_set_render_type ( thumb_render_quality ) ;
		
		if (  ( gdk_pixbuf_get_width(image) > thumb_size ) || ( gdk_pixbuf_get_height(image) > thumb_size ) )
		{ // we have to resize our image to fit thumbnail size..
			//gdk_imlib_render( image, thumb_width , thumb_height );
			//tnimage = gdk_imlib_clone_scaled_image( image, thumb_width, thumb_height);
			if ( gdk_pixbuf_get_has_alpha ( image ) ) // image contains alpha channel - so compose it :)
				output_image = gdk_pixbuf_composite_color_simple ( image, thumb_width, thumb_height, temp_thumb_render_quality, 255, 
																							check_size, checkcolora, checkcolorb ) ;
			else { // image has no alpha channel - so we do some simple scaling
				output_image = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( image ), gdk_pixbuf_get_has_alpha ( image ), 
															gdk_pixbuf_get_bits_per_sample ( image ),
															thumb_width, thumb_height ) ;
				gdk_pixbuf_scale	( image, output_image, 0, 0, thumb_width, thumb_height, 0, 0, 
										scale, scale, temp_thumb_render_quality ) ;
			}
			// now we create our thumbnail to be saved as an icon....
			//tnimage = gdk_pixbuf_scale_simple ( image, thumb_width, thumb_height, temp_thumb_render_quality ) ;
			tnimage = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( image ), gdk_pixbuf_get_has_alpha ( image ), 
														gdk_pixbuf_get_bits_per_sample ( image ),
														thumb_width, thumb_height ) ;
			gdk_pixbuf_scale	( image, tnimage, 0, 0, thumb_width, thumb_height, 0, 0, 
									scale, scale, temp_thumb_render_quality ) ;
		} else {
			//gdk_imlib_render( image, image->rgb_width , image->rgb_height );
			//tnimage = gdk_imlib_clone_scaled_image( image, image->rgb_width, image->rgb_height);
			if ( gdk_pixbuf_get_has_alpha ( image ) ) // image contains alpha channel - so compose it :)
			{
				output_image = gdk_pixbuf_composite_color_simple ( image, gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image), 
														temp_thumb_render_quality, 255, check_size, checkcolora, checkcolorb ) ;
				if ( isthumb == FALSE ) // have we saved this thumbnail already ? no? then...
				{ // now we create our thumbnail to be saved as an icon....
					//tnimage = gdk_pixbuf_scale_simple ( image, gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image), temp_thumb_render_quality ) ;
					tnimage = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( image ), gdk_pixbuf_get_has_alpha ( image ), 
																gdk_pixbuf_get_bits_per_sample ( image ),
																gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image) ) ;
					gdk_pixbuf_scale	( image, tnimage, 0, 0, gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image), 0, 0, 
											1, 1, temp_thumb_render_quality ) ;
				}
			} else { // image has no alpha channel - so we do some simple scaling
				/*output_image = gdk_pixbuf_new ( gdk_pixbuf_get_colorspace( image ), gdk_pixbuf_get_has_alpha ( image ), 
															gdk_pixbuf_get_bits_per_sample ( image ),
															gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image) ) ;
				gdk_pixbuf_scale	( image, output_image, 0, 0, gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image), 0, 0, 
										1, 1, temp_thumb_render_quality ) ;*/
				output_image = gdk_pixbuf_copy ( image ) ;
				tnimage = gdk_pixbuf_copy ( output_image ) ;
			}
		}
		
		if ( image ) gdk_pixbuf_unref ( image ) ;
		/*if ( ! tnimage )
			tnimage = gdk_pixbuf_new_from_xpm_data ( (const char**) brokenimage_xpm ) ;*/
		if ( ( image = gdk_pixbuf_copy ( output_image ) ) == FALSE ) not_loaded = TRUE ;
		
		printd("il: width = "); printd(text_from_var(thumb_width)); printd(", height = "); printd(text_from_var(thumb_height)); printd("\n" );
		if ( tnimage )
		{
			printd("il: imwidth = "); printd(text_from_var(gdk_pixbuf_get_width(tnimage))); 
			printd(", imheight = "); printd(text_from_var(gdk_pixbuf_get_width(tnimage))); printd("\n" );
		}
		
		// now go on with the thumbnail savings...
		if ( isthumb == TRUE )
		{
			printd("il: thumbnail doesn't have to be saved.\n");
		} else {
			struct stat buf ;
			char thumbdir[2048] ;
			printd("il: thumbnail is being saved...\n");
			if ( LATS_RC_THUMB_IN_HOME == TRUE )
				sprintf ( thumbdir, "%s%s", gnome_vfs_expand_initial_tilde ( LATS_RC_DIR ), LATS_RC_THUMB_DIR ) ;
			else
				sprintf ( thumbdir, "%s", LATS_THUMB_DIR ) ;
			stat ( thumbdir, &buf );
			if ( ! S_ISDIR(buf.st_mode) )
			{
				printd("il: thumbnail-dir has to be created first...\n");
				if ( ! mkdir ( thumbdir,  S_IRWXU ) )
					printd("il: ok. now go for the thumbnail...\n");
				else
					printd("il: !!! .thumbnails could not be created!!\n");
			}
			if ( pixbuf_to_file_as_png( tnimage, thumbfilename ) )
				printd("il: thumbnail saved.\n");
			else
				printd("il: !!! thumbnail couldn't be saved!\n");
		}
		// put thumbnail into iconlist!
		gdk_threads_enter () ;
		if ( image && task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE && icon_pos >= 0 ) 
		{	// now we replace the temporary icon in the iconlist with the real one!
			char name[2048] ;
			struct stat buf ;
			stat ( filename, &buf ) ;
			strcpy ( name, filename ) ;
			strcat ( name, " (" ) ;
			strcat ( name, text_from_size ( buf.st_size ) ) ;
			strcat ( name, ")" ) ;
			/*{
				GdkImlibImage *im ;
				GdkPixmap *pix ;
				GdkBitmap *msk ;
				gdk_pixbuf_render_pixmap_and_mask ( image, &pix, &msk, 255 ) ;
				im = gdk_imlib_create_image_from_drawable ( pix, msk, 0, 0, gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image) ) ;
				
				
				printd("il: insert image into iconlist...\n");
				gnome_icon_list_remove ( gil, icon_pos ) ;
				gnome_icon_list_insert_imlib ( gil, icon_pos, im, name ) ;
				gnome_icon_list_remove ( gil2, icon_pos ) ;
				gnome_icon_list_insert_imlib ( gil2, icon_pos, im, name ) ;
				//gdk_imlib_destroy_image ( im ) ;
				gdk_pixmap_unref ( pix ) ;
				gdk_bitmap_unref ( msk ) ;
			}*/
			{
				gnome_icon_list_remove ( gil, icon_pos ) ;
				gnome_icon_list_remove ( gil2, icon_pos ) ;
				gnome_icon_list_insert ( gil, icon_pos, thumbfilename, name ) ;
				gnome_icon_list_insert ( gil2, icon_pos, thumbfilename, name ) ;
			}
		} else 
			printd("il: don't insert image into iconlist.\n");
		
		if ( tnimage ) gdk_pixbuf_unref (tnimage);
		if ( output_image ) gdk_pixbuf_unref (output_image);
	} else {
		printd("il: !!! Could not load image.\n");
		printd("il: !!! I will create an empty file\n");
		printd("il: !!! in order not to take so long\n");
		printd("il: !!! next time when loading it...\n");
		creat ( thumbfilename, S_IRWXU | O_EXCL ) ;
		not_loaded = TRUE ;
	}
	
	if ( not_loaded )
	{
		// uncomment this if you want a retry every load
		//unlink ( thumbfilename ) ;
		if ( image ) gdk_pixbuf_unref ( image ) ;
		//image = gdk_pixbuf_new_from_xpm_data ( (const char**) brokenimage_xpm ) ;
		//if ( image ) gdk_pixbuf_unref ( image ) ;
		image = NULL ;
	}
	
	return image ;
}


void
read_dir_from_combo_thread ( int FORCE_RELOAD )
{
	char directory[2048] ;
	GtkWidget *parentwindow;
	GtkWidget *window;
	struct stat buf;
	GtkProgress *progress;
	GtkWidget *appbar1;
	GtkWidget *dirdisplay ;
	char finishline [2048] ;
	struct dirent **namelist;
	int n;
	int own_task = task_add_to_queue ( "reload_dir_list", NULL ) ;
	// let's get the right backgroundcolors for the list...
	GtkWidget *imagelist = lookup_widget ( MainWindow, "imagelist" ) ;
	guint32 bg_red = imagelist->style->bg->red ;
	guint32 bg_green = imagelist->style->bg->green ;
	guint32 bg_blue = imagelist->style->bg->blue ;
	GdkColor bg_color_good = { 0, bg_red, bg_green, bg_blue } ;
	GdkColor fg_color_bad = { 0, 35535, 35535, 35535 } ;
	GdkColor bg_color_bad = { 0, bg_red, bg_green, bg_blue } ;
	
	thread_count++ ;
	
	gdk_threads_enter () ;
	
	file_filter_use = gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(
									lookup_widget ( PrefsWindow, "prefsfiltercheckbutton1" )) ) ; // don't show every file
	file_filter_hide_dot_files = gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(
									lookup_widget ( PrefsWindow, "prefsfiltercheckbutton2" )) ) ; // don't show hidden files
	file_filter_hide_non_multimedia_files = gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(
									lookup_widget ( PrefsWindow, "prefsfiltercheckbutton3" )) ) ; // only show multimedia files
	file_filter_hide_audio = gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(
									lookup_widget ( PrefsWindow, "prefsfiltercheckbutton4" )) ) ; // don't show images
	file_filter_hide_images = gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(
									lookup_widget ( PrefsWindow, "prefsfiltercheckbutton5" )) ) ; // don't show movies
	file_filter_hide_movies = gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(
									lookup_widget ( PrefsWindow, "prefsfiltercheckbutton6" )) ) ; // don't show audio files
	
	window = lookup_widget( MainWindow, "combo_entry1" );
    parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
	
	appbar1 = lookup_widget (window, "appbar1");
	progress = gnome_appbar_get_progress (GNOME_APPBAR (appbar1));

	dirdisplay = lookup_widget ( MainWindow, "findimage" ) ;
	gtk_widget_set_sensitive ( dirdisplay, FALSE );
	
	update_screen ();
	
	strcpy ( directory, gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ) ;
	strcpy ( directory, gnome_vfs_expand_initial_tilde ( directory ) ) ;
	
	gnome_icon_list_clear ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")) ) ;
	gnome_icon_list_clear ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")) ) ;

	chdir (directory);
	
	gnome_appbar_set_status (GNOME_APPBAR (appbar1), _("Searching for images..."));

	gdk_threads_leave () ;

	n = scandir ("./", &namelist, 0, alphasort);

	printd("il: dirname!\n");
	printd("il: new directory!\n");
	printd("il: "); printd(directory); printd("\n");
	strcpy(dirname, directory) ;
	printd("il: new dir "); printd(dirname); printd("\n");
	
	if ( n>= 0 )
	{
		int imagelist_row = 0;
		int c, piccount = 0 ;
		GtkCList *imagelist, *dirlist ;
		gchar *imagelist_entry[4], *dirlist_entry[2];
		char cwd[2048] ;
		GdkPixmap *loading_picture ;
		GnomeIconList *gil, *gil2 ;
		GdkImlibImage *loadpic ;
		
		gdk_threads_enter () ;
		
		// this is for the loading-thumbnail in the iconlists
		gil = GNOME_ICON_LIST(lookup_widget(MainWindow, "iconlist")) ;
		gil2 = GNOME_ICON_LIST(lookup_widget(MainWindow, "iconlist1")) ;
		loadpic = gdk_imlib_create_image_from_xpm_data( (char**) loading_xpm ) ;
		//
		
		loading_picture = gdk_pixmap_create_from_xpm_d ( lookup_widget(MainWindow, "MainWindow")->window,
													NULL, NULL, (gchar**) loading_xpm ) ;
		imagelist = GTK_CLIST(lookup_widget( MainWindow, "imagelist" ));
		dirlist = GTK_CLIST(lookup_widget( MainWindow, "dirlist" ));
		gtk_clist_clear (dirlist);
		gtk_clist_set_column_auto_resize ( dirlist, 0, TRUE );
		dirlist_entry[0] = "." ;
		dirlist_entry[1] = NULL ;
		gtk_clist_append ( dirlist, dirlist_entry );
		
		getcwd ( cwd, 2048 ) ;
		if ( ! ( strlen(cwd) == 1 && cwd[0] == 47 ) )
		{
			dirlist_entry[0] = ".." ;
			dirlist_entry[1] = NULL ;
			gtk_clist_append ( dirlist, dirlist_entry );
			//printf("'%s'\n", cwd);
		}
		
		gtk_clist_set_column_justification ( dirlist, 0, GTK_JUSTIFY_LEFT );
		gtk_clist_set_column_justification ( dirlist, 1, GTK_JUSTIFY_RIGHT );
		gtk_clist_set_column_auto_resize ( dirlist, 0, TRUE );
		gtk_clist_set_column_auto_resize ( dirlist, 1, TRUE );
		
		gtk_clist_clear (imagelist);
		gtk_clist_set_column_visibility ( imagelist, 0, thumbnails ); // Thumbnails visible??
		gtk_clist_set_column_justification ( imagelist, 0, GTK_JUSTIFY_CENTER );
		gtk_clist_set_column_justification ( imagelist, 1, GTK_JUSTIFY_LEFT );
		gtk_clist_set_column_justification ( imagelist, 2, GTK_JUSTIFY_RIGHT );
		gtk_clist_set_column_justification ( imagelist, 3, GTK_JUSTIFY_CENTER );
		gtk_clist_set_column_auto_resize ( imagelist, 0, TRUE );
		gtk_clist_set_column_auto_resize ( imagelist, 1, TRUE );
		gtk_clist_set_column_auto_resize ( imagelist, 2, TRUE );
		gtk_clist_set_column_auto_resize ( imagelist, 3, TRUE );
		gtk_clist_set_row_height ( imagelist, no_thumb_size );
		
		gtk_clist_freeze ( imagelist );
		//gtk_clist_freeze ( dirlist );
		
		//gtk_widget_set_usize (  GTK_WIDGET( lookup_widget ( MainWindow, "frame1" ) ),  -1, 1 ) ; // hide imagelist ***
		gtk_widget_hide ( lookup_widget ( MainWindow, "frame1" ) ) ;
		gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, 
								lookup_widget ( MainWindow, "findimage" ) -> allocation.height ) ;
		
		gnome_icon_list_freeze ( gil ) ;
		gnome_icon_list_freeze ( gil2 ) ;
		
		for (c = 0; ( c < n && task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE ) ; c++)
		{
			char fullname[2048] ;
			
			gtk_progress_set_value (progress, ((gfloat) c) / (gfloat) (n - 1) * 100);
			
			//update_screen ();
			
			strcpy (fullname, filename);
			strcat (fullname, namelist[c]->d_name);
			//
			// What do we do next?
			//
			if ( file_filter(fullname) == TRUE && isdir(fullname) == FALSE )
			{
				//GdkImlibImage *im;
				//GdkPixmap *imagelist_entry_pixmap;
				
				piccount++ ; 
				printd("il: File: "); printd(text_from_var(piccount));
				printd(" "); printd(fullname); printd("\n");
				//printd("il: Put image into list...\n");
				//printd("il: stat\n");
				if ( piccount == 1 &&
					gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) != 1 )
				{
					//gtk_widget_set_usize (  GTK_WIDGET( lookup_widget ( MainWindow, "frame1" ) ),  -1, 220 ) ; // show imagelist ***
					gtk_widget_show ( lookup_widget ( MainWindow, "frame1" ) ) ;
					if ( GTK_WIDGET_SENSITIVE ( lookup_widget( MainWindow, "frame1movebutton2" ) ) == TRUE )
					{
						gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, -1 ) ;
						gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton1" ) , TRUE ) ;
						gtk_widget_show ( lookup_widget( MainWindow, "frame1nb" ) ) ;
					} else {
						gtk_widget_set_usize ( lookup_widget( MainWindow, "frame2" ), -1, 
									lookup_widget ( MainWindow, "findimage" ) -> allocation.height - 
									lookup_widget ( MainWindow, "hbox11" ) -> allocation.height ) ;
					}
					//gtk_widget_set_sensitive ( lookup_widget( MainWindow, "frame1movebutton2" ) , TRUE ) ;
					//gtk_widget_show ( lookup_widget ( MainWindow, "frame2" ) ) ;
				}
				stat ( fullname, &buf );
				printd("il: Filesize: "); printd(text_from_var(buf.st_size)); printd("\n");
				imagelist_entry[0] = NULL;//image ;
				imagelist_entry[1] = fullname;
				imagelist_entry[2] = text_from_size(buf.st_size) ;
				imagelist_entry[3] = text_from_time(buf.st_mtime) ;
				gtk_clist_append ( imagelist, imagelist_entry);
				if ( thumbnails == TRUE )
				{
					//printd("il: enter thumbnail\n");
					//gtk_clist_set_foreground ( imagelist, imagelist_row, &fg_color_good ) ;
					gtk_clist_set_background ( imagelist, imagelist_row, &bg_color_good ) ;
					gtk_clist_set_pixmap ( imagelist, imagelist_row, 0, loading_picture , NULL );
					gnome_icon_list_append_imlib ( 	gil, 
													//imagelist_row, 
													loadpic, 
													fullname );
					gnome_icon_list_append_imlib ( 	gil2, 
													//imagelist_row, 
													loadpic, 
													fullname );
					//printd("il: thumbnail entered\n");
					//if (im) gdk_imlib_destroy_image (im);
				}
				imagelist_row++;
				//printd("il: Image put into list. \n");
			} else if ( file_filter(fullname) == TRUE && isdir(fullname) == TRUE ) {
				//printd("il: Put dir into list...\n");
				dirlist_entry[0] = fullname ;
				dirlist_entry[1] = NULL ;
				gtk_clist_append ( dirlist, dirlist_entry);
				printd("il: Directory: "); printd(dirlist_entry[0]); printd("\n");
			}
			// and now free some memory.. :)
			free (namelist[c]);
		}
		gnome_icon_list_thaw ( gil ) ;
		gnome_icon_list_thaw ( gil2 ) ;
		gtk_clist_thaw ( imagelist );
		//gtk_clist_thaw ( dirlist );
		printd("il: list filled.\n");
			
		gtk_progress_set_value (progress, 0);
		gtk_widget_set_sensitive ( dirdisplay, TRUE );
		update_screen ();
		
		gtk_progress_configure ( GTK_PROGRESS(lookup_widget ( MainWindow, "progressbar1" )),
					0, 0, imagelist_row ) ;
		gtk_progress_configure ( GTK_PROGRESS(lookup_widget ( FullscreenWindowProgressbar, "progressbar1" )),
					0, 0, imagelist_row ) ;

		if ( thumbnails == TRUE && imagelist_row > 0 )
		{
			int max_row_height = no_thumb_size ;

			//gtk_widget_set_sensitive ( lookup_widget(MainWindow,"frame2"), FALSE ) ;
			
			gnome_appbar_set_status (GNOME_APPBAR (appbar1), _("Creating preview of images..."));
			
			printd("il: We will load "); printd(text_from_var(imagelist_row));
			printd(" thumbnails.\n");
			// now we enter the thumbnails!
			for ( c = 0; ( c < imagelist_row && task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE ); c++ )
			{
				char *filename ;
				gtk_progress_set_value (progress, ((gfloat) c) / (gfloat) imagelist_row * 100);
				//printd("thumnail number = "); printd(c); printd("\n");
				gtk_clist_get_text ( imagelist, c, 1, &filename);
				//printd("load thumb... "); printd(filename); printd("\n");
				printd("il: load image\n");
				if ( gtk_clist_get_pixmap ( imagelist, c, 0, NULL, NULL ) != 0 ) 
				{
					GdkPixbuf *im;
					GdkPixmap *imagelist_entry_pixmap;
					GdkBitmap *imagelist_entry_mask;
					
					im = get_thumbnail(filename, FORCE_RELOAD, c);
					
					if ( im && task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE )
					{
						if ( gdk_pixbuf_get_height(im) > max_row_height )
						{
							max_row_height = MIN( gdk_pixbuf_get_height(im), max_thumb_size ) ;
							printd("il: new thumb height = "); printd(text_from_var(max_row_height)); printd("\n");
							gtk_clist_set_row_height ( imagelist, max_row_height );
						}
						printd("il: create imagelist_entry_pixmap\n");
						//imagelist_entry_pixmap = gdk_imlib_move_image(im);
						//imagelist_entry_mask = gdk_imlib_move_mask(im);
						gdk_pixbuf_render_pixmap_and_mask ( im, &imagelist_entry_pixmap, &imagelist_entry_mask, 255 ) ;
						if (imagelist_entry_pixmap) 
							printd("il: imagelist_entry_pixmap created\n");
						else
							printd("il: !!! imagelist_entry_pixmap not created\n");

						gtk_clist_set_pixmap ( imagelist, c, 0, imagelist_entry_pixmap, imagelist_entry_mask );
						printd("il: thumbnail inserted\n");
						
						//gtk_clist_set_foreground ( imagelist, c, &fg_color_good ) ;
						gtk_clist_set_background ( imagelist, c, &bg_color_good ) ;

						printd("il: Image inserted into list. \n");
						gtk_clist_set_selectable ( imagelist, c, TRUE ) ;
						printd("il: free im\n");
						if ( im ) gdk_pixbuf_unref (im);
						if ( imagelist_entry_pixmap ) gdk_pixmap_unref ( imagelist_entry_pixmap ) ;
						if ( imagelist_entry_mask ) gdk_bitmap_unref ( imagelist_entry_mask ) ;
					} else {
						printd("il: !!! thumbnail could not be loaded!\n");
						//gtk_clist_set_pixmap ( imagelist, c, 0, NULL, NULL ) ;
						//gtk_clist_set_text ( imagelist, c, 0, "failed" ) ;
						gtk_clist_set_text ( imagelist, c, 0, gnome_mime_type(filename) ) ;
						gtk_clist_set_selectable ( imagelist, c, FALSE ) ;
						gtk_clist_set_foreground ( imagelist, c, &fg_color_bad ) ;
						gtk_clist_set_background ( imagelist, c, &bg_color_bad ) ;
					}
				} else
					printd("il: !!! thumbnail already loaded!\n");
			}
			printd("il: thumbs inserted.\n");
			gtk_clist_set_row_height ( imagelist, max_row_height );
		} else printd("il: we don't want thumbnails.\n");
		printd("il: --- finished reading.\n");
		//gtk_clist_set_reorderable ( imagelist, TRUE ) ;
		//gtk_clist_set_use_drag_icons ( imagelist, TRUE ) ;
		update_screen () ;
		printd("il: free some memory...\n");
		if ( loadpic ) gdk_imlib_destroy_image ( loadpic ) ;
		if ( loading_picture ) gdk_pixmap_unref ( loading_picture ) ;
		//gtk_widget_set_sensitive ( lookup_widget(MainWindow,"frame2"), TRUE ) ;
		//update_screen () ;
		printd("il: now show what we've got...\n");
		if ( imagelist_row > 0 ) 
			sprintf ( finishline, "%d files", imagelist_row ) ;
		else
			sprintf ( finishline, "no files" ) ;
		
		if ( gtk_toggle_button_get_active ( 
								GTK_TOGGLE_BUTTON(
									lookup_widget ( MainWindow, "spider" )) )  &&
			 task_already_exists_in_queue ( "stop_dir_list", NULL ) == FALSE )
		{
			// initiate dir-tree-spider!!!
			go_spider_go () ; // this will create another thread... so don't worry :)
		}
		
	} else { 
		printd ("il: open_dir"); 
		sprintf ( finishline, "no files" ) ;
		gdk_threads_enter () ;
	}

	free (namelist);
	
	gtk_progress_set_value (progress, 0);
	gnome_appbar_set_status (GNOME_APPBAR (appbar1),
									 _( finishline ));
	gtk_widget_set_sensitive ( dirdisplay, TRUE );

	gdk_threads_leave () ;
	task_remove_from_queue ( own_task ) ;
	thread_count-- ;
}


void
read_dir_from_combo_start_thread ( int FORCE_RELOAD )
{
	pthread_t dirlist_thread ;

	if ( FORCE_RELOAD )
		pthread_create ( &dirlist_thread, NULL, (void*)&read_dir_from_combo_thread, &FORCE_RELOAD ) ;
	else
		pthread_create ( &dirlist_thread, NULL, (void*)&read_dir_from_combo_thread, NULL ) ;
	pthread_detach ( dirlist_thread ) ;
	
	//read_dir_from_combo_thread ( FORCE_RELOAD, MainWindow, directory, imagedir, 
	//								progress, appbar1, dirdisplay ) ;
}


void
read_dir_from_combo ( int FORCE_RELOAD, gpointer user_data )
{
	char *directory ;
	GtkWidget *parentwindow;
	GtkWidget *window;
	struct stat buf;
	DIR *imagedir ;
	GtkProgress *progress;
	GtkWidget *appbar1;
	GtkWidget *dirdisplay ;
	char finishline [2048] ;

	window = lookup_widget( MainWindow, "combo_entry1" );
    parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
	
	appbar1 = lookup_widget (window, "appbar1");
	progress = gnome_appbar_get_progress (GNOME_APPBAR (appbar1));

	dirdisplay = lookup_widget ( MainWindow, "findimage" ) ;
	gtk_widget_set_sensitive ( dirdisplay, FALSE );
	
	refresh_screen ();
	
	// wait a moment...
	usleep ( 10000 ) ;
	
	directory = gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ;
	directory = gnome_vfs_expand_initial_tilde ( directory ) ;
	
	//printd("il: stat\n");
	stat ( directory, &buf );

	if (  S_ISDIR (buf.st_mode) && ( imagedir = opendir (directory) )  )
	{
		// make sure the spider isn't alive anymore...
		int own_task = task_add_to_queue ( "stop_spider", NULL ) ;
		
		while ( task_already_exists_in_queue ( "go_spider_go", NULL ) )
		{
			usleep ( 1000 ) ;
			refresh_screen () ;
			//printd ( "il: waiting for other spider threads to shutdown...\n" ) ;
		}
		task_remove_from_queue ( own_task ) ;		
			
		// now make sure the dir_list shuts down...
		own_task = task_add_to_queue ( "stop_dir_list", NULL ) ;
		
		while ( task_already_exists_in_queue ( "reload_dir_list", NULL ) )
		{
			usleep ( 1000 ) ;
			refresh_screen () ;
			//printd ( "il: waiting for other dir_list threads to shutdown...\n" ) ;
		}
		task_remove_from_queue ( own_task ) ;
		
		closedir (imagedir);
		
		// now read the new dir!
		read_dir_from_combo_start_thread ( FORCE_RELOAD ) ;
		
	} else {
		sprintf ( finishline, "Directory not found!" ) ;
		gtk_widget_set_sensitive ( dirdisplay, TRUE );
	}
	
	gtk_widget_set_sensitive ( dirdisplay, TRUE );
	free (directory) ;
}


void
go_to_previous_image_in_list ( void )
{
	GList *sList;
	GtkCList *clist ;
	int rNum = 0 ;
	int inzest = 0 ; // counter for self-looping
	
	clist = GTK_CLIST(lookup_widget(MainWindow,"imagelist")) ;
	
	// are there any images in the list?? otherwise quit from here...
	if ( gtk_clist_get_text ( clist, 0, 1, NULL ) == FALSE ) return ;
	
	sList = clist->selection;
	if ( sList ) 
	{
		rNum = (int) sList->data ;
		//return rNum ;
	} else {
		rNum = -1 ;
		//return FALSE ;
	}

	printd("il: selected row in imagelist = "); printd(text_from_var(rNum)); printd("\n");

	rNum-- ;
	//printf("il: row = %d\n", rNum ) ;
	while ( ( ( gtk_clist_get_selectable ( clist, rNum ) == FALSE &&
			gtk_clist_get_text ( clist, rNum, 1, NULL ) != FALSE ) || rNum < 0 || rNum > clist->rows-1 ) &&
			inzest < 5 )
	{
		rNum--;
		if ( rNum < 0 ) 
		{
			rNum = clist->rows-1 ;
			inzest++ ;
		}
		//printf("il: row = %d\n", rNum ) ;
	}

	gtk_clist_select_row ( clist, rNum, 0 ) ;
	gtk_clist_moveto ( clist, rNum, 0, 0.5, 0 ) ;
	gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), rNum ) ;
	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) == 1 )
		move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), rNum ) ;
	gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum ) ;
	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 1 )
		move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum ) ;
}


void
go_to_next_image_in_list ( void )
{
	GList *sList;
	GtkCList *clist ;
	int rNum = 0 ;
	int inzest = 0 ; // counter for self-looping
	
	clist = GTK_CLIST(lookup_widget(MainWindow,"imagelist")) ;
	
	// are there any images in the list?? otherwise quit from here...
	if ( gtk_clist_get_text ( clist, 0, 1, NULL ) == FALSE ) return ;
	
	sList = clist->selection;
	if ( sList ) 
	{
		rNum = (int) sList->data ;
		//return rNum ;
	} else {
		rNum = -1 ;
		//return FALSE ;
	}

	printd("il: selected row in imagelist = "); printd(text_from_var(rNum)); printd("\n");

	//if ( rNum >= clist->rows-1 ) rNum = -1 ;
	
	rNum++ ;
	//printf("il: row = %d\n", rNum ) ;
	while ( ( ( gtk_clist_get_selectable ( clist, rNum ) == FALSE &&
			gtk_clist_get_text ( clist, rNum, 1, NULL ) != FALSE ) || rNum > clist->rows-1 ) &&
			inzest < 5 )
	{
		rNum++ ;
		if ( rNum > clist->rows-1 ) 
		{
			rNum = 0 ;
			inzest++ ;
		}
		//printf("il: row = %d\n", rNum ) ;
	}

	gtk_clist_select_row ( clist, rNum, 0 ) ;
	gtk_clist_moveto ( clist, rNum, 0, 0.5, 0 ) ;
	gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), rNum ) ;
	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) == 1 )
		move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")), rNum ) ;
	gnome_icon_list_select_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum ) ;
	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"frame1nb")) ) == 1 )
		move_to_icon ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")), rNum ) ;
}


int
spider_gather_subdirinfo ( char *dirname, int depth )
{
	int count = 0 ;
	int current_dir_entries = 0 ;
	
	//printd ( "il: spider: subdir = " ); printd ( dirname ); printd ( "\n" );
	printd ( "il: spider: subdir found at depth " ) ; printd ( text_from_var(depth) ) ; printd ( " = " ) ; 
	printd ( dirname ) ; //printd ( "\n" ) ;
	
	// break out if dir is not accessible or a link !
	if ( /*isdir(dirname) == FALSE || islink(dirname) ||*/ depth >= spider_max_dir_depth ||
			task_already_exists_in_queue ( "stop_spider", NULL ) ) 
	{
		printd ( "il: spider: break out from subdir!\n" ) ;
		return 0 ; 
	}

	{
		struct dirent **namelist;
		//char cwd[2048], orgwd[2048] ;
		
		//getcwd ( orgwd, 2048 ) ;
		//chdir ( dirname ) ;
		//getcwd ( cwd, 2048 ) ;
		//chdir ( orgwd ) ;
		
		//if ( cwd [ strlen(cwd) - 1 ] != 47 )
		//	strcat ( cwd, "/" ) ;
		
		//strcat ( cwd, dirname ) ;
		
		//printd ( " = " ); printd ( cwd ); printd ( " = \n" );
		
		current_dir_entries = scandir (dirname, &namelist, 0, alphasort);
	
		if ( current_dir_entries > ( !strcmp(namelist[1]->d_name,"..") ? 2 : 1 ) )
		{
			int pos = 0 ;
			
			printd ( " ( " ); printd ( text_from_var( 
				current_dir_entries - ( !strcmp(namelist[1]->d_name,"..") ? 2 : 1 ) 
				) ); printd ( " entries )\n" );
			
			//printd( "il: spider: searching for subdirs...\n" );
			
			for ( pos = 0 ; ( pos < current_dir_entries && 
					task_already_exists_in_queue ( "stop_spider", NULL ) == FALSE ) ; pos++ )
			{
				char curname[2048] ;
				
				strcpy ( curname, dirname ) ;
				if ( curname [ strlen(curname) - 1 ] != 47 )
					strcat ( curname, "/" ) ;
				strcat ( curname, namelist[pos]->d_name ) ;
				
				//printd( "il: spider: examining file " ); printd ( curname ) ; printd ( "...\n" ) ;
				
				if ( file_filter( curname ) == TRUE )
				{
					int is_dir = isdir ( curname ) ;
					//int is_link = islink ( curname ) ;
					int is_link = FALSE ;
					if ( 	is_dir == TRUE &&
							is_link == FALSE &&
							strcmp(namelist[pos]->d_name, ".") != 0 && strcmp(namelist[pos]->d_name, "..") != 0 &&
							!( file_filter_hide_dot_files == TRUE && !strncmp(namelist[pos]->d_name, ".", 1) ) )
					{
						//printd ( "il: spider: subdir found at depth " ) ; printd ( text_from_var(depth) ) ; printd ( " = " ) ; 
						//printd ( curname ) ; printd ( "\n" ) ;
						count = count + spider_gather_subdirinfo ( curname, depth+1 ) ;
					} else if ( is_dir == FALSE &&
								is_link == FALSE )
						count++ ;
				}
			}
		} else {
			printd ( "il: spider: !!! could not read dirlist !!!\n" ) ;
		}
		
		free (namelist) ;
	}
	return count ;
}


void
spider_gather_dirinfo ( char *dirname, GtkCList *dirlist )
{
	char *current_directory, directory_name[2048], *currentdirname ;
	int count = 0, dirlistpos = 0 ;
	int current_dir_entries = 0 ;
	struct dirent **namelist;
	//guint32 bg_red = GTK_WIDGET(dirlist)->style->bg->red ;
	//guint32 bg_green = GTK_WIDGET(dirlist)->style->bg->green ;
	//guint32 bg_blue = GTK_WIDGET(dirlist)->style->bg->blue ;
	GdkColor fg_color = { 0, 0, 0, 35535 } ;
	//GdkColor bg_color = { 0, bg_red, bg_green, bg_blue } ;
		
	if ( task_already_exists_in_queue ( "stop_spider", NULL ) ) return ;

	//gdk_threads_enter () ;
	
	current_directory = gtk_entry_get_text( GTK_ENTRY( 
								gtk_widget_get_ancestor (	GTK_WIDGET(lookup_widget( MainWindow, "combo_entry1" )), 
															GTK_TYPE_EDITABLE )  )   ) ;
	
	//gdk_threads_leave () ;

	if ( !strcmp( dirname, "." ) || !strcmp( dirname, ".." ) ) 
	{
		dirlistpos = 0 ; // go to start of dirlist
		while ( gtk_clist_get_text ( dirlist, dirlistpos, 0, &currentdirname ) &&
				task_already_exists_in_queue ( "stop_spider", NULL ) == FALSE )
		{
			if ( !strcmp ( currentdirname, dirname ) )
			{
				gdk_threads_enter () ;
				gtk_clist_set_text ( dirlist, dirlistpos, 1, " " ) ;
				gdk_threads_leave () ;
			}
			dirlistpos++ ;
		}
		return ;
	} else {
		char cwd[2048], result[32] ;
		printd( "il: spider: gather from child dir...\n" );
		getcwd ( cwd, 2048 ) ;
		if ( cwd [ strlen(cwd) - 1 ] != 47 )
			strcat ( cwd, "/" ) ;
		strcpy ( directory_name, cwd ) ;
		strcat ( directory_name, dirname ) ;

		printd ( "il: spider: counting viewable content from " ); printd ( directory_name ); printd ( "...\n" );
	
		current_dir_entries = scandir (directory_name, &namelist, 0, alphasort);
	
		if ( current_dir_entries >= 0 ) 
		{
			int pos = 0 ;
			
			for ( pos = 0 ; ( pos < current_dir_entries && task_already_exists_in_queue ( "stop_spider", NULL ) == FALSE ) ; pos++ )
			{
				char curname[2048] ;
	
				if ( directory_name [ strlen(directory_name) - 1 ] != 47 )
					strcat ( directory_name, "/" ) ;
	
				
				strcpy ( curname, directory_name ) ;
				strcat ( curname, namelist[pos]->d_name ) ;
				
				if ( file_filter( curname ) == TRUE )
				{
					int is_dir = isdir ( curname ) ;
					int is_link = islink ( curname ) ;
					if ( 	is_dir == TRUE &&
							is_link == FALSE &&
							strcmp(namelist[pos]->d_name, ".") != 0 && strcmp(namelist[pos]->d_name, "..") != 0 )
					{
						count = count + spider_gather_subdirinfo ( curname, 1 ) ;
					} else if ( is_dir == FALSE &&
								is_link == FALSE )
						count++ ;
				}
			}
			
			printd ( "il: spider: found " ); printd ( text_from_var(count) ); printd ( " files...\n" );
		}
		
		printd ( "il: spider: done counting. writing counted amount into dirlist...\n" );
		
		if ( count > 0 )
			strcpy ( result, text_from_var(count) ) ;
		else
			strcpy ( result, " " ) ;
		
		dirlistpos = 0 ; // go to start of dirlist
		while ( gtk_clist_get_text ( dirlist, dirlistpos, 0, &currentdirname ) &&
				task_already_exists_in_queue ( "stop_spider", NULL ) == FALSE )
		{
			if ( !strcmp ( currentdirname, dirname ) )
			{
				gdk_threads_enter () ;
				gtk_clist_set_text ( dirlist, dirlistpos, 1, result ) ;
				if ( strcmp ( result, " " ) )
				{
					gtk_clist_set_foreground ( dirlist, dirlistpos, &fg_color ) ;
					//gtk_clist_set_background ( dirlist, dirlistpos, &bg_color ) ;
				}
				gdk_threads_leave () ;
			}
			dirlistpos++ ;
		}
		
		printd ( "il: spider: done.\n" );
		free (namelist) ;
	}
}


void
go_spider_thread ( void )
{
	int own_task = task_add_to_queue ( "go_spider_go", NULL ), dirlistpos = 0 ;
	GtkCList *dirlist ;
	char *dirname ;
	
	printd( "il: spider: Let out the spider to conquer earth... :) \n" ) ;
	
	gdk_threads_enter () ;
	
	dirlist = GTK_CLIST(lookup_widget( MainWindow, "dirlist" ));

	while ( gtk_clist_get_text ( dirlist, dirlistpos, 0, &dirname ) &&
			task_already_exists_in_queue ( "stop_spider", NULL ) == FALSE )
	{
		gtk_clist_set_text ( dirlist, dirlistpos, 1, "..." ) ;
		dirlistpos++ ;
	}
	
	gdk_threads_leave () ;
	
	dirlistpos = 0 ; // go to start of dirlist
	while ( gtk_clist_get_text ( dirlist, dirlistpos, 0, &dirname ) &&
			task_already_exists_in_queue ( "stop_spider", NULL ) == FALSE )
	{
		spider_gather_dirinfo ( dirname, dirlist ) ;

		dirlistpos++;
	}
	
	printd( "il: spider: now that the spider conquered earth, we are done here.\n" ) ;
	
	task_remove_from_queue ( own_task ) ;
}


void
go_spider_go ( void )
{
	pthread_t spider_thread ;
	int own_task = task_add_to_queue ( "stop_spider", NULL ) ;
	
	while ( task_already_exists_in_queue ( "go_spider_go", NULL ) )
	{
		usleep ( 1000 ) ;
		refresh_screen () ;
		//printd ( "il: waiting for other spider threads to shutdown...\n" ) ;
	}
	task_remove_from_queue ( own_task ) ;

	pthread_create ( &spider_thread, NULL, (void*)&go_spider_thread, NULL ) ;
	pthread_detach ( spider_thread ) ;
	//go_spider_thread() ;
}


void
move_to_icon ( GnomeIconList *iconlist, int pos )
{
	float adjustment = 0.5 ;
	int icons = iconlist->icons ;
	int in_a_row = gnome_icon_list_get_items_per_line(iconlist) ;
	int count = icons, finish = FALSE ;
	
	while ( count > 0 && finish == FALSE )
	{
		if ( count - in_a_row > 0 )
		{
			count = count - in_a_row ;
		} else {
			finish = TRUE ;
		}
	}
	
	//printf("il: in_a_row = %d ; count = %d ; icons = %d ;\n", in_a_row, count, icons ) ;
	//printf("il: number of icons in bottom line = %d ;\n", in_a_row - ( in_a_row - count ) ) ;
	
	if ( pos < in_a_row ) adjustment = 0 ;
	if ( pos >= icons - ( in_a_row - ( in_a_row - count ) ) ) adjustment = 1 ;
	
	gnome_icon_list_moveto ( iconlist, pos, adjustment ) ;
}


void
check_for_initial_directory_read ( void )
{
	char directory[2048] ;
	GtkWidget *window, *parentwindow, *clist ;
	char *clist_entry ;
	struct stat buf ;

	if ( lookup_widget( MainWindow, "imageinfo" )->allocation.width < 2 )
		return ;
	
	window = lookup_widget( MainWindow, "combo_entry1" );
    parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
	
	strcpy ( directory, gnome_vfs_expand_initial_tilde(gtk_entry_get_text( GTK_ENTRY( parentwindow ) )) ) ;
	
	clist = lookup_widget ( MainWindow, "dirlist" ) ;
	
	if ( stat(directory, &buf) == 0 && 
			gtk_clist_get_text ( GTK_CLIST(clist), 0, 0, &clist_entry ) == FALSE )
		read_dir_from_combo ( FALSE, MainWindow );	
}

