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

#include <goo-port.h>
#include <goo-iterator.h>
#include <goo-list.h>
#include <goo-port-iterator.h>
#include <goo-port-list.h>
#include <goo-utils.h>

static guint
goo_port_list_get_length (GooList* self)
{
        g_assert (GOO_IS_PORT_LIST (self));
        GooPortList* me = GOO_PORT_LIST (self);
        guint retval = 0;

        g_mutex_lock (me->list_lock);
        retval = me->first ? g_list_length (me->first) : 0;
        g_mutex_unlock (me->list_lock);

        return retval;
}

static void
goo_port_list_prepend (GooList* self, GooObject* item)
{
        g_assert (GOO_IS_PORT_LIST (self));
        g_assert (GOO_IS_PORT (item));
        GooPortList* me = GOO_PORT_LIST (self);
        GooPort *port = GOO_PORT (item);

        g_mutex_lock (me->list_lock);
        g_object_ref (G_OBJECT (port));
        me->first = g_list_prepend (me->first, port);
        g_mutex_unlock (me->list_lock);

        return;
}

static void
goo_port_list_append (GooList* self, GooObject* item)
{
        g_assert (GOO_IS_PORT_LIST (self));
        g_assert (GOO_IS_PORT (item));
        GooPortList* me = GOO_PORT_LIST (self);
        GooPort *port = GOO_PORT (item);

        g_mutex_lock (me->list_lock);
        g_object_ref (G_OBJECT (port));
        me->first = g_list_append (me->first, port);
        g_mutex_unlock (me->list_lock);

        return;
}

static void
goo_port_list_remove (GooList* self, GooObject* item)
{
        g_assert (GOO_IS_PORT_LIST (self));
        g_assert (GOO_IS_PORT (item));
        GooPortList* me = GOO_PORT_LIST (self);
        GooPort *port = GOO_PORT (item);

        g_mutex_lock (me->list_lock);
        GList *l = NULL;
        for (l = me->first; l != NULL; l = l->next)
        {
                if (port == GOO_PORT (l->data))
                {
                        g_object_unref (G_OBJECT (port));
                        break;
                }
        }
        g_mutex_unlock (me->list_lock);
}

static GooIterator*
goo_port_list_create_iterator (GooList* self)
{
        g_assert (GOO_IS_PORT_LIST (self));
        GooPortList* me = GOO_PORT_LIST (self);

        return GOO_ITERATOR (goo_port_iterator_new (me));
}

static GooList*
goo_port_list_copy (GooList* self)
{
        g_assert (GOO_IS_PORT_LIST (self));
        GooPortList* me = GOO_PORT_LIST (self);
        GooPortList* copy = g_object_new (GOO_TYPE_PORT_LIST, NULL);

        g_mutex_lock (me->list_lock);
        GList *list_copy = g_list_copy (me->first);
        g_list_foreach (list_copy, (GFunc) g_object_ref, NULL);
        copy->first = list_copy;
        g_mutex_unlock (me->list_lock);

        return GOO_LIST (copy);
}

static void
goo_port_list_foreach (GooList* self, GFunc func, gpointer user_data)
{
        g_assert (GOO_IS_PORT_LIST (self));
        GooPortList* me = GOO_PORT_LIST (self);

        g_mutex_lock (me->list_lock);
        g_list_foreach (me->first, func, user_data);
        g_mutex_unlock (me->list_lock);

        return;
}

static void
goo_list_init (GooListIface* klass)
{
        klass->get_length_func = goo_port_list_get_length;
        klass->prepend_func = goo_port_list_prepend;
        klass->append_func = goo_port_list_append;
        klass->remove_func = goo_port_list_remove;
        klass->create_iterator_func = goo_port_list_create_iterator;
        klass->copy_func = goo_port_list_copy;
        klass->foreach_func = goo_port_list_foreach;
}

G_DEFINE_TYPE_EXTENDED (GooPortList, goo_port_list, G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (GOO_TYPE_LIST, goo_list_init))

static void
destroy_ports (gpointer port, gpointer user_data)
{
        if (port && G_IS_OBJECT (port))
        {
                GOO_OBJECT_DEBUG (GOO_PORT (port), "unref %d -> %d",
                                  G_OBJECT (port)->ref_count,
                                  G_OBJECT (port)->ref_count - 1);
                g_object_unref (G_OBJECT (port));
        }

        return;
}

static void
goo_port_list_finalize (GObject* object)
{
        GooPortList* self = GOO_PORT_LIST (object);

        g_mutex_lock (self->list_lock);
        if (self->first)
        {
                g_list_foreach (self->first, destroy_ports, NULL);
                g_list_free (self->first);
                self->first = NULL;
        }
        g_mutex_unlock (self->list_lock);

        g_mutex_free (self->list_lock);
        self->list_lock = NULL;

        G_OBJECT_CLASS (goo_port_list_parent_class)->finalize (object);

        return;
}

static void
goo_port_list_class_init (GooPortListClass *klass)
{
        GObjectClass* object_class;

        object_class = (GObjectClass*) klass;

        object_class->finalize = goo_port_list_finalize;

        return;
}

static void
goo_port_list_init (GooPortList *self)
{
        self->list_lock = g_mutex_new ();
        self->first = NULL;
}

GooList*
goo_port_list_new (void)
{
        return GOO_LIST (g_object_new (GOO_TYPE_PORT_LIST, NULL));
}
