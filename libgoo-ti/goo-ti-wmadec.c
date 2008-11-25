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

#include <goo-ti-wmadec.h>
#include <goo-utils.h>
#include <stdio.h>
#include <stdlib.h>

#define ID "OMX.TI.WMA.decode"
#define DATAPATH_PARAM_NAME "OMX.TI.index.config.wmadec.datapath"
#define DASF_PARAM_NAME "OMX.TI.index.config.wmaheaderinfo"
#define FRAME_PARAM_NAME "OMX.TI.index.config.wmaheaderinfo"
#define STREAMID_PARAM_NAME "OMX.TI.index.config.wmastreamIDinfo"
#define INPUT_BUFFERSIZE (4096 * 2)
#define OUTPUT_BUFFERSIZE (4096 * 4)


G_DEFINE_TYPE (GooTiWmaDec, goo_ti_wmadec, GOO_TYPE_TI_AUDIO_DECODER)

enum _GooTiWmaDecProp
{
        PROP_0,
        PROP_WMATXTFILE
};

static void
goo_ti_wmadec_init (GooTiWmaDec* self)
{
        GOO_COMPONENT (self)->id = g_strdup (ID);
        GOO_TI_AUDIO_COMPONENT (self)->dasf_param_name =
                g_strdup (DASF_PARAM_NAME);
        GOO_TI_AUDIO_COMPONENT (self)->frame_param_name =
                g_strdup (FRAME_PARAM_NAME);
        GOO_TI_AUDIO_COMPONENT (self)->datapath_param_name =
                g_strdup (DATAPATH_PARAM_NAME);
        GOO_TI_AUDIO_COMPONENT (self)->streamid_param_name =
                g_strdup (STREAMID_PARAM_NAME);

        self->input_param = NULL;
        self->output_param = NULL;
	GOO_TI_AUDIO_COMPONENT (self)->audioinfo->wmaHeaderInfo = g_new0(WMA_HeadInfo, 1);

        return;
}

static void
goo_ti_wmadec_finalize (GObject* object)
{
        g_assert (GOO_IS_TI_WMADEC (object));
        GooTiWmaDec* self = GOO_TI_WMADEC (object);

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

        if (G_LIKELY (GOO_TI_AUDIO_COMPONENT (self)->audioinfo->wmaHeaderInfo))
        {
                g_free (GOO_TI_AUDIO_COMPONENT (self)->audioinfo->wmaHeaderInfo);
                GOO_TI_AUDIO_COMPONENT (self)->audioinfo->wmaHeaderInfo = NULL;
        }

        (*G_OBJECT_CLASS (goo_ti_wmadec_parent_class)->finalize) (object);

        return;
}

static void
goo_ti_wmadec_load_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_WMADEC (component));
        GooTiWmaDec* self = GOO_TI_WMADEC (component);
        g_assert (self->input_param == NULL && self->output_param == NULL);
        g_assert (component->cur_state != OMX_StateInvalid);

        GOO_OBJECT_DEBUG (self, "");

        self->input_param = g_new0 (OMX_AUDIO_PARAM_WMATYPE, 1);
        GOO_INIT_PARAM (self->input_param, OMX_AUDIO_PARAM_WMATYPE);

        self->output_param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
        GOO_INIT_PARAM (self->output_param, OMX_AUDIO_PARAM_PCMMODETYPE);

		goo_component_get_parameter_by_index (component,
											  OMX_IndexParamAudioWma,
											  self->input_param);

		goo_component_get_parameter_by_index (component,
											  OMX_IndexParamAudioPcm,
											  self->output_param);

        goo_ti_audio_component_audio_manager_activate (GOO_TI_AUDIO_COMPONENT (component));

        return;
}

static void
goo_ti_wmadec_set_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_WMADEC (component));
        GooTiWmaDec* self = GOO_TI_WMADEC (component);
        g_assert (G_LIKELY(self->input_param) && G_LIKELY(self->output_param));
        g_assert (component->cur_state == OMX_StateLoaded);

        GOO_OBJECT_DEBUG (self, "");

        goo_component_set_parameter_by_index (component,
                                              OMX_IndexParamAudioWma,
                                              self->input_param);

        goo_component_set_parameter_by_index (component,
                                              OMX_IndexParamAudioPcm,
                                              self->output_param);

        return;
}

static void
_goo_ti_wmadec_set_header_info (GooTiWmaDec *self)
{
	g_assert (self != NULL);

	goo_component_set_config_by_name (GOO_COMPONENT(self),
		GOO_TI_AUDIO_COMPONENT (self)->dasf_param_name,
		GOO_TI_AUDIO_COMPONENT (self)->audioinfo);

}

static void
goo_ti_wmadec_validate_ports_definitions (GooComponent* component)
{
        g_assert (GOO_IS_TI_WMADEC (component));
        GooTiWmaDec* self = GOO_TI_WMADEC (component);
        g_assert (G_LIKELY(self->input_param) && G_LIKELY(self->output_param));
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
                        OMX_AUDIO_CodingWMA;

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

                self->output_param->nChannels = self->input_param->nChannels;
                self->output_param->nSamplingRate = self->input_param->nBitRate;

                g_object_unref (iter);
                g_object_unref (port);
        }

        self->input_param->eFormat = OMX_AUDIO_WMAFormat9;

	_goo_ti_wmadec_set_header_info (self);

        return;
}

static void
_goo_ti_wmadec_set_wma_txt_file(GooTiWmaDec *self,
                               const gchar *wmatxtfile)
{
        GOO_OBJECT_DEBUG(self, "");
        WMA_HeadInfo* pHeaderInfo;

        GOO_OBJECT_DEBUG (self, "");
        OMX_U32 temp;

        FILE *parameterFile;
        parameterFile = fopen(wmatxtfile,"r");
        g_assert(G_LIKELY(parameterFile));

        pHeaderInfo = GOO_TI_AUDIO_COMPONENT (self)->audioinfo->wmaHeaderInfo;

        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iPackets.dwHi = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iPackets.dwLo = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iPlayDuration.dwHi = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iPlayDuration.dwLo = temp;

        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iMaxPacketSize = temp;

        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data1 = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data2 = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data3 = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data4[0] = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data4[1] = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data4[2] = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data4[3] = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data4[4] = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data4[5] = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data4[6] = temp;
        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iStreamType.Data4[7] = temp;

        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iTypeSpecific = temp ;
        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iStreamNum = temp    ;

        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iFormatTag = temp    ;
        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iSamplePerSec = temp ;
        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iAvgBytesPerSec  = temp ;
        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iBlockAlign = temp;
        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iChannel = temp;
        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iValidBitsPerSample =  temp;

        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iSizeWaveHeader =  temp;

        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iChannelMask =  temp;
        fscanf(parameterFile, "%lu",&temp);
        pHeaderInfo->iEncodeOptV =  temp;

        fscanf(parameterFile, "%lu",&temp);
//        pHeaderInfo->iSamplePerBlock =  temp;
        fclose(parameterFile);

        goo_component_set_config_by_name (GOO_COMPONENT(self),
                                GOO_TI_AUDIO_COMPONENT (self)->dasf_param_name,
                                GOO_TI_AUDIO_COMPONENT (self)->audioinfo);
}

static void
goo_ti_wmadec_set_property (GObject* object, guint prop_id,
                                     const GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_WMADEC (object));
        GooTiWmaDec* self = GOO_TI_WMADEC (object);

        switch (prop_id)
        {
                        case PROP_WMATXTFILE:
                                _goo_ti_wmadec_set_wma_txt_file (self,
                                        g_value_get_string (value));
                                break;

                        default:
                                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
                                break;
        }

        return;
}

static void
goo_ti_wmadec_class_init (GooTiWmaDecClass* klass)
{
        GObjectClass* g_klass = G_OBJECT_CLASS (klass);
        g_klass->finalize = goo_ti_wmadec_finalize;
	g_klass->set_property = goo_ti_wmadec_set_property;

        GParamSpec* spec;
        spec = g_param_spec_string ("wmatxtfile", "Header info file",
                                     "Header text info file",
                                     "", G_PARAM_WRITABLE);
        g_object_class_install_property (g_klass, PROP_WMATXTFILE, spec);

        GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
        o_klass->load_parameters_func = goo_ti_wmadec_load_parameters;
        o_klass->set_parameters_func = goo_ti_wmadec_set_parameters;
        o_klass->validate_ports_definition_func =
                goo_ti_wmadec_validate_ports_definitions;

        return;
}
