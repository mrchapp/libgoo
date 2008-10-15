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

#ifndef _GOO_ENGINE_H_
#define _GOO_ENGINE_H_

#include <goo-component.h>
#include <stdio.h>

G_BEGIN_DECLS



#define GOO_TYPE_ENGINE \
        (goo_engine_get_type ())
#define GOO_ENGINE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_ENGINE, GooEngine))
#define GOO_ENGINE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_ENGINE, GooEngineClass))
#define GOO_IS_ENGINE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_ENGINE))
#define GOO_IS_ENGINE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_ENGINE))
#define GOO_ENGINE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_ENGINE, GooEngineClass))

typedef enum
{
	GOO_ENGINE_UNKNOWN,
	GOO_ENGINE_SINK,
	GOO_ENGINE_SRC,
	GOO_ENGINE_FILTER
} GooEngineType;

typedef struct _GooEngine GooEngine;
typedef struct _GooEngineClass GooEngineClass;

/**
 * GooEngine:
 *
 * A simple object for testing OpenMAX components
 */
struct _GooEngine
{
        GooObject parent;

        GooComponent* component;

        GooPort* inport;
        FILE* instream;
	gchar* infile;

        GooPort* outport;
        FILE* outstream;
	gchar* outfile;

        gint incount, outcount, numbuffers;

	/* hack for camera-jpegenc tunnel */
	gboolean mainloop;

	/* hack for wma decoder and postprocessor */
	gboolean eosevent;

	/* Hack for supporting frames in GOO checks */
	gboolean vopparser;
	FILE* vopstream;
	gchar* vopfile;

	GooEngineType enginetype;
};

struct _GooEngineClass
{
        GooObjectClass parent_class;
};

GType goo_engine_get_type (void);
GooEngine* goo_engine_new (GooComponent *component,
                           gchar* infile, gchar* outfile);
GooEngine* goo_engine_new_vop (GooComponent *component,
			       gchar* infile, gchar* outfile,
			       gboolean vopparser);
void goo_engine_play (GooEngine* self);

#endif /* _GOO_ENGINE_H_ */
