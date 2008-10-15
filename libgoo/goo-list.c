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

#include <goo-list.h>

/**
 * goo_list_get_length:
 * @self: An #GooList instance
 *
 * Return value: the length of the list
 *
 **/
guint
goo_list_get_length (GooList *self)
{
        g_assert (GOO_IS_LIST (self));
        g_assert (GOO_LIST_GET_IFACE (self)->get_length_func != NULL);

        return GOO_LIST_GET_IFACE (self)->get_length_func (self);
}

/**
 * goo_list_prepend:
 * @self: An #GooList instance
 * @item: the item to prepend
 *
 * Prepends an item to a list. You can only prepend items that inherit from the
 * GooObject base item. That's because the goo list infrastructure does
 * reference counting. It effectively means that indeed you can't use non
 * GooObject types in a goo list. But there's not a single situation where
 * you must do that. If you must store a non GooObject in a list, you shouldn't
 * use the goo infrastructure for this. Consider using a doubly linked list
 * or a pointer array or any other list-type available on your development
 * platform.
 *
 * However, goo lists can cope with any valid GooObject. Not just the
 * GooObjects implemented by the goo framework.
 *
 * All reference handling in goo is reference neutral. Also the lists. This
 * means that if your plan is to reparent the item to the list, that you should
 * take care of that by, after prepending or appending it, unreferencing it to
 * get rid of its initial reference. If you don't want to reparent, but you do
 * want to destroy your item once removed from the list, then you must
 * unreference your items twice. Note that reparenting is highly recommended
 * in most such cases (because it's a much cleaner way). However, if reparented
 * and the list itself gets destroyed, then the item will also get unreferenced.
 *
 * Reparenting indeed means reparenting. Okay? Loosing your parent reference
 * means loosing your reason of existance. So you'll get destroyed.
 *
 * Implementers: if you have to choose, make this one the fast one
 *
 **/
void
goo_list_prepend (GooList *self, GooObject* item)
{
        g_assert (GOO_IS_LIST (self));
        gint length = goo_list_get_length (self);
        g_assert (item);
        g_assert (GOO_IS_OBJECT (item));
        g_assert (GOO_LIST_GET_IFACE (self)->prepend_func != NULL);

        GOO_LIST_GET_IFACE (self)->prepend_func (self, item);

        g_assert (goo_list_get_length (self) == length + 1);

        return;
}

/**
 * goo_list_append:
 * @self: An #GooList instance
 * @item: the item to append
 *
 * Appends an item to a list. You can only append items that inherit from the
 * GooObject base item. That's because the goo list infrastructure does
 * reference counting. It effectively means that indeed you can't use non
 * GooObject types in a goo list. But there's not a single situation where
 * you must do that. If you must store a non GooObject in a list, you shouldn't
 * use the goo infrastructure for this. Consider using a doubly linked list
 * or a pointer array or any other list-type available on your development
 * platform.
 *
 * However, goo lists can cope with any valid GooObject. Not just the
 * GooObjects implemented by the goo framework.
 *
 * All reference handling in goo is reference neutral. Also the lists. This
 * means that if your plan is to reparent the item to the list, that you should
 * take care of that by, after prepending or appending it, unreferencing it to
 * get rid of its initial reference. If you don't want to reparent, but you do
 * want to destroy your item once removed from the list, then you must
 * unreference your items twice. Note that reparenting is highly recommended
 * in most such cases (because it's a much cleaner way). However, if reparented
 * and the list itself gets destroyed, then the item will also get unreferenced.
 *
 * Reparenting indeed means reparenting. Okay? Loosing your parent reference
 * means loosing your reason of existance. So you'll get destroyed.
 *
 * Implementers: if you have to choose, make the prepend one the fast one
 *
 **/
void
goo_list_append (GooList *self, GooObject* item)
{
        g_assert (GOO_IS_LIST (self));
        gint length = goo_list_get_length (self);
        g_assert (item);
        g_assert (GOO_IS_OBJECT (item));
        g_assert (GOO_LIST_GET_IFACE (self)->append_func != NULL);

        GOO_LIST_GET_IFACE (self)->append_func (self, item);

        g_assert (goo_list_get_length (self) == length + 1);

        return;
}

/**
 * goo_list_remove:
 * @self: An #GooList instance
 * @item: the item to remove
 *
 * Removes an item from a list.  Removing a item might invalidate all existing
 * iterators or put them in an unknown and unspecified state. You'll need to
 * recreate the iterator(s) if you remove an item to be certain.
 *
 * If you want to clear a list, consider using the goo_list_foreach or simply
 * destroy the list instance and construct a new one. If you want to remove
 * specific items from a list, consider using a second list. You should not
 * attempt to remove items from a list while an (any) iterator is active on the
 * same list.
 *
 * Example (removing even items):
 * <informalexample><programlisting>
 * GooList *toremovefrom = ...
 * GooList *removethese = goo_simple_list_new ();
 * GooIterator *iter = goo_list_create_iterator (toremovefrom);
 * int i = 0;
 * while (!goo_iterator_is_done (iter))
 * {
 *      if (i % 2 == 0)
 *      {
 *           GooObject *obj = goo_iterator_get_current (iter);
 *           goo_list_prepend (removethese, obj);
 *           g_object_unref (G_OBJECT (obj));
 *      }
 *      i++;
 *      goo_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * iter = goo_list_create_iterator (removethese);
 * while (!goo_iterator_is_done (iter))
 * {
 *      GooObject *obj = goo_iterator_get_current (iter);
 *      goo_list_remove (toremovefrom, obj);
 *      g_object_unref (G_OBJECT (obj));
 *      goo_iterator_next (iter);
 * }
 * g_object_unref (G_OBJECT (iter));
 * g_object_unref (G_OBJECT (removethese));
 * g_object_unref (G_OBJECT (toremovefrom));
 * </programlisting></informalexample>
 *
 * There's no guarantee whatsoever that existing iterators of @self will be
 * valid after this method returned.
 *
 * Note that if you didn't remove the initial reference when putting the item
 * in the list, this remove will not take of that initial reference either.
 *
 **/
void
goo_list_remove (GooList *self, GooObject* item)
{
        g_assert (GOO_IS_LIST (self));
        gint nl, length = goo_list_get_length (self);
        g_assert (item);
        g_assert (GOO_IS_OBJECT (item));
        g_assert (GOO_LIST_GET_IFACE (self)->remove_func != NULL);

        GOO_LIST_GET_IFACE (self)->remove_func (self, item);

        nl = goo_list_get_length (self);
        g_assert (nl == length || nl == length - 1);

        return;
}

/**
 * goo_list_create_iterator:
 * @self: An #GooList instance
 *
 * Creates a new iterator instance for the list. The initial position
 * of the iterator is the first element.
 *
 * An iterator is a position indicator for a list. It keeps the position
 * state of a list iteration. The list itself does not keep any position
 * information. Consuming multiple iterator instances makes it possible to
 * have multiple list iterations simultanously (i.e. multiple threads or in
 * in a loop that simultanously works with multiple position states in a
 * single list).
 *
 * Example:
 * <informalexample><programlisting>
 * GooList *list = goo_simple_list_new ();
 * GooIterator *iter1 = goo_list_create_iterator (list);
 * GooIterator *iter2 = goo_list_create_iterator (list);
 * while (!goo_iterator_is_done (iter1))
 * {
 *      while (!goo_iterator_is_done (iter2))
 *            goo_iterator_next (iter2);
 *      goo_iterator_next (iter1);
 * }
 * g_object_unref (G_OBJECT (iter1));
 * g_object_unref (G_OBJECT (iter2));
 * g_object_unref (G_OBJECT (list));
 * </programlisting></informalexample>
 *
 * The reason why the method isn't called get_iterator is because it's a
 * object creation method (a factory method). It's not a property. It
 * effectively creates a new instance of an iterator. The returned iterator
 * object should (therefore) be unreferenced after use.
 *
 * Implementers: For custom lists you must create a private iterator type and
 * return a new instance of it. You shouldn't make the internal API of that
 * type public.
 *
 * The developer will always only use the GooIterator interface API on
 * instances of your type. You must therefore return your private iterator
 * type, that implements GooIterator, here.
 *
 * Return value: A new iterator for the list @self
 *
 **/
GooIterator*
goo_list_create_iterator (GooList *self)
{
        GooIterator *retval = NULL;

        g_assert (GOO_IS_LIST (self));
        g_assert (GOO_LIST_GET_IFACE (self)->create_iterator_func != NULL);

        retval = GOO_LIST_GET_IFACE (self)->create_iterator_func (self);

        g_assert (GOO_IS_ITERATOR (retval));

        return retval;
}

/**
 * goo_list_foreach:
 * @self: An #GooList instance
 * @func: the function to call with each element's data.
 * @user_data: user data to pass to the function.
 *
 * Calls a function for each element in a #GooList. It will use an internal
 * iteration which you don't have to worry about.
 *
 * Example:
 * <informalexample><programlisting>
 * static void
 * list_foreach_item (GooHeader *header, gpointer user_data)
 * {
 *      g_print ("%s\n", goo_header_get_subject (header));
 * }
 * </programlisting></informalexample>
 *
 * <informalexample><programlisting>
 * GooFolder *folder = ...
 * GooList *headers = goo_simple_list_new ();
 * goo_folder_get_headers (folder, headers, FALSE);
 * goo_list_foreach (headers, list_foreach_item, NULL);
 * g_object_unref (G_OBJECT (list));
 * </programlisting></informalexample>
 *
 * The purpose of this method is to have a fast foreach iteration. Using this
 * is faster than inventing your own foreach loop using the is_done and next
 * methods. The order is guaranteed to be the first element first, the last
 * element last. If during the iteration you don't remove items, it's guaranteed
 * that all current items will be iterated.
 *
 * In the func implementation and during the foreach operation you shouldn't
 * append, remove nor prepend items to the list. In multithreaded environments
 * it's advisable to introduce a lock when using this functionality.
 *
 **/
void
goo_list_foreach (GooList *self, GFunc func, gpointer user_data)
{
        g_assert (GOO_IS_LIST (self));
        g_assert (func);
        g_assert (GOO_LIST_GET_IFACE (self)->foreach_func != NULL);

        GOO_LIST_GET_IFACE (self)->foreach_func (self, func, user_data);
        return;
}


/**
 * goo_list_copy:
 * @self: An #GooList instance
 *
 * Creates a shallow copy of the list. It doesn't copy the items. It,
 * however, creates a new list with new references to the same
 * items. The items will get an extra reference added for the new list
 * being their second parent, setting their reference count to for
 * example two. Which means that both lists (the original and the
 * copy) must be unreferenced after use.
 *
 * Return value: A copy of this list
 *
 **/
GooList*
goo_list_copy (GooList *self)
{
        GooList *retval = NULL;

        g_assert (GOO_IS_LIST (self));
        g_assert (GOO_LIST_GET_IFACE (self)->copy_func != NULL);

        retval = GOO_LIST_GET_IFACE (self)->copy_func (self);

        g_assert (GOO_IS_LIST (retval));

        return retval;
}

static void
goo_list_base_init (gpointer g_class)
{
        static gboolean initialized = FALSE;

        if (!initialized)
        {
                /* create interface signals here. */
                initialized = TRUE;
        }
}

GType
goo_list_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY(type == 0))
        {
                static const GTypeInfo info =
                {
                  sizeof (GooListIface),
                  goo_list_base_init,   /* base_init */
                  NULL,   /* base_finalize */
                  NULL,   /* class_init */
                  NULL,   /* class_finalize */
                  NULL,   /* class_data */
                  0,
                  0,      /* n_preallocs */
                  NULL,   /* instance_init */
                  NULL
                };
                type = g_type_register_static (G_TYPE_INTERFACE,
                                               "GooList", &info, 0);
        }

        return type;
}
