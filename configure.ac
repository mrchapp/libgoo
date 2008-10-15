# Making releases:
#   goo_micro_version += 1;
#   goo_interface_age += 1;
#   goo_binary_age += 1;
# if any functions have been added, set goo_interface_age to 0.
# if backwards compatibility has been broken,
# set goo_binary_age and goo_interface_age to 0.

m4_define([goo_major_version], [0])
m4_define([goo_minor_version], [1])
m4_define([goo_micro_version], [0])
m4_define([goo_version],
          [goo_major_version.goo_minor_version.goo_micro_version])
m4_define([goo_interface_age], [0])
m4_define([goo_binary_age], [0])
m4_define([goo_api_version], [1.0])
m4_define([lt_current], [m4_eval(goo_micro_version - goo_interface_age)])
m4_define([lt_revision], [goo_interface_age])
m4_define([lt_age], [m4_eval(goo_binary_age - goo_interface_age)])

AC_INIT([goo], [goo_version], [http://www.ti.com])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([dist-bzip2 foreign])
AM_CONFIG_HEADER([config.h])

LT_VERSION_INFO="lt_current:lt_revision:lt_age"
LT_CURRENT_MINUS_AGE=m4_eval(lt_current - lt_age)
AC_SUBST(LT_VERSION_INFO)
AC_SUBST(LT_CURRENT_MINUS_AGE)

dnl ### Don't export functions that start with _ ##
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

API_VERSION=goo_api_version
AC_SUBST(API_VERSION)        
                
dnl versions of GLib
GOBJECT_MAJORMINOR=2.0
GOBJECT_REQUIRED=2.0.0

dnl versions of OpenMAX
OMXIL_MAJORMINOR=1.0
OMXIL_REQUIRED=1.0.0

dnl AM_MAINTAINER_MODE provides the option to enable maintainer mode
AM_MAINTAINER_MODE

dnl check for tools
AC_PROG_CC
if test "$GCC" = "yes"; then
   CFLAGS="$CFLAGS -Wall -g"
   AC_SUBST(CFLAGS)
fi
        
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl ### default values (will get overwritten)
build_unittests=false
build_outputqueue=false
build_ti_clock=false
build_ti_camera=true
OMXIMPL=1
OMXDIR=libgoo-ti
OMXTESTDIR=test

dnl ### Select the OMX implementation library
AC_ARG_WITH(omx,
	AC_HELP_STRING([--with-omx=ti],
		[OpenMAX implementation to use (ti or bellagio)]),
	[case "${with_omx}" in
	ti)
		OMXDIR=libgoo-ti
		OMXTESTDIR=test
		build_ti_clock=false
		build_ti_camera=true
		OMXIMPL=1
		;;
	bellagio)
		OMXDIR=libgoo-bellagio
		OMXTESTDIR=test-bellagio
		build_ti_clock=false
		build_ti_camera=false
		OMXIMPL=2
		;;
	*)
		OMXDIR=${with_omx}
		build_ti_clock=false
		build_ti_camera=false
		OMXIMPL=0
	esac], [OMXDIR=libgoo-ti
	        with_omx=ti])
AC_SUBST(OMXDIR)
AC_SUBST(OMXIMPL)
AC_SUBST(OMXTESTDIR)

dnl ### Enable buildiing the unit tests ###
AC_ARG_ENABLE(unit-tests,
        AC_HELP_STRING([--enable-unit-tests], [Build unit test (no, yes)]),
        [case "${enableval}" in
              yes) enable_unittests=true ;;
              no)  enable_unittests=false ;;
              *) AC_MSG_ERROR(bad value ${enableval} for --enable-unit-tests)
                ;;
        esac], [enable_unittests=$build_unittests])
AM_CONDITIONAL([BUILD_UNITTESTS], test x$enable_unittests = xtrue)

dnl ## Enable Output queue    
AC_ARG_ENABLE(output-queue,
        AC_HELP_STRING([--enable-output-queue], [Create another thread for output buffers processing.  Lower performance (no, yes)]),
        [case "${enableval}" in
              yes) enable_output_queue=true ;;
              no)  enable_output_queue=false ;;
         esac], [enable_output_queue=$build_outputqueue])
        
if test x$enable_output_queue = xtrue; then
   AC_DEFINE([OUTPUT_QUEUE], [1], [Define to enable the creation of the output queue processing thread])
fi

dnl ## Enable Clock
AC_ARG_ENABLE(ti-clock,
        AC_HELP_STRING([--enable-ti-clock], [Add support for OMX TI clock (no, yes)]),
        [case "${enableval}" in
              yes) enable_ti_clock=true ;;
              no)  enable_ti_clock=false ;;
              *) AC_MSG_ERROR(bad value ${enableval} for --enable-ti-clock) ;;
         esac],
        [enable_ti_clock=$build_ti_clock])

if test x$enable_ti_clock = xtrue; then
   AC_DEFINE([TI_CLOCK], [1], [Define to enable the support for OMX TI clock])
fi

dnl ## Enable Camera
AC_ARG_ENABLE(ti-camera,
        AC_HELP_STRING([--enable-ti-camera], [Add support for OMX TI camera (no, yes)]),
        [case "${enableval}" in
              yes) enable_ti_camera=true ;;
              no)  enable_ti_camera=false ;;
              *) AC_MSG_ERROR(bad value ${enableval} for --enable-ti-camera) ;;
         esac],
        [enable_ti_camera=$build_ti_camera])

if test x$enable_ti_camera = xtrue; then
   AC_DEFINE([TI_CAMERA], [1], [Define to enable the support for OMX TI camera])
fi

dnl ## Enable TI implementation

AM_CONDITIONAL([WITH_TI_CAMERA], test x$enable_ti_camera = xtrue)
                
dnl Check for gtk-doc
GTK_DOC_CHECK([1.4])

dnl Check for GObject
PKG_CHECK_MODULES([GOBJECT], [gobject-$GOBJECT_MAJORMINOR >= $GOBJECT_REQUIRED])

dnl Check for GLib
PKG_CHECK_MODULES([GLIB], [glib-$GOBJECT_MAJORMINOR >= $GOBJECT_REQUIRED])
                          
dnl Check for GThread
PKG_CHECK_MODULES([GTHREAD], [gthread-$GOBJECT_MAJORMINOR >= $GOBJECT_REQUIRED])

dnl Check for OpenMAX
PKG_CHECK_MODULES([OMXIL], [libomxil])

dnl Check for check :)
if test x$enable_unittests = xtrue; then    
   PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi

AC_OUTPUT([
Makefile
libgoo/Makefile
libgoo/libgoo.pc
libgoo-ti/Makefile
libgoo-ti/libgoo-ti.pc
libgoo-bellagio/Makefile
libgoo-bellagio/libgoo-bellagio.pc
test/Makefile
test-bellagio/Makefile
docs/Makefile
m4/Makefile
])

echo ""
echo "The following options are selected:"
echo ""
echo "   Unit tests:         $enable_unittests"
echo "   Output queue:       $enable_output_queue"
echo "   OMX implementation: ${with_omx}"
echo "   TI Clock:           $enable_ti_clock"
echo "   TI Camera:          $enable_ti_camera"
echo ""
echo "Version info:"
echo ""
echo "   Version:      $VERSION"
echo "   API version:  $API_VERSION"
echo "   ABI version:  $LT_VERSION_INFO - $LT_CURRENT_MINUS_AGE"
echo ""
