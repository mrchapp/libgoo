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

#ifndef _GOO_TI_VIDEO_ENCODER720P_H_
#define _GOO_TI_VIDEO_ENCODER720P_H_

#include <goo-component.h>
#include <OMX_Video.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_VIDEO_ENCODER720P \
	(goo_ti_video_encoder720p_get_type ())
#define GOO_TI_VIDEO_ENCODER720P(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_VIDEO_ENCODER720P, GooTiVideoEncoder720p))
#define GOO_TI_VIDEO_ENCODER720P_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_VIDEO_ENCODER720P, GooTiVideoEncoder720pClass))
#define GOO_IS_TI_VIDEO_ENCODER720P(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_VIDEO_ENCODER720P))
#define GOO_IS_TI_VIDEO_ENCODER720P_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_VIDEO_ENCODER720P))
#define GOO_TI_VIDEO_ENCODER720P_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_VIDEO_ENCODER720P, GooTiVideoEncoder720pClass))

#define GOO_TI_VIDEO_ENCODER720P_CONTROL_RATE (goo_ti_video_encoder720p_control_rate_get_type())

typedef struct _GooTiVideoEncoder720p GooTiVideoEncoder720p;
typedef struct _GooTiVideoEncoder720pClass GooTiVideoEncoder720pClass;

typedef enum GooTiVideoEncoder720pControlRate
{
	GOO_TI_VIDEO_ENCODER720P_CR_DISABLE,
	GOO_TI_VIDEO_ENCODER720P_CR_VARIABLE,
	GOO_TI_VIDEO_ENCODER720P_CR_CONSTANT,
	GOO_TI_VIDEO_ENCODER720P_CR_VARIABLE_SKIP,
	GOO_TI_VIDEO_ENCODER720P_CR_CONSTANT_SKIP

} GooTiVideoEncoder720pControlRate;

struct _GooTiVideoEncoder720p
{
	GooComponent parent;
	OMX_VIDEO_PARAM_BITRATETYPE* control_rate_param;
	guint frame_interval;
};

struct _GooTiVideoEncoder720pClass
{
	GooComponentClass parent_class;
};

GType goo_ti_video_encoder720p_get_type (void);
GType goo_ti_video_encoder720p_control_rate_get_type (void);

void goo_ti_video_encoder720p_set_control_rate (GooTiVideoEncoder720p* self,
						GooTiVideoEncoder720pControlRate control_rate);
guint goo_ti_video_encoder720p_get_control_rate (GooTiVideoEncoder720p* self);

G_END_DECLS

#endif /* _GOO_TI_VIDEO_DECODER720P_H_ */
