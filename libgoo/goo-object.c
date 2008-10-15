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

#include <goo-object.h>
#include <goo-utils.h>

enum
{
        PROP_O,
        PROP_NAME
};

G_DEFINE_TYPE (GooObject, goo_object, G_TYPE_OBJECT)

static void
goo_object_init (GooObject *self)
{
        self->name = NULL;
        self->owner = NULL;
        self->prefix = NULL;
	self->lock = g_mutex_new ();

        return;
}

static void
goo_object_finalize (GObject* object)
{
        GooObject* self = GOO_OBJECT (object);

        GOO_OBJECT_DEBUG (self, "unref = %d -> %d",
                          GOO_OBJECT_REFCOUNT (object),
                          GOO_OBJECT_REFCOUNT (object) - 1);

	if (G_LIKELY (self->lock))
	{
		g_mutex_free (self->lock);
		self->lock = NULL;
	}

        if (G_LIKELY (self->name))
        {
                g_free (self->name);
		self->name = NULL;
        }

        if (G_LIKELY (self->prefix))
        {
                g_free (self->prefix);
		self->prefix = NULL;
        }

        (*G_OBJECT_CLASS (goo_object_parent_class)->finalize) (object);

        return;
}

static void
goo_object_set_property (GObject *object, guint prop_id, const GValue *value,
                         GParamSpec *pspec)
{
        switch (prop_id)
        {
        case PROP_NAME:
        {
                if (G_LIKELY (GOO_OBJECT_NAME (object) != NULL))
                {
                        g_free (GOO_OBJECT (object)->name);
			GOO_OBJECT_NAME (object) = NULL;
                }

                GOO_OBJECT (object)->name = g_value_dup_string (value);

                break;
        }

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }

        return;
}

static void
goo_object_get_property (GObject *object, guint prop_id, GValue *value,
                         GParamSpec *pspec)
{
        switch (prop_id)
        {
        case PROP_NAME:
		g_value_set_string (value, GOO_OBJECT_NAME (object));
                break;

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }

        return;
}

static void
goo_object_class_init (GooObjectClass *klass)
{
        GObjectClass *g_klass = G_OBJECT_CLASS (klass);

        g_klass->finalize = goo_object_finalize;

        g_klass->set_property = goo_object_set_property;
        g_klass->get_property = goo_object_get_property;

        GParamSpec *pspec;

        pspec = g_param_spec_string ("name", "Name", "GOO object's name",
                                     NULL, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_NAME, pspec);

        return;
}

/**
 * goo_object_get_name:
 * @self: An #GooObject instance
 *
 * Returns a copy of the name of object. Caller should g_free() the return \
 * value after usage. For a nameless object, this returns NULL, which you
 * can safely g_free() as well.
 *
 * Return value: the name of object. g_free() after usage. MT safe.
 **/
gchar*
goo_object_get_name (GooObject *self)
{
        gchar *name = NULL;
        g_object_get (self, "name", &name, NULL);
        return name;
}

/**
 * goo_object_set_name:
 * @self: An #GooObject instance
 * @name: An #gchar with the object's name
 *
 * Set the name of object. This function makes a copy of the provided name,
 * so the caller retains ownership of the name it sent.
 **/
void
goo_object_set_name (GooObject *self, const gchar *name)
{
        g_object_set (self, "name", name, NULL);

	return;
}


/**
 * goo_object_set_owner:
 * @self: An #GooObject instance
 * @owner: The new #GooObject owner object
 *
 * Sets the owner of the object. The owner reference count will be
 * incremented.
 **/
void
goo_object_set_owner (GooObject* self, GooObject* owner)
{
        g_assert (GOO_IS_OBJECT (self));
        g_assert (GOO_IS_OBJECT (owner));

        self->owner = owner;

        if (G_LIKELY (GOO_OBJECT_NAME (owner) != NULL))
        {
                if (G_LIKELY (self->prefix))
                {
                        g_free (self->prefix);
                }

		self->prefix = g_strdup (GOO_OBJECT_NAME (owner));
        }

        return;
}

/**
 * goo_object_get_owner:
 * @self: An #GooObject instance
 *
 * Gets the object's owner. This functions increases the refcount of the
 * owner object so you should unref it after usage.
 *
 * Return value: The object's owner, this can be NULL if object has no parent.
 * unref owner after usage. MT safe.
 **/
GooObject*
goo_object_get_owner (GooObject* self)
{
        g_assert (GOO_IS_OBJECT (self));

        return g_object_ref (self->owner);
}
