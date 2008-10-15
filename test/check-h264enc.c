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
#include <goo-ti-h264enc.h>
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
	component = goo_component_factory_get_component (factory, GOO_TI_H264_ENCODER);
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
process (gchar *infile, gint width, gint height, gint incolor,
	gint framerate, gint bitrate, gint level)
{
	fail_unless (infile != NULL, "unspecified filename in test");
	fail_unless (g_file_test (infile, G_FILE_TEST_IS_REGULAR),
		     "file don't exist");

	gchar outfile[100];
	gchar *fn, *fn1;

	fn = g_path_get_basename (infile);
	fn1 = strchr (fn, '.');
	fn1[0] = '\0';
	g_snprintf (outfile, 100, "/tmp/%s.264", fn);
	g_free (fn);

	/* video properties */
	g_object_set (G_OBJECT (component),
		"level", level,
		"control-rate", GOO_TI_VIDEO_ENCODER_CR_VARIABLE,
		NULL);

	/* input port */
	{
		GooPort* port = goo_component_get_port (component, "input0");
		g_assert (port != NULL);
		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->nBufferCountActual = 4;
		param->format.video.cMIMEType = "yuv";
		param->format.video.nFrameWidth = width;
		param->format.video.nFrameHeight = height;
		param->format.video.eColorFormat = incolor;
		param->format.video.xFramerate = framerate;

		g_object_unref (G_OBJECT (port));
	}

	/* output port */
	{
		GooPort* port = goo_component_get_port (component, "output0");
		g_assert (port != NULL);
		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->nBufferCountActual = 4;
		param->format.video.cMIMEType = "264";
		param->format.video.nFrameWidth = width;
		param->format.video.nFrameHeight = height;
		param->format.video.nBitrate = bitrate;
		param->format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;

		g_object_unref (G_OBJECT (port));
	}

	GooEngine* engine = goo_engine_new (component, infile, outfile);

	goo_component_set_state_idle (component);
	goo_component_set_state_executing (component);

	g_object_set (component, "frame-interval", 30, NULL);

	goo_engine_play (engine);

	goo_component_set_state_idle (component);
	goo_component_set_state_loaded (component);

	g_object_unref (G_OBJECT (engine));

	return;
}

START_TEST (SR10567)
{
	process ("/multimedia/patterns/SR10567.yuv",
		 176, 144, OMX_COLOR_FormatYCbYCr, 15, 128000, 2);

	return;
}
END_TEST

START_TEST (SR10568)
{
	process ("/multimedia/patterns/SR10568.yuv",
		 352, 288, OMX_COLOR_FormatYCbYCr, 7, 192000, 4);

	return;
}
END_TEST


START_TEST (SR10569)
{
	process ("/multimedia/patterns/SR10569.yuv",
		 352, 288, OMX_COLOR_FormatYCbYCr, 15, 384000, 8);

	return;
}
END_TEST

START_TEST (SR11593)
{
	process ("/multimedia/patterns/SR11593.yuv",
		 352, 288, OMX_COLOR_FormatYCbYCr, 30, 768000, 16);

	return;
}
END_TEST

START_TEST (SR11594)
{
	process ("/multimedia/patterns/SR11594.yuv",
		 176, 144, OMX_COLOR_FormatYCbYCr, 15, 64000, 1);

	return;
}
END_TEST

START_TEST (SR11697)
{
	process ("/multimedia/patterns/SR11697.yuv",
		 640, 480, OMX_COLOR_FormatYCbYCr, 33, 10000000, 256);

	return;
}
END_TEST

START_TEST (SR15367)
{
	process ("/multimedia/patterns/SR15367.yuv",
		 352, 288, OMX_COLOR_FormatYCbYCr, 30, 2000000, 32);

	return;
}
END_TEST

START_TEST (SR15368)
{
	process ("/multimedia/patterns/SR15368.yuv",
		 720, 480, OMX_COLOR_FormatYCbYCr, 15, 4000000, 128);

	return;
}
END_TEST

START_TEST (SR19353)
{
	process ("/multimedia/patterns/SR19353.yuv",
		 352, 576, OMX_COLOR_FormatYCbYCr, 25, 4000000, 64);

	return;
}
END_TEST


void
fill_tcase (gchar* srd, gpointer func, TCase* tc_h264)
{
	tcase_add_test (tc_h264, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_h264 = tcase_create ("H264");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR10567", SR10567);
	g_hash_table_insert (ht, "SR10568", SR10568);
	g_hash_table_insert (ht, "SR10569", SR10569);
	g_hash_table_insert (ht, "SR11593", SR11593);
	g_hash_table_insert (ht, "SR11594", SR11594);
	g_hash_table_insert (ht, "SR11697", SR11697);
	g_hash_table_insert (ht, "SR15367", SR15367);
	g_hash_table_insert (ht, "SR15368", SR15368);
	g_hash_table_insert (ht, "SR19353", SR19353);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_h264);
	}
	else
	{
		tcase_add_test (tc_h264, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_h264, setup, teardown);
	tcase_set_timeout (tc_h264, 0);
	suite_add_tcase (s, tc_h264);

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
		  "Test option: (SR10567/SR10569/SR11593/SR11594/SR11697/SR15367/SR15368/SR19353)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- H264 Tests.");
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
