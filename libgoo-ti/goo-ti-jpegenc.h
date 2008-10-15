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

#ifndef _GOO_TI_JPEGENC_H_
#define _GOO_TI_JPEGENC_H_

#include <goo-component.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_JPEGENC \
        (goo_ti_jpegenc_get_type ())
#define GOO_TI_JPEGENC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_JPEGENC, GooTiJpegEnc))
#define GOO_TI_JPEGENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_JPEGENC, GooTiJpegEncClass))
#define GOO_IS_TI_JPEGENC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_JPEGENC))
#define GOO_IS_TI_JPEGENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_JPEGENC))
#define GOO_TI_JPEGENC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_JPEGENC_GET_CLASS, GooTiJpegEncClass))

typedef struct _GooTiJpegEnc GooTiJpegEnc;
typedef struct _GooTiJpegEncPriv GooTiJpegEncPriv;
typedef struct _ThumbnailJpegPrivate ThumbnailJpegPrivate;
typedef struct _GooTiJpegEncClass GooTiJpegEncClass;

/**
 * GooTiJpegEnc:
 * @param: The #OMX_IMAGE_PARAM_QFACTORTYPE structure
 *
 * It is the OMX JPEG encoder component
 */
struct _GooTiJpegEnc
{
        GooComponent parent;
        OMX_IMAGE_PARAM_QFACTORTYPE* param;
		ThumbnailJpegPrivate* thumbnail;
		gchar* comment;
};

struct _GooTiJpegEncClass
{
        GooComponentClass parent_class;
};

/**
 * GOO_TI_JPEGENC_GET_PARAM:
 * @jpegenc: A #GooTiJpegEnc instace
 *
 * Gets the #OMX_IMAGE_PARAM_QFACTORTYPE of the jpegenc
 *
 * Return value: The #OMX_IMAGE_PARAM_QFACTORTYPE pointer
 */
#define GOO_TI_JPEGENC_GET_PARAM(jpegenc) (GOO_TI_JPEGENC (jpegenc)->param)


GType goo_ti_jpegenc_get_type (void);

G_END_DECLS

#endif /* _GOO_TI_JPEGENC_H_ */
