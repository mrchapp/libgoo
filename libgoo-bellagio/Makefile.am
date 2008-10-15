lib_LTLIBRARIES = libgoo-bellagio.la

libgoo_bellagio_la_CFLAGS = \
	-I. -I$(top_srcdir) \
	-I$(top_srcdir)/libgoo \
	$(GOBJECT_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(OMXIL_CFLAGS) \
	$(GLIB_CFLAGS)

libgoo_bellagio_la_LIBADD = \
	$(GOBJECT_LIBS) \
	$(GTHREAD_LIBS) \
	$(OMXIL_LIBS) \
	$(GLIB_LIBS) \
	$(top_builddir)/libgoo/libgoo.la

libgoo_bellagio_la_LDFLAGS = \
	-export-dynamic -version-info \
	$(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)

libgoo_bellagio_la_sources = \
	goo-bellagio-component-factory.c \
	goo-bellagio-mp3dec.c 

libgoo_bellagio_la_headers = \
	goo-bellagio-component-factory.h \
	goo-bellagio-mp3dec.h

libgoo_bellagio_la_SOURCES = $(libgoo_bellagio_la_sources)
libgoo_bellagio_la_includedir = $(includedir)/libgoo-bellagio-$(API_VERSION)
libgoo_bellagio_la_include_HEADERS = $(libgoo_bellagio_la_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgoo-bellagio-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

CLEANFILES = \
	*~ \
	semantic.cache* \
	*.loT

DISTCLEANFILES = \
	$(pkgconfig_DATA)
