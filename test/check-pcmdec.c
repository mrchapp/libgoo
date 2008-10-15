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

#include <goo-ti-pcmdec.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>

START_TEST (test_pcmdec_1)
{
        GooComponentFactory* factory =
                goo_ti_component_factory_get_instance ();
        GooComponent* component =
                goo_component_factory_get_component (factory,
                                                     GOO_TI_PCM_DECODER);
        /* audio properties */

        g_object_set (G_OBJECT (component),
                      "dasf-mode", TRUE,
                       "data-path", NULL,
                      //"mute", FALSE,
                      //"volume", 100,
                      NULL);

        /* component parameters */
        {
                GOO_TI_PCMDEC_GET_INPUT_PARAM (component)->nChannels = 2;
                GOO_TI_PCMDEC_GET_INPUT_PARAM (component)->nSamplingRate = 44100;
        }

        GOO_OBJECT_WARNING (component, "stream ID = %d",
                            goo_ti_audio_component_get_stream_id
                            (GOO_TI_AUDIO_COMPONENT (component)));

        goo_component_set_state_idle (component);

        goo_component_set_state_executing (component);

        GooEngine* engine = goo_engine_new (
                component,
                "/omx/patterns/s44.wav",
                NULL
                );

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (component));
        g_object_unref (G_OBJECT (factory));

}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_pcmdec)
{
	tcase_add_test (tc_pcmdec, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_pcmdec = tcase_create ("PcmDecoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_pcmdec_1);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_pcmdec);
	}
	else
	{
		tcase_add_test (tc_pcmdec, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_set_timeout (tc_pcmdec, 0);
	suite_add_tcase (s, tc_pcmdec);

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
		  "Test option: (SR1)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- PCM Decoder Tests.");
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
