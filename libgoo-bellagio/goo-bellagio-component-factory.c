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

#include <goo-bellagio-component-factory.h>
#include <goo-utils.h>
#include <goo-component-list.h>

#include <goo-bellagio-mp3dec.h>
/* #include <goo-bellagio-mpeg4dec.h> */

struct _component_entry
{
	gchar* name;
	guint id;
	GType (*type) (void);
	guint count;
};

static struct _component_entry _components[] =
{
	{ "mp3dec", GOO_BELLAGIO_MP3_DECODER, goo_bellagio_mp3dec_get_type, 0 },
	/* { "mpeg4dec", GOO_BELLAGIO_MPEG4_DECODER, goo_bellagio_mpeg4dec_get_type, 0 }, */
	{ NULL, 0 },
};

static GooComponent*
goo_bellagio_component_factory_get_component (GooComponentFactory *self,
					      guint type)
{
	GOO_OBJECT_DEBUG (self, "");

	struct _component_entry *my_components = _components;
	GooComponent *component = NULL;

	while ((*my_components).name)
	{
		if (type == (*my_components).id)
		{
			component = g_object_new (((*my_components).type) (),
						  NULL);
			break;
		}
		my_components++;
	}

	if (component == NULL)
	{
		GOO_OBJECT_ERROR (self,
				  "The specified component does not exist!");
		return NULL;
	}

	/* set the component name */
	{
		gchar* name = g_strdup_printf ("%s%d", (*my_components).name,
					       (*my_components).count);
		goo_object_set_name (GOO_OBJECT (component), name);
		g_free (name);
		(*my_components).count++;
	}

	if (goo_component_load (component) == TRUE)
	{
		goo_component_factory_add_component (self, component);
	}
	else
	{
		g_object_unref (G_OBJECT (component));
		return NULL;
	}

	return component;
}

G_DEFINE_TYPE (GooBellagioComponentFactory, goo_bellagio_component_factory,
	       GOO_TYPE_COMPONENT_FACTORY);

static void
goo_bellagio_component_factory_init (GooBellagioComponentFactory *self)
{
	return;
}

static void
goo_bellagio_component_factory_class_init (GooBellagioComponentFactoryClass *klass)
{
	GObjectClass *g_klass = G_OBJECT_CLASS (klass);
	GooComponentFactoryClass *f_klass =
		GOO_COMPONENT_FACTORY_CLASS (klass);

	f_klass->get_component_func = goo_bellagio_component_factory_get_component;

	return;
}

/**
 * goo_bellagio_component_factory_get_instance:
 *
 * Create a new instance of the #GooComponentFactory if no exist previously.
 * Returns the previously created otherwise.
 *
 * Return value: the #GooComponentFactory singleton instance
 **/
GooComponentFactory*
goo_bellagio_component_factory_get_instance (void)
{
        GooBellagioComponentFactory* self =
                g_object_new (GOO_TYPE_BELLAGIO_COMPONENT_FACTORY, NULL);

        return GOO_COMPONENT_FACTORY (self);
}
