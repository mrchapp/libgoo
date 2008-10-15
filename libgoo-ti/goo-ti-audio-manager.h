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

#ifndef _GOO_TI_AUDIO_MANAGER_H_
#define _GOO_TI_AUDIO_MANAGER_H_

#include <goo-component.h>
#include <AudioManagerAPI.h>

#define GOO_TYPE_TI_AUDIO_MANAGER \
	(goo_ti_audio_manager_get_type ())
#define GOO_TI_AUDIO_MANAGER(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_AUDIO_MANAGER, GooTiAudioManager))
#define GOO_TI_AUDIO_MANAGER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_AUDIO_MANAGER, GooTiAudioManagerClass))
#define GOO_IS_TI_AUDIO_MANAGER(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_AUDIO_MANAGER))
#define GOO_IS_TI_AUDIO_MANAGER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_AUDIO_MANAGER))
#define GOO_TI_AUDIO_MANAGER_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_AUDIO_MANAGER, GooTiAudioManagerClass))

/**
 * FIFO1:
 * The input command fifo device
 */
#define FIFO1 "/dev/fifo.1"

/**
 * FIFO2:
 * The output reponse fifo device
 */
#define FIFO2 "/dev/fifo.2"

typedef struct _GooTiAudioManager GooTiAudioManager;
typedef struct _GooTiAudioManagerClass GooTiAudioManagerClass;

/**
 * GooTiAudioManager:
 * Audio manager proxy
 */
struct _GooTiAudioManager
{
	GooObject parent;

	/*< protected >*/
	int fdwrite;
	int fdread;
	AM_COMMANDDATATYPE* cmd;
};

struct _GooTiAudioManagerClass
{
	GooObjectClass parent_class;
};

GType goo_ti_audio_manager_get_type (void);

GooTiAudioManager* goo_ti_audio_manager_new (GooComponent* component);
void goo_ti_audio_manager_set_component (GooTiAudioManager* self,
					 GooComponent* component);
gint goo_ti_audio_manager_get_stream_id (GooTiAudioManager* self);

#endif /* _GOO_TI_AUDIO_MANAGER_H_ */
