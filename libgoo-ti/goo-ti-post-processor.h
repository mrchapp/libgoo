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

#ifndef _GOO_TI_POST_PROCESSOR_H_
#define _GOO_TI_POST_PROCESSOR_H_

#include <goo-component.h>
#include <goo-ti-component-factory.h>
/*wrong*/
/*set config start time */
#include <OMX_Other.h>
/* This is pure crap; against the OMX spirit */
#include <OMX_PostProc_CustomCmd.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_POST_PROCESSOR \
        (goo_ti_post_processor_get_type ())
#define GOO_TI_POST_PROCESSOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_POST_PROCESSOR, GooTiPostProcessor))
#define GOO_TI_POST_PROCESSOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_POST_PROCESSOR, GooTiPostProcessorClass))
#define GOO_IS_TI_POST_PROCESSOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_POST_PROCESSOR))
#define GOO_IS_TI_POST_PROCESSOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_POST_PROCESSOR))
#define GOO_TI_POST_PROCESSOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_POST_PROCESSOR, GooTiPostProcessorClass))

typedef enum
{
        GOO_TI_POST_PROCESSOR_ROTATION_NONE = 0,
        GOO_TI_POST_PROCESSOR_ROTATION_90   = 90,
        GOO_TI_POST_PROCESSOR_ROTATION_180  = 180,
        GOO_TI_POST_PROCESSOR_ROTATION_270  = 270
} GooTiPostProcessorRotation;

typedef enum
{
	GOO_TI_POST_PROCESSOR_OUTPUT_NONE = 0,
	GOO_TI_POST_PROCESSOR_OUTPUT_LCD,
	GOO_TI_POST_PROCESSOR_OUTPUT_TV,
	GOO_TI_POST_PROCESSOR_OUTPUT_BOTH,
} GooTiPostProcessorOutput;

typedef struct _GooTiPostProcessorPriv GooTiPostProcessorPriv;
typedef struct _GooTiPostProcessor GooTiPostProcessor;
typedef struct _GooTiPostProcessorClass GooTiPostProcessorClass;

/**
 * GooTiPostProcessor:
 * @video_pipeline: The pipeline to use in the frame buffer subsystem
 * @background_color: The param to set a background color
 * @transcolor_key: The param to transform a color for other
 *
 * It is the OMX postprocessor component
 */
struct _GooTiPostProcessor
{
        GooComponent parent;

        gint video_pipeline;
	POSTPROC_CUSTOM_BACKGROUNDCOLOR* background_color;
	POSTPROC_CUSTOM_TRANSCOLORKEYTYPE* transcolor_key;
};

struct _GooTiPostProcessorClass
{
        GooComponentClass parent_class;
};

GType goo_ti_post_processor_get_type (void);
GType goo_ti_post_processor_rotation_get_type (void);
GType goo_ti_post_processor_output_get_type (void);

/**
 * GOO_TI_POST_PROCESSOR_GET_BACKGROUND:
 * @postproc: A #GooTiPostProcessor instance
 *
 * Gets the #POSTPROC_CUSTOM_BACKGROUNDCOLOR of the camera
 *
 * Return value: The #POSTPROC_CUSTOM_BACKGROUNDCOLOR pointer
 */
#define GOO_TI_POST_PROCESSOR_GET_BACKGROUND(postproc) (GOO_TI_POST_PROCESSOR (postproc)->background_color)

/**
 * GOO_TI_POST_PROCESSOR_GET_TRANSCOLORKEY:
 * @postproc: A #GooTiPostProcessor instance
 *
 * Gets the #POSTPROC_CUSTOM_TRANSCOLORKEYTYPE of the camera
 *
 * Return value: The #POSTPROC_CUSTOM_TRANSCOLORKEYTYPE pointer
 */
#define GOO_TI_POST_PROCESSOR_GET_TRANSCOLORKEY(postproc) (GOO_TI_POST_PROCESSOR (postproc)->transcolor_key)

void goo_ti_post_processor_set_starttime (GooComponent* component, gint64 time_start);

G_END_DECLS

#endif /* _GOO_TI_POST_PROCESSOR_H_ */

