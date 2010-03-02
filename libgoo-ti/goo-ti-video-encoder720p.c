/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Copyright (C) 2010 Texas Instruments - http://www.ti.com/
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

#include <goo-ti-video-encoder720p.h>
#include <goo-utils.h>

#include <TIDspOmx.h>

#define ID "OMX.720P.Video.encoder"

enum _GooTiVideoEncoder720pProp
{
	PROP_0,
	PROP_CONTROLRATE,
	PROP_FRAMEINTERVAL
};

#define DEFAULT_CONTROL_RATE GOO_TI_VIDEO_ENCODER720P_CR_VARIABLE
#define DEFAULT_DEBLOCK_FILTER OMX_TRUE
#define DEFAULT_QPI 7
#define DEFAULT_FRAME_INTERVAL 30
#define DEFAULT_OUTPUT_BUFFER_SIZE 300000

G_DEFINE_TYPE (GooTiVideoEncoder720p, goo_ti_video_encoder720p, GOO_TYPE_COMPONENT)

GType
goo_ti_video_encoder720p_control_rate_get_type ()
{
	static GType goo_ti_video_encoder720p_control_rate_type = 0;
	static GEnumValue goo_ti_video_encoder720p_control_rate[] =
	{
		{ GOO_TI_VIDEO_ENCODER720P_CR_DISABLE, "0", "Disable" },
		{ GOO_TI_VIDEO_ENCODER720P_CR_VARIABLE, "1", "Variable" },
		{ GOO_TI_VIDEO_ENCODER720P_CR_CONSTANT, "2", "Constant" },
		{ GOO_TI_VIDEO_ENCODER720P_CR_VARIABLE_SKIP, "3", "Variable Skip" },
		{ GOO_TI_VIDEO_ENCODER720P_CR_CONSTANT_SKIP, "4", "Constant Skip" },
		{ 0, NULL, NULL },
	};

	if (!goo_ti_video_encoder720p_control_rate_type)
	{
		goo_ti_video_encoder720p_control_rate_type = g_enum_register_static
			("GooTiVideoEncoder720pControlRate",
			 goo_ti_video_encoder720p_control_rate);
	}

	return goo_ti_video_encoder720p_control_rate_type;
}

static void
goo_ti_video_encoder720p_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_VIDEO_ENCODER720P (component));
	GooTiVideoEncoder720p* self = GOO_TI_VIDEO_ENCODER720P (component);

	GOO_OBJECT_DEBUG (self, "Entering");

	g_assert (self->control_rate_param == NULL);
	g_assert (component->cur_state != OMX_StateInvalid);

	self->control_rate_param = g_new0 (OMX_VIDEO_PARAM_BITRATETYPE, 1);
	GOO_INIT_PARAM (self->control_rate_param, OMX_VIDEO_PARAM_BITRATETYPE);

	self->control_rate_param->nPortIndex = 1;
	goo_component_get_parameter_by_index (component,
					      OMX_IndexParamVideoBitrate,
					      self->control_rate_param);


	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}


static gboolean
_goo_ti_video_encoder720p_set_frame_interval (GooTiVideoEncoder720p* self, guint frame_interval)
{
	g_assert (self != NULL);

	/* known issue */
	g_assert (GOO_COMPONENT (self)->cur_state == OMX_StateExecuting);

	GOO_OBJECT_DEBUG (self, "Entering");

	gboolean retval = FALSE;
	guint value;

	if (self->control_rate_param->eControlRate == OMX_Video_ControlRateVariable)
	{
		value = frame_interval;
	}
	else
	{
		value = 0;
	}

	retval = goo_component_set_config_by_name (GOO_COMPONENT (self),
						   "OMX.TI.VideoEncode.Config.IntraFrameInterval",
						   &value);

	if (retval == TRUE)
	{
		self->frame_interval = value;
		GOO_OBJECT_INFO (self, "Frame Interval = %d", value);
	}
	else
	{
		GOO_OBJECT_ERROR (self, "Cannot set intra frame interval");
		g_assert (FALSE);
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return retval;
}

static void
goo_ti_video_encoder720p_validate_ports_definition (GooComponent* component)
{
	g_assert (GOO_IS_TI_VIDEO_ENCODER720P (component));
	GooTiVideoEncoder720p* self = GOO_TI_VIDEO_ENCODER720P (component);

	GOO_OBJECT_DEBUG (self, "Entering");

	g_assert (self->control_rate_param != NULL);
	g_assert (component->cur_state == OMX_StateLoaded);

	/* param */
	{
		g_assert (self->control_rate_param->eControlRate ==
			  GOO_TI_VIDEO_ENCODER720P_CR_VARIABLE &&
			  self->frame_interval == DEFAULT_FRAME_INTERVAL);
	}

	/* input */
	{
		OMX_PARAM_PORTDEFINITIONTYPE *param;

		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		param = GOO_PORT_GET_DEFINITION (port);

		OMX_COLOR_FORMATTYPE color_format;

		color_format = param->format.video.eColorFormat;

		switch (color_format)
		{
		case OMX_COLOR_FormatCbYCrY:
		case OMX_COLOR_FormatYCbYCr:
		case OMX_COLOR_Format16bitRGB565:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		case OMX_COLOR_FormatYUV420PackedPlanar:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 3 / 2;
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
		OMX_PARAM_PORTDEFINITIONTYPE *param;

		GooIterator* iter =
			goo_component_iterate_output_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		param = GOO_PORT_GET_DEFINITION (port);


		param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight / 2;

		/*param->nBufferSize = DEFAULT_OUTPUT_BUFFER_SIZE;*/

		g_object_unref (iter);
		g_object_unref (port);
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}


static void
goo_ti_video_encoder720p_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_VIDEO_ENCODER720P (component));
	GooTiVideoEncoder720p* self = GOO_TI_VIDEO_ENCODER720P (component);

	GOO_OBJECT_DEBUG (self, "Entering");

	g_assert (component->cur_state == OMX_StateLoaded);

	goo_component_set_parameter_by_index (component,
					      OMX_IndexParamVideoBitrate,
					      self->control_rate_param);

	{
		OMX_VIDEO_PARAM_QUANTIZATIONTYPE *param;
		param = g_new0 (OMX_VIDEO_PARAM_QUANTIZATIONTYPE, 1);
		GOO_INIT_PARAM (param, OMX_VIDEO_PARAM_QUANTIZATIONTYPE);

		param->nPortIndex = 1;
		goo_component_get_parameter_by_index (component,
						      OMX_IndexParamVideoQuantization,
						      param);

		param->nQpI = DEFAULT_QPI;

		goo_component_set_parameter_by_index (component,
						      OMX_IndexParamVideoQuantization,
						      param);

		GOO_OBJECT_INFO (self, "QPI = %d", param->nQpI);

		g_free (param);

	}

	{
		guint value;
		value = DEFAULT_DEBLOCK_FILTER;

		goo_component_set_parameter_by_name (component,
						     "OMX.TI.VideoEncode.Param.DeblockFilter",
						     &value);

		GOO_OBJECT_INFO (self, "Deblock Filter = %d", value);
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return;

}

static void
goo_ti_video_encoder720p_init (GooTiVideoEncoder720p* self)
{
	GOO_COMPONENT (self)->port_param_type = OMX_IndexParamVideoInit;
	GOO_COMPONENT (self)->id = g_strdup (ID);

	self->frame_interval = DEFAULT_FRAME_INTERVAL;
	self->control_rate_param = NULL;

	return;
}

static void
goo_ti_video_encoder720p_set_property (GObject* object, guint prop_id,
				   const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_VIDEO_ENCODER720P (object));
	GooTiVideoEncoder720p* self = GOO_TI_VIDEO_ENCODER720P (object);

	GOO_OBJECT_DEBUG (self, "Entering");

	switch (prop_id)
	{
	case PROP_CONTROLRATE:
		self->control_rate_param->eControlRate = g_value_get_enum (value);
		break;
	case PROP_FRAMEINTERVAL:
		_goo_ti_video_encoder720p_set_frame_interval (self, g_value_get_uint (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

static void
goo_ti_video_encoder720p_get_property (GObject* object, guint prop_id,
				   GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_VIDEO_ENCODER720P (object));
	GooTiVideoEncoder720p* self = GOO_TI_VIDEO_ENCODER720P (object);

	GOO_OBJECT_DEBUG (self, "Entering");

	switch (prop_id)
	{
	case PROP_CONTROLRATE:
		g_value_set_enum (value, self->control_rate_param->eControlRate);
		break;
	case PROP_FRAMEINTERVAL:
		g_value_set_uint (value, self->frame_interval);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

static void
goo_ti_video_encoder720p_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_VIDEO_ENCODER720P (object));
	GooTiVideoEncoder720p* self = GOO_TI_VIDEO_ENCODER720P (object);

	GOO_OBJECT_DEBUG (self, "Entering");

	if (G_LIKELY (self->control_rate_param))
	{
		GOO_OBJECT_DEBUG (self, "Freeing the control rate structure");

		g_free (self->control_rate_param);
		self->control_rate_param = NULL;
	}

	(*G_OBJECT_CLASS (goo_ti_video_encoder720p_parent_class)->finalize) (object);

	return;
}

static void
goo_ti_video_encoder720p_class_init (GooTiVideoEncoder720pClass* klass)
{

	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->set_property = goo_ti_video_encoder720p_set_property;
	g_klass->get_property = goo_ti_video_encoder720p_get_property;
	g_klass->finalize = goo_ti_video_encoder720p_finalize;

	GParamSpec* spec;
	spec = g_param_spec_enum ("control-rate", "Control Rate",
				  "Configures rate control",
				  GOO_TI_VIDEO_ENCODER720P_CONTROL_RATE,
				  DEFAULT_CONTROL_RATE, G_PARAM_READWRITE);

	g_object_class_install_property (g_klass, PROP_CONTROLRATE, spec);

	spec = g_param_spec_uint ("frame-interval", "Intra Frame Interval",
				  "Configures Intra Frame Interval",
				  0, G_MAXUINT, DEFAULT_FRAME_INTERVAL,
				  G_PARAM_READWRITE);

	g_object_class_install_property (g_klass, PROP_FRAMEINTERVAL, spec);

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->set_parameters_func = goo_ti_video_encoder720p_set_parameters;
	o_klass->load_parameters_func = goo_ti_video_encoder720p_load_parameters;
	o_klass->validate_ports_definition_func =
		goo_ti_video_encoder720p_validate_ports_definition;
	/* o_klass->set_state_idle_func = goo_ti_video_encoder720p_set_state_idle; */
	return;
}

/**
 * goo_ti_video_encoder720p_set_control_rate:
 * @self: An #GooTiVideoEncoder instance
 * @control_rate: a GooTiVideoEncoderControlRate value
 *
 * This method will set the type of frame streaming on
 * component.
 **/
void
goo_ti_video_encoder720p_set_control_rate (GooTiVideoEncoder720p* self,
					 GooTiVideoEncoder720pControlRate control_rate)
{
	g_assert (self != NULL);

	g_object_set (G_OBJECT (self), "control-rate", control_rate, NULL);

	return;
}

/**
 * goo_ti_video_encoder720p_get_control_rate:
 * @self: An #GooTiVideoEncoder720p instance
 *
 * Return the mode of frame processing operation.
 *
 * Return value: GooTiVideoEncoder720pControlRate enum value.
 **/
GooTiVideoEncoder720pControlRate
goo_ti_video_encoder720p_get_control_rate (GooTiVideoEncoder720p* self)
{
	g_assert (self != NULL);

	GooTiVideoEncoder720pControlRate retval;
	g_object_get (G_OBJECT (self), "control-rate", &retval, NULL);

	return retval;
}
