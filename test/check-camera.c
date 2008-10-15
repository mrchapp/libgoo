/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Copyright (C) 2008 Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation
 * version 2.1 of the License.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-ti-camera.h>
#include <goo-ti-post-processor.h>

#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>
#include <time.h>

GooComponentFactory* factory;
GooComponent* camera;
GooComponent* preview;
GRand* rnd;

const gchar* titlefmt = "\n\t*** %s ***\n";
#define T(title) g_print (titlefmt, title)

void
setup (void)
{
	factory = goo_ti_component_factory_get_instance ();
	camera =
		goo_component_factory_get_component (factory, GOO_TI_CAMERA);
	preview =
		goo_component_factory_get_component (factory,
						     GOO_TI_POST_PROCESSOR);

	rnd = g_rand_new_with_seed (time (0));

	return;
}

void
teardown (void)
{
	g_object_unref (camera);
	g_object_unref (preview);
	g_object_unref (factory);
	g_rand_free (rnd);

	return;
}

void
capture (gchar* filename, gchar* resolution, gboolean oneshot,
	 guint numbuffers, guint colorformat, guint framerate,
	 OMX_WHITEBALCONTROLTYPE balance, GooTiCameraZoom zoom)
{
	g_assert (filename != NULL);
	g_assert (resolution != NULL);

	ResolutionInfo rinfo = goo_get_resolution (resolution);
	ResolutionInfo vfres = goo_get_resolution ("pal");

	g_assert (rinfo.width != 0 && rinfo.height != 0);

	/* camera sensor */
	{
		OMX_PARAM_SENSORMODETYPE* sensor = NULL;
		sensor = GOO_TI_CAMERA_GET_PARAM (camera);

		sensor->bOneShot  = oneshot;
		sensor->nFrameRate = (oneshot) ? 0 : framerate;
		sensor->sFrameSize.nWidth  = rinfo.width;
		sensor->sFrameSize.nHeight = rinfo.height;
	}

	/* postroc input port */
	{
		GooPort* port = goo_component_get_port (preview, "input0");
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		gint width = MIN (vfres.width, rinfo.width);
		gint height = MIN (vfres.height, rinfo.height);

		param->format.video.nFrameWidth  = width;
		param->format.video.nFrameHeight = height;
		param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

		g_object_unref (port);
	}

	/* viewfinding port configuration */
	{
		GooPort* port = goo_component_get_port (camera, "output0");
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		gint width = MIN (vfres.width, rinfo.width);
		gint height = MIN (vfres.height, rinfo.height);

		param->format.video.nFrameWidth  = width;
		param->format.video.nFrameHeight = height;
		param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

		/* goo_component_disable_port (camera, port); */

		g_object_unref (port);
	}

	/* capture port configuration */
	{
		GooPort* port = goo_component_get_port (camera, "output1");
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		if (oneshot)
		{
			param->format.image.eColorFormat = colorformat;
		}
		else
		{
			param->format.video.eColorFormat = colorformat;
		}

		g_object_unref (port);
	}

	/* thumbnail port configuration */
	{
		GooPort* port = goo_component_get_port (camera, "output2");
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		gint width = MIN (vfres.width, rinfo.width);
		gint height = MIN (vfres.height, rinfo.height);

		param->format.video.nFrameWidth  = width;
		param->format.video.nFrameHeight = height;
		param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

		/* goo_component_disable_port (camera, port); */

		g_object_unref (port);
	}

	goo_component_set_tunnel_by_name (camera, "output0",
					  preview, "input0",
					  OMX_BufferSupplyInput);


	/* the engine must know about the tunnel */
	GooEngine* engine = goo_engine_new (
		camera,
		NULL,
		filename
		);

	g_object_set (engine, "num-buffers", numbuffers, NULL);

	goo_component_set_state_idle (camera);

	/* camera custom config params */
	{
		g_object_set (camera,
			      "vstab", TRUE,
			      "brightness", g_rand_int_range (rnd, 0, 100),
			      "contrast", g_rand_int_range (rnd, -100, 100),
			      NULL);

		if (balance != OMX_WhiteBalControlAuto)
		{
			g_object_set (camera, "balance", balance, NULL);
		}

		if (zoom != GOO_TI_CAMERA_ZOOM_1X)
		{
			g_object_set (camera, "zoom", zoom, NULL);
		}
	}


	/* postproc custom params */
	{
		guint rotation = (rinfo.width >= 320 || rinfo.height >= 240) ?
			GOO_TI_POST_PROCESSOR_ROTATION_90 :
			GOO_TI_POST_PROCESSOR_ROTATION_NONE;

		g_object_set (preview,
			      "rotation", rotation,
			      "x-scale", 100, "y-scale", 100,
			      "x-pos", 0, "y-pos", 0,
			      "mirror", FALSE, NULL);
	}

	goo_component_set_state_executing (camera);

	g_object_set (camera, "capture", TRUE, NULL);
	goo_engine_play (engine);
	g_object_set (camera, "capture", FALSE, NULL);

	goo_component_set_state_idle (camera);
	goo_component_set_state_loaded (camera);

	g_object_unref (G_OBJECT (engine));

	return;
}

/* ================================================================ */
/* == Still image capture                                           */
/* ================================================================ */

START_TEST (BF50_1)
{
	T ("CAMERA SHOT QCIF");
	capture ("/tmp/camera_shot_qcif_yuy2.yuv", "qcif", TRUE, 10,
		 OMX_COLOR_FormatYCbYCr, 0,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF50_2)
{
	T ("CAMERA SHOT SVGA");
	capture ("/tmp/camera_shot_svga_uyvy.yuv", "svga", TRUE, 5,
		 OMX_COLOR_FormatCbYCrY, 0,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF50_3)
{
	T ("CAMERA SHOT XVGA");
	capture ("/tmp/camera_shot_xvga_yuy2.yuv", "xvga", TRUE, 3,
		 OMX_COLOR_FormatYCbYCr, 0,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF51)
{
	T ("CAMERA SHOT SXGA");
	capture ("/tmp/camera_shot_sxvga_uyvy.yuv", "sxvga", TRUE, 2,
		 OMX_COLOR_FormatCbYCrY, 0,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF52)
{
	T ("CAMERA SHOT UXGA");
	capture ("/tmp/camera_shot_uxvga_yuy2.yuv", "uxvga", TRUE, 1,
		 OMX_COLOR_FormatYCbYCr, 0,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF53)
{
	T ("CAMERA SHOT QXGA");
	capture ("/tmp/camera_shot_uxvga_yuy2.yuv", "qxga", TRUE, 1,
		 OMX_COLOR_FormatYCbYCr, 0,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF54)
{
	T ("CAMERA SHOT WQXGA");
	capture ("/tmp/camera_shot_uxvga_yuy2.yuv", "wqxga", TRUE, 1,
		 OMX_COLOR_FormatYCbYCr, 0,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF55)
{
	T ("CAMERA SHOT QSXGA");
	capture ("/tmp/camera_shot_uxvga_yuy2.yuv", "qsxga", TRUE, 1,
		 OMX_COLOR_FormatYCbYCr, 0,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST


/* ================================================================ */
/* == Video capture with viewfinder & preview                       */
/* ================================================================ */

START_TEST (BF1)
{
	T ("CAMERA VIDEO QCIF YCbYCr");
	capture ("/dev/null", "qcif", FALSE, 100, OMX_COLOR_FormatYCbYCr, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF2)
{
	T ("CAMERA VIDEO QVGA YCbYCr");
	capture ("/dev/null", "qvga", FALSE, 30, OMX_COLOR_FormatYCbYCr, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF3)
{
	T ("CAMERA VIDEO CIF YCbYCr");
	capture ("/dev/null", "cif", FALSE, 50, OMX_COLOR_FormatYCbYCr, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF4)
{
	T ("CAMERA VIDEO VGA YCbYCr");
	capture ("/dev/null", "vga", FALSE, 20, OMX_COLOR_FormatYCbYCr, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF5)
{
	T ("CAMERA VIDEO D1 NTSC YCbYCr");
	capture ("/dev/null", "d1ntsc", FALSE, 20, OMX_COLOR_FormatYCbYCr, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF6)
{
	T ("CAMERA VIDEO PAL YCbYCr");
	capture ("/dev/null", "pal", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF7)
{
	T ("CAMERA VIDEO QCIF CbYCrY");
	capture ("/dev/null", "qcif", FALSE, 100, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF8)
{
	T ("CAMERA VIDEO QVGA CbYCrY");
	capture ("/dev/null", "qvga", FALSE, 30, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF9)
{
	T ("CAMERA VIDEO CIF CbYCrY");
	capture ("/dev/null", "cif", FALSE, 50, OMX_COLOR_FormatCbYCrY, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF10)
{
	T ("CAMERA VIDEO VGA CbYCrY");
	capture ("/dev/null", "vga", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF11)
{
	T ("CAMERA VIDEO D1 NTSC CbYCrY");
	capture ("/dev/null", "d1ntsc", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF12)
{
	T ("CAMERA VIDEO PAL CbYCrY");
	capture ("/dev/null", "pal", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

/* ================================================================ */
/* == Video capture with digital zoom                               */
/* ================================================================ */

START_TEST (BF19)
{
	T ("CAMERA VIDEO CIF 2X");
	capture ("/dev/null", "cif", FALSE, 50, OMX_COLOR_FormatCbYCrY, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_2X);

	return;
}
END_TEST

START_TEST (BF20)
{
	T ("CAMERA VIDEO CIF 3X");
	capture ("/dev/null", "cif", FALSE, 50, OMX_COLOR_FormatCbYCrY, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_3X);

	return;
}
END_TEST

START_TEST (BF21)
{
	T ("CAMERA VIDEO CIF 4X");
	capture ("/dev/null", "cif", FALSE, 50, OMX_COLOR_FormatCbYCrY, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_4X);

	return;
}
END_TEST

/* ================================================================ */
/* == Video capture with white balance                              */
/* ================================================================ */

START_TEST (BF56)
{
	T ("CAMERA VIDEO VGA DAY LIGHT");
	capture ("/dev/null", "vga", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlSunLight, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF58)
{
	T ("CAMERA VIDEO VGA INCANDESCENT");
	capture ("/dev/null", "vga", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlIncandescent, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF60)
{
	T ("CAMERA VIDEO VGA FLUORESCENT");
	capture ("/dev/null", "vga", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlFluorescent, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF62)
{
	T ("CAMERA VIDEO VGA HORIZON");
	capture ("/dev/null", "vga", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlHorizon, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF64)
{
	T ("CAMERA VIDEO VGA HORIZON");
	capture ("/dev/null", "vga", FALSE, 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlShade, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_camera)
{
	tcase_add_test (tc_camera, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo Camera Test");
	TCase *tc_camera = tcase_create ("Camera");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "BF50_1", BF50_1);
	g_hash_table_insert (ht, "BF50_2", BF50_2);
	g_hash_table_insert (ht, "BF50_3", BF50_3);
	g_hash_table_insert (ht, "BF51", BF51);
	g_hash_table_insert (ht, "BF52", BF52);
	g_hash_table_insert (ht, "BF53", BF53);
	g_hash_table_insert (ht, "BF54", BF54);
	g_hash_table_insert (ht, "BF55", BF55);  /* kernel oops! */
	g_hash_table_insert (ht, "BF1", BF1);
	g_hash_table_insert (ht, "BF2", BF2);
	g_hash_table_insert (ht, "BF3", BF3);
	g_hash_table_insert (ht, "BF4", BF4);
	g_hash_table_insert (ht, "BF5", BF5);
	g_hash_table_insert (ht, "BF6", BF6);
	g_hash_table_insert (ht, "BF7", BF7);
	g_hash_table_insert (ht, "BF8", BF8);
	g_hash_table_insert (ht, "BF9", BF9);
	g_hash_table_insert (ht, "BF10", BF10);
	g_hash_table_insert (ht, "BF11", BF11);
	g_hash_table_insert (ht, "BF12", BF12);
	g_hash_table_insert (ht, "BF19", BF19);
	g_hash_table_insert (ht, "BF20", BF20);
	g_hash_table_insert (ht, "BF21", BF21);
	g_hash_table_insert (ht, "BF56", BF56);
	g_hash_table_insert (ht, "BF58", BF58);
	g_hash_table_insert (ht, "BF60", BF60);
	g_hash_table_insert (ht, "BF62", BF62);
	g_hash_table_insert (ht, "BF64", BF64);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_camera);
	}
	else
	{
		tcase_add_test (tc_camera, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_camera, setup, teardown);
	tcase_set_timeout (tc_camera, 0);
	suite_add_tcase (s, tc_camera);

	return s;
}

static gchar*
parse_options (int *argc, char **argv[])
{
	GOptionContext* ctx;
	GError *error = NULL;
	gchar* testopt = "all";
	GOptionEntry options[] = {
		{ "test", 't', 0, G_OPTION_ARG_STRING, &testopt,
		  "Test option: ("
		  "BF50_1/BF50_2/BF50_3/BF51/BF52/BF53/BF54/BF55/"
		  "BF1/BF2/BF3/BF4/BF5/BF6/BF7/BF8/BF9/BF10/BF11/BF12"
		  "BF19/BF20/BF21/BF56/BF58/BF60/BF62/BF64)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- Camera Tests.");
	g_option_context_add_main_entries (ctx, options, NULL);

	if (!g_option_context_parse (ctx, argc, argv, &error))
	{
		g_print ("Failed to initialize: %s\n", error->message);
		g_error_free (error);
		return NULL;
	}

	g_option_context_free (ctx);

	return testopt;
}

gint
main (int argc, char *argv[])
{
	int number_failed;

	g_type_init ();
	if (!g_thread_supported ())
	{
		g_thread_init (NULL);
	}

	gchar* srd = parse_options (&argc, &argv);

	if (srd == NULL)
	{
		return EXIT_FAILURE;
	}

	Suite *s = goo_suite (srd);
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
