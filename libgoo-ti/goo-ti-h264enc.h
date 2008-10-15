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

#ifndef _GOO_TI_H264ENC_H_
#define _GOO_TI_H264ENC_H_

#include <goo-ti-video-encoder.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_H264ENC \
        (goo_ti_h264enc_get_type ())
#define GOO_TI_H264ENC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_H264ENC, GooTiH264Enc))
#define GOO_TI_H264ENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_H264ENC, GooTiH264EncClass))
#define GOO_IS_TI_H264ENC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_H264ENC))
#define GOO_IS_TI_H264ENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_H264ENC))
#define GOO_TI_H264ENC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_H264ENC_GET_CLASS, GooTiH264EncClass))

#define GOO_TI_H264ENC_LEVEL (goo_ti_h264enc_level_get_type())

typedef struct _GooTiH264Enc GooTiH264Enc;
typedef struct _GooTiH264EncClass GooTiH264EncClass;

struct _GooTiH264Enc
{
        GooTiVideoEncoder parent;
		OMX_VIDEO_PARAM_AVCTYPE* level_param;

};

struct _GooTiH264EncClass
{
        GooTiVideoEncoderClass parent_class;
};

/* functions */
GType goo_ti_h264enc_get_type (void);
GType goo_ti_h264enc_level_get_type (void);

GooComponent* goo_ti_h264enc_new ();

G_END_DECLS


#endif /* _GOO_TI_H264ENC_H_ */
