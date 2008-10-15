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

#include <goo-ti-audio-encoder.h>
#include <goo-utils.h>

enum _GooTiAudioEncoderProp
{
	PROP_0,
	PROP_NUMBUFFERS
};


G_DEFINE_TYPE (GooTiAudioEncoder, goo_ti_audio_encoder,
	       GOO_TYPE_TI_AUDIO_COMPONENT)

static gboolean
_goo_ti_audio_encoder_set_number_buffers (GooTiAudioEncoder *self,
					  gint number_buffers)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	gboolean retval = TRUE;

	GOO_OBJECT_DEBUG (self, "set numbers in audio encoder class");
	self->number_buffers = number_buffers;

	return retval;

}

static void
goo_ti_audio_encoder_init (GooTiAudioEncoder* self)
{
	/* noop */
	return;
}

static void
goo_ti_audio_encoder_set_property (GObject *object, guint prop_id,
				   const GValue *value, GParamSpec *spec)
{
	g_assert (GOO_IS_TI_AUDIO_ENCODER (object));
	GooTiAudioEncoder *self = GOO_TI_AUDIO_ENCODER (object);

	switch (prop_id)
	{
	case PROP_NUMBUFFERS:
		_goo_ti_audio_encoder_set_number_buffers
			(self, g_value_get_int (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}
	return;
}

static void
goo_ti_audio_encoder_get_property (GObject *object, guint prop_id,
				   GValue *value, GParamSpec *spec)
{
	g_assert (GOO_IS_TI_AUDIO_ENCODER (object));
	GooTiAudioEncoder *self = GOO_TI_AUDIO_ENCODER (object);

	switch (prop_id)
	{
	case PROP_NUMBUFFERS:
		g_value_set_int (value, self->number_buffers);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}
}

/**
 * @FIXME: OMAPS00141818 should be fixed to remove these lines.
 * The audio encoders are reaching a condition of sleep in the OMX component
 * that the component is not able to recover.  To remove these lines in order
 * to use the goo-component method.
 **/
static void
goo_ti_audio_encoder_set_state_idle (GooComponent *component)
{
	g_assert (GOO_IS_TI_AUDIO_ENCODER (component));
	GooTiAudioEncoder *self = GOO_TI_AUDIO_ENCODER (component);
	OMX_STATETYPE tmpstate = component->cur_state;
	component->next_state = OMX_StateIdle;

	GOO_OBJECT_DEBUG (self, "Enter");

	/* asking to the clock be in idle state */
	if (G_LIKELY (component->clock))
	{
		if (component->clock->cur_state != OMX_StateIdle)
		{
			goo_component_set_state_idle (component->clock);
		}
	}

	GOO_OBJECT_INFO (self, "Sending idle state command");

	if (tmpstate == OMX_StateLoaded)
	{
		goo_component_configure (component);
	}


	if (tmpstate == OMX_StateLoaded)
	{
		goo_component_allocate_all_ports (component);
	}
	else if (tmpstate == OMX_StateExecuting)
	{
	}
	else if (tmpstate == OMX_StatePause)
	{
	}

	GOO_OBJECT_LOCK (component);
	GOO_RUN (
		OMX_SendCommand (component->handle,
				 OMX_CommandStateSet,
				 component->next_state,
				 NULL)
		);
	GOO_OBJECT_UNLOCK (component);

	goo_component_wait_for_next_state (component);

	if (tmpstate == OMX_StateExecuting)
	{
		/* unblock all the async_queues */
		goo_component_flush_all_ports (component);
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return;

}

static void
goo_ti_audio_encoder_class_init (GooTiAudioEncoderClass* klass)
{
	GObjectClass *g_klass = G_OBJECT_CLASS (klass);
	GooTiAudioComponentClass *ac_klass =
		GOO_TI_AUDIO_COMPONENT_CLASS (klass);
	GParamSpec *spec;

	g_klass->set_property = goo_ti_audio_encoder_set_property;
	g_klass->get_property = goo_ti_audio_encoder_get_property;

	spec = g_param_spec_int ("number-buffers", "number of buffers",
				 "Number of input buffers",
				 -1, G_MAXINT, -1, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_NUMBUFFERS, spec);

	/** FIXME: OMAPS00141818 should be fixed to remove these lines.
	 *  The audio encoders are reaching a condition of sleep in the OMX component that
	 *  the component is not able to recover.  To remove these lines in order to
	 *  use the goo-component method.**/

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);

	o_klass->set_state_idle_func = goo_ti_audio_encoder_set_state_idle;

	ac_klass->iterate_ports_func = goo_ti_audio_encoder_iterate_ports;

	return;
}

GooIterator*
goo_ti_audio_encoder_iterate_ports (GooTiAudioComponent *self)
{
	GOO_OBJECT_DEBUG (self, "Enter");
	GooIterator *iter;

	GooComponent *component = GOO_COMPONENT (self);

	iter = goo_component_iterate_input_ports (component);
	g_assert (iter != NULL);
	GOO_OBJECT_DEBUG (self, "Exit");

	return iter;
}

/**
 * goo_ti_audio_encoder_set_number_buffers:
 * @self: An #GooTiAudioEncoder instance
 * @volume: An integer value from [-1,GINT_MAX]
 *
 * Set the number of buffers value when the component is in DASF mode
 **/
void
goo_ti_audio_encoder_set_number_buffers (GooTiAudioEncoder *self, gint number_buffers)
{
	g_assert (self != NULL);

	GOO_OBJECT_DEBUG (self, "set number-buffers: %d", number_buffers);
	g_object_set (G_OBJECT (self), "number-buffers", number_buffers, NULL);

	return;
}

/**
 * goo_ti_audio_encoder_get_number_buffers:
 * @self: An #GooTiAudioEncoder instance
 *
 * get the number of buffers value when the component is in DASF mode
 **/
gint
goo_ti_audio_encoder_get_number_buffers (GooTiAudioEncoder *self)
{
	g_assert (self != NULL);
	gint retval;

	g_object_get (G_OBJECT (self), "number-buffers", &retval, NULL);
	GOO_OBJECT_DEBUG (self, " get number-buffers: %d", retval);

	return retval;
}
