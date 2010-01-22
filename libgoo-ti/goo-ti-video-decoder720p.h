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

#ifndef _GOO_TI_VIDEO_DECODER720P_H_
#define _GOO_TI_VIDEO_DECODER720P_H_

#include <goo-component.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_VIDEO_DECODER720P		\
	(goo_ti_video_decoder720p_get_type ())
#define GOO_TI_VIDEO_DECODER720P(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_VIDEO_DECODER720P, GooTiVideoDecoder720p))
#define GOO_TI_VIDEO_DECODER720P_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_VIDEO_DECODER720P, GooTiVideoDecoder720pClass))
#define GOO_IS_TI_VIDEO_DECODER720P(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_VIDEO_DECODER720P))
#define GOO_IS_TI_VIDEO_DECODER720P_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_VIDEO_DECODER720P))
#define GOO_TI_VIDEO_DECODER720P_GET_CLASS(obj)				\
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_VIDEO_DECODER720P, GooTiVideoDecoder720pClass))

#define GOO_TI_VIDEO_DECODER720P_PROCESS_MODE		\
	(goo_ti_video_decoder720p_process_mode_get_type())

typedef enum _GooTiVideoDecoder720pProcessMode
{
	GOO_TI_VIDEO_DECODER720P_FRAMEMODE,
	GOO_TI_VIDEO_DECODER720P_STREAMMODE
} GooTiVideoDecoder720pProcessMode;

typedef struct _GooTiVideoDecoder720p GooTiVideoDecoder720p;
typedef struct _GooTiVideoDecoder720pClass GooTiVideoDecoder720pClass;

struct _GooTiVideoDecoder720p
{
	GooComponent parent;

	gboolean process_mode;
};

struct _GooTiVideoDecoder720pClass
{
	GooComponentClass parent_class;
};

GType goo_ti_video_decoder720p_get_type (void);
GType goo_ti_video_decoder720p_process_mode_get_type (void);

G_END_DECLS

#endif /* _GOO_TI_VIDEO_DECODER720P_H_ */

