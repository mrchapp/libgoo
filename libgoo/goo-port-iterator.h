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

#ifndef _GOO_PORT_ITERATOR_H_
#define _GOO_PORT_ITERATOR_H_

#include <goo-port-list.h>

G_BEGIN_DECLS

#define GOO_TYPE_PORT_ITERATOR \
        (goo_port_iterator_get_type ())
#define GOO_PORT_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_PORT_ITERATOR, GooPortIterator))
#define GOO_PORT_ITERATOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_PORT_ITERATOR, GooPortIteratorClass))
#define GOO_IS_PORT_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_PORT_ITERATOR))
#define GOO_IS_PORT_ITERATOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_PORT_ITERATOR))
#define GOO_PORT_ITERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_PORT_ITERATOR, GooPortIteratorClass))

typedef struct _GooPortIterator GooPortIterator;
typedef struct _GooPortIteratorClass GooPortIteratorClass;

struct _GooPortIterator
{
        GObject parent;

        GooPortList* list;
        GList* current;
};

struct _GooPortIteratorClass
{
        GObjectClass parent;
};

GType goo_port_iterator_get_type (void);
GooIterator* goo_port_iterator_new (GooPortList* list);
void goo_port_iterator_set_list (GooPortIterator* self, GooPortList* list);


G_END_DECLS

#endif /* _GOO_PORT_ITERATOR_H_ */
