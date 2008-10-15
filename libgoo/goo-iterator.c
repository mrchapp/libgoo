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

#include <goo-iterator.h>
#include <goo-list.h>

/**
 * goo_iterator_next:
 * @self: An #GooIterator instance
 *
 * Moves the iterator to the next node
 *
 **/
void
goo_iterator_next (GooIterator *self)
{
        g_assert (GOO_IS_ITERATOR (self));
        g_assert (GOO_ITERATOR_GET_IFACE (self)->next_func != NULL);

        GOO_ITERATOR_GET_IFACE (self)->next_func (self);

        return;
}

/**
 * goo_iterator_prev:
 * @self: An #GooIterator instance
 *
 * Moves the iterator to the previous node
 *
 **/
void
goo_iterator_prev (GooIterator *self)
{
        g_assert (GOO_IS_ITERATOR (self));
        g_assert (GOO_ITERATOR_GET_IFACE (self)->prev_func != NULL);

        GOO_ITERATOR_GET_IFACE (self)->prev_func (self);

        return;
}

/**
 * goo_iterator_first:
 * @self: An #GooIterator instance
 *
 * Moves the iterator to the first node
 *
 **/
void
goo_iterator_first (GooIterator *self)
{
        g_assert (GOO_IS_ITERATOR (self));
        g_assert (GOO_ITERATOR_GET_IFACE (self)->first_func != NULL);

        GOO_ITERATOR_GET_IFACE (self)->first_func (self);

        return;
}

/**
 * goo_iterator_nth:
 * @self: An #GooIterator instance
 * @nth: The nth position
 *
 * Moves the iterator to the nth node
 *
 **/
void
goo_iterator_nth (GooIterator *self, guint nth)
{
        g_assert (GOO_IS_ITERATOR (self));
        g_assert (GOO_ITERATOR_GET_IFACE (self)->nth_func != NULL);

        GOO_ITERATOR_GET_IFACE (self)->nth_func (self, nth);

        return;
}

/**
 * goo_iterator_get_current:
 * @self: An #GooIterator instance
 *
 * Does not move the iterator. Returns the objecto at the curren position. If
 * there's no current position, this method returns NULL. If not NULL, the
 * returned value must be unreferenced after use.
 *
 * Return value: the current object or NULL
 *
 **/
GooObject*
goo_iterator_get_current (GooIterator *self)
{
        GooObject *retval;

        g_assert (GOO_IS_ITERATOR (self));
        g_assert (GOO_ITERATOR_GET_IFACE (self)->get_current_func != NULL);

        retval = GOO_ITERATOR_GET_IFACE (self)->get_current_func (self);

        if (retval)
                g_assert (GOO_IS_OBJECT (retval));

        return retval;
}

/**
 * goo_iterator_is_done:
 * @self: An #GooIterator instance
 *
 * Does the iterator point to some valid list item? You can use this property
 * to make loops like:
 *
 * Example:
 * <informalexample><programlisting>
 * GooList *list = goo_simple_list_new ();
 * GooIterator *iter = goo_list_create_iterator (list);
 * while (!goo_iterator_is_done (iter))
 * {
 *    GooObject *cur = goo_iterator_get_current (iter);
 *    ...
 *    g_object_unref (G_OBJECT (cur));
 *    goo_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (list));
 * </programlisting></informalexample>
 *
 * Return value: TRUE if it points to a valid list item, FALSE otherwise
 *
 **/
gboolean
goo_iterator_is_done (GooIterator *self)
{
        g_assert (GOO_IS_ITERATOR (self));
        g_assert (GOO_ITERATOR_GET_IFACE (self)->is_done_func != NULL);

        return GOO_ITERATOR_GET_IFACE (self)->is_done_func (self);
}

/**
 * goo_iterator_get_list:
 * @self: An #GooIterator instance
 *
 * Does not move the iterator. Returns the list of which this iterator is an
 * iterator. The returned list object should be unreferenced after use.
 * Remember when using this property that lists shouldn't change while
 * iterating them.
 *
 * Return value: The #GooList instance being iterated
 *
 **/
GooList*
goo_iterator_get_list (GooIterator *self)
{
        GooList *retval = NULL;

        g_assert (GOO_IS_ITERATOR (self));
        g_assert (GOO_ITERATOR_GET_IFACE (self)->get_list_func != NULL);

        retval = GOO_ITERATOR_GET_IFACE (self)->get_list_func (self);

        g_assert (GOO_IS_LIST (retval));

        return retval;
}

static void
goo_iterator_base_init (gpointer g_class)
{
        static gboolean initialized = FALSE;

        if (!initialized)
        {
                initialized = TRUE;
        }
}

GType
goo_iterator_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo info =
                {
                        sizeof (GooIteratorIface),
                        goo_iterator_base_init,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        0,
                        0,
                        NULL,
                        NULL
                };
                type = g_type_register_static (G_TYPE_INTERFACE,
                                               "GooIterator", &info, 0);
        }

        return type;
}
