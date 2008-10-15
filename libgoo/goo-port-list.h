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

#ifndef _GOO_PORT_LIST_H_
#define _GOO_PORT_LIST_H_

#include <goo-object.h>
#include <goo-list.h>

G_BEGIN_DECLS

#define GOO_TYPE_PORT_LIST goo_port_list_get_type()

#define GOO_PORT_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_PORT_LIST, GooPortList))
#define GOO_PORT_LIST_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_PORT_LIST, GooPortListClass))
#define GOO_IS_PORT_LIST(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_PORT_LIST))
#define GOO_IS_PORT_LIST_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_PORT_LIST))
#define GOO_PORT_LIST_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_PORT_LIST, GooPortListClass))

typedef struct _GooPortList GooPortList;
typedef struct _GooPortListClass GooPortListClass;

struct _GooPortList
{
        GObject parent;
        GList* first;
        GMutex* list_lock;
};

struct _GooPortListClass
{
        GObjectClass parent_class;
};

GType goo_port_list_get_type (void);
GooList* goo_port_list_new (void);

G_END_DECLS

#endif /* _GOO_PORT_LIST_H_ */
