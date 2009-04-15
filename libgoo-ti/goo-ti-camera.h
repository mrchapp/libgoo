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

#ifndef _GOO_TI_CAMERA_H_
#define _GOO_TI_CAMERA_H_

#include <goo-component.h>
#include <goo-ti-post-processor.h>
#include <goo-ti-video-encoder.h>
#include <goo-ti-jpegenc.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_CAMERA \
	(goo_ti_camera_get_type ())
#define GOO_TI_CAMERA(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_CAMERA, GooTiCamera))
#define GOO_TI_CAMERA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_CAMERA, GooTiCameraClass))
#define GOO_IS_TI_CAMERA(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_CAMERA))
#define GOO_IS_TI_CAMERA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_CAMERA))
#define GOO_TI_CAMERA_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_TI_CAMERA, GooTiCameraClass))

typedef struct _GooTiCamera GooTiCamera;
typedef struct _GooTiCameraPriv GooTiCameraPriv;
typedef struct _GooTiCameraClass GooTiCameraClass;

/**
 * GooTiCamera:
 * @param: The camera sensor configuration (#OMX_PARAM_SENSORMODETYPE)
 *
 * It is the OMX camera component
 */
struct _GooTiCamera
{
	GooComponent parent;

	OMX_PARAM_SENSORMODETYPE* param;
};

struct _GooTiCameraClass
{
	GooComponentClass parent_class;
};

/**
 * GOO_TI_CAMERA_GET_PARAM:
 * @camera: A #GooTiCamera instace
 *
 * Gets the #OMX_PARAM_SENSORMODETYPE of the camera
 *
 * Return value: The #OMX_PARAM_SENSORMODETYPE pointer
 */
#define GOO_TI_CAMERA_GET_PARAM(camera) (GOO_TI_CAMERA (camera)->param)

/**
 * GooTiCameraZoom:
 * @GOO_TI_CAMERA_ZOOM_1X:
 * @GOO_TI_CAMERA_ZOOM_2X:
 * @GOO_TI_CAMERA_ZOOM_3X:
 * @GOO_TI_CAMERA_ZOOM_4X:
 *
 * Possible values for zooming
 */
typedef enum
{
	GOO_TI_CAMERA_ZOOM_1X = 0x10000,
	GOO_TI_CAMERA_ZOOM_2X = 0x20000,
	GOO_TI_CAMERA_ZOOM_3X = 0x30000,
	GOO_TI_CAMERA_ZOOM_4X = 0x40000
} GooTiCameraZoom;

#define GOO_TI_CAMERA_ZOOM (goo_ti_camera_zoom_get_type ())
#define GOO_TI_CAMERA_WHITE_BALANCE (goo_ti_camera_white_balance_type ())
#define GOO_TI_CAMERA_EXPOSURE (goo_ti_camera_exposure_type ())

GType goo_ti_camera_get_type (void);
GType goo_ti_camera_zoom_get_type (void);
GType goo_ti_camera_white_balance_type (void);
GType goo_ti_camera_exposure_type (void);

G_END_DECLS

#endif /* _GOO_TI_CAMERA_H_ */
