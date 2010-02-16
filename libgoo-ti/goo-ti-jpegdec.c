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
 * SECTION:goo-ti-jpegdec
 * @short_description: Represent a TI OpenMAX JPEG decoder component
 * @see_also: #GooComponent
 *
 * The #GooTiJpegDec is the abstraction of the OpenMAX JPEG decoder component
 * in the Texas Instrument implementation.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif




#include <string.h>

#include <goo-ti-jpegdec.h>
#include <goo-utils.h>

#define ID "OMX.TI.JPEG.decode"

#define MAX_NUM_BUFFERS 1
#define MAX_RES_SN_WIDTH 4000
#define MAX_RES_SN_HEIGTH 3008

typedef enum OMX_INDEXIMAGETYPE
{
	OMX_IndexCustomProgressiveFactor = 0xFF000001,
	OMX_IndexCustomInputFrameWidth	 = 0xFF000002,
	OMX_IndexCustomOutputColorFormat = 0xFF000003
} OMX_INDEXIMAGETYPE;

enum _GooTiJpegDecProp
{
	PROP_0,
	PROP_PROGRESSIVE,
	PROP_SCALE
};

#define GOO_TI_JPEGDEC_SCALE \
	(goo_ti_jpegdec_scale_get_type ())

GType
goo_ti_jpegdec_scale_get_type ()
{
	static GType goo_ti_jpegdec_scale_type = 0;

	if (G_UNLIKELY (goo_ti_jpegdec_scale_type == 0))
	{
		static const GEnumValue goo_ti_jpegdec_scale[] = {
			{ GOO_TI_JPEGDEC_SCALE_800, "800", "800% scale" },
			{ GOO_TI_JPEGDEC_SCALE_400, "400", "400% scale" },
			{ GOO_TI_JPEGDEC_SCALE_200, "200", "200% scale" },
			{ GOO_TI_JPEGDEC_SCALE_NONE, "100", "No scalation" },
			{ GOO_TI_JPEGDEC_SCALE_50,   "50",  "50% scale" },
			{ GOO_TI_JPEGDEC_SCALE_25,   "25",  "25% scale" },
			{ GOO_TI_JPEGDEC_SCALE_12,   "12",  "12.5% scale" },
			{ 0, NULL, NULL },
		};

		goo_ti_jpegdec_scale_type = g_enum_register_static
			("GooTiJpegDecScale", goo_ti_jpegdec_scale);
	}

	return goo_ti_jpegdec_scale_type;
}

G_DEFINE_TYPE (GooTiJpegDec, goo_ti_jpegdec, GOO_TYPE_COMPONENT)

static void
_goo_ti_jpegdec_set_progressive (GooTiJpegDec* self, gboolean progressive)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	self->progressive = progressive;
	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexCustomProgressiveFactor,
					   &progressive);

	GOO_OBJECT_DEBUG (self, "");

	return;
}

#if 0 /* not used */
static gboolean
_goo_ti_jpegdec_get_progressive (GooTiJpegDec* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);
	gboolean ret;

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexCustomProgressiveFactor,
					   &self->progressive);

	ret = self->progressive;

	GOO_OBJECT_DEBUG (self, "");

	return ret;
}
#endif

static void
_goo_ti_jpegdec_set_scale (GooTiJpegDec* self, GooTiJpegDecScale scale)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_CONFIG_SCALEFACTORTYPE* param;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	param->xWidth = (OMX_S32) scale;
	param->xHeight = (OMX_S32) scale;

	self->scale = scale;
	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonScale,
					   param);

	g_free (param);

	return;
}

#if 0 /* not used */
static GooTiJpegDecScale
_goo_ti_jpegdec_get_scale (GooTiJpegDec* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	GooTiJpegDecScale ret;
	OMX_CONFIG_SCALEFACTORTYPE* param;
	param = g_new0 (OMX_CONFIG_SCALEFACTORTYPE, 1);
	GOO_INIT_PARAM (param, OMX_CONFIG_SCALEFACTORTYPE);

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexConfigCommonScale,
					   param);

	ret = (GooTiJpegDecScale) param->xWidth;

	g_free (param);

	return ret;
}
#endif

static void
_goo_ti_jpegdec_set_outcolor (GooTiJpegDec* self, OMX_COLOR_FORMATTYPE color)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	goo_component_set_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexCustomOutputColorFormat,
					   &color);

	return;
}

#if 0
static OMX_COLOR_FORMATTYPE
_goo_ti_jpegdec_get_outcolor (GooTiJpegDec* self)
{
	g_assert (self != NULL);
	g_assert (GOO_COMPONENT (self)->cur_state != OMX_StateInvalid);

	OMX_COLOR_FORMATTYPE color;

	goo_component_get_config_by_index (GOO_COMPONENT (self),
					   OMX_IndexCustomOutputColorFormat,
					   &color);

	return color;
}
#endif

static void
goo_ti_jpegdec_load_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_JPEGDEC (component));
        GooTiJpegDec* self = GOO_TI_JPEGDEC (component);

    g_assert (self->section_decode == NULL);
	g_assert (self->subregion_decode == NULL);
	g_assert (self->max_res == NULL);

    g_assert (component->cur_state != OMX_StateInvalid);

    self->section_decode = g_new0 (OMX_CUSTOM_IMAGE_DECODE_SECTION, 1);
	self->subregion_decode = g_new0 (OMX_CUSTOM_IMAGE_DECODE_SUBREGION, 1);
	self->max_res = g_new0 (OMX_CUSTOM_RESOLUTION, 1);


	GOO_INIT_PARAM (self->section_decode, OMX_CUSTOM_IMAGE_DECODE_SECTION);
	GOO_INIT_PARAM (self->subregion_decode, OMX_CUSTOM_IMAGE_DECODE_SUBREGION);
	GOO_INIT_PARAM (self->subregion_decode, OMX_CUSTOM_RESOLUTION);

	GOO_OBJECT_DEBUG (self, "");

        return;
}

static void
goo_ti_jpegdec_set_parameters (GooComponent* component)
{
        g_assert (GOO_IS_TI_JPEGDEC (component));
        GooTiJpegDec* self = GOO_TI_JPEGDEC (component);

	OMX_CUSTOM_IMAGE_DECODE_SECTION* pSectionDecode;
	OMX_CUSTOM_IMAGE_DECODE_SUBREGION* pSubRegionDecode;
	OMX_CUSTOM_RESOLUTION *pMaxResolution;

    g_assert (self->section_decode != NULL);
	g_assert (self->subregion_decode != NULL);
	g_assert (self->max_res != NULL);

    g_assert (component->cur_state == OMX_StateLoaded);

    pSectionDecode = g_new0 (OMX_CUSTOM_IMAGE_DECODE_SECTION, 1);
	GOO_INIT_PARAM (pSectionDecode, OMX_CUSTOM_IMAGE_DECODE_SECTION);

	pSubRegionDecode = g_new0 (OMX_CUSTOM_IMAGE_DECODE_SUBREGION, 1);
	GOO_INIT_PARAM (pSubRegionDecode, OMX_CUSTOM_IMAGE_DECODE_SUBREGION);

    pMaxResolution = g_new0 (OMX_CUSTOM_RESOLUTION, 1);
    GOO_INIT_PARAM (self->subregion_decode, OMX_CUSTOM_RESOLUTION);

	/* Section decoding */
	goo_component_get_parameter_by_name(component,
					    "OMX.TI.JPEG.decode.Param.SectionDecode",
					    pSectionDecode);

	pSectionDecode->nMCURow = (self->section_decode)->nMCURow;
	pSectionDecode->bSectionsInput  = OMX_FALSE;
	pSectionDecode->bSectionsOutput = OMX_TRUE;

        goo_component_set_parameter_by_name (component,
					    "OMX.TI.JPEG.decode.Param.SectionDecode",
					    pSectionDecode);


	goo_component_get_parameter_by_name(component,
					    "OMX.TI.JPEG.decode.Param.SectionDecode",
					    pSectionDecode);


	/* SubRegion decoding */
	goo_component_get_parameter_by_name(component,
					    "OMX.TI.JPEG.decode.Param.SubRegionDecode",
					    pSubRegionDecode);

	pSubRegionDecode->nXOrg = (self->subregion_decode)->nXOrg;
	pSubRegionDecode->nYOrg  = (self->subregion_decode)->nYOrg;
	pSubRegionDecode->nXLength = (self->subregion_decode)->nXLength;
	pSubRegionDecode->nYLength = (self->subregion_decode)->nYLength;

        goo_component_set_parameter_by_name (component,
					    "OMX.TI.JPEG.decode.Param.SubRegionDecode",
					    pSubRegionDecode);

	/*Max resolution */

	goo_component_get_parameter_by_name(component,
					    "OMX.TI.JPEG.decode.Param.SetMaxResolution",
					    pMaxResolution);

	pMaxResolution->nWidth = MAX_RES_SN_WIDTH;
	pMaxResolution->nHeight = MAX_RES_SN_HEIGTH;

	GOO_OBJECT_DEBUG (self, "********InitMaxResolution->nWidth = %d, InitMaxResolution->nHeight= %d",(self->max_res)->nWidth, (self->max_res)->nHeight);

	goo_component_set_parameter_by_name (component,
					    "OMX.TI.JPEG.decode.Param.SetMaxResolution",
					    pMaxResolution);

        GOO_OBJECT_DEBUG (self, "");
        GOO_OBJECT_DEBUG (self, "********OutMaxResolution->nWidth = %d, OutMaxResolution->nHeight= %d", pMaxResolution->nWidth, pMaxResolution->nHeight);

        return;
}

/* Function that return true if number is a multiple of factor, false in other case */
static gboolean goo_ti_jpegdec_is_multiple(guint number, guint factor){
	if((number%factor)== 0)
		return TRUE;
	else
		return FALSE;
}


static void goo_ti_jpegdec_multiple_in_chromaformat(guint input_format, guint xorg, guint yorg){
	guint factors[5][2]={{16,16},  //XDM_YUV_420
			   {16, 8},  //XDM_YUV_422
			   { 8, 8},  //XDM_YUV_444
			   {32, 8},  //XDM_YUV_411
			   { 8, 8}}; //XDM_GRAY


	switch(input_format){

	case OMX_COLOR_FormatYUV420PackedPlanar:
		if(!goo_ti_jpegdec_is_multiple(xorg, factors[0][0]) || !goo_ti_jpegdec_is_multiple(yorg, factors[0][1])){
			g_critical("The properties of subregion must be number multiples of (%d, %d) respectively \n",factors[0][0], factors[0][1] );

		}
		g_assert(goo_ti_jpegdec_is_multiple(xorg, factors[0][0]) && goo_ti_jpegdec_is_multiple(yorg, factors[0][1]));
		break;
	case OMX_COLOR_FormatCbYCrY:
		if(!goo_ti_jpegdec_is_multiple(xorg, factors[1][0]) || !goo_ti_jpegdec_is_multiple(yorg, factors[1][1])){
			g_critical("The properties subregion must be number multiples of (%d, %d) respectively \n",factors[1][0], factors[1][1] );

		}
		g_assert(goo_ti_jpegdec_is_multiple(xorg, factors[1][0]) && goo_ti_jpegdec_is_multiple(yorg, factors[1][1]));
		break;
	case OMX_COLOR_FormatYUV444Interleaved:
		if(!goo_ti_jpegdec_is_multiple(xorg, factors[2][0]) || !goo_ti_jpegdec_is_multiple(yorg, factors[2][1])){
			g_critical("The properties subregion must be number multiples of (%d, %d) respectively \n",factors[2][0], factors[2][1] );

		}
		g_assert(goo_ti_jpegdec_is_multiple(xorg, factors[2][0]) && goo_ti_jpegdec_is_multiple(yorg, factors[2][1]));
		break;
	case OMX_COLOR_FormatYUV411Planar:
		if(!goo_ti_jpegdec_is_multiple(xorg, factors[3][0]) || !goo_ti_jpegdec_is_multiple(yorg, factors[3][1])){
			g_critical("The properties subregion must be number multiples of (%d, %d) respectively \n",factors[3][0], factors[3][1] );

		}
		g_assert(goo_ti_jpegdec_is_multiple(xorg, factors[3][0]) && goo_ti_jpegdec_is_multiple(yorg, factors[3][1]));
		break;
	case OMX_COLOR_FormatL8:
		if(!goo_ti_jpegdec_is_multiple(xorg, factors[4][0]) || !goo_ti_jpegdec_is_multiple(yorg, factors[4][1])){
			g_critical("The properties subregion must be number multiples of (%d, %d) respectively \n",factors[4][0], factors[4][1] );

		}
		g_assert(goo_ti_jpegdec_is_multiple(xorg, factors[4][0]) && goo_ti_jpegdec_is_multiple(yorg, factors[4][1]));
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}


static void
goo_ti_jpegdec_validate (GooComponent* component)
{
	g_assert (GOO_IS_TI_JPEGDEC (component));
	GooTiJpegDec* self = GOO_TI_JPEGDEC (component);

	g_assert (component->cur_state == OMX_StateLoaded);

	guint width, height, color;

	gdouble scalefactor;

	scalefactor = (self->scale != 12) ?
		self->scale * 1.0 / 100.0 : 12.5 / 100.0;

	/* section decoding */
	{
		OMX_CUSTOM_IMAGE_DECODE_SECTION* section_decode;
		section_decode = GOO_TI_JPEGDEC_GET_SECTION_DECODE (self);
		g_assert (section_decode->nMCURow >= 0 && section_decode->nMCURow < 32);

	}

	/* Subregion decoding */
	{
		OMX_CUSTOM_IMAGE_DECODE_SUBREGION* subregion_decode;
		subregion_decode = GOO_TI_JPEGDEC_GET_SUBREGION_DECODE (self);

		/* Validating that the x_org and y_org should set as multiples of the number
		   specified in the following table based on the input chromaformat */

		OMX_PARAM_PORTDEFINITIONTYPE *param;
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		param = GOO_PORT_GET_DEFINITION (port);


		goo_ti_jpegdec_multiple_in_chromaformat(param->format.image.eColorFormat, subregion_decode->nXOrg,subregion_decode->nYOrg );

		goo_ti_jpegdec_multiple_in_chromaformat(param->format.image.eColorFormat, subregion_decode->nXLength,subregion_decode->nYLength );

		if((subregion_decode->nXOrg +subregion_decode->nXLength) > param->format.image.nFrameWidth){
			g_critical("The property subregion-top plus subregion-height must be less than the width of the image.\n");
			g_assert((subregion_decode->nXOrg +subregion_decode->nXLength) < param->format.image.nFrameWidth);

		}

		if((subregion_decode->nYOrg +subregion_decode->nYLength) > param->format.image.nFrameHeight){
			g_critical("The property subregion-top plus subregion-height must be less than the height of the image.");			g_assert((subregion_decode->nYOrg +subregion_decode->nYLength) < param->format.image.nFrameHeight);

		}

	}

	/* inport */
	{
		OMX_PARAM_PORTDEFINITIONTYPE *param;

		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		param = GOO_PORT_GET_DEFINITION (port);

		param->format.image.cMIMEType = "image/jpeg";
		param->format.image.eCompressionFormat = OMX_IMAGE_CodingJPEG;

		param->nBufferCountActual = MAX_NUM_BUFFERS;

		g_assert (param->format.image.nFrameWidth > 0);
		g_assert (param->format.image.nFrameHeight > 0);

		color = param->format.image.eColorFormat;

		if (color == OMX_COLOR_FormatYCbYCr ||
		    /*color == OMX_COLOR_FormatYUV444Interleaved ||*/
		    color == OMX_COLOR_FormatUnused ||
		    color == OMX_COLOR_FormatCbYCrY)
		{
			param->format.image.eColorFormat =
				OMX_COLOR_FormatCbYCrY;
		}
		else if (color == OMX_COLOR_FormatYUV444Interleaved)
		{
			param->format.image.eColorFormat =
				OMX_COLOR_FormatYUV444Interleaved;
		}
		else
		{
			param->format.image.eColorFormat =
				OMX_COLOR_FormatYUV420PackedPlanar;
		}

		g_assert (param->nBufferSize > 0);

		param->format.image.nFrameWidth =
			GOO_ROUND_UP_16 (param->format.image.nFrameWidth);
		param->format.image.nFrameHeight =
			GOO_ROUND_UP_16 (param->format.image.nFrameHeight);

		width = param->format.image.nFrameWidth;
		height = param->format.image.nFrameHeight;

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* outport */
	{
		OMX_PARAM_PORTDEFINITIONTYPE *param;

		GooIterator* iter =
			goo_component_iterate_output_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		param = GOO_PORT_GET_DEFINITION (port);

		param->format.image.cMIMEType = "video/x-raw-yuv";
		param->format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;

		param->nBufferCountActual = MAX_NUM_BUFFERS;

		param->format.image.nFrameWidth =
			(guint) width * scalefactor;
		param->format.image.nFrameHeight =
			(guint) height * scalefactor;

		param->format.image.nFrameWidth =
			GOO_ROUND_UP_16 (param->format.image.nFrameWidth);
		param->format.image.nFrameHeight =
			GOO_ROUND_UP_16 (param->format.image.nFrameHeight);

		g_assert (param->format.image.nFrameWidth > 0);
		g_assert (param->format.image.nFrameHeight > 0);

		width = param->format.image.nFrameWidth;
		height = param->format.image.nFrameHeight;

		/** @fix known issue in OMX/SN **/
		if (color == OMX_COLOR_FormatYUV444Interleaved &&
		    param->format.image.eColorFormat ==
		    OMX_COLOR_FormatYUV420PackedPlanar)
		{
			GOO_OBJECT_ERROR (self, "Cannot output 420Planar when the input is 444Interleaved (known issue)");
			g_assert (FALSE);
		}

		switch (param->format.image.eColorFormat)
		{
		case OMX_COLOR_FormatYUV420PackedPlanar:
			if (color == OMX_COLOR_FormatYUV420PackedPlanar ||
			    color == OMX_COLOR_FormatYUV411Planar)
			{
				param->nBufferSize = width * height * 3 / 2;

			}
			else
			{
				g_assert_not_reached ();
			}
			break;
		case OMX_COLOR_FormatCbYCrY:
		case OMX_COLOR_Format16bitRGB565:
			param->nBufferSize = width * height * 2;
			break;
		case OMX_COLOR_Format24bitRGB888:
			param->nBufferSize = width * height * 3;
			break;
		case OMX_COLOR_Format32bitARGB8888:
			param->nBufferSize = width * height * 4;
			break;
		default:
			g_assert_not_reached ();
			break;
		}

		_goo_ti_jpegdec_set_outcolor
			(self, param->format.image.eColorFormat);

		g_assert (param->nBufferSize > 0);

		g_object_unref (iter);
		g_object_unref (port);
	}

	GOO_OBJECT_DEBUG (self, "");


	return;
}

static void
goo_ti_jpegdec_set_property (GObject* object, guint prop_id,
			     const GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_JPEGDEC (object));

	GooTiJpegDec* self = GOO_TI_JPEGDEC (object);

	switch (prop_id)
	{
	case PROP_PROGRESSIVE:
		_goo_ti_jpegdec_set_progressive (self,
						 g_value_get_boolean (value));
		break;
	case PROP_SCALE:
		_goo_ti_jpegdec_set_scale (self, g_value_get_enum (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_jpegdec_get_property (GObject* object, guint prop_id,
			     GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_JPEGDEC (object));

	GooTiJpegDec* self = GOO_TI_JPEGDEC (object);

	switch (prop_id)
	{
	case PROP_PROGRESSIVE:
		g_value_set_boolean (value, self->progressive);
		break;
	case PROP_SCALE:
		g_value_set_enum (value, self->scale);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}

static void
goo_ti_jpegdec_init (GooTiJpegDec* self)
{
	GOO_COMPONENT (self)->id = g_strdup (ID);
	GOO_COMPONENT (self)->port_param_type = OMX_IndexParamImageInit;

	self->progressive = FALSE;
	self->scale = GOO_TI_JPEGDEC_SCALE_NONE;

	return;
}

static void
goo_ti_jpegdec_class_init (GooTiJpegDecClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

	g_klass->set_property = goo_ti_jpegdec_set_property;
	g_klass->get_property = goo_ti_jpegdec_get_property;

	GParamSpec* spec;
	spec = g_param_spec_boolean ("progressive", "Progressive",
				     "Enable progressive decoding mode",
				     FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_PROGRESSIVE, spec);

	spec = g_param_spec_enum ("scale", "Scale factor",
				  "Scale factor resize",
				  GOO_TI_JPEGDEC_SCALE,
				  GOO_TI_JPEGDEC_SCALE_NONE,
				  G_PARAM_READWRITE);
	g_object_class_install_property (g_klass, PROP_SCALE, spec);

	GooComponentClass* c_klass = GOO_COMPONENT_CLASS (klass);
        c_klass->load_parameters_func = goo_ti_jpegdec_load_parameters;
        c_klass->set_parameters_func = goo_ti_jpegdec_set_parameters;
	c_klass->validate_ports_definition_func = goo_ti_jpegdec_validate;

	return;
}
