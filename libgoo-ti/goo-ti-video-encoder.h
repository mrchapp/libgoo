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

#ifndef _GOO_TI_VIDEO_ENCODER_H_
#define _GOO_TI_VIDEO_ENCODER_H_

#include <goo-component.h>
#include <OMX_Video.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_VIDEO_ENCODER \
	(goo_ti_video_encoder_get_type ())
#define GOO_TI_VIDEO_ENCODER(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_VIDEO_ENCODER, GooTiVideoEncoder))
#define GOO_TI_VIDEO_ENCODER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_VIDEO_ENCODER, GooTiVideoEncoderClass))
#define GOO_IS_TI_VIDEO_ENCODER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_VIDEO_ENCODER))
#define GOO_IS_TI_VIDEO_ENCODER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_VIDEO_ENCODER))
#define GOO_TI_VIDEO_ENCODER_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_VIDEO_ENCODER, GooTiVideoEncoderClass))

#define GOO_TI_VIDEO_ENCODER_CONTROL_RATE (goo_ti_video_encoder_control_rate_get_type())

typedef struct _GooTiVideoEncoder GooTiVideoEncoder;
typedef struct _GooTiVideoEncoderClass GooTiVideoEncoderClass;

typedef enum GooTiVideoEncoderControlRate
{
	GOO_TI_VIDEO_ENCODER_CR_DISABLE,
	GOO_TI_VIDEO_ENCODER_CR_VARIABLE,
	GOO_TI_VIDEO_ENCODER_CR_CONSTANT,
	GOO_TI_VIDEO_ENCODER_CR_VARIABLE_SKIP,
	GOO_TI_VIDEO_ENCODER_CR_CONSTANT_SKIP

} GooTiVideoEncoderControlRate;

struct _GooTiVideoEncoder
{
        GooComponent parent;
	OMX_VIDEO_PARAM_BITRATETYPE* control_rate_param;
	guint frame_interval;
};

struct _GooTiVideoEncoderClass
{
        GooComponentClass parent_class;
};

GType goo_ti_video_encoder_get_type (void);
GType goo_ti_video_encoder_control_rate_get_type (void);

void goo_ti_video_encoder_set_control_rate (GooTiVideoEncoder* self,
					    GooTiVideoEncoderControlRate control_rate);
guint goo_ti_video_encoder_get_control_rate (GooTiVideoEncoder* self);

G_END_DECLS

#endif /* _GOO_TI_VIDEO_DECODER_H_ */

