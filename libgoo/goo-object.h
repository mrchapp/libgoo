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

#ifndef _GOO_OBJECT_H_
#define _GOO_OBJECT_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define GOO_TYPE_OBJECT \
     (goo_object_get_type())
#define GOO_OBJECT(obj) \
     (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_OBJECT, GooObject))
#define GOO_OBJECT_CLASS(klass) \
     (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_OBJECT, GooObjectClass))
#define GOO_IS_OBJECT(obj) \
     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_OBJECT))
#define GOO_IS_OBJECT_CLASS(klass) \
     (G_TYPE_CHECK_CLASS_TYPE ((obj), GOO_TYPE_OBJECT))
#define GOO_OBJECT_GET_CLASS(obj) \
     (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_OBJECT, GooObjectClass))

typedef struct _GooObject GooObject;
typedef struct _GooObjectClass GooObjectClass;

/**
 * GooObject:
 * @name: the object's name. It is used for logging purposes.
 * @owner: the object who owns this object (component/port relationship)
 *
 * The #GooObject structure. Use the functions to update the variables.
 */
struct _GooObject
{
	GObject parent;

	GMutex* lock;
	gchar* name;
	gchar* prefix;
	GooObject* owner;
};

struct _GooObjectClass
{
	GObjectClass parent;
};

GType goo_object_get_type (void);

gchar* goo_object_get_name (GooObject* self);
void goo_object_set_name (GooObject* self, const gchar* name);
GooObject* goo_object_get_owner (GooObject* self);
void goo_object_set_owner (GooObject* self, GooObject* owner);

/**
 * GOO_OBJECT_NAME:
 * @obj: An #GooObject instance
 *
 * Returns the pointer to the the object's name string. Be careful.
 *
 * Return value: The pointer to the object's name. It is NOT MT safe.
 */
#define GOO_OBJECT_NAME(obj) (GOO_OBJECT(obj)->name)

/**
 * GOO_OBJECT_PREFIX:
 * @obj: An #GooObject instance
 *
 * Returns the pointer to the the object's prefix string. Be careful.
 *
 * Return value: The pointer to the object's name. It is NOT MT safe.
 */
#define GOO_OBJECT_PREFIX(obj) (GOO_OBJECT(obj)->prefix)


/**
 * GOO_OBJECT_OWNER:
 * @obj: An #GooObject instance
 *
 * Access to the owner's object memory address. Be careful
 *
 * Return value: The pointer to the object's owner. It is NOT MT safe.
 */
#define GOO_OBJECT_OWNER(obj) (GOO_OBJECT(obj)->owner)

/**
 * GOO_OBJECT_REFCOUNT:
 * @obj: a #GooObject
 *
 * Get access to the reference count fiel of the object.
 */
#define GOO_OBJECT_REFCOUNT(obj) ((G_OBJECT (obj)->ref_count))

/**
 * GOO_OBJECT_GET_LOCK:
 * @obj: a #GooObject
 *
 * Acquire a reference to the mutex of this object.
 */
#define GOO_OBJECT_GET_LOCK(obj) (GOO_OBJECT (obj)->lock)

/**
 * GOO_OBJECT_LOCK:
 * @obj: a #GooObject
 *
 * This macro will obtain a lock on the object, making seralization possible
 * It blocks until the lock can be obtanied.
 */
#define GOO_OBJECT_LOCK(obj) g_mutex_lock (GOO_OBJECT_GET_LOCK (obj))

/**
 * GOO_OBJECT_UNLOCK:
 * @obj: a #GooObject
 *
 * This macro releases a lock on the object.
 */
#define GOO_OBJECT_UNLOCK(obj) g_mutex_unlock (GOO_OBJECT_GET_LOCK (obj))

G_END_DECLS

#endif /* _GOO_OBJECT_H_ */
