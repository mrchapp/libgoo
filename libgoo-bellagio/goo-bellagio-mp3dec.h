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

#ifndef _GOO_BELLAGIO_MP3DEC_H_
#define _GOO_BELLAGIO_MP3DEC_H_

#include <goo-component.h>

G_BEGIN_DECLS

#define GOO_TYPE_BELLAGIO_MP3DEC \
        (goo_bellagio_mp3dec_get_type ())
#define GOO_BELLAGIO_MP3DEC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_BELLAGIO_MP3DEC, GooBellagioMp3Dec))
#define GOO_BELLAGIO_MP3DEC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_BELLAGIO_MP3DEC, GooBellagioMp3DecClass))
#define GOO_IS_BELLAGIO_MP3DEC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_BELLAGIO_MP3DEC))
#define GOO_IS_BELLAGIO_MP3DEC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_BELLAGIO_MP3DEC))
#define GOO_BELLAGIO_MP3DEC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_BELLAGIO_MP3DEC_GET_CLASS, GooBellagioMp3DecClass))

typedef struct _GooBellagioMp3Dec GooBellagioMp3Dec;
typedef struct _GooBellagioMp3DecClass GooBellagioMp3DecClass;

struct _GooBellagioMp3Dec
{
        GooComponent parent;

	OMX_AUDIO_PARAM_PCMMODETYPE* param;
};

struct _GooBellagioMp3DecClass
{
        GooComponentClass parent_class;
};

/**
 * GOO_BELLAGIO_MP3DEC_GET_PARAM:
 * @dec: a #GooBellagioMp3Dec instance
 *
 * Gets the OMX_AUDIO_PARAM_ structure of the MP3 component.
 *
 * Return value: The OMX_AUDIO_PARAM_ pointer.
 */
#define GOO_BELLAGIO_MP3DEC_GET_PARAM(dec) (GOO_BELLAGIO_MP3DEC (dec)->param)

/* funcbellagioons */
GType goo_bellagio_mp3dec_get_type (void);
GooComponent* goo_bellagio_mp3dec_new ();

G_END_DECLS


#endif /* _GOO_BELLAGIO_MP3DEC_H_ */
