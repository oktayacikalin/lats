## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = po macros src pixmaps

look_at_the_starsdocdir = ${prefix}/doc/look_at_the_stars
look_at_the_starsdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS\
	TODO\
	ABOUT-NLS

EXTRA_DIST = Look_at_the_stars.prj $(look_at_the_starsdoc_DATA)

gnomemenudir = $(prefix)/@NO_PREFIX_PACKAGE_MENU_DIR@/Graphics
gnomemenu_DATA = look_at_the_stars.desktop

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done


ACLOCAL_AMFLAGS = -I m4
