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

/**
 * SECTION:goo-ti-camera
 * @short_description: Represent a TI OpenMAX camera component
 * @see_also: #GooComponent
 *
 * The #GooTiCamera is the abstraction of the OpenMAX camera component in
 * the Texas Instrument implementation.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-ti-camera.h>
#include <goo-utils.h>

/* for memcpy */
#include <string.h>

#define ID "OMX.TI.Camera"

#define FOCUS_MODE	 "OMX.TI.Camera.Config.StartFocusMode"

enum _GooTiCameraProp
{
	PROP_0,
	PROP_CONTRAST,
	PROP_BRIGHTNESS,
	PROP_CAPTURE_MODE,
	PROP_BALANCE,
	PROP_EXPOSURE,
	PROP_ZOOM,
	PROP_VSTAB,
	PROP_FOCUS
};

enum _GooTiCameraPorts
{
	PORT_VIEWFINDING = 0x0,
	PORT_CAPTURE,
	PORT_THUMBNAIL
};

struct _GooTiCameraPriv
{
	gboolean capturemode;
	GooTiCameraZoom zoom;
	OMX_WHITEBALCONTROLTYPE balance;
	OMX_EXPOSURECONTROLTYPE exposure;
	OMX_CAMERA_CONFIG_FOCUS_MODE focus;
};

#define GOO_TI_CAMERA_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GOO_TYPE_TI_CAMERA, GooTiCameraPriv))

GType
goo_ti_camera_zoom_get_type ()
{
	static GType type = 0;

	if (type == 0)
	{
		static const GEnumValue values[] = {
			{ GOO_TI_CAMERA_ZOOM_1X, "1x", "No zoom" },
			{ GOO_TI_CAMERA_ZOOM_2X, "2x", "2x zoom" },
			{ GOO_TI_CAMERA_ZOOM_3X, "3x", "3x zoom" },
			{ GOO_TI_CAMERA_ZOOM_4X, "4x", "4x zoom" },
			{ 0, NULL, NULL },
		};

		type = g_enum_register_static ("GooTiCameraZoom", values);
	}

	return type;
}

GType
goo_ti_camera_white_balance_type ()
{
	static GType type = 0;
	if (type == 0)
	{
		static const GEnumValue values[] = {
			{ OMX_WhiteBalControlAuto, "Autobalance", "Autobalance" },
			{ OMX_WhiteBalControlSunLight, "Sunlight", "Sunlight" },
			{ OMX_WhiteBalControlCloudy, "Cloudy", "Cloudy" },
			{ OMX_WhiteBalControlTungsten, "Tungsten", "Tungsten" },
			{ OMX_WhiteBalControlFluorescent, "Fluorescent",  "Fluorescent" },
			{ OMX_WhiteBalControlHorizon, "Horizon", "Horizon" },
			{ 0, NULL, NULL },
		};

		type = g_enum_register_static ("GooTiCameraWhiteBalance", values);
	}

	return type;
}

GType
goo_ti_camera_focus_type ()
{
	static GType type = 0;

	if (type == 0)
	{
		static const GEnumValue values[] = {
			{ OMX_CameraConfigFocusAuto, "Autofocus", "Autofocus" },
			{ OMX_CameraConfigFocusInfinity, "Infinity", "Infinity" },
			{ OMX_CameraConfigFocusHyperfocal, "Hyperfocal", "Hyperfocal" },
			{ OMX_CameraConfigFocusMacro, "Macro", "Macro" },
			{ 0, NULL, NULL },
		};

		type = g_enum_register_static ("GooTiCameraFocus", values);
	}

	return type;
}

GType
goo_ti_camera_exposure_type ()
{
	static GType type = 0;

	if (type == 0)
	{
		static const GEnumValue values[] = {
			{ OMX_ExposureControlOff, "Off", "No exposure" },
			{ OMX_ExposureControlAuto, "Auto", "Auto exposure" },
			{ 0, NULL, NULL },
		};

		type = g_enum_register_static ("GooTiCameraExposure",
					       values);
	}

	return type;
}


/* default values */
#define DEFAULT_EXPOSURE   OMX_ExposureControlAuto
#define DEFAULT_BALANCE    OMX_WhiteBalControlAuto
#define DEFAULT_ZOOM       GOO_TI_CAMERA_ZOOM_1X
#define DEFAULT_CAPTURE    FALSE
#define DEFAULT_BRIGHTNESS 50
#define DEFAULT_CONTRAST   0
#define DEFAULT_VSTAB      FALSE
#define DEFAULT_FOCUS      OMX_CameraConfigFocusAuto

G_DEFINE_TYPE (GooTiCamera, goo_ti_camera, GOO_TYPE_COMPONENT);

/* remember to unref the port after usage */
static GooPort*
goo_ti_camera_get_port (GooTiCamera* self, guint index)
{
	g_assert (self != NULL);
	g_assert (index >= 0);

	GOO_OBJECT_DEBUG (self, "Getting port %d", index);

	GooIterator* iter;
	iter = goo_component_iterate_ports (GOO_COMPONENT (self));
	goo_iterator_nth (iter, index);
	GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
	g_assert (port != NULL);
	g_object_unref (iter);

	return port;
}

#if 0 /* workaround deprecated */
static void
goo_ti_camera_flush_port (GooComponent* self, GooPort* port)
{
	OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;
	param = GOO_PORT_GET_DEFINITION (port);

	gboolean queue_buffers = (param->eDir == OMX_DirOutput &&
				  !goo_port_is_tunneled (port) &&
				  !goo_port_is_disabled (port));


	GOO_OBJECT_DEBUG (port, "Buffer Count Actual = %d",
			  param->nBufferCountActual);

	guint index = param->nPortIndex;

	GOO_OBJECT_LOCK (self);
	GOO_RUN (OMX_SendCommand (self->handle, OMX_CommandFlush, index, 0));
	GOO_OBJECT_UNLOCK (self);

	if (queue_buffers == FALSE)
	{
		return;
	}

	gint count;
	while ((count = g_async_queue_length (port->buffer_queue)) <
	       (param->nBufferCountActual - 1))
	{
		GOO_OBJECT_DEBUG (self, "waiting buffers - %d", count);
		g_usleep (0.5 * G_USEC_PER_SEC);
	}

	return;
}
#endif

static void
_goo_ti_camera_set_contrast (GooTiCamera* self, gint contrast)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	g_return_if_fail (contrast >= -100 && contrast <= 100);

	OMX_CONFIG_CONTRASTTYPE* param;
	param = g_new0 (OMX_CONFIG_CONTRASTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_CONTRASTTYPE);
	param->nContrast = contrast;

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonContrast,
					   param);

	GOO_OBJECT_DEBUG (self, "Contrast = %d", param->nContrast);
	g_free (param);

	return;
}

static gint
_goo_ti_camera_get_contrast (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_CONFIG_CONTRASTTYPE* param;
	param = g_new0 (OMX_CONFIG_CONTRASTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_CONTRASTTYPE);
	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonContrast,
					   param);

	gint retval = param->nContrast;

	GOO_OBJECT_DEBUG (self, "Contrast = %d",retval );

	g_free (param);

	return retval;
}

static void
_goo_ti_camera_set_brightness (GooTiCamera* self, guint brightness)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_return_if_fail (brightness >= -100 && brightness <= 100);

	OMX_CONFIG_BRIGHTNESSTYPE* param;
	param = g_new0 (OMX_CONFIG_BRIGHTNESSTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_BRIGHTNESSTYPE);

	param->nBrightness = brightness;

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonBrightness,
					   param);

	GOO_OBJECT_DEBUG (self, "Brightness= %d", param->nBrightness);

	g_free (param);
	return;
}

static guint
_goo_ti_camera_get_brightness (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_CONFIG_BRIGHTNESSTYPE* param;
	param = g_new0 (OMX_CONFIG_BRIGHTNESSTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_BRIGHTNESSTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonBrightness,
					   param);

	guint retval = param->nBrightness;

	g_free (param);

	GOO_OBJECT_DEBUG (self, "Brightness= %d",retval);

	return retval;
}


static void
_goo_ti_camera_set_focus (GooTiCamera* self, OMX_CAMERA_CONFIG_FOCUS_MODE type)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	gboolean retval = TRUE;
	/* currently implemented controls */
	g_assert (  type == OMX_CameraConfigFocusAuto         ||
		  type == OMX_CameraConfigFocusInfinity     ||
		  type == OMX_CameraConfigFocusHyperfocal        ||
		  type == OMX_CameraConfigFocusMacro );

	OMX_CAMERA_CONFIG_FOCUS_MODE modo;
	modo = type;
	retval = goo_component_set_config_by_name (GOO_COMPONENT (self),
					   FOCUS_MODE,
					   (OMX_PTR*) &modo);
	if (retval == TRUE)
	{
		GooTiCameraPriv* priv = GOO_TI_CAMERA_GET_PRIVATE (self);
		priv->focus = modo;
		GOO_OBJECT_DEBUG (self, "Focus mode = %d", modo);
	}
	return;
}

static OMX_CAMERA_CONFIG_FOCUS_MODE
_goo_ti_camera_get_focus (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiCameraPriv* priv = GOO_TI_CAMERA_GET_PRIVATE (self);
	OMX_CAMERA_CONFIG_FOCUS_MODE modo;
	modo = priv->focus;
	return modo;
}



static OMX_EXPOSURECONTROLTYPE
_goo_ti_camera_get_exposure (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);


	GooTiCameraPriv* priv = GOO_TI_CAMERA_GET_PRIVATE (self);
	OMX_EXPOSURECONTROLTYPE retval;
	retval = priv->exposure;

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}


static void
_goo_ti_camera_set_exposure (GooTiCamera* self,
				  OMX_EXPOSURECONTROLTYPE type)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	/* currently implemented controls */
	g_assert (type == OMX_ExposureControlOff     ||
		  type == OMX_ExposureControlAuto);

	OMX_CONFIG_EXPOSURECONTROLTYPE* param;
	param = g_new0 (OMX_CONFIG_EXPOSURECONTROLTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_EXPOSURECONTROLTYPE);

	param->eExposureControl = type;

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonExposure,
					   param);

	g_free (param);

	GooTiCameraPriv* priv = GOO_TI_CAMERA_GET_PRIVATE (self);
	priv->exposure = type;

	GOO_OBJECT_DEBUG (self, "");

	return;
}


static void
_goo_ti_camera_set_white_balance (GooTiCamera* self,
				  OMX_WHITEBALCONTROLTYPE type)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	/* currently implemented controls */
	g_assert (  type == OMX_WhiteBalControlAuto         ||
		  type == OMX_WhiteBalControlSunLight     ||
		  type == OMX_WhiteBalControlCloudy        ||
		  type == OMX_WhiteBalControlTungsten  ||
		  type == OMX_WhiteBalControlFluorescent ||
		  type == OMX_WhiteBalControlHorizon);

	OMX_CONFIG_WHITEBALCONTROLTYPE* param;
	param = g_new0 (OMX_CONFIG_WHITEBALCONTROLTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_WHITEBALCONTROLTYPE);

	param->eWhiteBalControl = type;

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonWhiteBalance,
					   param);

	g_free (param);

	GooTiCameraPriv* priv = GOO_TI_CAMERA_GET_PRIVATE (self);
	priv->balance = type;

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static OMX_WHITEBALCONTROLTYPE
_goo_ti_camera_get_white_balance (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

#if 0
	OMX_CONFIG_WHITEBALCONTROLTYPE* param;
	param = g_new0 (OMX_CONFIG_WHITEBALCONTROLTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_WHITEBALCONTROLTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonWhiteBalance,
					   param);

	OMX_WHITEBALCONTROLTYPE retval;
	retval = param->eWhiteBalControl;

	g_free (param);
#else
	GooTiCameraPriv* priv = GOO_TI_CAMERA_GET_PRIVATE (self);
	OMX_WHITEBALCONTROLTYPE retval;
	retval = priv->balance;
#endif

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

#if 0 /* not used right now */
static void
_goo_ti_camera_set_zoom_width (GooTiCamera* self, GooTiCameraZoom zoom)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_CONFIG_SCALEFACTORTYPE* param;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonDigitalZoom,
					   param);

	param->xWidth = (OMX_S32) zoom;

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonDigitalZoom,
					   param);

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return;
}
#endif

#if 0 /* not used right now */
static GooTiCameraZoom
_goo_ti_camera_get_zoom_width (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_CONFIG_SCALEFACTORTYPE* param;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonDigitalZoom,
					   param);

	GooTiCameraZoom retval;
	retval = (GooTiCameraZoom) param->xWidth;

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}
#endif

#if 0 /* not used right now */
static void
_goo_ti_camera_set_zoom_height (GooTiCamera* self, GooTiCameraZoom zoom)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_CONFIG_SCALEFACTORTYPE* param;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonDigitalZoom,
					   param);

	param->xHeight = (OMX_S32) zoom;

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonDigitalZoom,
					   param);

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return;
}
#endif

#if 0 /* not used right now */
static GooTiCameraZoom
_goo_ti_camera_get_zoom_height (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_CONFIG_SCALEFACTORTYPE* param;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonDigitalZoom,
					   param);

	GooTiCameraZoom retval;
	retval = (GooTiCameraZoom) param->xHeight;

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}
#endif

static void
_goo_ti_camera_set_zoom (GooTiCamera* self, GooTiCameraZoom zoom)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_CONFIG_SCALEFACTORTYPE* param;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	param->xWidth = (OMX_S32) zoom;
	param->xHeight = (OMX_S32) zoom;

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonDigitalZoom,
					   param);

	g_free (param);

	GooTiCameraPriv* priv = GOO_TI_CAMERA_GET_PRIVATE (self);
	priv->zoom = zoom;

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static GooTiCameraZoom
_goo_ti_camera_get_zoom (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

#if 0
	OMX_CONFIG_SCALEFACTORTYPE* param;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonDigitalZoom,
					   param);

	g_assert (param->xWidth == param->xHeight);

	GooTiCameraZoom retval;
	retval = (GooTiCameraZoom) param->xHeight;

	g_free (param);
#else
	GooTiCameraPriv* priv = GOO_TI_CAMERA_GET_PRIVATE (self);
	GooTiCameraZoom retval;
	retval = priv->zoom;
#endif

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static void
_goo_ti_camera_set_vstab_mode (GooTiCamera* self, gboolean vstab_mode)
{
	g_assert (GOO_IS_TI_CAMERA (self));

	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooPort* port = goo_ti_camera_get_port (self, PORT_VIEWFINDING);
	OMX_PARAM_PORTDEFINITIONTYPE* param;
	param = GOO_PORT_GET_DEFINITION (port);

	if (vstab_mode == TRUE)

	{
		GOO_OBJECT_DEBUG (self, "activating vstab");

		OMX_CONFIG_FRAMESTABTYPE* param;
		param = g_new0 (OMX_CONFIG_FRAMESTABTYPE, 1);
		GOO_INIT_PARAM (param, OMX_CONFIG_FRAMESTABTYPE);

		param->bStab = vstab_mode;

		goo_component_set_config_by_index (GOO_COMPONENT (self),
						   OMX_IndexConfigCommonFrameStabilisation,
						   param);

		g_free (param);
	}

	g_object_unref (port);

	GOO_OBJECT_DEBUG (self, "");

	return;

}

static gboolean
_goo_ti_camera_get_vstab_mode (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	gboolean vstab_mode;
	OMX_CONFIG_FRAMESTABTYPE* param;
	param = g_new0 (OMX_CONFIG_FRAMESTABTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_FRAMESTABTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonFrameStabilisation,
					   &param);

	vstab_mode = param->bStab;

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return vstab_mode;
}


static void
_goo_ti_camera_set_capture_mode (GooTiCamera* self, gboolean capture_mode)
{
	g_assert (self != NULL);

	GooTiCameraPriv *priv = GOO_TI_CAMERA_GET_PRIVATE (self);

	if (GOO_COMPONENT (self)->cur_state == OMX_StateExecuting)
	{
		GOO_OBJECT_DEBUG (self, "prev = %d / new = %d",
			  priv->capturemode, capture_mode);

		OMX_BOOL param = (capture_mode == TRUE) ? OMX_TRUE : OMX_FALSE;
		g_assert (
			goo_component_set_config_by_index (GOO_COMPONENT (self),
						   OMX_IndexConfigCapturing,
						   &param)
		);

	#if 1
		if ((capture_mode == TRUE) && (priv->capturemode == FALSE))
		{
			GOO_OBJECT_INFO (self, "Preparing capture port");

			GooPort* port = goo_ti_camera_get_port (self, PORT_CAPTURE);
			goo_component_prepare_port (GOO_COMPONENT (self), port);
			g_object_unref (port);

			priv->capturemode = TRUE;
		}

		if ((capture_mode == FALSE) && (priv->capturemode == TRUE))
		{
			/** We are not using the thumbnail port at the moment*/
			/**
			GOO_OBJECT_INFO (self, "Preparing thumbnail port");
			GooPort* port = goo_ti_camera_get_port (GOO_TI_CAMERA (self),
							PORT_THUMBNAIL);
			goo_component_prepare_port (GOO_COMPONENT (self), port);
			g_object_unref (port);
			**/
			priv->capturemode = FALSE;
		}
	#endif
	}
	GOO_OBJECT_DEBUG (self, "");
	return;
}

static gboolean
_goo_ti_camera_get_capture_mode (GooTiCamera* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state == OMX_StateExecuting);

	GooTiCameraPriv *priv = GOO_TI_CAMERA_GET_PRIVATE (self);

	return priv->capturemode;
}

static void
goo_ti_camera_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_CAMERA (component));
	GooTiCamera* self = GOO_TI_CAMERA (component);

	g_assert (self->param == NULL);
	g_assert (component->cur_state != OMX_StateInvalid);

	self->param = g_new0 (OMX_PARAM_SENSORMODETYPE, 1);
	GOO_INIT_PARAM (self->param, OMX_PARAM_SENSORMODETYPE);

	goo_component_get_parameter_by_index (component,
					      OMX_IndexParamCommonSensorMode,
					      self->param);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_ti_camera_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_CAMERA (component));
	GooTiCamera* self = GOO_TI_CAMERA (component);

	g_assert (self->param != NULL);
	g_assert (component->cur_state == OMX_StateLoaded);

	goo_component_set_parameter_by_index (component,
					      OMX_IndexParamCommonSensorMode,
					      self->param);

	GOO_OBJECT_DEBUG (self, "set parameters exit");

	return;
}


static void
goo_ti_camera_validate (GooComponent* component)
{
	g_assert (GOO_IS_TI_CAMERA (component));
	GooTiCamera* self = GOO_TI_CAMERA (component);

	g_assert (self->param != NULL);
	g_assert (component->cur_state == OMX_StateLoaded);

	OMX_PARAM_SENSORMODETYPE* sensor;
	sensor = GOO_TI_CAMERA_GET_PARAM (self);

	ResolutionInfo rinfo = goo_get_resolution ("720p");

	/* param */
	{
		/* let's fix the width and the height */
		sensor->sFrameSize.nWidth =
			GOO_ROUND_UP_16 (sensor->sFrameSize.nWidth);
		sensor->sFrameSize.nHeight =
			GOO_ROUND_UP_16 (sensor->sFrameSize.nHeight);
	}

	/* capture port */
	{
		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;

		GooPort* port = goo_ti_camera_get_port (self, PORT_CAPTURE);
		param = GOO_PORT_GET_DEFINITION (port);

		OMX_COLOR_FORMATTYPE color_format;

		if (param->eDomain == OMX_PortDomainImage)
		{
			color_format = param->format.image.eColorFormat;

			/* synch data */
			param->format.image.nFrameWidth =
				sensor->sFrameSize.nWidth;
			param->format.image.nFrameHeight =
				sensor->sFrameSize.nHeight;

			/* the frame rate is always 0 */
			sensor->nFrameRate = 0;

			param->format.image.cMIMEType = "video/x-raw-yuv";

			param->format.image.eCompressionFormat =
			     OMX_IMAGE_CodingUnused;

			param->nBufferCountActual = 1;

			/*param->eDomain = OMX_PortDomainImage;*/
		}
		else
		{
			color_format = param->format.video.eColorFormat;

			/* synch data */
			param->format.video.nFrameWidth =
				sensor->sFrameSize.nWidth;
			param->format.video.nFrameHeight =
				sensor->sFrameSize.nHeight;

			g_assert (sensor->nFrameRate > 0);

			param->format.video.cMIMEType = "video/x-raw-yuv";

			param->format.video.eCompressionFormat =
			     OMX_VIDEO_CodingUnused;
			/*param->eDomain = OMX_PortDomainVideo; */
		}

		switch (color_format)
		{

		case OMX_COLOR_FormatCbYCrY:      			/* UYVY */
		case OMX_COLOR_FormatYCbYCr:				/* YUY2 */
		case OMX_COLOR_Format16bitRGB565:
			param->nBufferSize =
				sensor->sFrameSize.nWidth *
				sensor->sFrameSize.nHeight * 2;
			break;
		case OMX_COLOR_FormatYUV420PackedPlanar:	/*I420*/
			param->nBufferSize =
				sensor->sFrameSize.nWidth *
				sensor->sFrameSize.nHeight * 1.5;
			break;
		default:
			GOO_OBJECT_ERROR (self,
					  "Not valid color format: %s",
					  goo_strcolor (color_format));
			g_assert (FALSE);
		}

		g_object_unref (port);
	}

	/* viewfinding port */
	{
		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;

		GooPort* port = goo_ti_camera_get_port (self,
							PORT_VIEWFINDING);
		param = GOO_PORT_GET_DEFINITION (port);
		/* sync data */
		/* let's use the max available resolution */
		g_assert (param->format.video.nFrameWidth <= rinfo.width);
		g_assert (param->format.video.nFrameHeight <= rinfo.height);

		param->format.video.cMIMEType = "video/x-raw-yuv";

		param->format.video.eCompressionFormat =
			OMX_VIDEO_CodingUnused;

		switch (param->format.video.eColorFormat)
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
				param->format.video.nFrameHeight * 1.5;
			break;
		default:
			GOO_OBJECT_ERROR (self,
					  "Not valid color format: %s",
					  goo_strcolor
					  (param->format.video.eColorFormat));
			g_assert (FALSE);
		}

		g_object_unref (port);
	}

	/* thumbnail port */
	{
		OMX_PARAM_PORTDEFINITIONTYPE *param = NULL;

		GooPort* port = goo_ti_camera_get_port (self, PORT_THUMBNAIL);
		param = GOO_PORT_GET_DEFINITION (port);

		/* sync data */
		/* let's use the max available resolution */
		g_assert (param->format.video.nFrameWidth <= rinfo.width);
		g_assert (param->format.video.nFrameHeight <= rinfo.height);

		param->format.video.cMIMEType = "video/x-raw-yuv";

		param->format.video.eCompressionFormat =
			OMX_VIDEO_CodingUnused;

		switch (param->format.video.eColorFormat)
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
				param->format.video.nFrameHeight * 1.5;
			break;
		default:
			GOO_OBJECT_ERROR (self,
					  "Not valid color format: %s",
					  goo_strcolor
					  (param->format.video.eColorFormat));
			g_assert (FALSE);
		}

		g_object_unref (port);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_ti_camera_get_videoenc:
 * @self: a #GooTiCamera instance
 *
 * If the camera's capture port is tunneled with the videoenc,
 * this function will return the videoencoder instance. After use you
 * must unref the object.
 *
 * Return value: A #GooComponent referencing the GooTiVideoenc instance
 *               or NULL if the view finding port is not tunneled. You must
 *               unref the object once you don't use it anymore.
 */
static GooComponent*
goo_ti_camera_get_enc (GooComponent* self)
{
	GooComponent* retval = NULL;

	GooPort* port = goo_ti_camera_get_port (GOO_TI_CAMERA (self),
						PORT_CAPTURE);

	if (goo_port_is_tunneled (port) && goo_port_is_enabled (port))
	{
		GOO_OBJECT_INFO (self, "capture port is enabled");

		GooPort *peer_port;
		peer_port = goo_port_get_peer (port);

		g_assert (peer_port != NULL);

		retval = GOO_COMPONENT (
			goo_object_get_owner (GOO_OBJECT (peer_port))
			);
		g_assert (retval != NULL);

		g_object_unref (peer_port);
	}

	g_object_unref (port);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

/**
 * goo_ti_camera_get_postproc:
 * @self: a #GooTiCamera instance
 *
 * If the camera's viewfinding port is tunneled with the postprocessor,
 * this function will return the postprocessor instance. After use you
 * must unref the object.
 *
 * Return value: A #GooComponent referencing the GooTiPostprocessor instance
 *               or NULL if the view finding port is not tunneled. You must
 *               unref the object once you don't use it anymore.
 */
static GooComponent*
goo_ti_camera_get_postproc (GooComponent* self)
{
	GooComponent* retval = NULL;

	GooPort* port = goo_ti_camera_get_port (GOO_TI_CAMERA (self),
						PORT_VIEWFINDING);

	if (goo_port_is_tunneled (port) && goo_port_is_enabled (port))
	{
		GOO_OBJECT_INFO (self, "viewfinding port is enabled");

		GooPort *peer_port;
		peer_port = goo_port_get_peer (port);

		g_assert (peer_port != NULL);

		retval = GOO_COMPONENT (
			goo_object_get_owner (GOO_OBJECT (peer_port))
			);
		g_assert (retval != NULL);
		g_assert (GOO_IS_TI_POST_PROCESSOR (retval));

		g_object_unref (peer_port);
	}

	g_object_unref (port);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

/* must send to idle the postprocessor tunnel after camera   */
static void
goo_ti_camera_pp_set_idle (GooComponent* self)
{
	GooComponent *postproc = goo_ti_camera_get_postproc (self);

	if (postproc == NULL)
	{
		return;
	}

	GOO_OBJECT_DEBUG (postproc, "going to set postproc to idle");

	goo_component_set_state_idle (postproc);

	GOO_OBJECT_DEBUG (postproc, "going to wait for next idle state");

	goo_component_wait_for_next_state (postproc);
	g_object_unref (postproc);

	GOO_OBJECT_DEBUG (postproc, "done");

	return;
}


static void
goo_ti_camera_venc_set_idle (GooComponent* self)
{
	GooComponent *videoenc = goo_ti_camera_get_enc (self);

	if (videoenc == NULL)
	{
		return;
	}
	else if(!GOO_IS_TI_VIDEO_ENCODER (videoenc) )
	{
		return;
	}

	GOO_OBJECT_DEBUG (videoenc, "going to set videoenc to idle");

	goo_component_set_state_idle (videoenc);

	GOO_OBJECT_DEBUG (videoenc, "going to wait for next idle state");

	goo_component_wait_for_next_state (videoenc);
	g_object_unref (videoenc);

	GOO_OBJECT_DEBUG (videoenc, "done");

	return;
}

static void
goo_ti_camera_jpeg_set_idle (GooComponent* self)
{

	GooComponent *jpegenc = goo_ti_camera_get_enc (self);

	if (jpegenc == NULL)
	{
		return;
	}
	else if(!GOO_IS_TI_JPEGENC (jpegenc))
	{
		return;
	}

	GOO_OBJECT_DEBUG (jpegenc, "going to set jpegenc to idle");

	goo_component_set_state_idle (jpegenc);

	GOO_OBJECT_DEBUG (jpegenc, "going to wait for next idle state");

	goo_component_wait_for_next_state (jpegenc);

	g_object_unref (jpegenc);

	GOO_OBJECT_DEBUG (jpegenc, "done");

	return;
}
static void
goo_ti_camera_set_state_idle (GooComponent* self)
{
	OMX_STATETYPE tmpstate = self->cur_state;
	self->next_state = OMX_StateIdle;

	gboolean prev_outport_queue = TRUE;

	GOO_OBJECT_DEBUG (self, "tmpstate=%d", tmpstate);

	if (tmpstate == OMX_StateLoaded)
	{
		goo_component_configure (self);
	}
	else if ((tmpstate == OMX_StateExecuting) ||
		 (tmpstate == OMX_StatePause))
	{
		GooPort* port = goo_ti_camera_get_port (GOO_TI_CAMERA (self),
							PORT_CAPTURE);
		prev_outport_queue = goo_port_is_queued (port);
		g_object_set (port, "queued", TRUE, NULL);
		g_object_unref (port);
	}
	else
	{
		/* this state change is not valid */
		g_assert (FALSE);
	}

	if (tmpstate == OMX_StateLoaded)
	{
		goo_ti_camera_pp_set_idle (self);
	}
	if (tmpstate == OMX_StateExecuting)
	{
		goo_ti_camera_venc_set_idle (self);
		goo_ti_camera_jpeg_set_idle (self);
	}
	GOO_OBJECT_DEBUG (self, "going to set camera to idle");
	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SendCommand (self->handle,
				 OMX_CommandStateSet,
				 self->next_state,
				 NULL)
		);
	GOO_OBJECT_UNLOCK (self);

	if (tmpstate == OMX_StateLoaded)
	{
		GOO_OBJECT_INFO (self, "going from loaded->idle.. allocating ports");
		goo_component_allocate_all_ports (self);
	}

	if (tmpstate == OMX_StateExecuting)
	{
		goo_ti_camera_pp_set_idle (self);
	}

	GOO_OBJECT_DEBUG (self, "going to wait_for_next_state");
	goo_component_wait_for_next_state (self);
	GOO_OBJECT_DEBUG (self, "done wait_for_next_state");

	if (tmpstate == OMX_StateExecuting)
	{
		/* unblock all the async_queues */
		goo_component_flush_all_ports (self);

		GooPort* port = goo_ti_camera_get_port (GOO_TI_CAMERA (self),
							PORT_CAPTURE);
		g_object_set (port, "queued", prev_outport_queue, NULL);
		g_object_unref (port);
	}


	if (tmpstate == OMX_StateLoaded)
	{
		goo_ti_camera_venc_set_idle (self);
		goo_ti_camera_jpeg_set_idle (self);
	}

	GOO_OBJECT_DEBUG (self, "done");

	return;
}


/* must send to idle the postprocessor tunnel after camera   */
static void
goo_ti_camera_pp_set_loaded (GooComponent* self)
{

	GooComponent *postproc = goo_ti_camera_get_postproc (self);

	if (postproc == NULL)
	{
		return;
	}

	GOO_OBJECT_INFO (postproc, "Sending loaded state command to post proc");

	/* Sending postprocessor to idle state */
	goo_component_set_state_loaded (postproc);

	GOO_OBJECT_INFO (postproc, "going to wait for loaded state");
	goo_component_wait_for_next_state (postproc);
	g_object_unref (postproc);

	return;
}


static void
goo_ti_camera_venc_set_loaded (GooComponent* self)
{
	GooComponent *videoenc = goo_ti_camera_get_enc (self);

	if (videoenc == NULL)
	{
		return;
	}
	else if(!GOO_IS_TI_VIDEO_ENCODER (videoenc) )
	{
		return;
	}

	GOO_OBJECT_INFO (videoenc, "Sending loaded state command to videoenc");

	goo_component_set_state_loaded (videoenc);

	GOO_OBJECT_INFO (videoenc, "going to wait for loaded state");
	goo_component_wait_for_next_state (videoenc);
	g_object_unref (videoenc);
	return;
}

static void
goo_ti_camera_jpeg_set_loaded (GooComponent* self)
{
	GooComponent *jpegenc = goo_ti_camera_get_enc (self);

	if (jpegenc == NULL)
	{
		return;
	}

	else if(!GOO_IS_TI_JPEGENC (jpegenc) )
	{
		return;
	}

	GOO_OBJECT_INFO (jpegenc, "Sending loaded state command to jpegenc");
	goo_component_set_state_loaded (jpegenc);
	GOO_OBJECT_INFO (jpegenc, "going to wait for loaded state");
	goo_component_wait_for_next_state (jpegenc);
	g_object_unref (jpegenc);
	return;
}

static void
goo_ti_camera_set_state_loaded (GooComponent* self)
{
	self->next_state = OMX_StateLoaded;

	goo_ti_camera_pp_set_loaded (self);

	GOO_OBJECT_INFO (self, "Sending load state command to camera");
	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SendCommand (self->handle,
				 OMX_CommandStateSet,
				 self->next_state,
				 NULL)
		);
	GOO_OBJECT_UNLOCK (self);

	goo_ti_camera_venc_set_loaded (self);
	goo_ti_camera_jpeg_set_loaded (self);

	if (self->cur_state == OMX_StateIdle &&
	    self->next_state == OMX_StateLoaded)
	{
		GOO_OBJECT_INFO (self, "going to deallocate ports");
		goo_component_deallocate_all_ports (self);
		GOO_OBJECT_INFO (self, "done deallocate ports");
	}

	GOO_OBJECT_INFO (self, "going to wait for next state");
	goo_component_propagate_wait_for_next_state (self);
	GOO_OBJECT_INFO (self, "done");

	self->configured = FALSE;

	return;
}

/*
 * Check if output ports are in tunnel mode. If so, send peer components
 * to EXECUTING state recursively
 */
static void
goo_ti_camera_propagate_executing (GooComponent* self)
{
	GooComponent* postproc = goo_ti_camera_get_postproc (self);

	if (postproc != NULL)
	{
		GOO_OBJECT_INFO (postproc, "Sending executing state command");

		goo_component_set_state_executing (postproc);

		goo_component_wait_for_next_state (postproc);

		g_object_unref (postproc);
	}

	GooComponent* videoenc = goo_ti_camera_get_enc (self);

	if (videoenc != NULL)
	{
		GOO_OBJECT_INFO (videoenc, "Sending executing state command");

		goo_component_set_state_executing (videoenc);

		goo_component_wait_for_next_state (videoenc);

		g_object_unref (videoenc);
	}

	GooComponent* jpegenc = goo_ti_camera_get_enc (self);

	if (jpegenc != NULL)
	{
		GOO_OBJECT_INFO (jpegenc, "Sending executing state command");

		goo_component_set_state_executing (jpegenc);

		goo_component_wait_for_next_state (jpegenc);

		g_object_unref (jpegenc);
	}

	return;
}

static void
goo_ti_camera_set_state_executing (GooComponent* self)
{
	self->next_state = OMX_StateExecuting;

	GOO_OBJECT_INFO (self, "Sending executing state command");

	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SendCommand (self->handle,
				 OMX_CommandStateSet,
				 self->next_state,
				 NULL)
		);
	GOO_OBJECT_UNLOCK (self);

	goo_component_wait_for_next_state (self);

	goo_ti_camera_propagate_executing (self);

	if (self->prev_state == OMX_StateIdle)
	{
		/* goo_component_prepare_all_ports (self); */
		GOO_OBJECT_INFO (self, "Preparing only viewfinding port");
		GooPort* port = goo_ti_camera_get_port (GOO_TI_CAMERA (self),
							PORT_VIEWFINDING);
		goo_component_prepare_port (self, port);
		g_object_unref (port);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_ti_camera_set_property (GObject* object, guint prop_id,
			    const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_CAMERA (object));
	GooTiCamera* self = GOO_TI_CAMERA (object);

	switch (prop_id)
	{
	case PROP_CONTRAST:
		_goo_ti_camera_set_contrast (self, g_value_get_int (value));
		break;
	case PROP_BRIGHTNESS:
		_goo_ti_camera_set_brightness (self, g_value_get_uint (value));
		break;
	case PROP_CAPTURE_MODE:
		_goo_ti_camera_set_capture_mode (self,
						 g_value_get_boolean (value));
		break;
	case PROP_ZOOM:
		_goo_ti_camera_set_zoom (self, g_value_get_enum (value));
		break;
	case PROP_BALANCE:
		_goo_ti_camera_set_white_balance (self,
						  g_value_get_enum (value));
		break;
	case PROP_EXPOSURE:
		_goo_ti_camera_set_exposure (self,
						  g_value_get_enum (value));
		break;
	case PROP_FOCUS:
		_goo_ti_camera_set_focus (self,
						  g_value_get_enum (value));
		break;
	case PROP_VSTAB:
		_goo_ti_camera_set_vstab_mode (self,
					       g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_camera_get_property (GObject* object, guint prop_id,
			    GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_CAMERA (object));
	GooTiCamera* self = GOO_TI_CAMERA (object);

	switch (prop_id)
	{
	case PROP_CONTRAST:
		g_value_set_int (value, _goo_ti_camera_get_contrast (self));
		break;
	case PROP_BRIGHTNESS:
		g_value_set_uint (value, _goo_ti_camera_get_brightness (self));
		break;
	case PROP_CAPTURE_MODE:
		g_value_set_boolean (value,
				     _goo_ti_camera_get_capture_mode (self));
		break;
	case PROP_ZOOM:
		g_value_set_enum (value, _goo_ti_camera_get_zoom (self));
		break;
	case PROP_BALANCE:
		g_value_set_enum (value,
				  _goo_ti_camera_get_white_balance (self));
		break;
	case PROP_EXPOSURE:
		g_value_set_enum (value,
				  _goo_ti_camera_get_exposure (self));
		break;
	case PROP_FOCUS:
		g_value_set_enum (value,
				  _goo_ti_camera_get_focus (self));
		break;
	case PROP_VSTAB:
		g_value_set_boolean (value,
				     _goo_ti_camera_get_vstab_mode (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_camera_init (GooTiCamera* self)
{
	GOO_COMPONENT (self)->id = g_strdup (ID);
	GOO_COMPONENT (self)->port_param_type = OMX_IndexParamVideoInit;

	self->param = NULL;

	GooTiCameraPriv *priv = GOO_TI_CAMERA_GET_PRIVATE (self);

	priv->capturemode = FALSE;
	priv->zoom = DEFAULT_ZOOM;
	priv->balance = DEFAULT_BALANCE;
	priv->exposure = DEFAULT_EXPOSURE;

	return;
}

static void
goo_ti_camera_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_CAMERA (object));

	GooTiCamera* self = GOO_TI_CAMERA (object);

	if (G_LIKELY (self->param != NULL))
	{
		g_free (self->param);
		self->param = NULL;
	}

	(*G_OBJECT_CLASS (goo_ti_camera_parent_class)->finalize) (object);

	return;
}

static void
goo_ti_camera_class_init (GooTiCameraClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GooTiCameraPriv));
	g_klass->set_property = goo_ti_camera_set_property;
	g_klass->get_property = goo_ti_camera_get_property;
	g_klass->finalize = goo_ti_camera_finalize;

	GParamSpec* spec;
	spec = g_param_spec_int ("contrast", "Contrast",
				 "Set/Get the camera's contrast",
				 -100, 100, DEFAULT_CONTRAST,
				 G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_CONTRAST, spec);

	spec = g_param_spec_uint ("brightness", "Brightness",
				  "Set/Get the camera's brightness",
				  0, 100, DEFAULT_BRIGHTNESS,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_BRIGHTNESS, spec);

	spec = g_param_spec_boolean ("capture", "Capture mode",
				     "Turn on/off the capture mode",
				     DEFAULT_CAPTURE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_CAPTURE_MODE, spec);

	spec = g_param_spec_enum ("zoom", "Zoom value",
				  "Set/Get the zoom value",
				  GOO_TI_CAMERA_ZOOM, DEFAULT_ZOOM,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ZOOM, spec);

	spec = g_param_spec_enum ("balance",
				  "White balance control",
				  "Set/Get the white balance mode",
				  GOO_TI_CAMERA_WHITE_BALANCE,
				  DEFAULT_BALANCE,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_BALANCE, spec);

	spec = g_param_spec_enum ("exposure",
				  "Exposure control",
				  "Set/Get the exposure mode",
				  GOO_TI_CAMERA_EXPOSURE,
				  DEFAULT_EXPOSURE,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_EXPOSURE, spec);

	spec = g_param_spec_enum ("focus",
				  "Focus control",
				  "Set the autofocus mode ",
				  GOO_TI_CAMERA_MODE_FOCUS,
				  DEFAULT_FOCUS, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_FOCUS, spec);

	spec = g_param_spec_boolean ("vstab", "Video stabilization",
				     "Video stabilization",
				     DEFAULT_VSTAB, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_VSTAB, spec);

	GooComponentClass* c_klass = GOO_COMPONENT_CLASS (klass);
	c_klass->set_parameters_func = goo_ti_camera_set_parameters;
	c_klass->load_parameters_func = goo_ti_camera_load_parameters;
	c_klass->validate_ports_definition_func = goo_ti_camera_validate;
	c_klass->set_state_loaded_func = goo_ti_camera_set_state_loaded;
	c_klass->set_state_idle_func = goo_ti_camera_set_state_idle;
	c_klass->set_state_executing_func = goo_ti_camera_set_state_executing;
	/* c_klass->flush_port_func = goo_ti_camera_flush_port; */

	return;
}
