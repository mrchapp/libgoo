check_PROGRAMS = \
	check-mp3dec

bin_PROGRAMS = $(check_PROGRAMS)

AM_CFLAGS = \
	$(GOBJECT_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(OMXIL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(CHECK_CFLAGS) \
	-I. -I$(top_builddir) \
	-I$(top_builddir)/libgoo \
	-I$(top_builddir)/libgoo-bellagio

LDADD = \
	$(GOBJECT_LIBS) \
	$(GTHREAD_LIBS) \
	$(OMXIL_LIBS) \
	$(GLIB_LIBS) \
	$(CHECK_LIBS) \
	$(top_builddir)/libgoo/libgoo.la \
	$(top_builddir)/libgoo-bellagio/libgoo-bellagio.la 
 
CLEANFILES = \
	*~ \
	semantic.cache* \
	*.loT
