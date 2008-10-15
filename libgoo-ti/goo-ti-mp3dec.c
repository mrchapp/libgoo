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

#include <goo-ti-mp3dec.h>
#include <goo-utils.h>
#include <stdio.h>

#define ID "OMX.TI.MP3.decode"
#define DATAPATH_PARAM_NAME "OMX.TI.index.config.mp3.datapath"
#define DASF_PARAM_NAME "OMX.TI.index.config.mp3headerinfo"
#define FRAME_PARAM_NAME "OMX.TI.index.config.mp3headerinfo"
#define STREAMID_PARAM_NAME "OMX.TI.index.config.mp3streamIDinfo"
#define INPUT_BUFFERSIZE 4 * 1024
#define OUTPUT_BUFFERSIZE 4 * 1024

G_DEFINE_TYPE (GooTiMp3Dec, goo_ti_mp3dec, GOO_TYPE_TI_AUDIO_DECODER)

/* args */
enum
{
	PROP_0,
	PROP_LAYER,
};


static void
goo_ti_mp3dec_init (GooTiMp3Dec* self)
{
	GOO_COMPONENT (self)->id = g_strdup (ID);
	GOO_TI_AUDIO_COMPONENT (self)->dasf_param_name =
		g_strdup (DASF_PARAM_NAME);
	GOO_TI_AUDIO_COMPONENT (self)->frame_param_name =
		g_strdup (FRAME_PARAM_NAME);
	GOO_TI_AUDIO_COMPONENT (self)->streamid_param_name =
		g_strdup (STREAMID_PARAM_NAME);
	GOO_TI_AUDIO_COMPONENT (self)->datapath_param_name =
		g_strdup (DATAPATH_PARAM_NAME);
	self->input_param = NULL;
	self->output_param = NULL;

	return;
}

static void
goo_ti_mp3dec_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_MP3DEC (object));
	GooTiMp3Dec* self = GOO_TI_MP3DEC (object);

	if (G_LIKELY (self->input_param))
	{
		g_free (self->input_param);
		self->input_param = NULL;
	}

	if (G_LIKELY (self->output_param))
	{
		g_free (self->output_param);
		self->output_param = NULL;
	}

	(*G_OBJECT_CLASS (goo_ti_mp3dec_parent_class)->finalize) (object);

	return;
}

static void
goo_ti_mp3dec_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_MP3DEC (component));
	GooTiMp3Dec* self = GOO_TI_MP3DEC (component);
	g_assert (self->input_param == NULL && self->output_param == NULL);
	g_assert (component->cur_state != OMX_StateInvalid);

	GOO_OBJECT_DEBUG (self, "");

	{
		self->input_param = g_new0 (OMX_AUDIO_PARAM_MP3TYPE, 1);
		GOO_INIT_PARAM (self->input_param, OMX_AUDIO_PARAM_MP3TYPE);
		self->input_param->nPortIndex = 0;

		goo_component_get_parameter_by_index (component,
						      OMX_IndexParamAudioMp3,
						      self->input_param);
	}

	{
		self->output_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
		GOO_INIT_PARAM (self->output_param,
				OMX_AUDIO_PARAM_PCMMODETYPE);
		self->output_param->nPortIndex = 1;

		goo_component_get_parameter_by_index (component,
						      OMX_IndexParamAudioPcm,
						      self->output_param);
	}

	return;
}

static void
goo_ti_mp3dec_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_MP3DEC (component));
	GooTiMp3Dec* self = GOO_TI_MP3DEC (component);
	g_assert (self->input_param != NULL && self->output_param != NULL);
	g_assert (component->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "");

	if (goo_ti_audio_component_is_dasf_mode
	    (GOO_TI_AUDIO_COMPONENT (component)))
	{
		goo_component_set_parameter_by_index (component,
						      OMX_IndexParamAudioMp3,
						      self->input_param);
	}
	goo_component_set_parameter_by_index (component,
					      OMX_IndexParamAudioPcm,
					      self->output_param);

	return;
}

static void
goo_ti_mp3dec_validate_ports_definitions (GooComponent* component)
{
	g_assert (GOO_IS_TI_MP3DEC (component));
	GooTiMp3Dec* self = GOO_TI_MP3DEC (component);
	g_assert (self->input_param != NULL && self->output_param != NULL);
	g_assert (component->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "");

	/* input */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		GOO_PORT_GET_DEFINITION (port)->nBufferSize = INPUT_BUFFERSIZE;
		GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding =
			OMX_AUDIO_CodingMP3;

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

		GOO_PORT_GET_DEFINITION (port)->nBufferSize =
			OUTPUT_BUFFERSIZE;
		GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding =
			OMX_AUDIO_CodingPCM;

		g_object_unref (iter);
		g_object_unref (port);
	}

	return;
}

#if 0 /* i think it is not necessary now */
static void
goo_ti_mp3dec_release_buffer (GooComponent* self,
			      OMX_BUFFERHEADERTYPE* buffer)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (buffer != NULL);
	g_assert (self->cur_state == OMX_StateExecuting ||
		  self->cur_state == OMX_StateIdle);

	GooPort* port = GOO_PORT (g_object_ref (buffer->pAppPrivate));

	if (goo_port_is_eos (port) || goo_port_is_tunneled (port))
	{
		g_object_unref (port);
		return;
	}

	if ((buffer->nFlags & OMX_BUFFERFLAG_EOS) == 0x1)
	{
		GOO_OBJECT_INFO (port, "eos found!");
		goo_port_set_eos (port);
	}

	/* We should always push the EOS buffer */
	GOO_OBJECT_NOTICE (self, "OMX_EmptyThisBuffer, 0x%x", buffer);

	if (self->cur_state != OMX_StateIdle)
	{
		GOO_OBJECT_LOCK (self);
		GOO_RUN (
			OMX_EmptyThisBuffer (self->handle, buffer)
			);
		GOO_OBJECT_UNLOCK (self);
	}
	else
	{
		g_print("INFORMATION (workaround): Implementing workaround "
			"for DR OMAPS00143083\n");
	}

	g_object_unref (port);

	GOO_OBJECT_DEBUG (self, "");

	return;
}
#endif

void _goo_ti_mp3dec_set_layer(GooTiMp3Dec *self, guint layer)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (layer == 2 || layer == 3);
	GooTiAudioComponent *component = GOO_TI_AUDIO_COMPONENT(self);

	gboolean retval = FALSE;
	component->audioinfo->mpeg1_layer2 = (layer == 2) ? 1 : 0;
	retval = goo_component_set_config_by_name (GOO_COMPONENT (self),
						   component->dasf_param_name,
						   component->audioinfo);
	if (retval == TRUE)
	{
		self->layer = layer;

		GOO_OBJECT_INFO (self, "mpeg1_layer2 = %d",
				 component->audioinfo->mpeg1_layer2);
	}
}

static void
goo_ti_mp3dec_set_property (GObject* object, guint prop_id,
				     const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (object));
	GooTiMp3Dec* self = GOO_TI_MP3DEC (object);

	switch (prop_id)
	{
	case PROP_LAYER:
		_goo_ti_mp3dec_set_layer
			(self, g_value_get_uint (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_mp3dec_get_property (GObject* object, guint prop_id,
				     GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (object));
	GooTiMp3Dec* self = GOO_TI_MP3DEC (object);

	switch (prop_id)
	{
	case PROP_LAYER:
		g_value_set_uint (value, self->layer);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_mp3dec_class_init (GooTiMp3DecClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);
	g_klass->finalize = goo_ti_mp3dec_finalize;
	g_klass->set_property = goo_ti_mp3dec_set_property;
	g_klass->get_property = goo_ti_mp3dec_get_property;

	GParamSpec* spec;
	spec = g_param_spec_uint ("layer", "Mpeg1 layer",
				  "Mpeg1 layer",
				  1, 3, 3, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_LAYER, spec);

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->load_parameters_func = goo_ti_mp3dec_load_parameters;
	o_klass->set_parameters_func = goo_ti_mp3dec_set_parameters;
	o_klass->validate_ports_definition_func =
		goo_ti_mp3dec_validate_ports_definitions;
	/* o_klass->release_buffer_func = goo_ti_mp3dec_release_buffer; */

	return;
}
