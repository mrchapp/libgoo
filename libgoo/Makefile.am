lib_LTLIBRARIES = libgoo.la

libgoo_la_CFLAGS = \
	$(GOBJECT_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(OMXIL_CFLAGS) \
	$(GLIB_CFLAGS)

libgoo_la_LIBADD = \
	$(GOBJECT_LIBS) \
	$(GTHREAD_LIBS) \
	$(OMXIL_LIBS) \
	$(GLIB_LIBS)

libgoo_la_LDFLAGS = \
	-export-dynamic -version-info \
	$(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)

libgoo_la_sources = \
	goo-list.c \
	goo-iterator.c \
	goo-semaphore.c \
	goo-object.c \
	goo-port.c \
	goo-port-list.c \
	goo-port-iterator.c \
	goo-component.c \
	goo-component-list.c \
	goo-component-iterator.c \
	goo-component-factory.c \
	goo-engine.c \
	goo-log.c \
	goo-utils.c

libgoo_la_headers = \
	goo-list.h \
	goo-iterator.h \
	goo-semaphore.h \
	goo-object.h \
	goo-port.h \
	goo-port-list.h \
	goo-port-iterator.h \
	goo-component.h \
	goo-component-list.h \
	goo-component-iterator.h \
	goo-component-factory.h \
	goo-engine.h \
	goo-log.h \
	goo-utils.h \
	goo.h

libgoo_la_SOURCES = $(libgoo_la_sources)
libgoo_la_includedir = $(includedir)/libgoo-$(API_VERSION)
libgoo_la_include_HEADERS = $(libgoo_la_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgoo-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

CLEANFILES = \
	*~ \
	semantic.cache* \
	*.loT

DISTCLEANFILES = \
	$(pkgconfig_DATA)