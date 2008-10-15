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

#include <goo-ti-clock.h>
#include <goo-ti-pcmdec.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>
#include <time.h>
#include <glib/gprintf.h>

GooComponentFactory* factory;
GooComponent* component;
GRand* rnd;

void
setup (void)
{
	factory = goo_ti_component_factory_get_instance ();
	component =
		goo_component_factory_get_component (factory, GOO_TI_CLOCK);

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

START_TEST (test_no_stream)
{
	gint64 ts;

	goo_component_set_state_idle (component);
	goo_component_set_state_executing (component);

	g_object_get (component, "timestamp", &ts, NULL);
	g_printf ("timestamp = %lld\n", ts);

	g_usleep (5 * G_USEC_PER_SEC);

	g_object_get (component, "timestamp", &ts, NULL);
	g_printf ("timestamp = %lld\n", ts);

	g_usleep (5 * G_USEC_PER_SEC);

	g_object_get (component, "timestamp", &ts, NULL);
	g_printf ("timestamp = %lld\n", ts);

	g_usleep (5 * G_USEC_PER_SEC);

	g_object_get (component, "timestamp", &ts, NULL);
	g_printf ("timestamp = %lld\n", ts);

	goo_component_set_state_idle (component);
	goo_component_set_state_loaded (component);

	return;
}
END_TEST

START_TEST (test_pcm_stream)
{
	gint64 ts;

	GooComponent* pcmdec =
		goo_component_factory_get_component (factory,
						     GOO_TI_PCM_DECODER);

	g_object_set (G_OBJECT (pcmdec),
		      "dasf-mode", TRUE,
		      "mute", FALSE,
		      "volume", 100,
		      NULL);

	goo_component_set_clock (pcmdec, component);

	/* pcmdec parameters */
	{
		GOO_TI_PCMDEC_GET_INPUT_PARAM (pcmdec)->nChannels = 2;
		GOO_TI_PCMDEC_GET_INPUT_PARAM (pcmdec)->nSamplingRate = 44100;
	}

	GOO_OBJECT_WARNING (pcmdec, "stream ID = %d",
			    goo_ti_audio_component_get_stream_id
			    (GOO_TI_AUDIO_COMPONENT (pcmdec)));

	goo_component_set_state_idle (component);
	goo_component_set_state_idle (pcmdec);
	goo_component_set_state_executing (component);
	goo_component_set_state_executing (pcmdec);

	g_object_get (component, "timestamp", &ts, NULL);
	g_printf ("timestamp = %lld\n", ts);

	GooEngine* engine = goo_engine_new (
		pcmdec,
		"/omx/patterns/s44.wav",
		NULL
		);

	g_object_get (component, "timestamp", &ts, NULL);
	g_printf ("timestamp = %lld\n", ts);

	goo_engine_play (engine);

	g_object_get (component, "timestamp", &ts, NULL);
	g_printf ("timestamp = %lld\n", ts);

	goo_component_set_state_idle (component);
	goo_component_set_state_idle (pcmdec);
	goo_component_set_state_loaded (component);
	goo_component_set_state_loaded (pcmdec);

	g_object_unref (G_OBJECT (engine));
	g_object_unref (G_OBJECT (pcmdec));

	return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_clock)
{
	tcase_add_test (tc_clock, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo Clock Test");
	TCase *tc_clock = tcase_create ("Clock");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "no-stream", test_no_stream);
	g_hash_table_insert (ht, "pcm-stream", test_pcm_stream);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_clock);
	}
	else
	{
		tcase_add_test (tc_clock, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_clock, setup, teardown);
	tcase_set_timeout (tc_clock, 0);
	suite_add_tcase (s, tc_clock);

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
		  "Test option: (no-stream/pcm-stream)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- Clock Tests.");
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
