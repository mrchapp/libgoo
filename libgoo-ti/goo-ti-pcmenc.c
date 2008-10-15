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
#include <goo-utils.h>

#define ID "OMX.TI.PCM.encode"
#define DASF_PARAM_NAME "OMX.TI.index.config.pcmheaderinfo"
#define DATAPATH_PARAM_NAME "OMX.TI.index.config.pcm.datapath"
#define STREAMID_PARAM_NAME "OMX.TI.index.config.pcmstreamIDinfo"


G_DEFINE_TYPE (GooTiPcmEnc, goo_ti_pcmenc, GOO_TYPE_TI_AUDIO_ENCODER)

static void
goo_ti_pcmenc_init (GooTiPcmEnc* self)
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

	return;
}

static void
goo_ti_pcmenc_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_PCMENC (object));
	GooTiPcmEnc* self = GOO_TI_PCMENC (object);

	if (G_LIKELY (self->input_port_param))
	{
		g_free (self->input_port_param);
		self->input_port_param = NULL;
	}

	if (G_LIKELY (self->output_port_param))
	{
		g_free (self->output_port_param);
		self->output_port_param = NULL;
	}

	(*G_OBJECT_CLASS (goo_ti_pcmenc_parent_class)->finalize) (object);

	return;
}

static void
goo_ti_pcmenc_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_PCMENC (component));
	GooTiPcmEnc* self = GOO_TI_PCMENC (component);
	g_assert (self->output_port_param == NULL);

	self->input_port_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
	GOO_INIT_PARAM (self->input_port_param, OMX_AUDIO_PARAM_PCMMODETYPE);

	self->output_port_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
	GOO_INIT_PARAM (self->output_port_param, OMX_AUDIO_PARAM_PCMMODETYPE);

	goo_component_get_parameter_by_index (component,
										  OMX_IndexParamAudioPcm,
										  self->output_port_param);

	goo_ti_audio_component_audio_manager_activate
		 (GOO_TI_AUDIO_COMPONENT (component));

	GOO_OBJECT_DEBUG (self, "");
	return;
}

static void
goo_ti_pcmenc_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_PCMENC (component));
	GooTiPcmEnc* self = GOO_TI_PCMENC (component);
	g_assert (self->output_port_param != NULL);

	/* @todo move it to goo-utils */
	GOO_OBJECT_DEBUG (self, "\n"
				"nSize: %d\n"
				"nPortIndex: %d\n"
				"nChannels: %d\n"
				"nSamplingRate: %d",
				self->output_port_param->nSize,
				self->output_port_param->nPortIndex,
				self->output_port_param->nChannels,
				self->output_port_param->nSamplingRate
				);

	goo_component_set_parameter_by_index (component,
										  OMX_IndexParamAudioPcm,
										  self->output_port_param);
	GOO_OBJECT_DEBUG (self, "Exit");
	return;
}

#if 0
static void
goo_ti_pcmenc_eos_buffer_flag (GooComponent* self, guint portindex)
{
	g_assert (self != NULL);

	GOO_OBJECT_INFO (self, "EOS flag found in port %d", portindex);

	GooPort* port = NULL;
	OMX_PARAM_PORTDEFINITIONTYPE* param = NULL;
	GooIterator* iter = goo_component_iterate_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		port = GOO_PORT (goo_iterator_get_current (iter));
		param = GOO_PORT_GET_DEFINITION (port);
		if (param->nPortIndex == portindex)
		{
			if (param->eDir == OMX_DirInput)
			{
				goo_component_set_done (self);
			}
			else if (param->eDir == OMX_DirOutput)
			{
				/* @FIXME: This is an ugly workaround
				 * just to make it work in file-to-file
				 * mode */
				goo_component_set_done (self);
				//goo_port_set_eos (port);
			}

			g_object_unref (G_OBJECT (port));
			break;
		}
		g_object_unref (G_OBJECT (port));
		goo_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));

	return;
}
#endif

static void
goo_ti_pcmenc_validate_ports (GooComponent* component)
{
	g_assert (GOO_IS_TI_PCMENC (component));
	GooTiPcmEnc* self = GOO_TI_PCMENC (component);
	g_assert (self->output_port_param != NULL);

	OMX_PARAM_PORTDEFINITIONTYPE *def = NULL;

	GOO_OBJECT_DEBUG (self, "Enter");

	/* Input */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		def = GOO_PORT_GET_DEFINITION (port);

		def->nBufferSize = INPUT_BUFFER_SIZE;
		def->format.audio.eEncoding = OMX_AUDIO_CodingPCM;
		def->format.audio.cMIMEType = "audio/x-raw-int";

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* Output */
	{
		GooIterator* iter =
			goo_component_iterate_output_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		def = GOO_PORT_GET_DEFINITION (port);

		if ( goo_ti_audio_component_is_dasf_mode ( GOO_TI_AUDIO_COMPONENT (component)))
		{
			GOO_OBJECT_DEBUG (self, "the input port is tunneled");
			def->nBufferSize = OUTPUT_BUFFER_SIZE;
		}
		else
		{
			GOO_OBJECT_DEBUG (self, "the input port is not tunneled");
			def->nBufferSize = OUTPUT_BUFFER_SIZE_FILE;
		}
		def->format.audio.eEncoding = OMX_AUDIO_CodingPCM;
		def->format.audio.cMIMEType = "audio/x-raw-int";

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* param */
	{
		OMX_AUDIO_PARAM_PCMMODETYPE *param;

		param = GOO_TI_PCMENC_GET_OUTPUT_PORT_PARAM (component);
		param->nChannels = 1; /* Always MONO */
		param->nPortIndex = OMX_DirOutput;
		/* should validate the rest ? */
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

static void
goo_ti_pcmenc_class_init (GooTiPcmEncClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);
	g_klass->finalize = goo_ti_pcmenc_finalize;

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->load_parameters_func = goo_ti_pcmenc_load_parameters;
	o_klass->set_parameters_func = goo_ti_pcmenc_set_parameters;
	o_klass->validate_ports_definition_func =
		goo_ti_pcmenc_validate_ports;
	/* o_klass->eos_flag_func = goo_ti_pcmenc_eos_buffer_flag; */

	return;
}

