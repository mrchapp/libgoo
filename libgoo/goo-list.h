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

#ifndef _GOO_LIST_H_
#define _GOO_LIST_H_

#include <goo-iterator.h>

G_BEGIN_DECLS

#define GOO_TYPE_LIST \
        (goo_list_get_type ())
#define GOO_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_LIST, GooList))
#define GOO_IS_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_LIST))
#define GOO_LIST_GET_IFACE(inst) \
        (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GOO_TYPE_LIST, GooListIface))

/* typedef struct _GooList GooList; */
typedef struct _GooListIface GooListIface;

struct _GooListIface
{
        GTypeInterface parent;

        guint (*get_length_func) (GooList *self);
        void (*prepend_func) (GooList *self, GooObject* item);
        void (*append_func) (GooList *self, GooObject* item);
        void (*remove_func) (GooList *self, GooObject* item);
        void (*foreach_func) (GooList *self, GFunc func, gpointer user_data);
        GooList* (*copy_func) (GooList *self);
        GooIterator* (*create_iterator_func) (GooList *self);
};

GType goo_list_get_type (void);

guint goo_list_get_length (GooList *self);
void goo_list_prepend (GooList *self, GooObject* item);
void goo_list_append (GooList *self, GooObject* item);
void goo_list_remove (GooList *self, GooObject* item);
void goo_list_foreach (GooList *self, GFunc func, gpointer user_data);
GooIterator* goo_list_create_iterator (GooList *self);
GooList* goo_list_copy (GooList *self);

G_END_DECLS

#endif /* _GOO_LIST_H_ */
