## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR)\
	 -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/include/gdk-pixbuf-1.0 -I/usr/include/gnome-vfs-1.0\
	 -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses -Wpointer-arith -Wmissing-prototypes\
	 -g

bin_PROGRAMS = look_at_the_stars

look_at_the_stars_SOURCES = \
	callbacks.c\
	callbacks.h\
	interface.c\
	interface.h\
	main.c\
	support.c\
	support.h\
	importedfuncs.c\
	importedfuncs.h\
	imageview.c\
	imageview.h\
	toolbars.c\
	toolbars.h\
	colors.c\
	colors.h\
	imagelist.h\
	imagelist.c\
	pixbuf_util.h\
	pixbuf_util.c\
	tasks.h\
	tasks.c

look_at_the_stars_LDADD =  \
	$(INTLLIBS) \
	$(GNOME_LIBDIR) $(GNOMEUI_LIBS)\
	 -lgdk_pixbuf -lgnomevfs -lpng

