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

#include <goo-ti-vpp.h>
#include <goo-utils.h>

#define ID "OMX.TI.VPP"

#define DEFAULT_CROP_LEFT   0	/* No cropping in X axis */
#define DEFAULT_CROP_TOP    0	/* No cropping in Y axis */
#define DEFAULT_CROP_WIDTH  0	/* No cropping width */
#define DEFAULT_CROP_HEIGHT 0	/* No cropping height */
#define DEFAULT_ZOOMXOFFSET 0	/* Default zoom offset in X axis */
#define DEFAULT_ZOOMYOFFSET 0	/* Default zoom offset in Y axis */
#define DEFAULT_GLASSEFFECT 0	/* No glass effect */
#define DEFAULT_ZOOM_FACTOR 1	/* No Zoom*/
#define DEFAULT_ZOOM_LIMIT  64	/* Maximum zoom */
#define DEFAULT_ZOOM_SPEED  0	/* No dynamic zoom */
#define DEFAULT_CONTRAST    0	/* No constrast modification */
#define DEFAULT_ROTATION    GOO_TI_VPP_ROTATION_NONE
#define DEFAULT_MIRROR      FALSE /* No mirroring by default */

enum _GooTiVPPProp
{
	PROP_0,
	PROP_ROTATION,
	PROP_LEFTCROP,
	PROP_TOPCROP,
	PROP_WIDTHCROP,
	PROP_HEIGHTCROP,
	PROP_ZOOMFACTOR,
	PROP_ZOOMLIMIT,
	PROP_ZOOMSPEED,
	PROP_ZOOMXOFFSET,
	PROP_ZOOMYOFFSET,
	PROP_GLASSEFFECT,
	PROP_CONTRAST,
	PROP_MIRROR
};

struct _GooTiVPPPriv
{
	gint contrast;
	gint rotation;
	guint crop_left;
	guint crop_top;
	guint crop_width;
	guint crop_height;
	guint zoom_factor;
	guint zoom_limit;
	guint zoom_speed;
	guint zoom_x_offset;
	guint zoom_y_offset;
	guint glass_effect;
	gboolean mirror;
};

#define GOO_TI_VPP_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GOO_TYPE_TI_VPP, GooTiVPPPriv))

#define GOO_TI_VPP_ROTATION \
	(goo_ti_vpp_rotation_get_type())

G_DEFINE_TYPE (GooTiVPP, goo_ti_vpp, GOO_TYPE_COMPONENT)

GType
goo_ti_vpp_rotation_get_type ()
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0))
	{
		static const GEnumValue values[] = {
			{ GOO_TI_VPP_ROTATION_NONE, "0", "No Rotation" },
			{ GOO_TI_VPP_ROTATION_90, "90",
			  "90 degree rotation" },
			{ GOO_TI_VPP_ROTATION_180, "180",
			  "180 degree rotation" },
			{ GOO_TI_VPP_ROTATION_270, "270",
			  "270 degree rotation" },
			{ 0, NULL, NULL },
		};

		type = g_enum_register_static ("GooTiVPPRotation", values);
	}

	return type;
}

static gboolean
_goo_ti_vpp_set_rotation (GooTiVPP* self, GooTiVPPRotation rotation)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	gboolean retval = TRUE;
	OMX_CONFIG_ROTATIONTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_ROTATIONTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_ROTATIONTYPE);
	param->nRotation = (OMX_S32) rotation;

	GooIterator* iter =
		goo_component_iterate_output_ports (GOO_COMPONENT (self));
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		if (goo_port_is_enabled (port))
		{
			param->nPortIndex =
				GOO_PORT_GET_DEFINITION(port)->nPortIndex;
			GOO_OBJECT_DEBUG (self, "setting rotation port #%d",
					  param->nPortIndex);
			retval &= goo_component_set_config_by_index
				(GOO_COMPONENT (self),
				 OMX_IndexConfigCommonRotate, param);
		}
		g_object_unref (G_OBJECT (port));
		goo_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));

	if (retval == TRUE)
	{
		GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);
		priv->rotation = rotation;
		GOO_OBJECT_INFO (self, "rotation = %d", param->nRotation);
	}

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_crop_left (GooTiVPP* self, guint left)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_RECTTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_RECTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_RECTTYPE);

	param->nLeft   = (OMX_S32) left;
	param->nTop    = (OMX_S32) priv->crop_top;
	param->nWidth  = (OMX_S32) priv->crop_width;
	param->nHeight = (OMX_S32) priv->crop_height;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self), OMX_IndexConfigCommonInputCrop, param);

	if (retval == TRUE)
	{
		priv->crop_left = left;
		GOO_OBJECT_INFO (self, "X crop left = %d", param->nLeft);
	}

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_crop_width (GooTiVPP* self, guint width)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_RECTTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_RECTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_RECTTYPE);

	param->nLeft   = (OMX_S32) priv->crop_left;
	param->nTop    = (OMX_S32) priv->crop_top;
	param->nWidth  = (OMX_S32) width;
	param->nHeight = (OMX_S32) priv->crop_height;

	retval = goo_component_set_config_by_index (GOO_COMPONENT (self),
						    OMX_IndexConfigCommonInputCrop,
						    param);

	if (retval == TRUE)
	{
		priv->crop_width = width;
		GOO_OBJECT_INFO (self, "X crop width = %d", param->nWidth);
	}

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_crop_top (GooTiVPP* self, guint top)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_RECTTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_RECTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_RECTTYPE);

	param->nLeft   = (OMX_S32) priv->crop_left;
	param->nTop    = (OMX_S32) top;
	param->nWidth  = (OMX_S32) priv->crop_width;
	param->nHeight = (OMX_S32) priv->crop_height;

	retval = goo_component_set_config_by_index (GOO_COMPONENT (self),
						    OMX_IndexConfigCommonInputCrop,
						    param);

	if (retval == TRUE)
	{
		priv->crop_top = top;
		GOO_OBJECT_INFO (self, "Y crop top = %d", param->nTop);
	}

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_crop_height (GooTiVPP* self, guint height)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_RECTTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_RECTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_RECTTYPE);

	param->nLeft   = (OMX_S32) priv->crop_left;
	param->nTop    = (OMX_S32) priv->crop_top;
	param->nWidth  = (OMX_S32) priv->crop_width;
	param->nHeight = (OMX_S32) height;

	retval = goo_component_set_config_by_index (GOO_COMPONENT (self),
						    OMX_IndexConfigCommonInputCrop,
						    param);

	if (retval == TRUE)
	{
		priv->crop_height = height;
		GOO_OBJECT_INFO (self, "Y crop height = %d", param->nHeight);
	}

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_zoom_factor (GooTiVPP* self, guint factor)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);
	gboolean retval = FALSE;
	guint factor_scaled = factor << 10;

	retval = goo_component_set_config_by_name
		(GOO_COMPONENT (self), "OMX.TI.VPP.Param.ZoomFactor",
		 &factor_scaled);

	if (retval == TRUE)
	{
		priv->zoom_factor = factor;
		GOO_OBJECT_INFO (self, "Zoom factor = %d", factor);
	}

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_zoom_limit (GooTiVPP* self, guint limit)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);
	gboolean retval = FALSE;
	guint limit_scaled = limit << 10;
	retval = goo_component_set_config_by_name
		(GOO_COMPONENT (self), "OMX.TI.VPP.Param.ZoomLimit",
		 &limit_scaled);

	if (retval == TRUE)
	{
		priv->zoom_limit = limit;
		GOO_OBJECT_INFO (self, "Zoom limit = %d", limit);
	}

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_zoom_speed (GooTiVPP* self, guint speed)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GOO_OBJECT_DEBUG (self, "Entering");

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);
	gboolean retval = FALSE;

	retval = goo_component_set_config_by_name
		(GOO_COMPONENT (self), "OMX.TI.VPP.Param.ZoomSpeed",
		 &speed);

	if (retval == TRUE)
	{
		priv->zoom_speed = speed;
		GOO_OBJECT_INFO (self, "Zoom speed = %d", speed);
	}

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_zoom_x_offset (GooTiVPP* self, guint offset)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);
	gboolean retval = FALSE;
	guint xoffset = offset << 4;

	retval = goo_component_set_config_by_name
		(GOO_COMPONENT (self),
		 "OMX.TI.VPP.Param.ZoomXoffsetFromCenter16",
		 &xoffset);

	if (retval == TRUE)
	{
		priv->zoom_x_offset = offset;
		GOO_OBJECT_INFO (self, "Zoom x offset from center = %d", offset);
	}

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_zoom_y_offset (GooTiVPP* self, guint offset)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);
	gboolean retval = FALSE;
	guint yoffset = offset << 4;

	retval = goo_component_set_config_by_name
		(GOO_COMPONENT (self),
		 "OMX.TI.VPP.Param.ZoomYoffsetFromCenter16",
		 &yoffset);

	if (retval == TRUE)
	{
		priv->zoom_y_offset = offset;
		GOO_OBJECT_INFO (self, "Zoom y offset from center = %d",
				 offset);
	}

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_contrast (GooTiVPP* self, gint contrast)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	gboolean retval = FALSE;
	OMX_CONFIG_CONTRASTTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_CONTRASTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_CONTRASTTYPE);
	param->nContrast = contrast;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self), OMX_IndexConfigCommonContrast, param);

	if (retval == TRUE)
	{
		GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);
		priv->contrast = contrast;
		GOO_OBJECT_INFO (self, "contrast = %d", param->nContrast);
	}

	g_free (param);

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

static gboolean
_goo_ti_vpp_set_mirroring (GooTiVPP* self, gboolean mirror)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	gboolean retval = FALSE;

	if (mirror == FALSE)
	{
		goto beach;
	}

	GooIterator* iter =
		goo_component_iterate_output_ports (GOO_COMPONENT (self));
	goo_iterator_nth (iter, 0); /* RGB output port */
	GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
	g_return_val_if_fail (port != NULL, FALSE);

	if (goo_port_is_enabled (port))
	{
		OMX_CONFIG_MIRRORTYPE *param = NULL;
		param = g_new0 (OMX_CONFIG_MIRRORTYPE, 1);
		GOO_INIT_PARAM (param, OMX_CONFIG_MIRRORTYPE);

		param->nPortIndex = GOO_PORT_GET_DEFINITION(port)->nPortIndex;
		param->eMirror = OMX_MirrorHorizontal;

		retval = goo_component_set_config_by_index
			(GOO_COMPONENT (self),
			 OMX_IndexConfigCommonMirror, param);

		if (retval == TRUE)
		{
			GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);
			priv->mirror = TRUE;
			GOO_OBJECT_INFO (self, "Mirroring activated");
		}

		g_free (param);

	}

	g_object_unref (iter);
	g_object_unref (port);

beach:
	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

#if 0
/** @TODO: This function must be overloaded because the post processor
	doesn't send the EOS in the EventHandler **/
static void
goo_ti_vpp_eos_buffer_flag (GooComponent* self, guint portindex)
{
	g_assert (self != NULL);

	GOO_OBJECT_INFO (self, "EOS flag found in port %d", portindex);

	GooPort* port = NULL;
	OMX_PARAM_PORTDEFINITIONTYPE* param = NULL;
	GooIterator* iter = goo_component_iterate_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		port = GOO_PORT (goo_iterator_get_current (iter));
		param = GOO_PORT_GET_DEFINITION (port);
		if (param->nPortIndex == portindex)
		{
			if (param->eDir == OMX_DirInput)
			{
				goo_component_set_done (self);
			}
			else if (param->eDir == OMX_DirOutput)
			{
				if (goo_port_is_tunneled(port))
				{
					g_print ("INFORMATION (workaround): Implementing overided Eventhandler function\n");
					goo_component_set_done (self);
				}
				else
				{
					goo_port_set_eos (port);
				}
			}

			g_object_unref (G_OBJECT (port));
			break;
		}
		g_object_unref (G_OBJECT (port));
		goo_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));

	return;
}
#endif

#if 0
/*
 * Check if output ports are in tunnel mode. If so, send peer components
 * to EXECUTING state recursively
 */
static void
goo_ti_vpp_propagate_executing (GooComponent* self)
{
	GooIterator* iter = goo_component_iterate_output_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		if (goo_port_is_tunneled (port))
		{
			GOO_OBJECT_DEBUG (self, "Found a tunneled port");

			GooPort *peer_port;
			GooComponent *peer_component;

			peer_port = goo_port_get_peer (port);
			g_assert (peer_port != NULL);

			peer_component = GOO_COMPONENT (
				goo_object_get_owner (GOO_OBJECT(peer_port)));

			gchar* peer_name = goo_object_get_name
				(GOO_OBJECT (peer_port));
			GOO_OBJECT_DEBUG (self, "Peer port name = %s",
					  peer_name);
			g_free (peer_name);

			g_assert (peer_component != NULL);

			GOO_OBJECT_INFO (self, "Sending executing state to tunneled port");

			goo_component_set_state_executing (peer_component);

			goo_component_wait_for_next_state (peer_component);

			g_object_unref (peer_component);
			g_object_unref (peer_port);

		}
		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	return;
}
#endif

#if 0
static void
goo_ti_vpp_set_state_executing (GooComponent* self)
{
	g_print ("INFORMATION (workaround): Implementing workaround for executing state\n");
	GOO_OBJECT_INFO (self, "Sending executing state command");

	self->next_state = OMX_StateExecuting;

	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SendCommand (self->handle,
				 OMX_CommandStateSet,
				 self->next_state,
				 NULL)
		);
	GOO_OBJECT_UNLOCK (self);

	goo_component_wait_for_next_state (self);

	goo_ti_vpp_propagate_executing (self);

	GOO_OBJECT_DEBUG (self, "");

	return;
}
#endif

#if 0
/*
 * Check if output ports are in tunnel mode. If so, send peer components
 * to loaded state recursively
 */
static void
goo_ti_vpp_propagate_loaded (GooComponent* self)
{
	GooIterator* iter = goo_component_iterate_output_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		if (goo_port_is_tunneled (port))
		{
			GOO_OBJECT_DEBUG (self, "Found a tunneled port");

			GooPort *peer_port;
			GooComponent *peer_component;

			peer_port = goo_port_get_peer (port);
			g_assert (peer_port != NULL);

			peer_component = GOO_COMPONENT (
				goo_object_get_owner (GOO_OBJECT(peer_port)));

			gchar* peer_name = goo_object_get_name
				(GOO_OBJECT (peer_port));
			GOO_OBJECT_DEBUG (self, "Peer port name = %s",
					  peer_name);
			g_free (peer_name);

			g_assert (peer_component != NULL);

			GOO_OBJECT_INFO (self, "Sending loaded state to tunneled port");

			goo_component_set_state_loaded (peer_component);
			goo_component_wait_for_next_state (peer_component);

			g_object_unref (peer_component);
			g_object_unref (peer_port);

		}
		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	return;
}
#endif

#if 0
static void
goo_ti_vpp_set_state_loaded (GooComponent* self)
{

	GOO_OBJECT_INFO (self, "Sending loaded state command");

	/*OUT of SPEC */
	g_print ("INFORMATION (workaround): Implementing workaround for deinit loaded state\n");

	self->next_state = OMX_StateLoaded;

	GOO_OBJECT_DEBUG (self, "Propagating loaded state");

	goo_ti_vpp_propagate_loaded (self);

	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SendCommand (self->handle,
				 OMX_CommandStateSet,
				 self->next_state,
				 NULL)
		);

	GOO_OBJECT_UNLOCK (self);

	{
		if (self->cur_state == OMX_StateIdle &&
		    self->next_state == OMX_StateLoaded)
		{
			/*
			 * This is against the spec. We're begging for a
			 * race condition
			 */
			/* goo_component_disable_all_ports (self); */

			goo_component_deallocate_all_ports (self);
		}
	}

	goo_component_wait_for_next_state (self);

	self->configured = FALSE;

	GOO_OBJECT_DEBUG (self, "");

	return;
}
#endif

#if 0
static void
goo_ti_vpp_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_VPP (component));
	g_assert (component->cur_state == OMX_StateLoaded);
	GooTiVPP* self = GOO_TI_VPP (component);

	/* Overlay port */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 1);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		/* Disable Overlay port */
		goo_component_disable_port (component, port);

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* RGB port */
	{
		GooIterator* iter =
			goo_component_iterate_output_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		/* Disable RGB port */
		goo_component_disable_port (component, port);

		g_object_unref (iter);
		g_object_unref (port);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}
#endif

static void
goo_ti_vpp_validate_ports (GooComponent* component)
{
	g_assert (GOO_IS_TI_VPP (component));
	GooTiVPP* self = GOO_TI_VPP (component);
	g_assert (component->cur_state == OMX_StateLoaded);

	/* input port */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_FormatYUV420Planar:
			param->format.video.cMIMEType = "video/x-raw-yuv";
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 1.5;
			break;
		case OMX_COLOR_FormatYCbYCr:
			/* YUY2 */
		case OMX_COLOR_FormatCbYCrY:
			/* UYVY */
			param->format.video.cMIMEType = "video/x-raw-yuv";
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		case OMX_COLOR_Format16bitRGB565:
		case OMX_COLOR_Format12bitRGB444:
		case OMX_COLOR_Format8bitRGB332:
			param->format.video.cMIMEType = "video/x-raw-rgb";
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		case OMX_COLOR_Format24bitRGB888:
			param->format.video.cMIMEType = "video/x-raw-rgb";
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 3;
			break;
		case OMX_COLOR_FormatL8:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight;
			break;
		case OMX_COLOR_FormatL4:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight / 2;
			break;
		case OMX_COLOR_FormatL2:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight / 4;
			break;
		case OMX_COLOR_FormatMonochrome:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight / 8;
			break;
		default:
			GOO_OBJECT_ERROR (self, "Invalid color format");
			g_assert_not_reached ();
			break;
		}

		param->format.video.eCompressionFormat =
			OMX_IMAGE_CodingUnused;

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* overlay input port */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 1);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_Format24bitRGB888:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 3;
			break;
		default:
			GOO_OBJECT_ERROR (self, "Invalid color format");
			g_assert_not_reached ();
			break;
		}

		param->format.video.eCompressionFormat =
			OMX_IMAGE_CodingUnused;
		param->format.video.cMIMEType = "video/x-raw-rgb";

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* output RGB port */
	{
		GooIterator* iter =
			goo_component_iterate_output_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->format.video.eCompressionFormat =
			OMX_IMAGE_CodingUnused;
		param->format.video.cMIMEType = "video/x-raw-rgb";

		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_Format24bitRGB888: /* changed from BGR */
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 3;
				break;
		case OMX_COLOR_Format16bitRGB565:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		case OMX_COLOR_Format12bitRGB444:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		case OMX_COLOR_Format8bitRGB332:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight;
			break;
		case OMX_COLOR_Format32bitARGB8888:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 4;
			break;
		default:
		    //GOO_OBJECT_ERROR (self, "Invalid color format");
			//g_assert_not_reached ();
			/* Disable port */
			goo_component_disable_port (component, port);
			break;
		}

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* output YUV */
	{
		GooIterator* iter =
			goo_component_iterate_output_ports (component);
		goo_iterator_nth (iter, 1);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE* param =
			GOO_PORT_GET_DEFINITION (port);

		param->format.video.eCompressionFormat =
		OMX_IMAGE_CodingUnused;
    	param->format.video.cMIMEType = "video/x-raw-yuv";

		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_FormatYUV420Planar:
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 1.5;
			break;
		case OMX_COLOR_FormatYCbYCr:
			/* YUY2 */
		case OMX_COLOR_FormatCbYCrY:
			/* UYVY */
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		default:
			//GOO_OBJECT_ERROR (self, "Invalid color format");
			//g_assert_not_reached ();
			/* Disable port */
			goo_component_disable_port (component, port);

		}

		g_object_unref (iter);
		g_object_unref (port);

	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_ti_vpp_init (GooTiVPP* self)
{
	GOO_COMPONENT (self)->port_param_type = OMX_IndexParamVideoInit;
	GOO_COMPONENT (self)->id = g_strdup (ID);

	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);

	priv->rotation	    = DEFAULT_ROTATION;
	priv->crop_left	    = DEFAULT_CROP_LEFT;
	priv->crop_top	    = DEFAULT_CROP_TOP;
	priv->crop_width    = DEFAULT_CROP_WIDTH;
	priv->crop_height   = DEFAULT_CROP_HEIGHT;
	priv->zoom_factor   = DEFAULT_ZOOM_FACTOR;
	priv->zoom_limit    = DEFAULT_ZOOM_LIMIT;
	priv->zoom_speed    = DEFAULT_ZOOM_SPEED;
	priv->zoom_x_offset = DEFAULT_ZOOMXOFFSET;
	priv->zoom_y_offset = DEFAULT_ZOOMYOFFSET;
	priv->contrast	    = DEFAULT_CONTRAST;
	priv->mirror        = DEFAULT_MIRROR;

	return;
}

static void
goo_ti_vpp_set_property (GObject* object, guint prop_id,
			 const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_VPP (object));
	GooTiVPP* self = GOO_TI_VPP (object);

	switch (prop_id)
	{
	case PROP_ROTATION:
		_goo_ti_vpp_set_rotation (self, g_value_get_enum (value));
		break;
	case PROP_LEFTCROP:
		_goo_ti_vpp_set_crop_left (self, g_value_get_uint (value));
		break;
	case PROP_TOPCROP:
		_goo_ti_vpp_set_crop_top (self, g_value_get_uint (value));
		break;
	case PROP_WIDTHCROP:
		_goo_ti_vpp_set_crop_width (self, g_value_get_uint (value));
		break;
	case PROP_HEIGHTCROP:
		_goo_ti_vpp_set_crop_height (self, g_value_get_uint (value));
		break;
	case PROP_ZOOMFACTOR:
		_goo_ti_vpp_set_zoom_factor (self, g_value_get_uint (value));
		break;
	case PROP_ZOOMLIMIT:
		_goo_ti_vpp_set_zoom_limit (self, g_value_get_uint (value));
		break;
	case PROP_ZOOMSPEED:
		_goo_ti_vpp_set_zoom_speed (self, g_value_get_uint (value));
		break;
	case PROP_ZOOMXOFFSET:
		_goo_ti_vpp_set_zoom_x_offset (self,
					       g_value_get_uint (value));
		break;
	case PROP_ZOOMYOFFSET:
		_goo_ti_vpp_set_zoom_y_offset (self,
					       g_value_get_uint (value));
		break;
	case PROP_CONTRAST:
		_goo_ti_vpp_set_contrast (self, g_value_get_int (value));
		break;
	case PROP_MIRROR:
		_goo_ti_vpp_set_mirroring (self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_vpp_get_property (GObject* object, guint prop_id,
				     GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_VPP (object));
	GooTiVPP* self = GOO_TI_VPP (object);
	GooTiVPPPriv* priv = GOO_TI_VPP_GET_PRIVATE (self);

	switch (prop_id)
	{
	case PROP_ROTATION:
		g_value_set_enum (value, priv->rotation);
		break;
	case PROP_LEFTCROP:
		g_value_set_uint (value, priv->crop_left);
		break;
	case PROP_TOPCROP:
		g_value_set_uint (value, priv->crop_top);
		break;
	case PROP_WIDTHCROP:
		g_value_set_uint (value, priv->crop_width);
		break;
	case PROP_HEIGHTCROP:
		g_value_set_uint (value, priv->crop_height);
		break;
	case PROP_ZOOMFACTOR:
		g_value_set_uint (value, priv->zoom_factor);
		break;
	case PROP_ZOOMLIMIT:
		g_value_set_uint (value, priv->zoom_limit);
		break;
	case PROP_ZOOMSPEED:
		g_value_set_uint (value, priv->zoom_speed);
		break;
	case PROP_ZOOMXOFFSET:
		g_value_set_uint (value, priv->zoom_x_offset);
		break;
	case PROP_ZOOMYOFFSET:
		g_value_set_uint (value, priv->zoom_y_offset);
		break;
	case PROP_CONTRAST:
		g_value_set_int (value, priv->contrast);
		break;
	case PROP_MIRROR:
		g_value_set_boolean (value, priv->mirror);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_vpp_class_init (GooTiVPPClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);
	g_type_class_add_private (g_klass, sizeof (GooTiVPPPriv));

	g_klass->set_property = goo_ti_vpp_set_property;
	g_klass->get_property = goo_ti_vpp_get_property;

	GParamSpec* spec = NULL;
	spec = g_param_spec_enum ("rotation", "Rotation",
				  "Rotates the output by a multiple of 90 degrees",
				  GOO_TI_VPP_ROTATION,
				  DEFAULT_ROTATION, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ROTATION, spec);

	spec = g_param_spec_uint ("crop-left", "Crop Left",
				  "Sets the value on the x axis to start cropping",
				  0, G_MAXUINT, DEFAULT_CROP_LEFT,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_LEFTCROP, spec);

	spec = g_param_spec_uint ("crop-width", "Crop Width",
				  "Sets the value on the width to crop on the  x axis",
				  0, G_MAXUINT, DEFAULT_CROP_WIDTH,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_WIDTHCROP, spec);

	spec = g_param_spec_uint ("crop-top", "Crop top",
				  "Sets the value on the y axis to start cropping",
				  0, G_MAXUINT, DEFAULT_CROP_TOP,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_TOPCROP, spec);

	spec = g_param_spec_uint ("crop-height", "Crop Height",
				  "Sets the value on the height to crop on the	y axis",
				  0, G_MAXUINT, DEFAULT_CROP_HEIGHT,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_HEIGHTCROP, spec);

	spec = g_param_spec_uint ("zoom-factor", "Zoom Factor",
				  "Sets the value of the zoom factor to magnify the output of the yuv",
				  1, 64, DEFAULT_ZOOM_FACTOR,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ZOOMFACTOR, spec);

	spec = g_param_spec_uint ("zoom-limit", "Zoom Limit",
				  "Sets the value of the magnification limit",
				  1, 64, DEFAULT_ZOOM_LIMIT,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ZOOMLIMIT, spec);

	spec = g_param_spec_uint ("zoom-speed", "Zoom Speed",
				  "Sets the value of the dynamic magnification speed",
				  0, G_MAXUINT, DEFAULT_ZOOM_SPEED,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ZOOMSPEED, spec);

	spec = g_param_spec_uint ("zoom-xoffset", "Zoom x offset",
				  "Sets the value of the magnification offset on the X axis",
				  0, G_MAXUINT, DEFAULT_ZOOMXOFFSET,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ZOOMXOFFSET, spec);

	spec = g_param_spec_uint ("zoom-yoffset", "Zoom y offset",
				  "Sets the value of the magnification offset on the Y axis",
				  0, G_MAXUINT, DEFAULT_ZOOMYOFFSET,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ZOOMYOFFSET, spec);

	spec = g_param_spec_int ("contrast", "Contrast",
				 "Sets the value of the contrast",
				 -100, 100, DEFAULT_CONTRAST,
				 G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_CONTRAST, spec);

	spec = g_param_spec_boolean ("mirror", "Mirror effect",
				     "Mirror effect on RGB output",
				     DEFAULT_MIRROR, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_MIRROR, spec);

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->validate_ports_definition_func = goo_ti_vpp_validate_ports;
	o_klass->flush_port_func = NULL;

/* 	o_klass->load_parameters_func = goo_ti_vpp_load_parameters; */
/* 	o_klass->eos_flag_func = goo_ti_vpp_eos_buffer_flag; */
/* 	o_klass->set_state_executing_func = goo_ti_vpp_set_state_executing; */
/* 	o_klass->set_state_loaded_func = goo_ti_vpp_set_state_loaded; */

	return;
}
