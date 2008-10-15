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

#ifndef _GOO_ITERATOR_H_
#define _GOO_ITERATOR_H_

#include <goo-object.h>

G_BEGIN_DECLS

#define GOO_TYPE_ITERATOR \
	(goo_iterator_get_type ())
#define GOO_ITERATOR(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_ITERATOR, GooIterator))
#define GOO_IS_ITERATOR(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_ITERATOR))
#define GOO_ITERATOR_GET_IFACE(inst) \
	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), GOO_TYPE_ITERATOR, GooIteratorIface))

typedef struct _GooList GooList;

/**
 * GooList:
 *
 * An iterable list type
 */

typedef struct _GooIterator GooIterator;

/**
 * GooIterator:
 *
 * An iterator for a #GooList is used for iterating over a list. The iterator
 * is an instance that keeps the position state information. The #GooList instance
 * will not by itself keep a position state. Only iterators can keep a position
 * state for a list.
 */

typedef struct _GooIteratorIface GooIteratorIface;

struct _GooIteratorIface
{
	GTypeInterface parent;

	void (*next_func) (GooIterator *self);
	void (*prev_func) (GooIterator *self);
	void (*first_func) (GooIterator *self);
	void (*nth_func) (GooIterator *self, guint nth);
	GooObject* (*get_current_func) (GooIterator *self);

	gboolean (*is_done_func) (GooIterator *self);
	GooList* (*get_list_func) (GooIterator *self);
};

GType goo_iterator_get_type (void);

void goo_iterator_next (GooIterator *self);
void goo_iterator_prev (GooIterator *self);
void goo_iterator_first (GooIterator *self);
void goo_iterator_nth (GooIterator *self, guint nth);
GooObject* goo_iterator_get_current (GooIterator *self);
gboolean goo_iterator_is_done (GooIterator *self);
GooList* goo_iterator_get_list (GooIterator *self);

G_END_DECLS

#endif /* _GOO_ITERATOR_H_ */
