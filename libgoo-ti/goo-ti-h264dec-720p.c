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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-ti-h264dec-720p.h>
#include <goo-utils.h>

#define DEFAULT_NALU_BYTES_TYPE GOO_TI_H264DEC720P_NALU_BYTES_TYPE_4B

enum _GooTiH264Dec720pProp
{
        PROP_0,
        PROP_H264BYTETYPE,
};

struct _GooTiH264Dec720pPriv
{
	gint NALU_bytes_type;
	gboolean bBigEnd;
};

#define GOO_TI_H264DEC720P_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GOO_TYPE_TI_H264DEC720P, GooTiH264Dec720pPriv))

#define GOO_TI_H264DEC720P_NALU_BYTE_TYPE \
	(goo_ti_h264dec_720p_NALU_bytes_type_get_type())

static GType
goo_ti_h264dec_720p_NALU_bytes_type_get_type ()
{
	static GType goo_ti_h264dec_720p_NALU_bytes_type_type = 0;
	static GEnumValue goo_ti_h264dec_720p_NALU_bytes_type[] =
	{
		{ GOO_TI_H264DEC720P_NALU_BYTES_TYPE_0B,	"0", "Bitstream mode" },
		{ GOO_TI_H264DEC720P_NALU_BYTES_TYPE_1B,	"1", "NALU 1 bytes wide" },
		{ GOO_TI_H264DEC720P_NALU_BYTES_TYPE_2B,	"2", "NALU 2 bytes wide" },
		{ GOO_TI_H264DEC720P_NALU_BYTES_TYPE_2B,	"3", "NALU 3 bytes wide" },
		{ GOO_TI_H264DEC720P_NALU_BYTES_TYPE_4B,	"4", "NALU 4 bytes wide" },
		{ 0, NULL, NULL },
	};

	if (!goo_ti_h264dec_720p_NALU_bytes_type_type)
	{
		goo_ti_h264dec_720p_NALU_bytes_type_type = g_enum_register_static
			("GooTiH264720pNALUBytesType",
			 goo_ti_h264dec_720p_NALU_bytes_type);
	}

	return goo_ti_h264dec_720p_NALU_bytes_type_type;
}

G_DEFINE_TYPE (GooTiH264Dec720p, goo_ti_h264dec_720p, GOO_TYPE_TI_VIDEO_DECODER720P)

static void
goo_ti_h264dec_720p_validate_ports_definitions (GooComponent* component)
{
	g_assert (GOO_IS_TI_H264DEC720P (component));
	GooTiH264Dec720p* self = GOO_TI_H264DEC720P (component);
	g_assert (component->cur_state == OMX_StateLoaded);

	OMX_PARAM_PORTDEFINITIONTYPE *param;

	GOO_OBJECT_DEBUG (self, "Entering");

	/* input */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		param = GOO_PORT_GET_DEFINITION (port);

		param->format.video.cMIMEType = "video/x-h264";
		param->format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;

		param->format.video.nFrameWidth =
			GOO_ROUND_UP_16 (param->format.video.nFrameWidth);
		param->format.video.nFrameHeight =
			GOO_ROUND_UP_16 (param->format.video.nFrameHeight);

		/* ever? */
		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_FormatCbYCrY:
		case OMX_COLOR_FormatYCbYCr:
		{
			/** @Todo check how this could be fixed
			    We need to specify a bigger blocksize
			    for qcif or smaller files. Corner this **/
			if (param->format.video.nFrameWidth <= 176 ||
			    param->format.video.nFrameHeight <= 144)
			{
				param->nBufferSize =
					param->format.video.nFrameWidth *
					param->format.video.nFrameHeight * 2;
			}
			else
			{
				param->nBufferSize =
					param->format.video.nFrameWidth *
					param->format.video.nFrameHeight * 2;
			}
			break;
		}
		case OMX_COLOR_FormatYUV420PackedPlanar:
			param->nBufferSize =
				(param->format.video.nFrameWidth *
				 param->format.video.nFrameHeight) * 2;
			break;
		default:
			GOO_OBJECT_ERROR (self, "Not valid color format");
			g_assert (FALSE);
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

		param = GOO_PORT_GET_DEFINITION (port);

		param->format.video.nFrameWidth =
			GOO_ROUND_UP_16 (param->format.video.nFrameWidth);
		param->format.video.nFrameHeight =
			GOO_ROUND_UP_16 (param->format.video.nFrameHeight);

		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_FormatCbYCrY:
		case OMX_COLOR_FormatYCbYCr:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		case OMX_COLOR_FormatYUV420PackedPlanar:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 1.5;
			break;
		default:
			GOO_OBJECT_ERROR (self, "Not valid color format");
			g_assert (FALSE);
		}

		param->format.video.cMIMEType = "video/x-raw-yuv";
		param->format.video.eCompressionFormat =
			OMX_VIDEO_CodingUnused;

		/** @todo validate frame size, bitrate and framerate */

		g_object_unref (iter);
		g_object_unref (port);
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

static void
goo_ti_h264dec_720p_set_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_H264DEC720P (component));
		GooTiH264Dec720p* self = GOO_TI_H264DEC720P (component);
		GooTiH264Dec720pPriv* priv = GOO_TI_H264DEC720P_GET_PRIVATE (self);
        g_assert (component->cur_state == OMX_StateLoaded);


        GOO_OBJECT_DEBUG (self, "");

        OMX_INDEXTYPE index, index2;

        GOO_RUN (
                OMX_GetExtensionIndex (component->handle,
                              "OMX.TI.VideoDecode.Param.H264BitStreamFormat",
                              &index)
                );

		GOO_RUN (
			OMX_SetParameter (component->handle,
				index, &priv->NALU_bytes_type)
		);

		GOO_OBJECT_DEBUG (self, "NALU_bytes_type = %d", priv->NALU_bytes_type);

#if 1
		if (priv->NALU_bytes_type)
		{
			priv->bBigEnd = OMX_TRUE;
			GOO_RUN (
				OMX_GetExtensionIndex (component->handle,
					               "OMX.TI.VideoDecode.Param.IsNALBigEndian",
					               &index2)
			);

			GOO_RUN (
				OMX_SetParameter (component->handle,
						  index2, &priv->bBigEnd)
			);

			GOO_OBJECT_DEBUG (self, "NALU_BigEndian = %d", priv->bBigEnd);
		}
#endif

		(*GOO_COMPONENT_CLASS (goo_ti_h264dec_720p_parent_class)->set_parameters_func) (component);

	return;
}

static void
goo_ti_h264dec_720p_set_property (GObject* object, guint prop_id,
                                     const GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_H264DEC720P (object));
        GooTiH264Dec720p* self = GOO_TI_H264DEC720P (object);
		GooTiH264Dec720pPriv* priv =
			GOO_TI_H264DEC720P_GET_PRIVATE (self);

        switch (prop_id)
        {
        case PROP_H264BYTETYPE:
                priv->NALU_bytes_type = g_value_get_enum (value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
                break;
        }

        return;
}


static void
goo_ti_h264dec_720p_get_property (GObject* object, guint prop_id,
                                     GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_H264DEC720P (object));
        GooTiH264Dec720p* self = GOO_TI_H264DEC720P (object);
		GooTiH264Dec720pPriv* priv =
			GOO_TI_H264DEC720P_GET_PRIVATE (self);

        switch (prop_id)
        {
        case PROP_H264BYTETYPE:
                g_value_set_enum (value, priv->NALU_bytes_type);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
                break;
        }

        return;
}



static void
goo_ti_h264dec_720p_init (GooTiH264Dec720p* self)
{
	return;
}

static void
goo_ti_h264dec_720p_class_init (GooTiH264Dec720pClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_type_class_add_private (g_klass, sizeof (GooTiH264Dec720pPriv));
	g_klass->set_property = goo_ti_h264dec_720p_set_property;
	g_klass->get_property = goo_ti_h264dec_720p_get_property;

	GParamSpec* spec = NULL;
	spec = g_param_spec_enum ("NALU-byte-type", "NAL units size",
				  "Selects between 0, 1 2 or 4 bytes",
				  GOO_TI_H264DEC720P_NALU_BYTE_TYPE,
				  DEFAULT_NALU_BYTES_TYPE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_H264BYTETYPE, spec);

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);

	o_klass->validate_ports_definition_func =
		goo_ti_h264dec_720p_validate_ports_definitions;
	o_klass->set_parameters_func =
		goo_ti_h264dec_720p_set_parameters;
	return;
}


