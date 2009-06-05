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

#include <goo-ti-gsmfrdec.h>
#include <goo-utils.h>

#define ID "OMX.TI.GSMFR.decode"
#define DATAPATH_PARAM_NAME "OMX.TI.index.config.gsmfr.datapath"
#define DASF_PARAM_NAME "OMX.TI.index.config.gsmfrheaderinfo"
#define FRAME_PARAM_NAME "OMX.TI.index.config.gsmfrheaderinfo"
#define STREAMID_PARAM_NAME "OMX.TI.index.config.gsmfrstreamIDinfo"
#define INPUT_GSMFRDEC_BUFFER_SIZE 102
#define OUTPUT_GSMFRDEC_BUFFER_SIZE 320

G_DEFINE_TYPE (GooTiGsmFrDec, goo_ti_gsmfrdec, GOO_TYPE_TI_AUDIO_DECODER)

enum _GooTiGsmFrDecProp
{
        PROP_0,
};

static void
goo_ti_gsmfrdec_init (GooTiGsmFrDec* self)
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
goo_ti_gsmfrdec_finalize(GObject* object)
{
    g_assert (GOO_IS_TI_GSMFRDEC(object));
    GooTiGsmFrDec* self = GOO_TI_GSMFRDEC (object);

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

    (*G_OBJECT_CLASS (goo_ti_gsmfrdec_parent_class)->finalize)(object);

    return;
}

static void
goo_ti_gsmfrdec_load_parameters(GooComponent* component)
{
    g_assert (GOO_IS_TI_GSMFRDEC(component));
    GooTiGsmFrDec* self = GOO_TI_GSMFRDEC (component);
    g_assert (self->input_port_param == NULL);
    g_assert (self->output_port_param == NULL);
    g_assert (component->cur_state != OMX_StateInvalid);

    GOO_OBJECT_DEBUG (self, "");

    GOO_OBJECT_DEBUG(self, "Loading input port");
    self->input_port_param = g_new0 (OMX_AUDIO_PARAM_GSMFRTYPE, 1);
    GOO_INIT_PARAM (self->input_port_param, OMX_AUDIO_PARAM_GSMFRTYPE);

    GOO_OBJECT_DEBUG(self, "Loading output port");
    self->output_port_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
    GOO_INIT_PARAM (self->output_port_param, OMX_AUDIO_PARAM_PCMMODETYPE);

    goo_component_get_parameter_by_index (component,
                                            OMX_IndexParamAudioGsm_FR,
                                            self->input_port_param);

    goo_ti_audio_component_audio_manager_activate (GOO_TI_AUDIO_COMPONENT (component));

    return;
}

static void
goo_ti_gsmfrdec_set_parameters(GooComponent* component)
{
    g_assert (GOO_IS_TI_GSMFRDEC(component));
    GooTiGsmFrDec* self = GOO_TI_GSMFRDEC (component);
    g_assert (G_LIKELY(self->input_port_param));
    g_assert (G_LIKELY(self->output_port_param));
    g_assert (component->cur_state == OMX_StateLoaded);

    GOO_OBJECT_DEBUG (self, "");

#if 0
    goo_component_set_parameter_by_index (component,
                                        OMX_IndexParamAudioGsm_FR,
                                        self->input_port_param);

    goo_component_set_parameter_by_index (component,
                                        OMX_IndexParamAudioPcm,
                                        self->output_port_param);
#endif

    return;
}

static void
goo_ti_gsmfrdec_validate_ports_definitions(GooComponent* component)
{
    g_assert (GOO_IS_TI_GSMFRDEC(component));
    GooTiGsmFrDec* self = GOO_TI_GSMFRDEC (component);
    g_assert (self->input_port_param != NULL);
    g_assert (self->output_port_param != NULL);
    g_assert (component->cur_state == OMX_StateLoaded);

    GOO_OBJECT_DEBUG (self, "Enter");

    /* input */
    {
        GooIterator *iter =
            goo_component_iterate_input_ports (component);
        goo_iterator_nth (iter, 0);
        GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
        g_assert (port != NULL);

        GOO_PORT_GET_DEFINITION (port)->nBufferSize = INPUT_GSMFRDEC_BUFFER_SIZE;
        GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding = OMX_AUDIO_CodingGSMFR;
        GOO_PORT_GET_DEFINITION (port)->format.audio.cMIMEType = "audio/x-adpcm";

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

        GOO_PORT_GET_DEFINITION (port)->nBufferSize = OUTPUT_GSMFRDEC_BUFFER_SIZE;
        GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding = OMX_AUDIO_CodingPCM;
        GOO_PORT_GET_DEFINITION (port)->format.audio.cMIMEType = "audio/x-raw-int";

        g_object_unref (iter);
        g_object_unref (port);
    }

    /* param */
	{
		OMX_AUDIO_PARAM_GSMFRTYPE* param;
		param = GOO_TI_GSMFRDEC_GET_INPUT_PORT_PARAM (component);

		param->nPortIndex = 0; /* setting the input port */
		/* should validate the rest ? */
	}

    GOO_OBJECT_DEBUG (self, "Exit");
    return;
}

static void
goo_ti_gsmfrdec_class_init(GooTiGsmFrDecClass* klass)
{
    GObjectClass* g_klass = G_OBJECT_CLASS (klass);
    g_klass->finalize = goo_ti_gsmfrdec_finalize;

    GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
    o_klass->load_parameters_func = goo_ti_gsmfrdec_load_parameters;
    o_klass->set_parameters_func = goo_ti_gsmfrdec_set_parameters;
    o_klass->validate_ports_definition_func =
            goo_ti_gsmfrdec_validate_ports_definitions;

    return;
}
