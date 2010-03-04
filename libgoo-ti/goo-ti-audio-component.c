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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-ti-audio-component.h>
#include <goo-ti-clock.h>
#include <goo-utils.h>

/* WRONG! */
/* for index */
#include <OMX_Clock.h>

#define MAX_VOLUME 100
#define MIN_VOLUME 0

enum _GooTiAudioComponentProp
{
	PROP_0,
	PROP_DASFMODE,
	PROP_VOLUME,
	PROP_MUTE,
	PROP_FRAMEMODE,
	PROP_ACOUSTICMODE,
	PROP_DATAPATH,
	PROP_STREAMID
};

GooIterator* goo_ti_audio_component_iterate_ports (GooTiAudioComponent *self);

static gboolean
_goo_ti_audio_component_set_acoustic_mode (GooTiAudioComponent *self,
					   gboolean acoustic_mode)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (self->acoustic_param_name != NULL);

	gboolean retval = FALSE;
	self->audioinfo->acousticMode = (acoustic_mode) ? 1 : 0;
	retval = goo_component_set_config_by_name (GOO_COMPONENT (self),
						   self->acoustic_param_name,
						   self->audioinfo);
	if (retval == TRUE)
	{
		self->acoustic_mode = acoustic_mode;
		GOO_OBJECT_INFO (self, "acoustic mode = %d",
				 self->audioinfo->acousticMode);
	}

	return retval;
}

static gboolean
_goo_ti_audio_component_set_frame_mode (GooTiAudioComponent* self,
					gboolean frame_mode)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (self->frame_param_name != NULL);

	gboolean retval = FALSE;
	self->audioinfo->framemode = (frame_mode) ? 1 : 0;
	retval = goo_component_set_config_by_name (GOO_COMPONENT (self),
						   self->frame_param_name,
						   self->audioinfo);
	if (retval == TRUE)
	{
		self->frame_mode = frame_mode;
		GOO_OBJECT_INFO (self, "dasf mode = %d",
				 self->audioinfo->framemode);

#if 0
		/* output port is tunneled in DASF mode */
		GooPort* port = NULL;
		GooIterator* iter = goo_component_iterate_output_ports
			(GOO_COMPONENT (self));
		while (!goo_iterator_is_done (iter))
		{
			port = GOO_PORT (goo_iterator_get_current (iter));
			g_object_set (G_OBJECT (port),
				      "tunneled", self->dasf_mode, NULL);
			g_object_unref (G_OBJECT (port));
			goo_iterator_next (iter);
		}
		g_object_unref (G_OBJECT (iter));
#endif
	}

	return retval;
}

static gboolean
_goo_ti_audio_component_set_data_path (GooTiAudioComponent* self,
				       guint datapath)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (self->datapath_param_name != NULL);
	gboolean retval = FALSE;

	self->datapath= datapath ? 1: 0;
	GOO_OBJECT_DEBUG (self, "datapath set to: %p",
			  self->datapath);
	retval = goo_component_set_config_by_name (GOO_COMPONENT (self),
						   self->datapath_param_name,
						   &(self->datapath));

	GOO_OBJECT_DEBUG (self, "datapath set to: %d", self->datapath);

	return retval;
}

static gboolean
_goo_ti_audio_component_get_stream_id (GooTiAudioComponent* self)
{
	g_assert (GOO_TI_AUDIO_COMPONENT (self));
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (self->streamid_param_name != NULL);

	gboolean retval = FALSE;

	if (self->stream_id > -1)
	{
		return retval;
	}

	GOO_OBJECT_DEBUG (self, "");

#if 0  /* this is deprected since audio manager */
	TI_OMX_STREAM_INFO param;

	retval = goo_component_get_config_by_name (GOO_COMPONENT (self),
						   self->streamid_param_name,
						   &param);

	if (retval == TRUE)
	{
		self->stream_id = param.streamId;
		GOO_INFO ("stream id = %d", self->stream_id);
	}
#else
	if (G_LIKELY (self->manager != NULL))
	{
		self->stream_id = goo_ti_audio_manager_get_stream_id (self->manager);
		if (self->stream_id > 0)
		{
			retval = TRUE;
		}
		else
		{
			retval = FALSE;
		}
	}
	else
	{
		retval = FALSE;
	}
#endif


	return retval;
}

static gboolean
_goo_ti_audio_component_set_dasf_mode (GooTiAudioComponent* self,
				       gboolean dasf_mode)
{
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (self));
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (self->dasf_param_name != NULL);

	gboolean retval = FALSE;

	self->audioinfo->dasfMode = (dasf_mode) ? 1 : 0;

	if (self->audioinfo->dasfMode == 1)
	{
		if (G_UNLIKELY (self->manager == NULL))
		{
			self->manager = goo_ti_audio_manager_new
				(GOO_COMPONENT (self));

			_goo_ti_audio_component_get_stream_id (self);
		}

		g_assert (self->stream_id > 0);

		self->audioinfo->streamId = self->stream_id;
		GOO_OBJECT_INFO (self, "stream id = %ld",
				 self->audioinfo->streamId);
	}

	retval = goo_component_set_config_by_name (GOO_COMPONENT (self),
						   self->dasf_param_name,
						   self->audioinfo);

	if (retval == TRUE && self->audioinfo->dasfMode == 1)
	{
		self->dasf_mode = dasf_mode;
		GOO_OBJECT_INFO (self, "dasf mode = %d",
				 self->audioinfo->dasfMode);

		/* output port is tunneled in DASF mode */
		GooPort* port = NULL;
		GooIterator *iter =
			goo_ti_audio_component_iterate_ports (self);
		while (!goo_iterator_is_done (iter))
		{
			port = GOO_PORT (goo_iterator_get_current (iter));
			g_object_set (G_OBJECT (port),
				      "tunneled", self->dasf_mode, NULL);
			g_object_unref (G_OBJECT (port));
			goo_iterator_next (iter);
		}
		g_object_unref (G_OBJECT (iter));
	}

	return retval;
}

static gboolean
_goo_ti_audio_component_set_mute (GooTiAudioComponent* self, gboolean mute)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	g_return_val_if_fail (self->dasf_mode == TRUE, FALSE);

	gboolean retval = FALSE;

	OMX_AUDIO_CONFIG_MUTETYPE* param = NULL;
	param = g_new0 (OMX_AUDIO_CONFIG_MUTETYPE, 1);
	GOO_INIT_PARAM (param, OMX_AUDIO_CONFIG_MUTETYPE);

	param->nPortIndex = 0;
	param->bMute = (mute) ? OMX_TRUE : OMX_FALSE;
	retval = goo_component_set_config_by_index (GOO_COMPONENT (self),
						    OMX_IndexConfigAudioMute,
						    param);

	if (retval == TRUE)
	{
		self->mute = mute;
		GOO_OBJECT_INFO (self, "mute = %s",
				 (param->bMute) ? "true" : "false");
	}

	g_free (param);

	return retval;
}

static gboolean
_goo_ti_audio_component_set_volume (GooTiAudioComponent* self, guint volume)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	g_assert (volume >= MIN_VOLUME && volume <= MAX_VOLUME);

	g_return_val_if_fail (self->dasf_mode == TRUE, FALSE);

	gboolean retval = FALSE;
	OMX_AUDIO_CONFIG_VOLUMETYPE* param = NULL;
	param = g_new0 (OMX_AUDIO_CONFIG_VOLUMETYPE, 1);
	GOO_INIT_PARAM (param, OMX_AUDIO_CONFIG_VOLUMETYPE);

	param->bLinear = OMX_FALSE;
	param->sVolume.nValue = volume;
	param->sVolume.nMin = MIN_VOLUME;
	param->sVolume.nMax = MAX_VOLUME;

	retval = goo_component_set_config_by_index (GOO_COMPONENT (self),
						    OMX_IndexConfigAudioVolume,
						    param);

	if (retval == TRUE)
	{
		self->volume = volume;
		GOO_OBJECT_INFO (self, "volume = %d", param->sVolume.nValue);
	}

	g_free (param);

	return retval;
}

G_DEFINE_TYPE (GooTiAudioComponent, goo_ti_audio_component,
	       GOO_TYPE_COMPONENT);

static void
goo_ti_audio_component_init (GooTiAudioComponent* self)
{
	GOO_COMPONENT (self)->port_param_type = OMX_IndexParamAudioInit;

	self->dasf_mode = FALSE;
	self->frame_mode = FALSE;
	self->acoustic_mode = FALSE;
	self->volume = 50;
	self->mute = TRUE;
	self->stream_id = -1;

	self->audioinfo = g_new0 (TI_OMX_DSP_DEFINITION, 1);

	self->manager = NULL;

	self->dasf_param_name = NULL;
	self->acoustic_param_name = NULL;
	self->streamid_param_name = NULL;
	self->datapath_param_name = NULL;

	return;
}

static void
goo_ti_audio_component_dispose (GObject* object)
{
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (object));
	GooTiAudioComponent* self = GOO_TI_AUDIO_COMPONENT (object);

	(*G_OBJECT_CLASS (goo_ti_audio_component_parent_class)->dispose)
		(object);

	if (G_LIKELY (self->manager != NULL))
	{
		g_object_unref (self->manager);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_ti_audio_component_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (object));
	GooTiAudioComponent* self = GOO_TI_AUDIO_COMPONENT (object);

	if (G_LIKELY (self->dasf_param_name))
	{
		g_free (self->dasf_param_name);
		self->dasf_param_name = NULL;
	}
	if (G_LIKELY (self->frame_param_name))
	{
		g_free (self->frame_param_name);
		self->frame_param_name = NULL;
	}
	if (G_LIKELY (self->acoustic_param_name))
	{
		g_free (self->acoustic_param_name);
		self->acoustic_param_name = NULL;
	}
	if (G_LIKELY (self->datapath_param_name))
	{
		g_free (self->datapath_param_name);
		self->datapath_param_name = NULL;
	}
	if (G_LIKELY (self->audioinfo))
	{
		g_free (self->audioinfo);
		self->audioinfo = NULL;
	}
	if (G_LIKELY (self->streamid_param_name))
	{
		g_free (self->streamid_param_name);
		self->streamid_param_name = NULL;
	}

	GOO_COMPONENT (self)->port_param_type = OMX_IndexComponentStartUnused;

	(*G_OBJECT_CLASS (goo_ti_audio_component_parent_class)->finalize)
		(object);

	return;
}

static void
goo_ti_audio_component_set_property (GObject* object, guint prop_id,
				     const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (object));
	GooTiAudioComponent* self = GOO_TI_AUDIO_COMPONENT (object);

	switch (prop_id)
	{
	case PROP_DASFMODE:
		_goo_ti_audio_component_set_dasf_mode
			(self, g_value_get_boolean (value));
		break;
	case PROP_MUTE:
		_goo_ti_audio_component_set_mute
			(self, g_value_get_boolean (value));
		break;
	case PROP_VOLUME:
		_goo_ti_audio_component_set_volume
			(self, g_value_get_uint (value));
		break;
	case PROP_FRAMEMODE:
		_goo_ti_audio_component_set_frame_mode
			(self, g_value_get_boolean (value));
		break;
	case PROP_ACOUSTICMODE:
		_goo_ti_audio_component_set_acoustic_mode
			(self, g_value_get_boolean (value));
		break;
	case PROP_DATAPATH:
		_goo_ti_audio_component_set_data_path
			(self, g_value_get_uint (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_audio_component_get_property (GObject* object, guint prop_id,
				     GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (object));
	GooTiAudioComponent* self = GOO_TI_AUDIO_COMPONENT (object);

	switch (prop_id)
	{
	case PROP_DASFMODE:
		g_value_set_boolean (value, self->dasf_mode);
		break;
	case PROP_MUTE:
		g_value_set_boolean (value, self->mute);
		break;
	case PROP_VOLUME:
		g_value_set_uint (value, self->volume);
		break;
	case PROP_STREAMID:
		_goo_ti_audio_component_get_stream_id (self);
		g_value_set_int (value, self->stream_id);
		break;
	case PROP_FRAMEMODE:
		g_value_set_boolean (value, self->frame_mode);
		break;
	case PROP_ACOUSTICMODE:
		g_value_set_boolean (value, self->acoustic_mode);
		break;
	case PROP_DATAPATH:
		g_value_set_uint (value, self->datapath);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

gboolean
goo_ti_audio_component_set_clock (GooComponent *component,
				  GooComponent *clock)
{
	g_assert (GOO_IS_TI_CLOCK (clock));
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (component));

	GooTiAudioComponent* self = GOO_TI_AUDIO_COMPONENT (component);

	gboolean retval = FALSE;
	gint streamid = goo_ti_audio_component_get_stream_id (self);

	self->clock = clock;
	retval = goo_component_set_config_by_index (clock,
						    OMX_IndexCustomSetStreamId,
						    (void *) &streamid);
	return retval;
}

static void
goo_ti_audio_component_class_init (GooTiAudioComponentClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	klass->iterate_ports_func = NULL;

	g_klass->dispose = goo_ti_audio_component_dispose;
	g_klass->finalize = goo_ti_audio_component_finalize;
	g_klass->set_property = goo_ti_audio_component_set_property;
	g_klass->get_property = goo_ti_audio_component_get_property;

	GParamSpec* spec;
	spec = g_param_spec_boolean ("dasf-mode", "DASF mode",
				     "Activate/Deactivate the DASF mode",
				     FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_DASFMODE, spec);

	spec = g_param_spec_boolean ("frame-mode", "Frame mode",
				     "Activate/Deactivate the Frame mode",
				     FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_FRAMEMODE, spec);

	spec = g_param_spec_boolean ("acdn-mode", "Acoustic mode",
				     "Activate/Deactivate the Acoustic mode",
				     FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_ACOUSTICMODE, spec);

	spec = g_param_spec_boolean ("mute", "Mute",
				     "Mute in DASF mode",
				     TRUE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_MUTE, spec);

	spec = g_param_spec_uint ("data-path", "Data Path",
				  "Audio Manager Data Path",
				  0, 2, 0, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_DATAPATH, spec);

	spec = g_param_spec_uint ("volume", "Volume",
				  "Volume in DASF mode",
				  0, 100, 50, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_VOLUME, spec);

	spec = g_param_spec_int ("streamid", "Stream ID", "Get stream ID",
				 -1, G_MAXINT, -1, G_PARAM_READABLE);
	g_object_class_install_property (g_klass, PROP_STREAMID, spec);

	GooComponentClass* c_klass = GOO_COMPONENT_CLASS (klass);
	c_klass->set_clock_func = goo_ti_audio_component_set_clock;

	return;
}

GooIterator*
goo_ti_audio_component_iterate_ports (GooTiAudioComponent *self)
{
	GOO_OBJECT_DEBUG (self, "Enter");
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (self));
	GooTiAudioComponentClass *klass =
		GOO_TI_AUDIO_COMPONENT_GET_CLASS (self);
	GooIterator *iter=NULL;

	if (klass->iterate_ports_func != NULL)
	{
		iter = (klass->iterate_ports_func) (self);
	}
	GOO_OBJECT_DEBUG (self, "Exit");
	return iter;
}

/**
 * goo_ti_audio_component_set_dasf_mode:
 * @self: An #GooTiAudioComponent instance
 * @dasf_mode: a boolean value
 *
 * This method will actividate/deactivate de DASF mode in the specified audio
 * component.
 **/
void
goo_ti_audio_component_set_dasf_mode (GooTiAudioComponent* self,
				      gboolean dasf_mode)
{
	g_assert (self != NULL);

	g_object_set (G_OBJECT (self), "dasf-mode", dasf_mode, NULL);

	return;
}

/**
 * goo_ti_audio_component_set_frame_mode:
 * @self: An #GooTiAudioComponent instance
 * @dasf_mode: a boolean value
 *
 * This method will actividate/deactivate de Frame mode in the specified audio
 * component.
 **/
void
goo_ti_audio_component_set_frame_mode (GooTiAudioComponent* self,
				      gboolean frame_mode)
{
	g_assert (self != NULL);

	g_object_set (G_OBJECT (self), "frame-mode", frame_mode, NULL);

	return;
}

/**
 * goo_ti_audio_component_set_acoustic_mode:
 * @self: An #GooTiAudioComponent instance
 * @dasf_mode: a boolean value
 *
 * This method will actividate/deactivate the Acoustic mode in the specified audio
 * component.
 **/
void
goo_ti_audio_component_set_acoustic_mode (GooTiAudioComponent* self,
				      gboolean acoustic_mode)
{
	g_assert (self != NULL);

	g_object_set (G_OBJECT (self), "acdn-mode", acoustic_mode, NULL);

	return;
}

/**
 * goo_ti_audio_component_is_dasf_mode:
 * @self: An #GooTiAudioComponent instance
 *
 * Return if the DASF mode is activated in the audio component
 *
 * Return value: TRUE if the component is in DASF mode; FALSE otherwise
 **/
gboolean
goo_ti_audio_component_is_dasf_mode (GooTiAudioComponent* self)
{
	g_assert (self != NULL);

	gboolean retval = FALSE;
	g_object_get (G_OBJECT (self), "dasf-mode", &retval, NULL);

	return retval;
}

/**
 * goo_ti_audio_component_is_frame_mode:
 * @self: An #GooTiAudioComponent instance
 *
 * Return if the Frame mode is activated in the audio component
 *
 * Return value: TRUE if the component is in Frame mode; FALSE otherwise
 **/
gboolean
goo_ti_audio_component_is_frame_mode (GooTiAudioComponent* self)
{
	g_assert (self != NULL);

	gboolean retval = FALSE;
	g_object_get (G_OBJECT (self), "frame-mode", &retval, NULL);

	return retval;
}

/**
 * goo_ti_audio_component_set_volume:
 * @self: An #GooTiAudioComponent instance
 * @volume: An integer value from 0-100
 *
 * Set the volume value when the component is in DASF mode
 **/
void
goo_ti_audio_component_set_volume (GooTiAudioComponent* self, guint volume)
{
	g_assert (self != NULL);

	g_object_set (G_OBJECT (self), "volume", volume, NULL);
	return;
}

/**
 * goo_ti_audio_component_get_volume:
 * @self: An #GooTiAudioComponent instance
 *
 * Get the volume value when the component is in DASF mode
 *
 * Return value: if the component is in DASF mode will return a value between
 * 0 and 100;
 **/
guint
goo_ti_audio_component_get_volume (GooTiAudioComponent* self)
{
	g_assert (self != NULL);

	guint retval = 50;
	g_object_get (G_OBJECT (self), "volume", &retval, NULL);

	return retval;
}
/**
 * goo_ti_audio_component_set_data_path:
 * @self: An #GooTiAudioComponent instance
 * @data_path: a uint value
 *
 * This method will actividate/deactivate de DATA-PATH in the specified audio
 * component.
 **/
void
goo_ti_audio_component_set_data_path (GooTiAudioComponent* self,
				      guint data_path)
{
	g_assert (self != NULL);

	g_object_set (G_OBJECT (self), "data-path", data_path, NULL);

	return;
}

/**
 * goo_ti_audio_component_set_mute:
 * @self: An #GooTiAudioComponent instance
 * @mute: An boolean value
 *
 * Turn OFF and turn ON the mute when the audio component is in DASF mode
 **/
void
goo_ti_audio_component_set_mute (GooTiAudioComponent* self, gboolean mute)
{
	g_assert (self != NULL);

	g_object_set (G_OBJECT (self), "mute", mute, NULL);
	return;
}

/**
 * goo_ti_audio_component_is_mute:
 * @self: An #GooTiAudioComponent instance
 *
 * Verify the muteness of the component when it is in DASF mode
 *
 * Return value: TRUE if the component is mute; FALSE otherwise
 **/
gboolean
goo_ti_audio_component_is_mute (GooTiAudioComponent* self)
{
	g_assert (self != NULL);

	gboolean retval = FALSE;
	g_object_get (G_OBJECT (self), "mute", &retval, NULL);

	return retval;
}


/**
 * goo_ti_audio_component_get_stream_id:
 * @self: An #GooTiAudioComponent instance
 *
 * Extract the stream number processed in the DSP
 *
 * Return value: an integer value
 **/
gint
goo_ti_audio_component_get_stream_id (GooTiAudioComponent* self)
{
	g_assert (self != NULL);

	gint retval = 0;
	g_object_get (G_OBJECT (self), "streamid", &retval, NULL);

	return retval;
}

/**
 * goo_ti_audio_component_audio_manager_activate:
 * @self: A #GooTiAudioComponent instance
 *
 * dummy function
 */
void
goo_ti_audio_component_audio_manager_activate (GooTiAudioComponent* self)
{
	return;
}
