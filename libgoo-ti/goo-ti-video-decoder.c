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

#include <goo-ti-video-decoder.h>
#include <goo-utils.h>

#include <TIDspOmx.h>
#define ID "OMX.TI.Video.Decoder"

enum _GooTiVideoDecoderProp
{
	PROP_0,
	PROP_PROCESSMODE,
};

#define DEFAULT_PROCESS_MODE GOO_TI_VIDEO_DECODER_FRAMEMODE

GType
goo_ti_video_decoder_process_mode_get_type ()
{
	static GType goo_ti_video_decoder_process_mode_type = 0;
	if (!goo_ti_video_decoder_process_mode_type)
	{
		static const GEnumValue values[] = {
			{ GOO_TI_VIDEO_DECODER_FRAMEMODE, "0", "Frame-Mode" },
			{ GOO_TI_VIDEO_DECODER_STREAMMODE, "1", "Stream-Mode" },
			{ 0, NULL, NULL },
		};

		goo_ti_video_decoder_process_mode_type =
			g_enum_register_static ("GooTiVideoDecoderProcessMode",
						values);
	}

	return goo_ti_video_decoder_process_mode_type;
}

static void
goo_ti_video_decoder_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_VIDEO_DECODER (component));
	GooTiVideoDecoder* self = GOO_TI_VIDEO_DECODER (component);
	g_assert (component->cur_state == OMX_StateLoaded);

	OMX_INDEXTYPE index;

	GOO_RUN (
		OMX_GetExtensionIndex (component->handle,
				       "OMX.TI.VideoDecode.Param.ProcessMode",
				       &index)
		);

	GOO_RUN (
		OMX_SetParameter (component->handle,
				  index, &self->process_mode)
		);

	GOO_OBJECT_DEBUG (self, "process_mode = %d", self->process_mode);

	return;
}

G_DEFINE_TYPE (GooTiVideoDecoder, goo_ti_video_decoder, GOO_TYPE_COMPONENT)

static void
goo_ti_video_decoder_init (GooTiVideoDecoder* self)
{
	GOO_COMPONENT (self)->port_param_type = OMX_IndexParamVideoInit;
	GOO_COMPONENT (self)->id = g_strdup (ID);

	self->process_mode = GOO_TI_VIDEO_DECODER_STREAMMODE;

	return;
}

static void
goo_ti_video_decoder_set_property (GObject* object, guint prop_id,
				     const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_VIDEO_DECODER (object));
	GooTiVideoDecoder* self = GOO_TI_VIDEO_DECODER (object);

	switch (prop_id)
	{
	case PROP_PROCESSMODE:
		self->process_mode = g_value_get_enum (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_video_decoder_get_property (GObject* object, guint prop_id,
				     GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_VIDEO_DECODER (object));
	GooTiVideoDecoder* self = GOO_TI_VIDEO_DECODER (object);

	switch (prop_id)
	{
	case PROP_PROCESSMODE:
		g_value_set_enum (value, self->process_mode);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_video_decoder_class_init (GooTiVideoDecoderClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->set_property = goo_ti_video_decoder_set_property;
	g_klass->get_property = goo_ti_video_decoder_get_property;

	GParamSpec* spec;
	spec = g_param_spec_enum ("process-mode", "Process mode",
				  "Stream/Frame mode",
				  GOO_TI_VIDEO_DECODER_PROCESS_MODE,
				  DEFAULT_PROCESS_MODE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_PROCESSMODE, spec);

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->set_parameters_func = goo_ti_video_decoder_set_parameters;

	return;
}

/**
 * goo_ti_video_decoder_set_process_mode:
 * @self: An #GooTiVideoDecoder instance
 * @proces_mode: a GooTiVideoDecoderProcessMode value
 *
 * This method will set frame or stream mode operation on the
 * component.
 **/
void
goo_ti_video_decoder_set_process_mode (GooTiVideoDecoder* self,
				       GooTiVideoDecoderProcessMode process_mode)
{
	g_assert (self != NULL);

	g_object_set (G_OBJECT (self), "process-mode", process_mode, NULL);

	return;
}

/**
 * goo_ti_video_decoder_get_process_mode:
 * @self: An #GooTiVideoDecoder instance
 *
 * Return the mode of frame processing operation.
 *
 * Return value: GooTiVideoDecoderProcessMode enum value.
 **/
GooTiVideoDecoderProcessMode
goo_ti_video_decoder_get_process_mode (GooTiVideoDecoder* self)
{
	g_assert (self != NULL);

	GooTiVideoDecoderProcessMode retval;
	g_object_get (G_OBJECT (self), "process-mode", &retval, NULL);

	return retval;
}


