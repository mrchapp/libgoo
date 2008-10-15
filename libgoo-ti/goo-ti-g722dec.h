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

#ifndef _GOO_TI_G722DEC_H_
#define _GOO_TI_G722DEC_H_

#include <goo-ti-audio-component.h>
#include <goo-ti-audio-decoder.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_G722DEC \
        (goo_ti_g722dec_get_type ())
#define GOO_TI_G722DEC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_G722DEC, GooTiG722Dec))
#define GOO_TI_G722DEC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_G722DEC, GooTiG722DecClass))
#define GOO_IS_TI_G722DEC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_G722DEC))
#define GOO_IS_TI_G722DEC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_G722DEC))
#define GOO_TI_G722DEC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_G722DEC_GET_CLASS, GooTiG722DecClass))

typedef struct _GooTiG722Dec GooTiG722Dec;
typedef struct _GooTiG722DecClass GooTiG722DecClass;

struct _GooTiG722Dec
{
        GooTiAudioDecoder parent;

        OMX_AUDIO_PARAM_PCMMODETYPE *input_param;
        OMX_AUDIO_PARAM_PCMMODETYPE *output_param;
};

struct _GooTiG722DecClass
{
        GooTiAudioDecoderClass parent_class;
};

/**
 * GOO_TI_G722DEC_GET_PARAM:
 * @dec: a #GooTiG722Dec instance
 *
 * Gets the OMX_AUDIO_PARAM_ structure of the PCM component.
 *
 * Return value: The OMX_AUDIO_PARAM_ pointer.
 */
#define GOO_TI_G722DEC_GET_INPUT_PARAM(dec) (GOO_TI_G722DEC (dec)->input_param)
#define GOO_TI_G722DEC_GET_OUTPUT_PARAM(dec) (GOO_TI_G722DEC (dec)->output_param)

/* functions */
GType goo_ti_g722dec_get_type (void);
GooComponent* goo_ti_g722dec_new ();

G_END_DECLS


#endif /* _GOO_TI_G722DEC_H_ */
