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

#include <goo-ti-gsmhrenc.h>
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
		(factory, GOO_TI_GSMHR_ENCODER);

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
process (gchar* infile, gboolean dtx, gboolean bHiPassFilter, guint frames)
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
		g_snprintf (outfile, 100, "/tmp/%s-%s%s.gsmhr", fn,
			    (dtx) ? "dtxon" : "dtxoff",
			    (bHiPassFilter) ? "_HiPass" : "");
		g_free (fn);
	}
	else
	{
		fail_unless (frames > 0, "unspecified number of frames");
		g_snprintf (outfile, 100, "/tmp/dasf-%s%s.cod",
			    (dtx) ? "dtxon" : "dtxoff",
			    (bHiPassFilter) ? "_HiPass" : "");
		dasf = TRUE;
	}

	/* input port */
	{
		GooIterator *iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		GOO_PORT_GET_DEFINITION (port)->nBufferSize =
			GOO_TI_GSMHRENC_INPUT_BUFFER_SIZE;

		g_object_unref (port);
		g_object_unref (iter);
	}

	/* output port */
	{
		/* noop */
	}

	/* parameter */
	{
		OMX_AUDIO_PARAM_GSMHRTYPE* param;
		param = GOO_TI_GSMHRENC_GET_OUTPUT_PORT_PARAM (component);

		if (dtx == TRUE)
		{
			param->bDTX = OMX_TRUE;
		}
		else
		{
			param->bDTX = OMX_FALSE;
		}

		if (bHiPassFilter == TRUE)
		{
			param->bHiPassFilter = OMX_TRUE;
		}
		else
		{
			param->bHiPassFilter = OMX_FALSE;
		}

	}

	if (dasf == TRUE)
	{
		g_object_set (G_OBJECT (component),
			      "dasf-mode", TRUE,
			      "data-path", 0, NULL);
	}
	else
	{
		g_object_set (G_OBJECT (component),
			      "dasf-mode", FALSE,
			      "data-path", 0, NULL);
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

START_TEST (test_gsmhrenc_1)
{
	process ("/omx/patterns/T00.pcm",
		TRUE, FALSE, 0);
}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_gsmhrenc)
{
	tcase_add_test (tc_gsmhrenc, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_gsmhrenc = tcase_create ("GsmHrEncoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_gsmhrenc_1);
	//g_hash_table_insert (ht, "SR2", test_gsmhrenc_2);
	//g_hash_table_insert (ht, "SR3", test_gsmhrenc_3);
	//g_hash_table_insert (ht, "SR4", test_gsmhrenc_4);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_gsmhrenc);
	}
	else
	{
		tcase_add_test (tc_gsmhrenc, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_gsmhrenc, setup, teardown);
	tcase_set_timeout (tc_gsmhrenc, 0);
	suite_add_tcase (s, tc_gsmhrenc);

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

	ctx = g_option_context_new ("- GsmHr Encoder Tests.");
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
