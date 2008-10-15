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

#include <goo-ti-nbamrenc.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>
#include <string.h>

GooComponentFactory* factory;
GooComponent* component;

void
setup (void)
{
        factory = goo_ti_component_factory_get_instance ();
        component = goo_component_factory_get_component
		(factory, GOO_TI_NBAMR_ENCODER);

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
process (gchar* infile, OMX_AUDIO_AMRBANDMODETYPE band,
	 gboolean dtx, gboolean mime, guint frames)
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
		g_snprintf (outfile, 100, "/tmp/%s-%d%s_%s.nbamr", fn, band,
			    (mime) ? "mime" : "", (dtx) ? "dtxon" : "dtxoff");
		g_free (fn);
	}
	else
	{
		fail_unless (frames > 0, "unspecified number of frames");
		g_snprintf (outfile, 100, "/tmp/dasf-%d-%d%s_%s.nbamr",
			    band, frames, (mime) ? "mime" : "",
			    (dtx) ? "dtxon" : "dtxoff");
		dasf = TRUE;
	}

	/* input port */
	{
		GooIterator *iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		guint bufsiz = (dasf == TRUE) ? 0 :
			GOO_TI_NBAMRENC_INPUT_BUFFER_SIZE;

		GOO_PORT_GET_DEFINITION (port)->nBufferSize = bufsiz;

		g_object_unref (port);
		g_object_unref (iter);
	}

	/* output port */
	{
		/* noop */
	}

	/* parameter */
	{
		OMX_AUDIO_PARAM_AMRTYPE* param;
		param = GOO_TI_NBAMRENC_GET_OUTPUT_PORT_PARAM (component);

		if (mime == TRUE)
		{
			param->eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatFSF;
		}
		else
		{
			param->eAMRFrameFormat =
				OMX_AUDIO_AMRFrameFormatConformance;
		}

		if (dtx == TRUE)
		{
			param->eAMRDTXMode = OMX_AUDIO_AMRDTXModeOnAuto;
		}
		else
		{
			param->eAMRDTXMode = OMX_AUDIO_AMRDTXModeOff;
		}

		param->eAMRBandMode = band;
	}

	if (dasf == TRUE)
	{
		g_object_set (G_OBJECT (component),
			      "dasf-mode", TRUE,
			      "data-path", 0, NULL);
	}

	GooEngine* engine = goo_engine_new (component, infile, outfile);

	if (dasf == TRUE)
	{
		g_object_set (engine, "num-buffers", frames, NULL);
	}

	if (mime == TRUE && engine->outstream != NULL)
	{
		fwrite ("#!AMR", 6, 1, engine->outstream);
	}

	goo_component_set_state_idle (component);
	goo_component_set_state_executing (component);

	goo_engine_play (engine);

	goo_component_set_state_idle (component);
	goo_component_set_state_loaded (component);

	g_object_unref (engine);

	return;
}

START_TEST (test_nbamrenc_1)
{
	process ("/omx/patterns/T00.inp", OMX_AUDIO_AMRBandModeNB7,
		 FALSE, TRUE, 0);
}
END_TEST

START_TEST (test_nbamrenc_2)
{
	process ("/omx/patterns/T00.inp", OMX_AUDIO_AMRBandModeNB4,
		 FALSE, TRUE, 0);
}
END_TEST

START_TEST (test_nbamrenc_3)
{
	process ("/omx/patterns/T00.inp", OMX_AUDIO_AMRBandModeNB1,
		 FALSE, TRUE, 0);
}
END_TEST


START_TEST (test_nbamrenc_4)
{
	process (NULL, OMX_AUDIO_AMRBandModeNB7, FALSE, TRUE, 500);
}
END_TEST


void
fill_tcase (gchar* srd, gpointer func, TCase* tc_nbamrenc)
{
	tcase_add_test (tc_nbamrenc, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_nbamrenc = tcase_create ("NbAmrEncoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_nbamrenc_1);
	g_hash_table_insert (ht, "SR2", test_nbamrenc_2);
	g_hash_table_insert (ht, "SR3", test_nbamrenc_3);
	g_hash_table_insert (ht, "SR4", test_nbamrenc_4);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_nbamrenc);
	}
	else
	{
		tcase_add_test (tc_nbamrenc, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_nbamrenc, setup, teardown);
	tcase_set_timeout (tc_nbamrenc, 0);
	suite_add_tcase (s, tc_nbamrenc);

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

	ctx = g_option_context_new ("- NbAmr Encoder Tests.");
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
