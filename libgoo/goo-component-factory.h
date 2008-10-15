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

#ifndef _GOO_COMPONENT_FACTORY_H_
#define _GOO_COMPONENT_FACTORY_H_

#include <goo-component.h>
#include <goo-list.h>

G_BEGIN_DECLS

#define GOO_TYPE_COMPONENT_FACTORY \
	(goo_component_factory_get_type())
#define GOO_COMPONENT_FACTORY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_COMPONENT_FACTORY, GooComponentFactory))
#define GOO_COMPONENT_FACTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_COMPONENT_FACTORY, GooComponentFactoryClass))
#define GOO_IS_COMPONENT_FACTORY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_COMPONENT_FACTORY))
#define GOO_IS_COMPONENT_FACTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_COMPONENT_FACTORY))
#define GOO_COMPONENT_FACTORY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_COMPONENT_FACTORY, GooComponentFactoryClass))

typedef struct _GooComponentFactory GooComponentFactory;
typedef struct _GooComponentFactoryClass GooComponentFactoryClass;

/**
 * GooComponentFactory:
 *
 * It is base object for the OpenMAX component factories
 *
 * @components: The #GooList of created components
 */
struct _GooComponentFactory
{
	GooObject parent;
	GooList* components;
};

struct _GooComponentFactoryClass
{
	GooObjectClass parent;

	GooComponent* (*get_component_func) (GooComponentFactory *self,
					     guint type);
};

GType goo_component_factory_get_type (void);

GooComponent* goo_component_factory_get_component (GooComponentFactory *self,
						   guint type);

void goo_component_factory_add_component (GooComponentFactory* self,
					  GooComponent* component);

G_END_DECLS

#endif /* _GOO_FACTORY_H_ */
