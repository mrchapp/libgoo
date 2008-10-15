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
 * SECTION:goo-port
 * @short_description: Object which represents an OpenMAX port which is
 *		       contained by components.
 * @see_also: #GooComponent
 *
 * A #GooComponent configures the incoming and outgoing data throught it's
 * "ports", also using these ports the OMX components are linked among them.
 * This also means that the buffers interchange between the OMX thread and
 * the client thread are done by the ports.
 *
 * The #GooPorts are created internally when the component is loaded. You must
 * configure them before set the #GooComponent in idle state.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-port.h>
#include <goo-log.h>
#include <goo-utils.h>

enum _GooPortProp
{
	PROP_0,
	PROP_PADDING,
	PROP_TUNNEL,
	PROP_BUFFERSIZE,
	PROP_BUFFERCOUNT,
	PROP_QUEUED
};

#define BUFFERSIZE_DEFAULT 1024
#define BUFFERCOUNT_DEFAULT 4

G_DEFINE_TYPE (GooPort, goo_port, GOO_TYPE_OBJECT)

static void
goo_port_init (GooPort *self)
{
	self->buffer_header = NULL;
	self->definition = NULL;
	self->buffers_data = NULL;
	self->padding = 0;
	self->use_buffer = FALSE;
	self->tunneled = FALSE;
	self->supplier = FALSE;
	self->eos = FALSE;
	self->enqueue = TRUE;
	self->eos_mutex = g_mutex_new ();
	self->buffer_queue = NULL;
	self->processbuffer_func = NULL;
	self->peer = NULL;

	return;
}

static void
goo_port_dispose (GObject* object)
{
	g_assert (GOO_IS_PORT (object));
	GooPort* self = GOO_PORT (object);

	(*G_OBJECT_CLASS (goo_port_parent_class)->dispose) (object);

	if (G_LIKELY (self->peer != NULL))
	{
		g_object_unref (self->peer);
	}

	return;
}

static void
goo_port_finalize (GObject* object)
{
	g_assert (GOO_IS_PORT (object));
	GooPort* self = GOO_PORT (object);

	GOO_OBJECT_DEBUG (self, "");

	if (G_LIKELY (self->buffer_queue))
	{
		g_async_queue_unref (self->buffer_queue);
		self->buffer_queue = NULL;
	}

	if (G_LIKELY (self->eos_mutex))
	{
		g_mutex_free (self->eos_mutex);
		self->eos_mutex = NULL;
	}

	if (! self->tunneled)
	{
		if (self->use_buffer)
		{
			if (G_LIKELY (self->buffers_data))
			{
				g_free (self->buffers_data);
				self->buffers_data = NULL;
			}
		}

		if (G_LIKELY (self->buffer_header))
		{
			g_free (self->buffer_header);
			self->buffer_header = NULL;
		}
	}

	if (G_LIKELY (self->definition))
	{
		g_free (self->definition);
	}

	(*G_OBJECT_CLASS (goo_port_parent_class)->finalize) (object);

	return;
}

static void
goo_port_set_property (GObject* object, guint prop_id,
		       const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_PORT (object));
	GooPort* self = GOO_PORT (object);

	switch (prop_id)
	{
	case PROP_PADDING:
		self->padding = g_value_get_uint (value);
		break;
	case PROP_TUNNEL:
		self->tunneled = g_value_get_boolean (value);
		break;
	case PROP_BUFFERCOUNT:
		if (self->definition != NULL)
		{
			self->definition->nBufferCountActual =
				g_value_get_uint (value);
		}
		else
		{
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
							   spec);
		}
		break;
	case PROP_BUFFERSIZE:
		if (self->definition != NULL)
		{
			self->definition->nBufferSize =
				g_value_get_uint (value);
		}
		else
		{
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
							   spec);
		}
		break;
	case PROP_QUEUED:
		self->enqueue = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_port_get_property (GObject* object, guint prop_id,
		       GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_PORT (object));
	GooPort* self = GOO_PORT (object);

	switch (prop_id)
	{
	case PROP_PADDING:
		g_value_set_uint (value, self->padding);
		break;
	case PROP_TUNNEL:
		g_value_set_boolean (value, self->tunneled);
		break;
	case PROP_BUFFERSIZE:
		if (self->definition != NULL)
		{
			g_value_set_uint (value,
					  self->definition->nBufferSize);
		}
		else
		{
			g_value_set_uint (value, BUFFERSIZE_DEFAULT);
		}
		break;
	case PROP_BUFFERCOUNT:
		if (self->definition != NULL)
		{
			g_value_set_uint
				(value, self->definition->nBufferCountActual);
		}
		else
		{
			g_value_set_uint (value, BUFFERCOUNT_DEFAULT);
		}
		break;
	case PROP_QUEUED:
		g_value_set_boolean (value, self->enqueue);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_port_class_init (GooPortClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->dispose = goo_port_dispose;
	g_klass->finalize = goo_port_finalize;
	g_klass->set_property = goo_port_set_property;
	g_klass->get_property = goo_port_get_property;

	GParamSpec* spec;
	spec = g_param_spec_uint ("padding", "Padding",
				  "The padding to use in the buffers address",
				  0, 1024, 0, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_PADDING, spec);

	spec = g_param_spec_boolean ("tunneled", "Tunneled",
				     "Specifies if the port is going to operate in tunneled mode",
				     FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_TUNNEL, spec);

	spec = g_param_spec_uint ("buffersize", "Buffer size",
				  "The OMX port buffer size",
				  1, G_MAXUINT32, BUFFERSIZE_DEFAULT,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_BUFFERSIZE, spec);

	spec = g_param_spec_uint ("buffercount", "Buffer count",
				  "The number of OMX port buffers",
				  1, 100, BUFFERCOUNT_DEFAULT,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_BUFFERCOUNT, spec);

	spec = g_param_spec_boolean ("queued", "Buffers queued",
				     "Specifies if the buffers are going to be queued or processed",
				     TRUE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_QUEUED, spec);

	return;
}

GooPort*
goo_port_new ()
{
	GooPort *self = g_object_new (GOO_TYPE_PORT, NULL);
	return self;
}

/**
 * goo_port_prepare_definition:
 * @self: An #GooPort instance
 *
 * Initializes the #OMX_PARAM_PORTDEFINITIONTYPE structure.
 * This method is used internally in the #GooComponentClass.
 **/
void
goo_port_prepare_definition (GooPort *self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (GOO_PORT_GET_DEFINITION (self) == NULL);

	GOO_PORT_GET_DEFINITION (self) = \
		g_new0 (OMX_PARAM_PORTDEFINITIONTYPE, 1);
	g_assert (GOO_PORT_GET_DEFINITION (self) != NULL);

	OMX_PARAM_PORTDEFINITIONTYPE *def = GOO_PORT_GET_DEFINITION (self);
	GOO_INIT_PARAM (def, OMX_PARAM_PORTDEFINITIONTYPE);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_port_allocate_buffer_header:
 * @self: An #GooPort instance
 *
 * Allocate the memory needed by the #OMX_BUFFERHEADERTYPE array.
 * This method is used internally in the #GooComponentClass.
 **/
void
goo_port_allocate_buffer_header (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (GOO_PORT_GET_DEFINITION (self) != NULL);
	g_assert (self->tunneled != TRUE);

	guint numbuf = self->definition->nBufferCountActual;
	g_assert (numbuf > 0);

	self->buffer_header = g_new0 (OMX_BUFFERHEADERTYPE*, numbuf);
	g_assert (self->buffer_header != NULL);

	guint i;
	for (i = 0; i < numbuf; i++)
	{
		self->buffer_header[i] = NULL;
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_port_allocate_buffers:
 * @self: An #GooPort instance
 *
 * Allocates the array of buffers data where the buffers are going to be
 * stored.
 * This method should be called in the specific compoment.
 * This method turns on the USE_BUFFER operation mode.
 **/
void
goo_port_allocate_buffers (GooPort *self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (GOO_PORT_GET_DEFINITION (self) != NULL);
	g_assert (self->tunneled != TRUE);
	g_assert (self->buffers_data == NULL);

	gint numbuf;
	numbuf = GOO_PORT_GET_DEFINITION (self)->nBufferCountActual;
	g_assert (numbuf > 0);

	gint bufsize;
	bufsize = GOO_PORT_GET_DEFINITION (self)->nBufferSize;
	g_assert (bufsize > 0);

	self->buffers_data = (void*) g_malloc0 (sizeof (void*) * numbuf);
	g_assert (self->buffers_data != NULL);

	/** seems to be not required */
#if 0
	gint i;
	for (i = 0; i < num_buffers; i++)
	{
		self->buffers[i] = g_new0 (OMX_BUFFERHEADERTYPE, 1);
		g_assert (GOO_PORT (self)->buffers[i] != NULL);

		self->buffers_data[i] = g_malloc (bufsize + self->padding);
		g_assert (GOO_PORT (self)->buffers_data[i] != NULL);
	}
#endif

	self->use_buffer = TRUE;

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_port_prepare_buffer_queue:
 * @self: An #GooPort instances
 *
 * Initializes the async queue for buffer interchange.
 * This method is used internally in the #GooComponentClass.
 **/
void
goo_port_prepare_buffer_queue (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (GOO_PORT_GET_DEFINITION (self) != NULL);
	g_assert (self->buffer_header != NULL);
	g_assert (self->tunneled != TRUE);

	self->buffer_queue = g_async_queue_new ();
	g_assert (self->buffer_queue != NULL);

	guint numbuf = self->definition->nBufferCountActual;
	g_assert (numbuf > 0);

	guint i = 0;
	for (i = 0; i < numbuf; i++)
	{
		g_async_queue_push (self->buffer_queue,
				    self->buffer_header[i]);
		GOO_OBJECT_INFO (self, "queued buffer[%d] = %x",
				  i, self->buffer_header[i]);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_port_push_buffer:
 * @self: An #GooPort instance
 * @buffer: The #GOO_BUFFERHEADERTYPE pointer to the ready to use buffer.
 *
 * Push the pointer to the async queue in the port.
 * This method is used only by the OMX callbacks in the #GooComponentClass.
 **/
void
goo_port_push_buffer (GooPort* self, OMX_BUFFERHEADERTYPE* buffer)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (self->buffer_queue != NULL);
	g_assert (buffer != NULL);

	GAsyncQueue *queue = g_async_queue_ref (self->buffer_queue);
	g_async_queue_push (queue, buffer);
	g_async_queue_unref (queue);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_port_try_grab_buffer:
 * @self: An #GooPort instance
 *
 * Will try to pop a "ready to use" buffer from the async queue. If the queue
 * is empty, a NULL value will be returned.
 * This function doesn't block the thread.
 *
 * Return value: a #OMX_BUFFERHEADERTYPE pointer if there is already "a ready
 * to use" buffer in the async queue. If there is not one, a NULL value is
 * returned.
 **/
OMX_BUFFERHEADERTYPE*
goo_port_try_grab_buffer (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (self->buffer_queue != NULL);

	OMX_BUFFERHEADERTYPE* buffer = NULL;

	GAsyncQueue *queue = g_async_queue_ref (self->buffer_queue);
	buffer = (OMX_BUFFERHEADERTYPE*) g_async_queue_try_pop (queue);
	g_async_queue_unref (queue);

	GOO_OBJECT_DEBUG (self, "");

	return buffer;
}

/**
 * goo_port_grab_buffer:
 * @self: An #GooPort instance
 *
 * Will pop a "ready to use" buffer from the async queue. If the queue
 * is empty the thread is blocked until a buffer address arrive.
 * This function blocks the thread until a new buffer is grabbed.
 *
 * Return value: a #OMX_BUFFERHEADERTYPE pointer "ready to use"
 **/
OMX_BUFFERHEADERTYPE*
goo_port_grab_buffer (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (self->buffer_queue != NULL);
	/* g_assert (goo_port_is_eos (self) == FALSE); */

	OMX_BUFFERHEADERTYPE* buffer = NULL;

	GAsyncQueue *queue = g_async_queue_ref (self->buffer_queue);
	buffer = (OMX_BUFFERHEADERTYPE*) g_async_queue_pop (queue);
	g_assert (buffer != NULL);
	g_async_queue_unref (queue);

	GOO_OBJECT_DEBUG (self, "");

	return buffer;
}

/**
 * goo_port_reset:
 * @self: A #GooPort instance
 *
 * Reset all the internal values of the port in order to reuse it
 * after a cycle loaded->idle->executing->idle->loaded
 **/
void
goo_port_reset (GooPort *self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (self->eos_mutex != NULL);

	self->tunneled = FALSE;
	self->enqueue = TRUE;

	g_mutex_lock (self->eos_mutex);
	self->eos = FALSE;
	g_mutex_unlock (self->eos_mutex);

	return;
}

/**
 * goo_port_set_eos:
 * @self: An #GooPort instance
 *
 * Tells to the port that the End-Of-Stream have been processed in one
 * of its buffers
 **/
void
goo_port_set_eos (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (self->eos_mutex != NULL);

	if (goo_port_is_eos (self))
	{
		return;
	}

	g_mutex_lock (self->eos_mutex);
	self->eos = TRUE;
	g_mutex_unlock (self->eos_mutex);

	return;
}

/**
 * goo_port_is_eos:
 * @self: An #GooPort instance
 *
 * Notice is an EOS buffer have been processed in the port.
 *
 * Return value: TRUE is an EOS have been processed in the port
 *		 FALSE if no EOS flag have been processed
 **/
gboolean
goo_port_is_eos (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (self->eos_mutex != NULL);

	gboolean retval;
	g_mutex_lock (self->eos_mutex);
	retval = self->eos;
	g_mutex_unlock (self->eos_mutex);

	return retval;
}

/**
 * goo_port_is_tunneled:
 * @self: An #GooPort instance
 *
 * Check if the port is marked as tunneled.
 *
 * Return value: TRUE if the port is tunneled, FALSE if not.
 **/
gboolean
goo_port_is_tunneled (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));

	gboolean retval;
	g_object_get (G_OBJECT (self), "tunneled", &retval, NULL);

	return retval;
}

/**
 * goo_port_is_disabled:
 * @self: An #GooPort instance
 *
 * Check if the port is disabled.
 *
 * Return value: TRUE if the port is disabled, FALSE if not.
 **/
gboolean
goo_port_is_disabled (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));

	gboolean retval;
	retval = (GOO_PORT_GET_DEFINITION (self)->bEnabled == OMX_FALSE);

	return retval;
}
/**
 * goo_port_is_enabled:
 * @self: An #GooPort instance
 *
 * Check if the port is enabled.
 *
 * Return value: TRUE if the port is enabled, FALSE if not.
 **/
gboolean
goo_port_is_enabled (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));

	gboolean retval;
	retval = (GOO_PORT_GET_DEFINITION (self)->bEnabled == OMX_TRUE);

	return retval;
}

/**
 * goo_port_is_queued:
 * @self: a #GooPort instance
 *
 * Check if the buffers are queued.
 *
 * Return value: TRUE if the buffers are queued to its port's async queue;
 *               FALSE if the buffers are going to be processed by a callback
 **/
gboolean
goo_port_is_queued (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));

	gboolean retval;
	retval = (self->enqueue == TRUE);

	return retval;
}

/**
 * goo_port_set_process_buffer_function:
 * @self: a #GooPort instance
 * @pbf: The #GooPortProcessBufferFunction to set
 *
 * Set the given process_buffer function for the port. The process_buffer
 * function is called to process a #OMX_BUFFERHEADERTYPE arrived from the
 * port. See #GooPortProcessBufferFunction for more details.
 **/
void
goo_port_set_process_buffer_function (GooPort* self,
				      GooPortProcessBufferFunction pbf)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (pbf != NULL);
	g_assert (self->processbuffer_func == NULL);

	g_return_if_fail (self->tunneled == FALSE);

	self->processbuffer_func = pbf;

	self->enqueue = FALSE;

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_port_process_buffer:
 * @self: An #GooPort instance
 * @buffer: An #OMX_BUFFERHEADERTYPE pointer
 * @data: A client data to pass to the client thread
 *
 * Callback trigger.
 **/
void
goo_port_process_buffer (GooPort* self, OMX_BUFFERHEADERTYPE* buffer,
			 gpointer data)
{
	g_assert (GOO_IS_PORT (self));

	if (self->enqueue == TRUE)
	{
		GOO_OBJECT_DEBUG (self, "queueing");
		goo_port_push_buffer (self, buffer);
	}
	else if (self->processbuffer_func != NULL)
	{
		GOO_OBJECT_DEBUG (self, "callbacking");
		self->processbuffer_func (self, buffer, data);
	}
	else
	{
		g_assert_not_reached ();
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_port_set_peer:
 * @self: A #GooPort instance
 * @peer: The #GooPort instance to link with
 *
 * Set the port peer in a tunnel
 *
 * Return value: TRUE if can be set of FALSE if not
 */
gboolean
goo_port_set_peer (GooPort* self, GooPort* peer)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (GOO_IS_PORT (peer));

	g_return_val_if_fail (self->tunneled == FALSE, FALSE);
	g_return_val_if_fail (peer->tunneled == FALSE, FALSE);

	g_object_set (self, "tunneled", TRUE, NULL);
	g_object_set (peer, "tunneled", TRUE, NULL);

	self->peer = g_object_ref (peer);
	peer->peer = self;
	g_object_add_weak_pointer (G_OBJECT (self), (gpointer *) &peer->peer);

	g_assert (self->tunneled == TRUE);
	g_assert (peer->tunneled == TRUE);

	return TRUE;
}

/**
 * goo_port_get_peer:
 * @self: An #GooPort instance
 *
 * Get the port peer in a tunnel.
 *
 * Return value: The #GooPort instance tunneled or %NULL.
 * unref peer after usage. MT safe.
 */
GooPort*
goo_port_get_peer (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));
	g_return_val_if_fail (self->tunneled == TRUE, NULL);

	if (G_UNLIKELY (self->peer == NULL))
	{
		return NULL;
	}

	return g_object_ref (self->peer);
}

/**
 * goo_port_is_my_buffer:
 * @self: A #GooPort instance
 * @buffer: An OMX buffer structure
 *
 * Identifies if an OMX buffer belongs to a specific port
 *
 * Return value: TRUE if the buffer belongs to the port; FALSE if otherwise
 */
gboolean
goo_port_is_my_buffer (GooPort* self, OMX_BUFFERHEADERTYPE* buffer)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (buffer != NULL);

	gboolean retval;
	retval = (buffer->pAppPrivate == self);

	return retval;
}

/**
 * goo_port_is_enabled:
 * @self: An #GooPort instance
 *
 * Check if the port uses external buffers.
 *
 * Return value: TRUE if the port is uses external buffers, FALSE if not.
 **/
gboolean
goo_port_is_external_buffer (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));

	gboolean retval;
	retval = (self->use_buffer == OMX_TRUE);

	return retval;
}

/**
 * goo_port_is_supplier:
 * @self: An #GooPort instance
 *
 * Check if the port is a supplier port.
 *
 * Return value: TRUE if the port is the supplier in the tunnel, FALSE if not.
 **/
gboolean
goo_port_is_supplier (GooPort* self)
{
	g_assert (GOO_IS_PORT (self));
	g_assert (goo_port_is_tunneled (self));

	gboolean retval;
	retval = (self->supplier == OMX_TRUE);

	return retval;
}
