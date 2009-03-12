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

#include <goo-ti-armaacdec.h>
#include <goo-utils.h>

#define ID "OMX.TI.ARMAAC.decode"
#define DASF_PARAM_NAME "OMX.TI.index.config.armaacdecHeaderInfo"
#define DATAPATH_PARAM_NAME "OMX.TI.index.config.armaacdec.datapath"
#define FRAME_PARAM_NAME "OMX.TI.index.config.armaacdecHeaderInfo"
#define STREAMID_PARAM_NAME "OMX.TI.index.config.armaacdecstreamIDinfo"
#define INPUT_BUFFERSIZE 2000
#define OUTPUT_BUFFERSIZE 8192

G_DEFINE_TYPE (GooTiArmAacDec, goo_ti_armaacdec, GOO_TYPE_TI_AUDIO_DECODER)

static void
goo_ti_armaacdec_init (GooTiArmAacDec* self)
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

        self->input_port_param = NULL;
        self->output_port_param = NULL;

        return;
}

static void
goo_ti_armaacdec_finalize (GObject* object)
{
        g_assert (GOO_IS_TI_ARMAACDEC (object));
        GooTiArmAacDec* self = GOO_TI_ARMAACDEC (object);

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
        (*G_OBJECT_CLASS (goo_ti_armaacdec_parent_class)->finalize) (object);

        return;
}

static void
goo_ti_armaacdec_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_ARMAACDEC (component));
	GooTiArmAacDec* self = GOO_TI_ARMAACDEC (component);
	g_assert (self->input_port_param == NULL);
	g_assert (self->output_port_param == NULL);
	g_assert (component->cur_state != OMX_StateInvalid);

	GOO_OBJECT_DEBUG (self, "");

	self->input_port_param = g_new0 (OMX_AUDIO_PARAM_AACPROFILETYPE, 1);
	GOO_INIT_PARAM (self->input_port_param, OMX_AUDIO_PARAM_AACPROFILETYPE);

	self->output_port_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
	GOO_INIT_PARAM (self->output_port_param, OMX_AUDIO_PARAM_PCMMODETYPE);

	goo_component_get_parameter_by_index (component,
										  OMX_IndexParamAudioAac,
										  self->input_port_param);

	goo_component_get_parameter_by_index (component,
										  OMX_IndexParamAudioPcm,
										  self->output_port_param);

	goo_ti_audio_component_audio_manager_activate (GOO_TI_AUDIO_COMPONENT (component));

	return;
}

static void
goo_ti_armaacdec_set_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_ARMAACDEC (component));
        GooTiArmAacDec* self = GOO_TI_ARMAACDEC (component);
        g_assert (self->input_port_param != NULL);
        g_assert (self->output_port_param != NULL);
        g_assert (component->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "");

	goo_component_set_parameter_by_index (component,
					      OMX_IndexParamAudioAac,
					      self->input_port_param);
	goo_component_set_parameter_by_index (component,
					      OMX_IndexParamAudioPcm,
					      self->output_port_param);
        return;
}

static void
goo_ti_armaacdec_validate_ports_definitions (GooComponent* component)
{
	g_assert (GOO_IS_TI_ARMAACDEC (component));
	GooTiArmAacDec* self = GOO_TI_ARMAACDEC (component);
	g_assert (self->input_port_param != NULL);
	g_assert (self->output_port_param != NULL);
	g_assert (component->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "Enter");

	/* input */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		GOO_PORT_GET_DEFINITION (port)->nBufferSize = INPUT_BUFFERSIZE;
		GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding =
                        OMX_AUDIO_CodingAAC;

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

	GOO_OBJECT_DEBUG (self, "Exit");
	return;
}

static void
goo_ti_armaacdec_class_init (GooTiArmAacDecClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);
	g_klass->finalize = goo_ti_armaacdec_finalize;

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->load_parameters_func = goo_ti_armaacdec_load_parameters;
	o_klass->set_parameters_func = goo_ti_armaacdec_set_parameters;
	o_klass->validate_ports_definition_func =
		goo_ti_armaacdec_validate_ports_definitions;

	return;
}
