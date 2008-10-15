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

#ifndef _GOO_TI_WBAMRENC_H_
#define _GOO_TI_WBAMRENC_H_

#include <goo-ti-audio-component.h>
#include <goo-ti-audio-encoder.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_WBAMRENC \
        (goo_ti_wbamrenc_get_type ())
#define GOO_TI_WBAMRENC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_WBAMRENC, GooTiWbAmrEnc))
#define GOO_TI_WBAMRENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_WBAMRENC, GooTiWbAmrEncClass))
#define GOO_IS_TI_WBAMRENC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_WBAMRENC))
#define GOO_IS_TI_WBAMRENC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_WBAMRENC))
#define GOO_TI_WBAMRENC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_WBAMRENC_GET_CLASS, GooTiWbAmrEncClass))

typedef struct _GooTiWbAmrEnc GooTiWbAmrEnc;
typedef struct _GooTiWbAmrEncClass GooTiWbAmrEncClass;

struct _GooTiWbAmrEnc
{
	GooTiAudioEncoder parent;

	OMX_AUDIO_PARAM_AMRTYPE *output_port_param;
};

struct _GooTiWbAmrEncClass
{
        GooTiAudioEncoderClass parent_class;
};

/**
 * GOO_TI_WBAMRENC_GET_PARAM:
 * @enc: a #GooTiWbAmrEnc instance
 *
 * Gets the OMX_AUDIO_PARAM_ structure of the AAC component.
 *
 * Return value: The OMX_AUDIO_PARAM_ pointer.
 */
#define GOO_TI_WBAMRENC_GET_OUTPUT_PORT_PARAM(enc) \
	(GOO_TI_WBAMRENC (enc)->output_port_param)

#define GOO_TI_WBAMRENC_INPUT_BUFFER_SIZE 640
#define GOO_TI_WBAMRENC_OUTPUT_BUFFER_SIZE 116

GType goo_ti_wbamrenc_get_type (void);

G_END_DECLS


#endif /* _GOO_TI_WBAMRENC_H_ */

