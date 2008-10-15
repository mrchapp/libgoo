/*-*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-*/

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

#include <goo-ti-post-processor.h>
#include <goo-ti-clock.h>
#include <goo-utils.h>

#define ID "OMX.TI.PostProc"

#define VIDEO_PIPELINE	 "OMX.TI.PostProc.Param.VideoPipeline"
#define BACKGROUND_COLOR "OMX.TI.PostProc.Param.BackgroundColor"
#define TRANSPARENCY_KEY "OMX.TI.PostProc.Param.TransparencyKey"
#define CLOCK_SOURCE	 "OMX.TI.PostProc.Param.SetClockSource"
#define OUTPUT_DEVICE	 "OMX.TI.PostProc.Param.OutputDevice"

#define DEFAULT_ROTATION   GOO_TI_POST_PROCESSOR_ROTATION_NONE
#define DEFAULT_OUTPUT	   GOO_TI_POST_PROCESSOR_OUTPUT_LCD
#define DEFAULT_OPACITY    255      /* Full opacity*/
#define DEFAULT_XSCALE	   100	 /* No scaling in X axis */
#define DEFAULT_YSCALE	   100	 /* No scaling in Y axis */
#define DEFAULT_XPOS	   0	 /* Display position starts left edge */
#define DEFAULT_YPOS	   0	 /* Display position starts top edge */
#define DEFAULT_MIRROR	   FALSE /* No mirrored image */
#define DEFAULT_CROPLEFT   0	 /* No left crop   */
#define DEFAULT_CROPTOP	   0	 /* No top crop	   */
#define DEFAULT_CROPWIDTH  0	 /* No width crop  */
#define DEFAULT_CROPHEIGHT 0	 /* No height crop */

enum _GooTiPostProcessorProp
{
	PROP_0,
	PROP_ROTATION,
        PROP_XSCALE,
	PROP_YSCALE,
	PROP_XPOS,
	PROP_YPOS,
	PROP_MIRROR,
	PROP_CROPLEFT,
	PROP_CROPTOP,
	PROP_CROPWIDTH,
	PROP_CROPHEIGHT,
	PROP_OUTPUT,
	PROP_OPACITY,
};

struct _GooTiPostProcessorPriv
{
	GooTiPostProcessorRotation rotation;
        guint xscale;
	guint yscale;
	guint xpos;
	guint ypos;
	gboolean mirror;
	guint cropleft;
	guint croptop;
	guint cropwidth;
	guint cropheight;
	GooTiPostProcessorOutput output;
	guint opacity;
	guint out_device;
};

#define GOO_TI_POST_PROCESSOR_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GOO_TYPE_TI_POST_PROCESSOR, GooTiPostProcessorPriv))

#define GOO_TI_POST_PROCESSOR_ROTATION \
	(goo_ti_post_processor_rotation_get_type ())


GType
goo_ti_post_processor_rotation_get_type ()
{
	static GType type = 0;

	if (!type)
	{
		static GEnumValue values[] = {
			{ GOO_TI_POST_PROCESSOR_ROTATION_NONE,
			  "0", "No Rotation" },
			{ GOO_TI_POST_PROCESSOR_ROTATION_90,
			  "90", "90 degree rotation" },
			{ GOO_TI_POST_PROCESSOR_ROTATION_180,
			  "180", "180 degree rotation" },
			{ GOO_TI_POST_PROCESSOR_ROTATION_270,
			  "270", "270 degree rotation" },
			{ 0, NULL, NULL },
		};

		type = g_enum_register_static
			("GooTiPostProcessorRotation", values);
	}

	return type;
}

#define GOO_TI_POST_PROCESSOR_OUTPUT \
	(goo_ti_post_processor_output_get_type ())

GType
goo_ti_post_processor_output_get_type ()
{
	static GType type = 0;

	if (!type)
	{
		static GEnumValue values[] = {
			{ GOO_TI_POST_PROCESSOR_OUTPUT_LCD,
			  "LCD", "LCD display output" },
			{ GOO_TI_POST_PROCESSOR_OUTPUT_TV,
			  "TV", "TV display output" },
			{ GOO_TI_POST_PROCESSOR_OUTPUT_BOTH,
			  "TV & LCD", "Simultaneous display output" },
			{ 0, NULL, NULL },
		};

		type = g_enum_register_static
			("GooTiPostProcessorOutput", values);
	}

	return type;
}

static gboolean
_goo_ti_post_processor_set_rotation (GooTiPostProcessor* self,
				     GooTiPostProcessorRotation rotation)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	gboolean retval = FALSE;
	OMX_CONFIG_ROTATIONTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_ROTATIONTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_ROTATIONTYPE);

	param->nRotation = rotation;

	GOO_OBJECT_INFO (self, "");

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self),
		 OMX_IndexConfigCommonRotate, param);

	if (retval == TRUE)
	{
		GooTiPostProcessorPriv* priv =
			GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);
		priv->rotation = rotation;
		GOO_OBJECT_INFO (self, "rotation = %d", param->nRotation);
	}

	g_free (param);

	return retval;

}



static gboolean
_goo_ti_post_processor_set_opacity (GooTiPostProcessor* self, guint opacity)
{
	GOO_OBJECT_DEBUG (self, "Entering set opacity method");
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (opacity >= 0 && opacity <= 255);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_COLORBLENDTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_COLORBLENDTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_COLORBLENDTYPE);


	param->eColorBlend  	 = 	OMX_ColorBlendAlphaConstant;
	param->nRGBAlphaConstant = 	opacity;

	retval = goo_component_set_config_by_index
						(GOO_COMPONENT (self),
						OMX_IndexConfigCommonColorBlend, param);
	if (retval == TRUE)
	{
		priv->opacity = opacity;
		GOO_OBJECT_INFO (self, "Opacity = %d", param->nRGBAlphaConstant);
	}

	g_free (param);

	return retval;
}


static gboolean
_goo_ti_post_processor_set_xscale (GooTiPostProcessor* self, guint xscale)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (xscale >= 25 && xscale <= 800);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_SCALEFACTORTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	param->xWidth = xscale;
	param->xHeight = priv->yscale;

	retval = goo_component_set_config_by_index (GOO_COMPONENT (self),
						    OMX_IndexConfigCommonScale,
						    param);

	if (retval == TRUE)
	{
		priv->xscale = xscale;
		GOO_OBJECT_INFO (self, "X scale = %d", param->xWidth);
	}

	g_free (param);

	return retval;
}

static gboolean
_goo_ti_post_processor_set_xpos (GooTiPostProcessor* self, guint xpos)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_POINTTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_POINTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_POINTTYPE);

	param->nX = xpos;
	param->nY = priv->ypos;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self),
		 OMX_IndexConfigCommonOutputPosition, param);

	if (retval == TRUE)
	{
		priv->xpos = xpos;
		GOO_OBJECT_INFO (self, "X pos = %d", param->nX);
	}

	g_free (param);

	return retval;

}

static gboolean
_goo_ti_post_processor_set_ypos (GooTiPostProcessor* self, guint ypos)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_POINTTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_POINTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_POINTTYPE);

	param->nX = priv->xpos;
	param->nY = ypos;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self),
		 OMX_IndexConfigCommonOutputPosition, param);

	if (retval == TRUE)
	{
		priv->ypos = ypos;
		GOO_OBJECT_INFO (self, "Y pos = %d", param->nY);
	}

	g_free (param);

	return retval;

}

static gboolean
_goo_ti_post_processor_set_yscale (GooTiPostProcessor* self, guint yscale)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (yscale >= 25 && yscale <= 800);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_SCALEFACTORTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	param->xWidth = priv->xscale;
	param->xHeight = yscale;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self), OMX_IndexConfigCommonScale, param);

	if (retval == TRUE)
	{
		priv->yscale = yscale;
		GOO_OBJECT_INFO (self, "Y scale = %d", param->xWidth);
	}

	g_free (param);

	return retval;
}

static gboolean
_goo_ti_post_processor_set_mirror (GooTiPostProcessor* self, gboolean mirror)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_MIRRORTYPE *param = NULL;
	param = g_new0 (OMX_CONFIG_MIRRORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	param->eMirror = (mirror == TRUE) ?
		OMX_MirrorHorizontal : OMX_MirrorNone;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self),
		 OMX_IndexConfigCommonMirror, param);

	if (retval == TRUE)
	{
		priv->mirror = mirror;
		GOO_OBJECT_INFO (self, "Mirror = %s", mirror ? "yes" : "no");
	}

	g_free (param);

	return retval;
}

static gboolean
_goo_ti_post_processor_set_crop_top (GooTiPostProcessor* self, guint croptop)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_RECTTYPE* param = NULL;
	param = g_new0 (OMX_CONFIG_RECTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_RECTTYPE);

	param->nLeft   = priv->cropleft;
	param->nTop    = croptop;
	param->nWidth  = priv->cropwidth;
	param->nHeight = priv->cropheight;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self),
		 OMX_IndexConfigCommonOutputCrop, param);

	if (retval == TRUE)
	{
		priv->croptop = croptop;
		GOO_OBJECT_INFO (self, "Crop Top = %d", priv->croptop);
	}

	g_free (param);

	return retval;
}

static gboolean
_goo_ti_post_processor_set_crop_left (GooTiPostProcessor* self,
				      guint cropleft)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_RECTTYPE* param = NULL;
	param = g_new0 (OMX_CONFIG_RECTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_RECTTYPE);

	param->nLeft   = cropleft;
	param->nTop    = priv->croptop;
	param->nWidth  = priv->cropwidth;
	param->nHeight = priv->cropheight;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self),
		 OMX_IndexConfigCommonOutputCrop, param);

	if (retval == TRUE)
	{
		priv->cropleft = cropleft;
		GOO_OBJECT_INFO (self, "Crop Left = %d", priv->cropleft);
	}

	g_free (param);

	return retval;
}

static gboolean
_goo_ti_post_processor_set_crop_width (GooTiPostProcessor* self,
				       guint cropwidth)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_RECTTYPE* param = NULL;
	param = g_new0 (OMX_CONFIG_RECTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_RECTTYPE);

	param->nLeft   = priv->cropleft;
	param->nTop    = priv->croptop;
	param->nWidth  = cropwidth;
	param->nHeight = priv->cropheight;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self),
		 OMX_IndexConfigCommonOutputCrop, param);

	if (retval == TRUE)
	{
		priv->cropwidth = cropwidth;
		GOO_OBJECT_INFO (self, "Crop Width = %d", priv->cropwidth);
	}

	g_free (param);

	return retval;
}

static gboolean
_goo_ti_post_processor_set_crop_height (GooTiPostProcessor* self,
					guint cropheight)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	gboolean retval = FALSE;
	OMX_CONFIG_RECTTYPE* param = NULL;
	param = g_new0 (OMX_CONFIG_RECTTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_RECTTYPE);

	param->nLeft   = priv->cropleft;
	param->nTop    = priv->croptop;
	param->nWidth  = priv->cropwidth;
	param->nHeight = cropheight;

	retval = goo_component_set_config_by_index
		(GOO_COMPONENT (self),
		 OMX_IndexConfigCommonOutputCrop, param);

	if (retval == TRUE)
	{
		priv->cropheight = cropheight;
		GOO_OBJECT_INFO (self, "Crop Height = %d", priv->cropheight);
	}

	g_free (param);

	return retval;
}

static gboolean
_goo_ti_post_processor_set_ouput (GooTiPostProcessor* self,
				  GooTiPostProcessorOutput output)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	gboolean retval = TRUE;
	POSTPROC_CUSTOM_OUTPUTDEVICES outdev;

	if (self->video_pipeline == 1 &&
	    output == GOO_TI_POST_PROCESSOR_OUTPUT_LCD)
	{
		outdev.nVideoPipeline1 = GOO_TI_POST_PROCESSOR_OUTPUT_LCD;
		outdev.nVideoPipeline2 = GOO_TI_POST_PROCESSOR_OUTPUT_NONE;
	}
	else if (self->video_pipeline == 2 &&
		 output == GOO_TI_POST_PROCESSOR_OUTPUT_LCD)
	{
		outdev.nVideoPipeline1 = GOO_TI_POST_PROCESSOR_OUTPUT_NONE;
		outdev.nVideoPipeline2 = GOO_TI_POST_PROCESSOR_OUTPUT_LCD;
	}
	else if (self->video_pipeline == 1 &&
		 output == GOO_TI_POST_PROCESSOR_OUTPUT_TV)
	{
		outdev.nVideoPipeline1 = GOO_TI_POST_PROCESSOR_OUTPUT_TV;
		outdev.nVideoPipeline2 = GOO_TI_POST_PROCESSOR_OUTPUT_NONE;
	}
	else if (self->video_pipeline == 2 &&
		 output == GOO_TI_POST_PROCESSOR_OUTPUT_TV)
	{
		outdev.nVideoPipeline1 = GOO_TI_POST_PROCESSOR_OUTPUT_NONE;
		outdev.nVideoPipeline2 = GOO_TI_POST_PROCESSOR_OUTPUT_TV;
	}
	else if (output == GOO_TI_POST_PROCESSOR_OUTPUT_BOTH)
	{
		outdev.nVideoPipeline1 = GOO_TI_POST_PROCESSOR_OUTPUT_LCD;
		outdev.nVideoPipeline2 = GOO_TI_POST_PROCESSOR_OUTPUT_TV;
	}
	else
	{
		GOO_OBJECT_WARNING (self, "No configurable output");
		return retval;
	}

	retval = goo_component_set_config_by_name (GOO_COMPONENT (self),
						   OUTPUT_DEVICE,
						   (OMX_PTR*) &outdev);

	if (retval == TRUE)
	{
		GooTiPostProcessorPriv* priv =
			GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);
		priv->output = output;
		GOO_OBJECT_INFO (self, "output = %d", priv->output);
	}

	return retval;
}

static void
goo_ti_post_processor_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_POST_PROCESSOR (component));
	GooTiPostProcessor* self = GOO_TI_POST_PROCESSOR (component);

	self->background_color = g_new0 (POSTPROC_CUSTOM_BACKGROUNDCOLOR, 1);
	self->background_color->nOutputDev = 0;
	self->background_color->nColor = 0x000000;

	self->transcolor_key = g_new0 (POSTPROC_CUSTOM_TRANSCOLORKEYTYPE, 1);
	self->transcolor_key->nOutputDev = 0;
	self->transcolor_key->nKeyType = 1;
	self->transcolor_key->bTransColorEnable = OMX_FALSE;
	self->transcolor_key->nColor = 0x000000;

	self->video_pipeline = 1;

	GOO_OBJECT_DEBUG (self, "");
	return;
}

static void
goo_ti_post_processor_validate (GooComponent* component)
{
	g_assert (GOO_IS_TI_POST_PROCESSOR (component));
	GooTiPostProcessor* self = GOO_TI_POST_PROCESSOR (component);
	g_assert (component->cur_state == OMX_StateLoaded);

	/* params */
	{
		g_assert (self->video_pipeline >= 1 &&
			  self->video_pipeline <= 2);

		/* this is temporal */
		self->background_color->nOutputDev = 0;

		g_assert (self->background_color->nColor >= 0x000000 &&
			  self->background_color->nColor <= 0xffffff);

		/* this is temporal */
		self->transcolor_key->nOutputDev = 0;

		self->transcolor_key->nKeyType = 1;
		g_assert (self->transcolor_key->nColor >= 0x000000 &&
			  self->transcolor_key->nColor <= 0xffffff);

	}

	/* input */
	{
		GooIterator* iter =
		goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		OMX_PARAM_PORTDEFINITIONTYPE* param;
		param = GOO_PORT_GET_DEFINITION (port);

		/* let's use the max available resolution. */
		ResolutionInfo rinfo = goo_get_resolution ("sxvga");
		g_assert ((param->format.video.nFrameWidth <= rinfo.width) &&
			  (param->format.video.nFrameHeight <= rinfo.height));

		param->format.video.cMIMEType = "video/x-raw-yuv";

		switch (param->format.video.eColorFormat)
		{
		case OMX_COLOR_FormatYUV420Planar:
			/* I420 */
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 1.5;
		break;
		case OMX_COLOR_FormatYCbYCr:
			/* YUY2? */
		case OMX_COLOR_FormatCbYCrY:
			/* UYVY? */
		case OMX_COLOR_Format16bitRGB565:
			/* RGB */
			param->nBufferSize =
				param->format.video.nFrameWidth *
				param->format.video.nFrameHeight * 2;
			break;
		default:
			GOO_OBJECT_DEBUG (self, "Invalid color format");
			g_assert_not_reached ();
		}

		param->format.video.eCompressionFormat =
			OMX_VIDEO_CodingUnused;

		g_object_unref (iter);
		g_object_unref (port);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}


void
goo_ti_post_processor_set_starttime (GooComponent* component, gint64 time_start)
{
	g_assert (GOO_IS_TI_POST_PROCESSOR (component));
	GooTiPostProcessor* self = GOO_TI_POST_PROCESSOR (component);
	GOO_OBJECT_DEBUG (self, "Set start time by sync AV");
	GooTiComponentFactory *factory =
		GOO_TI_COMPONENT_FACTORY (
			goo_object_get_owner (GOO_OBJECT (component))
			);

	if (factory->clock != NULL)
	{
		GooComponent *clock = g_object_ref (factory->clock);

		OMX_TIME_CONFIG_TIMESTAMPTYPE* param;
		param = g_new0 (OMX_TIME_CONFIG_TIMESTAMPTYPE, 1);
		GOO_INIT_PARAM (param, OMX_TIME_CONFIG_TIMESTAMPTYPE);

		param->nTimestamp = time_start;

		goo_component_set_config_by_index (clock,
						OMX_IndexConfigTimeClientStartTime, param);

		g_object_unref (clock);
		g_free (param);
		g_object_unref (factory);

	}
}



static gboolean
goo_ti_post_processor_set_clock (GooComponent* component)
{
	g_assert (GOO_IS_TI_POST_PROCESSOR (component));
	gboolean retval = FALSE;

	GooTiComponentFactory *factory =
		GOO_TI_COMPONENT_FACTORY (
			goo_object_get_owner (GOO_OBJECT (component))
			);

	if (factory == NULL)
	{
		return retval;
	}

	if (factory->clock != NULL)
	{
		GooComponent *clock = g_object_ref (factory->clock);
		retval = goo_component_set_parameter_by_name (component,
							      CLOCK_SOURCE,
							      clock->handle);
		g_object_unref (clock);
	}

	g_object_unref (factory);

	return retval;
}

static void
goo_ti_post_processor_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_POST_PROCESSOR (component));
	GooTiPostProcessor* self = GOO_TI_POST_PROCESSOR (component);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	goo_component_set_parameter_by_name (component, VIDEO_PIPELINE,
					     (OMX_PTR) &self->video_pipeline);


	_goo_ti_post_processor_set_ouput(self, priv->out_device);

	goo_component_set_parameter_by_name (component,
					     BACKGROUND_COLOR,
					     self->background_color);

	if (self->transcolor_key->bTransColorEnable != OMX_FALSE)
	{
		goo_component_set_parameter_by_name (component,
						     TRANSPARENCY_KEY,
						     self->transcolor_key);
	}

	/* Check if there is an OMX Clock existent */
	goo_ti_post_processor_set_clock (component);

	GOO_OBJECT_DEBUG (self, "");

	return;
}


G_DEFINE_TYPE (GooTiPostProcessor, goo_ti_post_processor, GOO_TYPE_COMPONENT)

static void
goo_ti_post_processor_init (GooTiPostProcessor* self)
{
	GOO_COMPONENT (self)->port_param_type = OMX_IndexParamVideoInit;
	GOO_COMPONENT (self)->id = g_strdup (ID);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	priv->rotation 	= DEFAULT_ROTATION;
	priv->opacity= DEFAULT_OPACITY;
	priv->xscale   	= DEFAULT_XSCALE;
	priv->yscale   	= DEFAULT_YSCALE;
	priv->xpos     	= DEFAULT_XPOS;
	priv->ypos     	= DEFAULT_YPOS;
	priv->mirror   	= DEFAULT_MIRROR;
	priv->output   	= DEFAULT_OUTPUT;

	self->video_pipeline   = 1;
	self->background_color = NULL;
	self->transcolor_key   = NULL;

	return;
}

static void
goo_ti_post_processor_set_property (GObject* object, guint prop_id,
				    const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_POST_PROCESSOR (object));
	GooTiPostProcessor* self = GOO_TI_POST_PROCESSOR (object);

	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	switch (prop_id)
	{
	case PROP_ROTATION:
		_goo_ti_post_processor_set_rotation
			(self, g_value_get_enum (value));
		break;
	case PROP_OPACITY:
		_goo_ti_post_processor_set_opacity
			(self, g_value_get_uint (value));
		break;

	case PROP_XSCALE:
		_goo_ti_post_processor_set_xscale
			(self, g_value_get_uint (value));
		break;
	case PROP_YSCALE:
		_goo_ti_post_processor_set_yscale
			(self, g_value_get_uint (value));
		break;
	case PROP_XPOS:
		_goo_ti_post_processor_set_xpos
			(self, g_value_get_uint (value));
		break;
	case PROP_YPOS:
		_goo_ti_post_processor_set_ypos
			(self, g_value_get_uint (value));
		break;
	case PROP_MIRROR:
		_goo_ti_post_processor_set_mirror
			(self, g_value_get_boolean (value));
		break;
	case PROP_CROPTOP:
		_goo_ti_post_processor_set_crop_top
			(self, g_value_get_uint (value));
		break;
	case PROP_CROPLEFT:
		_goo_ti_post_processor_set_crop_left
			(self, g_value_get_uint (value));
		break;
	case PROP_CROPWIDTH:
		_goo_ti_post_processor_set_crop_width
			(self, g_value_get_uint (value));
		break;
	case PROP_CROPHEIGHT:
		_goo_ti_post_processor_set_crop_height
			(self, g_value_get_uint (value));
		break;
	case PROP_OUTPUT:
		priv->out_device = g_value_get_enum (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_post_processor_get_property (GObject* object, guint prop_id,
				     GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_POST_PROCESSOR (object));
	GooTiPostProcessor* self = GOO_TI_POST_PROCESSOR (object);
	GooTiPostProcessorPriv* priv =
		GOO_TI_POST_PROCESSOR_GET_PRIVATE (self);

	switch (prop_id)
	{
	case PROP_ROTATION:
		g_value_set_enum (value, priv->rotation);
		break;
	case PROP_OPACITY:
		g_value_set_uint (value, priv->opacity);
		break;
	case PROP_XSCALE:
		g_value_set_uint (value, priv->xscale);
		break;
	case PROP_YSCALE:
		g_value_set_uint (value, priv->yscale);
		break;
	case PROP_XPOS:
		g_value_set_uint (value, priv->xpos);
		break;
	case PROP_YPOS:
		g_value_set_uint (value, priv->ypos);
		break;
	case PROP_MIRROR:
		g_value_set_boolean (value, priv->mirror);
		break;
	case PROP_CROPLEFT:
		g_value_set_uint (value, priv->cropleft);
		break;
	case PROP_CROPTOP:
		g_value_set_uint (value, priv->croptop);
		break;
	case PROP_CROPWIDTH:
		g_value_set_uint (value, priv->cropwidth);
		break;
	case PROP_CROPHEIGHT:
		g_value_set_uint (value, priv->cropheight);
		break;
	case PROP_OUTPUT:
		g_value_set_enum (value, priv->output);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_post_processor_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_POST_PROCESSOR (object));
	GooTiPostProcessor* self = GOO_TI_POST_PROCESSOR (object);

	if (G_LIKELY (self->background_color))
	{
		g_free (self->background_color);
		self->background_color = NULL;
	}

	if (G_LIKELY (self->transcolor_key))
	{
		g_free (self->transcolor_key);
		self->transcolor_key = NULL;
	}

	(*G_OBJECT_CLASS (goo_ti_post_processor_parent_class)->finalize) (object);

	return;
}

static void
goo_ti_post_processor_class_init (GooTiPostProcessorClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_type_class_add_private (g_klass, sizeof (GooTiPostProcessorPriv));
	g_klass->set_property = goo_ti_post_processor_set_property;
	g_klass->get_property = goo_ti_post_processor_get_property;
	g_klass->finalize = goo_ti_post_processor_finalize;

	GParamSpec* spec = NULL;
	spec = g_param_spec_enum ("rotation", "Rotation",
				  "Rotates the output frame",
				  GOO_TI_POST_PROCESSOR_ROTATION,
				  DEFAULT_ROTATION, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ROTATION, spec);

	spec = g_param_spec_uint ("opacity", "Opacity ",
				  "Modify the opacity",
				  0, 255, DEFAULT_OPACITY,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_OPACITY, spec);


	spec = g_param_spec_uint ("x-scale", "X scale position",
				  "Scales X axis by a factor",
				  0, G_MAXUINT, DEFAULT_XSCALE,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_XSCALE, spec);

	spec = g_param_spec_uint ("y-scale", "Y scale position",
				  "Scales Y axis by a factor",
				  0, G_MAXUINT, DEFAULT_YSCALE,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_YSCALE, spec);

	spec = g_param_spec_uint ("x-pos", "X position",
				  "Display X axis with an offset",
				  0, G_MAXUINT, DEFAULT_XPOS,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_XPOS, spec);

	spec = g_param_spec_uint ("y-pos", "Y position",
				  "Display Y axis with an offset",
				  0, G_MAXUINT, DEFAULT_YPOS,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_YPOS, spec);

	spec = g_param_spec_boolean ("mirror", "Mirror", "Mirror effect",
				     FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_MIRROR, spec);

	spec = g_param_spec_uint ("crop-left", "Left cropping",
				 "The number of pixels to crop from left",
				 0, G_MAXUINT, 0, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_CROPLEFT, spec);

	spec = g_param_spec_uint ("crop-top", "Top cropping",
				 "The number of pixels to crop from top",
				 0, G_MAXUINT, 0, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_CROPTOP, spec);

	spec = g_param_spec_uint ("crop-width", "Cropped width",
				  "The width of cropped display",
				  0, G_MAXUINT, 0, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_CROPWIDTH, spec);

	spec = g_param_spec_uint ("crop-height", "Cropped height",
				  "The height of cropped display",
				  0, G_MAXUINT, 0, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_CROPHEIGHT, spec);

	spec = g_param_spec_enum ("out-device", "Output device",
				  "The output device",
				  GOO_TI_POST_PROCESSOR_OUTPUT,
				  DEFAULT_OUTPUT, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_OUTPUT, spec);

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);

	o_klass->validate_ports_definition_func =
		goo_ti_post_processor_validate;
	o_klass->set_parameters_func =
		goo_ti_post_processor_set_parameters;
	o_klass->load_parameters_func =
		goo_ti_post_processor_load_parameters;
	/* o_klass->set_clock_func = goo_ti_post_processor_set_clock; */

	return;
}
