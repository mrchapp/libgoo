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
 * SECTION:goo-ti-clock
 * @short_description: This class represents an TI OMX clock.
 *
 * This class represents an TI OMX clock. It is used by some other TI OMX
 * components to synchronize each other.
 *
 * The instance of this class must be unique in all the TI OMX components,
 * So it is managed by the #GooComponentFactory. The client code should not
 * modify or access to this object.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-utils.h>
#include <goo-ti-clock.h>

#define ID "OMX_Clock"
#define NAME "clock"

enum _GooTiClockProp
{
	PROP_0,
	PROP_TIMESTAMP
};

G_DEFINE_TYPE (GooTiClock, goo_ti_clock, GOO_TYPE_COMPONENT);

/**
 * goo_ti_clock_get_timestamp:
 * @self: A #GooTiClock instance
 *
 * Retrieve the current media time ellapsed from the moment of an audio stream
 * started to now.
 *
 * Return value: The number of ticks in microseconds ocurred to now.
 */
gint64
goo_ti_clock_get_timestamp (GooTiClock* self)
{
	OMX_TIME_CONFIG_TIMESTAMPTYPE* param;
	gint64 ts = 0;

	param = g_new0 (OMX_TIME_CONFIG_TIMESTAMPTYPE, 1);
	GOO_INIT_PARAM (param, OMX_TIME_CONFIG_TIMESTAMPTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigTimeCurrentMediaTime,
					   param);

	ts = (gint64) param->nTimestamp;

	g_free (param);

	return ts;
}

/**
 * Reset the start-time of the clock
 */
void
goo_ti_clock_set_starttime (GooTiClock* self, gint64 time_start)
{
	g_assert (GOO_IS_TI_CLOCK (self));
	GOO_OBJECT_INFO (self, "time_start=%lld", time_start);
	OMX_TIME_CONFIG_TIMESTAMPTYPE* param =
		g_new0 (OMX_TIME_CONFIG_TIMESTAMPTYPE, 1);
	GOO_INIT_PARAM (param, OMX_TIME_CONFIG_TIMESTAMPTYPE);

	param->nTimestamp = time_start;

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					OMX_IndexConfigTimeClientStartTime, param);

	g_free (param);
}

static void
goo_ti_clock_set_state_idle (GooComponent* self)
{
        self->next_state = OMX_StateIdle;

        GOO_OBJECT_INFO (self, "Sending idle state command");

	GOO_OBJECT_LOCK (self);
        GOO_RUN (
                OMX_SendCommand (self->handle,
                                 OMX_CommandStateSet,
                                 self->next_state,
                                 NULL)
                );
	GOO_OBJECT_UNLOCK (self);

        goo_component_wait_for_next_state (self);

        GOO_OBJECT_DEBUG (self, "");

        return;
}

static void
goo_ti_clock_set_state_executing (GooComponent* self)
{
        GOO_OBJECT_INFO (self, "Sending executing state command");

        self->next_state = OMX_StateExecuting;

        GOO_OBJECT_LOCK (self);
        GOO_RUN (
                OMX_SendCommand (self->handle,
                                 OMX_CommandStateSet,
                                 self->next_state,
                                 NULL)
                );
	GOO_OBJECT_UNLOCK (self);

        goo_component_wait_for_next_state (self);

        GOO_OBJECT_DEBUG (self, "");

        return;
}

static void
goo_ti_clock_set_state_loaded (GooComponent* self)
{
        GOO_OBJECT_INFO (self, "Sending loaded state command");

        self->next_state = OMX_StateLoaded;

        GOO_OBJECT_LOCK (self);
        GOO_RUN (
                OMX_SendCommand (self->handle,
                                 OMX_CommandStateSet,
                                 self->next_state,
                                 NULL)
                );
	GOO_OBJECT_UNLOCK (self);

        goo_component_wait_for_next_state (self);

        GOO_OBJECT_DEBUG (self, "");

        return;
}

static gboolean
goo_ti_clock_load (GooComponent* self)
{
	GOO_OBJECT_LOCK (self);
        RETURN_GOO_RUN (
                OMX_GetHandle (&self->handle, self->id, self,
                               &self->callbacks)
                );

        GOO_RUN (
                OMX_GetState (self->handle, &self->cur_state)
                );
	GOO_OBJECT_UNLOCK (self);

        GOO_OBJECT_DEBUG (self, "");

        return TRUE;
}

static void
goo_ti_clock_get_property (GObject* object, guint prop_id,
			   GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_CLOCK (object));
	GooTiClock* self = GOO_TI_CLOCK (object);

	switch (prop_id)
	{
	case PROP_TIMESTAMP:
	{
		gint64 ts;
		ts = goo_ti_clock_get_timestamp (self);
		g_value_set_int64 (value, ts);
		break;
	}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_clock_init (GooTiClock* self)
{
        GOO_COMPONENT (self)->id = g_strdup (ID);
        GOO_COMPONENT (self)->port_param_type = OMX_IndexParamOtherInit;

        goo_object_set_name (GOO_OBJECT (self), NAME);

        return;
}

static void
goo_ti_clock_class_init (GooTiClockClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->get_property = goo_ti_clock_get_property;

        GooComponentClass* c_klass = GOO_COMPONENT_CLASS (klass);

        c_klass->load_func = goo_ti_clock_load;

        c_klass->set_state_idle_func = goo_ti_clock_set_state_idle;
        c_klass->set_state_executing_func = goo_ti_clock_set_state_executing;
        c_klass->set_state_loaded_func = goo_ti_clock_set_state_loaded;

	GParamSpec* spec;
	spec = g_param_spec_int64 ("timestamp", "Timestamp",
				   "Current media time",
				   0, G_MAXINT64, 0, G_PARAM_READABLE);
	g_object_class_install_property (g_klass, PROP_TIMESTAMP, spec);

        return;
}
