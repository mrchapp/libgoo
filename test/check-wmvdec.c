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

#include <goo-ti-component-factory.h>
#include <goo-engine.h>
#include <goo-utils.h>
#include <goo-ti-wmvdec.h>
#include <check.h>
#include <stdlib.h>

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <glib.h>

GooComponentFactory* factory;
GooComponent* component;
GRand* rnd;

void
setup (void)
{
	factory = goo_ti_component_factory_get_instance ();
	component = goo_component_factory_get_component (factory, GOO_TI_WMV_DECODER);
	rnd = g_rand_new_with_seed (time (0));

	return;
}

void
teardown (void)
{
	g_object_unref (component);
	g_object_unref (factory);
	g_rand_free (rnd);

	return;
}

static void
process (gchar *infile, gint width, gint height, gint outcolor, GooTiVideoDecoderProcessMode process_mode)
{
	fail_unless (infile != NULL, "unspecified filename in test");
	fail_unless (g_file_test (infile, G_FILE_TEST_IS_REGULAR),
		     "file don't exist");

	gchar outfile[100];
	gchar *fn, *fn1;
	gboolean vopparser;

	fn = g_path_get_basename (infile);
	fn1 = strchr (fn, '.');
	fn1[0] = '\0';
	g_snprintf (outfile, 100, "/tmp/%s.yuv", fn);
	g_free (fn);

	/* input port */
	{
		GooPort* port = goo_component_get_port (component, "input0");
		g_assert (port != NULL);
		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->nBufferCountActual = 4;
		param->format.video.nFrameWidth = width;
		param->format.video.nFrameHeight = height;
		param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

		g_object_unref (G_OBJECT (port));
	}

	/* output port */
	{
		GooPort* port = goo_component_get_port (component, "output0");
		g_assert (port != NULL);
		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->nBufferCountActual = 4;
		param->format.video.nFrameWidth = width;
		param->format.video.nFrameHeight = height;
		param->format.video.eColorFormat = outcolor;

		g_object_unref (G_OBJECT (port));
	}

	/* video properties */
	  g_object_set (G_OBJECT (component),
					"process-mode", process_mode,
					NULL);
	vopparser = (process_mode == GOO_TI_VIDEO_DECODER_FRAMEMODE) ? TRUE : FALSE;

/*	GooEngine* engine = goo_engine_new_vop (component, infile, outfile, vopparser); */
	GooEngine* engine = goo_engine_new_vop (component, infile, "/dev/null", vopparser);

	goo_component_set_state_idle (component);
	goo_component_set_state_executing (component);

	goo_engine_play (engine);

	goo_component_set_state_idle (component);
	goo_component_set_state_loaded (component);

	g_object_unref (G_OBJECT (engine));

	return;
}

START_TEST (BF0034)
{
	process ("/omx/patterns/SA00040_qcif.vc1",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0035)
{
	process ("/omx/patterns/SA00045_cif.vc1",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0038)
{
	process ("/omx/patterns/SA00059.vc1",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0039)
{
	process ("/omx/patterns/SA10176.vc1",
		 720, 576, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0041)
{
	process ("/omx/patterns/SA00045_cif.vc1",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0042)
{
	process ("/omx/patterns/SA00045_cif.vc1",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0043)
{
	process ("/omx/patterns/SA00045_cif.vc1",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0044)
{
	process ("/omx/patterns/SA00045_cif.vc1",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0126)
{
	process ("/omx/patterns/sa00005_w352_h288_cif.vc1",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0127)
{
	process ("/omx/patterns/sa00019_w352_h288_cif.vc1",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0128)
{
	process ("/omx/patterns/sa10000_w720_h480.vc1",
		 720, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0129)
{
	process ("/omx/patterns/sa10014_w720_h480.vc1",
		 720, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0162)
{
	process ("/omx/patterns/SA00045_cif.vc1",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0164)
{
	process ("/omx/patterns/SA00059.vc1",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0165)
{
	process ("/omx/patterns/Iced_640x480_30fps_2100kbps.vc1",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0166)
{
	process ("/omx/patterns/Iced_640x480_30fps_2100kbps.vc1",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0192)
{
	process ("/omx/patterns/pinball_176x144_ap.vc1",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST



START_TEST (SR15404)
{
	process ("/multimedia/patterns/SR15404.vc1",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST


START_TEST (SR15405)
{
	process ("/multimedia/patterns/SR15405.vc1",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (SR17024)
{
	process ("/multimedia/patterns/SR17024.vc1",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_wmv)
{
	tcase_add_test (tc_wmv, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_wmv = tcase_create ("WMV");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "BF0034", BF0034);
	g_hash_table_insert (ht, "BF0035", BF0035);
	g_hash_table_insert (ht, "BF0038", BF0038);
	g_hash_table_insert (ht, "BF0039", BF0039);
	g_hash_table_insert (ht, "BF0041", BF0041);
	g_hash_table_insert (ht, "BF0042", BF0042);
	g_hash_table_insert (ht, "BF0043", BF0043);
	g_hash_table_insert (ht, "BF0044", BF0044);
	g_hash_table_insert (ht, "BF0126", BF0126);
	g_hash_table_insert (ht, "BF0127", BF0127);
	g_hash_table_insert (ht, "BF0128", BF0128);
	g_hash_table_insert (ht, "BF0129", BF0129);
	g_hash_table_insert (ht, "BF0162", BF0162);
	g_hash_table_insert (ht, "BF0164", BF0164);
	g_hash_table_insert (ht, "BF0165", BF0165);
	g_hash_table_insert (ht, "BF0166", BF0166);
	g_hash_table_insert (ht, "BF0192", BF0192);
	g_hash_table_insert (ht, "SR15404", SR15404);
	g_hash_table_insert (ht, "SR15405", SR15405);
	g_hash_table_insert (ht, "SR17024", SR17024);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_wmv);
	}
	else
	{
		tcase_add_test (tc_wmv, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_wmv, setup, teardown);
	tcase_set_timeout (tc_wmv, 0);
	suite_add_tcase (s, tc_wmv);

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
		  "Test option: (BF0034/BF0035/BF0038/BF0039/BF0041/BF0042/BF0043/BF0044/BF0126/BF0127/BF0128/BF0129/BF0162/BF0164/BF0165/BF0166/BF0192/SR15404/SR15405/SR17024)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- WMV Tests.");
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
