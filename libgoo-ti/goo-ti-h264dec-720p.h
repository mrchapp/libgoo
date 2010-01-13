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

#ifndef _GOO_TI_H264DEC720P_H_
#define _GOO_TI_H264DEC720P_H_

#include <goo-ti-video-decoder.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_H264DEC720P			\
	(goo_ti_h264dec_720p_get_type ())
#define GOO_TI_H264DEC720P(obj)						\
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_H264DEC720P, GooTiH264Dec720p))
#define GOO_TI_H264DEC720P_CLASS(klass)					\
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_H264DEC720P, GooTiH264Dec720pClass))
#define GOO_IS_TI_H264DEC720P(obj)						\
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_H264DEC720P))
#define GOO_IS_TI_H264DEC720P_CLASS(klass)					\
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_H264DEC720P))
#define GOO_TI_H264DEC720P_GET_CLASS(obj)					\
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_H264DEC720P_GET_CLASS, GooTiH264Dec720pClass))

typedef struct _GooTiH264Dec720p GooTiH264Dec720p;
typedef struct _GooTiH264Dec720pClass GooTiH264Dec720pClass;
typedef struct _GooTiH264Dec720pPriv GooTiH264Dec720pPriv;


struct _GooTiH264Dec720p
{
	GooTiVideoDecoder parent;
};

struct _GooTiH264Dec720pClass
{
	GooTiVideoDecoderClass parent_class;
};

typedef enum
{
        GOO_TI_H264DEC720P_NALU_BYTES_TYPE_0B,
        GOO_TI_H264DEC720P_NALU_BYTES_TYPE_1B,
        GOO_TI_H264DEC720P_NALU_BYTES_TYPE_2B,
        GOO_TI_H264DEC720P_NALU_BYTES_TYPE_3B,
        GOO_TI_H264DEC720P_NALU_BYTES_TYPE_4B,
} GooTiH264720pNALUBytesType;


/* functions */
GType goo_ti_h264dec_720p_get_type (void);
GooComponent* goo_ti_h264dec_720p_new ();

G_END_DECLS


#endif /* _GOO_TI_H264DEC720P_H_ */
