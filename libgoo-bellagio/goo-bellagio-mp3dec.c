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

#include <goo-bellagio-mp3dec.h>
#include <goo-utils.h>

#define ID "OMX.st.audio_decoder.mp3.mad"
#define INPUT_BUFFERSIZE 4 * 1024
#define OUTPUT_BUFFERSIZE 8 * 1024

G_DEFINE_TYPE (GooBellagioMp3Dec, goo_bellagio_mp3dec, GOO_TYPE_COMPONENT)

static void
goo_bellagio_mp3dec_init (GooBellagioMp3Dec* self)
{
	GOO_COMPONENT (self)->id = g_strdup (ID);
	GOO_COMPONENT (self)->port_param_type = OMX_IndexParamAudioInit;

	self->param = NULL;

	return;
}

static void
goo_bellagio_mp3dec_finalize (GObject* object)
{
	g_assert (GOO_IS_BELLAGIO_MP3DEC (object));
	GooBellagioMp3Dec* self = GOO_BELLAGIO_MP3DEC (object);

	if (G_LIKELY (self->param))
	{
		g_free (self->param);
		self->param = NULL;
	}

	(*G_OBJECT_CLASS (goo_bellagio_mp3dec_parent_class)->finalize) (object);

	return;
}

static void
goo_bellagio_mp3dec_load_parameters (GooComponent* component)
{
	g_assert (GOO_IS_BELLAGIO_MP3DEC (component));
	GooBellagioMp3Dec* self = GOO_BELLAGIO_MP3DEC (component);
	g_assert (self->param == NULL);
	g_assert (component->cur_state != OMX_StateInvalid);

	GOO_OBJECT_DEBUG (self, "");

	self->param = g_new0 (OMX_AUDIO_PARAM_PCMMODETYPE, 1);
	GOO_INIT_PARAM (self->param, OMX_AUDIO_PARAM_PCMMODETYPE);

	goo_component_get_parameter_by_index (component,
					      OMX_IndexParamAudioPcm,
					      self->param);

	return;
}

static void
goo_bellagio_mp3dec_set_parameters (GooComponent* component)
{
	g_assert (GOO_IS_BELLAGIO_MP3DEC (component));
	GooBellagioMp3Dec* self = GOO_BELLAGIO_MP3DEC (component);
	g_assert (self->param != NULL);
	g_assert (component->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "");

	goo_component_set_parameter_by_index (component,
					      OMX_IndexParamAudioPcm,
					      self->param);
	return;
}

static void
goo_bellagio_mp3dec_validate_ports_definitions (GooComponent* component)
{
	g_assert (GOO_IS_BELLAGIO_MP3DEC (component));
	GooBellagioMp3Dec* self = GOO_BELLAGIO_MP3DEC (component);
	g_assert (self->param != NULL);
	g_assert (component->cur_state == OMX_StateLoaded);

	GOO_OBJECT_DEBUG (self, "");

	/* input */
	{
		GooIterator* iter =
			goo_component_iterate_input_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		GOO_PORT_GET_DEFINITION (port)->nBufferSize = INPUT_BUFFERSIZE;
		GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding =
			OMX_AUDIO_CodingMP3;

		g_object_unref (iter);
		g_object_unref (port);
	}

	/* output */
	{
		GooIterator* iter =
			goo_component_iterate_output_ports (component);
		goo_iterator_nth (iter, 0);
		GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
		g_assert (port != NULL);

		GOO_PORT_GET_DEFINITION (port)->nBufferSize =
			OUTPUT_BUFFERSIZE;
		GOO_PORT_GET_DEFINITION (port)->format.audio.eEncoding =
			OMX_AUDIO_CodingPCM;

		g_object_unref (iter);
		g_object_unref (port);
	}

	return;
}

static void
goo_bellagio_mp3dec_class_init (GooBellagioMp3DecClass* klass)
{
	GObjectClass* g_klass = G_OBJECT_CLASS (klass);
	g_klass->finalize = goo_bellagio_mp3dec_finalize;

	GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
	o_klass->load_parameters_func = goo_bellagio_mp3dec_load_parameters;
	o_klass->set_parameters_func = goo_bellagio_mp3dec_set_parameters;
	o_klass->validate_ports_definition_func =
		goo_bellagio_mp3dec_validate_ports_definitions;

	return;
}
