## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = lats-2

lats_2_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	splash.c splash.h \
	mainwindow.c mainwindow.h \
	prefswindow.c prefswindow.h \
	fullscreenwindow.c fullscreenwindow.h \
	tasks.c tasks.h \
	dirlist.c dirlist.h \
	imageview.c imageview.h \
	text.c text.h

lats_2_LDADD = @PACKAGE_LIBS@

