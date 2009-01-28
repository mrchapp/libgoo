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
GooComponent* venc;
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
capture (gchar* filename, const gchar* resolution, 
	 guint numbuffers, guint colorformat, guint framerate,
	 OMX_WHITEBALCONTROLTYPE balance, GooTiCameraZoom zoom)
{
	g_assert (filename != NULL);
	g_assert (resolution != NULL);

	ResolutionInfo rinfo = goo_get_resolution (resolution);

	g_assert (rinfo.width != 0 && rinfo.height != 0);

	/* camera sensor */
	{
		OMX_PARAM_SENSORMODETYPE* sensor = NULL;
		sensor = GOO_TI_CAMERA_GET_PARAM (camera);

		sensor->bOneShot = OMX_FALSE;
		sensor->nFrameRate = framerate;
		sensor->sFrameSize.nWidth  = rinfo.width;
		sensor->sFrameSize.nHeight = rinfo.height;
	}

	/* postroc input port */
	{
		GooPort* port = goo_component_get_port (preview, "input0");
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		param->format.video.nFrameWidth  = rinfo.width;
		param->format.video.nFrameHeight = rinfo.height;
		param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

		g_object_unref (port);
	}

	/* viewfinding port configuration */
	{
		GooPort* port = goo_component_get_port (camera, "output0");
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		param->format.video.nFrameWidth  = rinfo.width;
		param->format.video.nFrameHeight = rinfo.height;
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

		param->format.video.eColorFormat = colorformat;

		g_object_unref (port);
	}

	/* thumbnail port configuration */
	{
		GooPort* port = goo_component_get_port (camera, "output2");
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		param->format.video.nFrameWidth  = rinfo.width;
		param->format.video.nFrameHeight = rinfo.height;
		param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

		goo_component_disable_port (camera, port);

		g_object_unref (port);
	}
						     
	/* video encoder port configuration */
	{
		/* video encoder instanciation */
		/* we cannot do it in the setup because we should choose in run 
		 * time which encoder we want */
		venc = goo_component_factory_get_component (factory,
						     GOO_TI_H264_ENCODER);
						    
		gint bitrate = 4000000;
		gint level = 128;

		/* video properties */
		g_object_set (G_OBJECT (venc),
			"level", level,
			"control-rate", GOO_TI_VIDEO_ENCODER_CR_VARIABLE,
			NULL);

		/* input port */
		{
			GooPort* port = goo_component_get_port (venc, "input0");
			g_assert (port != NULL);
			OMX_PARAM_PORTDEFINITIONTYPE* param =
				GOO_PORT_GET_DEFINITION (port);

			param->nBufferCountActual = 4;
			param->format.video.nFrameWidth = rinfo.width;
			param->format.video.nFrameHeight = rinfo.height;
			param->format.video.eColorFormat = colorformat;
			param->format.video.xFramerate = framerate;

			g_object_unref (port);
		}

		/* output port */
		{
			GooPort* port = goo_component_get_port (venc, "output0");
			g_assert (port != NULL);
			OMX_PARAM_PORTDEFINITIONTYPE* param =
				GOO_PORT_GET_DEFINITION (port);

			param->nBufferCountActual = 4;
			param->format.video.nFrameWidth = rinfo.width;
			param->format.video.nFrameHeight = rinfo.height;
			param->format.video.nBitrate = bitrate;
			param->format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;

			g_object_unref (port);
		}
	}
	
	goo_component_set_tunnel_by_name (camera, "output1",
					  venc, "input0",
					  OMX_BufferSupplyInput);

	/* the engine must know about the tunnel */
	GooEngine* engine = goo_engine_new (
		venc,
		NULL,
		filename
		);

	g_object_set (engine, "num-buffers", numbuffers, NULL);

	goo_component_set_tunnel_by_name (camera, "output0",
					  preview, "input0",
					  OMX_BufferSupplyInput);

	{
		GooPort* port = goo_component_get_port (preview, "input0");
		g_assert (port != NULL);
		goo_component_set_supplier_port (preview, port,
                     OMX_BufferSupplyInput);
        g_object_unref (port);
	}
	
	
	goo_component_set_state_idle (camera);

	/* camera custom config params */
	{
		g_print("\n **************************************entramos a las configuraciones : vstab no esta activado **********************\n");
		g_object_set (camera,
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

	/* venc custom params */
	{
		g_object_set (venc, "frame-interval", 30, NULL);
	}

	g_object_set (camera, "capture", TRUE, NULL);
	goo_engine_play (engine);
	g_object_set (camera, "capture", FALSE, NULL);

	goo_component_set_state_idle (camera);

	goo_component_set_state_loaded (camera);
	goo_component_set_state_loaded (venc);

	g_object_unref (G_OBJECT (engine));
	g_object_unref(venc);
	return;
}

START_TEST (BF1)
{
	T ("CAMERA VIDEO QCIF YCbYCr");
	capture ("/dev/null", "qcif", 100, OMX_COLOR_FormatYCbYCr, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF2)
{
	T ("CAMERA VIDEO QVGA YCbYCr");
	capture ("/dev/null", "qvga", 30, OMX_COLOR_FormatYCbYCr, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF3)
{
	T ("CAMERA VIDEO CIF YCbYCr");
	capture ("/dev/null", "cif", 50, OMX_COLOR_FormatYCbYCr, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF4)
{
	T ("CAMERA VIDEO VGA YCbYCr");
	capture ("/dev/null", "vga", 20, OMX_COLOR_FormatYCbYCr, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF5)
{
	T ("CAMERA VIDEO D1 NTSC YCbYCr");
	capture ("/dev/null", "d1ntsc", 20, OMX_COLOR_FormatYCbYCr, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF6)
{
	T ("CAMERA VIDEO PAL YCbYCr");
	capture ("video_test6.h264", "pal", 100, OMX_COLOR_FormatCbYCrY, 25,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF7)
{
	T ("CAMERA VIDEO QCIF CbYCrY");
	capture ("/dev/null", "qcif", 100, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF8)
{
	T ("CAMERA VIDEO QVGA CbYCrY");
	capture ("/dev/null", "qvga", 30, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF9)
{
	T ("CAMERA VIDEO CIF CbYCrY");
	capture ("/dev/null", "cif", 50, OMX_COLOR_FormatCbYCrY, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF10)
{
	T ("CAMERA VIDEO VGA CbYCrY");
	capture ("/dev/null", "vga", 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF11)
{
	T ("CAMERA VIDEO D1 NTSC CbYCrY");
	capture ("/dev/null", "d1ntsc", 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF12)
{
	T ("CAMERA VIDEO PAL CbYCrY");
	capture ("/dev/null", "pal", 20, OMX_COLOR_FormatCbYCrY, 30,
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
	capture ("/dev/null", "cif", 50, OMX_COLOR_FormatCbYCrY, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_2X);

	return;
}
END_TEST

START_TEST (BF20)
{
	T ("CAMERA VIDEO CIF 3X");
	capture ("/dev/null", "cif", 50, OMX_COLOR_FormatCbYCrY, 30,
		OMX_WhiteBalControlAuto, GOO_TI_CAMERA_ZOOM_3X);

	return;
}
END_TEST

START_TEST (BF21)
{
	T ("CAMERA VIDEO CIF 4X");
	capture ("/dev/null", "cif", 50, OMX_COLOR_FormatCbYCrY, 30,
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
	capture ("/dev/null", "vga", 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlSunLight, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF58)
{
	T ("CAMERA VIDEO VGA INCANDESCENT");
	capture ("/dev/null", "vga", 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlIncandescent, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF60)
{
	T ("CAMERA VIDEO VGA FLUORESCENT");
	capture ("/dev/null", "vga", 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlFluorescent, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF62)
{
	T ("CAMERA VIDEO VGA HORIZON");
	capture ("/dev/null", "vga", 20, OMX_COLOR_FormatCbYCrY, 30,
		 OMX_WhiteBalControlHorizon, GOO_TI_CAMERA_ZOOM_1X);

	return;
}
END_TEST

START_TEST (BF64)
{
	T ("CAMERA VIDEO VGA HORIZON");
	capture ("/dev/null", "vga", 20, OMX_COLOR_FormatCbYCrY, 30,
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
