ACLOCAL_AMFLAGS = -I m4

SUBDIRS = m4 \
	libgoo \
	$(OMXDIR)

if BUILD_UNITTESTS
SUBDIRS += $(OMXTESTDIR)
endif

if ENABLE_GTK_DOC
SUBDIRS += docs
endif

DIST_SUBDIRS = \
	libgoo \
	libgoo-ti \
	libgoo-bellagio \
	m4 \
	test \
	test-bellagio \
	docs

CLEANFILES = \
	*~ 
	semantic.cache* \
	*.loT

DISTCHECK_CONFIGURE_FLAGS = \
	--enable-gtk-doc \
	--enable-unit-tests \
	--with-omx=bellagio
