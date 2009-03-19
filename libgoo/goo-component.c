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
 * SECTION:goo-component
 * @short_description: Object which represents an OpenMAX component.
 * @see_also: #GooPort
 *
 * The #GooComponent is a higher abstraction representation of an OpenMAX
 * component.
 * Establish the internal state of the component, control the streams and it
 * is responsible of the comunication with the OMX Core.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-component.h>
#include <goo-utils.h>
#include <goo-port-list.h>

#include <string.h>

/* Disable or deallocate port at deinitialization? */
/* #define USE_DISABLE_PORT */
#undef USE_DISABLE_PORT

/* The OR for state validation is useless because when none of valid states
 * are TRUE, the OR returns TRUE, so we need an XOR, which is not supported
 * by C directly
 */
#define XOR(p,q) (((p) || (q)) && !((p) && (q)))

enum _GooComponentProp
{
	PROP_0,
};

G_DEFINE_TYPE (GooComponent, goo_component, GOO_TYPE_OBJECT)

/**
 * _goo_component_command_state_set:
 * @self: A #GooComponent instance
 * @state: The arrived state
 *
 * Controls the state change in the component.
 */
static void
_goo_component_command_state_set (GooComponent* self, OMX_STATETYPE state)
{
	g_assert (self != NULL);

	GOO_OBJECT_INFO (self, "Current %s | Requested %s | Arriving %s",
			 goo_strstate (self->cur_state),
			 goo_strstate (self->next_state),
			 goo_strstate (state));

	if (state == self->next_state ||
	    state == OMX_StateInvalid ||
	    state == OMX_StateWaitForResources)
	{
		self->prev_state = self->cur_state;
		self->cur_state	 = state;
		self->next_state = OMX_StateInvalid;
		GOO_OBJECT_INFO (self, "New state: %s",
				 goo_strstate (self->cur_state));

		goo_semaphore_up (self->state_sem);
	}
	else if (state == self->cur_state)
	{
		GOO_OBJECT_WARNING (self, "Spurious change state event");
	}
	/* for those stupid components which change their state
	 * automatically */
	else if (state != self->next_state && state != self->cur_state)
	{
		GOO_OBJECT_WARNING (self, "Automatic change state event");

		self->prev_state = self->cur_state;
		self->cur_state	 = state;
		self->next_state = OMX_StateInvalid;
		GOO_OBJECT_INFO (self, "New state: %s",
				 goo_strstate (self->cur_state));

		goo_semaphore_up (self->state_sem);
	}

	return;
}

/**
 * _goo_component_get_operation:
 * @self: A #GooComponent instance
 *
 * Tries to deduce which type of component is.
 *
 * Return value: a #GooCompontentOp value
 */
static GooComponentOp
_goo_component_get_operation (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);

	guint inports = 0;
	{
		GooIterator* iter = goo_component_iterate_input_ports (self);
		while (!goo_iterator_is_done (iter))
		{
			GooPort* port = GOO_PORT (
				goo_iterator_get_current (iter)
				);

			if (goo_port_is_enabled (port))
			{
				if (goo_port_is_tunneled (port))
				{
					GooPort* peer = NULL;
					peer = goo_port_get_peer (port);
					if (peer != NULL)
					{
						inports++;
						g_object_unref (peer);
					}
				}
				else
				{
					inports++;
				}
			}

			g_object_unref (port);
			goo_iterator_next (iter);
		}
		g_object_unref (iter);
	}

	guint outports = 0;
	{
		GooIterator* iter = goo_component_iterate_output_ports (self);
		while (!goo_iterator_is_done (iter))
		{
			GooPort* port = GOO_PORT (
				goo_iterator_get_current (iter)
				);

			if (goo_port_is_enabled (port))
			{
				if (goo_port_is_tunneled (port))
				{
					GooPort* peer = NULL;
					peer = goo_port_get_peer (port);
					if (peer != NULL)
					{
						outports++;
						g_object_unref (peer);
					}
				}
				else
				{
					outports++;
				}
			}

			g_object_unref (port);
			goo_iterator_next (iter);
		}
		g_object_unref (iter);
	}

	if (inports == 0 && outports > 0)
	{
		GOO_OBJECT_INFO (self, "Component is SRC");
		return GOO_COMPONENT_IS_SRC;
	}
	if (inports > 0 && outports == 0)
	{
		GOO_OBJECT_INFO (self, "Component is SINK");
		return GOO_COMPONENT_IS_SINK;
	}
	else if (inports > 0 && outports > 0)
	{
		GOO_OBJECT_INFO (self, "Component is FILTER");
		return GOO_COMPONENT_IS_FILTER;
	}

	return GOO_COMPONENT_IS_UNKNOWN;
}

/**
 * goo_component_eos_buffer_flag:
 * @self: A #GooComponent instance
 * @portindex: The port index in the component where the EOS flag where raised
 *
 * Manages the EOS flag event raised in a specific port.
 */
static void
goo_component_eos_buffer_flag (GooComponent* self, guint portindex)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (portindex >= 0);

	GOO_OBJECT_INFO (self, "EOS flag found in port %d", portindex);

	GooPort* port = NULL;
	OMX_PARAM_PORTDEFINITIONTYPE* param = NULL;

	GooIterator* iter = goo_component_iterate_ports (self);
	goo_iterator_nth (iter, portindex);
	port = GOO_PORT (goo_iterator_get_current (iter));
	g_assert (port != NULL);

	goo_port_set_eos (port);

	param = GOO_PORT_GET_DEFINITION (port);

	if (param->eDir == OMX_DirInput)
	{
		if (_goo_component_get_operation (self) ==
		    GOO_COMPONENT_IS_SINK)
		{
			goo_component_set_done (self);
		}
	}
	else if (param->eDir == OMX_DirOutput)
	{
		if (goo_port_is_tunneled (port))
		{
			goo_component_set_done (self);
		}
	}

	g_object_unref (G_OBJECT (port));
	g_object_unref (G_OBJECT (iter));

	return;
}

/**
 * goo_component_event_handler:
 *
 * OpenMAX callback
 **/
static OMX_ERRORTYPE
goo_component_event_handler (OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
			     OMX_EVENTTYPE eEvent, OMX_U32 nData1,
			     OMX_U32 nData2, OMX_PTR pEventData)
{
	GooComponent* self = GOO_COMPONENT (g_object_ref (pAppData));

	switch (eEvent)
	{
	case OMX_EventCmdComplete:
	{
		OMX_COMMANDTYPE cmd = (OMX_COMMANDTYPE) nData1;
		switch (cmd)
		{
		case OMX_CommandStateSet:
		{
			OMX_STATETYPE state = (OMX_STATETYPE) nData2;
			_goo_component_command_state_set (self, state);
			break;
		}
		case OMX_CommandPortDisable:
			GOO_OBJECT_INFO (self, "Port #%d disabled", nData2);
			goo_semaphore_up (self->port_sem);
			break;
		case OMX_CommandPortEnable:
			GOO_OBJECT_INFO (self, "Port #%d enabled", nData2);
			goo_semaphore_up (self->port_sem);
			break;
		case OMX_CommandFlush:
			GOO_OBJECT_INFO (self, "Port #%d flushed", nData2);
			goo_semaphore_up (self->port_sem);
			break;
		default:
			GOO_OBJECT_INFO (self,
					 "EventCmdComplete - command: %s",
					 goo_strcommand (cmd));
		}
		break;
	}

	case OMX_EventError:
	{
		OMX_ERRORTYPE err = (OMX_ERRORTYPE) nData1;
		switch (err)
		{
		case OMX_ErrorNone:
			break;
		case OMX_ErrorPortUnpopulated:
			/** @todo: some components send this error.
			 * This is generaly produced because we don't disable
			 * the port before change to load state. */
			GOO_OBJECT_WARNING (self, "%s - %s",
					    goo_strerror (err),
					    GOO_STR_NULL (pEventData));
			break;
		default:
			GOO_OBJECT_ERROR (self, "Error: %s - %s",
					  goo_strerror (err),
					  GOO_STR_NULL (pEventData));
			break;

		}
		break;
	}

	case OMX_EventBufferFlag:
	{
		GOO_OBJECT_INFO (self,
				 "Buffer flag %x in port %d", nData2, nData1);

		if ((nData2 & OMX_BUFFERFLAG_EOS) == 0x1)
		{
			GooComponentClass* klass =
				GOO_COMPONENT_GET_CLASS (self);
			if (klass->eos_flag_func != NULL)
			{
				(klass->eos_flag_func) (self, (guint) nData1);
			}
		}
		break;
	}

	default:
		GOO_OBJECT_INFO (self, "%s", goo_strevent (eEvent));
	}

	g_object_unref (G_OBJECT (self));

	return OMX_ErrorNone;
}

/**
 * goo_component_empty_buffer_done:
 *
 * OpenMAX callback
 **/
static OMX_ERRORTYPE
goo_component_empty_buffer_done (OMX_HANDLETYPE hComponent,
				 OMX_PTR pAppData,
				 OMX_BUFFERHEADERTYPE* pBuffer)
{
	g_assert (GOO_IS_COMPONENT (pAppData));

	GooComponent* self = g_object_ref (GOO_COMPONENT (pAppData));

	GooPort* port = NULL;

	if (GOO_IS_PORT (pBuffer->pAppPrivate))
	{
		port = g_object_ref (GOO_PORT(pBuffer->pAppPrivate));
	}
	else /* last resort */
	{
		GooIterator* iter = goo_component_iterate_ports (self);
		goo_iterator_nth (iter, pBuffer->nInputPortIndex);
		port = GOO_PORT (goo_iterator_get_current (iter));
		g_object_unref (iter);
		g_object_unref (self);
	}

	if (G_LIKELY (port != NULL))
	{
		goo_port_process_buffer (port, pBuffer, self);
		GOO_OBJECT_NOTICE (port, "new buffer pointer 0x%x", pBuffer);
		g_object_unref (port);
	}

	g_object_unref (self);

	return OMX_ErrorNone;
}

/**
 * goo_component_fill_buffer_done:
 *
 * OpenMAX callback
 **/
static OMX_ERRORTYPE
goo_component_fill_buffer_done (OMX_HANDLETYPE hComponent,
				OMX_PTR pAppData,
				OMX_BUFFERHEADERTYPE* pBuffer)
{
	g_assert (GOO_IS_COMPONENT (pAppData));

	GooComponent* self = g_object_ref (GOO_COMPONENT (pAppData));

	GooPort* port = NULL;

	if (GOO_IS_PORT (pBuffer->pAppPrivate))
	{
		port = g_object_ref (GOO_PORT(pBuffer->pAppPrivate));
	}
	else if (GOO_IS_COMPONENT (pAppData)) /* last resort */
	{
		GooIterator* iter = goo_component_iterate_ports (self);
		goo_iterator_nth (iter, pBuffer->nOutputPortIndex);
		port = GOO_PORT (goo_iterator_get_current (iter));
		g_object_unref (iter);
	}

	if (G_LIKELY (port != NULL))
	{
		goo_port_process_buffer (port, pBuffer, self);
		GOO_OBJECT_NOTICE (port, "new buffer pointer 0x%x", pBuffer);
		g_object_unref (port);
	}

	g_object_unref (self);

	return OMX_ErrorNone;
}

/* this function is because the tunnel and the idle state must
 * configure the port before doing their process, but NOT must be
 * executed twice */
void
goo_component_configure (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);

	if (!self->configured)
	{
		goo_component_validate_ports_definition (self);
		goo_component_set_parameters (self);
		goo_component_configure_all_ports (self);
		self->configured = TRUE;
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_get_peer_component:
 * @self: A #GooComponent instance
 * @port: A #GooPort instance
 *
 * Gets the GooComponent of the peer element. Must be unrefed
 * after usage.
 *
 * return value: NULL or a GooComponent instance. The return value must be
 * unrefed after used.
 */
GooComponent*
goo_component_get_peer_component (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));
	g_assert (goo_component_is_my_port (self, port));

	GooPort *peer_port = NULL;
	peer_port = goo_port_get_peer (port);

	if (peer_port == NULL)
	{
		GOO_OBJECT_INFO (port, "Port doesn't have a peer port");
		return NULL;
	}

	GooComponent *peer_component = NULL;
	peer_component = GOO_COMPONENT (
		goo_object_get_owner (GOO_OBJECT (peer_port))
		);

	/* not need it anymore */
	g_object_unref (peer_port);

	return peer_component;
}

/**
 * goo_component_set_supplier_port:
 * @self: A #GooComponent instance
 * @port: A #GooPort instance
 * @index: A #OMX_BUFFERSUPPLIERTYPE enumeration
 *
 *
 * This function sets a port as a supplier or
 * non supplier.
 */
void
goo_component_set_supplier_port (GooComponent *self, GooPort *port,
				 OMX_BUFFERSUPPLIERTYPE stype)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));
	g_assert (goo_component_is_my_port (self, port));

	OMX_PARAM_BUFFERSUPPLIERTYPE *param = NULL;

	param = g_new0 (OMX_PARAM_BUFFERSUPPLIERTYPE, 1);

	param->nSize = sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE);
	param->nPortIndex = GOO_PORT_GET_DEFINITION(port)->nPortIndex;
	param->eBufferSupplier = stype;

	goo_component_set_parameter_by_index
		(self, OMX_IndexParamCompBufferSupplier, param);

	if (GOO_PORT_GET_DEFINITION (port)->eDir == OMX_DirInput &&
	    stype == OMX_BufferSupplyInput)
	{
		port->supplier = TRUE;
	}

	if (GOO_PORT_GET_DEFINITION (port)->eDir == OMX_DirOutput &&
	    stype == OMX_BufferSupplyOutput)
	{
		port->supplier = TRUE;
	}

	g_free (param);

	GOO_OBJECT_DEBUG (port, "Updating the port defintion");
	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_GetParameter (self->handle, OMX_IndexParamPortDefinition,
				  GOO_PORT_GET_DEFINITION (port))
		);
	GOO_OBJECT_UNLOCK (self);

	return;
}

static gboolean
goo_component_set_state (GooComponent *self, OMX_STATETYPE state)
{
	gboolean retval = TRUE;

	switch (state)
	{
		case OMX_StateLoaded:
			goo_component_set_state_loaded (self);
			break;
		case OMX_StateExecuting:
			goo_component_set_state_executing (self);
			break;
		case OMX_StateIdle:
			goo_component_set_state_idle (self);
			break;
		default:
			retval = FALSE;
			break;
	}

	return retval;
}

/*
 * This function handles the required tunnel deinitialization in the OMX spec
 * section 3.4.3.1.
 *
 * Waits for the supplier component to change state before waiting for the
 * non-supplier.
 *
 * If no ports are tunneled then it just waits for the next state.
 */
void
goo_component_propagate_wait_for_next_state_default (GooComponent * self)
{
	gboolean tunneled_inports = FALSE;
	gboolean supplier_outports = FALSE;

	GooIterator* iter = goo_component_iterate_output_ports (self);

	/* Check for supplier peer tunneled outports.
	 * Wait for their change state if they exist. */
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (
			goo_iterator_get_current (iter)
			);

		if (goo_port_is_tunneled (port) && goo_port_is_enabled (port))
		{
			GooComponent* peer_component;
			peer_component = goo_component_get_peer_component
				(self, port);
			if (peer_component == NULL)
			{
				GOO_OBJECT_INFO (self, "DASF/unplug port");
				g_object_unref (port);
				goo_iterator_next (iter);
				continue;
			}

			GooPort* peer_port;
			peer_port = goo_port_get_peer (port);

			/* According to spec 1.1 we must wait on the state
			 * change for the supplier port before the non-supplier
			 * ports */
			if (peer_port->supplier == TRUE)
			{
				GOO_OBJECT_INFO (self,
						 "Peer component is the "
						 "supplier port. Wating for "
						 "its change state...");

				goo_component_wait_for_next_state
					(peer_component);
			}

			g_object_unref (peer_component);
			g_object_unref (peer_port);
		}
		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	/* If you have a tunneled input port then the peer component should
	 * wait for your state change.
	 * @TODO: In GStreamer the graph goes from left to right so the peer
	 * element will wait for the state change. This could be different in
	 * other situations. */
	iter = goo_component_iterate_input_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		if (goo_port_is_tunneled (port) && goo_port_is_enabled (port))
		{
			GOO_OBJECT_DEBUG (self, "Found a tunneled input port");
			tunneled_inports = TRUE;
		}

		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	/* Check for your own supplier tunneled outports. If we don't have
	 * input tunneled ports then wait for our state change and then wait
	 * for the peer. */
	iter = goo_component_iterate_output_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (
			goo_iterator_get_current (iter)
			);

		if (goo_port_is_tunneled (port) && goo_port_is_enabled (port))
		{
			GooComponent *peer_component;
			peer_component = goo_component_get_peer_component
				(self, port);
			if (peer_component == NULL)
			{
				GOO_OBJECT_INFO (self,
						 "DASF/unplug tunneled port");
				g_object_unref (port);
				goo_iterator_next (iter);
				continue;
			}

			/* According to spec 1.1 we must wait on the state
			 * change for the supplier port before the
			 * non-supplier ports */
			if (port->supplier == TRUE)
			{
				if (tunneled_inports == FALSE)
				{
					GOO_OBJECT_INFO (self,
							 "Supplier port. "
							 "Waiting for our "
							 "state change and "
							 "then the peer port "
							 "change");

					goo_component_wait_for_next_state (self);
					goo_component_wait_for_next_state
						(peer_component);
					supplier_outports = TRUE;
				}
			}
			g_object_unref (peer_component);
		}
		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);


	/* If no tunneled inports then just wait for the next state */
	if (tunneled_inports == FALSE && supplier_outports == FALSE)
	{
		GOO_OBJECT_INFO (self, "Wating for state change");
		goo_component_wait_for_next_state (self);
	}
	return;
}

/**
 * goo_component_propagate_state_default:
 * @self: A #GooComponent instance
 * @state: The next #OMX_STATETYPE of @self
 *
 * This function changes the state of other components if they are connected
 * in a tunnel with @self.
 *
 * If this component is A and the peer component is B then:
 *	If A is the supplier:
 *	   A -> change state then B -> change state.
 *
 *	If B is the supplier:
 *	   B -> change state then A -> change state.
 *
 * Consult OMX spec 1.1 section 3.4.3.1.
 *
 * Return value: TRUE if the component should propagate the change state
 *		 command
 *		 FALSE if it has already been changed by a peer component.
 **/
static gboolean
goo_component_propagate_state_default (GooComponent* self, OMX_STATETYPE state)
{
	gboolean state_changed = FALSE;

	/* First we need to find the supplier peer ports and change their
	 * state before ours. */
	GOO_OBJECT_DEBUG (self, "Checking for peer supplier ports");

	GooIterator* iter = goo_component_iterate_output_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		if (goo_port_is_tunneled (port) && goo_port_is_enabled (port))
		{
			GOO_OBJECT_DEBUG (self, "Found a tunneled output port");

			GooComponent *peer_component = NULL;
			peer_component = goo_component_get_peer_component
				(self, port);

			if (peer_component == NULL)
			{
				GOO_OBJECT_INFO (self,
						 "DASF/unplug tunneled port");
				g_object_unref (port);
				goo_iterator_next (iter);
				continue;
			}

			GooPort *peer_port = NULL;
			peer_port = goo_port_get_peer (port);
			if (peer_port->supplier == TRUE)
			{
				GOO_OBJECT_INFO (self,
						 "Peer component is the "
						 "supplier port. Changing "
						 "state");
				goo_component_set_state (peer_component, state);
			}

			g_object_unref (peer_component);
			g_object_unref (peer_port);
		}
		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	/* Then we need to find all the ports we supply and change our state
	 * before the peer element */
	GOO_OBJECT_DEBUG (self, "Checking for own supplier ports");

	iter = goo_component_iterate_output_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		if (goo_port_is_tunneled (port) && goo_port_is_enabled (port))
		{
			GOO_OBJECT_DEBUG (self, "Found a tunneled output port");

			GooComponent *peer_component;
			peer_component = goo_component_get_peer_component
				(self, port);
			if (peer_component == NULL)
			{
				GOO_OBJECT_INFO (self,
						 "DASF/unplug tunneled port");
				g_object_unref (port);
				goo_iterator_next (iter);
				continue;
			}

			GooPort *peer_port;
			peer_port = goo_port_get_peer (port);
			if (port->supplier == TRUE)
			{
				GOO_OBJECT_INFO (self,
						 "Peer component is the "
						 "supplier port. Changing "
						 "state");

				GOO_OBJECT_LOCK (self);
				GOO_RUN (
					OMX_SendCommand (self->handle,
							 OMX_CommandStateSet,
							 self->next_state,
							 NULL)
					);
				GOO_OBJECT_UNLOCK (self);

				goo_component_set_state (peer_component, state);

				state_changed = TRUE;
			}
			g_object_unref (peer_component);
			g_object_unref (peer_port);
		}
		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	return state_changed;
}

static void
goo_component_set_state_idle_default (GooComponent* self)
{
	OMX_STATETYPE tmpstate = self->cur_state;
	self->next_state = OMX_StateIdle;

	gint opnum = 0;
	gboolean *prev_port_enqueue = NULL;

	if (tmpstate == OMX_StateLoaded)
	{
		goo_component_configure (self);
	}
	else if ((tmpstate == OMX_StateExecuting) ||
		 (tmpstate == OMX_StatePause))
	{
		gint i = 0;
		opnum = goo_list_get_length (self->output_ports);
		if (opnum > 0)
		{
			prev_port_enqueue = g_new0 (gboolean, opnum);

			GooPort* port = NULL;
			GooIterator* iter =
				goo_component_iterate_output_ports (self);
			while (!goo_iterator_is_done (iter))
			{
				port = GOO_PORT (
					goo_iterator_get_current (iter)
					);
				GOO_OBJECT_DEBUG (port, "queueing");
				prev_port_enqueue[i++] =
					goo_port_is_queued (port);
				g_object_set (port, "queued", TRUE, NULL);
				g_object_unref (port);
				goo_iterator_next (iter);
			}
			g_object_unref (iter);
		}
	}
	else
	{
		/* this state change is not valid */
		g_assert (FALSE);
	}

	/* When we propage states we might have already changed our state */
	if (!goo_component_propagate_state (self, OMX_StateIdle))
	{
		GOO_OBJECT_INFO (self, "Sending idle state command");

		GOO_OBJECT_LOCK (self);
		GOO_RUN (
			OMX_SendCommand (self->handle,
					 OMX_CommandStateSet,
					 self->next_state,
					 NULL)
			);
		GOO_OBJECT_UNLOCK (self);
	}

	if (tmpstate == OMX_StateLoaded)
	{
		goo_component_allocate_all_ports (self);
	}
	else if (tmpstate == OMX_StateExecuting)
	{
	}
	else if (tmpstate == OMX_StatePause)
	{
	}

	/* If the component doesn't have and outport tunneled then it
	 * will only wait for the next state */
	goo_component_propagate_wait_for_next_state (self);

	if (tmpstate == OMX_StateExecuting)
	{
		/* unblock all the async_queues */
		goo_component_flush_all_ports (self);

		if (opnum > 0)
		{
			gint i = 0;
			GooPort* port = NULL;
			GooIterator* iter =
				goo_component_iterate_output_ports (self);
			while (!goo_iterator_is_done (iter))
			{
				port = GOO_PORT (
					goo_iterator_get_current (iter)
					);
				g_object_set (port, "queued",
					      prev_port_enqueue[i++], NULL);
				g_object_unref (port);
				goo_iterator_next (iter);

			}
			g_object_unref (iter);
		}
	}

	if (prev_port_enqueue != NULL)
	{
		g_free (prev_port_enqueue);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_component_set_state_executing_default (GooComponent* self)
{
	self->next_state = OMX_StateExecuting;

	/* When we propage states we might have already changed our state */
	if (!goo_component_propagate_state (self, OMX_StateExecuting))
	{
		GOO_OBJECT_INFO (self, "Sending executing state command");

		GOO_OBJECT_LOCK (self);
		GOO_RUN (
			OMX_SendCommand (self->handle,
					 OMX_CommandStateSet,
					 self->next_state,
					 NULL)
			);
		GOO_OBJECT_UNLOCK (self);
	}

	goo_component_wait_for_next_state (self);

	if (self->prev_state == OMX_StateIdle)
	{
		goo_component_prepare_all_ports (self);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_component_set_state_loaded_default (GooComponent* self)
{
	self->next_state = OMX_StateLoaded;

#if USE_DISABLE_PORT /* DR!: not supported by video decoder */
	if (self->cur_state == OMX_StateIdle)
	{
		goo_component_disable_all_ports (self);
	}
#endif

	/* When we propage states we might have already changed our state */
	if (!goo_component_propagate_state (self, OMX_StateLoaded))
	{
		GOO_OBJECT_INFO (self, "Sending loaded state command");

		GOO_OBJECT_LOCK (self);
		GOO_RUN (
			OMX_SendCommand (self->handle,
					 OMX_CommandStateSet,
					 self->next_state,
					 NULL)
			);
		GOO_OBJECT_UNLOCK (self);
	}

#if !USE_DISABLE_PORT
	if (self->cur_state == OMX_StateIdle &&
	    self->next_state == OMX_StateLoaded)
	{
		goo_component_deallocate_all_ports (self);
	}
#endif

	/* If the component doesn't have and outport tunneled then it
	 * will only wait for the next state */
	goo_component_propagate_wait_for_next_state (self);

	self->configured = FALSE;

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_component_set_state_pause_default (GooComponent* self)
{
	self->next_state = OMX_StatePause;

	GOO_OBJECT_INFO (self, "Sending pause state command");

	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SendCommand (self->handle,
				 OMX_CommandStateSet,
				 self->next_state,
				 NULL)
		);
	GOO_OBJECT_UNLOCK (self);

	goo_component_wait_for_next_state (self);

	return;
}

static gboolean
goo_component_load_default (GooComponent* self)
{
	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_GetHandle (&self->handle,
			       self->id, self, &self->callbacks)
		);

	GOO_RUN (
		OMX_GetState (self->handle, &self->cur_state)
		);
	GOO_OBJECT_UNLOCK (self);

	/** @todo this is just an idea */
	{
		goo_component_load_ports (self);
		goo_component_load_parameters (self);
		goo_component_set_ports_definition (self);
	}

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

static void
goo_component_release_buffer_default (GooComponent* self,
				      OMX_BUFFERHEADERTYPE* buffer)
{
	g_assert (GOO_IS_COMPONENT (self));

	g_return_if_fail (buffer != NULL);
	g_return_if_fail (GOO_IS_PORT (buffer->pAppPrivate));

	g_return_if_fail (self->cur_state != OMX_StateLoaded);
	g_return_if_fail (self->cur_state != OMX_StateInvalid);
	g_return_if_fail (self->cur_state != OMX_StateWaitForResources);

	GooPort* port = g_object_ref (GOO_PORT (buffer->pAppPrivate));

	if (goo_port_is_eos (port) || goo_port_is_tunneled (port))
	{
		GOO_OBJECT_WARNING (port, "Highly unusual buffer!");
		goto beach;
	}

	if ((buffer->nFlags & OMX_BUFFERFLAG_EOS) == 0x1)
	{
		GOO_OBJECT_INFO (port, "eos found!");
		goo_port_set_eos (port);
	}

	if (self->cur_state == OMX_StateIdle)
	{
		GOO_OBJECT_DEBUG (port, "Pushing buffer to queue");
		goo_port_push_buffer (port, buffer);
		goto beach;
	}

	if (GOO_PORT_GET_DEFINITION (port)->eDir == OMX_DirInput)
	{

		/* We should always push the EOS buffer */
		GOO_OBJECT_NOTICE (self, "OMX_EmptyThisBuffer, 0x%x", buffer);

		GOO_OBJECT_LOCK (self);
		GOO_RUN (
			OMX_EmptyThisBuffer (self->handle, buffer)
			);
		GOO_OBJECT_UNLOCK (self);

	}
	else if (GOO_PORT_GET_DEFINITION (port)->eDir == OMX_DirOutput)
	{
		/* we shouldn't push another buffer when the EOS is found */
		if (!goo_port_is_eos (port))
		{
			GOO_OBJECT_NOTICE (self,
					   "OMX_FillThisBuffer, 0x%x", buffer);

			GOO_OBJECT_LOCK (self);
			GOO_RUN (
				OMX_FillThisBuffer (self->handle, buffer)
				);
			GOO_OBJECT_UNLOCK (self);
		}
	}

beach:
	g_object_unref (port);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_component_flush_port_default (GooComponent* self, GooPort* port)
{
	OMX_PARAM_PORTDEFINITIONTYPE* param = NULL;
	param = GOO_PORT_GET_DEFINITION (port);

	gboolean prev_port_enqueue = FALSE;
	gboolean queue_buffers = (param->eDir == OMX_DirOutput &&
				  !goo_port_is_tunneled (port) &&
				  !goo_port_is_disabled (port));

	if (queue_buffers == TRUE)
	{
		prev_port_enqueue = goo_port_is_queued (port);
		g_object_set (port, "queued", TRUE, NULL);
	}

	guint index = param->nPortIndex;

	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SendCommand (self->handle, OMX_CommandFlush, index, 0)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (port, "Waiting for port flushing event...");
	goo_semaphore_down (self->port_sem, FALSE);

	if (queue_buffers == TRUE)
	{
		g_object_set (port, "queued", prev_port_enqueue, NULL);
	}

	if (!goo_port_is_tunneled (port) && !goo_port_is_disabled (port))
	{
		if (port->buffer_queue)
		{
			gint buffer_queued = g_async_queue_length (port->buffer_queue);
			GOO_OBJECT_INFO (port, "%d buffers queued", buffer_queued);

#if 0 /* @todo: check this assertion */
			/* this thread is blocking so we substract 1 */
			g_assert (buffer_count - 1 == buffer_queued);
#endif
		}
		else
		{
			GOO_OBJECT_WARNING (port, "warning: tunneled port, but no buffer_queue??");
		}
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

gboolean
goo_component_unload_default (GooComponent* self)
{
	GOO_OBJECT_DEBUG (self, "");

	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_FreeHandle (self->handle)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

static void
goo_component_init (GooComponent* self)
{
	self->ports = goo_port_list_new ();
	g_assert (self->ports != NULL);

	self->input_ports = goo_port_list_new ();
	g_assert (self->input_ports != NULL);

	self->output_ports = goo_port_list_new ();
	g_assert (self->output_ports != NULL);

	self->handle = NULL;

	self->cur_state =
		self->prev_state =
		self->next_state = OMX_StateInvalid;

	self->callbacks.EventHandler = goo_component_event_handler;
	self->callbacks.EmptyBufferDone = goo_component_empty_buffer_done;
	self->callbacks.FillBufferDone = goo_component_fill_buffer_done;

	self->clock = NULL;

	self->state_sem = goo_semaphore_new (0);
	g_assert (self->state_sem != NULL);

	self->port_sem = goo_semaphore_new (0);

	self->done = FALSE;

	self->done_sem = goo_semaphore_new (0);
	g_assert (self->done_sem != NULL);

	self->configured = FALSE;

	self->id = NULL;
	self->port_param_type = OMX_IndexComponentStartUnused;

#ifdef OUTPUT_QUEUE
	self->output_thread = NULL;
#endif

	return;
}

static void
goo_component_dispose (GObject* object)
{
	GooComponent* self = GOO_COMPONENT (object);
	g_assert (GOO_IS_COMPONENT (self));

	g_assert (self->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "");

	(*G_OBJECT_CLASS (goo_component_parent_class)->dispose) (object);

	if (G_LIKELY (self->clock))
	{
		g_object_unref (self->clock);
	}

	if (G_LIKELY (self->ports))
	{
		g_object_unref (self->ports);
	}

	if (G_LIKELY (self->input_ports))
	{
		g_object_unref (self->input_ports);
	}

	if (G_LIKELY (self->output_ports))
	{
		g_object_unref (self->output_ports);
	}

	if (G_LIKELY (self->handle))
	{
		goo_component_unload (self);
		self->handle = NULL;
	}

	return;
}

static void
goo_component_finalize (GObject* object)
{
	g_assert (GOO_IS_COMPONENT (object));
	GooComponent* self = GOO_COMPONENT (object);
	g_assert (self->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "");

	if (G_LIKELY (self->state_sem))
	{
		goo_semaphore_free (self->state_sem);
		self->state_sem = NULL;
	}

	if (G_LIKELY (self->port_sem))
	{
		goo_semaphore_free (self->port_sem);
		self->port_sem = NULL;
	}

	if (G_LIKELY (self->done_sem))
	{
		goo_semaphore_free (self->done_sem);
		self->done_sem = NULL;
	}

	if (G_LIKELY (self->id))
	{
		g_free (self->id);
		self->id = NULL;
	}

	(*G_OBJECT_CLASS (goo_component_parent_class)->finalize) (object);

	return;
}

static void
goo_component_class_init (GooComponentClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->dispose = goo_component_dispose;
	g_klass->finalize = goo_component_finalize;

	klass->load_parameters_func = NULL;
	klass->set_parameters_func = NULL;

	klass->set_ports_definition_func = NULL;
	klass->validate_ports_definition_func = NULL;

	klass->set_clock_func = NULL;

	klass->load_func = goo_component_load_default;
	klass->unload_func = goo_component_unload_default;

	klass->set_state_idle_func =
		goo_component_set_state_idle_default;
	klass->set_state_executing_func =
		goo_component_set_state_executing_default;
	klass->set_state_loaded_func =
		goo_component_set_state_loaded_default;
	klass->set_state_pause_func =
		goo_component_set_state_pause_default;
	klass->propagate_wait_for_next_state_func =
		goo_component_propagate_wait_for_next_state_default;
	klass->propagate_state_func =
		goo_component_propagate_state_default;

	klass->eos_flag_func = goo_component_eos_buffer_flag;
	klass->release_buffer_func = goo_component_release_buffer_default;
	klass->flush_port_func = goo_component_flush_port_default;

	return;
}

/**
 * goo_component_wait_for_next_state:
 * @self: An #GooComponent instance
 *
 * This method will block the thread until the OMX component have changed his
 * state to the new one specified previously.
 **/
void
goo_component_wait_for_next_state (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	while (OMX_StateInvalid != self->next_state)
	{
		goo_semaphore_down (self->state_sem, TRUE);
		if (OMX_StateInvalid != self->next_state)
		{
			GOO_OBJECT_WARNING (self, "Still waiting for %s",
					    goo_strstate (self->next_state));
		}
	}

	GOO_OBJECT_DEBUG (self, "");

	return;

}

/**
 * goo_component_get_state:
 * @self: A #GooComponent instance
 *
 * This method executes the OMX_GetState macro to obtain the current
 * component's state.
 *
 * Return value: The #OMX_STATETYPE current value
 **/
OMX_STATETYPE goo_component_get_state (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	OMX_STATETYPE state;

	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_GetState (self->handle, &state)
		);
	GOO_OBJECT_UNLOCK (self);

	g_assert (state == self->cur_state);

	return state;
}

/**
 * goo_component_load_ports:
 * @self: An #GooComponent instance
  *
 * This function will load all the ports needed by the component.
 *
 * Return value: TRUE if the port were loaded correctly; FALSE otherwise
 **/
gboolean
goo_component_load_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);
	g_assert (goo_list_get_length (self->ports) == 0);
	g_assert (self->port_param_type != OMX_IndexComponentStartUnused);

	OMX_PORT_PARAM_TYPE *port_param = g_new0 (OMX_PORT_PARAM_TYPE, 1);
	GOO_INIT_PARAM (port_param, OMX_PORT_PARAM_TYPE);

	/** @todo memory leak if fail */
	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_GetParameter (self->handle,
				  self->port_param_type, port_param)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_INFO (self, "nStartPortNumber = %d, nPorts = %d",
			 port_param->nStartPortNumber, port_param->nPorts);
	int i = 0;
	for (i = port_param->nStartPortNumber;
	     i <  (port_param->nStartPortNumber + port_param->nPorts); i++)
	{
		GooPort* port = goo_port_new ();
		g_assert (port != NULL);
		goo_port_prepare_definition (port);
		port->definition->nPortIndex = i;

		GOO_OBJECT_LOCK (self);
		GOO_RUN (
			OMX_GetParameter (self->handle,
					  OMX_IndexParamPortDefinition,
					  GOO_PORT_GET_DEFINITION (port))
			);
		GOO_OBJECT_UNLOCK (self);

		goo_list_append (self->ports, GOO_OBJECT (port));
		goo_object_set_owner (GOO_OBJECT (port), GOO_OBJECT (self));
		GOO_OBJECT_DEBUG (self, "refcount = %d",
				  G_OBJECT (self)->ref_count);

		if (port->definition->eDir == OMX_DirOutput)
		{
			gchar *name =
				g_strdup_printf ("output%d",
						 goo_list_get_length
						 (self->output_ports));
			goo_object_set_name (GOO_OBJECT (port), name);
			g_free (name);

			goo_list_append (self->output_ports,
					 GOO_OBJECT (port));
		}
		else if (port->definition->eDir == OMX_DirInput)
		{
			gchar *name =
				g_strdup_printf ("input%d",
						 goo_list_get_length
						 (self->input_ports));
			goo_object_set_name (GOO_OBJECT (port), name);
			g_free (name);

			goo_list_append (self->input_ports,
					 GOO_OBJECT (port));
		}

		GOO_OBJECT_INFO (port, "port created");
		g_object_unref (port);
	}

	g_free (port_param);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_flush_all_ports:
 * @self: An #GooComponent instance
 *
 * Flush all the pending buffers in the port's queue.
 */
void
goo_component_flush_all_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);

	GooIterator* iter = goo_component_iterate_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));

		/* @todo: figure out how to do this with  the -1 parameter */
		/* @fixme: must to implement a non-documented mechanism for
		* flushing tunneled ports */
		if (!goo_port_is_tunneled (port))
		{
			goo_component_flush_port (self, port);
		}

		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_configure_port:
 * @self: An #GooComponent instance
 * @port: An #GooPort instance
 *
 * Set in the OMX core the configuration of a specified port.
 **/
void
goo_component_configure_port (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_PORT (port));
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (goo_component_is_my_port (self, port));

	GOO_OBJECT_DEBUG (self, "Entering");

	if (goo_port_is_disabled (port))
	{
		GOO_OBJECT_DEBUG (port, "Port is disabled");
		return;
	}

	GOO_OBJECT_LOCK (self);

	GOO_RUN (
		OMX_SetParameter (self->handle,
				  OMX_IndexParamPortDefinition,
				  GOO_PORT_GET_DEFINITION (port))
		);

	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (port, "Port #%d configured",
			  GOO_PORT_GET_DEFINITION (port)->nPortIndex);

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

/**
 * goo_component_configure_all_ports:
 * @self: An #GooComponent instance
 *
 * Set in the OMX core the configuration of each port.
 **/
void
goo_component_configure_all_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "Entering");

	GooIterator* iter = goo_component_iterate_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));

		GOO_OBJECT_DEBUG (self, "Found one port, configuring it.");

		goo_component_configure_port (self, port);

		g_object_unref (port);
		goo_iterator_next (iter);
	}

	g_object_unref (iter);

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

/**
 * goo_component_set_ports_definition:
 * @self: A #GooComponent instance
 *
 * Set the port's definitions default values for each port in the component.
 * This method should be overriden in every specfic component.
 **/
void
goo_component_set_ports_definition (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);

	GooComponentClass *klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->set_ports_definition_func != NULL)
	{
		(klass->set_ports_definition_func) (self);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_validate_ports_definition:
 * @self: A #GooComponent instance
 *
 * Validates and calculates the assigned port's definitions values for each
 * port in the component.
 * This method should be overriden in every specfic component.
 **/
void
goo_component_validate_ports_definition (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "Entering");

	GooComponentClass *klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->validate_ports_definition_func != NULL)
	{
		(klass->validate_ports_definition_func) (self);
	}

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

/**
 * goo_component_allocate_port:
 * @self: An #GooComponent instance
 * @port: An #GooPort instance
 *
 * Allocates the buffers memory to use in the specified port in the component.
 **/
void
goo_component_allocate_port (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));
	g_assert (goo_component_is_my_port (self, port));

	if (goo_port_is_tunneled (port) || goo_port_is_disabled (port))
	{
		GOO_OBJECT_INFO (self, "Port is tunneled/disabled");
		return;
	}

	/* We must to assure this here because some components,
	   with only one port change its state with out the need to
	   allocate the port if it is tunneled */
	g_assert (self->cur_state == OMX_StateLoaded);

	guint numbuf = port->definition->nBufferCountActual;
	g_assert (numbuf > 0);
	guint bufsiz = port->definition->nBufferSize;
	g_assert (bufsiz > 0);
	guint index = port->definition->nPortIndex;

	gchar* portdef = goo_strportdef (GOO_PORT_GET_DEFINITION (port));
	GOO_OBJECT_INFO (self, "%s", portdef);
	g_free (portdef);

	/** after setting parameters cos bufsize */
	goo_port_allocate_buffer_header (port);

	guint i = 0;
	for (i = 0; i < numbuf; i++)
	{
		if (!port->use_buffer)
		{
			GOO_OBJECT_INFO (self, "AllocateBuffer 0x%x #%d",
					 port->buffer_header[i], i);
			GOO_OBJECT_LOCK (self);
			GOO_RUN (
				OMX_AllocateBuffer (self->handle,
						    &port->buffer_header[i],
						    index,
						    port,
						    bufsiz)
				);
			GOO_OBJECT_UNLOCK (self);
		}
		else
		{
			GOO_OBJECT_INFO (self, "UseBuffer 0x%d #%d",
					 port->buffer_header[i], i);
			GOO_OBJECT_LOCK (self);
			GOO_RUN (
				OMX_UseBuffer (self->handle,
					       &port->buffer_header[i],
					       index,
					       port,
					       bufsiz,
					       port->buffers_data[i])
				);
			GOO_OBJECT_UNLOCK (self);
		}
	}

	/* after allocating buffers in component cos pointers */
	goo_port_prepare_buffer_queue (port);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_allocate_all_ports:
 * @self: An #GooComponent instance
 *
 * Allocates the buffers memory to use in the specified port in the component.
 **/
void
goo_component_allocate_all_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	GooIterator* iter = goo_component_iterate_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		goo_component_allocate_port (self, port);
		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_wait_for_port_disabled:
 * @self: An #GooComponent instance
 * @port: An #GooPort instance
 *
 * Wait for port semaphore and update the port defintion
 **/
void
goo_component_wait_for_port_disabled (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));
	g_assert (goo_component_is_my_port (self, port));
	g_assert (self->cur_state != OMX_StateInvalid);

	GOO_OBJECT_INFO (port, "Waiting for port disabling");
	goo_semaphore_down (self->port_sem, TRUE);

	GOO_OBJECT_DEBUG (port, "Updating the port defintion");
	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_GetParameter (self->handle, OMX_IndexParamPortDefinition,
				  GOO_PORT_GET_DEFINITION (port))
		);
	GOO_OBJECT_UNLOCK (self);

	return;
}

/**
 * goo_component_disable_port:
 * @self: An #GooComponent instance
 * @port: An #GooPort instance
 *
 * Ask to the component to disable the specified port. We are assuming that
 * always the disabling petitions are downstream.
 **/
void
goo_component_disable_port (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));
	g_assert (goo_component_is_my_port (self, port));
	g_assert (self->cur_state != OMX_StateInvalid);

	if (goo_port_is_disabled (port))
	{
		GOO_OBJECT_INFO (port, "Port is already disabled");
		return;
	}

	gboolean dasftunnel = FALSE;
	gboolean prev_outport_queue = FALSE;

	OMX_PARAM_PORTDEFINITIONTYPE* param = NULL;
	param = GOO_PORT_GET_DEFINITION (port);

	guint index = param->nPortIndex;
	g_assert (index >= 0);

	gboolean queue_buffers = (param->eDir == OMX_DirOutput &&
				  !goo_port_is_tunneled (port));

	if (queue_buffers == TRUE)
	{
		/* we only want to fill our port queue */
		prev_outport_queue = goo_port_is_queued (port);
		g_object_set (port, "queued", TRUE, NULL);
	}

	if (goo_port_is_tunneled (port))
	{
		GooPort* peer_port = goo_port_get_peer (port);

		if (peer_port != NULL && port->supplier == FALSE)
		{
			GooComponent* peer_component;
			peer_component = GOO_COMPONENT (
				goo_object_get_owner (GOO_OBJECT (peer_port))
				);

			GOO_OBJECT_DEBUG (port,
					  "Propagating port disabling to the "
					  "supplier port");
			goo_component_disable_port (peer_component, peer_port);

			g_object_unref (peer_component);
			g_object_unref (peer_port);

			/* we are done now */
			return;
		}

		if (peer_port == NULL)
		{
			GOO_OBJECT_INFO (port, "DASF/unplugged port");
			dasftunnel = TRUE;
		}
	}

	GOO_OBJECT_INFO (port, "Disabling port");
	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SendCommand (self->handle, OMX_CommandPortDisable,
				 index, 0)
		);
	GOO_OBJECT_UNLOCK (self);

	/* non tunneled ports and allocated */
	if ((!goo_port_is_tunneled (port) || dasftunnel == TRUE) &&
	    (port->buffer_header != NULL))
	{
		guint i;
		OMX_BUFFERHEADERTYPE* buffer;

		for (i = 0; i < param->nBufferCountActual; i++)
		{
			buffer = goo_port_grab_buffer (port);

			GOO_OBJECT_INFO (port, "Deallocating buffer 0x%x #%d",
					 buffer, i);

			GOO_OBJECT_LOCK (self);
			GOO_RUN (
				OMX_FreeBuffer (self->handle, index, buffer)
				);
			GOO_OBJECT_UNLOCK (self);
		}
	}
	else if (goo_port_is_tunneled (port) && port->supplier == TRUE)
	{
		/* let's disable peer not supplier port */
		GooPort* peer_port = goo_port_get_peer (port);

		if (peer_port != NULL)
		{
			guint peer_index =
				GOO_PORT_GET_DEFINITION (peer_port)->nPortIndex;

			GooComponent* peer_component;
			peer_component = GOO_COMPONENT (
				goo_object_get_owner (GOO_OBJECT (peer_port))
				);

			GOO_OBJECT_INFO (peer_port, "Disabling port");
			GOO_OBJECT_LOCK (self);
			GOO_RUN (
				OMX_SendCommand (
					peer_component->handle,
					OMX_CommandPortDisable,
					peer_index, 0)
				);
			GOO_OBJECT_UNLOCK (self);

			goo_component_wait_for_port_disabled (peer_component,
							      peer_port);
		}
		else
		{
			GOO_OBJECT_ERROR (port,
					  "Something is wrong with this port");
		}
	}

	if (queue_buffers == TRUE)
	{
		g_object_set (port, "queued", prev_outport_queue, NULL);
	}

	goo_component_wait_for_port_disabled (self, port);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_disable_all_ports:
 * @self: An #GooComponent instance
 *
 * Ask to the component to disable all the port in it
 **/
void
goo_component_disable_all_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);

	GooIterator* iter = goo_component_iterate_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));

		/* DR!: not all the components raise an event after a
		 * tunneled port is disabled */
		if (!goo_port_is_tunneled (port))
		{
			goo_component_disable_port (self, port);
		}

		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_deallocate_port:
 * @self: An #GooComponent instance
 * @port: An #GooPort instance
 *
 * Free the memory allocated for the specified port's buffers.
 **/
void
goo_component_deallocate_port (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));
	g_assert (goo_component_is_my_port (self, port));
	g_assert (self->cur_state == OMX_StateIdle &&
		  self->next_state == OMX_StateLoaded);

	if (goo_port_is_tunneled (port) || goo_port_is_disabled (port))
	{
		GOO_OBJECT_INFO (self, "Port is tunneled/disabled");
		return;
	}

	guint numbuf = port->definition->nBufferCountActual;
	g_assert (numbuf > 0);
	guint index = port->definition->nPortIndex;
	g_assert (index >= 0);

	GOO_OBJECT_INFO (self, "Deallocating port's buffer #%d", index);

	guint i = 0;
	for (i = 0; i < numbuf; i++)
	{
		GOO_OBJECT_INFO (self, "Deallocating buffer 0x%x #%d",
				 port->buffer_header[i], i);
		GOO_OBJECT_LOCK (self);
		GOO_RUN (
			OMX_FreeBuffer (self->handle,
					index,
					port->buffer_header[i])
			);
		GOO_OBJECT_UNLOCK (self);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_deallocate_all_ports:
 * @self: An #GooComponent instance
 *
 * Free the buffers of all the ports associated to this compoment
 **/
void
goo_component_deallocate_all_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateIdle &&
		  self->next_state == OMX_StateLoaded);

	GooIterator* iter = goo_component_iterate_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		goo_component_deallocate_port (self, port);
		/** just to ensure the eos/tunneled values */
		goo_port_reset (port);
		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

#ifdef OUTPUT_QUEUE
/** function from output port buffer processing thread */
static void
goo_component_process_output_buffers (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	OMX_BUFFERHEADERTYPE* buffer = NULL;
	GooPort* port = NULL;
	GooIterator* iter = goo_component_iterate_output_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		port = GOO_PORT (goo_iterator_get_current (iter));
		if (!goo_port_is_tunneled (port) && !goo_port_is_eos (port))
		{
			buffer = goo_port_grab_buffer (port);
			GOO_OBJECT_INFO (port,
					 "popt output buffer: 0x%x", buffer);
			goo_port_process_buffer (port, buffer, self);
		}
		g_object_unref (G_OBJECT (port));
		goo_iterator_next (iter);
	}
	g_object_unref (G_OBJECT (iter));

	GOO_OBJECT_DEBUG (self, "");

	return;
}
#endif

#ifdef OUTPUT_QUEUE
/** thread function */
static void
goo_component_process_output_buffers_thread (gpointer data)
{
	g_assert (GOO_IS_COMPONENT (data));
	GooComponent* component = GOO_COMPONENT (g_object_ref (G_OBJECT (data)));

	while (!goo_component_is_done (component) &&
		(component->cur_state == OMX_StateExecuting ||
		 component->cur_state == OMX_StatePause))
	{
		goo_component_process_output_buffers (component);
	}

	g_object_unref (G_OBJECT (component));
	GOO_OBJECT_DEBUG (component, "refcount = %d",
			  G_OBJECT (component)->ref_count);

	GOO_OBJECT_DEBUG (component, "")
	g_thread_exit (NULL);
}
#endif

/**
 * goo_component_prepare_port:
 * @self: An #GooPort instance
 *
 * Take the output port and push all its buffers to the component
 * to wait to be filled.
 **/
void
goo_component_prepare_port (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));
	g_assert (goo_component_is_my_port (self, port));
	g_assert ((self->cur_state == OMX_StateExecuting) &&
		  (self->prev_state == OMX_StateIdle));

	guint numbuf, i;
	OMX_BUFFERHEADERTYPE* buffer = NULL;

	if (goo_port_is_tunneled (port))
	{
		GOO_OBJECT_DEBUG (port, "Port is tunneled");
		return;
	}

	if (goo_port_is_disabled (port))
	{
		GOO_OBJECT_DEBUG (port, "Port is disabled");
		return;
	}


	numbuf = GOO_PORT_GET_DEFINITION (port)->nBufferCountActual;
	g_assert (numbuf > 0);

	for (i = 0; i < numbuf; i++)
	{
		buffer = goo_port_try_grab_buffer (port);
		if (buffer != NULL)
		{
			buffer->nFilledLen = 0;
			goo_component_release_buffer (self, buffer);
		}
		else
		{
			break;
		}
	}

}

/**
 * goo_component_prepare_all_ports:
 * @self: An #GooComponent instance
 *
 * Take all the output port and push all its buffers to the component
 * to wait to be filled. This method should be called when the component
 * is in transition from idle to executing.
 **/
void
goo_component_prepare_all_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert ((self->cur_state == OMX_StateExecuting) &&
		  (self->prev_state == OMX_StateIdle));

	GooPort* port = NULL;
	gboolean do_thread = FALSE;

	GooIterator* iter = goo_component_iterate_output_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		port = GOO_PORT (goo_iterator_get_current (iter));
		GOO_OBJECT_DEBUG (port, "processing output port");

		do_thread = TRUE;

		goo_component_prepare_port (self, port);

		g_object_unref (port);
		goo_iterator_next (iter);
	}
	g_object_unref (iter);

	/** @todo what happens if there's only output ports?? there's
	 * no need of this thread!!
	 */
#ifdef OUTPUT_QUEUE
	if (do_thread == TRUE && goo_list_get_length (self->input_ports) > 0)
	{
		GOO_OBJECT_INFO (self, "Creating the output thread");
		self->output_thread = g_thread_create
			(goo_component_process_output_buffers_thread, self,
			 FALSE, NULL);
		g_assert (self->output_thread != NULL);
	}
#endif

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_iterate_ports:
 * @self: an #GooComponent instance
 *
 * Retrieves an iterator of component's ports.	The iterator should be unref
 * after usage.
 *
 * Returns: the #GooIterator of #GooPort. Unref each port after use.
 *
 * MT Safe.
 **/
GooIterator*
goo_component_iterate_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	GooIterator* iter = goo_list_create_iterator (self->ports);
	g_assert (iter != NULL);

	return iter;
}

/**
 * goo_component_iterate_input_ports:
 * @self: an #GooComponent instance
 *
 * Retrieves an iterator of component's input ports.  The iterator should
 * be unref after usage.
 *
 * Returns: the #GooIterator of input #GooPort. Unref each port after use.
 *
 * MT Safe.
 **/
GooIterator*
goo_component_iterate_input_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	GooIterator* iter = goo_list_create_iterator (self->input_ports);
	g_assert (iter != NULL);

	return iter;
}

/**
 * goo_component_iterate_output_ports:
 * @self: an #GooComponent instance
 *
 * Retrieves an iterator of component's output ports.  The iterator should
 * be unref after usage.
 *
 * Returns: the #GooIterator of output #GooPort. Unref each port after use.
 *
 * MT Safe.
 **/
GooIterator*
goo_component_iterate_output_ports (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	GooIterator* iter = goo_list_create_iterator (self->output_ports);
	g_assert (iter != NULL);

	return iter;
}

/**
 * goo_component_get_port:
 * @self: An #GooComponent instance
 * @name: The name of the port. It could "input#" or "output#"
 *
 * Retrieves the requested port label with the given name.  It should be
 * unref after usage.
 *
 * Returns: The #GooPort or %NULL
 **/
GooPort*
goo_component_get_port (GooComponent *self, gchar* name)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (name != NULL);

	gchar* port_name = NULL;
	GooPort* port = NULL;
	GooIterator* iter = goo_component_iterate_ports (self);
	while (!goo_iterator_is_done (iter))
	{
		port = GOO_PORT (goo_iterator_get_current (iter));

		port_name = goo_object_get_name (GOO_OBJECT (port));
		if (port_name != NULL)
		{
			if (g_ascii_strncasecmp (name, port_name,
						 strlen (port_name)) == 0)
			{
				g_free (port_name);
				g_object_unref (G_OBJECT (iter));

				return port;
			}

			g_free (port_name);
		}

		g_object_unref (G_OBJECT (port));
		goo_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));

	GOO_OBJECT_DEBUG (self, "");

	return NULL;
}

/**
 * goo_component_is_my_port:
 * @self: A #GooComponent instance
 * @port: A #GooPort instance
 *
 * Validate if the specified instance of #GooPort belong to the specified
 * #GooComponent instance.
 *
 * Retrun value: TRUE if the the port is part of the component; FALSE otherwise
 **/
gboolean
goo_component_is_my_port (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));

	gboolean retval;
	GooComponent *parent = GOO_COMPONENT (
		goo_object_get_owner (GOO_OBJECT (port))
		);
	retval = (parent == self);
	g_object_unref (parent);

	return retval;
}

/**
 * goo_component_set_parameter_by_name:
 * @self: An #GooComponent instance
 * @name: A string with the name of the custom parameter to set
 * @data: A pointer to the data to set in the custom parameter
 *
 * Set a custom parameter in the OMX component referenced by its name.
 *
 * Return value: TRUE if the value is set correctly; FALSE otherwise
 **/
gboolean
goo_component_set_parameter_by_name (GooComponent *self,
				     gchar* name, gpointer data)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);
	g_assert (data != NULL);
	g_assert (name != NULL);

	OMX_INDEXTYPE index;
	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_GetExtensionIndex (self->handle,
				       name,
				       (OMX_INDEXTYPE*) &index)
		);

	RETURN_GOO_RUN (
		OMX_SetParameter (self->handle,
			       index,
			       (OMX_PTR) data)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_set_parameter_by_index:
 * @self: An #GooComponent instance
 * @index: An enum value which identifies the custom parameter to set
 * @data: A pointer to the data to set in the custom parameter
 *
 * Set a custom parameter in the OMX component referenced by its
 * index value.
 *
 * Return value: TRUE if the value is set correctly; FALSE otherwise
 **/
gboolean
goo_component_set_parameter_by_index (GooComponent *self,
				      OMX_INDEXTYPE index, gpointer data)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);
	g_assert (data != NULL);
	g_assert (index >= 0);

	GOO_OBJECT_LOCK (self);
	GOO_RUN (
		OMX_SetParameter (self->handle,
				  index,
				  (OMX_PTR) data)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_get_parameter_by_name:
 * @self: An #GooComponent instance
 * @name: A string with the name of the custom parameter to set
 * @data: A pointer to the data to set in the custom parameter
 *
 * Get a custom parameter in the OMX component referenced by its name.
 *
 * Return value: TRUE if the value is set correctly; FALSE otherwise
 **/
gboolean
goo_component_get_parameter_by_name (GooComponent *self,
				     gchar* name, gpointer data)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);
	g_assert (data != NULL);
	g_assert (name != NULL);

	OMX_INDEXTYPE index;
	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_GetExtensionIndex (self->handle,
				       name,
				       (OMX_INDEXTYPE*) &index)
		);

	RETURN_GOO_RUN (
		OMX_GetParameter (self->handle,
				  index,
				  (OMX_PTR) data)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_get_parameter_by_index:
 * @self: An #GooComponent instance
 * @index: An enum value which identifies the custom parameter to set
 * @data: A pointer to the data to set in the custom parameter
 *
 * Get a custom parameter in the OMX component referenced by its
 * index value.
 *
 * Return value: TRUE if the value is set correctly; FALSE otherwise
 **/
gboolean
goo_component_get_parameter_by_index (GooComponent *self,
				      OMX_INDEXTYPE index, gpointer data)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);
	g_assert (data != NULL);
	g_assert (index >= 0);

	GOO_OBJECT_LOCK (self);

	GOO_RUN (
		OMX_GetParameter (self->handle,
				  index,
				  (OMX_PTR) data)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_set_config_by_name:
 * @self: An #GooComponent instance
 * @name: A string with the name of the custom parameter to set
 * @data: A pointer to the data to set in the custom parameter
 *
 * Set a custom configuration in the OMX component referenced by its name.
 *
 * Return value: TRUE if the value is set correctly; FALSE otherwise
 **/
gboolean
goo_component_set_config_by_name (GooComponent *self,
				  gchar* name, gpointer data)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);
	g_assert (data != NULL);
	g_assert (name != NULL);

	OMX_INDEXTYPE index;
	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_GetExtensionIndex (self->handle,
				       name,
				       (OMX_INDEXTYPE*) &index)
		);

	RETURN_GOO_RUN (
		OMX_SetConfig (self->handle,
			       index,
			       (OMX_PTR) data)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_set_config_by_index:
 * @self: An #GooComponent instance
 * @index: An enum value which identifies the custom parameter to set
 * @data: A pointer to the data to set in the custom parameter
 *
 * Set a custom configuration in the OMX component referenced by its
 * index value.
 *
 * Return value: TRUE if the value is set correctly; FALSE otherwise
 **/
gboolean
goo_component_set_config_by_index (GooComponent *self,
				   OMX_INDEXTYPE index, gpointer data)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);
	g_assert (data != NULL);
	g_assert (index >= 0);

	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_SetConfig (self->handle,
			       index,
			       (OMX_PTR) data)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_get_config_by_name:
 * @self: An #GooComponent instance
 * @name: A string with the name of the custom parameter to get
 * @data: A pointer where the data is going to be stored
 *
 * Get a custom parameter in the OMX component referenced by its name.
 *
 * Return value: TRUE if the value is get correctly; FALSE otherwise
 **/
 gboolean
goo_component_get_config_by_name (GooComponent* self,
				  gchar* name, gpointer data)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);
	g_assert (data != NULL);
	g_assert (name != NULL);

	OMX_INDEXTYPE index;
	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_GetExtensionIndex (self->handle,
				       name,
				       (OMX_INDEXTYPE*) &index)
		);

	RETURN_GOO_RUN (
		OMX_GetConfig (self->handle,
			       index,
			       (OMX_PTR) data)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_get_config_by_index:
 * @self: An #GooComponent instance
 * @index: An enum value of the custom parameter to get
 * @data: A pointer where the data is going to be stored
 *
 * Get a custom parameter in the OMX component referenced by its index.
 *
 * Return value: TRUE if the value is get correctly; FALSE otherwise
 **/
gboolean
goo_component_get_config_by_index (GooComponent *self,
				   OMX_INDEXTYPE index, gpointer data)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state != OMX_StateInvalid);
	g_assert (data != NULL);
	g_assert (index >= 0);

	GOO_OBJECT_LOCK (self);
	RETURN_GOO_RUN (
		OMX_GetConfig (self->handle,
			       index,
			       (OMX_PTR) data)
		);
	GOO_OBJECT_UNLOCK (self);

	GOO_OBJECT_DEBUG (self, "");

	return TRUE;
}

/**
 * goo_component_set_parameters:
 * @self: An #GooComponent instance
 *
 * Configure all the miscellaneous parameters of the specific compoment.
 * This is a pure virtual function which your component should override.
 **/
void
goo_component_set_parameters (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);

	GooComponentClass *klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->set_parameters_func != NULL)
	{
		(klass->set_parameters_func) (self);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_load_parameters:
 * @self: An #GooComponent instance
 *
 * Initializes all the miscellaneous parameters of the specific compoment.
 * This is a pure virtual function which your component should override.
 **/
void
goo_component_load_parameters (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateLoaded);

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->load_parameters_func != NULL)
	{
		(klass->load_parameters_func) (self);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}


/**
 * goo_component_set_done:
 * @self: An #GooComponent instance
 *
 * Mark the component if all the stream have been processed.
 **/
void
goo_component_set_done (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	if (goo_component_is_done (self))
	{
		return;
	}

	g_assert ((self->cur_state == OMX_StateExecuting) &&
		  (self->prev_state == OMX_StateIdle || self->prev_state == OMX_StatePause));

	/* GOO_OBJECT_LOCK (self); */
	self->done = TRUE;
	/* GOO_OBJECT_UNLOCK (self); */

	goo_semaphore_up (self->done_sem);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_is_done:
 * @self: An #GooComponent instance
 *
 * Check if the component have been processed all the stream
 *
 * Return value: TRUE if the stream have been processed; FALSE otherwise
 **/
gboolean
goo_component_is_done (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert ((self->cur_state == OMX_StateExecuting) &&
		  (self->prev_state == OMX_StateIdle || self->prev_state == OMX_StatePause));

	gboolean retval;

	/* GOO_OBJECT_LOCK (self); */
	retval = self->done;
	/* GOO_OBJECT_UNLOCK (self); */

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

/**
 * goo_component_wait_for_done:
 * @self: A #GooComponent instance
 *
 * The thread will be block until the done condition is activated (this is a
 * library user reponsability).
 **/
void
goo_component_wait_for_done (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->cur_state == OMX_StateExecuting);

	while (!goo_component_is_done (self))
	{
		goo_semaphore_down (self->done_sem, FALSE);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_load:
 * @self: An #GooComponent instance
 *
 * This methods will locate the component specified, load that component
 * into memory, and validate it.
 *
 * Return value: TRUE if the component was loaded correctly; FALSE otherwise
 **/
gboolean
goo_component_load (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (self->id != NULL);
	g_assert (self->cur_state == OMX_StateInvalid);

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	gboolean retval = FALSE;
	if (klass->load_func != NULL)
	{
		retval = (klass->load_func) (self);
	}

	return retval;
}

/**
 * goo_component_unload:
 * @self: An #GooComponent instance
 *
 * This method will free the OMX component from memory.
 *
 * Return value: TRUE if the component is free correctly; FALSE if otherwise
 */
gboolean
goo_component_unload (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	g_assert (XOR (self->cur_state == OMX_StateInvalid,
		       self->cur_state == OMX_StateLoaded));

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	gboolean retval = FALSE;
	if (klass->unload_func != NULL)
	{
		retval = (klass->unload_func) (self);
	}

	return retval;
}

/**
 * goo_component_set_state_idle:
 * @self: An #GooComponent instance
 *
 * Set the component in Idle State.
 * Before you call this method, you should loaded the component, loaded it
 * ports and configured them, as the configured the component itself.
 **/
void
goo_component_set_state_idle (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (XOR (XOR (XOR (self->cur_state == OMX_StateLoaded,
				 self->cur_state == OMX_StateExecuting),
			    self->cur_state == OMX_StateIdle),
		       self->cur_state == OMX_StatePause));

	if (self->cur_state == OMX_StateIdle)
	{
		return;
	}

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->set_state_idle_func != NULL)
	{
		(klass->set_state_idle_func) (self);
	}

	return;
}

/**
 * goo_component_set_state_executing:
 * @self: An #GooComponent instance
 *
 * Set the component in Executing state.
 * After calling this methods you should process the buffers interchange.
 **/
void
goo_component_set_state_executing (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (XOR (XOR (self->cur_state == OMX_StateIdle,
			    self->cur_state == OMX_StateExecuting),
		       self->cur_state == OMX_StatePause));

	if (self->cur_state == OMX_StateExecuting)
	{
		return;
	}

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->set_state_executing_func != NULL)
	{
		(klass->set_state_executing_func) (self);
	}

	return;
}

/**
 * goo_component_set_state_loaded:
 * @self: An #GooComponent instance
 *
 * Set the compoment in Loaded state
 * This state should be called when we're going to unload the component.
 * The method disable and deallocates all the component's ports.
 **/
void
goo_component_set_state_loaded (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (XOR (self->cur_state == OMX_StateIdle,
		       self->cur_state == OMX_StateLoaded));

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (self->cur_state == OMX_StateLoaded)
	{
		return;
	}

	if (klass->set_state_loaded_func != NULL)
	{
		(klass->set_state_loaded_func) (self);
	}

	return;
}

/**
 * goo_component_set_state_pause:
 * @self: An #GooComponent instance
 *
 * Set the compoment in Pause state.
 * This state should be called when the component is in idle or executing
 * state.
 **/
void
goo_component_set_state_pause (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (XOR (XOR (self->cur_state == OMX_StateIdle,
			    self->cur_state == OMX_StateExecuting),
		       self->cur_state == OMX_StatePause));

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (self->cur_state == OMX_StatePause)
	{
		return;
	}

	if (klass->set_state_pause_func != NULL)
	{
		(klass->set_state_pause_func) (self);
	}

	return;
}

/**
 * goo_component_release_buffer:
 * @self: An #GooComponent instance
 * @buffer: An #OMX_BUFFERHEADERTYPE pointer
 *
 * This method will call the functions OMX_FillThisBuffer() or
 * OMX_EmptyThisBuffer() depending on the port type defined in the buffer
 * structure.
 **/
void
goo_component_release_buffer (GooComponent* self, OMX_BUFFERHEADERTYPE* buffer)
{
	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);
	(klass->release_buffer_func) (self, buffer);
}

/**
 * goo_component_send_eos:
 * @self: a #GooComponent instance
 *
 * This method will send an empty buffer with the EOS flag raised to every
 * input port if it is not tunneled and also it is not marked as EOS already.
 **/
void
goo_component_send_eos (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (XOR (self->cur_state == OMX_StateExecuting,
		       self->cur_state == OMX_StateIdle));

	OMX_BUFFERHEADERTYPE* buffer = NULL;
	GooPort* port = NULL;
	GooIterator* iter = goo_component_iterate_input_ports (self);

	while (!goo_iterator_is_done (iter))
	{
		port = GOO_PORT (goo_iterator_get_current (iter));

		if (!goo_port_is_tunneled (port) &&
		    !goo_port_is_eos (port) &&
		    goo_port_is_enabled (port))
		{
			GOO_OBJECT_INFO (self,
					 "sending an empty buffer "
					 "with EOS flag");
			buffer = goo_port_grab_buffer (port);
			buffer->nFlags |= OMX_BUFFERFLAG_EOS;
			buffer->nFilledLen = 0;
			goo_component_release_buffer (self, buffer);
		}

		g_object_unref (G_OBJECT (port));
		goo_iterator_next (iter);
	}

	g_object_unref (G_OBJECT (iter));

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_set_tunnel:
 * @src: a #GooComponent instance
 * @srcport: a #GooPort instance
 * @sink: a #GooComponent instance
 * @sinkport: a #GooPort instance
 * @index: a #OMX_BUFFERSUPPLIER enumeration
 *
 * This method will setup a tunnel between two GooComponents.
 *
 **/
void
goo_component_set_tunnel (GooComponent* src,  GooPort* srcport,
			  GooComponent* sink, GooPort* sinkport,
			  OMX_BUFFERSUPPLIERTYPE index)
{
	g_assert (GOO_IS_COMPONENT (src));
	g_assert (GOO_IS_COMPONENT (sink));

	g_assert (GOO_IS_PORT (srcport));
	g_assert (GOO_IS_PORT (sinkport));
	g_assert (srcport != sinkport);

	g_assert ((src->cur_state == OMX_StateLoaded) &
		  (sink->cur_state == OMX_StateLoaded));

	g_assert ((!goo_port_is_tunneled (srcport)) &
		  (!goo_port_is_tunneled (sinkport)));

	/* as in idle state */
	{
		goo_component_configure (src);
		goo_component_configure (sink);
	}

	guint src_port_num = GOO_PORT_GET_DEFINITION (srcport)->nPortIndex;
	guint sink_port_num = GOO_PORT_GET_DEFINITION (sinkport)->nPortIndex;

	GOO_OBJECT_LOCK (src);
	GOO_RUN (
		OMX_SetupTunnel (src->handle, src_port_num,
				 sink->handle, sink_port_num)
		);
	GOO_OBJECT_UNLOCK (src);

	goo_port_set_peer (srcport, sinkport);
/*
	goo_component_set_supplier_port (src, srcport, index);
	goo_component_set_supplier_port (sink, sinkport, index);
*/
	GOO_OBJECT_DEBUG (src, "");
	GOO_OBJECT_DEBUG (sink, "");

	return;
}

/**
 * goo_component_set_tunnel_by_name:
 * @src: a #GooComponent instance
 * @srcport: the name of the source component port
 * @sink: a #GooComponent instance
 * @sinkport: the name of the sink component port
 *
 * This method will setup a tunnel between two GooComponents.
 *
 **/
void
goo_component_set_tunnel_by_name (GooComponent* src,  gchar* srcportname,
				  GooComponent* sink, gchar* sinkportname,
				  OMX_BUFFERSUPPLIERTYPE index)
{
	g_assert (GOO_IS_COMPONENT (src));
	g_assert (GOO_IS_COMPONENT (sink));

	g_assert (srcportname != NULL);
	g_assert (sinkportname != NULL);

	GooPort* srcport = goo_component_get_port (src, srcportname);
	g_assert (GOO_IS_PORT (srcport));

	GooPort* sinkport = goo_component_get_port (sink, sinkportname);
	g_assert (GOO_IS_PORT (sinkport));

	goo_component_set_tunnel(src, srcport, sink, sinkport, index);

	g_object_unref (srcport);
	g_object_unref (sinkport);

	return;
}

/**
 * goo_component_propagate_state:
 * @self: An #GooComponent instance
 *
 * Manage changes of states between supplier
 * and non-supplier ports in a tunnel configuration.
 **/
gboolean
goo_component_propagate_state (GooComponent* self, OMX_STATETYPE state)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (XOR (XOR (state == OMX_StateIdle,
			    state == OMX_StateExecuting),
		       state == OMX_StateLoaded));

	gboolean retval = FALSE;
	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->propagate_state_func != NULL)
	{
		retval = (klass->propagate_state_func) (self, state);
	}

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

/**
 * goo_component_propagate_wait_for_next_state:
 * @self: An #GooComponent instance
 *
 * Waits for the supplier port to change state before
 * waiting for the non-supplier to change state
 *
 **/
void
goo_component_propagate_wait_for_next_state (GooComponent* self)
{
	g_assert (GOO_IS_COMPONENT (self));

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->propagate_wait_for_next_state_func != NULL)
	{
		(klass->propagate_wait_for_next_state_func) (self);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

/**
 * goo_component_set_clock:
 * @self: A #GooComponent instance
 * @clock: The factory's clock instance
 *
 * This method assigns a global clock component to the component.
 * This methods is should be used only by the #GooComponentFactory instance.
 */
gboolean
goo_component_set_clock (GooComponent* self, GooComponent* clock)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_COMPONENT (clock));
	g_assert (self->cur_state == OMX_StateLoaded);
	g_assert (clock->cur_state == OMX_StateLoaded);

	gboolean retval = FALSE;
	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->set_clock_func != NULL)
	{
		retval = (klass->set_clock_func) (self, clock);
	}

	if (retval == TRUE)
	{
		self->clock = g_object_ref (G_OBJECT (clock));
	}

	GOO_OBJECT_DEBUG (self, "");

	return retval;
}

/**
 * goo_component_flush_port:
 * @self: A #GooComponent instance
 * @port: A #GooPort instances
 *
 * This function flush the buffers and queue them into the port queue.
 *
 * This function will activate the use of the out port queue. So if your
 * component use the callback to process the data, these buffers won't be
 * processed.
 */
void
goo_component_flush_port (GooComponent* self, GooPort* port)
{
	g_assert (GOO_IS_COMPONENT (self));
	g_assert (GOO_IS_PORT (port));
	g_assert (self->cur_state != OMX_StateInvalid);
	g_assert (goo_component_is_my_port (self, port));

	GooComponentClass* klass = GOO_COMPONENT_GET_CLASS (self);

	if (klass->flush_port_func != NULL)
	{
		(klass->flush_port_func) (self, port);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}
