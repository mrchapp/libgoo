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

#ifndef _GOO_TI_COMPONENT_FACTORY_H_
#define _GOO_TI_COMPONENT_FACTORY_H_

#include <goo-component-factory.h>

G_BEGIN_DECLS

typedef enum _GooComponentType GooComponentType;
enum _GooComponentType
{
	GOO_TI_NBAMR_DECODER,
	GOO_TI_PCM_ENCODER,
	GOO_TI_NBAMR_ENCODER,
	GOO_TI_GSMHR_ENCODER,
	GOO_TI_AAC_DECODER,
	GOO_TI_AMR_DECODER,
	GOO_TI_MP3_DECODER,
	GOO_TI_WBAMR_DECODER,
	GOO_TI_PCM_DECODER,
	GOO_TI_IMAADPCM_DECODER,
	GOO_TI_GSMHR_DECODER,
	GOO_TI_G722_DECODER,
	GOO_TI_G711_DECODER,
	GOO_TI_WMA_DECODER,
	GOO_TI_AMR_ENCODER,
	GOO_TI_WBAMR_ENCODER,
	GOO_TI_GSMFR_ENCODER,
	GOO_TI_AAC_ENCODER,
	GOO_TI_JPEG_ENCODER,
	GOO_TI_JPEG_DECODER,
	GOO_TI_MPEG4_DECODER,
	GOO_TI_SPARK_DECODER,
	GOO_TI_H264_DECODER,
	GOO_TI_H263_DECODER,
	GOO_TI_WMV_DECODER,
	GOO_TI_MPEG4_ENCODER,
	GOO_TI_H264_ENCODER,
	GOO_TI_H263_ENCODER,
	GOO_TI_CAMERA,
	GOO_TI_VPP,
	GOO_TI_POST_PROCESSOR,
	GOO_TI_MPEG2_DECODER,
	GOO_TI_GSMFR_DECODER,
	GOO_TI_CLOCK
};

#define GOO_TYPE_TI_COMPONENT_FACTORY \
     (goo_ti_component_factory_get_type())
#define GOO_TI_COMPONENT_FACTORY(obj) \
     (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_COMPONENT_FACTORY, GooTiComponentFactory))
#define GOO_TI_COMPONENT_FACTORY_CLASS(klass) \
     (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_COMPONENT_FACTORY, GooTiComponentFactoryClass))
#define GOO_IS_TI_COMPONENT_FACTORY(obj) \
     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_COMPONENT_FACTORY))
#define GOO_IS_TI_COMPONENT_FACTORY_CLASS(klass) \
     (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_COMPONENT_FACTORY))
#define GOO_TI_COMPONENT_FACTORY_GET_CLASS(obj) \
     (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_COMPONENT_FACTORY, GooTiComponentFactory))

typedef struct _GooTiComponentFactory GooTiComponentFactory;
typedef struct _GooTiComponentFactoryClass GooTiComponentFactoryClass;

/**
 * GooTiComponentFactory:
 *
 * Is the implementation of the #GooComponentFactory for Texas Instrument
 * OpenMAX components.
 */
struct _GooTiComponentFactory
{
	GooComponentFactory parent;
	GooComponent* clock;
};

struct _GooTiComponentFactoryClass
{
	GooComponentFactoryClass parent;
};

GType goo_ti_component_factory_get_type (void);
GooComponentFactory* goo_ti_component_factory_get_instance (void);

G_END_DECLS

#endif /* _GOO_TI_COMPONENT_FACTORY_H_ */
