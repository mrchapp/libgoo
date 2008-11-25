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
#include <goo-ti-mpeg2dec.h>
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
	component = goo_component_factory_get_component (factory, GOO_TI_MPEG2_DECODER);
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

START_TEST (BF0170)
{
	process ("/omx/patterns/akiyo_frame.m2v",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0171)
{
	process ("/omx/patterns/TankT80UK_CIF.m2v",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0172)
{
	process ("/omx/patterns/ForeMan_VGA.m2v",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0173)
{
	process ("/omx/patterns/akiyo_frame.m2v",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0174)
{
	process ("/omx/patterns/TankT80UK_CIF.m2v",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0175)
{
	process ("/omx/patterns/ForeMan_VGA.m2v",
		 640, 480, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0176)
{
	process ("/omx/patterns/akiyo_frame.m2v",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0177)
{
	process ("/omx/patterns/TankT80UK_CIF.m2v",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0178)
{
	process ("/omx/patterns/ForeMan_VGA.m2v",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0179)
{
	process ("/omx/patterns/akiyo_frame.m2v",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0180)
{
	process ("/omx/patterns/TankT80UK_CIF.m2v",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0181)
{
	process ("/omx/patterns/ForeMan_VGA.m2v",
		 640, 480, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST


START_TEST (qcif_uyvy)
{
	process ("/multimedia/akiyo_frame.m2v",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST


void
fill_tcase (gchar* srd, gpointer func, TCase* tc_mpeg2)
{
	tcase_add_test (tc_mpeg2, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_mpeg2 = tcase_create ("MPEG2");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "BF0170", BF0170);
	g_hash_table_insert (ht, "BF0171", BF0171);
	g_hash_table_insert (ht, "BF0172", BF0172);
	g_hash_table_insert (ht, "BF0173", BF0173);
	g_hash_table_insert (ht, "BF0174", BF0174);
	g_hash_table_insert (ht, "BF0175", BF0175);
	g_hash_table_insert (ht, "BF0176", BF0176);
	g_hash_table_insert (ht, "BF0177", BF0177);
	g_hash_table_insert (ht, "BF0178", BF0178);
	g_hash_table_insert (ht, "BF0179", BF0179);
	g_hash_table_insert (ht, "BF0180", BF0180);
	g_hash_table_insert (ht, "BF0181", BF0181);
	g_hash_table_insert (ht, "qcif_uyvy", qcif_uyvy);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_mpeg2);
	}
	else
	{
		tcase_add_test (tc_mpeg2, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_mpeg2, setup, teardown);
	tcase_set_timeout (tc_mpeg2, 0);
	suite_add_tcase (s, tc_mpeg2);

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
		  "Test option: (BF0170/BF0171/BF0172/BF0173/BF0174/BF0175/BF0176/BF0177/BF0178/BF0179/BF0180/BF0181/qcif_uyvy)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- MPEG2 Tests.");
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
