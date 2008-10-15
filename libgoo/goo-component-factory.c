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
 * SECTION:goo-component-factory
 * @short_description: It is the base class for all the OMX component factories
 * @see_also: #GooComponent
 *
 * The #GooComponentFactory is the base class for all the differents OMX
 * component factories for every OMX implementation supported by libgoo.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-component-factory.h>
#include <goo-component-list.h>
#include <goo-log.h>
#include <goo-utils.h>

static GooComponentFactory *singleton = NULL;

/**
 * goo_component_factory_get_component:
 * @self: the #GooComponentFactory instance
 * @type: an enum of the object to create
 *
 * Create a new #GooComponent instance. The returned instance must be
 * unreferenced after use.
 *
 * Return value: a #GooComponent instance
 **/
GooComponent*
goo_component_factory_get_component (GooComponentFactory *self, guint type)
{
        g_assert (GOO_IS_COMPONENT_FACTORY (self));
        g_assert (GOO_COMPONENT_FACTORY_GET_CLASS (self)->get_component_func);

        GooComponent* retval = NULL;
        retval = GOO_COMPONENT_FACTORY_GET_CLASS (self)->get_component_func (self, type);
        g_assert (retval != NULL);

        return retval;
}

/**
 * goo_component_factory_add_component:
 * @self: The #GooComponentFactory instance
 * @component: The #GooComponent instance you want to add to the factory
 *
 * The factory maintains a list of components created by itself. This method
 * add component to this list. This function is intented to be used only by
 * the derived component factories.
 */
void
goo_component_factory_add_component (GooComponentFactory* self,
				     GooComponent* component)
{
	g_assert (GOO_IS_COMPONENT_FACTORY (self));
	g_assert (GOO_IS_COMPONENT (component));

	goo_list_append (self->components, GOO_OBJECT (component));
	goo_object_set_owner (GOO_OBJECT (component), GOO_OBJECT (self));

	GOO_OBJECT_DEBUG (component, "Refing component %d",
			  G_OBJECT (component)->ref_count);

	return;
}

G_DEFINE_TYPE (GooComponentFactory, goo_component_factory, GOO_TYPE_OBJECT)

static GObject*
goo_component_factory_constructor (GType type, guint n_construct_params,
				   GObjectConstructParam *construct_params)
{
	GObject *object = NULL;

	if (!singleton) {
		GOO_DEBUG ("Creating the factory singleton");
		object = G_OBJECT_CLASS (goo_component_factory_parent_class)->constructor (type, n_construct_params, construct_params);
		singleton = GOO_COMPONENT_FACTORY (object);
		goo_object_set_name (GOO_OBJECT (singleton), "factory0");
	} else {
		GOO_DEBUG ("Returning a reference to factory singleton");
		object = g_object_ref (G_OBJECT (singleton));
		object = G_OBJECT (singleton);
		g_object_freeze_notify (G_OBJECT (singleton));
	}

	return object;
}

static void
goo_component_factory_init (GooComponentFactory *self)
{
	self->components = goo_component_list_new ();

	if (!g_thread_supported ())
	{
		GOO_OBJECT_DEBUG (self, "Starting thread subsystem");
		g_thread_init (NULL);
	}

	GOO_INFO ("OMX_Init");
	GOO_RUN (
		OMX_Init ()
		);

	return;
}

static void
goo_component_factory_dispose (GObject *object)
{
	GooComponentFactory *self = GOO_COMPONENT_FACTORY (object);

	GOO_OBJECT_DEBUG (self, "");

	(*G_OBJECT_CLASS (goo_component_factory_parent_class)->dispose) (object);

	g_object_unref (G_OBJECT (self->components));

	return;
}

static void
goo_component_factory_finalize (GObject* object)
{
	GooComponentFactory* self = GOO_COMPONENT_FACTORY (object);

	GOO_OBJECT_INFO (self, "OMX_Deinit");
	GOO_RUN (
		OMX_Deinit ()
		);

	singleton = NULL;

	(*G_OBJECT_CLASS (goo_component_factory_parent_class)->finalize) (object);
	return;
}

static void
goo_component_factory_class_init (GooComponentFactoryClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->finalize = goo_component_factory_finalize;
	g_klass->dispose = goo_component_factory_dispose;
	g_klass->constructor = goo_component_factory_constructor;

	return;
}
