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

#include <goo-ti-armaacenc.h>
#include <goo-utils.h>

#include <math.h>

#define ID "OMX.TI.ARMAAC.encode"
#define DASF_PARAM_NAME "OMX.TI.index.config.armaacencHeaderInfo"
#define DATAPATH_PARAM_NAME "OMX.TI.index.config.armaac.datapath"
#define STREAMID_PARAM_NAME "OMX.TI.index.config.armaacencstreamIDinfo"

#define FRAMESPERBUFFER "OMX.TI.index.config.armaacencframesPerOutBuf"

#define BITRATEMODE_DEFAULT  0
#define OUTPUTFRAMES_DEFAULT 12

G_DEFINE_TYPE (GooTiArmAacEnc, goo_ti_armaacenc, GOO_TYPE_TI_AUDIO_ENCODER)

enum _GooTiArmAacEncProp
{
	PROP_0,
	PROP_BITRATEMODE,
	PROP_OUTPUTFRAMES
};

#define GOO_TYPE_BITRATEMODE \
	(goo_ti_armaacenc_bitratemode_get_type())

GType
goo_ti_armaacenc_bitratemode_get_type ()
{
	static GType type = 0;

	if (type == 0)
	{
		const static GEnumValue values[] = {
			{ GOO_TI_ARMAACENC_BR_CBR, "0", "Constant Bit Rate" },
			{ GOO_TI_ARMAACENC_BR_VBR1, "1",
			  "Variable Bit Rate Quality Level 1" },
			{ GOO_TI_ARMAACENC_BR_VBR2, "2",
			  "Variable Bit Rate Quality Level 2" },
			{ GOO_TI_ARMAACENC_BR_VBR3, "3",
			  "Variable Bit Rate Quality Level 3" },
			{ GOO_TI_ARMAACENC_BR_VBR4, "4",
			  "Variable Bit Rate Quality Level 4" },
			{ GOO_TI_ARMAACENC_BR_VBR5, "5",
			  "Variable Bit Rate Quality Level 5" },
			{ 0, NULL, NULL },
		};

		type = g_enum_register_static ("GooTiArmAacEncBitRateMode", values);
	}

	return type;
}

static guint
_output_buffer_size (GooTiArmAacEnc* self)
{
	g_assert (GOO_IS_TI_ARMAACENC (self));

	OMX_AUDIO_PARAM_AACPROFILETYPE* param = NULL;
	param = GOO_TI_AACENC_GET_OUTPUT_PORT_PARAM (self);

	gfloat avgframesiz = 0;
	gfloat fps = 0;
	gint outbufsiz = 0;

	fps = 1.0 * param->nSampleRate / 1024;
	avgframesiz = (1.0 * param->nBitRate / fps) / 8 * 1.3;
	outbufsiz = ceil ((avgframesiz * self->output_frames) + 1200);

	return outbufsiz;
}

static gboolean
_validate_sample_rate (OMX_AUDIO_PARAM_AACPROFILETYPE* param)
{
	g_assert (param != NULL);

	const static gint valid_frec[] = {
		8000, 11025, 16000, 22050, 32000,
		44100, 48000
	};

	gint i;
	for (i = 0; i <= GOO_ARRAY_SIZE (valid_frec); i++)
	{
		if (param->nSampleRate == valid_frec[i])
		{
			return TRUE;
		}
	}

	return FALSE;
}

static gboolean
_validate_bit_rate (OMX_AUDIO_PARAM_AACPROFILETYPE* param)
{
	g_assert (param != NULL);

	return (param->nBitRate >= 8000 && param->nBitRate <= 320000);
}

static gboolean
_validate_parameters_combination (OMX_AUDIO_PARAM_AACPROFILETYPE* param)
{
	const gint MONO	  = 1;
	const gint STEREO = 2;

	guint limits[12][4] = {
		/* sample rate, channel, min bitrate, max bitrate*/
		{ 8000,	 MONO,	 8000,	42000  },
		{ 8000,	 STEREO, 16000, 84000  },
		{ 16000, MONO,	 8000,	84000  },
		{ 16000, STEREO, 16000, 168000 },
		{ 22050, STEREO, 16000, 232000 },
		{ 22050, STEREO, 16000, 232000 },
		{ 32000, STEREO, 16000, 320000 },
		{ 32000, STEREO, 16000, 320000 },
		{ 44100, MONO,	 8000,	160000 },
		{ 44100, STEREO, 16000, 320000 },
		{ 48000, MONO,	 8000,	160000 },
		{ 48000, STEREO, 16000, 320000 }
	};

	guint i, j;

	if (param->nSampleRate > 48000)
	{
		g_warning ("Samplerate and Bitrate combination has not been "
			   "validated");
		return TRUE;
	}

	for (i = 0; i <= GOO_ARRAY_SIZE (limits); i += 2)
	{
		if (param->nSampleRate == limits[i][0])
		{
			for (j = 0; j < 2; j++)
			{
				if (param->nChannels == limits[i + j][1])
				{
					if (param->nBitRate>=limits[i+j][2] &&
					    param->nBitRate<=limits[i+j][3])
					{
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
			}

			if (j >= 2)
			{
				g_warning ("nChanels = %d",
					   (int) param->nChannels);
				return FALSE;
			}
		}
	}

	return TRUE;
}

static void
_goo_ti_armaacenc_set_bit_rate_mode (GooTiArmAacEnc *self,
				  GooTiAmrAacEncBitRateMode bit_rate_mode)
{
	g_assert (self != NULL);
	GooTiAudioComponent *me = GOO_TI_AUDIO_COMPONENT (self);

	me->audioinfo->aacencHeaderInfo->bitratemode = bit_rate_mode;

	goo_component_set_config_by_name (GOO_COMPONENT (self),
					  me->dasf_param_name,
					  me->audioinfo);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
_goo_ti_armaacenc_set_output_frames (GooTiArmAacEnc *self)
{
	g_assert (self != NULL);

	GOO_OBJECT_DEBUG (self, "Frames per output = %d", self->output_frames);

	goo_component_set_config_by_name (GOO_COMPONENT (self),
					  FRAMESPERBUFFER,
					  &self->output_frames);

	return;
}

static void
goo_ti_armaacenc_set_property (GObject *object, guint prop_id,
			    const GValue* value, GParamSpec *spec)
{
	g_assert (GOO_IS_TI_AACENC (object));
	GooTiArmAacEnc *self = GOO_TI_ARMAACENC (object);

	switch ( prop_id )
	{
	case PROP_BITRATEMODE:
		_goo_ti_armaacenc_set_bit_rate_mode (self,
						  g_value_get_enum (value));
		break;
	case PROP_OUTPUTFRAMES:
		self->output_frames = g_value_get_uint (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}
}

static void
goo_ti_armaacenc_get_property (GObject *object, guint prop_id,
			    GValue* value, GParamSpec *spec)
{
	g_assert (GOO_IS_TI_ARMAACENC (object));
	GooTiAacEnc *self = GOO_TI_ARMAACENC (object);

	switch ( prop_id )
	{
	case PROP_BITRATEMODE:
		g_value_set_enum (value, GOO_TI_AUDIO_COMPONENT (self)->audioinfo->aacencHeaderInfo->bitratemode);
		break;
	case PROP_OUTPUTFRAMES:
		g_value_set_uint (value, self->output_frames);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}
}

static void
goo_ti_armaacenc_init (GooTiArmAacEnc* self)
{
	GOO_COMPONENT (self)->id = g_strdup (ID);
	GOO_TI_AUDIO_COMPONENT (self)->dasf_param_name =
		g_strdup (DASF_PARAM_NAME);
	GOO_TI_AUDIO_COMPONENT (self)->streamid_param_name =
		g_strdup (STREAMID_PARAM_NAME);
	GOO_TI_AUDIO_COMPONENT (self)->datapath_param_name =
		g_strdup (DATAPATH_PARAM_NAME);

	self->input_port_param = NULL;
	self->output_port_param = NULL;

	self->output_frames = OUTPUTFRAMES_DEFAULT;

	GooTiAudioComponent *me = GOO_TI_AUDIO_COMPONENT (self);
	if (me->audioinfo->aacencHeaderInfo == NULL)
	{
		me->audioinfo->aacencHeaderInfo = g_new0 (AACENC_HeadInfo, 1);
	}

	return;
}

static void
goo_ti_armaacenc_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_ARMAACENC (object));
	GooTiArmAacEnc* self = GOO_TI_ARMAACENC (object);

	if (G_LIKELY (self->input_port_param != NULL))
	{
		g_free (self->input_port_param);
		self->input_port_param = NULL;
	}

	if (G_LIKELY (self->output_port_param != NULL))
	{
		g_free (self->output_port_param);
		self->output_port_param = NULL;
	}

	GooTiAudioComponent *me = GOO_TI_AUDIO_COMPONENT (self);
	if (me->audioinfo->aacencHeaderInfo != NULL)
	{
		g_free (me->audioinfo->aacencHeaderInfo);
		me->audioinfo->aacencHeaderInfo = NULL;
	}


	(*G_OBJECT_CLASS (goo_ti_aacenc_parent_class)->finalize) (object);

	return;
}

static void
goo_ti_armaacenc_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_ARMAACENC (component));
	GooTiArmAacEnc* self = GOO_TI_ARMAACENC (component);

	g_assert (self->input_port_param == NULL);
	g_assert (self->output_port_param == NULL);

	self->input_port_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
	GOO_INIT_PARAM (self->input_port_param, OMX_AUDIO_PARAM_PCMMODETYPE);

	goo_component_get_parameter_by_index (component,
					      OMX_IndexParamAudioPcm,
					      self->input_port_param);

	self->output_port_param = g_new0 (OMX_AUDIO_PARAM_AACPROFILETYPE, 1);
	GOO_INIT_PARAM (self->output_port_param,
			OMX_AUDIO_PARAM_AACPROFILETYPE);

	goo_component_get_parameter_by_index (component,
					      OMX_IndexParamAudioAac,
					      self->output_port_param);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_ti_armaacenc_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_ARMAACENC (component));
	GooTiArmAacEnc* self = GOO_TI_ARMAACENC (component);

	g_assert (self->input_port_param != NULL);
	g_assert (self->output_port_param != NULL);

	goo_component_set_parameter_by_index (component,
					      OMX_IndexParamAudioPcm,
					      self->input_port_param);

	goo_component_set_parameter_by_index (component,
					      OMX_IndexParamAudioAac,
					      self->output_port_param);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_ti_armaacenc_validate_ports (GooComponent* component)
{
	g_assert (GOO_IS_TI_ARMAACENC (component));
	GooTiArmAacEnc* self = GOO_TI_ARMAACENC (component);
	g_assert (self->input_port_param != NULL);
	g_assert (self->output_port_param != NULL);

	_goo_ti_armaacenc_set_output_frames (self);

	/* input param */
	{
		OMX_AUDIO_PARAM_PCMMODETYPE* param = NULL;
		param = GOO_TI_ARMAACENC_GET_INPUT_PORT_PARAM (self);

		param->nBitPerSample = 16;
	}

	/* output param */
	{
		OMX_AUDIO_PARAM_AACPROFILETYPE* param = NULL;
		param = GOO_TI_ARMAACENC_GET_OUTPUT_PORT_PARAM (self);

		g_assert (param->eAACProfile == OMX_AUDIO_AACObjectLC ||
			  param->eAACProfile == OMX_AUDIO_AACObjectHE ||
			  param->eAACProfile == OMX_AUDIO_AACObjectHE_PS);

		g_assert (param->eAACStreamFormat ==
			  OMX_AUDIO_AACStreamFormatRAW ||
			  param->eAACStreamFormat ==
			  OMX_AUDIO_AACStreamFormatADIF ||
			  param->eAACStreamFormat ==
			  OMX_AUDIO_AACStreamFormatMP4ADTS);

		g_assert (_validate_sample_rate (param) == TRUE);
		g_assert (_validate_bit_rate (param) == TRUE);
		g_assert (_validate_parameters_combination (param) == TRUE);

		if (param->eAACProfile == OMX_AUDIO_AACObjectHE)
		{
			g_assert (param->nBitRate <= 48000);
		}
		else if (param->eAACProfile == OMX_AUDIO_AACObjectHE_PS)
		{
			 g_assert (param->nBitRate <= 128000);
		}

		param->nAudioBandWidth = 0x0; /* let's the encoder decide */
		param->nFrameLength    = 0x0; /* let's the encoder decide */
		param->nAACtools       = OMX_AUDIO_AACToolPNS + \
			OMX_AUDIO_AACToolTNS + OMX_AUDIO_AACToolMS;
		param->nAACERtools     = OMX_AUDIO_AACERNone;

		if (param->nChannels == 1)
		{
			param->eChannelMode = OMX_AUDIO_ChannelModeMono;
		}
		else if (param->nChannels == 2)
		{
			param->eChannelMode = OMX_AUDIO_ChannelModeStereo;
		}
		else
		{
			g_assert_not_reached ();
		}

	}

	/* input */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE* param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		param->nBufferSize = GOO_TI_ARMAACENC_INPUT_BUFFER_SIZE;
		param->format.audio.eEncoding = OMX_AUDIO_CodingPCM;

		GooTiAudioComponent *me = GOO_TI_AUDIO_COMPONENT (self);
		if (me->audioinfo->dasfMode == 1)
		{
			param->nBufferCountActual = 0;
		}

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* output */
	{
		GooIterator* iter =
			goo_component_iterate_output_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE* param = NULL;
		param = GOO_PORT_GET_DEFINITION (port);

		param->nBufferSize = _output_buffer_size (self);
		param->format.audio.eEncoding = OMX_AUDIO_CodingAAC;

		GooTiAudioComponent *me = GOO_TI_AUDIO_COMPONENT (self);
		OMX_AUDIO_PARAM_AACPROFILETYPE* profile = NULL;
		profile = GOO_TI_ARMAACENC_GET_OUTPUT_PORT_PARAM (self);

		if (me->audioinfo->dasfMode == 1)
		{
			param->nBufferCountActual = 2;
		}
		else if (profile->nChannels == 2)
		{
			param->nBufferCountActual = 4;
		}

		g_object_unref (iter);
		g_object_unref (port);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/* WORKAROUND for DR OMAPS00164744 */
static void
goo_ti_armaacenc_eos_buffer_flag (GooComponent* self, guint portindex)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (portindex >= 0);

	g_print ("INFORMATION: workaround for OMAPS00164744\n");

	GooPort* port = NULL;
	OMX_PARAM_PORTDEFINITIONTYPE* param = NULL;

	GooIterator* iter = goo_component_iterate_ports (self);
	goo_iterator_nth (iter, portindex);
	port = GOO_PORT (goo_iterator_get_current (iter));
	g_assert (port != NULL);

	goo_port_set_eos (port);

	param = GOO_PORT_GET_DEFINITION (port);

	if (param->eDir == OMX_DirOutput)
	{
		goo_component_set_done (self);
	}

	g_object_unref (G_OBJECT (port));
	g_object_unref (G_OBJECT (iter));

	return;
}

static void
goo_ti_armaacenc_class_init (GooTiArmAacEncClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);
	g_klass->finalize = goo_ti_armaacenc_finalize;

	g_klass->set_property =	goo_ti_armaacenc_set_property;
	g_klass->get_property = goo_ti_armaacenc_get_property;

	GParamSpec *spec;

	spec = g_param_spec_enum ("bitrate-mode", "bit rate mode",
				  "Specifies the bit rate mode",
				  GOO_TYPE_BITRATEMODE,
				  BITRATEMODE_DEFAULT, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_BITRATEMODE, spec);

	spec = g_param_spec_uint ("frames-buffer", "Frames per Buffer",
				  "Number of frames per output buffer",
				  1, 24, OUTPUTFRAMES_DEFAULT,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_OUTPUTFRAMES, spec);

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->load_parameters_func = goo_ti_armaacenc_load_parameters;
	o_klass->set_parameters_func = goo_ti_armaacenc_set_parameters;
	o_klass->validate_ports_definition_func = goo_ti_armaacenc_validate_ports;
	o_klass->eos_flag_func = goo_ti_armaacenc_eos_buffer_flag;

	/* no flush workaround */
	o_klass->flush_port_func = NULL;

	return;
}
