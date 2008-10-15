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

#ifndef _GOO_SEMAPHORE_H_
#define _GOO_SEMAPHORE_H_

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GooSemaphore GooSemaphore;

/**
 * GooSemaphore:
 *
 * A simple semaphore for threads syncronization
 */
struct _GooSemaphore
{
        GCond *condition;
        GMutex *mutex;
        gint counter;
};

GooSemaphore* goo_semaphore_new (gint counter);
void goo_semaphore_free (GooSemaphore* self);
void goo_semaphore_down (GooSemaphore* self, gboolean timeout);
void goo_semaphore_up   (GooSemaphore* self);

G_END_DECLS

#endif /* _GOO_SEMAPHORE_H_ */
