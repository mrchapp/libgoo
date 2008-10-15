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

#ifndef _GOO_PORT_H_
#define _GOO_PORT_H_

#include <goo-object.h>
#include <OMX_Core.h>

G_BEGIN_DECLS

#define GOO_TYPE_PORT \
	(goo_port_get_type ())
#define GOO_PORT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_PORT, GooPort))
#define GOO_PORT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_PORT, GooPortClass))
#define GOO_IS_PORT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_PORT))
#define GOO_IS_PORT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_PORT))
#define GOO_PORT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_PORT, GooPortClass))

typedef struct _GooPort GooPort;
typedef struct _GooPortClass GooPortClass;

/**
 * GooPortProcessBufferFunction:
 * @port: The #GooPort instance that process the buffer
 * @buffer: The #OMX_BUFFERHEADERTYPE that is going to be processed,
 * not %NULL.
 * @data: A pointer to application data
 *
 * A function that will be called on ports when processing buffers.
 * The function typically puts data on the buffer from the source, if it is
 * an input port; or pull out data from the buffer to the sink, if it is an
 * output port.
 **/
typedef void (*GooPortProcessBufferFunction) (GooPort* port, OMX_BUFFERHEADERTYPE* buffer, gpointer data);

/**
 * GooPort:
 * @buffer_header: the #OMX_BUFFERHEADERTYPE array of buffers
 * @definition: The #OMX_PARAM_PORTDEFINITIONTYPE of the port
 * @buffer_data: The buffers data if we're in use_buffer mode
 * @use_buffer: TRUE if we are in use_buffer mode
 * @tunneled: TRUE if the port is tunneled
 * @supplier: TRUE if the port is the buffer supplier
 * @padding: The padding size of the buffer data if we are in use_buffer mode
 * @eos: TRUE if the port have received an End-Of-Stream message
 * @buffer_queue: Queue of available buffers in the port
 * @processbuffer_func: The function pointer to #GooPortProcessBufferFunction
 * @enqueue: TRUE if the buffers are queued to @buffer_queue; FALSE if the
 *           are processed by @processbuffer_func
 * @peer: The next #GooPort in a tunnel
 *
 * Represent a higher abstraction of an OpenMAX port
 */
struct _GooPort
{
	/*< protected >*/
	GooObject parent;

	OMX_BUFFERHEADERTYPE** buffer_header;
	OMX_PARAM_PORTDEFINITIONTYPE* definition;

	gpointer* buffers_data;
	gboolean use_buffer;
	gboolean tunneled;
	gboolean supplier;
	guint padding;
	gboolean eos;

	/*< protected >*/
	GMutex* eos_mutex;

	gboolean enqueue;
	GAsyncQueue* buffer_queue;
	GooPortProcessBufferFunction processbuffer_func;
	GooPort* peer;
};

struct _GooPortClass
{
	GooObjectClass parent_class;
};

/* helper macros */
/**
 * GOO_PORT_GET_DEFINITION:
 * @port: An #GooPort instance
 *
 * Extract the port's definition
 *
 * Return value: a pointer to the #OMX_PARAM_PORTDEFINITIONTYPE structure
 **/
#define GOO_PORT_GET_DEFINITION(port) (GOO_PORT (port)->definition)

/* functions */
GType goo_port_get_type (void);
GooPort* goo_port_new ();

void goo_port_prepare_definition (GooPort* self);
void goo_port_allocate_buffer_header (GooPort* self);
void goo_port_prepare_buffer_queue (GooPort* self);
void goo_port_allocate_buffers (GooPort* self);

void goo_port_push_buffer (GooPort* self, OMX_BUFFERHEADERTYPE* buffer);
OMX_BUFFERHEADERTYPE* goo_port_grab_buffer (GooPort* self);
OMX_BUFFERHEADERTYPE* goo_port_try_grab_buffer (GooPort* self);

void goo_port_reset (GooPort* self);
void goo_port_set_eos (GooPort* self);
gboolean goo_port_is_eos (GooPort* self);

gboolean goo_port_is_tunneled (GooPort* self);
gboolean goo_port_is_disabled (GooPort* self);

gboolean goo_port_is_enabled (GooPort* port);

gboolean goo_port_is_queued (GooPort* port);
void goo_port_set_process_buffer_function (GooPort* self,
					   GooPortProcessBufferFunction pbf);
void goo_port_process_buffer (GooPort* self, OMX_BUFFERHEADERTYPE* buffer,
			      gpointer data);

gboolean goo_port_set_peer (GooPort* self, GooPort* peer);
GooPort* goo_port_get_peer (GooPort* self);

gboolean goo_port_is_my_buffer (GooPort* self, OMX_BUFFERHEADERTYPE* buffer);
gboolean goo_port_is_external_buffer (GooPort* self);


G_END_DECLS

#endif /* _GOO_PORT_H_ */
