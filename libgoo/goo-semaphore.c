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

#include <goo-semaphore.h>
#include <goo-log.h>

#define SEM_TMOUT 30 * G_USEC_PER_SEC

/**
 * goo_semaphore_new:
 * @counter: the semaphore's counter initial value
 *
 * Creates a new semaphore for threading syncronization
 *
 * Returns: a new #GooSemaphore structure
 */
GooSemaphore*
goo_semaphore_new (gint counter)
{
        g_assert (counter <= 0);
        GooSemaphore *self;

        self = g_new (GooSemaphore, 1);
        g_assert (self != NULL);

        self->counter = counter;

        self->condition = g_cond_new ();
        self->mutex = g_mutex_new ();

        return self;
}

/**
 * goo_semaphore_free:
 * @self: an #GooSemaphore structure
 *
 * Free the structure
 */
void
goo_semaphore_free (GooSemaphore *self)
{
        g_assert (self != NULL);

        g_cond_free (self->condition);
        g_mutex_free (self->mutex);
        g_free (self);

        return;
}

/**
 * goo_semaphore_down:
 * @self: an #GooSemaphore structure
 * @timeout: TRUE for a timed semaphore, FALSE for an untimed semaphore
 *
 * While the counter is 0, the process will be stopped, waiting for
 * an up signal
 */
void
goo_semaphore_down (GooSemaphore *self, gboolean timeout)
{
        g_assert (self != NULL);

        g_mutex_lock (self->mutex);

        while (self->counter == 0)
        {
                if (timeout)
                {
                        GTimeVal time;
                        g_get_current_time (&time);
                        g_time_val_add (&time, SEM_TMOUT);

                        g_assert (
                                g_cond_timed_wait (self->condition,
                                                   self->mutex, &time) == TRUE
                                );
                }
                else
                {
                        g_cond_wait (self->condition, self->mutex);
                }
        }

        self->counter--;

        g_mutex_unlock (self->mutex);

        return;
}

/**
 * goo_semaphore_up:
 * @self: an #GooSemaphore structure
 *
 * Increments the semaphore's counter and signals the condition for thread
 * execution continuation.
 */
void
goo_semaphore_up (GooSemaphore *self)
{
        g_assert (self != NULL);

        g_mutex_lock (self->mutex);
        self->counter++;
        g_cond_signal (self->condition);
        g_mutex_unlock (self->mutex);

        return;
}
