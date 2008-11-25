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

#include <goo-ti-vpp.h>

#include <check.h>
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
	/* system ("/etc/init.d/bridge restart"); */
        factory = goo_ti_component_factory_get_instance ();
        component = goo_component_factory_get_component (factory, GOO_TI_VPP);
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
process (gchar *infile, gint inw, gint inh, gint incolor,
	 gint outw, gint outh, gint outcolor, gint rotation)
{
	fail_unless (infile != NULL, "unspecified filename in test");
	fail_unless (g_file_test (infile, G_FILE_TEST_IS_REGULAR),
		     "file don't exist");

	gchar outfile[100];
	gchar *fn, *fn1;

        fn = g_path_get_basename (infile);
        fn1 = strchr (fn, '.');
        fn1[0] = '\0';
        g_snprintf (outfile, 100, "/tmp/%s-vpp.yuv", fn);
        g_free (fn);

	/* input port */
	{
		GooPort* port = goo_component_get_port (component, "input0");
		g_assert (port != NULL);
		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->format.video.nFrameWidth = inw;
		param->format.video.nFrameHeight = inh;
		param->format.video.eColorFormat = incolor;

		g_object_unref (G_OBJECT (port));
	}

	/* overlay port */
	{
		GooPort* port = goo_component_get_port (component, "input1");
		g_assert (port != NULL);

		goo_component_disable_port (component, port);

		g_object_unref (G_OBJECT (port));
	}

	/* rbg output port */
	{
		GooPort* port = goo_component_get_port (component, "output0");
		g_assert (port != NULL);

		goo_component_disable_port (component, port);

		g_object_unref (G_OBJECT (port));
	}

	/* rby&yuv output port */
	{
		GooPort* port = goo_component_get_port (component, "output1");
		g_assert (port != NULL);
		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->format.video.nFrameWidth = outw;
		param->format.video.nFrameHeight = outh;
		param->format.video.eColorFormat = outcolor;

		g_object_unref (G_OBJECT (port));
	}

	g_object_set (component,
		      "rotation", rotation,
/* 		      "zoom-factor", g_rand_int_range (rnd, 1, 64), */
/* 		      "zoom-limit", g_rand_int_range (rnd, 1, 64), */
		      "contrast", g_rand_int_range (rnd, -100, 100),
		      NULL);

	GooEngine* engine = goo_engine_new (component, infile, outfile);

	/* FALSE = don't send eos/empty buffer */
	g_object_set (engine, "eosevent", TRUE, NULL);

	goo_component_set_state_idle (component);
	goo_component_set_state_executing (component);

	goo_engine_play (engine);

	goo_component_set_state_idle (component);
	goo_component_set_state_loaded (component);

	g_object_unref (G_OBJECT (engine));

	return;
}

START_TEST (test_vpp_1)
{
	process ("/multimedia/patterns/videotestsrc_qcif_yuy2.yuv",
		 176, 144, OMX_COLOR_FormatYCbYCr,
		 352, 288, OMX_COLOR_FormatYCbYCr,
		 GOO_TI_VPP_ROTATION_NONE);

	return;
}
END_TEST

START_TEST (test_vpp_2)
{
	process ("/omx/patterns/JPGE_CONF_011.yuv",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar,
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar,
		 GOO_TI_VPP_ROTATION_90);

	return;
}
END_TEST

START_TEST (test_vpp_3)
{
	process ("/omx/patterns/uxga.yuv",
		 1600, 1200, OMX_COLOR_FormatCbYCrY,
		 320, 240, OMX_COLOR_FormatYUV420PackedPlanar,
		 GOO_TI_VPP_ROTATION_270);

	return;
}
END_TEST

START_TEST (test_vpp_4)
{
	process ("/omx/patterns/foreman_qcif_422i.yuv",
		 176, 144, OMX_COLOR_FormatYCbYCr,
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar,
		 GOO_TI_VPP_ROTATION_NONE);

	return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_vpp)
{
	tcase_add_test (tc_vpp, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_vpp = tcase_create ("VPP");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_vpp_1);
	g_hash_table_insert (ht, "SR2", test_vpp_2);
	g_hash_table_insert (ht, "SR3", test_vpp_3);
	g_hash_table_insert (ht, "SR4", test_vpp_4);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_vpp);
	}
	else
	{
		tcase_add_test (tc_vpp, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_vpp, setup, teardown);
	tcase_set_timeout (tc_vpp, 0);
	suite_add_tcase (s, tc_vpp);

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
		  "Test option: (SR1/SR2/SR3/SR4)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- VPP Tests.");
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
