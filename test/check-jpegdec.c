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

#include <goo-ti-jpegdec.h>
#include <goo-ti-component-factory.h>

#include <goo-engine.h>
#include <goo-utils.h>

#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define M_SOF0  0xC0            /* nStart Of Frame N */
#define M_SOF1  0xC1            /* N indicates which compression process */
#define M_SOF2  0xC2            /* Only SOF0-SOF2 are now in common use */
#define M_SOF3  0xC3
#define M_SOF5  0xC5            /* NB: codes C4 and CC are NOT SOF markers */
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8            /* nStart Of Image (beginning of datastream) */
#define M_EOI   0xD9            /* End Of Image (end of datastream) */
#define M_SOS   0xDA            /* nStart Of Scan (begins compressed data) */
#define M_JFIF  0xE0            /* Jfif marker */
#define M_EXIF  0xE1            /* Exif marker */
#define M_COM   0xFE            /* Comment  */
#define M_DQT   0xDB
#define M_DHT   0xC4
#define M_DRI   0xDD

#define MAX_SECTIONS 20

typedef struct {
        guint size, width, height;
	gint color;
        gboolean progressive;
} ImageInfo;

ImageInfo *iminfo;
GooComponentFactory* factory;
GooComponent* component;
GRand* rnd;

/* Convert a 16 bit unsigned value from file's native byte order */
static gint
get_16m (const guchar *data)
{
        return (((guchar *) data)[0] << 8) | ((guchar *) data)[1];
}

static gint
get_color_format (const guchar *data)
{
	guchar Nf;
	gint j, i, temp;
	gshort H[4], V[4];

	Nf = data[7];

	if (Nf != 3)
	{
		goto done;
	}

	for (j = 0; j < Nf; j++)
	{
		i = j * 3 + 7 + 2;
		/* H[j]: upper 4 bits of a byte, horizontal sampling factor. */
		/* V[j]: lower 4 bits of a byte, vertical sampling factor.   */
		H[j] = (0x0f & (data[i] >> 4));
		/* printf ("h[%d] = %x\t", j, H[j]);  */
		V[j] = (0x0f & data[i]);
		/* printf ("v[%d] = %x\n", j, V[j]);  */
	}

	temp = (V[0] * H[0]) / (V[1] * H[1]);
	/* printf ("temp = %x\n", temp); */

	if (temp == 4 && H[0] == 2)
	{
		return OMX_COLOR_FormatYUV420PackedPlanar;
	}

	if (temp == 4 && H[0] == 4)
	{
		return OMX_COLOR_FormatYUV411Planar;
	}

	if (temp == 2)
	{
		return OMX_COLOR_FormatCbYCrY; /* YUV422 interleaved, little endian */
	}

	if (temp == 1)
	{
		return OMX_COLOR_FormatYUV444Interleaved;
	}

done:
	return OMX_COLOR_FormatUnused;
}

/* Parse the marker stream until SOS or EOI is seen */
static gboolean
read_jpeg_sections (FILE *file)
{
        gint a = 0;
        gint sectionsread = 0;

        a = fgetc (file);
        if (a != 0xff || fgetc (file) != M_SOI)
        {
                return FALSE;
        }

        for (sectionsread = 0; sectionsread < MAX_SECTIONS - 1; )
        {
                gint itemlen;
                gint marker = 0;
                gint ll, lh, got;
                guchar *data = NULL;

		for (a = 0; a < 7; a++)
		{
			marker = fgetc (file);

			if (marker != 0xff)
			{
				break;
			}

			if (a >= 6)
			{
				/* too many padding bytes */
				if (G_LIKELY (data))
				{
					g_free (data);
					data = NULL;
				}
				return FALSE;
			}
		}

		if (marker == 0xff)
		{
			/* 0xff is legal padding, but if we get that many,
			   something's wrong. */
			return FALSE;
		}

		lh = fgetc (file);
		ll = fgetc (file);

		itemlen = (lh << 8) | ll;

		if (itemlen < 2)
		{
			/* Invalid marker */
			return FALSE;
		}

		data = g_new (guchar, itemlen);
		data[0] = (guchar) lh;
		data[1] = (guchar) ll;

		got = fread (data + 2, 1, itemlen - 2, file);
		if (got != itemlen - 2)
		{
			/* Premature end of file? */
			return FALSE;
		}

		sectionsread++;

		switch (marker)
		{
		case M_SOS:
			if (G_LIKELY (data))
			{
				g_free (data);
				data = NULL;
			}
			return TRUE;

		case M_EOI: /* No image in jpeg! */
			if (G_LIKELY (data))
			{
				g_free (data);
				data = NULL;
			}
			return FALSE;

		case M_COM:  /* Comment section */
		case M_JFIF: /* non exif image tag */
		case M_EXIF: /* exif image tag */
			break;

		case M_SOF2:
			iminfo->progressive = TRUE;

		case M_SOF0:
		case M_SOF1:
		case M_SOF3:
		case M_SOF5:
		case M_SOF6:
		case M_SOF7:
		case M_SOF9:
		case M_SOF10:
		case M_SOF11:
		case M_SOF13:
		case M_SOF14:
		case M_SOF15:
		{
/* 			int i; */
/* 			for (i = 0; i < itemlen; i++) */
/* 			{ */
/* 				printf ("%x-", data[i]); */
/* 			} */
/* 			printf ("\n"); */
			iminfo->height = get_16m (data + 3);
			iminfo->width = get_16m (data + 5);
			iminfo->color = get_color_format (data);
			break;
		}

		default:
			break;
		}

		if (G_LIKELY (data))
		{
			g_free (data);
			data = NULL;
		}
	}

	return FALSE;
}

static void
parse_jpeg (gchar* filename)
{
	FILE* stream;
	struct stat buf;

	stream = fopen (filename, "r");
	fail_unless (stream != NULL, "can't open jpeg file");

	fstat (fileno (stream), &buf);
	iminfo->size = buf.st_size;

	fail_unless (read_jpeg_sections (stream) == TRUE, "isn't a jpeg file");

	fclose (stream);

	return;
}

void
setup (void)
{
        iminfo = g_new0 (ImageInfo, 1);
        factory = goo_ti_component_factory_get_instance ();
        component = goo_component_factory_get_component (factory,
                                                         GOO_TI_JPEG_DECODER);

        rnd = g_rand_new_with_seed (time (0));

        return;
}

void
teardown (void)
{
        g_object_unref (component);
        g_object_unref (factory);
        g_rand_free (rnd);
        g_free (iminfo);

        return;
}

void
process (gchar* filename)
{
	fail_unless (filename != NULL, "unspecified filename in test");

	gchar outfile[100];
        gchar *fn, *fn1;

	parse_jpeg (filename);

	GOO_INFO ("size = %d", iminfo->size);
	GOO_INFO ("width = %d", iminfo->width);
	GOO_INFO ("height = %d", iminfo->height);
	GOO_INFO ("progressive = %d", iminfo->progressive);
	GOO_INFO ("color = %s", goo_strcolor (iminfo->color));

	/* param */
	{
		gint dice = g_rand_int_range (rnd, 1, 5);

		switch (dice)
		{
		case 1:
			g_object_set (component, "scale", 100, NULL);
			break;
		case 2:
			g_object_set (component, "scale", 50, NULL);
			break;
		case 3:
			g_object_set (component, "scale", 25, NULL);
			break;
		case 4:
			g_object_set (component, "scale", 12, NULL);
			break;
		}

		GOO_INFO ("resize: %d", GOO_TI_JPEGDEC (component)->scale);
	}

	{
		fn = g_path_get_basename (filename);
		fn1 = strchr (fn, '.');
		fn1[0] = '\0';
		g_snprintf (outfile, 100, "/tmp/%s-%d.yuv", fn,
			    GOO_TI_JPEGDEC (component)->scale);
		g_free (fn);
	}


	/* inport */
	{
		GooPort* port = goo_component_get_port (component, "input0");
                g_assert (port != NULL);

                OMX_PARAM_PORTDEFINITIONTYPE* param;
                param = GOO_PORT_GET_DEFINITION (port);
                param->format.image.nFrameWidth = iminfo->width;
                param->format.image.nFrameHeight = iminfo->height;
                param->format.image.eColorFormat = iminfo->color;
		param->nBufferSize = iminfo->size;

                g_object_unref (port);
	}

	/* outport */
	{
		GooPort* port = goo_component_get_port (component, "output0");
		g_assert (port != NULL);

                OMX_PARAM_PORTDEFINITIONTYPE* param;
                param = GOO_PORT_GET_DEFINITION (port);

		if (iminfo->color == OMX_COLOR_FormatYUV444Interleaved)
		{
			param->format.image.eColorFormat =
				OMX_COLOR_FormatCbYCrY;
			goto outport_done;
		}

		guint dice = g_rand_int_range (rnd, 1, 3);

		switch (dice)
		{
		case 1:
			param->format.image.eColorFormat =
				OMX_COLOR_FormatYUV420PackedPlanar;
			break;
		case 2:
			param->format.image.eColorFormat =
				OMX_COLOR_FormatCbYCrY;
			break;
		}

	outport_done:
                g_object_unref (port);
	}

	GooEngine* engine = goo_engine_new (
		component,
		filename,
		outfile
		);

	g_object_set (component, "progressive", iminfo->progressive, NULL);

	goo_component_set_state_idle (component);

        goo_component_set_state_executing (component);

        goo_engine_play (engine);

        goo_component_set_state_idle (component);
        goo_component_set_state_loaded (component);

        g_object_unref (engine);

	return;
}

START_TEST (test_jpegdec_0)
{
	process ("/omx/patterns/4MP_internet_seq.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_1)
{
	process ("/omx/patterns/penguin_UXGA.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_2)
{
	process ("/omx/patterns/stone_1600x1200.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_3)
{
	process ("/omx/patterns/5MP_internet_seq.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_4)
{
	process ("/omx/patterns/6MP_internet_seq.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_5)
{
	process ("/omx/patterns/stockholm.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_6)
{
	process ("/omx/patterns/wind_1280x960.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_7)
{
	process ("/omx/patterns/flower.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_8)
{
	process ("/omx/patterns/img02_1022x765_422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_9)
{
	process ("/omx/patterns/64K_Exif.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_10)
{
	process ("/omx/patterns/lamp_1024x1280.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_11)
{
	process ("/omx/patterns/img02_899x701_444.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_12)
{
	process ("/omx/patterns/puu_2400x1800_Prog.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_13)
{
	process ("/omx/patterns/img01_2400x1800_420.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_14)
{
	process ("/omx/patterns/img04_1001x651_444.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_15)
{
	process ("/omx/patterns/ash34_800x640.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_16)
{
	process ("/omx/patterns/img03_2001x1682_411.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_17)
{
	process ("/omx/patterns/img04_800x599_422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_18)
{
	process ("/omx/patterns/img01_1600x1200_420.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_19)
{
	process ("/omx/patterns/paws_PAL.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_20)
{
	process ("/omx/patterns/parking_WVGA.jpg");
	return;
}
END_TEST

#if 0
START_TEST (test_jpegdec_21)
{
	process ("/omx/patterns/8.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_22)
{
	process ("/omx/patterns/img04_730x599_411.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_23)
{
	process ("/omx/patterns/patzcuaro.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_24)
{
	process ("/omx/patterns/J9_640x480.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_25)
{
	process ("/omx/patterns/img04_791x463_420.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_26)
{
	process ("/omx/patterns/img04_1767x800_gray.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_27)
{
	process ("/omx/patterns/J11_640x480.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_28)
{
	process ("/omx/patterns/pool_CGA.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_29)
{
	process ("/omx/patterns/dog.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_30)
{
	process ("/omx/patterns/img02_421x609_420.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_31)
{
	process ("/omx/patterns/shrek.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_32)
{
	process ("/omx/patterns/shrek_data_err02.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_33)
{
	process ("/omx/patterns/shrek_data_err04.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_34)
{
	process ("/omx/patterns/img02_929x738_gray.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_35)
{
	process ("/omx/patterns/laugh.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_36)
{
	process ("/omx/patterns/dog_sea1-minq444.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_37)
{
	process ("/omx/patterns/dog_sea1_minq420.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_38)
{
	process ("/omx/patterns/input_420.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_39)
{
	process ("/omx/patterns/input_422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_40)
{
	process ("/omx/patterns/dog_sea1-minq422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_41)
{
	process ("/omx/patterns/leaf_autumn_411.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_42)
{
	process ("/omx/patterns/city_gray.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_43)
{
	process ("/omx/patterns/paris1_minq444.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_44)
{
	process ("/omx/patterns/leaf_autumn1_minq422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_45)
{
	process ("/omx/patterns/img03_320x202_444.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_46)
{
	process ("/omx/patterns/leaf_autumn1_minq422_pr.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_47)
{
	process ("/omx/patterns/img01_299x208_411.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_48)
{
	process ("/omx/patterns/img01_240x357_gray.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_49)
{
	process ("/omx/patterns/rubalnight_411.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_50)
{
	process ("/omx/patterns/gull2_minq.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_51)
{
	process ("/omx/patterns/buildi10_Invalid_mcu_size.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_52)
{
	process ("/omx/patterns/buildi9gray.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_53)
{
	process ("/omx/patterns/img03_323x206_422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_54)
{
	process ("/omx/patterns/sunset4_prog411_pr.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_55)
{
	process ("/omx/patterns/waterfall4_minq422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_56)
{
	process ("/omx/patterns/J101_176x144.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_57)
{
	process ("/omx/patterns/img01_177x144_422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_58)
{
	process ("/omx/patterns/face10422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_59)
{
	process ("/omx/patterns/dog_sea1_minq444.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_60)
{
	process ("/omx/patterns/dog_sea1_minq.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_70)
{
	process ("/omx/patterns/img02_176x173_411.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_71)
{
	process ("/omx/patterns/land_sky1_minqgray.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_72)
{
	process ("/omx/patterns/roof.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_73)
{
	process ("/omx/patterns/sunset2_minq422_prog.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_74)
{
	process ("/omx/patterns/kuva_294x293.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_75)
{
	process ("/omx/patterns/img01_94x80_444.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_76)
{
	process ("/omx/patterns/J100_176x144.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_77)
{
	process ("/omx/patterns/jfif422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_78)
{
	process ("/omx/patterns/refin_90x102_422.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_79)
{
	process ("/omx/patterns/img03_90x102_420.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_80)
{
	process ("/omx/patterns/J100_128x96.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_81)
{
	process ("/omx/patterns/img03_86x99_gray.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_82)
{
	process ("/omx/patterns/J100_40x40.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_83)
{
	process ("/omx/patterns/flower_40x40_420.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_84)
{
	process ("/omx/patterns/J100_80x48.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_85)
{
	process ("/omx/patterns/00000000.jpg");
	return;
}
END_TEST

START_TEST (test_jpegdec_86)
{
	process ("/omx/patterns/J100_64x32.jpg");
	return;
}
END_TEST
#endif

void
fill_tcase (gchar* srd, gpointer func, TCase* tc_jpegdec)
{
	tcase_add_test (tc_jpegdec, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo JPEG Decoder");
	TCase *tc_jpegdec = tcase_create ("JPEG Decoder");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "SR1", test_jpegdec_0);
	g_hash_table_insert (ht, "SR2", test_jpegdec_1);
	g_hash_table_insert (ht, "SR3", test_jpegdec_2);
	g_hash_table_insert (ht, "SR4", test_jpegdec_3);
	g_hash_table_insert (ht, "SR5", test_jpegdec_4);
	g_hash_table_insert (ht, "SR6", test_jpegdec_5);
	g_hash_table_insert (ht, "SR7", test_jpegdec_6);
	g_hash_table_insert (ht, "SR8", test_jpegdec_7);
	g_hash_table_insert (ht, "SR9", test_jpegdec_8);
	g_hash_table_insert (ht, "SR10", test_jpegdec_9);
	g_hash_table_insert (ht, "SR11", test_jpegdec_10);
	g_hash_table_insert (ht, "SR12", test_jpegdec_11);
	g_hash_table_insert (ht, "SR13", test_jpegdec_12);
	g_hash_table_insert (ht, "SR14", test_jpegdec_13);
	g_hash_table_insert (ht, "SR15", test_jpegdec_14);
	g_hash_table_insert (ht, "SR16", test_jpegdec_15);
	g_hash_table_insert (ht, "SR17", test_jpegdec_16);
	g_hash_table_insert (ht, "SR18", test_jpegdec_17);
	g_hash_table_insert (ht, "SR19", test_jpegdec_18);
	g_hash_table_insert (ht, "SR20", test_jpegdec_19);
	g_hash_table_insert (ht, "SR21", test_jpegdec_20);


	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_jpegdec);
	}
	else
	{
		tcase_add_test (tc_jpegdec, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_jpegdec, setup, teardown);
	tcase_set_timeout (tc_jpegdec, 0);
	suite_add_tcase (s, tc_jpegdec);

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
		  "Test option: (SR1/SR2/.../SR20/SR21)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- JPEG Decoder Tests.");
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
