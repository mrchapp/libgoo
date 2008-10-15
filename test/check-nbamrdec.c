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

#include <goo-ti-nbamrdec.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>

START_TEST (test_nbamrdec_1)
{
  GooComponentFactory *factory = goo_ti_component_factory_get_instance ();
  GooComponent *component = goo_component_factory_get_component (factory,

								 GOO_TI_NBAMR_DECODER);

  /* audio properties */
  g_object_set (G_OBJECT (component),
		"dasf-mode", TRUE, "mute", FALSE, "volume", 90, NULL);

  /* input port component parameters */
  {
    GOO_TI_NBAMRDEC_GET_INPUT_PORT_PARAM (component)->nBitRate = 8000;
    GOO_TI_NBAMRDEC_GET_INPUT_PORT_PARAM (component)->eAMRBandMode = 1;
    GOO_TI_NBAMRDEC_GET_INPUT_PORT_PARAM (component)->eAMRDTXMode =
      OMX_AUDIO_AMRDTXModeOff;
    GOO_TI_NBAMRDEC_GET_INPUT_PORT_PARAM (component)->eAMRFrameFormat =
      OMX_AUDIO_AMRFrameFormatFSF;
  }

  /* output port component parameters */
  {
    GOO_TI_NBAMRDEC_GET_OUTPUT_PORT_PARAM (component)->nChannels = 1;
    GOO_TI_NBAMRDEC_GET_OUTPUT_PORT_PARAM (component)->nBitRate = 8000;
  }

  GOO_OBJECT_WARNING (component, "streamID = %d",
		      goo_ti_audio_component_get_stream_id
		      (GOO_TI_AUDIO_COMPONENT (component)));

//      g_printf ("goo_component_set_state_idle\n");
  goo_component_set_state_idle (component);

//      g_printf ("goo_component_set_state_executing\n");
  goo_component_set_state_executing (component);

  GooEngine *engine = goo_engine_new (component,

				      "/omx/patterns/8khz_4.75kbps.amr",

				      "/tmp/test_mime.pcm");

//      g_printf ("goo_engine\n");
  goo_engine_play (engine);

//      g_printf ("goo_component_set_state_idle\n");
  goo_component_set_state_idle (component);

//      g_printf ("goo_component_set_state_loaded\n");
  goo_component_set_state_loaded (component);

//      g_printf ("g_object_unref->engine\n");
  g_object_unref (G_OBJECT (engine));
//      g_printf ("g_object_unref->component\n");
  g_object_unref (G_OBJECT (component));
//      g_printf ("g_object_unref->factory\n");
  g_object_unref (G_OBJECT (factory));

}
END_TEST

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_nbamrdec)
{
	tcase_add_test (tc_nbamrdec, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_nbamrdec = tcase_create ("NbAmrDecoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_nbamrdec_1);
/*	g_hash_table_insert (ht, "SR2", test_nbamrdec_2);
	g_hash_table_insert (ht, "SR3", test_nbamrdec_3);
*/
	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_nbamrdec);
	}
	else
	{
		tcase_add_test (tc_nbamrdec, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_set_timeout (tc_nbamrdec, 0);
	suite_add_tcase (s, tc_nbamrdec);

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

	ctx = g_option_context_new ("- NbAmr Decoder Tests.");
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
//      g_printf ("srunner_create\n");
  SRunner *sr = srunner_create (s);
//      g_printf ("srunner_run_all\n");
  srunner_run_all (sr, CK_NORMAL);
//      g_printf ("number_failed\n");
  number_failed = srunner_ntests_failed (sr);
//      g_printf ("srunner_free\n");
  srunner_free (sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
