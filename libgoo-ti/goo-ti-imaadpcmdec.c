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

#include <goo-ti-imaadpcmdec.h>
#include <goo-utils.h>
#include <stdio.h>
#include <stdlib.h>

#define ID "OMX.TI.IMA.decode"
#define DATAPATH_PARAM_NAME "OMX.TI.index.config.IMAdec.datapath"
#define DASF_PARAM_NAME "OMX.TI.index.config.IMAheaderinfo"
#define FRAME_PARAM_NAME "OMX.TI.index.config.IMAheaderinfo"
#define STREAMID_PARAM_NAME "OMX.TI.index.config.IMAstreamIDinfo"

G_DEFINE_TYPE (GooTiImaAdPcmDec, goo_ti_imaadpcmdec, GOO_TYPE_TI_AUDIO_DECODER)

enum _GooTiImaAdPcmDecProp
{
        PROP_0,
};

static void
goo_ti_imaadpcmdec_init (GooTiImaAdPcmDec* self)
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
goo_ti_imaadpcmdec_finalize (GObject* object)
{
        g_assert (GOO_IS_TI_IMAADPCMDEC (object));
        GooTiImaAdPcmDec* self = GOO_TI_IMAADPCMDEC (object);

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

        (*G_OBJECT_CLASS (goo_ti_imaadpcmdec_parent_class)->finalize) (object);

        return;
}

static void
goo_ti_imaadpcmdec_load_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_IMAADPCMDEC (component));
        GooTiImaAdPcmDec* self = GOO_TI_IMAADPCMDEC (component);
        g_assert (self->input_param == NULL);
        g_assert (component->cur_state != OMX_StateInvalid);

        GOO_OBJECT_DEBUG (self, "");

		{
			self->input_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
			GOO_INIT_PARAM (self->input_param, OMX_AUDIO_PARAM_PCMMODETYPE);

			goo_component_get_parameter_by_index (component,
												OMX_IndexParamAudioPcm,
												self->input_param);

			self->input_param->nPortIndex = OMX_DirInput;
		}
		
		{
			self->output_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
			GOO_INIT_PARAM (self->output_param, OMX_AUDIO_PARAM_PCMMODETYPE);

			goo_component_get_parameter_by_index (component,
												OMX_IndexParamAudioPcm,
												self->output_param);

			self->output_param->nPortIndex = OMX_DirOutput;
		}
		
        goo_ti_audio_component_audio_manager_activate (GOO_TI_AUDIO_COMPONENT (component));

        return;
}

static void
goo_ti_imaadpcmdec_set_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_IMAADPCMDEC (component));
        GooTiImaAdPcmDec* self = GOO_TI_IMAADPCMDEC (component);
        g_assert (G_LIKELY(self->input_param));
		g_assert (G_LIKELY(self->output_param));
        g_assert (component->cur_state == OMX_StateLoaded);

        GOO_OBJECT_DEBUG (self, "");

        goo_component_set_parameter_by_index (component,
                                              OMX_IndexParamAudioPcm,
                                              self->input_param);
											  
		goo_component_set_parameter_by_index (component,
                                              OMX_IndexParamAudioPcm,
                                              self->output_param);

        return;
}

static void
goo_ti_imaadpcmdec_validate_ports_definitions (GooComponent* component)
{
        g_assert (GOO_IS_TI_IMAADPCMDEC (component));
        GooTiImaAdPcmDec* self = GOO_TI_IMAADPCMDEC (component);
        g_assert (G_LIKELY(self->input_param));
		g_assert (G_LIKELY(self->output_param));
        g_assert (component->cur_state == OMX_StateLoaded);
		guint block_align;
		guint channels;
		
		block_align = GOO_TI_IMAADPCMDEC_GET_INPUT_PARAM (component)->nBitPerSample;
		channels = GOO_TI_IMAADPCMDEC_GET_INPUT_PARAM (component)->nChannels;

        GOO_OBJECT_DEBUG (self, "");

        /* input */
        {
                GooIterator* iter = goo_component_iterate_input_ports (component);
                goo_iterator_nth (iter, 0);
                GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
                g_assert (port != NULL);
				
                GOO_PORT_GET_DEFINITION (port)->nBufferSize = block_align;
                GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding = OMX_AUDIO_CodingADPCM;
				
                g_object_unref (iter);
                g_object_unref (port);
        }

        /* output */
        {
                GooIterator* iter = goo_component_iterate_output_ports (component);
                goo_iterator_nth (iter, 0);
                GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
                g_assert (port != NULL);
				
                GOO_PORT_GET_DEFINITION (port)->nBufferSize = (((2*block_align)/channels)-7)*2;
                GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding = OMX_AUDIO_CodingPCM;

                g_object_unref (iter);
                g_object_unref (port);
        }

        return;
}

static void
goo_ti_imaadpcmdec_set_property (GObject* object, guint prop_id, const GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_AUDIO_COMPONENT (object));
        //GooTiAudioComponent* self = GOO_TI_AUDIO_COMPONENT (object);

        switch (prop_id)
        {
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
                        break;
        }

        return;
}

static void
goo_ti_imaadpcmdec_class_init (GooTiImaAdPcmDecClass* klass)
{
        GObjectClass* g_klass = G_OBJECT_CLASS (klass);
        g_klass->finalize = goo_ti_imaadpcmdec_finalize;
	    g_klass->set_property = goo_ti_imaadpcmdec_set_property;

        //GParamSpec* spec;
        GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
        o_klass->load_parameters_func = goo_ti_imaadpcmdec_load_parameters;
        o_klass->set_parameters_func = goo_ti_imaadpcmdec_set_parameters;
        o_klass->validate_ports_definition_func = goo_ti_imaadpcmdec_validate_ports_definitions;

        return;
}
