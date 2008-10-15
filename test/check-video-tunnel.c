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

#include <goo-ti-mpeg4dec.h>
#include <goo-ti-post-processor.h>

#include <check.h>
#include <stdlib.h>

START_TEST (test_videotunnel_1)
{
        GooComponentFactory* factory =
        goo_ti_component_factory_get_instance ();
        GooComponent* mpeg4dec =
        goo_component_factory_get_component (factory,
                        GOO_TI_MPEG4_DECODER);
        GooComponent* postproc =
        goo_component_factory_get_component (factory,
                        GOO_TI_POST_PROCESSOR);


        /* MPEG4 video properties */
        g_object_set (G_OBJECT (mpeg4dec),
                        "process-mode", GOO_TI_VIDEO_DECODER_STREAMMODE,
                        NULL);

        /* MPEG4 input port */
        {
                GooPort* port = goo_component_get_port (mpeg4dec, "input0");
                g_assert (port != NULL);

                g_object_set (port, "buffercount", 1, NULL);
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			176;
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			144;
                GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
			OMX_COLOR_FormatCbYCrY;

                g_object_unref (G_OBJECT (port));
        }

        /* MPEG4 output port */
        {
                GooPort* port = goo_component_get_port (mpeg4dec, "output0");
                g_assert (port != NULL);

                g_object_set (port, "buffercount", 1, NULL);
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			176;
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			144;
                GOO_PORT_GET_DEFINITION (port)->format.video.nBitrate = 0;
                GOO_PORT_GET_DEFINITION (port)->format.video.xFramerate = 0;
                GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
			OMX_COLOR_FormatCbYCrY;

                g_object_unref (G_OBJECT (port));
        }

        /* Display input port */
        {
                GooPort* port = goo_component_get_port (postproc, "input0");
                g_assert (port != NULL);

		g_object_set (port, "buffercount", 1, NULL);
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			176;
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			144;
                GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
			OMX_COLOR_FormatCbYCrY;

                g_object_unref (G_OBJECT (port));
        }

        /* display properties */
/*         g_object_set (G_OBJECT (postproc), */
/*                         "rotation", GOO_TI_POST_PROCESSOR_ROTATION_NONE, */
/*                         "x-scale", 100, */
/*                         "y-scale", 100, */
/*                         "x-pos", 0, */
/*                         "y-pos", 0, */
/*                         NULL); */

        /* Setup the tunnel */
        {
                goo_component_set_tunnel_by_name (mpeg4dec, "output0",
						  postproc, "input0", OMX_BufferSupplyInput);
        }

        goo_component_set_state_idle (mpeg4dec);
        goo_component_set_state_idle (postproc);

        goo_component_set_state_executing (mpeg4dec);
        goo_component_set_state_executing (postproc);

        GooEngine* engine = goo_engine_new (
                mpeg4dec,
                "/omx/patterns/mi3_qcif.m4v",
                NULL
                );

        goo_engine_play (engine);

        goo_component_set_state_idle (mpeg4dec);
        goo_component_set_state_idle (postproc);

        goo_component_set_state_loaded (mpeg4dec);
        goo_component_set_state_loaded (postproc);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (mpeg4dec));
        g_object_unref (G_OBJECT (postproc));
        g_object_unref (G_OBJECT (factory));

}
END_TEST

START_TEST (test_videotunnel_2)
{
        GooComponentFactory* factory =
        goo_ti_component_factory_get_instance ();
        GooComponent* vpp =
        goo_component_factory_get_component (factory,
                        GOO_TI_VPP);
        GooComponent* postproc =
        goo_component_factory_get_component (factory,
                        GOO_TI_POST_PROCESSOR);


        /* VPP input port */
        {
                GooPort* port = goo_component_get_port (vpp, "input0");
                g_assert (port != NULL);

                g_object_set (port, "buffercount", 1, NULL);
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			176;
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			144;
                GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
			OMX_COLOR_FormatYCbYCr;

                g_object_unref (G_OBJECT (port));
        }

		/* output port */
		{
			GooPort* port = goo_component_get_port (vpp, "output1");
			g_assert (port != NULL);
			OMX_PARAM_PORTDEFINITIONTYPE* param =
				GOO_PORT_GET_DEFINITION (port);

			g_object_set (port, "buffercount", 1, NULL);
			param->format.video.nFrameWidth = 176;
			param->format.video.nFrameHeight = 144;
			param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

			g_object_unref (G_OBJECT (port));
		}

        /* Display input port */
        {
                GooPort* port = goo_component_get_port (postproc, "input0");
                g_assert (port != NULL);

		g_object_set (port, "buffercount", 1, NULL);
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			176;
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			144;
                GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
			OMX_COLOR_FormatCbYCrY;

                g_object_unref (G_OBJECT (port));
        }

        /* display properties */
/*         g_object_set (G_OBJECT (postproc), */
/*                         "rotation", GOO_TI_POST_PROCESSOR_ROTATION_NONE, */
/*                         "x-scale", 100, */
/*                         "y-scale", 100, */
/*                         "x-pos", 0, */
/*                         "y-pos", 0, */
/*                         NULL); */


        /* Setup the tunnel */
        {
                goo_component_set_tunnel_by_name (vpp, "output1",
						  postproc, "input0", OMX_BufferSupplyInput);
        }

        goo_component_set_state_idle (vpp);
        goo_component_set_state_idle (postproc);

        goo_component_set_state_executing (vpp);
        goo_component_set_state_executing (postproc);

        GooEngine* engine = goo_engine_new (
                vpp,
                "/multimedia/patterns/videotestsrc_qcif_yuy2.yuv",
                NULL
                );

		g_object_set (engine, "eosevent", FALSE, NULL);

        goo_engine_play (engine);

        goo_component_set_state_idle (vpp);
        goo_component_set_state_idle (postproc);

        goo_component_set_state_loaded (vpp);
        goo_component_set_state_loaded (postproc);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (vpp));
        g_object_unref (G_OBJECT (postproc));
        g_object_unref (G_OBJECT (factory));

}
END_TEST

START_TEST (test_videotunnel_3)
{
        GooComponentFactory* factory =
        goo_ti_component_factory_get_instance ();
        GooComponent* mpeg4dec =
        goo_component_factory_get_component (factory,
                        GOO_TI_MPEG4_DECODER);
        GooComponent* vpp =
        goo_component_factory_get_component (factory,
                        GOO_TI_VPP);


        /* MPEG4 video properties */
        g_object_set (G_OBJECT (mpeg4dec),
                        "process-mode", GOO_TI_VIDEO_DECODER_STREAMMODE,
                        NULL);

        /* MPEG4 input port */
        {
                GooPort* port = goo_component_get_port (mpeg4dec, "input0");
                g_assert (port != NULL);

                g_object_set (port, "buffercount", 1, NULL);
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			176;
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			144;
                GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
			OMX_COLOR_FormatCbYCrY;

                g_object_unref (G_OBJECT (port));
        }

        /* MPEG4 output port */
        {
                GooPort* port = goo_component_get_port (mpeg4dec, "output0");
                g_assert (port != NULL);

                g_object_set (port, "buffercount", 1, NULL);
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			176;
                GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			144;
                GOO_PORT_GET_DEFINITION (port)->format.video.nBitrate = 0;
                GOO_PORT_GET_DEFINITION (port)->format.video.xFramerate = 0;
                GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
			OMX_COLOR_FormatCbYCrY;

                g_object_unref (G_OBJECT (port));
        }

		/* VPP input port */
		 {
				 GooPort* port = goo_component_get_port (vpp, "input0");
				 g_assert (port != NULL);

				 g_object_set (port, "buffercount", 1, NULL);
				 GOO_PORT_GET_DEFINITION (port)->format.video.nFrameWidth =
			 176;
				 GOO_PORT_GET_DEFINITION (port)->format.video.nFrameHeight =
			 144;
				 GOO_PORT_GET_DEFINITION (port)->format.video.eColorFormat =
			 OMX_COLOR_FormatYCbYCr;

				 g_object_unref (G_OBJECT (port));
		 }

		 /* output port */
		 {
			 GooPort* port = goo_component_get_port (vpp, "output1");
			 g_assert (port != NULL);
			 OMX_PARAM_PORTDEFINITIONTYPE* param =
				 GOO_PORT_GET_DEFINITION (port);

			 g_object_set (port, "buffercount", 1, NULL);
			 param->format.video.nFrameWidth = 352;
			 param->format.video.nFrameHeight = 288;
			 param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

			 g_object_unref (G_OBJECT (port));
		 }


        /* Setup the tunnel */
        {
                goo_component_set_tunnel_by_name (mpeg4dec, "output0",
						  vpp, "input0", OMX_BufferSupplyOutput);
        }

        goo_component_set_state_idle (mpeg4dec);
        goo_component_set_state_idle (vpp);

        goo_component_set_state_executing (mpeg4dec);
        goo_component_set_state_executing (vpp);

        GooEngine* engine = goo_engine_new (
                mpeg4dec,
                "/omx/patterns/mi3_qcif.m4v",
                "output.yuv"
                );

        goo_engine_play (engine);

        goo_component_set_state_idle (mpeg4dec);
        goo_component_set_state_idle (vpp);

        goo_component_set_state_loaded (mpeg4dec);
        goo_component_set_state_loaded (vpp);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (mpeg4dec));
        g_object_unref (G_OBJECT (vpp));
        g_object_unref (G_OBJECT (factory));

}
END_TEST



void
fill_tcase (gchar* srd, gpointer func, TCase* tc_videotunnel)
{
	tcase_add_test (tc_videotunnel, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_videotunnel = tcase_create ("Video-Tunnel");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_videotunnel_1);
	g_hash_table_insert (ht, "SR2", test_videotunnel_2);
	g_hash_table_insert (ht, "SR3", test_videotunnel_3);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_videotunnel);
	}
	else
	{
		tcase_add_test (tc_videotunnel, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_set_timeout (tc_videotunnel, 0);
	suite_add_tcase (s, tc_videotunnel);

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
		  "Test option: (SR1/SR2/SR3)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- Video Tunnel Tests.");
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
