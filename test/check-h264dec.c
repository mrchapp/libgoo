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

#include <goo-ti-component-factory.h>
#include <goo-engine.h>
#include <goo-utils.h>
#include <goo-ti-h264dec.h>
#include <check.h>
#include <stdlib.h>

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <glib.h>

GooComponentFactory* factory;
GooComponent* component;
GRand* rnd;

void
setup (void)
{
	factory = goo_ti_component_factory_get_instance ();
	component = goo_component_factory_get_component (factory, GOO_TI_H264_DECODER);
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

static void
process (gchar *infile, gint width, gint height, gint outcolor, GooTiVideoDecoderProcessMode process_mode)
{
	fail_unless (infile != NULL, "unspecified filename in test");
	fail_unless (g_file_test (infile, G_FILE_TEST_IS_REGULAR),
		     "file don't exist");

	gchar outfile[100];
	gchar *fn, *fn1;
	gboolean vopparser;

	fn = g_path_get_basename (infile);
	fn1 = strchr (fn, '.');
	fn1[0] = '\0';
	g_snprintf (outfile, 100, "/tmp/%s.yuv", fn);
	g_free (fn);

	/* input port */
	{
		GooPort* port = goo_component_get_port (component, "input0");
		g_assert (port != NULL);
		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->nBufferCountActual = 4;
		param->format.video.nFrameWidth = width;
		param->format.video.nFrameHeight = height;
		param->format.video.eColorFormat = OMX_COLOR_FormatCbYCrY;

		g_object_unref (G_OBJECT (port));
	}

	/* output port */
	{
		GooPort* port = goo_component_get_port (component, "output0");
		g_assert (port != NULL);
		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->nBufferCountActual = 4;
		param->format.video.nFrameWidth = width;
		param->format.video.nFrameHeight = height;
		param->format.video.eColorFormat = outcolor;

		g_object_unref (G_OBJECT (port));
	}

	/* video properties */
	  g_object_set (G_OBJECT (component),
					"process-mode", process_mode,
					NULL);
	vopparser = (process_mode == GOO_TI_VIDEO_DECODER_FRAMEMODE) ? TRUE : FALSE;

/*	GooEngine* engine = goo_engine_new_vop (component, infile, outfile, vopparser); */
	GooEngine* engine = goo_engine_new_vop (component, infile, "/dev/null", vopparser);

	goo_component_set_state_idle (component);
	goo_component_set_state_executing (component);

	goo_engine_play (engine);

	goo_component_set_state_idle (component);
	goo_component_set_state_loaded (component);

	g_object_unref (G_OBJECT (engine));

	return;
}

START_TEST (BF0001)
{
	process ("/omx/patterns/H264_CIF_299f_30fps_2000kbits_L2.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);

	return;
}
END_TEST

START_TEST (BF0002)
{
	process ("/omx/patterns/H264_CIF_150f_7_5fps_192kbits_L1_1.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (BF0003)
{
	process ("/omx/patterns/L12_QCIF_B26135_N17_BA1_Sony_C.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (BF0004)
{
	process ("/omx/patterns/H264_CIF_1700f_30fps_768kbits_L1_3.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (BF0005)
{
	process ("/omx/patterns/H264_CIF_1700f_30fps_768kbits_L1_3.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (BF0006)
{
	process ("/omx/patterns/L3_VGA_B133444_N100_foreman.264",
		 640, 480, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0007)
{
	process ("/omx/patterns/H264_QCIF_100f_15fps_64kbits_L1.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0008)
{
	process ("/omx/patterns/L3_VGA_B133444_N100_foreman.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0009)
{
	process ("/omx/patterns/foreman_i_p1_176x144.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0010)
{
	process ("/omx/patterns/foreman_i_p1_176x144.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0011)
{
	process ("/omx/patterns/foreman_i_p1_176x144.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0012)
{
	process ("/omx/patterns/foreman_i_p1_176x144.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0013)
{
	process ("/omx/patterns/foreman_i_p1_176x144.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0014)
{
	process ("/omx/patterns/foreman_i_p1_176x144.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0047)
{
	process ("/omx/patterns/overthehedge_video_640x480_24fps.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0048)
{
	process ("/omx/patterns/overthehedge_video_640x480_24fps.264",
		 640, 480, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0051)
{
	process ("/omx/patterns/overthehedge_video_640x480_24fps.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0052)
{
	process ("/omx/patterns/overthehedge_video_640x480_24fps.264",
		 640, 480, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0055)
{
	process ("/omx/patterns/overthehedge_video_640x480_24fps.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0056)
{
	process ("/omx/patterns/overthehedge_video_640x480_24fps.264",
		 640, 480, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0059)
{
	process ("/omx/patterns/overthehedge_video_640x480_24fps.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0148)
{
	process ("/omx/patterns/overthehedge_video_640x480_24fps.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0061)
{
	process ("/omx/patterns/AUD_MW_E.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0062)
{
	process ("/omx/patterns/AUD_MW_E.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0063)
{
	process ("/omx/patterns/AUD_MW_E.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0064)
{
	process ("/omx/patterns/AUD_MW_E.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0065)
{
	process ("/omx/patterns/AUD_MW_E.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0066)
{
	process ("/omx/patterns/AUD_MW_E.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0067)
{
	process ("/omx/patterns/AUD_MW_E.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0068)
{
	process ("/omx/patterns/AUD_MW_E.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0069)
{
	process ("/omx/patterns/CVBS3_Sony_C.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0070)
{
	process ("/omx/patterns/CVBS3_Sony_C.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0071)
{
	process ("/omx/patterns/CVBS3_Sony_C.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0072)
{
	process ("/omx/patterns/CVBS3_Sony_C.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0073)
{
	process ("/omx/patterns/son.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0074)
{
	process ("/omx/patterns/son.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0075)
{
	process ("/omx/patterns/son.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0076)
{
	process ("/omx/patterns/son.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0077)
{
	process ("/omx/patterns/NLMQ2_JVC_C.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0078)
{
	process ("/omx/patterns/NLMQ2_JVC_C.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0079)
{
	process ("/omx/patterns/NLMQ2_JVC_C.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0080)
{
	process ("/omx/patterns/NLMQ2_JVC_C.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0081)
{
	process ("/omx/patterns/son_i_p_20.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0082)
{
	process ("/omx/patterns/son_i_p_20.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0083)
{
	process ("/omx/patterns/son_i_p_20.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0084)
{
	process ("/omx/patterns/son_i_p_20.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0085)
{
	process ("/omx/patterns/son_i_p_20_352x288.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0086)
{
	process ("/omx/patterns/son_i_p_20_352x288.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0087)
{
	process ("/omx/patterns/son_i_p_20_352x288.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0088)
{
	process ("/omx/patterns/son_i_p_20_352x288.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0089)
{
	process ("/omx/patterns/P_FrameWorstCase.264",
		 704, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0090)
{
	process ("/omx/patterns/P_FrameWorstCase.264",
		 704, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0091)
{
	process ("/omx/patterns/football_5Mbps_30frames.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0092)
{
	process ("/omx/patterns/football_5Mbps_30frames.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0093)
{
	process ("/omx/patterns/NRF_MW_E.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0094)
{
	process ("/omx/patterns/NRF_MW_E.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0095)
{
	process ("/omx/patterns/fire_30frames_5_Mbps.264",
		 720, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0096)
{
	process ("/omx/patterns/fire_30frames_5_Mbps.264",
		 720, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0097)
{
	process ("/omx/patterns/BA2_Sony_F.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0098)
{
	process ("/omx/patterns/BA2_Sony_F.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0099)
{
	process ("/omx/patterns/H264_QCIF_100f_15fps_64kbits_L1_bursty_3%_biterror.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0100)
{
	process ("/omx/patterns/H264_QCIF_100f_15fps_64kbits_L1_bursty_5%_biterror.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0101)
{
	process ("/omx/patterns/H264_QCIF_100f_15fps_64kbits_L1_MissingIFrame.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0102)
{
	process ("/omx/patterns/H264_QCIF_100f_15fps_64kbits_L1_packetloss_5%_error.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0103)
{
	process ("/omx/patterns/H264_QCIF_100f_15fps_64kbits_L1_packetloss_10%_error.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0108)
{
	process ("/omx/patterns/64_matrix.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0113)
{
	process ("/omx/patterns/64_matrix.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0114)
{
	process ("/omx/patterns/BAMQ1_JVC_B_30.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0115)
{
	process ("/omx/patterns/BAMQ1_JVC_B_30.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0119)
{
	process ("/omx/patterns/L10_QCIF_B4424_N100_AUD_MW_C.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0120)
{
	process ("/omx/patterns/H264_VGA_53f_30fps_3000kbits_L3.264",
		 640, 480, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0121)
{
	process ("/omx/patterns/H264_VGA_53f_30fps_3000kbits_L3.264",
		 640, 480, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0124)
{
	process ("/omx/patterns/L10_QCIF_B4424_N100_AUD_MW_C.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0125)
{
	process ("/omx/patterns/L10_QCIF_B4424_N100_AUD_MW_C.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0130)
{
	process ("/omx/patterns/H264_CIF_150f_7_5fps_192kbits_L1_1_nal_frame.264nb",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0131)
{
	process ("/omx/patterns/H264_CIF_150f_7_5fps_192kbits_L1_1_nal_frame.264nb",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0132)
{
	process ("/omx/patterns/howtoparkyourmotorcr_nal_frame_qvga.264nb",
		 320, 240, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0133)
{
	process ("/omx/patterns/howtoparkyourmotorcr_nal_frame_qvga.264nb",
		 320, 240, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST

START_TEST (BF0134)
{
	process ("/omx/patterns/L30_CIF_B_N400_dlp_32QP_10fps_2Fskip_nal_stream.264nb",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0155)
{
	process ("/omx/patterns/H264_QCIF_100f_15fps_128kbits_L1_b.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0156)
{
	process ("/omx/patterns/H264_CIF_150f_7_5fps_192kbits_L1_1.264",
		 352, 288, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_STREAMMODE);
	return;
}
END_TEST

START_TEST (BF0157)
{
	process ("/omx/patterns/L12_QCIF_B26135_N17_BA1_Sony_C.264",
		 176, 144, OMX_COLOR_FormatYUV420PackedPlanar, GOO_TI_VIDEO_DECODER_FRAMEMODE);
	return;
}
END_TEST


START_TEST (SR10560)
{
	process ("/multimedia/patterns/SR10560.264",
		 176, 144, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (SR10561)
{
	process ("/multimedia/patterns/SR10561.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST


START_TEST (SR10562)
{
	process ("/multimedia/patterns/SR10562.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (SR10563)
{
	process ("/multimedia/patterns/SR10563.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST

START_TEST (SR10564)
{
	process ("/multimedia/patterns/SR10564.264",
		 352, 288, OMX_COLOR_FormatCbYCrY, GOO_TI_VIDEO_DECODER_STREAMMODE);

	return;
}
END_TEST


void
fill_tcase (gchar* srd, gpointer func, TCase* tc_h264)
{
	tcase_add_test (tc_h264, func);

	return;
}

Suite *
goo_suite (gchar* srd)
{
	Suite *s = suite_create ("Goo");
	TCase *tc_h264 = tcase_create ("H264");

	GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (ht, "BF0001", BF0001);
	g_hash_table_insert (ht, "BF0002", BF0002);
	g_hash_table_insert (ht, "BF0003", BF0003);
	g_hash_table_insert (ht, "BF0004", BF0004);
	g_hash_table_insert (ht, "BF0005", BF0005);
	g_hash_table_insert (ht, "BF0006", BF0006);
	g_hash_table_insert (ht, "BF0007", BF0007);
	g_hash_table_insert (ht, "BF0008", BF0008);
	g_hash_table_insert (ht, "BF0009", BF0009);
	g_hash_table_insert (ht, "BF0010", BF0010);
	g_hash_table_insert (ht, "BF0011", BF0011);
	g_hash_table_insert (ht, "BF0012", BF0012);
	g_hash_table_insert (ht, "BF0013", BF0013);
	g_hash_table_insert (ht, "BF0014", BF0014);
	g_hash_table_insert (ht, "BF0047", BF0047);
	g_hash_table_insert (ht, "BF0048", BF0048);
	g_hash_table_insert (ht, "BF0051", BF0051);
	g_hash_table_insert (ht, "BF0052", BF0052);
	g_hash_table_insert (ht, "BF0055", BF0055);
	g_hash_table_insert (ht, "BF0056", BF0056);
	g_hash_table_insert (ht, "BF0059", BF0059);
	g_hash_table_insert (ht, "BF0148", BF0148);
	g_hash_table_insert (ht, "BF0061", BF0061);
	g_hash_table_insert (ht, "BF0062", BF0062);
	g_hash_table_insert (ht, "BF0063", BF0063);
	g_hash_table_insert (ht, "BF0064", BF0064);
	g_hash_table_insert (ht, "BF0065", BF0065);
	g_hash_table_insert (ht, "BF0066", BF0066);
	g_hash_table_insert (ht, "BF0067", BF0067);
	g_hash_table_insert (ht, "BF0068", BF0068);
	g_hash_table_insert (ht, "BF0069", BF0069);
	g_hash_table_insert (ht, "BF0070", BF0070);
	g_hash_table_insert (ht, "BF0071", BF0071);
	g_hash_table_insert (ht, "BF0072", BF0072);
	g_hash_table_insert (ht, "BF0073", BF0073);
	g_hash_table_insert (ht, "BF0074", BF0074);
	g_hash_table_insert (ht, "BF0075", BF0075);
	g_hash_table_insert (ht, "BF0076", BF0076);
	g_hash_table_insert (ht, "BF0077", BF0077);
	g_hash_table_insert (ht, "BF0078", BF0078);
	g_hash_table_insert (ht, "BF0079", BF0079);
	g_hash_table_insert (ht, "BF0080", BF0080);
	g_hash_table_insert (ht, "BF0081", BF0081);
	g_hash_table_insert (ht, "BF0082", BF0082);
	g_hash_table_insert (ht, "BF0083", BF0083);
	g_hash_table_insert (ht, "BF0084", BF0084);
	g_hash_table_insert (ht, "BF0085", BF0085);
	g_hash_table_insert (ht, "BF0086", BF0086);
	g_hash_table_insert (ht, "BF0087", BF0087);
	g_hash_table_insert (ht, "BF0088", BF0088);
	g_hash_table_insert (ht, "BF0089", BF0089);
	g_hash_table_insert (ht, "BF0090", BF0090);
	g_hash_table_insert (ht, "BF0091", BF0091);
	g_hash_table_insert (ht, "BF0092", BF0092);
	g_hash_table_insert (ht, "BF0093", BF0093);
	g_hash_table_insert (ht, "BF0094", BF0094);
	g_hash_table_insert (ht, "BF0095", BF0095);
	g_hash_table_insert (ht, "BF0096", BF0096);
	g_hash_table_insert (ht, "BF0097", BF0097);
	g_hash_table_insert (ht, "BF0098", BF0098);
	g_hash_table_insert (ht, "BF0099", BF0099);
	g_hash_table_insert (ht, "BF0100", BF0100);
	g_hash_table_insert (ht, "BF0101", BF0101);
	g_hash_table_insert (ht, "BF0102", BF0102);
	g_hash_table_insert (ht, "BF0103", BF0103);
	g_hash_table_insert (ht, "BF0108", BF0108);
	g_hash_table_insert (ht, "BF0113", BF0113);
	g_hash_table_insert (ht, "BF0114", BF0114);
	g_hash_table_insert (ht, "BF0115", BF0115);
	g_hash_table_insert (ht, "BF0119", BF0119);
	g_hash_table_insert (ht, "BF0120", BF0120);
	g_hash_table_insert (ht, "BF0121", BF0121);
	g_hash_table_insert (ht, "BF0124", BF0124);
	g_hash_table_insert (ht, "BF0125", BF0125);
	g_hash_table_insert (ht, "BF0130", BF0130);
	g_hash_table_insert (ht, "BF0131", BF0131);
	g_hash_table_insert (ht, "BF0132", BF0132);
	g_hash_table_insert (ht, "BF0133", BF0133);
	g_hash_table_insert (ht, "BF0134", BF0134);
	g_hash_table_insert (ht, "BF0155", BF0155);
	g_hash_table_insert (ht, "BF0156", BF0156);
	g_hash_table_insert (ht, "BF0157", BF0157);
	g_hash_table_insert (ht, "SR10560", SR10560);
	g_hash_table_insert (ht, "SR10561", SR10561);
	g_hash_table_insert (ht, "SR10562", SR10562);
	g_hash_table_insert (ht, "SR10563", SR10563);
	g_hash_table_insert (ht, "SR10564", SR10564);

	if (g_ascii_strncasecmp ("all", srd, 3) == 0)
	{
		g_hash_table_foreach (ht, (GHFunc) fill_tcase, tc_h264);
	}
	else
	{
		tcase_add_test (tc_h264, g_hash_table_lookup (ht, srd));
	}

	g_hash_table_destroy (ht);

	tcase_add_checked_fixture (tc_h264, setup, teardown);
	tcase_set_timeout (tc_h264, 0);
	suite_add_tcase (s, tc_h264);

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
		  "Test option: (BF0001/BF0002/BF0003/BF0004/BF0005/BF0006/BF0007/BF0008/BF0009/BF0010/BF0011/BF0012/BF0013/BF0014/BF0047/BF0048/BF0051/BF0052/BF0055/BF0056/BF0059/BF0148/BF0061/BF0062/BF0063/BF0064/BF0065/BF0066/BF0067/BF0068/BF0069/BF0070/BF0071/BF0072/BF0073/BF0074/BF0075/BF0076/BF0077/BF0078/BF0079/BF0080/BF0081/BF0082/BF0083/BF0084/BF0085/BF0086/BF0087/BF0088/BF0089/BF0090/BF0091/BF0092/BF0093/BF0094/BF0095/BF0096/BF0097/BF0098/BF0099/BF0100/BF0101/BF0102/BF0103/BF0108/BF0113/BF0114/BF0115/BF0119/BF0120/BF0121/BF0124/BF0125/BF0130/BF0131/BF0132/BF0133/BF0134/BF0155/BF0156/BF0157/BF0188/BF0198/SR10560/SR10562/SR10563/SR10564)", "S" },
		{ NULL }
	};

	ctx = g_option_context_new ("- H264 Tests.");
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

