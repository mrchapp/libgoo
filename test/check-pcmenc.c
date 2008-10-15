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

#include <goo-ti-pcmenc.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>

START_TEST (test_pcmenc_1)
{
  GooComponentFactory *factory = goo_ti_component_factory_get_instance ();
  GooComponent *component = goo_component_factory_get_component (factory,
								 GOO_TI_PCM_ENCODER);
  /* audio properties */
  g_object_set (G_OBJECT (component),
				"dasf-mode", FALSE,
				"data-path", NULL,
				NULL);

  {
    GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nPortIndex = 0;
    GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nChannels = 1;
  }

  /* output port component parameters */
  {
    GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nPortIndex = 1;
    GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nChannels = 1;
  }
  GOO_OBJECT_WARNING (component, "stream ID = %d",
		      goo_ti_audio_component_get_stream_id
		      (GOO_TI_AUDIO_COMPONENT (component)));

  GooEngine *engine = goo_engine_new (component,
				      "/omx/patterns/m08.wav",
				      "/tmp/m_8000.pcm");

  goo_component_set_state_idle (component);

  goo_component_set_state_executing (component);

  goo_engine_play (engine);

  goo_component_set_state_idle (component);
  goo_component_set_state_loaded (component);

  g_object_unref (G_OBJECT (engine));
  g_object_unref (G_OBJECT (component));
  g_object_unref (G_OBJECT (factory));

}

END_TEST

START_TEST (test_pcmenc_2)
{
  GooComponentFactory *factory = goo_ti_component_factory_get_instance ();
  GooComponent *component = goo_component_factory_get_component (factory,
								 GOO_TI_PCM_ENCODER);
  /* audio properties */
  g_object_set (G_OBJECT (component),
				"dasf-mode", FALSE,
				"data-path", NULL,
				NULL);

  {
    GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nPortIndex = 0;
    GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nChannels = 1;
  }

  /* output port component parameters */
  {
    GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nPortIndex = 1;
    GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nChannels = 1;
  }
  GOO_OBJECT_WARNING (component, "stream ID = %d",
		      goo_ti_audio_component_get_stream_id
		      (GOO_TI_AUDIO_COMPONENT (component)));

  GooEngine *engine = goo_engine_new (component,
				      "/omx/patterns/m16.wav",
				      "/tmp/m_16000.pcm");

  goo_component_set_state_idle (component);

  goo_component_set_state_executing (component);

  goo_engine_play (engine);

  goo_component_set_state_idle (component);
  goo_component_set_state_loaded (component);

  g_object_unref (G_OBJECT (engine));
  g_object_unref (G_OBJECT (component));
  g_object_unref (G_OBJECT (factory));

}

END_TEST

START_TEST (test_pcmenc_3)
{
  GooComponentFactory *factory = goo_ti_component_factory_get_instance ();
  GooComponent *component = goo_component_factory_get_component (factory,
								 GOO_TI_PCM_ENCODER);
  /* audio properties */
  g_object_set (G_OBJECT (component),
				"dasf-mode", FALSE,
				"data-path", NULL,
				NULL);

  {
    GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nPortIndex = 0;
    GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nChannels = 1;
  }

  /* output port component parameters */
  {
    GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nPortIndex = 1;
    GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nChannels = 1;
  }
  GOO_OBJECT_WARNING (component, "stream ID = %d",
		      goo_ti_audio_component_get_stream_id
		      (GOO_TI_AUDIO_COMPONENT (component)));

  GooEngine *engine = goo_engine_new (component,
				      "/omx/patterns/m32.wav",
				      "/tmp/m_32000.pcm");

  goo_component_set_state_idle (component);

  goo_component_set_state_executing (component);

  goo_engine_play (engine);

  goo_component_set_state_idle (component);
  goo_component_set_state_loaded (component);

  g_object_unref (G_OBJECT (engine));
  g_object_unref (G_OBJECT (component));
  g_object_unref (G_OBJECT (factory));

}

END_TEST

START_TEST (test_pcmenc_4)
{
  GooComponentFactory *factory = goo_ti_component_factory_get_instance ();
  GooComponent *component = goo_component_factory_get_component (factory,
								 GOO_TI_PCM_ENCODER);
  /* audio properties */
  g_object_set (G_OBJECT (component),
				"dasf-mode", FALSE,
				"data-path", NULL,
				NULL);

  {
    GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nPortIndex = 0;
    GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nChannels = 1;
  }

  /* output port component parameters */
  {
    GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nPortIndex = 1;
    GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nChannels = 1;
  }
  GOO_OBJECT_WARNING (component, "stream ID = %d",
		      goo_ti_audio_component_get_stream_id
		      (GOO_TI_AUDIO_COMPONENT (component)));

  GooEngine *engine = goo_engine_new (component,
				      "/omx/patterns/m44.wav",
				      "/tmp/m_44100.pcm");

  goo_component_set_state_idle (component);

  goo_component_set_state_executing (component);

  goo_engine_play (engine);

  goo_component_set_state_idle (component);
  goo_component_set_state_loaded (component);

  g_object_unref (G_OBJECT (engine));
  g_object_unref (G_OBJECT (component));
  g_object_unref (G_OBJECT (factory));

}

END_TEST

#if 0
START_TEST (test_pcmenc_2)
{
  GooComponentFactory *factory = goo_ti_component_factory_get_instance ();
  GooComponent *component = goo_component_factory_get_component (factory,
								 GOO_TI_PCM_ENCODER);
  /* audio properties */
  g_object_set (G_OBJECT (component),
				"dasf-mode", TRUE,
				"data-path", NULL,
				"mute", FALSE,
				NULL);

  {
	GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nPortIndex = OMX_DirInput;
	GOO_TI_PCMENC_GET_INPUT_PORT_PARAM (component)->nChannels = 1;
  }

  /* output port component parameters */
  {
	GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nPortIndex = OMX_DirOutput;
	GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nChannels = 1;
	GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component)->nSamplingRate = 8000;
  }
  GOO_OBJECT_WARNING (component, "stream ID = %d",
		      goo_ti_audio_component_get_stream_id
		      (GOO_TI_AUDIO_COMPONENT (component)));

  GooEngine *engine = goo_engine_new (component,
				      NULL,
				      "/tmp/check_goo_8000.pcm");

	g_object_set (engine, "num-buffers", 500, NULL);

  goo_component_set_state_idle (component);

  goo_component_set_state_executing (component);

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
fill_tcase (gchar* srd, gpointer func, TCase* tc_pcmenc)
{
	tcase_add_test (tc_pcmenc, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_pcmenc = tcase_create ("PcmEncoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_pcmenc_1);
	g_hash_table_insert (ht, "SR2", test_pcmenc_2);
	g_hash_table_insert (ht, "SR3", test_pcmenc_3);
	g_hash_table_insert (ht, "SR4", test_pcmenc_4);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_pcmenc);
	}
	else
	{
		tcase_add_test (tc_pcmenc, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_set_timeout (tc_pcmenc, 0);
	suite_add_tcase (s, tc_pcmenc);

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

	ctx = g_option_context_new ("- PCM Encoder Tests.");
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
