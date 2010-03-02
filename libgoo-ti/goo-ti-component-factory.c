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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-ti-component-factory.h>
#include <goo-utils.h>
#include <goo-component-list.h>

#include <goo-ti-clock.h>
#ifdef TI_CAMERA
#include <goo-ti-camera.h>
#endif

#include <goo-ti-wbamrdec.h>
#include <goo-ti-pcmenc.h>
#include <goo-ti-nbamrdec.h>
#include <goo-ti-nbamrenc.h>
#include <goo-ti-gsmhrenc.h>
#include <goo-ti-wbamrenc.h>
#include <goo-ti-gsmfrenc.h>
#include <goo-ti-aacenc.h>
#include <goo-ti-mp3dec.h>
#include <goo-ti-wmadec.h>
#include <goo-ti-pcmdec.h>
#include <goo-ti-imaadpcmdec.h>
#include <goo-ti-gsmhrdec.h>
#include <goo-ti-g722dec.h>
#include <goo-ti-g711dec.h>
#include <goo-ti-aacdec.h>
#include <goo-ti-mpeg4dec.h>
#include <goo-ti-mpeg4dec-720p.h>
#include <goo-ti-sparkdec.h>
#include <goo-ti-mpeg4enc.h>
#include <goo-ti-mpeg4enc-720p.h>
#include <goo-ti-post-processor.h>
#include <goo-ti-jpegenc.h>
#include <goo-ti-h264dec.h>
#include <goo-ti-h264dec-720p.h>
#include <goo-ti-h263dec.h>
#include <goo-ti-wmvdec.h>
#include <goo-ti-h264enc.h>
#include <goo-ti-vpp.h>
#include <goo-ti-h263enc.h>
#include <goo-ti-jpegdec.h>
#include <goo-ti-mpeg2dec.h>
#include <goo-ti-gsmfrdec.h>
#include <goo-ti-armaacdec.h>
#include <goo-ti-armaacenc.h>

struct _component_entry
{
	gchar* name;
	guint id;
	GType (*type) (void);
	guint count;
};

static struct _component_entry _components[] =
{
    { "armaacenc", GOO_TI_ARMAAC_ENCODER, goo_ti_armaacenc_get_type, 0 },
    { "armaacdec", GOO_TI_ARMAAC_DECODER, goo_ti_armaacdec_get_type, 0 },
	{ "wbamrdec", GOO_TI_WBAMR_DECODER, goo_ti_wbamrdec_get_type, 0 },
	{ "gsmfrdec", GOO_TI_GSMFR_DECODER, goo_ti_gsmfrdec_get_type, 0 },
	{ "nbamrdec", GOO_TI_NBAMR_DECODER, goo_ti_nbamrdec_get_type, 0 },
	{ "pcmenc", GOO_TI_PCM_ENCODER, goo_ti_pcmenc_get_type, 0 },
	{ "nbamrenc", GOO_TI_NBAMR_ENCODER, goo_ti_nbamrenc_get_type, 0 },
	{ "gsmhrenc", GOO_TI_GSMHR_ENCODER, goo_ti_gsmhrenc_get_type, 0 },
	{ "wbamrenc", GOO_TI_WBAMR_ENCODER, goo_ti_wbamrenc_get_type, 0 },
	{ "gsmfrenc", GOO_TI_GSMFR_ENCODER, goo_ti_gsmfrenc_get_type, 0 },
	{ "aacenc", GOO_TI_AAC_ENCODER, goo_ti_aacenc_get_type, 0 },
	{ "mp3dec", GOO_TI_MP3_DECODER, goo_ti_mp3dec_get_type, 0 },
	{ "wmadec", GOO_TI_WMA_DECODER, goo_ti_wmadec_get_type, 0 },
	{ "pcmdec", GOO_TI_PCM_DECODER, goo_ti_pcmdec_get_type, 0 },
	{ "imaadpcmdec", GOO_TI_IMAADPCM_DECODER, goo_ti_imaadpcmdec_get_type, 0 },
	{ "gsmhrdec", GOO_TI_GSMHR_DECODER, goo_ti_gsmhrdec_get_type, 0 },
	{ "g722dec", GOO_TI_G722_DECODER, goo_ti_g722dec_get_type, 0 },
	{ "g711dec", GOO_TI_G711_DECODER, goo_ti_g711dec_get_type, 0 },
	{ "aacdec", GOO_TI_AAC_DECODER, goo_ti_aacdec_get_type, 0 },
	{ "mpeg4dec", GOO_TI_MPEG4_DECODER, goo_ti_mpeg4dec_get_type, 0 },
	{ "mpeg4dec720p", GOO_TI_MPEG4_720P_DECODER, goo_ti_mpeg4dec_720p_get_type, 0 },
	{ "sparkdec", GOO_TI_SPARK_DECODER, goo_ti_sparkdec_get_type, 0 },
	{ "h264dec", GOO_TI_H264_DECODER, goo_ti_h264dec_get_type, 0 },
	{ "h264720pdec", GOO_TI_H264_720P_DECODER, goo_ti_h264dec_720p_get_type, 0 },
	{ "h263dec", GOO_TI_H263_DECODER, goo_ti_h263dec_get_type, 0 },
	{ "mpeg4enc", GOO_TI_MPEG4_ENCODER, goo_ti_mpeg4enc_get_type, 0 },
	{ "mpeg4enc720p", GOO_TI_MPEG4_720P_ENCODER, goo_ti_mpeg4enc_720p_get_type, 0 },
	{ "postproc", GOO_TI_POST_PROCESSOR, goo_ti_post_processor_get_type, 0 },
#ifdef TI_CAMERA
	{ "camera", GOO_TI_CAMERA, goo_ti_camera_get_type, 0 },
#endif
	{ "jpegenc", GOO_TI_JPEG_ENCODER, goo_ti_jpegenc_get_type, 0 },
	{ "wmvdec", GOO_TI_WMV_DECODER, goo_ti_wmvdec_get_type, 0 },
	{ "h264enc", GOO_TI_H264_ENCODER, goo_ti_h264enc_get_type, 0 },
	{ "h263enc", GOO_TI_H263_ENCODER, goo_ti_h263enc_get_type, 0 },
	{ "jpegdec", GOO_TI_JPEG_DECODER, goo_ti_jpegdec_get_type, 0 },
	{ "vpp", GOO_TI_VPP, goo_ti_vpp_get_type, 0 },
	{ "mpeg2dec", GOO_TI_MPEG2_DECODER, goo_ti_mpeg2dec_get_type, 0 },
	{ NULL, 0 },
};

G_DEFINE_TYPE (GooTiComponentFactory, goo_ti_component_factory,
	       GOO_TYPE_COMPONENT_FACTORY);

static void
goo_ti_component_factory_create_clock (GooTiComponentFactory* self)
{
	g_assert (self->clock == NULL);

#if TI_CLOCK
	self->clock = g_object_new (GOO_TYPE_TI_CLOCK, NULL);
	g_assert (self->clock != NULL);
	goo_object_set_name (GOO_OBJECT (self->clock), "clock0");
	if (goo_component_load (self->clock) != TRUE)
	{
		GOO_WARNING ("Could not load the clock!");
		g_object_unref (self->clock);
		self->clock = NULL;
	}
	else
	{
		goo_object_set_owner (GOO_OBJECT (self->clock),
				      GOO_OBJECT (self));
	}
#endif

	GOO_DEBUG ("");

	return;
}

static void
goo_ti_component_factory_destroy_clock (GooTiComponentFactory* self)
{
	if (G_LIKELY (self->clock != NULL))
	{
		g_object_unref (self->clock);
	}

	GOO_OBJECT_DEBUG (self, "");

	return;
}

static GooComponent*
goo_ti_component_factory_get_component (GooComponentFactory *self,
					guint type)
{
	GOO_OBJECT_DEBUG (self, "");

	struct _component_entry *my_components = _components;
	GooComponent *component = NULL;
	GooTiComponentFactory* me = GOO_TI_COMPONENT_FACTORY (self);
#ifdef TI_CLOCK
	if (type == GOO_TI_CLOCK)
	{
		/* clock is singleton */
		if (me->clock == NULL)
		{
			goo_ti_component_factory_create_clock (me);
		}

		return g_object_ref (me->clock);
	}
#endif
	while ((*my_components).name)
	{
		if (type == (*my_components).id)
		{
			component = g_object_new (((*my_components).type) (),
						  NULL);
			break;
		}
		my_components++;
	}

	if (component == NULL)
	{
		GOO_OBJECT_ERROR (self,
				  "The specified component does not exist!");
		return NULL;
	}

	/* set the component name */
	{
		gchar* name = g_strdup_printf ("%s%d", (*my_components).name,
					       (*my_components).count);
		goo_object_set_name (GOO_OBJECT (component), name);
		g_free (name);
		(*my_components).count++;
	}

	if (goo_component_load (component) == TRUE)
	{
#ifdef TI_CLOCK
		if (me->clock != NULL)
		{
			/* goo_component_set_clock (component, me->clock); */
		}
#endif
		goo_component_factory_add_component (self, component);
	}
	else
	{
		g_object_unref (G_OBJECT (component));
		return NULL;
	}

	return component;
}

static void
goo_ti_component_factory_init (GooTiComponentFactory *self)
{
	self->clock = NULL;

	/* goo_ti_component_factory_create_clock (self); */

	return;
}

static void
goo_ti_component_factory_dispose (GObject *object)
{
	GooTiComponentFactory *self = GOO_TI_COMPONENT_FACTORY (object);

	GOO_OBJECT_DEBUG (self, "");

	(*G_OBJECT_CLASS (goo_ti_component_factory_parent_class)->dispose) (object);

	goo_ti_component_factory_destroy_clock (self);

	return;
}

static void
goo_ti_component_factory_class_init (GooTiComponentFactoryClass *klass)
{
	GObjectClass *g_klass = G_OBJECT_CLASS (klass);
	GooComponentFactoryClass *f_klass =
		GOO_COMPONENT_FACTORY_CLASS (klass);

	g_klass->dispose = goo_ti_component_factory_dispose;
	f_klass->get_component_func = goo_ti_component_factory_get_component;

	return;
}

/**
 * goo_ti_component_factory_get_instance:
 *
 * Create a new instance of the #GooComponentFactory if no exist previously.
 * Returns the previously created otherwise.
 *
 * Return value: the #GooComponentFactory singleton instance
 **/
GooComponentFactory*
goo_ti_component_factory_get_instance (void)
{
	GooTiComponentFactory* self =
		g_object_new (GOO_TYPE_TI_COMPONENT_FACTORY, NULL);

	return GOO_COMPONENT_FACTORY (self);
}
