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

#ifndef _GOO_TI_VIDEO_DECODER_H_
#define _GOO_TI_VIDEO_DECODER_H_

#include <goo-component.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_VIDEO_DECODER		\
	(goo_ti_video_decoder_get_type ())
#define GOO_TI_VIDEO_DECODER(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_VIDEO_DECODER, GooTiVideoDecoder))
#define GOO_TI_VIDEO_DECODER_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_VIDEO_DECODER, GooTiVideoDecoderClass))
#define GOO_IS_TI_VIDEO_DECODER(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_VIDEO_DECODER))
#define GOO_IS_TI_VIDEO_DECODER_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_VIDEO_DECODER))
#define GOO_TI_VIDEO_DECODER_GET_CLASS(obj)				\
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_VIDEO_DECODER, GooTiVideoDecoderClass))

#define GOO_TI_VIDEO_DECODER_PROCESS_MODE		\
	(goo_ti_video_decoder_process_mode_get_type())

typedef enum _GooTiVideoDecoderProcessMode
{
	GOO_TI_VIDEO_DECODER_FRAMEMODE,
	GOO_TI_VIDEO_DECODER_STREAMMODE
} GooTiVideoDecoderProcessMode;

typedef struct _GooTiVideoDecoder GooTiVideoDecoder;
typedef struct _GooTiVideoDecoderClass GooTiVideoDecoderClass;

struct _GooTiVideoDecoder
{
	GooComponent parent;

	gboolean process_mode;
};

struct _GooTiVideoDecoderClass
{
	GooComponentClass parent_class;
};

GType goo_ti_video_decoder_get_type (void);
GType goo_ti_video_decoder_process_mode_get_type (void);

G_END_DECLS

#endif /* _GOO_TI_VIDEO_DECODER_H_ */

