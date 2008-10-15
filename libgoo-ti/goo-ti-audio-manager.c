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

/**
 * SECTION:goo-ti-audio-manager
 * @short_description: Object which represent a command proxy to the
 *		       TI audio manager.
 * @see_alos: #GooTiAudioComponent
 *
 * A #GooTiAudioComponent must to establish communication, outside of the
 * OMX spec, with the TI audio manager in order to reserve a stream id.
 * This class represents that communcation protocol
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-ti-audio-manager.h>
#include <goo-utils.h>
#include <goo-ti-audio-component.h>
#include <goo-ti-audio-encoder.h>
#include <goo-ti-audio-decoder.h>

/* open & close calls */
#include <fcntl.h>
#include <unistd.h>

/* these fifos ared defined in
 * $OMX/system/src/openmax_il/audio_manager/inc/AudioManager.h
 * which is not exportated. POSSIBLE DR!
 */
#define FIFO1 "/dev/fifo.1"
#define FIFO2 "/dev/fifo.2"

enum _GooTiAudioManagerProp
{
	PROP_0,
	PROP_COMPONENT
};

G_DEFINE_TYPE (GooTiAudioManager, goo_ti_audio_manager, GOO_TYPE_OBJECT)

/**
 * goo_ti_audio_manager_new:
 * @component: A #GooTiAudioComponent instance
 *
 * Get a new audio manager proxy associated to the @component's stream
 *
 * Return value: a #GooTiAudioManager instance
 */
GooTiAudioManager*
goo_ti_audio_manager_new (GooComponent* component)
{
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (component));

	GooTiAudioManager* self = g_object_new
		(GOO_TYPE_TI_AUDIO_MANAGER,
		 "component", GOO_TI_AUDIO_COMPONENT (component), NULL);

	return self;
}


/**
 * goo_ti_audio_manager_set_component:
 * @self: A #GooTiAudioManager instance
 * @component: A #GooTiAudioComponent instance
 *
 * Associate a #GooTiAudioComponent instance with the audio manager.
 */
void
goo_ti_audio_manager_set_component (GooTiAudioManager* self,
				    GooComponent* component)
{
	g_assert (GOO_IS_TI_AUDIO_MANAGER (self));
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (component));
	g_assert (GOO_OBJECT (self)->owner == NULL);

	GOO_OBJECT_DEBUG (self, "");

	goo_object_set_owner (GOO_OBJECT (self), GOO_OBJECT (component));
	self->cmd->hComponent = GOO_COMPONENT (component)->handle;

	return;
}

/**
 * goo_ti_audio_manager_get_streamid:
 * @self: A #GooTiAudioManager instance
 *
 * Request a stream id to the audio manager to be assigned to the
 * #GooTiAudioComponent associated.
 */
gint
goo_ti_audio_manager_get_stream_id (GooTiAudioManager* self)
{
	g_assert (GOO_IS_TI_AUDIO_MANAGER (self));

	gint ret;

	GOO_OBJECT_DEBUG (self, "");

	self->cmd->param1 = 0;

	GooComponent* component = GOO_COMPONENT
		(goo_object_get_owner (GOO_OBJECT (self)));

	if (GOO_IS_TI_AUDIO_ENCODER (component))
	{
		self->cmd->AM_Cmd = AM_CommandIsInputStreamAvailable;
	}
	else if (GOO_IS_TI_AUDIO_DECODER (component))
	{
		self->cmd->AM_Cmd = AM_CommandIsOutputStreamAvailable;
	}

	g_object_unref (component);

	ret = write (self->fdwrite, self->cmd, sizeof (AM_COMMANDDATATYPE));
	g_assert (ret ==  sizeof (AM_COMMANDDATATYPE));

	ret = read (self->fdread, self->cmd, sizeof (AM_COMMANDDATATYPE));
	g_assert (ret == sizeof (AM_COMMANDDATATYPE));

	GOO_OBJECT_INFO (self, "stream id = %d", self->cmd->streamID);

	g_assert (self->cmd->streamID > 0);
	return self->cmd->streamID;
}

static void
goo_ti_audio_manager_close (GooTiAudioManager* self)
{
	g_assert (GOO_IS_TI_AUDIO_MANAGER (self));
	gint ret;

	GOO_OBJECT_DEBUG (self, "");

	/* no component associated yet */
	GooComponent* component = GOO_COMPONENT
		(goo_object_get_owner (GOO_OBJECT (self)));
	if (component == NULL)
	{
		g_object_unref (component);
		return;
	}

	g_object_unref (component);

	self->cmd->AM_Cmd = AM_Exit;

	ret = write (self->fdwrite, self->cmd, sizeof (AM_COMMANDDATATYPE));
	g_assert (ret == sizeof (AM_COMMANDDATATYPE));

	ret = close (self->fdwrite);
	g_assert (ret == 0);

	ret = close (self->fdread);
	g_assert (ret == 0);

	return;
}

static void
goo_ti_audio_manager_init (GooTiAudioManager* self)
{
	/* open the command write pipe */
	self->fdwrite = open (FIFO1, O_WRONLY);
	g_assert (self->fdwrite > 0);

	/* open the command read pipe */
	self->fdread = open (FIFO2, O_RDONLY);
	g_assert (self->fdread > 0);

	/* allocate the command struct */
	self->cmd = g_new0 (AM_COMMANDDATATYPE, 1);
	g_assert (self->cmd != NULL);

	goo_object_set_name (GOO_OBJECT (self), "audiomanager");

	return;
}

static void
goo_ti_audio_manager_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_AUDIO_MANAGER (object));
	GooTiAudioManager* self = GOO_TI_AUDIO_MANAGER (object);

	goo_ti_audio_manager_close (self);

	if (G_LIKELY (self->cmd != NULL))
	{
		g_free (self->cmd);
		self->cmd = NULL;
	}

	(*G_OBJECT_CLASS (goo_ti_audio_manager_parent_class)->finalize)
		(object);

	return;
}

static void
goo_ti_audio_manager_set_property (GObject* object, guint prop_id,
				   const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_AUDIO_MANAGER (object));
	GooTiAudioManager* self = GOO_TI_AUDIO_MANAGER (object);

	switch (prop_id)
	{
	case PROP_COMPONENT:
	{
		GooComponent* comp = GOO_COMPONENT
			(g_value_get_object (value));
		goo_ti_audio_manager_set_component (self, comp);
		break;
	}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_audio_manager_class_init (GooTiAudioManagerClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->finalize = goo_ti_audio_manager_finalize;
	g_klass->set_property = goo_ti_audio_manager_set_property;

	GParamSpec* spec;
	spec = g_param_spec_object ("component", "Component",
				    "The audio component to associate",
				    GOO_TYPE_TI_AUDIO_COMPONENT,
				    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (g_klass, PROP_COMPONENT, spec);

	return;
}
