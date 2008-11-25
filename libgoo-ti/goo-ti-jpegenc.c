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

/**
 * SECTION:goo-ti-jpegenc
 * @short_description: Represent a TI OpenMAX JPEG encoder component
 * @see_also: #GooComponent
 *
 * The #GooTiJpegEnc is the abstraction of the OpenMAX JPEG encoder component
 * in the Texas Instrument implementation.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <goo-ti-jpegenc.h>
#include <goo-utils.h>

#define ID "OMX.TI.JPEG.encoder"

#define MAX_NUM_BUFFERS 4

enum _GooTiJpegEncProc
{
    PROP_O,
	PROP_COMMENT,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_APP0I,
	PROP_APP0W,
	PROP_APP0H,
	PROP_APP1I,
	PROP_APP1W,
	PROP_APP1H,
	PROP_APP13I,
	PROP_APP13W,
	PROP_APP13H
};

typedef struct _APP_INFO {
  OMX_U8 *app;
  OMX_U16 size;
} APP_INFO;

typedef enum OMX_JPEGE_INDEXTYPE
{
    OMX_IndexCustomCommentFlag = 0xFF000001,
	OMX_IndexCustomCommentString = 0xFF000002,
	OMX_IndexCustomInputFrameWidth,
	OMX_IndexCustomInputFrameHeight,
	OMX_IndexCustomThumbnailAPP0_INDEX,
	OMX_IndexCustomThumbnailAPP0_W,
	OMX_IndexCustomThumbnailAPP0_H,
	OMX_IndexCustomThumbnailAPP0_BUF,
	OMX_IndexCustomThumbnailAPP1_INDEX,
	OMX_IndexCustomThumbnailAPP1_W,
	OMX_IndexCustomThumbnailAPP1_H,
	OMX_IndexCustomThumbnailAPP1_BUF,
	OMX_IndexCustomThumbnailAPP13_INDEX,
	OMX_IndexCustomThumbnailAPP13_W,
	OMX_IndexCustomThumbnailAPP13_H,
	OMX_IndexCustomThumbnailAPP13_BUF,
	OMX_IndexCustomDRI
} OMX_INDEXIMAGETYPE;

struct _ThumbnailJpegPrivate
{
	guint width;
	guint height;
	gboolean APP0_Index;
	guint APP0_width;
	guint APP0_height;
	gboolean APP1_Index;
	guint APP1_width;
	guint APP1_height;
	gboolean APP13_Index;
	guint APP13_width;
	guint APP13_height;
};

#define WIDTH 1024
#define HEIGHT 780
#define THUMBNAIL_WIDTH 128
#define THUMBNAIL_HEIGHT 96

OMX_U8 APPLICATION1[300]={
/* 0 */0, 0, 0, 0, 0x45, 0x78, 0x69, 0x66, 0x00, 0x00,      /* Indicate Exif Data*/
/* 10 */ 0x49, 0x49,                                                  /* "Intel" type byte align*/
0x2A, 0x00,                                             /* Confirm "Intel" type byte align*/
/* 14 */ 0x08, 0x00, 0x00, 0x00,            /* Offset to first IFDc*/
0x06, 0x00,                     /* Number of IFD as 1*/

/* 21 */0x0f, 0x01,                     /* TAG: Make*/
0x02, 0x00,                    /* Type: Data format is 0x0002, ASCII */
0x0c, 0x00, 0x00, 0x00,  /* Count: number of chars*/
0x56, 0x00, 0x00, 0x00, /* Offset Make data*/

/* 33 */0x10, 0x01,                     /* TAG: Model*/
0x02, 0x00,                    /* Type: Data format is 0x0002, ASCII */
0x05, 0x00, 0x00, 0x00,  /* Count: number of chars*/
0x62, 0x00, 0x00, 0x00, /* Offset Model data*/

/*45*/0x12, 0x01,                     /* TAG: Orientation*/
0x03, 0x00,                    /* Type: Data format is 0x0003,  (short)*/
0x01, 0x00, 0x00, 0x00,  /* Count: number of chars*/
0x01, 0x00, 0x00, 0x00, /* 1 means normal orientation*/

/*57*/0x31, 0x01,                     /* TAG: Software*/
0x02, 0x00,                    /* Type: ASCII*/
0x08, 0x00, 0x00, 0x00,  /* Count: number of chars*/
0x67, 0x00, 0x00, 0x00, /* Offset*/

/*69*/0x3b, 0x01,                     /* TAG: Artist*/
0x02, 0x00,                    /* Type: ASCII*/
0x09, 0x00, 0x00, 0x00,  /* Count: number of chars*/
0x6f, 0x00, 0x00, 0x00, /* Offset*/


/* 69 */0x69, 0x87,                     /* Exif SubIFD*/
/* 71 */ 0x04, 0x00,                    /* Data format is 0x0004, ULInt*/
0x01, 0x00,  0x00, 0x00,            /* number of components is 1.*/
/*long integer data size is 4bytes/components, so total data length is 1x4=4bytes*/
/* 77 */ 0x78, 0x00,0x00, 0x00,             /* Offset of Exif data*/

/*81*/0x9E, 0x00,  0x00, 0x00,    /* Offset to next IFD. As we are saying only one directory( Number of IFD as 1) this indicate the offset of next IFD*/

/*85*/0x53, 0x61, 0x73, 0x6b, 0x65, 0x6e, 0x20, 0x26, 0x20, 0x54, 0x49, 0x00, /*Make*/

/*97*/0x4f, 0x4d, 0x41, 0x50,0x00, /*Model*/

/*102*/0x4f, 0x70, 0x65, 0x6e, 0x4d, 0x61, 0x78, 0x00, /*Software*/

/*110*/0x47, 0x65, 0x6f, 0x72, 0x67, 0x65, 0x20, 0x53, 0x00, /*Artist*/

/* exif ub-ID start Here */
/* 119 */ 0x03,  0x00,   /* Number of Exif ID*/
0x00, 0x90, /* Exif Version*/
0x07, 0x00,     /*Data format is 0x0007, undefined*/
0x04, 0x00, 0x00, 0x00,         /* number of components is 4.*/
/*Undefined data size is   1bytes/components, so total data length is 4x1=4bytes*/
0x30, 0x32, 0x32, 0x30, /* Exif version number 30 32 32 30 meaning 0220 (2.2)*/

/*133*/0x02,  0xA0,    /* Exif Image Width  (0xA002)*/
0x04, 0x00,                     /* Data format is 0x0004, ULInt*/
0x01, 0x00,  0x00, 0x00,            /* number of components is 1.*/
/* 141 */ 0xB0, 0x00,0x00, 0x00,     /* Image width  , 0x00000280 i.e. 640*/

/*145*/0x03,  0xA0,    /* Exif Image Width  (0xA003)*/
0x04, 0x00,                     /* Data format is 0x0004, ULInt*/
0x01, 0x00,  0x00, 0x00,            /* number of components is 1.*/
/* 153 */ 0x90, 0x00,0x00, 0x00,     /* Image Height  , 0x000001E0 i.e. 480*/



/* next IFD start Here */
/*157*/0x03,  0x00,    /* Number of IFD1*/
/*159*/0x03,  0x01,    /* Compression  (0x0103)*/
0x03, 0x00,                     /*Data format is 0x0003 unsigned short,*/
0x01, 0x00,  0x00, 0x00,            /* number of components is 1.*/
/*unsigned short  data size is 2bytes/components, so total data length is 1x2=2bytes*/

0x06, 0x00,0x00, 0x00,  /* '6' means JPEG compression.*/
                        /*Shows compression method.
                        o   '1' means no compression,
                        o    '6' means JPEG compression.*/

/*171*/            0x01,  0x02,    /* JpegIFOffset  (0x0201)*/
                        0x04, 0x00,                     /* Data format is 0x0004, ULInt*/
                        0x01, 0x00,  0x00, 0x00,            /* number of components is 1.*/
/*179*/            0x22, 0x01,0x00, 0x00,  /* Address 0f Thumbnail data*/

/*183*/            0x02,  0x02,    /* JpegIFByteCount (0x0202)*/
                        0x04, 0x00,                     /* Data format is 0x0004, ULInt*/
                        0x01, 0x00,  0x00, 0x00,            /* number of components is 1.*/
/*191*/            0xff, 0xff,0xff, 0xff,  /* Legth of thumbnail data*/
};

/*Set the fist 4 bytes to 0*/
OMX_U8 APPLICATION13[200] = {
    0x00, 0x00, 0x00, 0x00, /*We should set the first 4 bytes to 0 */
    0x50, 0x68, 0x6f, 0x74, 0x6f, 0x73, 0x68, 0x6f, 0x70, 0x20, 0x33, 0x2e, 0x30, 0x00,/*Photoshop header*/
    0x38, 0x42, 0x49, 0x4d, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4e, /* Initial marker*/
    /*IPTC Marker       TAG         Size of string*/
    0x1c,      0x02,       0x78,       0x00, 0x20,
    0x54, 0x68, 0x69, 0x73,  0x20, 0x69, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x61, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x69, 0x6d, 0x61, 0x67, 0x65, /*String of data (ASCII)*/
    0x1c, 0x02, 0x7a, 0x00, 0x16,
    0x49, 0x27,0x6d, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x61, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x77, 0x72, 0x69, 0x74, 0x65, 0x72,
    0x1c, 0x02, 0x5a, 0x00, 0x09,
    0x4d, 0x6f, 0x6e, 0x74, 0x65, 0x72, 0x72, 0x65, 0x79,

};

OMX_U8 APPLICATION0[10]={0, 0, 0, 0, 0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};

G_DEFINE_TYPE (GooTiJpegEnc, goo_ti_jpegenc, GOO_TYPE_COMPONENT)

static void
_goo_ti_jpegenc_set_comment (GooTiJpegEnc* self)
{
        g_assert (self != NULL);
        g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
        gint flag;

        if (self->comment != NULL && strlen (self->comment) != 0)
        {
			GOO_OBJECT_DEBUG (self, "comment");
            flag = 1;
            goo_component_set_config_by_index (GOO_COMPONENT (self),
                                                OMX_IndexCustomCommentFlag,
                                                &flag);
            goo_component_set_config_by_index (GOO_COMPONENT (self),
                                                OMX_IndexCustomCommentString,
                                                self->comment);
        }
		else
        {
            flag = 0;
            goo_component_set_config_by_index (GOO_COMPONENT (self),
                                                OMX_IndexCustomCommentFlag,
                                                &flag);
        }

        GOO_OBJECT_DEBUG (self, "");

        return;
}

static void
_goo_ti_jpegenc_get_comment (GooTiJpegEnc* self)
{
        g_assert (self != NULL);
        g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

        goo_component_get_config_by_index (GOO_COMPONENT (self),
                                           OMX_IndexCustomCommentString,
                                           self->comment);

        GOO_OBJECT_DEBUG (self, "");

        return;
}

static void
_goo_ti_jpegenc_set_thumbnail(GooTiJpegEnc* self, guint prop_id)
{
		g_assert (self != NULL);
		g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
		APP_INFO app_info;
		guint flag = 0;

		switch (prop_id)
        {
			case PROP_APP1I:
			{
				if(self->thumbnail->APP1_Index)
				{
					flag = 1;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP1_INDEX,
														&flag);
					app_info.app = (OMX_U8 *)g_malloc(sizeof(APPLICATION1));
					memcpy(app_info.app, APPLICATION1, sizeof(APPLICATION1));
					app_info.size = sizeof(APPLICATION1);
					flag = self->thumbnail->width;
					app_info.app[58] = flag & 0xFF;
					app_info.app[59] = (flag >> 8) & 0xFF;
					flag = self->thumbnail->height;
					app_info.app[70] = flag;
					app_info.app[71] = (flag >> 8) & 0xFF;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP1_BUF,
														&app_info);
					flag = self->thumbnail->APP1_width;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP1_W,
														&flag);
					flag = self->thumbnail->APP1_height;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP1_H,
														&flag);
				}
				break;
			}
			case PROP_APP13I:
			{
				if(self->thumbnail->APP13_Index)
				{
					flag = 1;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP13_INDEX,
														&flag);
					app_info.app = (OMX_U8 *)g_malloc(sizeof(APPLICATION13));
					memcpy(app_info.app, APPLICATION13, sizeof(APPLICATION13));
					app_info.size = sizeof(APPLICATION13);
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP13_BUF,
														&app_info);
					flag = self->thumbnail->APP13_width;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP13_W,
														&flag);
					flag = self->thumbnail->APP13_height;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP13_H,
														&flag);
				}
				break;
			}
			case PROP_APP0I:
			{
				if(self->thumbnail->APP0_Index)
				{
					flag = 1;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP0_INDEX,
														&flag);
					app_info.size = 0;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP0_BUF,
														&app_info);
					flag = self->thumbnail->APP0_width;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP0_W,
														&flag);
					flag = self->thumbnail->APP0_height;
					goo_component_set_config_by_index (GOO_COMPONENT (self),
														OMX_IndexCustomThumbnailAPP0_H,
														&flag);
				}
				break;
			}
		}

		GOO_OBJECT_DEBUG (self, "");
        return;
}

/*
static ThumbnailJpegPrivate*
_goo_ti_jpegenc_get_thumbnail (GooTiJpegEnc* self)
{
    g_assert (self != NULL);
    g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	ThumbnailJpegPrivate* thumbnail;
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP0_INDEX,
                                        &(thumbnail->APP0_Index));
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP0_W,
                                        &(thumbnail->APP0_width));
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP0_H,
                                        &(thumbnail->APP0_height));
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP1_INDEX,
                                        &(thumbnail->APP1_Index));
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP1_W,
                                        &(thumbnail->APP1_width));
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP1_H,
                                        &(thumbnail->APP1_height));
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP13_INDEX,
                                        &(thumbnail->APP13_Index));
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP13_W,
                                        &(thumbnail->APP13_width));
	goo_component_get_config_by_index (GOO_COMPONENT (self),
                                        OMX_IndexCustomThumbnailAPP13_H,
                                        &(thumbnail->APP13_height));

	self->thumbnail->APP1_Index = thumbnail->APP1_Index;
	self->thumbnail->APP1_width = thumbnail->APP1_width;
	self->thumbnail->APP1_height = thumbnail->APP1_height;
	self->thumbnail->APP13_Index = thumbnail->APP13_Index;
	self->thumbnail->APP13_width = thumbnail->APP13_width;
	self->thumbnail->APP13_height = thumbnail->APP13_height;

	GOO_OBJECT_DEBUG (self, "");

	return thumbnail;
}
*/

static void
goo_ti_jpegenc_load_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_JPEGENC (component));
        GooTiJpegEnc* self = GOO_TI_JPEGENC (component);

        g_assert (self->param == NULL);
		g_assert (self->thumbnail == NULL);
        g_assert (component->cur_state != OMX_StateInvalid);

        self->param = g_new0 (OMX_IMAGE_PARAM_QFACTORTYPE, 1);
		self->thumbnail = g_new0 (ThumbnailJpegPrivate, 1);
		self->comment = g_new0 (gchar, 1024);

		self->thumbnail->APP0_Index = FALSE;
        self->thumbnail->APP1_Index = FALSE;
		self->thumbnail->APP13_Index = FALSE;
		GOO_INIT_PARAM (self->param, OMX_IMAGE_PARAM_QFACTORTYPE);

        GOO_OBJECT_DEBUG (self, "");

        return;
}

static void
goo_ti_jpegenc_set_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_JPEGENC (component));
        GooTiJpegEnc* self = GOO_TI_JPEGENC (component);

        g_assert (self->param != NULL);
        g_assert (component->cur_state == OMX_StateLoaded);

		goo_component_set_parameter_by_index (component,
                                              OMX_IndexParamQFactor,
                                              self->param);

        GOO_OBJECT_DEBUG (self, "");

        return;
}

static void
goo_ti_jpegenc_validate (GooComponent* component)
{
    g_assert (GOO_IS_TI_JPEGENC (component));
    GooTiJpegEnc* self = GOO_TI_JPEGENC (component);
    g_assert (self->param != NULL);
    g_assert (component->cur_state == OMX_StateLoaded);

    /* param */
    {
        OMX_IMAGE_PARAM_QFACTORTYPE* param;
        param = GOO_TI_JPEGENC_GET_PARAM (self);

        g_assert (param->nQFactor >= 0 && param->nQFactor <= 100);
    }

    /* inport */
    {
        OMX_PARAM_PORTDEFINITIONTYPE *param;

        GooIterator* iter = goo_component_iterate_input_ports (component);
        goo_iterator_nth (iter, 0);
        GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
        g_assert (port != NULL);

        param = GOO_PORT_GET_DEFINITION (port);

        param->format.image.cMIMEType = "video/x-raw-yuv";
        param->format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
        guint width = param->format.image.nFrameWidth;
        guint height = param->format.image.nFrameHeight;

		/* the buffer size must be multiple of 16 */
		width = GOO_ROUND_UP_16 (width);
		height = GOO_ROUND_UP_16 (height);

		switch (param->format.image.eColorFormat)
		{
		case OMX_COLOR_FormatCbYCrY:
			param->nBufferSize = width * height * 2;
		break;
		case OMX_COLOR_FormatYUV420PackedPlanar:
            /* as in test app */
			param->nBufferSize = width * height * 1.5;
			break;
		default:
			GOO_OBJECT_ERROR (self, "Not valid color format");
			g_assert (FALSE);
			break;
		}

		g_assert (param->nBufferCountActual >= 1 &&
		param->nBufferCountActual <= MAX_NUM_BUFFERS);

		g_object_unref (iter);
		g_object_unref (port);
    }

    /* outport */
    {
        OMX_PARAM_PORTDEFINITIONTYPE *param;
		guint prop_id;

        GooIterator* iter =
            goo_component_iterate_output_ports (component);
        goo_iterator_nth (iter, 0);
        GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
        g_assert (port != NULL);

        param = GOO_PORT_GET_DEFINITION (port);

        param->format.image.cMIMEType = "image/jpeg";
        param->format.image.eCompressionFormat = OMX_IMAGE_CodingEXIF;

		guint width = param->format.image.nFrameWidth;
        guint height = param->format.image.nFrameHeight;

        /* the buffer size must be multiple of 16 */
		width = GOO_ROUND_UP_16 (width);
		height = GOO_ROUND_UP_16 (height);

        switch (param->format.image.eColorFormat)
        {
            case OMX_COLOR_FormatCbYCrY:
                param->nBufferSize = width * height * 2;
                break;
            case OMX_COLOR_FormatYUV420PackedPlanar:
                param->nBufferSize = width * height * 1.5;
                break;
            default:
                GOO_OBJECT_ERROR (self, "Not valid color format");
                g_assert (FALSE);
        }

        g_assert (param->nBufferCountActual >= 1 &&
        param->nBufferCountActual <= MAX_NUM_BUFFERS);

		goo_component_configure_port(component, port);

		prop_id = PROP_APP1I;
		_goo_ti_jpegenc_set_thumbnail (self, prop_id);
		/*prop_id = PROP_APP13I;
		_goo_ti_jpegenc_set_thumbnail (self, prop_id);
		prop_id = PROP_APP0I;
		_goo_ti_jpegenc_set_thumbnail (self, prop_id);*/
		_goo_ti_jpegenc_set_comment (self);

        g_object_unref (iter);
        g_object_unref (port);
    }

    GOO_OBJECT_DEBUG (self, "");

    return;
}

static void
goo_ti_jpegenc_set_property (GObject* object, guint prop_id,
                             const GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_JPEGENC (object));
		GooTiJpegEnc* self = GOO_TI_JPEGENC (object);

		switch (prop_id)
        {
			case PROP_WIDTH:
			{
				GOO_OBJECT_DEBUG (self, "WIDTH");
				self->thumbnail->width = g_value_get_uint(value);
				break;
			}
			case PROP_HEIGHT:
			{
				GOO_OBJECT_DEBUG (self, "HEIGHT");
				self->thumbnail->height = g_value_get_uint(value);
				break;
			}
			case PROP_COMMENT:
			{
				GOO_OBJECT_DEBUG (self, "comment");
				self->comment = g_value_dup_string (value);
				break;
			}
			case PROP_APP0I:
			{
				GOO_OBJECT_DEBUG (self, "APP0_Index");
				self->thumbnail->APP0_Index = g_value_get_boolean (value);
				break;
			}
			case PROP_APP0W:
			{
				GOO_OBJECT_DEBUG (self, "APP0_W");
				self->thumbnail->APP0_width = g_value_get_uint(value);
				break;
			}
			case PROP_APP0H:
			{
				GOO_OBJECT_DEBUG (self, "APP0_H");
				self->thumbnail->APP0_height = g_value_get_uint(value);
				break;
			}
			case PROP_APP1I:
			{
				GOO_OBJECT_DEBUG (self, "APP1_Index");
				self->thumbnail->APP1_Index = g_value_get_boolean (value);
				break;
			}
			case PROP_APP1W:
			{
				GOO_OBJECT_DEBUG (self, "APP1_W");
				self->thumbnail->APP1_width = g_value_get_uint(value);
				break;
			}
			case PROP_APP1H:
			{
				GOO_OBJECT_DEBUG (self, "APP1_H");
				self->thumbnail->APP1_height = g_value_get_uint(value);
				break;
			}
			case PROP_APP13I:
			{
				GOO_OBJECT_DEBUG (self, "APP13_Index");
				self->thumbnail->APP13_Index = g_value_get_boolean (value);
				break;
			}
			case PROP_APP13W:
			{
				GOO_OBJECT_DEBUG (self, "APP13_W");
				self->thumbnail->APP13_width = g_value_get_uint(value);
				break;
			}
			case PROP_APP13H:
			{
				GOO_OBJECT_DEBUG (self, "APP13_H");
				self->thumbnail->APP13_height = g_value_get_uint(value);
				break;
			}
			default:
			{
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
				break;
			}
        }

        return;
}

static void
goo_ti_jpegenc_get_property (GObject* object, guint prop_id,
                            GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_JPEGENC (object));
		GooTiJpegEnc* self = GOO_TI_JPEGENC (object);

        switch (prop_id)
        {
			case PROP_WIDTH:
			{
				self->thumbnail->width = g_value_get_uint (value);
				break;
			}
			case PROP_HEIGHT:
			{
				self->thumbnail->height = g_value_get_uint (value);
				break;
			}
			case PROP_COMMENT:
			{
				_goo_ti_jpegenc_get_comment (self);
				break;
			}
			case PROP_APP0I:
			{
				self->thumbnail->APP0_Index = g_value_get_boolean (value);
				break;
			}
			case PROP_APP0W:
			{
				self->thumbnail->APP0_width = g_value_get_uint (value);
				break;
			}
			case PROP_APP0H:
			{
			self->thumbnail->APP0_height = g_value_get_uint (value);
					break;
			}
			case PROP_APP1I:
			{
				self->thumbnail->APP1_Index = g_value_get_boolean (value);
				break;
			}
			case PROP_APP1W:
			{
				self->thumbnail->APP1_width = g_value_get_uint (value);
				break;
			}
			case PROP_APP1H:
			{
			self->thumbnail->APP1_height = g_value_get_uint (value);
					break;
			}
			case PROP_APP13I:
			{
					self->thumbnail->APP13_Index = g_value_get_boolean (value);
				break;
			}
			case PROP_APP13W:
			{
					self->thumbnail->APP13_width = g_value_get_uint (value);
				break;
			}
			case PROP_APP13H:
			{
					self->thumbnail->APP13_height = g_value_get_uint (value);
			break;
			}
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
				break;
        }

        return;
}

static void
goo_ti_jpegenc_init (GooTiJpegEnc* self)
{
        GOO_COMPONENT (self)->id = g_strdup (ID);
        GOO_COMPONENT (self)->port_param_type = OMX_IndexParamImageInit;

        self->param = NULL;

        return;
}

static void
goo_ti_jpegenc_finalize (GObject* object)
{
        g_assert (GOO_IS_TI_JPEGENC (object));

        GooTiJpegEnc* self = GOO_TI_JPEGENC (object);

        GOO_OBJECT_DEBUG (self, "");

        if (G_LIKELY (self->param))
        {
                g_free (self->param);
                self->param = NULL;
        }

        (*G_OBJECT_CLASS (goo_ti_jpegenc_parent_class)->finalize) (object);

        return;
}

static void
goo_ti_jpegenc_class_init (GooTiJpegEncClass* klass)
{
        GObjectClass* g_klass = G_OBJECT_CLASS (klass);
		GParamSpec* spec;

        g_klass->set_property = goo_ti_jpegenc_set_property;
        g_klass->get_property = goo_ti_jpegenc_get_property;
        g_klass->finalize = goo_ti_jpegenc_finalize;
		/*Image*/
		spec = g_param_spec_uint ("width", "Width", "Get an image width",
									1, 2147483647, WIDTH, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_WIDTH, spec);
		spec = g_param_spec_uint ("height", "Height", "Get an image height",
									1, 2147483647, HEIGHT, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_HEIGHT, spec);
		/*Comment*/
        spec = g_param_spec_string ("comment", "Comment string","Set/Get an image comment",
									NULL, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_COMMENT, spec);
		/*APP0*/
		spec = g_param_spec_boolean ("app0i", "APP0i", "Set/Get an image APP0Thumbnail",
									FALSE, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP0I, spec);
		spec = g_param_spec_uint ("app0w", "APP0w", "Set/Get an image APP0 width",
									16, 320, THUMBNAIL_WIDTH, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP0W, spec);
		spec = g_param_spec_uint ("app0h", "APP0h", "Set/Get an image APP0 height",
									16, 320, THUMBNAIL_HEIGHT, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP0H, spec);
		/*APP1*/
		spec = g_param_spec_boolean ("app1i", "APP1i", "Set/Get an image APP1Thumbnail",
									FALSE, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP1I, spec);
		spec = g_param_spec_uint ("app1w", "APP1w", "Set/Get an image APP1 width",
									16, 320, THUMBNAIL_WIDTH, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP1W, spec);
		spec = g_param_spec_uint ("app1h", "APP1h", "Set/Get an image APP1 height",
									16, 320, THUMBNAIL_HEIGHT, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP1H, spec);
		/*APP13*/
		spec = g_param_spec_boolean ("app13i", "APP13i", "Set/Get an image APP13Thumbnail",
									FALSE, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP13I, spec);
		spec = g_param_spec_uint ("app13w", "APP13w", "Set/Get an image APP13 width",
									16, 320, THUMBNAIL_WIDTH, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP13W, spec);
		spec = g_param_spec_uint ("app13h", "APP13h", "Set/Get an image APP13 height",
									16, 320, THUMBNAIL_HEIGHT, G_PARAM_READWRITE);
        g_object_class_install_property (g_klass, PROP_APP13H, spec);

        GooComponentClass* c_klass = GOO_COMPONENT_CLASS (klass);
        c_klass->load_parameters_func = goo_ti_jpegenc_load_parameters;
        c_klass->set_parameters_func = goo_ti_jpegenc_set_parameters;
        c_klass->validate_ports_definition_func = goo_ti_jpegenc_validate;

        return;
}
