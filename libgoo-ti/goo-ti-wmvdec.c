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

#include <goo-ti-wmvdec.h>
#include <goo-utils.h>

#define DEFAULT_FILE_TYPE GOO_TI_WMVDEC_FILE_TYPE_VC1

enum _GooTiWMVDecProp
{
        PROP_0,
        PROP_WMVFILETYPE,
};

struct _GooTiWMVDecPriv
{
	guint file_type;
};

#define GOO_TI_WMVDEC_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GOO_TYPE_TI_WMVDEC, GooTiWMVDecPriv))

#define GOO_TI_WMVDEC_FILE_TYPE \
	(goo_ti_wmvdec_file_type_get_type())

static GType
goo_ti_wmvdec_file_type_get_type ()
{
	static GType goo_ti_wmvdec_file_type_type = 0;
	static GEnumValue goo_ti_wmvdec_file_type[] =
	{
		{ GOO_TI_WMVDEC_FILE_TYPE_VC1,	"0", "VC1 Mode" },
		{ GOO_TI_WMVDEC_FILE_TYPE_RCV,   "1", "RCV Mode" },
		{ 0, NULL, NULL },
	};

	if (!goo_ti_wmvdec_file_type_type)
	{
		goo_ti_wmvdec_file_type_type = g_enum_register_static
			("GooTiWMVDecFileType",
			 goo_ti_wmvdec_file_type);
	}

	return goo_ti_wmvdec_file_type_type;
}


G_DEFINE_TYPE (GooTiWMVDec, goo_ti_wmvdec, GOO_TYPE_TI_VIDEO_DECODER)

static void
goo_ti_wmvdec_validate_ports_definitions (GooComponent* component)
{
	g_assert (GOO_IS_TI_WMVDEC (component));
        GooTiWMVDec* self = GOO_TI_WMVDEC (component);
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

		param->format.video.cMIMEType = "WMV";
		param->format.video.eCompressionFormat = OMX_VIDEO_CodingWMV;
		param->format.video.bFlagErrorConcealment = OMX_FALSE;

		/* ever? */
		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_FormatCbYCrY:
		case OMX_COLOR_FormatYCbYCr:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight / 2;
			break;
		case OMX_COLOR_FormatYUV420Planar:
			param->nBufferSize =
				(param->format.video.nFrameWidth *
				 param->format.video.nFrameHeight * 3) / 8;
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

		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_FormatCbYCrY:
		case OMX_COLOR_FormatYCbYCr:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		case OMX_COLOR_FormatYUV420Planar:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 1.5;
			break;
		default:
			GOO_OBJECT_ERROR (self, "Not valid color format");
			g_assert (FALSE);
		}

		param->format.video.cMIMEType = "YUV";
		param->format.video.pNativeRender = NULL;
		param->format.video.bFlagErrorConcealment = OMX_FALSE;
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
goo_ti_wmvdec_set_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_WMVDEC (component));
        GooTiWMVDec* self = GOO_TI_WMVDEC (component);
		GooTiWMVDecPriv* priv = GOO_TI_WMVDEC_GET_PRIVATE (self);
        g_assert (component->cur_state == OMX_StateLoaded);

        GOO_OBJECT_DEBUG (self, "");

        OMX_INDEXTYPE index;

        GOO_RUN (
                OMX_GetExtensionIndex (component->handle,
                                       "OMX.TI.VideoDecode.Param.WMVFileType",
                                       &index)
                );

        GOO_RUN (
                OMX_SetParameter (component->handle,
                                  index, &priv->file_type)
                );

        GOO_OBJECT_DEBUG (self, "file_type = %d", priv->file_type);

		(*GOO_COMPONENT_CLASS (goo_ti_wmvdec_parent_class)->set_parameters_func) (component);


        return;

}

static void
goo_ti_wmvdec_set_property (GObject* object, guint prop_id,
                                     const GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_WMVDEC (object));
        GooTiWMVDec* self = GOO_TI_WMVDEC (object);
		GooTiWMVDecPriv* priv =
			GOO_TI_WMVDEC_GET_PRIVATE (self);

        switch (prop_id)
        {
        case PROP_WMVFILETYPE:
                priv->file_type = g_value_get_enum (value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
                break;
        }

        return;
}

static void
goo_ti_wmvdec_get_property (GObject* object, guint prop_id,
                                     GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_WMVDEC (object));
        GooTiWMVDec* self = GOO_TI_WMVDEC (object);
		GooTiWMVDecPriv* priv =
			GOO_TI_WMVDEC_GET_PRIVATE (self);

        switch (prop_id)
        {
        case PROP_WMVFILETYPE:
                g_value_set_enum (value, priv->file_type);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
                break;
        }

        return;
}


static void
goo_ti_wmvdec_init (GooTiWMVDec* self)
{
        return;
}

static void
goo_ti_wmvdec_class_init (GooTiWMVDecClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_type_class_add_private (g_klass, sizeof (GooTiWMVDecPriv));
	g_klass->set_property = goo_ti_wmvdec_set_property;
	g_klass->get_property = goo_ti_wmvdec_get_property;

	GParamSpec* spec = NULL;
	spec = g_param_spec_enum ("file-type", "File Type",
				  "Selects between RCV or VC1 type",
				  GOO_TI_WMVDEC_FILE_TYPE,
				  DEFAULT_FILE_TYPE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_WMVFILETYPE, spec);


	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);

	o_klass->validate_ports_definition_func =
		goo_ti_wmvdec_validate_ports_definitions;
	o_klass->set_parameters_func =
		goo_ti_wmvdec_set_parameters;

	return;
}


