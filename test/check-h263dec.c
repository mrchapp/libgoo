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
#include <goo-ti-h263dec.h>
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
	component = goo_component_factory_get_component (factory, GOO_TI_H263_DECODER);
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

START_TEST (BF0015)
{
	process ("/omx/patterns/foreman_cif_bp45_30fps_2mbps.263",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);

	return;
}
END_TEST

START_TEST (BF0021)
{
	process ("/omx/patterns/foreman_cif_bp20_15fps_128kbps.263",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0023)
{
	process ("/omx/patterns/carphone_qcif_bp10_15fps_64kbps.263",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0024)
{
	process ("/omx/patterns/foreman_cif_bp20_15fps_128kbps.263",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0025)
{
	process ("/omx/patterns/foreman_cif_bp30_30fps_384kbps.263",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0026)
{
	process ("/omx/patterns/carphone_qcif_bp45_15fps_128kbps.263",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0135)
{
	process ("/omx/patterns/foreman_cif_annexJ.263",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0136)
{
	process ("/omx/patterns/foreman_cif_annexJ.263",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST


START_TEST (BF0158)
{
	process ("/omx/patterns/foreman_cif_bp45_30fps_2mbps.263",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST


START_TEST (SR11992)
{
	process ("/multimedia/patterns/SR11992.263",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST


START_TEST (SR11624)
{
	process ("/multimedia/patterns/SR11624.263",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (SR11625)
{
	process ("/multimedia/patterns/SR11625.263",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (SR11626)
{
	process ("/multimedia/patterns/SR11626.263",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (SR11627)
{
	process ("/multimedia/patterns/SR11627.263",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_h263)
{
	tcase_add_test (tc_h263, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_h263 = tcase_create ("H263");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "BF0015", BF0015);
	g_hash_table_insert (ht, "BF0021", BF0021);
	g_hash_table_insert (ht, "BF0023", BF0023);
	g_hash_table_insert (ht, "BF0024", BF0024);
	g_hash_table_insert (ht, "BF0025", BF0025);
	g_hash_table_insert (ht, "BF0026", BF0026);
	g_hash_table_insert (ht, "BF0135", BF0135);
	g_hash_table_insert (ht, "BF0136", BF0136);
	g_hash_table_insert (ht, "BF0158", BF0158);
	g_hash_table_insert (ht, "SR11992", SR11992);
	g_hash_table_insert (ht, "SR11624", SR11624);
	g_hash_table_insert (ht, "SR11625", SR11625);
	g_hash_table_insert (ht, "SR11626", SR11626);
	g_hash_table_insert (ht, "SR11627", SR11627);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_h263);
	}
	else
	{
		tcase_add_test (tc_h263, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_h263, setup, teardown);
	tcase_set_timeout (tc_h263, 0);
	suite_add_tcase (s, tc_h263);

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
		  "Test option: (BF0015/BF0021/BF0023/BF0024/BF0025/BF0026/BF135/BF136/BF0158/SR11992/SR11624/SR11625/SR11626/SR11627)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- H263 Tests.");
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
