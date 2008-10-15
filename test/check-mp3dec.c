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

#include <goo-ti-mp3dec.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>

GooComponentFactory* factory;
GooComponent* component;

void
setup (void)
{
	factory = goo_ti_component_factory_get_instance ();
	component = goo_component_factory_get_component (factory,
							 GOO_TI_MP3_DECODER);

	return;
}

void
teardown (void)
{
	g_object_unref (component);
	g_object_unref (factory);

	return;
}

void
process (gchar* infile,	 gchar* outfile, gboolean dasf, guint volume,
	 gint bitpersample, gint channels, gint samplingrate)
{
	g_assert (infile != NULL);

	if (dasf == FALSE)
	{
		g_assert (outfile != NULL);
	}

	if (dasf == TRUE)
	{
		g_object_set (G_OBJECT (component),
			      "dasf-mode", TRUE,
			      "data-path", NULL,
			      "mute", FALSE,
			      "volume", volume,
			      NULL);
	}

	/* component parameters */
	{
		GOO_TI_MP3DEC_GET_OUTPUT_PARAM (component)->nBitPerSample =
			bitpersample;
		GOO_TI_MP3DEC_GET_OUTPUT_PARAM (component)->nChannels =
			channels;
		GOO_TI_MP3DEC_GET_OUTPUT_PARAM (component)->nSamplingRate =
			samplingrate;
	}

	if (dasf == TRUE)
	{
		GOO_TI_MP3DEC_GET_INPUT_PARAM (component)->nChannels =
			channels;
		GOO_TI_MP3DEC_GET_INPUT_PARAM (component)->nSampleRate =
			samplingrate;
	}

	if (dasf == TRUE)
	{
		GOO_OBJECT_WARNING (component, "stream ID = %d",
				    goo_ti_audio_component_get_stream_id
				    (GOO_TI_AUDIO_COMPONENT (component)));
	}

	goo_component_set_state_idle (component);
	goo_component_set_state_executing (component);

	GooEngine* engine = goo_engine_new (component, infile, outfile);

	goo_engine_play (engine);

	goo_component_set_state_idle (component);
	goo_component_set_state_loaded (component);

	g_object_unref (G_OBJECT (engine));

	return;
}

START_TEST (test_mp3dec_1)
{
	process ("/omx/patterns/hecommon_Stereo_44Khz.mp3",
		 "/tmp/hecommon_Stereo_44Khz.pcm",
		 FALSE, 0, 16, 2, 44100);

	return;
}
END_TEST

START_TEST (test_mp3dec_2)
{
	process ("/omx/patterns/16_Stereo_160_16.mp3",
		 "/tmp/16_Stereo_160_16.pcm",
		 FALSE, 0, 16, 2, 32000);

	return;
}
END_TEST

START_TEST (test_mp3dec_3)
{
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

	process ("/omx/patterns/MJ32khz128kbps_Stereo.mp3", NULL,
		 TRUE, 100, 16, 2, 32000);

	return;
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_mp3dec)
{
	tcase_add_test (tc_mp3dec, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_mp3dec = tcase_create ("Mp3Decoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_mp3dec_1);
	g_hash_table_insert (ht, "SR2", test_mp3dec_2);
	g_hash_table_insert (ht, "SR3", test_mp3dec_3);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_mp3dec);
	}
	else
	{
		tcase_add_test (tc_mp3dec, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_mp3dec, setup, teardown);
	tcase_set_timeout (tc_mp3dec, 0);
	suite_add_tcase (s, tc_mp3dec);

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

	ctx = g_option_context_new ("- AAC Decoder Tests.");
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
