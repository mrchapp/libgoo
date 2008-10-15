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

#include <goo-component.h>
#include <goo-component-list.h>
#include <goo-component-iterator.h>
#include <goo-utils.h>

static guint
goo_component_list_get_length (GooList* self)
{
        g_assert (GOO_IS_COMPONENT_LIST (self));
        GooComponentList* me = GOO_COMPONENT_LIST (self);

        guint retval = 0;
        g_mutex_lock (me->iterator_lock);
        retval = me->first ? g_list_length (me->first) : 0;
        g_mutex_unlock (me->iterator_lock);

        return retval;
}

static void
goo_component_list_prepend (GooList* self, GooObject* item)
{
        g_assert (GOO_IS_COMPONENT_LIST (self));
        g_assert (GOO_IS_COMPONENT (item));

        GooComponentList* me = GOO_COMPONENT_LIST (self);
        GooComponent* component = GOO_COMPONENT (item);

        g_mutex_lock (me->iterator_lock);
        g_object_ref (G_OBJECT (component));
        me->first = g_list_prepend (me->first, component);
        g_mutex_unlock (me->iterator_lock);

        return;
}

static void
goo_component_list_append (GooList* self, GooObject* item)
{
        g_assert (GOO_IS_COMPONENT_LIST (self));
        g_assert (GOO_IS_COMPONENT (item));
        GooComponentList* me = GOO_COMPONENT_LIST (self);
        GooComponent *component = GOO_COMPONENT (item);

        g_mutex_lock (me->iterator_lock);
        g_object_ref (G_OBJECT (component));
        me->first = g_list_append (me->first, component);
        g_mutex_unlock (me->iterator_lock);

        return;
}

static void
goo_component_list_remove (GooList* self, GooObject* item)
{
        g_assert (GOO_IS_COMPONENT_LIST (self));
        g_assert (GOO_IS_COMPONENT (item));
        GooComponentList* me = GOO_COMPONENT_LIST (self);
        GooComponent *component = GOO_COMPONENT (item);

        g_mutex_lock (me->iterator_lock);
        GList *l = NULL;
        for (l = me->first; l != NULL; l = l->next)
        {
                if (component == GOO_COMPONENT (l->data))
                {
                        g_object_unref (G_OBJECT (component));
                        break;
                }
        }
        g_mutex_unlock (me->iterator_lock);
}

static GooIterator*
goo_component_list_create_iterator (GooList* self)
{
        g_assert (GOO_IS_COMPONENT_LIST (self));
        GooComponentList* me = GOO_COMPONENT_LIST (self);

        return GOO_ITERATOR (goo_component_iterator_new (me));
}

static GooList*
goo_component_list_copy (GooList* self)
{
        g_assert (GOO_IS_COMPONENT_LIST (self));
        GooComponentList* me = GOO_COMPONENT_LIST (self);
        GooComponentList* copy = g_object_new
                (GOO_TYPE_COMPONENT_LIST, NULL);

        g_mutex_lock (me->iterator_lock);
        GList *list_copy = g_list_copy (me->first);
        g_list_foreach (list_copy, (GFunc) g_object_ref, NULL);
        copy->first = list_copy;
        g_mutex_unlock (me->iterator_lock);

        return GOO_LIST (copy);
}

static void
goo_component_list_foreach (GooList* self, GFunc func, gpointer user_data)
{
        g_assert (GOO_IS_COMPONENT_LIST (self));
        GooComponentList* me = GOO_COMPONENT_LIST (self);

        g_mutex_lock (me->iterator_lock);
        g_list_foreach (me->first, func, user_data);
        g_mutex_unlock (me->iterator_lock);

        return;
}


static void
goo_list_init (GooListIface* klass)
{
        klass->get_length_func = goo_component_list_get_length;
        klass->prepend_func = goo_component_list_prepend;
        klass->append_func = goo_component_list_append;
        klass->remove_func = goo_component_list_remove;
        klass->create_iterator_func = goo_component_list_create_iterator;
        klass->copy_func = goo_component_list_copy;
        klass->foreach_func = goo_component_list_foreach;
}

G_DEFINE_TYPE_EXTENDED (GooComponentList, goo_component_list,
                        G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (GOO_TYPE_LIST, goo_list_init))

static void
destroy_components (gpointer component, gpointer user_data)
{
        if (component && G_IS_OBJECT (component))
        {
                GOO_OBJECT_DEBUG (component, "unref %d -> %d",
                                  G_OBJECT (component)->ref_count,
                                  G_OBJECT (component)->ref_count - 1);
                g_object_unref (G_OBJECT (component));
        }

        return;
}

static void
goo_component_list_finalize (GObject* object)
{
        GooComponentList *self = GOO_COMPONENT_LIST (object);

        g_mutex_lock (self->iterator_lock);
        if (self->first)
        {
                g_list_foreach (self->first, destroy_components, NULL);
                g_list_free (self->first);
                self->first = NULL;
        }
        g_mutex_unlock (self->iterator_lock);

        g_mutex_free (self->iterator_lock);
        self->iterator_lock = NULL;

        G_OBJECT_CLASS (goo_component_list_parent_class)->finalize (object);

        return;
}

static void
goo_component_list_class_init (GooComponentListClass* klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = goo_component_list_finalize;
}

static void
goo_component_list_init (GooComponentList* self)
{
        self->iterator_lock = g_mutex_new ();
        self->first = NULL;
}

GooList*
goo_component_list_new (void)
{
        return GOO_LIST (g_object_new (GOO_TYPE_COMPONENT_LIST, NULL));
}
