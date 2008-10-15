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

#ifndef _GOO_TI_JPEGDEC_H_
#define _GOO_TI_JPEGDEC_H_

#include <goo-component.h>
//#include "OMX_JpegDec_Utils.h"

G_BEGIN_DECLS

#define GOO_TYPE_TI_JPEGDEC \
	(goo_ti_jpegdec_get_type ())
#define GOO_TI_JPEGDEC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_JPEGDEC, GooTiJpegDec))
#define GOO_TI_JPEGDEC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_JPEGDEC, GooTiJpegDecClass))
#define GOO_IS_TI_JPEGDEC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_JPEGDEC))
#define GOO_IS_TI_JPEGDEC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_JPEGDEC))
#define GOO_TI_JPEGDEC_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_JPEGDEC_GET_CLASS, GooTiJpegDecClass))

typedef struct _GooTiJpegDec GooTiJpegDec;
typedef struct _GooTiJpegDecPriv GooTiJpegDecPriv;
typedef struct _GooTiJpegDecClass GooTiJpegDecClass;

/**
 * GooTiJpegDecScale:
 * Scales proportions
 */
typedef enum
{
	GOO_TI_JPEGDEC_SCALE_800  = 800,
	GOO_TI_JPEGDEC_SCALE_400  = 400,
	GOO_TI_JPEGDEC_SCALE_200  = 200,
	GOO_TI_JPEGDEC_SCALE_NONE = 100,
	GOO_TI_JPEGDEC_SCALE_50	  = 50,
	GOO_TI_JPEGDEC_SCALE_25	  = 25,
	GOO_TI_JPEGDEC_SCALE_12	  = 12
} GooTiJpegDecScale;

typedef struct OMX_CUSTOM_IMAGE_DECODE_SECTION
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nMCURow;
    OMX_U32 nAU;
    OMX_BOOL bSectionsInput;
    OMX_BOOL bSectionsOutput;
}OMX_CUSTOM_IMAGE_DECODE_SECTION;

typedef struct OMX_CUSTOM_IMAGE_DECODE_SUBREGION
{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nXOrg;         /*Sectional decoding: X origin*/
    OMX_U32 nYOrg;         /*Sectional decoding: Y origin*/
    OMX_U32 nXLength;      /*Sectional decoding: X lenght*/
    OMX_U32 nYLength;      /*Sectional decoding: Y lenght*/
}OMX_CUSTOM_IMAGE_DECODE_SUBREGION;


/**
 * GooTiJpegDec:
 * @param: The #OMX_IMAGE_PARAM_QFACTORTYPE structure
 *
 * It is the OMX JPEG decoder component
 */
struct _GooTiJpegDec
{
	GooComponent parent;
	GooTiJpegDecScale scale;
	gboolean progressive;
	OMX_CUSTOM_IMAGE_DECODE_SECTION* section_decode;
	OMX_CUSTOM_IMAGE_DECODE_SUBREGION* subregion_decode;
};

struct _GooTiJpegDecClass
{
	GooComponentClass parent_class;
};

#define GOO_TI_JPEGDEC_GET_SECTION_DECODE(jpegdec) (GOO_TI_JPEGDEC (jpegdec)->section_decode)
#define GOO_TI_JPEGDEC_GET_SUBREGION_DECODE(jpegdec) (GOO_TI_JPEGDEC (jpegdec)->subregion_decode)


GType goo_ti_jpegdec_scale_get_type();
GType goo_ti_jpegdec_get_type (void);

G_END_DECLS

#endif /* _GOO_TI_JPEGDEC_H_  */
