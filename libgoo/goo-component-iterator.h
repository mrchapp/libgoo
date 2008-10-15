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

#ifndef _GOO_COMPONENT_ITERATOR_H_
#define _GOO_COMPONENT_ITERATOR_H_

#include <goo-component-list.h>

G_BEGIN_DECLS

#define GOO_TYPE_COMPONENT_ITERATOR \
        (goo_component_iterator_get_type ())
#define GOO_COMPONENT_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_COMPONENT_ITERATOR, GooComponentIterator))
#define GOO_COMPONENT_ITERATOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_COMPONENT_ITERATOR, GooComponentIteratorClass))
#define GOO_IS_COMPONENT_ITERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_COMPONENT_ITERATOR))
#define GOO_IS_COMPONENT_ITERATOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_COMPONENT_ITERATOR))
#define GOO_COMPONENT_ITERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_COMPONENT_ITERATOR, GooComponentIteratorClass))

typedef struct _GooComponentIterator GooComponentIterator;
typedef struct _GooComponentIteratorClass GooComponentIteratorClass;

/**
 * GooComponentIterator:
 *
 */
struct _GooComponentIterator
{
        GObject parent;

        GooComponentList *list;
        GList *current;
};

struct _GooComponentIteratorClass
{
        GObjectClass parent;
};

GType goo_component_iterator_get_type (void);
GooIterator* goo_component_iterator_new (GooComponentList* list);
void goo_component_iterator_set_list (GooComponentIterator* self,
				      GooComponentList* list);


G_END_DECLS

#endif /* _GOO_COMPONENT_ITERATOR_H_ */
