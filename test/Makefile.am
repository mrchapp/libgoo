# TESTS = check_libgoo

check_PROGRAMS = \
	check-pcmenc \
	check-wbamrdec \
	check-nbamrdec \
	check-nbamrenc \
	check-gsmhrenc \
	check-wbamrenc \
	check-gsmfrenc \
	check-aacenc \
	check-mp3dec \
	check-wmadec \
	check-pcmdec \
	check-imaadpcmdec \
	check-gsmhrdec \
	check-g722dec \
	check-g711dec \
	check-aacdec \
	check-mpeg4dec \
	check-postproc \
	check-video-tunnel \
	check-mpeg4enc \
	check-jpegenc \
	check-h264dec \
	check-h263dec \
	check-wmvdec \
	check-h264enc \
	check-h263enc \
	check-jpegdec \
	check-vpp \
	check-clock \
	check-mpeg2dec \
	check-gsmfrdec

if WITH_TI_CAMERA
check_PROGRAMS += check-camera check-image-capture
endif

bin_PROGRAMS = $(check_PROGRAMS)

AM_CFLAGS = \
	$(GOBJECT_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(OMXIL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(CHECK_CFLAGS) \
	-I. -I$(top_builddir) \
	-I$(top_builddir)/libgoo \
	-I$(top_builddir)/libgoo-ti

LDADD = \
	$(GOBJECT_LIBS) \
	$(GTHREAD_LIBS) \
	$(OMXIL_LIBS) \
	$(GLIB_LIBS) \
	$(CHECK_LIBS) \
	$(top_builddir)/libgoo/libgoo.la \
	$(top_builddir)/libgoo-ti/libgoo-ti.la 
 
bin_SCRIPTS = check-all

check-all: check-all.in
	sed -e "s,@programs@,$(bin_PROGRAMS),; \
                s,@directory@,," < $^ > $@

EXTRA_DIST = \
	check-all.in

CLEANFILES = \
	*~ \
	semantic.cache* \
	*.loT
