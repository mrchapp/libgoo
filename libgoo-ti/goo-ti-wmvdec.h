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

#ifndef _GOO_TI_WMVDEC_H_
#define _GOO_TI_WMVDEC_H_

#include <goo-ti-video-decoder.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_WMVDEC \
        (goo_ti_wmvdec_get_type ())
#define GOO_TI_WMVDEC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_WMVDEC, GooTiWMVDec))
#define GOO_TI_WMVDEC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_WMVDEC, GooTiWMVDecClass))
#define GOO_IS_TI_WMVDEC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_WMVDEC))
#define GOO_IS_TI_WMVDEC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_WMVDEC))
#define GOO_TI_WMVDEC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_WMVDEC_GET_CLASS, GooTiWMVDecClass))

typedef struct _GooTiWMVDec GooTiWMVDec;
typedef struct _GooTiWMVDecClass GooTiWMVDecClass;
typedef struct _GooTiWMVDecPriv GooTiWMVDecPriv;


struct _GooTiWMVDec
{
        GooTiVideoDecoder parent;

};

struct _GooTiWMVDecClass
{
        GooTiVideoDecoderClass parent_class;
};

typedef enum
{
        GOO_TI_WMVDEC_FILE_TYPE_VC1,
        GOO_TI_WMVDEC_FILE_TYPE_RCV,
} GooTiWMVDecFileType;


/* functions */
GType goo_ti_wmvdec_get_type (void);
GooComponent* goo_ti_wmvdec_new ();

G_END_DECLS


#endif /* _GOO_TI_WMVDEC_H_ */
