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

#ifndef _GOO_TI_AUDIO_ENCODER_H_
#define _GOO_TI_AUDIO_ENCODER_H_

#include <goo-ti-audio-component.h>

#if 0
#include <TIDspOmx.h>
#endif

G_BEGIN_DECLS

#define GOO_TYPE_TI_AUDIO_ENCODER \
	(goo_ti_audio_encoder_get_type ())
#define GOO_TI_AUDIO_ENCODER(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_AUDIO_ENCODER, GooTiAudioEncoder))
#define GOO_TI_AUDIO_ENCODER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_AUDIO_ENCODER, GooTiAudioEncoderClass))
#define GOO_IS_TI_AUDIO_ENCODER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_AUDIO_ENCODER))
#define GOO_IS_TI_AUDIO_ENCODER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_AUDIO_ENCODER))
#define GOO_TI_AUDIO_ENCODER_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_AUDIO_ENCODER, GooTiAudioEncoderClass))

typedef struct _GooTiAudioEncoder GooTiAudioEncoder;
typedef struct _GooTiAudioEncoderClass GooTiAudioEncoderClass;

/**
 * GooTiAudioEncoder:
 *
 * Base clase for Audio TI OMX components
 */
struct _GooTiAudioEncoder
{
	/*< protected >*/
	GooTiAudioComponent parent;
	gint number_buffers;
};

struct _GooTiAudioEncoderClass
{
	GooTiAudioComponentClass parent_class;
};

GType goo_ti_audio_encoder_get_type (void);

GooIterator*
goo_ti_audio_encoder_iterate_ports (GooTiAudioComponent *self);
void goo_ti_audio_encoder_set_stream_direction (GooTiAudioComponent *self);
void goo_ti_audio_encoder_set_number_buffers (GooTiAudioEncoder *self, gint number_buffers);
gint goo_ti_audio_encoder_get_number_buffers (GooTiAudioEncoder *self);

G_END_DECLS

#endif /* _GOO_TI_AUDIO_ENCODER_H_ */
