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

#include <goo-ti-aacenc.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>
#include <string.h>

GooComponentFactory* factory;
GooComponent* component;

static void
setup (void)
{
	factory = goo_ti_component_factory_get_instance ();
	component = goo_component_factory_get_component (factory,
							 GOO_TI_AAC_ENCODER);

	return;
}

static void
teardown (void)
{
	g_object_unref (component);
	g_object_unref (factory);

	return;
}

static gchar*
get_filedesc (guint channels, guint samplerate, guint bitrate,
	      OMX_AUDIO_AACPROFILETYPE profile,
	      OMX_AUDIO_AACSTREAMFORMATTYPE format,
	      GooTiAacEncBitRateMode bitratemode)
{
	gchar* channels_name = NULL;
	if (channels == 1)
	{
		channels_name = "MONO";
	}
	else if (channels == 2)
	{
		channels_name = "STEREO";
	}

	gchar* profile_name = NULL;
	if (profile == OMX_AUDIO_AACObjectHE)
	{
		profile_name = "HE";
	}
	else if (profile == OMX_AUDIO_AACObjectHE_PS)
	{
		profile_name = "HEPS";
	}
	else
	{
		profile_name = "";
	}

	gchar* format_name = NULL;
	if (format == OMX_AUDIO_AACStreamFormatRAW)
	{
		format_name = "raw";
	}
	else if (format == OMX_AUDIO_AACStreamFormatADIF)
	{
		format_name = "adif";
	}
	else if (format == OMX_AUDIO_AACStreamFormatMP4ADTS)
	{
		format_name = "adts";
	}

	gchar* brm_name = NULL;
	if (bitratemode == GOO_TI_AACENC_BR_CBR)
	{
		brm_name = "cbr";
	}
	else
	{
		brm_name = "vbr";
	}

	gchar *desc = g_new0 (gchar, 50);

	g_snprintf (desc, 49, "%s_%dKHz_%dKbps_%s_%s_%s",
		    channels_name, samplerate / 1000, bitrate / 1000,
		    profile_name, brm_name, format_name);

	return desc;
}

static void
process (gchar* infile, guint channels, guint samplerate, guint bitrate,
	 OMX_AUDIO_AACPROFILETYPE profile,
	 OMX_AUDIO_AACSTREAMFORMATTYPE format,
	 GooTiAacEncBitRateMode bitratemode, guint frames)
{
	gchar outfile[100] = "\0";
	gboolean dasf = FALSE;

	if (infile != NULL)
	{
		fail_unless (g_file_test (infile, G_FILE_TEST_IS_REGULAR),
			     "input file doesn't exist");

		gchar *fn, *fn1;

		fn = g_path_get_basename (infile);
		fn1 = strchr (fn, '.');
		fn1[0] = '\0';
		gchar* desc = get_filedesc (channels, samplerate, bitrate,
					    profile, format, bitratemode);
		g_snprintf (outfile, 100, "/tmp/%s-%s.aac", fn, desc);
		g_free (fn);
		g_free (desc);
	}
	else
	{
		fail_unless (frames > 0, "unspecified number of frames");
		gchar* desc = get_filedesc (channels, samplerate, bitrate,
					    profile, format, bitratemode);
		g_snprintf (outfile, 100, "/tmp/dasf-%s-%d.aac", desc, frames);
		dasf = TRUE;
		g_free (desc);
	}

	/* output params */
	{
		OMX_AUDIO_PARAM_AACPROFILETYPE* param = NULL;
		param = GOO_TI_AACENC_GET_OUTPUT_PORT_PARAM (component);

		param->nChannels = channels;
		param->nBitRate = bitrate;
		param->nSampleRate = samplerate;
		param->eAACProfile = profile;
		param->eAACStreamFormat = format;
	}

	if (dasf == TRUE)
	{
		g_object_set (component,
			      "dasf-mode", TRUE,
			      "data-path", DATAPATH_APPLICATION, NULL);
		g_object_set (component, "bitrate-mode", bitratemode, NULL);
	}

	GooEngine* engine = goo_engine_new (component, infile, outfile);

	if (dasf == TRUE)
	{
		g_object_set (engine, "num-buffers", frames, NULL);
	}

	goo_component_set_state_idle (component);
	goo_component_set_state_executing (component);

	goo_engine_play (engine);

	goo_component_set_state_idle (component);
	goo_component_set_state_loaded (component);

	g_object_unref (engine);

	return;
}

START_TEST (test_aacenc_1)
{
	printf ("test_aacenc_1\n");
	process ("/omx/patterns/sbc_test_01_48k_16_m.pcm",
		1, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_2)
{
	printf ("test_aacenc_2\n");
	process (NULL, 1, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 30);
}
END_TEST

START_TEST (test_aacenc_3)
{
	printf ("test_aacenc_3\n");
	process (NULL, 1, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 15);
}
END_TEST

START_TEST (test_aacenc_4)
{
	printf ("test_aacenc_4\n");
	process ("/omx/patterns/sbc_test_07_16k_16_m.pcm",
		 1, 16000, 32000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_5)
{
	printf ("test_aacenc_5\n");
	process ("/omx/patterns/sbc_test_02_48k_16_s.pcm",
		 2, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_6)
{
	printf ("test_aacenc_6\n");
	process ("/omx/patterns/Blip_11k_16_m.pcm",
		 1, 11025, 40000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_7)
{
	printf ("test_aacenc_7\n");
	process ("/omx/patterns/chimes_22k_16_s.pcm",
		 2, 22050, 56000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_8)
{
	printf ("test_aacenc_8\n");
	process ("/omx/patterns/BONJOVI_11_2.pcm",
		 2, 11025, 48000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_9)
{
	printf ("test_aacenc_9\n");
	process ("/omx/patterns/BONJOVI_16_2.pcm",
		 2, 16000, 40000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_10)
{
	printf ("test_aacenc_10\n");
	process ("/omx/patterns/BONJOVI_32_2.pcm",
		 2, 32000, 112000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_11)
{
	printf ("test_aacenc_11\n");
	process ("/omx/patterns/BONJOVI_32_2.pcm",
		 2, 32000, 96000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_12)
{
	printf ("test_aacenc_12\n");
	process ("/omx/patterns/loveboat_8k_16_m.pcm",
		 1, 8000, 8000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_13)
{
	printf ("test_aacenc_13\n");
	process ("/omx/patterns/sbc_test_01_48k_16_m.pcm",
		 1, 48000, 48000, OMX_AUDIO_AACObjectHE,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_14)
{
	printf ("test_aacenc_14\n");
	process (NULL, 1, 44100, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 60);
}
END_TEST

START_TEST (test_aacenc_15)
{
	printf ("test_aacenc_15\n");
	process (NULL, 1, 16000, 56000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 30);
}
END_TEST

START_TEST (test_aacenc_16)
{
	printf ("test_aacenc_16\n");
	process (NULL, 1, 16000, 48000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 15);
}
END_TEST

START_TEST (test_aacenc_17)
{
	printf ("test_aacenc_17\n");
	process (NULL, 1, 16000, 8000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 7);
}
END_TEST

START_TEST (test_aacenc_18)
{
	printf ("test_aacenc_18\n");
	process ("/omx/patterns/sbc_test_01_48k_16_m.pcm",
		 1, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatMP4ADTS, GOO_TI_AACENC_BR_VBR5, 0);
}
END_TEST

START_TEST (test_aacenc_19)
{
	printf ("test_aacenc_19\n");
	process ("/omx/patterns/sbc_test_01_48k_16_m.pcm",
		 1, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatRAW, GOO_TI_AACENC_BR_CBR, 0);
}
END_TEST

START_TEST (test_aacenc_20)
{
	printf ("test_aacenc_20\n");
	process (NULL, 1, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 30);
}
END_TEST

START_TEST (test_aacenc_21)
{
	printf ("test_aacenc_21\n");
	process (NULL, 1, 48000, 48000, OMX_AUDIO_AACObjectHE,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 100);
}
END_TEST

START_TEST (test_aacenc_22)
{
	printf ("test_aacenc_22\n");
	process (NULL, 1, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 7);
}
END_TEST

START_TEST (test_aacenc_23)
{
	printf ("test_aacenc_23\n");
	process (NULL, 1, 60000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 30);
}
END_TEST

/*
START_TEST (test_aacenc_24)
{
	printf ("test_aacenc_24\n");
	process (NULL, 1, 48000, 128000, OMX_AUDIO_AACObjectHE_PS,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 30);
}
END_TEST
*/

START_TEST (test_aacenc_25)
{
	printf ("test_aacenc_25\n");
	process (NULL, 1, 60000, 100000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 30);
}
END_TEST

START_TEST (test_aacenc_26)
{
	printf ("test_aacenc_26\n");
	process (NULL, 1, 48000, 100000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatADIF, GOO_TI_AACENC_BR_CBR, 30);
}
END_TEST

START_TEST (test_aacenc_27)
{
	printf ("test_aacenc_27\n");
	process (NULL, 1, 48000, 128000, OMX_AUDIO_AACObjectLC,
		 OMX_AUDIO_AACStreamFormatMP4ADTS, GOO_TI_AACENC_BR_CBR, 60);
}
END_TEST


void
fill_tcase (gchar* srd, gpointer func, TCase* tc_aacenc)
{
	tcase_add_test (tc_aacenc, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_aacenc = tcase_create ("AacEncoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_aacenc_1);
	g_hash_table_insert (ht, "SR2", test_aacenc_2);
	g_hash_table_insert (ht, "SR3", test_aacenc_3);
	g_hash_table_insert (ht, "SR4", test_aacenc_4);
	g_hash_table_insert (ht, "SR5", test_aacenc_5);
	g_hash_table_insert (ht, "SR6", test_aacenc_6);
	g_hash_table_insert (ht, "SR7", test_aacenc_7);
	g_hash_table_insert (ht, "SR8", test_aacenc_8);
	g_hash_table_insert (ht, "SR9", test_aacenc_9);
	g_hash_table_insert (ht, "SR10", test_aacenc_10);
	g_hash_table_insert (ht, "SR11", test_aacenc_11);
	g_hash_table_insert (ht, "SR12", test_aacenc_12);
	g_hash_table_insert (ht, "SR13", test_aacenc_13);
	g_hash_table_insert (ht, "SR14", test_aacenc_14);
	g_hash_table_insert (ht, "SR15", test_aacenc_15);
	g_hash_table_insert (ht, "SR16", test_aacenc_16);
	g_hash_table_insert (ht, "SR17", test_aacenc_17);
	g_hash_table_insert (ht, "SR18", test_aacenc_18);
	g_hash_table_insert (ht, "SR19", test_aacenc_19);
	g_hash_table_insert (ht, "SR20", test_aacenc_20);
	g_hash_table_insert (ht, "SR21", test_aacenc_21);
	g_hash_table_insert (ht, "SR22", test_aacenc_22);
	g_hash_table_insert (ht, "SR23", test_aacenc_23);
	/* g_hash_table_insert (ht, "SR24", test_aacenc_24); OMAPS00139232 */
	g_hash_table_insert (ht, "SR25", test_aacenc_25);
	g_hash_table_insert (ht, "SR26", test_aacenc_26);
	g_hash_table_insert (ht, "SR27", test_aacenc_27);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_aacenc);
	}
	else
	{
		tcase_add_test (tc_aacenc, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_aacenc, setup, teardown);
	tcase_set_timeout (tc_aacenc, 0);
	suite_add_tcase (s, tc_aacenc);

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
		  "Test option: (SR1/SR2/../SR27)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- AAC Encoder Tests.");
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
