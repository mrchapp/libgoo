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

#include <goo-ti-g711dec.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>

START_TEST (test_g711dec_1)
{
        GooComponentFactory* factory =
                goo_ti_component_factory_get_instance ();
        GooComponent* component =
                goo_component_factory_get_component (factory,
                                                     GOO_TI_G711_DECODER);
        /* audio properties */

        g_object_set (G_OBJECT (component),
                      "dasf-mode", FALSE,
                      NULL);

        /* input component parameters */
        {
                GOO_TI_G711DEC_GET_INPUT_PARAM (component)->nChannels = 1;
                GOO_TI_G711DEC_GET_INPUT_PARAM (component)->ePCMMode = OMX_AUDIO_PCMModeALaw;
        }

        /* output component parameters */
        {
                GOO_TI_G711DEC_GET_OUTPUT_PARAM (component)->nChannels = 1;
                GOO_TI_G711DEC_GET_OUTPUT_PARAM (component)->nSamplingRate = 8000;
        }
        GOO_OBJECT_WARNING (component, "stream ID = %d",
                            goo_ti_audio_component_get_stream_id
                            (GOO_TI_AUDIO_COMPONENT (component)));

        goo_component_set_state_idle (component);

        goo_component_set_state_executing (component);

        GooEngine* engine = goo_engine_new (
                component,
                "/omx/patterns/mono_8000Hz_80_alaw.bit",
                "/tmp/mono_8000Hz_80_alaw.pcm"
                );

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (component));
        g_object_unref (G_OBJECT (factory));

}
END_TEST

START_TEST (test_g711dec_2)
{
        GooComponentFactory* factory =
                goo_ti_component_factory_get_instance ();
        GooComponent* component =
                goo_component_factory_get_component (factory,
                                                     GOO_TI_G711_DECODER);
        /* audio properties */

        g_object_set (G_OBJECT (component),
                      "dasf-mode", TRUE,
                      "data-path", NULL,
                      "mute", FALSE,
                      "volume", 100,
                      NULL);

        /* input component parameters */
        {
                GOO_TI_G711DEC_GET_INPUT_PARAM (component)->nChannels = 1;
                GOO_TI_G711DEC_GET_INPUT_PARAM (component)->ePCMMode = OMX_AUDIO_PCMModeALaw;
        }

        /* input component parameters */
        {
                GOO_TI_G711DEC_GET_OUTPUT_PARAM (component)->nChannels = 1;
                GOO_TI_G711DEC_GET_OUTPUT_PARAM (component)->nSamplingRate = 8000;
                GOO_TI_G711DEC_GET_OUTPUT_PARAM (component)->ePCMMode = OMX_AUDIO_PCMModeLinear;
        }
        GOO_OBJECT_WARNING (component, "stream ID = %d",
                            goo_ti_audio_component_get_stream_id
                            (GOO_TI_AUDIO_COMPONENT (component)));

        goo_component_set_state_idle (component);

        goo_component_set_state_executing (component);

        GooEngine* engine = goo_engine_new (
                component,
                "/omx/patterns/mono_8000Hz_80_alaw.bit",
                NULL);

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (component));
        g_object_unref (G_OBJECT (factory));

}
END_TEST

START_TEST (test_g711dec_3)
{
        GooComponentFactory* factory =
                goo_ti_component_factory_get_instance ();
        GooComponent* component =
                goo_component_factory_get_component (factory,
                                                     GOO_TI_G711_DECODER);
        /* audio properties */

        g_object_set (G_OBJECT (component),
                      "dasf-mode", TRUE,
                      "data-path", NULL,
                      "mute", FALSE,
                      "volume", 100,
                      NULL);

        /* input component parameters */
        {
                GOO_TI_G711DEC_GET_INPUT_PARAM (component)->nChannels = 1;
                GOO_TI_G711DEC_GET_INPUT_PARAM (component)->ePCMMode = OMX_AUDIO_PCMModeMULaw;
        }

        /* input component parameters */
        {
                GOO_TI_G711DEC_GET_OUTPUT_PARAM (component)->nChannels = 1;
                GOO_TI_G711DEC_GET_OUTPUT_PARAM (component)->nSamplingRate = 8000;
                GOO_TI_G711DEC_GET_OUTPUT_PARAM (component)->ePCMMode = OMX_AUDIO_PCMModeLinear;
        }
        GOO_OBJECT_WARNING (component, "stream ID = %d",
                            goo_ti_audio_component_get_stream_id
                            (GOO_TI_AUDIO_COMPONENT (component)));

        goo_component_set_state_idle (component);

        goo_component_set_state_executing (component);

        GooEngine* engine = goo_engine_new (
                component,
                "/omx/patterns/mono_8000Hz_80_ulaw.bit",
                NULL);

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (G_OBJECT (engine));
        g_object_unref (G_OBJECT (component));
        g_object_unref (G_OBJECT (factory));

}
END_TEST

Suite *
goo_suite (void)
{
        Suite *s = suite_create ("Goo");

        TCase *tc_g711dec = tcase_create ("G711Decoder");
        tcase_add_test (tc_g711dec, test_g711dec_1);
        tcase_add_test (tc_g711dec, test_g711dec_2);
        tcase_add_test (tc_g711dec, test_g711dec_3);
        tcase_set_timeout (tc_g711dec, 0);
        suite_add_tcase (s, tc_g711dec);

        return s;
}

gint
main (void)
{
        int number_failed;

        g_type_init ();
        if (!g_thread_supported ())
        {
                g_thread_init (NULL);
        }

        Suite *s = goo_suite ();
        SRunner *sr = srunner_create (s);
        srunner_run_all (sr, CK_NORMAL);
        number_failed = srunner_ntests_failed (sr);
        srunner_free (sr);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
