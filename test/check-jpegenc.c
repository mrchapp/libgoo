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

#include <goo-ti-jpegenc.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

GooComponentFactory* factory;
GooComponent* component;
GRand* rnd;

void
setup (void)
{
        factory = goo_ti_component_factory_get_instance ();
        component = goo_component_factory_get_component (factory,
                                                         GOO_TI_JPEG_ENCODER);

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
process (gchar* filename, gint colorformat, gint width, gint height)
{
        g_assert (filename != NULL);

        gchar outfile[100];
        gchar *fn, *fn1;

        fn = g_path_get_basename (filename);
        fn1 = strchr (fn, '.');
        fn1[0] = '\0';
        g_snprintf (outfile, 100, "/tmp/%s.jpeg", fn);
        g_free (fn);

        /* param */
        {
                OMX_IMAGE_PARAM_QFACTORTYPE* param;

                param = GOO_TI_JPEGENC_GET_PARAM (component);
                param->nQFactor = g_rand_int_range (rnd, 0, 100);
        }

        /* inport */
        {
                GooPort* port = goo_component_get_port (component, "input0");
                g_assert (port != NULL);

                OMX_PARAM_PORTDEFINITIONTYPE* param;
                param = GOO_PORT_GET_DEFINITION (port);
                param->format.image.nFrameWidth = width;
                param->format.image.nFrameHeight = height;
                param->format.image.eColorFormat = colorformat;

                /* mandatory for JPEG encoder */
                g_object_set (port, "buffercount", 1, NULL);

                g_object_unref (port);
        }

        /* outport */
        {
                GooPort* port = goo_component_get_port (component, "output0");
                g_assert (port != NULL);

                OMX_PARAM_PORTDEFINITIONTYPE* param;
                param = GOO_PORT_GET_DEFINITION (port);
                param->format.image.nFrameWidth = width;
                param->format.image.nFrameHeight = height;
                param->format.image.eColorFormat = colorformat;

                /* mandatory for JPEG encoder */
                g_object_set (port, "buffercount", 1, NULL);

                g_object_unref (port);
        }

        GooEngine* engine = goo_engine_new (component, filename, outfile);

        goo_component_set_state_idle (component);

        g_object_set (component, "comment", "hello world", NULL);

        goo_component_set_state_executing (component);

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (engine);

        return;
}

START_TEST (test_jpegenc_0)
{
        process ("/omx/patterns/JPGE_CONF_011.yuv",
                 OMX_COLOR_FormatYUV420PackedPlanar, 176, 144);
        return;
}
END_TEST

START_TEST (test_jpegenc_1)
{
        process ("/omx/patterns/JPGE_CONF_003.yuv",
                 OMX_COLOR_FormatCbYCrY, 176, 144);
        return;
}
END_TEST

START_TEST (test_jpegenc_2)
{
         process ("/omx/patterns/JPGE_CONF_004.yuv",
                 OMX_COLOR_FormatCbYCrY, 320, 240);
        return;
}
END_TEST

START_TEST (test_jpegenc_3)
{
         process ("/omx/patterns/JPGE_CONF_005.yuv",
                 OMX_COLOR_FormatCbYCrY, 352, 288);
        return;
}
END_TEST

START_TEST (test_jpegenc_4)
{
        process ("/omx/patterns/JPGE_CONF_006.yuv",
                 OMX_COLOR_FormatCbYCrY, 640, 480);
        return;
}
END_TEST

START_TEST (test_jpegenc_5)
{
          process ("/omx/patterns/JPGE_CONF_037.yuv",
                 OMX_COLOR_FormatYUV420PackedPlanar, 704, 576);
        return;
}
END_TEST

START_TEST (test_jpegenc_6)
{
        process ("/omx/patterns/sxga.yuv",
                 OMX_COLOR_FormatCbYCrY, 1280, 1024);
        return;
}
END_TEST

START_TEST (test_jpegenc_7)
{
        process ("/omx/patterns/uxga.yuv",
                 OMX_COLOR_FormatCbYCrY, 1600, 1200);
        return;
}
END_TEST

START_TEST (test_jpegenc_8)
{
        process ("/omx/patterns/stockholm.yuv",
                 OMX_COLOR_FormatCbYCrY, 1536, 1152);
        return;
}
END_TEST

START_TEST (test_jpegenc_9)
{
        process ("/omx/patterns/6MP_1600x3600.yuv",
                 OMX_COLOR_FormatCbYCrY, 1600, 3600);
        return;
}
END_TEST

START_TEST (test_jpegenc_10)
{
        process ("/omx/patterns/7_8MP_1600x4800.yuv",
                 OMX_COLOR_FormatCbYCrY, 1600, 4800);
        return;
}
END_TEST

START_TEST (test_jpegenc_11)
{
        process ("/omx/patterns/stockholmSXGA.yuv",
                 OMX_COLOR_FormatCbYCrY, 1280, 1024);
        return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_jpegenc)
{
	tcase_add_test (tc_jpegenc, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_jpegenc = tcase_create ("JpegDecoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_jpegenc_0);
	g_hash_table_insert (ht, "SR2", test_jpegenc_1);
	g_hash_table_insert (ht, "SR3", test_jpegenc_2);
	g_hash_table_insert (ht, "SR4", test_jpegenc_3);
	g_hash_table_insert (ht, "SR5", test_jpegenc_4);
	g_hash_table_insert (ht, "SR6", test_jpegenc_5);
	g_hash_table_insert (ht, "SR7", test_jpegenc_6);
	g_hash_table_insert (ht, "SR8", test_jpegenc_7);
	g_hash_table_insert (ht, "SR9", test_jpegenc_8);
	g_hash_table_insert (ht, "SR10", test_jpegenc_9);
	g_hash_table_insert (ht, "SR11", test_jpegenc_10);
	g_hash_table_insert (ht, "SR12", test_jpegenc_11);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_jpegenc);
	}
	else
	{
		tcase_add_test (tc_jpegenc, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_jpegenc, setup, teardown);
	tcase_set_timeout (tc_jpegenc, 0);
	suite_add_tcase (s, tc_jpegenc);

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
		  "Test option: (SR1/SR2/.../SR11/SR12)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- JPEG Encoder Tests.");
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
