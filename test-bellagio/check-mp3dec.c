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

#include <goo-bellagio-mp3dec.h>
#include <goo-bellagio-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>
#include <string.h>

#define PREFIX "/data/EVM_filesystems/ceyusa/target"

GooComponentFactory* factory;
GooComponent* component;
GRand* rnd;

void
setup (void)
{
        factory = goo_bellagio_component_factory_get_instance ();
        component = goo_component_factory_get_component
		(factory, GOO_BELLAGIO_MP3_DECODER);

        rnd = g_rand_new_with_seed (time (0));

        return;
}

void
teardown (void)
{
	GOO_DEBUG ("unref = %d -> %d",
		   GOO_OBJECT_REFCOUNT (component),
		   GOO_OBJECT_REFCOUNT (component) - 1);

        g_object_unref (component);
        g_object_unref (factory);
        g_rand_free (rnd);

        return;
}

void
process (guint channels, guint samplingrate, gchar* filename)
{
	gchar outfile[100];

	fail_unless (filename != NULL, "unspecified filename in test");

	{
		gchar *fn, *fn1;

		fn = g_path_get_basename (filename);
		fn1 = strchr (fn, '.');
		fn1[0] = '\0';
		g_snprintf (outfile, 100, "/tmp/%s.pcm", fn);
		g_free (fn);
	}

        GooEngine* engine = goo_engine_new (component, filename, outfile);

	/* g_object_set (engine, "eosevent", TRUE, NULL); */

	goo_component_set_state_idle (component);
        goo_component_set_state_executing (component);

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (engine);

	return;
}


START_TEST (test_mp3dec_1)
{
	process (2, 44100, PREFIX "/omx/patterns/hecommon_Stereo_44Khz.mp3");
	return;
}
END_TEST

START_TEST (test_mp3dec_2)
{
	process (2, 32000, PREFIX "/omx/patterns/16_Stereo_160_16.mp3");
	return;
}
END_TEST

Suite *
goo_suite (void)
{
        Suite *s = suite_create ("Goo");

        TCase *tc_mp3dec = tcase_create ("Mp3Decoder");
        tcase_add_test (tc_mp3dec, test_mp3dec_1);
        tcase_add_test (tc_mp3dec, test_mp3dec_2);
        tcase_add_checked_fixture (tc_mp3dec, setup, teardown);
        tcase_set_timeout (tc_mp3dec, 0);
        suite_add_tcase (s, tc_mp3dec);

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
