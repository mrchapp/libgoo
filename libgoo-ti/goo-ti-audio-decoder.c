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

#include <goo-ti-audio-decoder.h>
#include <goo-utils.h>

enum _GooTiAudioDecoderProp
{
	PROP_0,
};


G_DEFINE_TYPE (GooTiAudioDecoder, goo_ti_audio_decoder,
	       GOO_TYPE_TI_AUDIO_COMPONENT)

static void
goo_ti_audio_decoder_init (GooTiAudioDecoder* self)
{
	/* noop */
	return;
}

static void
goo_ti_audio_decoder_class_init (GooTiAudioDecoderClass* klass)
{
	GooTiAudioComponentClass *ac_klass =
		GOO_TI_AUDIO_COMPONENT_CLASS (klass);

	ac_klass->iterate_ports_func = goo_ti_audio_decoder_iterate_ports;

	return;
}

GooIterator*
goo_ti_audio_decoder_iterate_ports (GooTiAudioComponent *self)
{
	GOO_OBJECT_DEBUG (self, "Enter");
	g_assert (GOO_IS_TI_AUDIO_COMPONENT (self));
	GooIterator *iter;

	GooComponent *component = GOO_COMPONENT (self);

	iter = goo_component_iterate_output_ports (component);
	g_assert (iter != NULL);
	GOO_OBJECT_DEBUG (self, "Exit");

	return iter;

}

