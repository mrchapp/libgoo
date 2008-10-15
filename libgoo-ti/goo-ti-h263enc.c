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

#include <goo-ti-h263enc.h>
#include <goo-utils.h>

G_DEFINE_TYPE (GooTiH263Enc, goo_ti_h263enc, GOO_TYPE_TI_VIDEO_ENCODER)

enum _GooTiH263EncProp
{
        PROP_0,
	PROP_LEVEL
};

GType
goo_ti_h263enc_level_get_type ()
{
	static GType goo_ti_h263enc_level_type = 0;
	static GEnumValue goo_ti_h263enc_level[] =
	{
		{OMX_VIDEO_H263Level10, "10", "Level 10"},
		{OMX_VIDEO_H263Level20, "20", "Level 20"},
		{OMX_VIDEO_H263Level30, "30", "Level 30"},
		{OMX_VIDEO_H263Level40, "40", "Level 40"},
		{OMX_VIDEO_H263Level45, "45", "Level 45"},
		{OMX_VIDEO_H263Level50, "50", "Level 50"},
		{OMX_VIDEO_H263Level60, "60", "Level 60"},
		{OMX_VIDEO_H263Level70, "70", "Level 70"},
		{ 0, NULL, NULL },
	};

	if (!goo_ti_h263enc_level_type)
	{
		goo_ti_h263enc_level_type =
			g_enum_register_static ("GooTiH263EncLevel",
						goo_ti_h263enc_level);
	}

	return goo_ti_h263enc_level_type;
}

static void
goo_ti_h263enc_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_H263ENC (component));
	GooTiH263Enc* self = GOO_TI_H263ENC (component);

	GOO_OBJECT_DEBUG (self, "Entering");

	/* Call the parent method to load video encoder parameters */
	(*GOO_COMPONENT_CLASS (goo_ti_h263enc_parent_class)->load_parameters_func) (component);

	g_assert (self->level_param == NULL);

	self->level_param = g_new0 (OMX_VIDEO_PARAM_H263TYPE, 1);
	GOO_INIT_PARAM (self->level_param, OMX_VIDEO_PARAM_H263TYPE);

	self->level_param->nPortIndex = 1;

	/* @Todo: Set a property for the Gob Header interval */
	self->level_param->nGOBHeaderInterval = 0;

	goo_component_get_parameter_by_index (component,
					      OMX_IndexParamVideoH263,
					      self->level_param);


	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

static void
goo_ti_h263enc_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_TI_H263ENC (component));
	GooTiH263Enc* self = GOO_TI_H263ENC (component);

	GOO_OBJECT_DEBUG (self, "Entering");

	g_assert (component->cur_state == OMX_StateLoaded);

	/* Call the parent method to set video encoder parameters */
	(*GOO_COMPONENT_CLASS (goo_ti_h263enc_parent_class)->set_parameters_func) (component);

	goo_component_set_parameter_by_index (component,
				      OMX_IndexParamVideoH263,
				      self->level_param);

	GOO_OBJECT_DEBUG (self, "Exit");

	return;

}

static void
goo_ti_h263enc_validate_ports_definitions (GooComponent* component)
{
	g_assert (GOO_IS_TI_H263ENC (component));
        GooTiH263Enc* self = GOO_TI_H263ENC (component);

	GOO_OBJECT_DEBUG (self, "Entering");

	(*GOO_COMPONENT_CLASS (goo_ti_h263enc_parent_class)->validate_ports_definition_func) (component);

	GOO_OBJECT_DEBUG (self, "Exit");

	return;
}

static void
goo_ti_h263enc_set_property (GObject* object, guint prop_id,
                                     const GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_H263ENC (object));
        GooTiH263Enc* self = GOO_TI_H263ENC (object);

        switch (prop_id)
        {
        case PROP_LEVEL:
		self->level_param->eLevel = g_value_get_enum (value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
                break;
	}

	return;
}

static void
goo_ti_h263enc_get_property (GObject* object, guint prop_id,
                                     GValue* value, GParamSpec* spec)
{
        g_assert (GOO_IS_TI_H263ENC (object));
        GooTiH263Enc* self = GOO_TI_H263ENC (object);

        switch (prop_id)
        {
        case PROP_LEVEL:
		g_value_set_enum (value, self->level_param->eLevel);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
                break;
	}

	return;
}


static void
goo_ti_h263enc_init (GooTiH263Enc* self)
{
        return;
}

static void
goo_ti_h263enc_finalize (GObject* object)
{
	g_assert (GOO_IS_TI_H263ENC (object));
	GooTiH263Enc* self = GOO_TI_H263ENC (object);

	GOO_OBJECT_DEBUG (self, "Entering");

	if (G_LIKELY (self->level_param))
	{
		GOO_OBJECT_DEBUG (self, "Freeing the level param structure");

		g_free (self->level_param);
		self->level_param = NULL;
	}

	(*G_OBJECT_CLASS (goo_ti_h263enc_parent_class)->finalize) (object);

	return;
}

static void
goo_ti_h263enc_class_init (GooTiH263EncClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);

  	g_klass->set_property = goo_ti_h263enc_set_property;
	g_klass->get_property = goo_ti_h263enc_get_property;
	g_klass->finalize = goo_ti_h263enc_finalize;

	GParamSpec* spec;
	spec = g_param_spec_enum ("level", "Level",
		"Sets the encoding level.",
		GOO_TI_H263ENC_LEVEL,
		OMX_VIDEO_H263Level10, G_PARAM_READWRITE);

	g_object_class_install_property (g_klass, PROP_LEVEL, spec);

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->set_parameters_func = goo_ti_h263enc_set_parameters;
	o_klass->load_parameters_func = goo_ti_h263enc_load_parameters;
	o_klass->validate_ports_definition_func =
	goo_ti_h263enc_validate_ports_definitions;

	return;
}






