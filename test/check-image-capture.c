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
#include <goo-ti-jpegenc.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>
#include <time.h>

GooComponentFactory* factory;
GooComponent* jpegenc;
GooComponent* camera;
GRand* rnd;

void
setup (void)
{
        factory = goo_ti_component_factory_get_instance ();
        jpegenc = goo_component_factory_get_component (factory,
                                                       GOO_TI_JPEG_ENCODER);
        camera = goo_component_factory_get_component (factory, GOO_TI_CAMERA);


        rnd = g_rand_new_with_seed (time (0));

        return;
}

void
teardown (void)
{
        g_object_unref (jpegenc);
        g_object_unref (camera);
        g_object_unref (factory);
        g_rand_free (rnd);

        return;
}

void
capture (gchar* filename, guint numbuffers,
         guint width, guint height,
         guint quality, gint contrast, guint brightness)
{
        g_assert (filename != NULL);

        /* CAMERA */
        {
                {
                        OMX_PARAM_SENSORMODETYPE* param;
                        param = GOO_TI_CAMERA_GET_PARAM (camera);
                        param->bOneShot = OMX_TRUE;
                        param->nFrameRate = 0;
                        param->sFrameSize.nWidth = width;
                        param->sFrameSize.nHeight = height;
                }

                /* output port */
                {
                        GooPort* port = goo_component_get_port (camera,
                                                                "output1");
                        g_assert (port != NULL);

			g_object_set (port, "buffercount", 1, NULL);

			OMX_PARAM_PORTDEFINITIONTYPE* param;
                        param = GOO_PORT_GET_DEFINITION (port);
                        param->format.image.eColorFormat =
                                OMX_COLOR_FormatCbYCrY;

                        g_object_unref (port);
                }

        }

        /* JPEG ECODER */
        {
                /* param */
                {
                        OMX_IMAGE_PARAM_QFACTORTYPE* param;

                        param = GOO_TI_JPEGENC_GET_PARAM (jpegenc);
                        param->nQFactor = quality;
                }

                /* inport */
                {
                        GooPort* port = goo_component_get_port (jpegenc,
                                                                "input0");
                        g_assert (port != NULL);

                        OMX_PARAM_PORTDEFINITIONTYPE* param;
                        param = GOO_PORT_GET_DEFINITION (port);
                        param->format.image.nFrameWidth = width;
                        param->format.image.nFrameHeight = height;
                        param->format.image.eColorFormat =
                                OMX_COLOR_FormatCbYCrY;

                        /* mandatory for JPEG encoder */
                        g_object_set (port, "buffercount", 1, NULL);

                        g_object_unref (port);
                }

                /* outport */
                {
                        GooPort* port = goo_component_get_port (jpegenc,
                                                                "output0");
                        g_assert (port != NULL);

                        OMX_PARAM_PORTDEFINITIONTYPE* param;
                        param = GOO_PORT_GET_DEFINITION (port);
                        param->format.image.nFrameWidth = width;
                        param->format.image.nFrameHeight = height;
                        param->format.image.eColorFormat =
                                OMX_COLOR_FormatCbYCrY;

                        /* Mandatory for JPEG encoder */
                        g_object_set (port, "buffercount", 1, NULL);

                        g_object_unref (port);
                }
        }

        GooEngine* engine = goo_engine_new (
                jpegenc,
                NULL,
                filename
                );
        g_object_set (engine, "num-buffers", numbuffers, NULL);
        g_object_set (engine, "mainloop", FALSE, NULL);

        goo_component_set_tunnel_by_name (camera, "output1",
                                          jpegenc, "input0", OMX_BufferSupplyOutput);

        goo_component_set_state_idle (camera);
        goo_component_set_state_idle (jpegenc);

        g_object_set (camera,
                      "brightness", brightness,
                      "contrast", contrast, NULL);

        g_object_set (jpegenc, "comment", "hello world", NULL);

        goo_component_set_state_executing (camera);
        goo_component_set_state_executing (jpegenc);

        g_usleep (5 * G_USEC_PER_SEC);

        g_object_set (camera, "capture", TRUE, NULL);

        goo_engine_play (engine);

        goo_component_set_state_idle (camera);
        goo_component_set_state_idle (jpegenc);
        goo_component_set_state_loaded (camera);
        goo_component_set_state_loaded (jpegenc);

        g_object_unref (G_OBJECT (engine));

        return;
}

START_TEST (test_capture_svga)
{
        capture ("/tmp/capture_svga.jpeg", 5, 800, 600,
                 g_rand_int_range (rnd, 0, 100),
                 g_rand_int_range (rnd, -100, 100),
                 g_rand_int_range (rnd, 0, 100));

        return;
}
END_TEST

START_TEST (test_capture_xvga)
{
        capture ("/tmp/capture_xvga.jpeg", 3, 1024, 768,
                 g_rand_int_range (rnd, 0, 100),
                 g_rand_int_range (rnd, -100, 100),
                 g_rand_int_range (rnd, 0, 100));

        return;
}
END_TEST

START_TEST (test_capture_sxvga)
{
        capture ("/tmp/capture_sxvga.jpeg", 2, 1280, 1024,
                 g_rand_int_range (rnd, 0, 100),
                 g_rand_int_range (rnd, -100, 100),
                 g_rand_int_range (rnd, 0, 100));

        return;
}
END_TEST

START_TEST (test_capture_uxvga)
{
        capture ("/tmp/capture_uxvga.jpeg", 1, 1600, 1200,
                 g_rand_int_range (rnd, 0, 100),
                 g_rand_int_range (rnd, -100, 100),
                 g_rand_int_range (rnd, 0, 100));

        return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_capture)
{
	tcase_add_test (tc_capture, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo Image Capture Test");
	TCase *tc_capture = tcase_create ("Camera");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "capture-svga", test_capture_svga);
	g_hash_table_insert (ht, "capture_xvga", test_capture_xvga);
	g_hash_table_insert (ht, "capture_sxvga", test_capture_sxvga);
	g_hash_table_insert (ht, "capture_uxvga", test_capture_uxvga);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_capture);
	}
	else
	{
		tcase_add_test (tc_capture, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_capture, setup, teardown);
	tcase_set_timeout (tc_capture, 0);
	suite_add_tcase (s, tc_capture);

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
		  "Test option: (capture-svga/xvga/sxvga/uxvga)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- Image Capture Tests.");
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
