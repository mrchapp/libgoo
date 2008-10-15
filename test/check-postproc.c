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

#include <goo-ti-post-processor.h>

#include <check.h>
#include <stdlib.h>
#include <time.h>

GooComponentFactory* factory;
GooComponent* component;
GRand* rnd;

void
setup (void)
{
	factory = goo_ti_component_factory_get_instance ();
        component =
                goo_component_factory_get_component (factory,
						     GOO_TI_POST_PROCESSOR);

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

void
display (gchar* filename, gchar* resolution, guint color, guint rotation,
	 guint scale, guint xpos, guint ypos, guint bkg, gboolean mirror)
{
        g_assert (filename != NULL);
        g_assert (resolution != NULL);


	ResolutionInfo rinfo = goo_get_resolution (resolution);
        g_assert (rinfo.width != 0 && rinfo.height != 0);

	/* params */
	{
		GOO_TI_POST_PROCESSOR_GET_BACKGROUND (component)->nColor =
			bkg;
	}

        /* input port */
        {
                GooPort* port = goo_component_get_port (component, "input0");
                g_assert (port != NULL);

                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			rinfo.width;
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			rinfo.height;
                GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
                        color;

                g_object_unref (G_OBJECT (port));
        }

	goo_component_set_state_idle (component);

	g_object_set (G_OBJECT (component),
                      "rotation", rotation,
                      "x-scale", scale,
                      "y-scale", scale,
                      "x-pos", xpos,
                      "y-pos", ypos,
		      "mirror", mirror,
                      NULL);

        goo_component_set_state_executing (component);

        GooEngine* engine = goo_engine_new (component, filename, NULL);

        goo_engine_play (engine);

	/* horrible hack */
	/* g_usleep (0.1 * G_USEC_PER_SEC); */
        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

	g_object_unref (G_OBJECT (engine));

	return;
}

START_TEST (test_postproc_1)
{
	display ("/omx/patterns/videotestsrc-uyvy-qcif.yuv", "qcif",
		 OMX_COLOR_FormatCbYCrY, GOO_TI_POST_PROCESSOR_ROTATION_NONE,
		 100, 10, 10, 0xffff00, TRUE);

	return;
}
END_TEST

START_TEST (test_postproc_2)
{
	display ("/omx/patterns/foreman_qcif_422i.yuv", "qcif",
		 OMX_COLOR_FormatYCbYCr, GOO_TI_POST_PROCESSOR_ROTATION_90,
		 100, 20, 20, 0x000000, FALSE);

	return;
}
END_TEST

START_TEST (test_postproc_3)
{
	display ("/omx/patterns/carphone_qcif422-90frames-DECREF.yuv", "qcif",
		 OMX_COLOR_FormatCbYCrY, GOO_TI_POST_PROCESSOR_ROTATION_90,
		 100, 0, 0, 0x000000, TRUE);

	return;
}
END_TEST

START_TEST (test_postproc_4)
{
	display ("/omx/patterns/monster_qcif.yuv", "qcif",
		 OMX_COLOR_FormatCbYCrY, GOO_TI_POST_PROCESSOR_ROTATION_180,
		 100, 20, 30, 0x0000ff, FALSE);

	return;
}
END_TEST

START_TEST (test_postproc_5)
{
	display ("/omx/patterns/foreman_vga_422i_short.yuv", "vga",
		 OMX_COLOR_FormatYCbYCr, GOO_TI_POST_PROCESSOR_ROTATION_90,
		 50, 0, 0, 0x0000ff, FALSE);

	return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_postproc)
{
	tcase_add_test (tc_postproc, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_postproc = tcase_create ("PostProcessor");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_postproc_1);
	g_hash_table_insert (ht, "SR2", test_postproc_2);
	g_hash_table_insert (ht, "SR3", test_postproc_3);
	g_hash_table_insert (ht, "SR4", test_postproc_4);
	g_hash_table_insert (ht, "SR5", test_postproc_5);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_postproc);
	}
	else
	{
		tcase_add_test (tc_postproc, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_postproc, setup, teardown);
        tcase_set_timeout (tc_postproc, 0);
        suite_add_tcase (s, tc_postproc);

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
		  "Test option: (SR1/SR2/SR3/SR4/SR5)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- Post Processor Tests.");
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
