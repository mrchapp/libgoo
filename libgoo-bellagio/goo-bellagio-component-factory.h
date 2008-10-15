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

#ifndef _GOO_BELLAGIO_COMPONENT_FACTORY_H_
#define _GOO_BELLAGIO_COMPONENT_FACTORY_H_

#include <goo-component-factory.h>

G_BEGIN_DECLS

typedef enum _GooComponentType GooComponentType;
enum _GooComponentType
{
	GOO_BELLAGIO_MP3_DECODER,
	GOO_BELLAGIO_MPEG4_DECODER,
};

#define GOO_TYPE_BELLAGIO_COMPONENT_FACTORY \
     (goo_bellagio_component_factory_get_type())
#define GOO_BELLAGIO_COMPONENT_FACTORY(obj) \
     (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_BELLAGIO_COMPONENT_FACTORY, GooBellagioComponentFactory))
#define GOO_BELLAGIO_COMPONENT_FACTORY_CLASS(klass) \
     (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_BELLAGIO_COMPONENT_FACTORY, GooBellagioComponentFactoryClass))
#define GOO_IS_BELLAGIO_COMPONENT_FACTORY(obj) \
     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_BELLAGIO_COMPONENT_FACTORY))
#define GOO_IS_BELLAGIO_COMPONENT_FACTORY_CLASS(klass) \
     (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_BELLAGIO_COMPONENT_FACTORY))
#define GOO_BELLAGIO_COMPONENT_FACTORY_GET_CLASS(obj) \
     (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_BELLAGIO_COMPONENT_FACTORY, GooBellagioComponentFactory))

typedef struct _GooBellagioComponentFactory GooBellagioComponentFactory;
typedef struct _GooBellagioComponentFactoryClass GooBellagioComponentFactoryClass;

/**
 * GooBellagioComponentFactory:
 *
 * Is the implementation of the #GooComponentFactory for Texas Instrument
 * OpenMAX components.
 */
struct _GooBellagioComponentFactory
{
	GooComponentFactory parent;
	GooComponent* clock;
};

struct _GooBellagioComponentFactoryClass
{
	GooComponentFactoryClass parent;
};

GType goo_bellagio_component_factory_get_type (void);
GooComponentFactory* goo_bellagio_component_factory_get_instance (void);

G_END_DECLS

#endif /* _GOO_BELLAGIO_COMPONENT_FACTORY_H_ */
