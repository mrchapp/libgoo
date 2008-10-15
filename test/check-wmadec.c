/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

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

#include <goo-ti-wmadec.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>

START_TEST (test_wmadec_1)
{
        GooComponentFactory* factory =
                goo_ti_component_factory_get_instance ();
        GooComponent* component =
                goo_component_factory_get_component (factory,
                                                     GOO_TI_WMA_DECODER);
        /* audio properties */

        g_object_set (G_OBJECT (component),
                      "wmatxtfile", "/omx/patterns/test1_wma_v8_5kbps_8khz_1.rca.txt",
                      "dasf-mode", TRUE,
                      //"mute", FALSE,
                      //"volume", 100,
                      NULL);

        /* component parameters */
        {
                GOO_TI_WMADEC_GET_INPUT_PARAM (component)->nBitRate = 8000;
                GOO_TI_WMADEC_GET_INPUT_PARAM (component)->nChannels = 1;
        }

        GOO_OBJECT_WARNING (component, "stream ID = %d",
                            goo_ti_audio_component_get_stream_id
                            (GOO_TI_AUDIO_COMPONENT (component)));

        goo_component_set_state_idle (component);

        goo_component_set_state_executing (component);

        GooEngine* engine = goo_engine_new (
                component,
                "/omx/patterns/test1_wma_v8_5kbps_8khz_1.rca",
                "/tmp/hecommon_Stereo_44Khz.pcm"
                );

        g_object_set (engine, "eosevent", TRUE, NULL);

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (component));
        g_object_unref (G_OBJECT (factory));

}
END_TEST

#if 0
START_TEST (test_wmadec_2)
{
        GooComponentFactory* factory =
                goo_ti_component_factory_get_instance ();
        GooComponent* component =
                goo_component_factory_get_component (factory,
                                                     GOO_TI_WMA_DECODER);

        /* audio properties */
        /* DR for 18.7 !! */
/*         g_object_set (G_OBJECT (component), */
/*                       "dasf-mode", FALSE, */
/*                       "mute", FALSE, */
/*                       "volume", 100, */
/*                       NULL); */

        /* component parameters */
        {
                GOO_TI_WMADEC_GET_PARAM (component)->nBitPerSample = 16;
                GOO_TI_WMADEC_GET_PARAM (component)->nChannels = 2;
                GOO_TI_WMADEC_GET_PARAM (component)->nSamplingRate = 16000;
        }

        goo_component_set_state_idle (component);

        guint streamid;
        g_object_get (G_OBJECT (component), "streamid", &streamid, NULL);
        GOO_OBJECT_DEBUG (component, "stream ID = %d", streamid);

        goo_component_set_state_executing (component);

        GooEngine* engine = goo_engine_new (
                component,
                "/omx/patterns/16_Stereo_160_16.wma",
                "/tmp/16_Stereo_160_16.pcm"
                );

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (component));
        g_object_unref (G_OBJECT (factory));

}
END_TEST

START_TEST (test_wmadec_3)
{
        GooComponentFactory* factory =
                goo_ti_component_factory_get_instance ();
        GooComponent* component =
                goo_component_factory_get_component (factory,
                                                     GOO_TI_WMA_DECODER);

        /* audio properties */
        g_object_set (G_OBJECT (component),
                      "dasf-mode", TRUE,
                      "mute", FALSE,
                      "volume", 100,
                      NULL);

        /* component parameters */
        {
                GOO_TI_WMADEC_GET_PARAM (component)->nBitPerSample = 16;
                GOO_TI_WMADEC_GET_PARAM (component)->nChannels = 2;
                GOO_TI_WMADEC_GET_PARAM (component)->nSamplingRate = 32000;
        }

        /* input port */
        {
                GooPort* port = goo_component_get_port (component, "input0");
                g_assert (port != NULL);
                g_object_set (port, "buffercount", 4, NULL);
                g_object_unref (G_OBJECT (port));
        }

        /* output port */
        {
                GooPort* port = goo_component_get_port (component, "output0");
                g_assert (port != NULL);
                g_object_set (port, "buffercount", 4, NULL);
                g_object_unref (G_OBJECT (port));
        }

        goo_component_set_state_idle (component);

        guint streamid;
        g_object_get (G_OBJECT (component), "streamid", &streamid, NULL);
        GOO_OBJECT_DEBUG (component, "stream ID = %d", streamid);

        goo_component_set_state_executing (component);

        GooEngine* engine = goo_engine_new (
                component,
                "/omx/patterns/MJ32khz128kbps_Stereo.wma",
                "/tmp/MJ32khz128kbps_Stereo.pcm"
                );

        g_object_set (engine, "eosevent", TRUE, NULL);

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (component));
        g_object_unref (G_OBJECT (factory));

}
END_TEST
#endif


void
fill_tcase (gchar* srd, gpointer func, TCase* tc_wmadec)
{
	tcase_add_test (tc_wmadec, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_wmadec = tcase_create ("WmaDecoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_wmadec_1);
/*	g_hash_table_insert (ht, "SR2", test_wmadec_2);
	g_hash_table_insert (ht, "SR3", test_wmadec_3);
*/
	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_wmadec);
	}
	else
	{
		tcase_add_test (tc_wmadec, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_set_timeout (tc_wmadec, 0);
	suite_add_tcase (s, tc_wmadec);

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

	ctx = g_option_context_new ("- WMA Decoder Tests.");
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
