#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <sys/types.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <dirent.h> 

#include <unistd.h>

#include <string.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#include <libgnomevfs/gnome-vfs.h>

#include <math.h>

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

char extern *LATS_RC_DIR ;
char extern *LATS_RC_THUMB_DIR ;
char extern *LATS_THUMB_DIR ;
int extern LATS_RC_THUMB_IN_HOME ;

GtkWidget extern *MainWindow ;
GtkWidget extern *FullscreenWindow ;

gboolean
isimage (char *filename)
{
  /* we might want to keep video/ too, if imlib didn't crash trying to make
     thumbnails of them... - presumably it's meant to work. */

  if ( strncmp (filename, ".", 1 ) && !strncmp (gnome_mime_type (filename), "image/", 6))
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

	if ( S_ISDIR (buf.st_mode) && strncmp (filename, ".", 1 ) && ( imagedir=opendir(filename) ) )
	{
		closedir(imagedir);
		return (TRUE);
	}
	
	return (FALSE);
}


GdkPixbuf
*get_thumbnail ( char *filename, int FORCE_RELOAD )
{
	GdkPixbuf *image = NULL ;
	double scale = 1 ;
	char thumbfilename[2048];
	int isthumb = FALSE ;
	int supported = TRUE ;
	int not_loaded = FALSE ;
	struct stat buf, buf2 ;
	guint32 checkcolora = 0xEEEEEE ; //( check_color_a * 256 * 256 ) + ( check_color_a * 256 ) + check_color_a ;
	guint32 checkcolorb = 0xEEEEEE ; //( check_color_b * 256 * 256 ) + ( check_color_b * 256 ) + check_color_b ;
	
	if ( LATS_RC_THUMB_IN_HOME == TRUE ) {
		int pos = 0 ;
		char cwd[2048] ;
		getcwd ( cwd, 2048 ) ;
		strcat ( cwd, "/" ) ;
		for ( pos = 0 ; pos < strlen(cwd) ; pos++ )
		{
			//printd( text_from_var(  strlen(index(cwd,47))  ) ); printd(" ");
			if ( cwd[pos] == 47 ) cwd[pos] = 183 ;
		}
		strcat ( cwd, filename ) ;
		printd(cwd); printd("\n");
		printd("put thumb into homedir\n");
		sprintf( thumbfilename, "%s%s%s.png", gnome_vfs_expand_initial_tilde ( LATS_RC_DIR ), LATS_RC_THUMB_DIR, cwd ) ;
	} else {
		printd("put thumb into local dir\n");
		sprintf( thumbfilename, "%s%s.png", LATS_THUMB_DIR, filename ) ;
	}
	
	printd(thumbfilename);printd("\n");
	
	stat ( thumbfilename, &buf );
	stat ( filename, &buf2 );
	
	/*if ( strstr ( filename, ".xcf" ) != NULL ) { // not yet supported :-/
		printd(".xcf nicht unterstützt.\n");
		supported = FALSE ;
	} else */if ( ( image = gdk_pixbuf_new_from_file(thumbfilename) ) && 
					buf.st_mtime >= buf2.st_mtime && 	// thumbnail ? newer than image?
					FORCE_RELOAD == FALSE  ) {
		printd("thumbnail vorhanden!\n");
		isthumb = TRUE ;
	} else /*if ( buf.st_size != 0 )*/ { // no thumbnail or image newer than thumbnail
		printd("lade bild...\n");
		image = gdk_pixbuf_new_from_file(filename);
		isthumb = FALSE ;
	} 
	
	if ( image == FALSE )
	{
		image = gdk_pixbuf_new_from_xpm_data ( (const char**) brokenimage_xpm ) ;
		not_loaded = TRUE ;
	}
	
	if ( image && supported == TRUE )
	{
		GnomeIconList *gil = GNOME_ICON_LIST(lookup_widget(MainWindow, "iconlist")) ;
		GnomeIconList *gil2 = GNOME_ICON_LIST(lookup_widget(MainWindow, "iconlist1")) ;
		int thumb_width ;
		int thumb_height ;
		GdkPixbuf *tnimage ;
		printd("thumbnail geladen\n");
		if ( isthumb == FALSE )
		{
			scale = ((double) thumb_size) / 
					((double) MAX (gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image)));
			unlink ( thumbfilename ) ;
		} else if ( isthumb == TRUE && gdk_pixbuf_get_width(image) <= max_thumb_size && gdk_pixbuf_get_height(image) <= max_thumb_size ) {
			scale = 1 ;
		} else {
			scale = ((double) max_thumb_size) / 
					((double) MAX (gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image)));
		}
		//printd("thumbnail scale = "); printd(text_from_var(scale)); printd("\n");
		printd("thumbnail scale = "); printd(text_from_var(scale*100)); printd("\n");
		
		thumb_width = ( gdk_pixbuf_get_width(image) ) * scale ;
		thumb_height = ( gdk_pixbuf_get_height(image) ) * scale ;
		
		//gdk_imlib_set_render_type ( thumb_render_quality ) ;
		
		if (  ( gdk_pixbuf_get_width(image) > thumb_size ) || ( gdk_pixbuf_get_height(image) > thumb_size ) )
		{
			//gdk_imlib_render( image, thumb_width , thumb_height );
			//tnimage = gdk_imlib_clone_scaled_image( image, thumb_width, thumb_height);
			tnimage = gdk_pixbuf_composite_color_simple ( image, thumb_width, thumb_height, thumb_render_quality, 255, 
																					check_size, checkcolora, checkcolorb ) ;
		} else {
			//gdk_imlib_render( image, image->rgb_width , image->rgb_height );
			//tnimage = gdk_imlib_clone_scaled_image( image, image->rgb_width, image->rgb_height);
			tnimage = gdk_pixbuf_composite_color_simple ( image, gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image), thumb_render_quality, 255, 
																					check_size, checkcolora, checkcolorb ) ;
		}
		
		if ( image ) gdk_pixbuf_unref ( image ) ;
		image = gdk_pixbuf_copy ( tnimage ) ;
		
		printd("width = "); printd(text_from_var(thumb_width)); printd(", height = "); printd(text_from_var(thumb_height)); printd("\n" );
		printd("imwidth = "); printd(text_from_var(gdk_pixbuf_get_width(tnimage))); 
		printd(", imheight = "); printd(text_from_var(gdk_pixbuf_get_width(tnimage))); printd("\n" );
		
		// no go on with the thumbnail savings...
		if ( isthumb == TRUE )
		{
			printd("thumbnail muss nicht gespeichert werden.\n");
		} else {
			struct stat buf ;
			char thumbdir[2048] ;
			printd("thumbnail wird gespeichert...\n");
			if ( LATS_RC_THUMB_IN_HOME == TRUE )
				sprintf ( thumbdir, "%s%s", gnome_vfs_expand_initial_tilde ( LATS_RC_DIR ), LATS_RC_THUMB_DIR ) ;
			else
				sprintf ( thumbdir, "%s", LATS_THUMB_DIR ) ;
			stat ( thumbdir, &buf );
			if ( ! S_ISDIR(buf.st_mode) )
			{
				printd("thumbnails Ordner muss erst noch erstellt werden...\n");
				if ( ! mkdir ( thumbdir,  S_IRWXU ) )
					printd("ok. nun zum thumbnail...\n");
				else
					printd("!!! .thumbnails konnte nicht erstellt werden!!\n");
			}
			if ( pixbuf_to_file_as_png( tnimage, thumbfilename ) )
				printd("thumbnail gespeichert.\n");
			else
				printd("!!! thumbnail konnte nicht gespeichert werden!\n");
		}
		if ( tnimage ) gdk_pixbuf_unref (tnimage);
		// put thumbnail into iconlist!
		printd("image in iconlist eintragen...\n");
		if ( image ) gnome_icon_list_append ( gil, thumbfilename, filename ) ;
		if ( image ) gnome_icon_list_append ( gil2, thumbfilename, filename ) ;
	} else {
		printd("!!! Bild konnte nicht geladen werden.\n");
		printd("!!! ich werden eine leere Datei anlegen,\n");
		printd("!!! damit wir das nächste Mal nicht so\n");
		printd("!!! lange warten müssen...\n");
		creat ( thumbfilename, S_IRWXU | O_EXCL ) ;
		image = gdk_pixbuf_new_from_xpm_data ( (const char**) brokenimage_xpm ) ;
	}
	
	if ( not_loaded )
	{
		unlink ( thumbfilename ) ;
		image = NULL ;
	}
	
	return image ;
}


void
read_dir_from_combo		               ( int FORCE_RELOAD, gpointer         user_data)
{
	char *directory ;
	GtkWidget *parentwindow;
	GtkWidget *window;
	struct stat buf;
	DIR *imagedir;
	GtkProgress *progress;
	GtkWidget *appbar1;
	GtkWidget *dirdisplay ;
	char finishline [2048] ;

	window = lookup_widget( user_data, "combo_entry1" );
    parentwindow = gtk_widget_get_ancestor (GTK_WIDGET(window), GTK_TYPE_EDITABLE);
	
	appbar1 = lookup_widget (window, "appbar1");
	progress = gnome_appbar_get_progress (GNOME_APPBAR (appbar1));

	dirdisplay = lookup_widget ( user_data, "findimage" ) ;
	gtk_widget_set_sensitive ( dirdisplay, FALSE );
	
	refresh_screen ();
	
	directory = gtk_entry_get_text( GTK_ENTRY( parentwindow ) ) ;
	directory = gnome_vfs_expand_initial_tilde ( directory ) ;
	
	//printd("stat\n");
	stat ( directory, &buf );
	
	if (  S_ISDIR (buf.st_mode) && ( imagedir = opendir (directory) )  )
	{
		struct dirent **namelist;
        int n;

		gnome_icon_list_clear ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist")) ) ;
		gnome_icon_list_clear ( GNOME_ICON_LIST(lookup_widget(MainWindow,"iconlist1")) ) ;

        closedir (imagedir);

		chdir (directory);
		
		gnome_appbar_set_status (GNOME_APPBAR (appbar1), _("Suche Bilddateien..."));
		
        n = scandir ("./", &namelist, 0, alphasort);

		printd("dirname!\n");
		printd("neues directory!\n");
		printd(directory); printd("\n");
		sprintf(dirname, "%s", directory) ;
		printd("neuer Ordner "); printd(dirname); printd("\n");
		
		if ( n>= 0 )
		{
			int imagelist_row = 0;
			int c, piccount = 0 ;
			GtkCList *imagelist, *dirlist ;
			gchar *imagelist_entry[4], *dirlist_entry[0];
			char cwd[2048] ;
			GdkPixmap *loading_picture ;
			loading_picture = gdk_pixmap_create_from_xpm_d ( lookup_widget(MainWindow, "MainWindow")->window,
														NULL, NULL, (gchar**) loading_xpm ) ;
			imagelist = GTK_CLIST(lookup_widget( user_data, "imagelist" ));
			dirlist = GTK_CLIST(lookup_widget( user_data, "dirlist" ));
			gtk_clist_clear (dirlist);
			gtk_clist_set_column_auto_resize ( dirlist, 0, TRUE );
			dirlist_entry[0] = "." ;
			gtk_clist_append ( dirlist, dirlist_entry );
			
			getcwd ( cwd, 2048 ) ;
			if ( ! ( strlen(cwd) == 1 && cwd[0] == 47 ) )
			{
				dirlist_entry[0] = ".." ;
				gtk_clist_append ( dirlist, dirlist_entry );
				//printf("'%s'\n", cwd);
			}
			
			gtk_clist_clear (imagelist);
			gtk_clist_set_column_visibility ( imagelist, 0, TRUE ); // Thumbnails visible??
			gtk_clist_set_column_justification ( imagelist, 0, GTK_JUSTIFY_CENTER );
			gtk_clist_set_column_justification ( imagelist, 1, GTK_JUSTIFY_LEFT );
			gtk_clist_set_column_justification ( imagelist, 2, GTK_JUSTIFY_RIGHT );
			gtk_clist_set_column_justification ( imagelist, 3, GTK_JUSTIFY_CENTER );
			gtk_clist_set_column_auto_resize ( imagelist, 0, TRUE );
			gtk_clist_set_column_auto_resize ( imagelist, 1, TRUE );
			gtk_clist_set_column_auto_resize ( imagelist, 2, TRUE );
			gtk_clist_set_column_auto_resize ( imagelist, 3, TRUE );
			gtk_clist_set_row_height ( imagelist, no_thumb_size );
			
			//gtk_clist_freeze ( imagelist );
			//gtk_clist_freeze ( dirlist );
			
			//gtk_widget_set_usize (  GTK_WIDGET( lookup_widget ( user_data, "frame1" ) ),  -1, 1 ) ; // imagelist verstecken ***
			gtk_widget_hide ( lookup_widget ( user_data, "frame1" ) ) ;
			gtk_widget_set_usize ( lookup_widget( user_data, "frame2" ), -1, lookup_widget ( user_data, "findimage" ) -> allocation.height ) ;
			
            for (c = 0; (c < n) ; c++)
            {
				char fullname[2048] ;
				
				gtk_progress_set_value (progress, ((gfloat) c) / (gfloat) (n - 1) * 100);
				
				refresh_screen ();
				
				strcpy (fullname, filename);
				strcat (fullname, namelist[c]->d_name);
				//
				// Was machen wir jetzt mit dem Eintrag??
				//
				if ( isimage(fullname) && strstr ( fullname, ".xcf" ) == NULL )
				{
					//GdkImlibImage *im;
					//GdkPixmap *imagelist_entry_pixmap;
					
					piccount++ ; 
					printd("Bild: "); printd(text_from_var(piccount));
					printd(" "); printd(fullname); printd("\n");
					//printd("Bild in Liste eintragen...\n");
					//printd("stat\n");
					if ( piccount == 1 &&
						gtk_notebook_get_current_page ( GTK_NOTEBOOK(lookup_widget(MainWindow,"imagedisplaynb")) ) != 1 )
					{
						//gtk_widget_set_usize (  GTK_WIDGET( lookup_widget ( user_data, "frame1" ) ),  -1, 220 ) ; // imagelist darstellen ***
						gtk_widget_set_usize ( lookup_widget( user_data, "frame2" ), -1, -1 ) ;
						gtk_widget_show ( lookup_widget ( user_data, "frame1" ) ) ;
					}
					stat ( fullname, &buf );
					printd("Bildgrösse: "); printd(text_from_var(buf.st_size)); printd("\n");
					imagelist_entry[0] = NULL;//image ;
					imagelist_entry[1] = fullname;
					imagelist_entry[2] = text_from_size(buf.st_size) ;
					imagelist_entry[3] = text_from_time(buf.st_mtime) ;
					gtk_clist_append ( imagelist, imagelist_entry);
					//printd("thumbnail eintragen\n");
					gtk_clist_set_pixmap ( imagelist, imagelist_row, 0, loading_picture , NULL );
					//printd("thumbnail eingetragen\n");
					//if (im) gdk_imlib_destroy_image (im);
					imagelist_row++;
					//printd("Bild in Liste eingetragen. \n");
				} else if ( isdir(fullname) ) {
					//printd("Verzeichnis in Liste eintragen...\n");
					dirlist_entry[0] = fullname;
					gtk_clist_append ( dirlist, dirlist_entry);
					printd("Verzeichnis: "); printd(dirlist_entry[0]); printd("\n");

				}
				// und jetzt wieder den Speicher freimachen.. :)
				free (namelist[c]);
			}
			//gtk_clist_thaw ( imagelist );
			//gtk_clist_thaw ( dirlist );
			printd("listen gefüllt.\n");
			gtk_progress_set_value (progress, 0);
			gnome_appbar_set_status (GNOME_APPBAR (appbar1), _("Erstelle Voransicht der Bilder..."));
			gtk_widget_set_sensitive ( dirdisplay, TRUE );
			refresh_screen ();
			
			gtk_progress_configure ( GTK_PROGRESS(lookup_widget ( MainWindow, "progressbar1" )),
						0, 0, imagelist_row ) ;
			gtk_progress_configure ( GTK_PROGRESS(lookup_widget ( FullscreenWindow, "progressbar1" )),
						0, 0, imagelist_row ) ;

			if ( thumbnails == TRUE && imagelist_row > 0 )
			{
				int max_row_height = no_thumb_size ;
				//GnomeIconList *gil = GNOME_ICON_LIST(lookup_widget(MainWindow, "iconlist")) ;
				int update_interval = 10 ; // how many thumbs should appear at once ( WARNING: may hang at some vars... 2? )
				
				//gnome_icon_list_freeze ( gil ) ;
				//gtk_widget_set_sensitive ( lookup_widget(MainWindow,"frame2"), FALSE ) ;
				
				printd("Es werden "); printd(text_from_var(imagelist_row));
				printd(" thumbnails geladen.\n");
				// nun die thumbnails eintragen!
				for ( c = 0; c < imagelist_row ; c++ )
				{
					char *filename ;
					gtk_progress_set_value (progress, ((gfloat) c) / (gfloat) imagelist_row * 100);
					if ( fmod( c, update_interval ) == 0 ) refresh_screen ();
					//printd("thumnail nummer = "); printd(c); printd("\n");
					gtk_clist_get_text ( imagelist, c, 1, &filename);
					//printd("thumb laden... "); printd(filename); printd("\n");
					printd("lade image\n");
					if ( gtk_clist_get_pixmap ( imagelist, c, 0, NULL, NULL ) != 0 ) 
					{
						GdkPixbuf *im;
						GdkPixmap *imagelist_entry_pixmap;
						GdkBitmap *imagelist_entry_mask;
						
						im = get_thumbnail(filename, FORCE_RELOAD);
						
						//refresh_screen () ;
						
						if ( im )
						{
							if ( gdk_pixbuf_get_height(im) > max_row_height )
							{
								max_row_height = MIN( gdk_pixbuf_get_height(im), max_thumb_size ) ;
								printd("neue thumb höhe = "); printd(text_from_var(max_row_height)); printd("\n");
								gtk_clist_set_row_height ( imagelist, max_row_height );
							}
							printd("imagelist_entry_pixmap erstellen\n");
							//imagelist_entry_pixmap = gdk_imlib_move_image(im);
							//imagelist_entry_mask = gdk_imlib_move_mask(im);
							gdk_pixbuf_render_pixmap_and_mask ( im, &imagelist_entry_pixmap, &imagelist_entry_mask, 255 ) ;
							if (imagelist_entry_pixmap) 
								printd("imagelist_entry_pixmap erstellt\n");
							else
								printd("!!! imagelist_entry_pixmap nicht erstellt\n");
							printd("thumbnail eintragen\n");
	
							gtk_clist_set_pixmap ( imagelist, c, 0, imagelist_entry_pixmap, imagelist_entry_mask );
							printd("thumbnail eingetragen\n");
	
							printd("Bild in Liste eingetragen. \n");
							//gtk_clist_set_selectable ( imagelist, c, TRUE ) ;
							printd("im freisetzen\n");
							if ( im ) gdk_pixbuf_unref (im);
							if ( imagelist_entry_pixmap ) gdk_pixmap_unref ( imagelist_entry_pixmap ) ;
							if ( imagelist_entry_mask ) gdk_bitmap_unref ( imagelist_entry_mask ) ;
						} else {
							printd("!!! thumbnail konnte nicht geladen werden!\n");
							gtk_clist_set_pixmap ( imagelist, c, 0, NULL, NULL );
							//gtk_clist_set_selectable ( imagelist, c, FALSE ) ;
						}
						if ( fmod( c, update_interval ) == 0 ) refresh_screen ();
					} else
						printd("!!! thumbnail schon geladen!\n");
				}
				printd("thumbs eingetragen.\n");
				gtk_clist_set_row_height ( imagelist, max_row_height );
			} else printd("keine thumbnails erwünscht.\n");
			printd("--- auslesen beendet.\n");
			gtk_clist_set_reorderable ( imagelist, TRUE ) ;
			gtk_clist_set_use_drag_icons ( imagelist, TRUE ) ;
			gdk_pixmap_unref ( loading_picture ) ;
			//gtk_widget_set_sensitive ( lookup_widget(MainWindow,"frame2"), TRUE ) ;
			refresh_screen () ;
			if ( imagelist_row > 0 ) 
				sprintf ( finishline, "%d Bilder", imagelist_row ) ;
			else
				sprintf ( finishline, "Keine Bilder" ) ;
		} else { 
			perror ("open_dir"); 
			sprintf ( finishline, "Keine Bilder" ) ;
		}

		free (namelist);
	} else {
		sprintf ( finishline, "Verzeichnis nicht gefunden!" ) ;
	}
	gtk_progress_set_value (progress, 0);
	gnome_appbar_set_status (GNOME_APPBAR (appbar1),
									 _( finishline ));
	gtk_widget_set_sensitive ( dirdisplay, TRUE );
}


