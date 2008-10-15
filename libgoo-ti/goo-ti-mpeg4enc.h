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

#ifndef _GOO_TI_MPEG4ENC_H_
#define _GOO_TI_MPEG4ENC_H_

#include <goo-ti-video-encoder.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_MPEG4ENC \
        (goo_ti_mpeg4enc_get_type ())
#define GOO_TI_MPEG4ENC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_MPEG4ENC, GooTiMpeg4Enc))
#define GOO_TI_MPEG4ENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_MPEG4ENC, GooTiMpeg4EncClass))
#define GOO_IS_TI_MPEG4ENC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_MPEG4ENC))
#define GOO_IS_TI_MPEG4ENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_MPEG4ENC))
#define GOO_TI_MPEG4ENC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_MPEG4ENC_GET_CLASS, GooTiMpeg4EncClass))

#define GOO_TI_MPEG4ENC_LEVEL (goo_ti_mpeg4enc_level_get_type())

typedef struct _GooTiMpeg4Enc GooTiMpeg4Enc;
typedef struct _GooTiMpeg4EncClass GooTiMpeg4EncClass;

struct _GooTiMpeg4Enc
{
        GooTiVideoEncoder parent;
	OMX_VIDEO_PARAM_MPEG4TYPE* level_param;

};

struct _GooTiMpeg4EncClass
{
        GooTiVideoEncoderClass parent_class;
};

/* functions */
GType goo_ti_mpeg4enc_get_type (void);
GType goo_ti_mpeg4enc_level_get_type (void);

GooComponent* goo_ti_mpeg4enc_new ();

G_END_DECLS


#endif /* _GOO_TI_MPEG4ENC_H_ */
