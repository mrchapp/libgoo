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

#ifndef _GOO_COMPONENT_H_
#define _GOO_COMPONENT_H_

#include <goo-list.h>
#include <goo-semaphore.h>
#include <goo-port.h>

G_BEGIN_DECLS

#define GOO_TYPE_COMPONENT \
	(goo_component_get_type ())
#define GOO_COMPONENT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_COMPONENT, GooComponent))
#define GOO_COMPONENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_COMPONENT, GooComponentClass))
#define GOO_IS_COMPONENT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_COMPONENT))
#define GOO_IS_COMPONENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_COMPONENT))
#define GOO_COMPONENT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_COMPONENT, GooComponentClass))

/**
 * GooComponentOp:
 * @GOO_COMPONENT_IS_UNKNOWN: component type is unknown.
 * @GOO_COMPONENT_IS_SRC: the component is a source.
 * @GOO_COMPONENT_IS_SINK: the component is a sink.
 * @GOO_COMPONENT_IS_FILTER: the component is a filter.
 *
 * The type of the component operation.
 */
typedef enum
{
	GOO_COMPONENT_IS_UNKNOWN,
	GOO_COMPONENT_IS_SINK,
	GOO_COMPONENT_IS_SRC,
	GOO_COMPONENT_IS_FILTER
} GooComponentOp;

typedef struct _GooComponent GooComponent;
typedef struct _GooComponentClass GooComponentClass;

/**
 * GooComponent:
 * @ports: The list of available ports
 * @input_ports: The list of available input ports
 * @output_ports: The list of available output ports
 * @handle: The #OMX_HANDLETYPE of the compoment
 * @prev_state: The previous component's state
 * @cur_state: The current component's state
 * @next_state: The next component's state
 * @callbacks: Component's OpenMAX callbacks
 * @id: The name of the OpenMAX component's handle
 * @port_param_type: The component's type
 @ @clock: pointer to the global OMX clock
 *
 * Represent a higher abstraction of an OpenMAX component.
 */
struct _GooComponent
{
	/*< protected >*/
	GooObject parent;

	GooList* ports;
	GooList* input_ports;
	GooList* output_ports;

	OMX_HANDLETYPE handle;
	OMX_STATETYPE prev_state;
	OMX_STATETYPE cur_state;
	OMX_STATETYPE next_state;
	OMX_CALLBACKTYPE callbacks;

	GooComponent* clock;

	/*< protected >*/
	GooSemaphore* state_sem;
	GooSemaphore* port_sem;

	/*< protected >*/
	gboolean done;
	GooSemaphore* done_sem;

	gchar* id;
	OMX_INDEXTYPE port_param_type;

	/*< protected >*/
	gboolean configured;

	/*< protected >*/
#ifdef OUTPUT_QUEUE
	GThread* output_thread;
#endif
};

struct _GooComponentClass
{
	GooObjectClass parent_class;

	/* virtual functions */
	void (*load_parameters_func) (GooComponent* self);
	void (*set_parameters_func) (GooComponent* self);

	void (*set_ports_definition_func) (GooComponent* self);
	void (*validate_ports_definition_func) (GooComponent* self);

	gboolean (*load_func) (GooComponent* self);
	gboolean (*unload_func) (GooComponent* self);

	void (*set_state_idle_func) (GooComponent* self);
	void (*set_state_executing_func) (GooComponent* self);
	void (*set_state_loaded_func) (GooComponent* self);
	void (*set_state_pause_func) (GooComponent* self);
	gboolean (*propagate_state_func) (GooComponent* self,
					  OMX_STATETYPE state);
	void (*propagate_wait_for_next_state_func) (GooComponent* self);

	gboolean (*set_clock_func) (GooComponent* self, GooComponent *clock);
	void (*eos_flag_func) (GooComponent* self, guint portindex);
	void (*release_buffer_func) (GooComponent* self,
				     OMX_BUFFERHEADERTYPE* buffer);
	void (*flush_port_func) (GooComponent* self, GooPort* port);
	void (*event_handler_extra) (
			OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
			OMX_EVENTTYPE eEvent, OMX_U32 nData1,
			OMX_U32 nData2, OMX_PTR pEventData);
};

GType goo_component_get_type (void);

/* handle */
gboolean goo_component_load (GooComponent* self);
gboolean goo_component_unload (GooComponent* self);

/* state management */
void goo_component_set_state_idle (GooComponent* self);
void goo_component_set_state_executing (GooComponent* self);
void goo_component_set_state_loaded (GooComponent* self);
void goo_component_set_state_pause (GooComponent* self);
void goo_component_wait_for_next_state (GooComponent* self);
OMX_STATETYPE goo_component_get_state (GooComponent* self);
gboolean goo_component_propagate_state (GooComponent* self,
					OMX_STATETYPE state);
void goo_component_propagate_wait_for_next_state (GooComponent* self);

/* port management */
gboolean goo_component_load_ports (GooComponent* self);
void goo_component_configure_all_ports (GooComponent* self);
void goo_component_configure_port (GooComponent* self, GooPort* port);
void goo_component_configure (GooComponent* self);
void goo_component_set_ports_definition (GooComponent* self);
void goo_component_validate_ports_definition (GooComponent* self);
void goo_component_allocate_port (GooComponent* self, GooPort* port);
void goo_component_allocate_all_ports (GooComponent* self);
void goo_component_disable_port (GooComponent* self, GooPort* port);
void goo_component_disable_all_ports (GooComponent* self);
void goo_component_deallocate_port (GooComponent* self, GooPort* port);
void goo_component_deallocate_all_ports (GooComponent* self);
void goo_component_prepare_all_ports (GooComponent* self);
void goo_component_prepare_port (GooComponent* self, GooPort* port);
void goo_component_flush_port (GooComponent* self, GooPort* port);
void goo_component_flush_all_ports (GooComponent* self);

GooIterator* goo_component_iterate_ports (GooComponent* self);
GooIterator* goo_component_iterate_input_ports (GooComponent* self);
GooIterator* goo_component_iterate_output_ports (GooComponent* self);

GooPort* goo_component_get_port (GooComponent* self, gchar* name);
GooComponent* goo_component_get_peer_component (GooComponent* self,
						GooPort* port);
GooComponent * goo_component_get_tunnel_head (GooComponent *self);
gboolean goo_component_is_my_port (GooComponent* self, GooPort* port);
gboolean goo_component_set_clock (GooComponent* self, GooComponent* clock);

/* config manangement */
gboolean goo_component_set_parameter_by_name (GooComponent* self,
					      gchar* name, gpointer data);
gboolean goo_component_set_parameter_by_index (GooComponent* self,
					       OMX_INDEXTYPE index,
					       gpointer data);
gboolean goo_component_get_parameter_by_name (GooComponent* self,
					      gchar* name, gpointer data);
gboolean goo_component_get_parameter_by_index (GooComponent* self,
					       OMX_INDEXTYPE index,
					       gpointer data);
gboolean goo_component_set_config_by_name (GooComponent* self,
					   gchar* name, gpointer data);
gboolean goo_component_set_config_by_index (GooComponent* self,
					    OMX_INDEXTYPE index,
					    gpointer data);
gboolean goo_component_get_config_by_name (GooComponent* self,
					    gchar* name, gpointer data);
gboolean goo_component_get_config_by_index (GooComponent* self,
					    OMX_INDEXTYPE index,
					    gpointer data);
void goo_component_load_parameters (GooComponent* self);
void goo_component_set_parameters (GooComponent* self);

/* eos management */
void goo_component_set_done (GooComponent* self);
gboolean goo_component_is_done (GooComponent* self);
void goo_component_wait_for_done (GooComponent* self);

/* buffer handling */
void goo_component_release_buffer (GooComponent* self,
				   OMX_BUFFERHEADERTYPE* buffer);
void goo_component_send_eos (GooComponent* self);

/* advanced candies */
void goo_component_set_tunnel (GooComponent* src, GooPort* srcport,
			       GooComponent* sink, GooPort* sinkport,
			       OMX_BUFFERSUPPLIERTYPE index);
void goo_component_set_tunnel_by_name (GooComponent* src, gchar* srcportname,
				       GooComponent* sink, gchar* sinkportname,
				       OMX_BUFFERSUPPLIERTYPE index);

G_END_DECLS

#endif /* _GOO_COMPONENT_H_ */
