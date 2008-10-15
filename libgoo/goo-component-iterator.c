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
#include <goo-iterator.h>
#include <goo-component-iterator.h>

static void
goo_component_iterator_next (GooIterator *self)
{
        g_assert (GOO_IS_COMPONENT_ITERATOR (self));
        GooComponentIterator* me = GOO_COMPONENT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->current || !me->list))
        {
                return;
        }

        g_mutex_lock (me->list->iterator_lock);
        me->current = g_list_next (me->current);
        g_mutex_unlock (me->list->iterator_lock);

        return;
}

static void
goo_component_iterator_prev (GooIterator *self)
{
        g_assert (GOO_IS_COMPONENT_ITERATOR (self));
        GooComponentIterator* me = GOO_COMPONENT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->current || !me->list))
        {
                return;
        }

        g_mutex_lock (me->list->iterator_lock);
        me->current = g_list_previous (me->current);
        g_mutex_unlock (me->list->iterator_lock);

        return;
}

static void
goo_component_iterator_first (GooIterator *self)
{
        g_assert (GOO_IS_COMPONENT_ITERATOR (self));
        GooComponentIterator* me = GOO_COMPONENT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->current || !me->list))
        {
                return;
        }

        g_mutex_lock (me->list->iterator_lock);
        me->current = me->list->first;
        g_mutex_unlock (me->list->iterator_lock);

        return;
}

static void
goo_component_iterator_nth (GooIterator *self, guint nth)
{
        g_assert (GOO_IS_COMPONENT_ITERATOR (self));
        GooComponentIterator* me = GOO_COMPONENT_ITERATOR (self);

        if (G_UNLIKELY (!me || !me->current || !me->list))
        {
                return;
        }

        g_mutex_lock (me->list->iterator_lock);
        me->current = g_list_nth (me->list->first, nth);
        g_mutex_unlock (me->list->iterator_lock);

        return;
}

static GooObject*
goo_component_iterator_get_current (GooIterator* self)
{
        g_assert (GOO_IS_COMPONENT_ITERATOR (self));
        GooComponentIterator* me = GOO_COMPONENT_ITERATOR (self);
        GooComponent *component = NULL;

        if (G_UNLIKELY (!me->current || !me->list))
        {
                return NULL;
        }

        g_mutex_lock (me->list->iterator_lock);
        component = (G_UNLIKELY (me->current)) ?
                GOO_COMPONENT (me->current->data) : NULL;
        g_mutex_unlock (me->list->iterator_lock);

        if (component)
                g_object_ref (G_OBJECT (component));

        return GOO_OBJECT (component);
}

static GooList*
goo_component_iterator_get_list (GooIterator* self)
{
        g_assert (GOO_IS_COMPONENT_ITERATOR (self));
        GooComponentIterator* me = GOO_COMPONENT_ITERATOR (self);

        if (G_LIKELY (!me->current || !me->list))
        {
                return NULL;
        }

        g_object_ref (G_OBJECT (me->list));

        return (GooList*) me->list ;
}

static gboolean
goo_component_iterator_is_done (GooIterator* self)
{
        g_assert (GOO_IS_COMPONENT_ITERATOR (self));
        GooComponentIterator* me = GOO_COMPONENT_ITERATOR (self);

        /** @todo check this assert */
        g_assert (G_LIKELY (!me->current || !me->list));

        return me->current == NULL;
}

static void
goo_iterator_init (GooIteratorIface *klass)
{
        klass->next_func = goo_component_iterator_next;
        klass->prev_func = goo_component_iterator_prev;
        klass->first_func = goo_component_iterator_first;
        klass->nth_func = goo_component_iterator_nth;
        klass->get_current_func = goo_component_iterator_get_current;
        klass->get_list_func = goo_component_iterator_get_list;
        klass->is_done_func = goo_component_iterator_is_done;
}

G_DEFINE_TYPE_EXTENDED (GooComponentIterator, goo_component_iterator,
                        G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (GOO_TYPE_ITERATOR,
                                               goo_iterator_init))
static void
goo_component_iterator_finalize (GObject *object)
{
        G_OBJECT_CLASS (goo_component_iterator_parent_class)->finalize (object);
        return;
}

static void
goo_component_iterator_class_init (GooComponentIteratorClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = goo_component_iterator_finalize;

        return;
}

static void
goo_component_iterator_init (GooComponentIterator *self)
{
        self->list = NULL;
        self->current = NULL;
}

void
goo_component_iterator_set_list (GooComponentIterator* self,
                                 GooComponentList* list)
{
        self->list = list;
        self->current = list->first;
}

GooIterator*
goo_component_iterator_new (GooComponentList *list)
{
        GooComponentIterator *self = g_object_new
                (GOO_TYPE_COMPONENT_ITERATOR, NULL);

        goo_component_iterator_set_list (self, list);

        return GOO_ITERATOR (self);
}
