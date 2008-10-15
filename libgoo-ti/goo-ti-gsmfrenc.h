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

#ifndef _GOO_TI_GSMFRENC_H_
#define _GOO_TI_GSMFRENC_H_

#include <goo-ti-audio-component.h>
#include <goo-ti-audio-encoder.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_GSMFRENC \
        (goo_ti_gsmfrenc_get_type ())
#define GOO_TI_GSMFRENC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_GSMFRENC, GooTiGsmFrEnc))
#define GOO_TI_GSMFRENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_GSMFRENC, GooTiGsmFrEncClass))
#define GOO_IS_TI_GSMFRENC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_GSMFRENC))
#define GOO_IS_TI_GSMFRENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_GSMFRENC))
#define GOO_TI_GSMFRENC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_GSMFRENC_GET_CLASS, GooTiGsmFrEncClass))

typedef struct _GooTiGsmFrEnc GooTiGsmFrEnc;
typedef struct _GooTiGsmFrEncClass GooTiGsmFrEncClass;

struct _GooTiGsmFrEnc
{
	GooTiAudioEncoder parent;

	OMX_AUDIO_PARAM_GSMFRTYPE *output_port_param;
};

struct _GooTiGsmFrEncClass
{
        GooTiAudioEncoderClass parent_class;
};

/**
 * GOO_TI_GSMFRENC_GET_PARAM:
 * @enc: a #GooTiGsmFrEnc instance
 *
 * Gets the OMX_AUDIO_PARAM_GSMFRTYPE structure of the GSM component.
 *
 * Return value: The OMX_AUDIO_PARAM_GSMFRTYPE pointer.
 */

#define GOO_TI_GSMFRENC_GET_OUTPUT_PORT_PARAM(enc) \
	(GOO_TI_GSMFRENC (enc)->output_port_param)

#define GOO_TI_GSMFRENC_INPUT_BUFFER_SIZE 320
#define GOO_TI_GSMFRENC_OUTPUT_BUFFER_SIZE 34

GType goo_ti_gsmfrenc_get_type (void);

G_END_DECLS


#endif /* _GOO_TI_GSMFRENC_H_ */

