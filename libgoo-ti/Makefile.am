lib_LTLIBRARIES = libgoo-ti.la

libgoo_ti_la_CFLAGS = \
	-I. -I$(top_srcdir) \
	-I$(top_srcdir)/libgoo \
	$(GOBJECT_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(OMXIL_CFLAGS) \
	$(GLIB_CFLAGS)

libgoo_ti_la_LIBADD = \
	$(GOBJECT_LIBS) \
	$(GTHREAD_LIBS) \
	$(OMXIL_LIBS) \
	$(GLIB_LIBS) \
	$(top_builddir)/libgoo/libgoo.la \
	-lm

libgoo_ti_la_LDFLAGS = \
	-export-dynamic -version-info \
	$(LT_VERSION_INFO) $(LIBTOOL_EXPORT_OPTIONS)

libgoo_ti_la_sources = \
	goo-ti-component-factory.c \
	goo-ti-audio-component.c \
	goo-ti-audio-encoder.c \
	goo-ti-audio-decoder.c \
	goo-ti-audio-manager.c \
	goo-ti-wbamrdec.c \
	goo-ti-nbamrdec.c \
	goo-ti-pcmenc.c \
	goo-ti-nbamrenc.c \
	goo-ti-gsmhrenc.c \
	goo-ti-wbamrenc.c \
	goo-ti-gsmfrenc.c \
	goo-ti-aacenc.c \
	goo-ti-armaacenc.c \
	goo-ti-mp3dec.c \
	goo-ti-wmadec.c \
	goo-ti-pcmdec.c \
	goo-ti-imaadpcmdec.c \
	goo-ti-gsmhrdec.c \
	goo-ti-g722dec.c \
	goo-ti-g711dec.c \
	goo-ti-aacdec.c \
	goo-ti-armaacdec.c \
	goo-ti-video-decoder.c \
	goo-ti-video-decoder720p.c \
	goo-ti-mpeg4dec.c \
	goo-ti-sparkdec.c \
	goo-ti-video-encoder.c \
	goo-ti-mpeg4enc.c \
	goo-ti-post-processor.c \
	goo-ti-jpegenc.c \
	goo-ti-clock.c \
	goo-ti-h264dec.c \
	goo-ti-h264dec-720p.c \
	goo-ti-h263dec.c \
	goo-ti-wmvdec.c \
	goo-ti-h264enc.c \
	goo-ti-h263enc.c \
	goo-ti-jpegdec.c \
	goo-ti-vpp.c \
	goo-ti-mpeg2dec.c \
	goo-ti-gsmfrdec.c


libgoo_ti_la_headers = \
	goo-ti-component-factory.h \
	goo-ti-audio-component.h \
	goo-ti-audio-encoder.h \
	goo-ti-audio-decoder.h \
	goo-ti-audio-manager.h \
	goo-ti-wbamrdec.h \
	goo-ti-nbamrdec.h \
	goo-ti-pcmenc.h \
	goo-ti-nbamrenc.h \
	goo-ti-gsmhrenc.h \
	goo-ti-wbamrenc.h \
	goo-ti-gsmfrenc.h \
	goo-ti-aacenc.h \
	goo-ti-armaacenc.h \
	goo-ti-mp3dec.h \
	goo-ti-wmadec.h \
	goo-ti-pcmdec.h \
	goo-ti-imaadpcmdec.h \
	goo-ti-gsmhrdec.h \
	goo-ti-g722dec.h \
	goo-ti-g711dec.h \
	goo-ti-aacdec.h \
	goo-ti-armaacdec.h \
	goo-ti-video-decoder.h \
	goo-ti-video-decoder720p.h \
	goo-ti-mpeg4dec.h \
	goo-ti-video-encoder.h \
	goo-ti-mpeg4enc.h \
	goo-ti-sparkdec.h \
	goo-ti-post-processor.h \
	goo-ti-jpegenc.h \
	goo-ti-clock.h \
	goo-ti-h264dec.h \
	goo-ti-h264dec-720p.h \
	goo-ti-h263dec.h \
	goo-ti-wmvdec.h  \
	goo-ti-h264enc.h \
	goo-ti-h263enc.h \
	goo-ti-jpegdec.h \
	goo-ti-vpp.h \
	goo-ti-mpeg2dec.h \
	goo-ti-gsmfrdec.h

if WITH_TI_CAMERA
libgoo_ti_la_sources +=	goo-ti-camera.c
libgoo_ti_la_headers +=	goo-ti-camera.h
endif

libgoo_ti_la_SOURCES = $(libgoo_ti_la_sources)
libgoo_ti_la_includedir = $(includedir)/libgoo-ti-$(API_VERSION)
libgoo_ti_la_include_HEADERS = $(libgoo_ti_la_headers)

%-$(API_VERSION).pc: %.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgoo-ti-$(API_VERSION).pc
EXTRA_DIST = $(pkgconfig_DATA:-$(API_VERSION).pc=.pc.in)

CLEANFILES = \
	*~ \
	semantic.cache* \
	*.loT

DISTCLEANFILES = \
	$(pkgconfig_DATA)
