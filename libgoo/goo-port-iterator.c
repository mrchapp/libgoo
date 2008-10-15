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
#include <goo-port-iterator.h>

static void
goo_port_iterator_next (GooIterator *self)
{
        g_assert (GOO_IS_PORT_ITERATOR (self));
        GooPortIterator* me = GOO_PORT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->current || !me->list))
        {
                return;
        }

        g_mutex_lock (me->list->list_lock);
        me->current = g_list_next (me->current);
        g_mutex_unlock (me->list->list_lock);

        return;
}

static void
goo_port_iterator_prev (GooIterator *self)
{
        g_assert (GOO_IS_PORT_ITERATOR (self));
        GooPortIterator* me = GOO_PORT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->current || !me->list))
        {
                return;
        }

        g_mutex_lock (me->list->list_lock);
        me->current = g_list_previous (me->current);
        g_mutex_unlock (me->list->list_lock);

        return;
}

static void
goo_port_iterator_first (GooIterator *self)
{
        g_assert (GOO_IS_PORT_ITERATOR (self));
        GooPortIterator* me = GOO_PORT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->current || !me->list))
        {
                return;
        }

        g_mutex_lock (me->list->list_lock);
        me->current = me->list->first;
        g_mutex_unlock (me->list->list_lock);

        return;
}

static void
goo_port_iterator_nth (GooIterator *self, guint nth)
{
        g_assert (GOO_IS_PORT_ITERATOR (self));
        GooPortIterator* me = GOO_PORT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->current || !me->list))
        {
                return;
        }

        g_mutex_lock (me->list->list_lock);
        me->current = g_list_nth (me->list->first, nth);
        g_mutex_unlock (me->list->list_lock);

        return;
}

static GooObject*
goo_port_iterator_get_current (GooIterator* self)
{
        g_assert (GOO_IS_PORT_ITERATOR (self));
        GooPortIterator* me = GOO_PORT_ITERATOR (self);
        GooPort *port = NULL;

        if (G_UNLIKELY (!me->current || !me->list))
        {
                return NULL;
        }

        g_mutex_lock (me->list->list_lock);
        port = (G_UNLIKELY (me->current)) ?
                GOO_PORT (me->current->data) : NULL;
        g_mutex_unlock (me->list->list_lock);

        if (port)
                g_object_ref (G_OBJECT (port));

        return GOO_OBJECT (port);
}

static GooList*
goo_port_iterator_get_list (GooIterator* self)
{
        g_assert (GOO_IS_PORT_ITERATOR (self));
        GooPortIterator* me = GOO_PORT_ITERATOR (self);

        if (G_LIKELY (!me->current || !me->list))
        {
                return NULL;
        }

        g_object_ref (G_OBJECT (me->list));

        return (GooList*) me->list ;
}

static gboolean
goo_port_iterator_is_done (GooIterator* self)
{
        GooPortIterator* me = GOO_PORT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->list))
        {
                return TRUE;
        }

        return me->current == NULL;
}

static void
goo_iterator_init (GooIteratorIface* klass)
{
        klass->next_func = goo_port_iterator_next;
        klass->prev_func = goo_port_iterator_prev;
        klass->first_func = goo_port_iterator_first;
        klass->nth_func = goo_port_iterator_nth;
        klass->get_current_func = goo_port_iterator_get_current;
        klass->get_list_func = goo_port_iterator_get_list;
        klass->is_done_func = goo_port_iterator_is_done;
}

G_DEFINE_TYPE_EXTENDED (GooPortIterator, goo_port_iterator,
                        G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (GOO_TYPE_ITERATOR,
                                               goo_iterator_init))
static void
goo_port_iterator_finalize (GObject* object)
{
        G_OBJECT_CLASS (goo_port_iterator_parent_class)->finalize (object);
        return;
}

static void
goo_port_iterator_class_init (GooPortIteratorClass* klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = goo_port_iterator_finalize;

        return;
}

static void
goo_port_iterator_init (GooPortIterator* self)
{
        self->list = NULL;
        self->current = NULL;
}

void
goo_port_iterator_set_list (GooPortIterator* self, GooPortList* list)
{
        self->list = list;
        self->current = list->first;
}

GooIterator*
goo_port_iterator_new (GooPortList* list)
{
        GooPortIterator *self = g_object_new (GOO_TYPE_PORT_ITERATOR, NULL);

        goo_port_iterator_set_list (self, list);

        return GOO_ITERATOR (self);
}
