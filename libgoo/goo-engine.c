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

#include <goo-engine.h>
#include <goo-port.h>
#include <goo-utils.h>

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

G_DEFINE_TYPE (GooEngine, goo_engine, GOO_TYPE_OBJECT)

enum _GooEngineProp
{
	PROP_0,
	PROP_COMPONENT,
	PROP_INFILE,
	PROP_OUTFILE,
	PROP_NUMBUFFERS,
	PROP_EOSEVENT,
	PROP_VOPFILE
};

typedef enum
{
	PORT_INPUT,
	PORT_OUTPUT
} GooEnginePortType;

static guint
goo_engine_get_buffer_len (GooEngine* self, GooPort* port)
{
	g_assert (GOO_IS_ENGINE (self));
	g_assert (GOO_IS_PORT (port));

	if (self->vopstream == NULL)
	{
		return port->definition->nBufferSize;
	}

	static guint first_value = 0;
	static guint second_value = 0;
	guint value_type;
	guint timestamp;
	guint nread;
	guint bytes_to_read;

	first_value = second_value;

	nread = fscanf (self->vopstream, "%d %d %d\n",
			&second_value, &value_type, &timestamp);

	/* If we are not on EOF and we can't read 3 values then the VOP is
	 * not a valid one */
	g_assert (nread == 3 || feof (self->vopstream));

	if (feof (self->vopstream))
	{
		bytes_to_read = 0;
	}
	else
	{
		bytes_to_read = second_value - first_value;
	}

	return bytes_to_read;

}

static void
goo_engine_inport_cb (GooPort* port, OMX_BUFFERHEADERTYPE* buffer,
		      gpointer data)
{
	g_assert (GOO_IS_PORT (port));
	g_assert (buffer != NULL);
	g_assert (GOO_IS_COMPONENT (data));

	GooComponent* component = GOO_COMPONENT (data);
	GooEngine* self = GOO_ENGINE (
		g_object_get_data (G_OBJECT (component), "engine")
		);

	g_assert (self->instream != NULL);

	guint r;
	guint read_bytes;

	read_bytes = goo_engine_get_buffer_len (self, port);

	r = fread (buffer->pBuffer, 1, read_bytes, self->instream);

	GOO_OBJECT_DEBUG (self, "%d bytes read", r);

	if (read_bytes == 0 || feof (self->instream))
	{
		/* set EOS flag */
		buffer->nFlags |= OMX_BUFFERFLAG_EOS;

		if (self->eosevent == TRUE)
		{
			goo_component_set_done (self->component);
		}
	}

	buffer->nFilledLen = (r >= 0) ? r : 0;

	return;
}

static void
goo_engine_outport_cb (GooPort* port, OMX_BUFFERHEADERTYPE* buffer,
		       gpointer data)
{
	g_assert (GOO_IS_PORT (port));
	g_assert (buffer != NULL);
	g_assert (GOO_IS_COMPONENT (data));

	GooComponent* component = GOO_COMPONENT (data);
	GooEngine* self = GOO_ENGINE (
		g_object_get_data (G_OBJECT (component), "engine")
		);

	g_assert (self->outstream != NULL);

	if (buffer->nFilledLen <= 0 &&
	    (buffer->nFlags & OMX_BUFFERFLAG_EOS) != 0x1)
	{
		GOO_OBJECT_ERROR (self, "Empty buffer received!!");
	}

	if (buffer->nFilledLen > 0)
	{
		GOO_OBJECT_DEBUG (self, "%d bytes written",
				  buffer->nFilledLen);
		fwrite (buffer->pBuffer, 1, buffer->nFilledLen,
			self->outstream);
		/* fflush (self->outfile); */
	}

	/* we count the empty buffer only if it have de EOS flag */
	if ((buffer->nFilledLen > 0) ||
	    ((buffer->nFlags & OMX_BUFFERFLAG_EOS) == 0x1 &&
	     buffer->nFilledLen == 0))
	{
		g_atomic_int_inc (&self->outcount);
	}

	/* if we assigned the number of buffer to process */
	if (self->numbuffers != 0 && self->outcount == self->numbuffers)
	{
		goo_port_set_eos (port);
	}

	if ((buffer->nFlags & OMX_BUFFERFLAG_EOS) == 0x1 ||
	    goo_port_is_eos (port))
	{
		goo_component_set_done (self->component);
	}

	goo_component_release_buffer (component, buffer);

	return;
}

static GooEngineType
goo_engine_find_type (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));

	if (self->inport != NULL && self->outport == NULL)
	{
		GOO_OBJECT_DEBUG (self, "sink");
		return GOO_ENGINE_SINK;
	}
	else if (self->inport == NULL && self->outport != NULL)
	{
		GOO_OBJECT_DEBUG (self, "src");
		return GOO_ENGINE_SRC;
	}
	else if (self->inport != NULL && self->outport != NULL)
	{
		GOO_OBJECT_DEBUG (self, "filter");
		return GOO_ENGINE_FILTER;
	}

	g_print ("Can't find engine type");

	return GOO_ENGINE_UNKNOWN;
}

static GooPort*
goo_engine_get_port (GooComponent* component, GooEnginePortType porttype)
{
	g_assert (GOO_IS_COMPONENT (component));

	GooIterator* iter = NULL;
	GooPort* port = NULL;

	if (porttype == PORT_INPUT)
	{
		iter = goo_component_iterate_input_ports (component);
	}
	else if (porttype == PORT_OUTPUT)
	{
		iter = goo_component_iterate_output_ports (component);
	}
	else
	{
		g_assert_not_reached ();
	}

	g_assert (iter != NULL);

	while (!goo_iterator_is_done (iter))
	{
		port = GOO_PORT (goo_iterator_get_current (iter));
		GOO_OBJECT_INFO (port, "");

		if (goo_port_is_enabled (port))
		{
			if (!goo_port_is_tunneled (port))
			{
				GOO_OBJECT_INFO (component, "%s = %s",
						 (porttype == PORT_INPUT) ?
						 "input" : "output",
						 goo_object_get_name
						 (GOO_OBJECT (port)));

				/* we only bind the first untunneled port */
				g_object_unref (iter);
				return port;
			}
			else
			{
				GooComponent* peer = NULL;
				peer = goo_component_get_peer_component
					(component, port);

				if (peer != NULL)
				{
					GooPort* pport = NULL;
					pport = goo_engine_get_port
						(peer, porttype);
					g_object_unref (peer);
					if (pport != NULL)
					{
						g_object_unref (port);
						g_object_unref (iter);

						GOO_OBJECT_INFO
							(component, "%s = %s",
							 (porttype == PORT_INPUT) ?
							 "input" : "output",
							 goo_object_get_name
							 (GOO_OBJECT (port)));

						return pport;
					}
				}
			}
		}

		g_object_unref (port);
		goo_iterator_next (iter);
	}

	g_object_unref (iter);
	return NULL;
}

/**
 * NOTE: A component in the engine can have one and only one input and
 * output port, which are the first enabled and not tunneled in the iterator
 **/
static void
goo_engine_setup_component (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));

	GOO_OBJECT_DEBUG (self, "setting up");

	g_object_set_data (G_OBJECT (self->component), "engine", self);

	g_assert (self->inport == NULL);
	{
		self->inport = goo_engine_get_port (self->component,
						    PORT_INPUT);
	}

	g_assert (self->outport == NULL);
	{
		self->outport = goo_engine_get_port (self->component,
						     PORT_OUTPUT);
	}

	self->enginetype = goo_engine_find_type (self);

	if (self->enginetype == GOO_ENGINE_FILTER)
	{
		GOO_OBJECT_DEBUG (self, "setting output callback");
		goo_port_set_process_buffer_function
			(self->outport, goo_engine_outport_cb);
	}


	return;
}

static void
goo_engine_set_instream (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));

	struct stat buf;

	if (self->infile != NULL)
	{
		g_assert (self->instream == NULL);
		g_assert (stat (self->infile, &buf) == 0);
		g_assert (buf.st_size > 0);

		self->instream = fopen (self->infile, "r");

		g_assert (self->instream != NULL);
	}

	return;
}

static void
goo_engine_set_outstream (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));

	if (self->outfile != NULL)
	{
		g_assert (self->outstream == NULL);

		self->outstream = fopen (self->outfile, "w");

		g_assert (self->outstream != NULL);
	}

	return;
}

static void
goo_engine_set_vop (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));

	if (self->vopparser == TRUE)
	{
		g_assert (self->vopfile == NULL);
		g_assert (self->vopstream == NULL);

		gchar *fn = NULL, *fn1 = NULL, *path = NULL;

		fn = g_path_get_basename (self->infile);
		fn1 = strchr (fn, '.');
		fn1[0] = '\0';

		path = g_path_get_dirname (self->infile);

		self->vopfile = g_strdup_printf ("%s/%s.vop", path, fn);
		GOO_OBJECT_INFO (self, "vopfile = %s", self->vopfile);

		g_free (fn);
		g_free (path);

		struct stat buf;
		if (self->vopfile != NULL)
		{
			g_assert (stat (self->vopfile, &buf) == 0);
			g_assert (buf.st_size > 0);

			self->vopstream = fopen (self->vopfile, "r");

			g_assert (self->vopstream != NULL);
		}
		else
		{
			self->vopparser = FALSE;
		}
	}
}

static void
goo_engine_init (GooEngine* self)
{
	self->component = NULL;

	self->inport = NULL;
	self->instream = NULL;
	self->infile = NULL;

	self->outport = NULL;
	self->outstream = NULL;
	self->outfile = NULL;

	self->incount = 0;
	self->outcount = 0;

	self->eosevent = FALSE;

	self->vopparser = FALSE;
	self->vopfile = NULL;
	self->vopstream = NULL;

	self->enginetype = GOO_ENGINE_UNKNOWN;

	goo_object_set_name (GOO_OBJECT (self), "engine");

	return;
}

static void
goo_engine_dispose (GObject* object)
{
	g_assert (GOO_IS_ENGINE (object));
	GooEngine* self = GOO_ENGINE (object);

	(*G_OBJECT_CLASS (goo_engine_parent_class)->dispose) (object);

	if (G_LIKELY (self->inport))
	{
		g_object_unref (G_OBJECT (self->inport));
	}

	if (G_LIKELY (self->outport))
	{
		g_object_unref (G_OBJECT (self->outport));
	}

	if (G_LIKELY (self->component))
	{
		g_object_unref (G_OBJECT (self->component));
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static void
goo_engine_finalize (GObject* object)
{
	g_assert (GOO_IS_ENGINE (object));
	GooEngine* self = GOO_ENGINE (object);

	if (G_LIKELY (self->infile != NULL))
	{
		g_free (self->infile);
		self->infile = NULL;
	}

	if (G_LIKELY (self->outfile != NULL))
	{
		g_free (self->outfile);
		self->outfile = NULL;
	}

	if (G_LIKELY (self->vopfile != NULL))
	{
		g_free (self->vopfile);
		self->vopfile = NULL;
	}

	if (G_LIKELY (self->instream != NULL))
	{
		fclose (self->instream);
		self->instream = NULL;
	}

	if (G_LIKELY (self->outstream != NULL))
	{
		fflush (self->outstream);
		fclose (self->outstream);
		self->outstream = NULL;
	}

	if (G_LIKELY (self->vopstream != NULL))
	{
		fclose (self->vopstream);
		self->vopstream = NULL;
	}

	(*G_OBJECT_CLASS (goo_engine_parent_class)->finalize) (object);

	return;
}

static void
goo_engine_set_property (GObject* object, guint prop_id,
			 const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_ENGINE (object));
	GooEngine* self = GOO_ENGINE (object);

	switch (prop_id)
	{
	case PROP_COMPONENT:
	{
		GooComponent* component =
			GOO_COMPONENT (g_value_get_object (value));
		g_assert (component != NULL);
		g_assert (self->component == NULL);
		self->component = g_object_ref (component);
		goo_engine_setup_component (self);
	}
	break;
	case PROP_INFILE:
		self->infile = g_value_dup_string (value);
		goo_engine_set_instream (self);
		break;
	case PROP_OUTFILE:
		self->outfile = g_value_dup_string (value);
		goo_engine_set_outstream (self);
		break;
	case PROP_NUMBUFFERS:
		self->numbuffers = g_value_get_uint (value);
		break;
	case PROP_EOSEVENT:
		self->eosevent = g_value_get_boolean (value);
		break;
	case PROP_VOPFILE:
		self->vopparser = g_value_get_boolean (value);
		goo_engine_set_vop (self);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}
}

static void
goo_engine_get_property (GObject* object, guint prop_id,
			 GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_ENGINE (object));
	GooEngine* self = GOO_ENGINE (object);

	switch (prop_id)
	{
	case PROP_NUMBUFFERS:
		g_value_set_uint (value, self->numbuffers);
		break;
	case PROP_EOSEVENT:
		g_value_set_boolean (value, self->eosevent);
		break;
	case PROP_VOPFILE:
		g_value_set_boolean (value, self->vopparser);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}
}

static void
goo_engine_class_init (GooEngineClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->dispose = goo_engine_dispose;
	g_klass->finalize = goo_engine_finalize;
	g_klass->set_property = goo_engine_set_property;
	g_klass->get_property = goo_engine_get_property;

	GParamSpec* spec;
	spec = g_param_spec_object ("component", "Component",
				    "The compoment to execute",
				    GOO_TYPE_COMPONENT, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_COMPONENT, spec);

	spec = g_param_spec_string ("infile", "In filename",
				    "The input filename", NULL,
				    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (g_klass, PROP_INFILE, spec);

	spec = g_param_spec_string ("outfile", "Out filename",
				    "The output filename", NULL,
				    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (g_klass, PROP_OUTFILE, spec);

	spec = g_param_spec_uint ("num-buffers", "Number of buffers",
				  "Number of buffers to process",
				  0, G_MAXUINT, 0, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_NUMBUFFERS, spec);

	spec = g_param_spec_boolean ("eosevent", "EOS event",
				     "Use the EOS event to finish",
				     FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_EOSEVENT, spec);

	spec = g_param_spec_boolean ("vopparser", "VOP parsing",
				     "Use a VOP file to supply frames",
				     FALSE,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (g_klass, PROP_VOPFILE, spec);

	return;
}

/**
 * goo_engine_new:
 * @component: An #GooComponent instance in Executing state
 * @infile: the input file filename, may be %NULL
 * @outfile: the output file filename, may be %NULL
 *
 * Creates a new #GooEngine instance
 *
 * Return value: a #GooEngine instance
 **/
GooEngine*
goo_engine_new (GooComponent* component, gchar* infile, gchar* outfile)
{
	g_assert (GOO_IS_COMPONENT (component));

	GOO_DEBUG ("creaing an engine");
	GooEngine *self = g_object_new (GOO_TYPE_ENGINE,
					"component", component,
					"infile", infile,
					"outfile", outfile,
					NULL);
	return self;
}

/**
 * goo_engine_new_vop:
 * @component: An #GooComponent instance in Executing state
 * @infile: the input file filename, may be %NULL
 * @outfile: the output file filename, may be %NULL
 * @vopparser: The vop file which supplies frames
 * Creates a new #GooEngine instance using VOP parsing
 *
 * Return value: a #GooEngine instance
 **/
GooEngine*
goo_engine_new_vop (GooComponent* component, gchar* infile, gchar* outfile, gboolean vopparser)
{
	g_assert (GOO_IS_COMPONENT (component));

	GooEngine *self = g_object_new (GOO_TYPE_ENGINE,
					"component", component,
					"infile", infile,
					"outfile", outfile,
					"vopparser", vopparser,
					NULL);
	return self;
}


static void
goo_engine_process_output (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));
	g_assert (GOO_IS_COMPONENT (self->component));

	g_return_if_fail (G_LIKELY (self->outport));

	if (goo_port_is_tunneled (self->outport) ||
	    goo_port_is_eos (self->outport))
	{
		return;
	}

	OMX_BUFFERHEADERTYPE* buffer = NULL;

	GOO_OBJECT_DEBUG (self, "grabbing..");
	buffer = goo_port_grab_buffer (self->outport);
	GOO_OBJECT_DEBUG (self, "popt output buffer: 0x%x", buffer);

	goo_engine_outport_cb (self->outport, buffer, self->component);

	/* goo_port_process_buffer (self->outport, buffer, self->component); */
	/* goo_component_release_buffer (self->component, buffer); */

	return;
}

static void
goo_engine_process_input (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));
	g_assert (GOO_IS_COMPONENT (self->component));

	g_return_if_fail (G_LIKELY (self->inport));

	if (goo_port_is_tunneled (self->inport) ||
	    goo_port_is_eos (self->inport))
	{
		return;
	}

	OMX_BUFFERHEADERTYPE* buffer = NULL;

	buffer = goo_port_grab_buffer (self->inport);

	GOO_OBJECT_DEBUG (self, "popt input buffer: 0x%x", buffer);

	goo_engine_inport_cb (self->inport, buffer, self->component);
	/* goo_port_process_buffer (self->inport, buffer, self); */

	g_atomic_int_inc (&self->incount);

	goo_component_release_buffer (self->component, buffer);

	return;
}

/** not used ! */
#if 0
static void
goo_engine_chain (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));
	g_assert (GOO_IS_COMPONENT (self->component));

	gboolean input_processed = FALSE;
	OMX_BUFFERHEADERTYPE* buffer = NULL;

	while (!input_processed && !goo_component_is_done (self->component))
	{
		/* OUTPUT PROCESSING */
		if (G_LIKELY (self->outport) &&
		    !goo_port_is_tunneled (self->outport) &&
		    !goo_port_is_eos (self->outport))
		{
			while ((buffer =
				goo_port_try_grab_buffer (self->outport))
			       != NULL)
			{
				GOO_OBJECT_DEBUG (self,
						  "popt output buffer: 0x%x",
						  buffer);

				goo_port_process_buffer (self->outport,
							 buffer,
							 self->component);

				goo_component_release_buffer (self->component,
							      buffer);
			}

		}

		/* INPUT PROCESSING */
		if (G_LIKELY (self->inport) &&
		    !goo_port_is_tunneled (self->inport) &&
		    !goo_port_is_eos (self->inport))
		{

			if ((buffer = goo_port_try_grab_buffer (self->inport))
			    != NULL)
			{
				GOO_OBJECT_DEBUG (self,
						  "popt input buffer: 0x%x",
						  buffer);
				goo_port_process_buffer (self->inport, buffer,
							 self);
				goo_component_release_buffer (self->component,
							      buffer);
				input_processed = TRUE;
			}
		}
	}

	return;
}
#endif

/**
 * goo_engine_play:
 * @self: An #GooEngine instance
 *
 * Interchange buffers
 **/
void
goo_engine_play (GooEngine* self)
{
	g_assert (GOO_IS_ENGINE (self));
	g_assert (GOO_IS_COMPONENT (self->component));
	g_assert (self->component->cur_state == OMX_StateExecuting);

	GOO_OBJECT_DEBUG (self, "playing");

	if (self->enginetype == GOO_ENGINE_FILTER)
	{
		/* a filter must has an input and an output port */
		g_assert (self->outstream != NULL && self->instream != NULL);
	}
	else if (self->enginetype == GOO_ENGINE_SINK)
	{
		/* a sink must has an input port */
		g_assert (self->instream != NULL);
		// g_assert (self->outstream == NULL && self->instream != NULL);
	}
	else if (self->enginetype == GOO_ENGINE_SRC)
	{
		/* a src must has an output port */
		g_assert (self->outstream != NULL);
		// g_assert  (self->outstream != NULL && self->instream == NULL);
	}
	else
	{
		g_assert (self->enginetype != GOO_ENGINE_UNKNOWN);
	}

	if (self->enginetype == GOO_ENGINE_SINK ||
	    self->enginetype == GOO_ENGINE_FILTER)
	{
		GOO_OBJECT_INFO (self, "inport loop");
		while (!goo_port_is_eos (self->inport))
		{
			goo_engine_process_input (self);
		}
	}
	else if (self->enginetype == GOO_ENGINE_SRC)
	{
		GOO_OBJECT_INFO (self, "outport loop");
		while (!goo_port_is_eos (self->outport))
		{
			goo_engine_process_output (self);
		}
	}

	GOO_OBJECT_INFO (self, "Waiting for done...");
	goo_component_wait_for_done (self->component);

	g_print ("\tInput buffers count: %d\n", self->incount);
	g_print ("\tOutput buffers count: %d\n", self->outcount);

	return;
}
