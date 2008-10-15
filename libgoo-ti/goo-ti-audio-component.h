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

#ifndef _GOO_TI_AUDIO_COMPONENT_H_
#define _GOO_TI_AUDIO_COMPONENT_H_

#include <goo-component.h>
#include <goo-ti-audio-manager.h>

#if 1
#include <TIDspOmx.h>
#endif

G_BEGIN_DECLS

#define GOO_TYPE_TI_AUDIO_COMPONENT \
	(goo_ti_audio_component_get_type ())
#define GOO_TI_AUDIO_COMPONENT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_AUDIO_COMPONENT, GooTiAudioComponent))
#define GOO_TI_AUDIO_COMPONENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_AUDIO_COMPONENT, GooTiAudioComponentClass))
#define GOO_IS_TI_AUDIO_COMPONENT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_AUDIO_COMPONENT))
#define GOO_IS_TI_AUDIO_COMPONENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_AUDIO_COMPONENT))
#define GOO_TI_AUDIO_COMPONENT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_AUDIO_COMPONENT, GooTiAudioComponentClass))

typedef struct _GooTiAudioComponent GooTiAudioComponent;
typedef struct _GooTiAudioComponentClass GooTiAudioComponentClass;

/**
 * GooTiAudioComponent:
 *
 * Base clase for Audio TI OMX components
 */
struct _GooTiAudioComponent
{
	/*< protected >*/
	GooComponent parent;

	GooTiAudioManager* manager;

	gboolean dasf_mode;
	gboolean frame_mode;
	gboolean acoustic_mode;
	TI_OMX_DSP_DEFINITION *audioinfo;
	TI_OMX_DATAPATH datapath;

	guint volume;
	gboolean mute;
	gint stream_id;

	gchar *dasf_param_name;
	gchar *acoustic_param_name;
	gchar *streamid_param_name;
	gchar *datapath_param_name;
	gchar *frame_param_name;
};

struct _GooTiAudioComponentClass
{
	GooComponentClass parent_class;

	GooIterator* (*iterate_ports_func) (GooTiAudioComponent *self);
};

GType goo_ti_audio_component_get_type (void);

void goo_ti_audio_component_set_dasf_mode (GooTiAudioComponent* self,
					   gboolean dasf_mode);
void goo_ti_audio_component_set_volume (GooTiAudioComponent* self,
					guint volume);
void goo_ti_audio_component_set_mute (GooTiAudioComponent* self,
				      gboolean mute);
void goo_ti_audio_component_set_frame_mode (GooTiAudioComponent* self,
					    gboolean frame_mode);
void goo_ti_audio_component_set_acoustic_mode (GooTiAudioComponent* self,
					       gboolean frame_mode);

gboolean goo_ti_audio_component_is_dasf_mode (GooTiAudioComponent* self);
guint goo_ti_audio_component_get_volume (GooTiAudioComponent* self);
gboolean goo_ti_audio_component_is_mute (GooTiAudioComponent* self);
gint goo_ti_audio_component_get_stream_id (GooTiAudioComponent* self);

void goo_ti_audio_component_audio_manager_activate (GooTiAudioComponent* self);


G_END_DECLS

#endif /* _GOO_TI_AUDIO_COMPONENT_H_ */
