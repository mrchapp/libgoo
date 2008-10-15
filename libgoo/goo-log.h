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

#ifndef __GOO_LOG_H__
#define __GOO_LOG_H__

#include <glib.h>

/**
 * GOO_DEBUG_LEVEL:
 *
 * String that specifies the environment variable name to set the debug level
 **/
#define GOO_DEBUG_LEVEL "GOO_DEBUG_LEVEL"


typedef enum _GooLogLevel GooLogLevel;
/**
 * GooLogLevel:
 * @GOO_LOG_UNDEFINED: special log level when the system is not initalized yet.
 * @GOO_LOG_NONE: No debug level specified or desired. Used to deactivate debugging
 *  output.
 * @GOO_LOG_ERROR: Error messages are to be used only when an error ocurred that
 *  stops the application from keeping working correctly.
 * @GOO_LOG_WARNING: Warning messages ar to inform about abnormal behaviour that
 *  could lead to problems or weird behaviour later on.
 * @GOO_LOG_INFO: Informational messages should be used to keep the developer updated
 *  about what is happening.
 * @GOO_LOG_DEBUG: Debugging messages should be used when something common happens
 *  thant is no the expected default behavior.
 */
enum _GooLogLevel
{
     GOO_LOG_UNDEFINED = -1,
     GOO_LOG_NONE = 0,
     GOO_LOG_ERROR,
     GOO_LOG_WARNING,
     GOO_LOG_INFO,
     GOO_LOG_NOTICE,
     GOO_LOG_DEBUG
};

void goo_log (guint level, const gchar *file, const gchar *function,
              gint line, const gchar *fmt, ...);

void goo_object_log (const gchar* objname, const gchar* prefix, guint level,
		     const gchar* file, const gchar* function, gint line,
		     const gchar* fmt, ...);

/**
 * GOO_LOG:
 * @level: the #GooLogLevel of the message
 * @...: printf-style message to output
 *
 * Macro for goo_log() function
 **/
#define GOO_LOG(level, ...) goo_log ((level), __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__);


/**
 * GOO_DEBUG:
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_DEBUG(...)   GOO_LOG(GOO_LOG_DEBUG, __VA_ARGS__)
/**
 * GOO_NOTICE:
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_NOTICE(...)  GOO_LOG(GOO_LOG_NOTICE, __VA_ARGS__)
/**
 * GOO_INFO:
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_INFO(...)    GOO_LOG(GOO_LOG_INFO, __VA_ARGS__)
/**
 * GOO_WARNING:
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_WARNING(...) GOO_LOG(GOO_LOG_WARNING, __VA_ARGS__)
/**
 * GOO_ERROR:
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_ERROR(...)   GOO_LOG(GOO_LOG_ERROR, __VA_ARGS__)

/**
 * GOO_OBJECT_LOG:
 * @obj: The #GooObject which generates the message
 * @level: the #GooLogLevel of the message
 * @...: printf-style message to output
 *
 * Macro for goo_object_log() function
 **/
#define GOO_OBJECT_LOG(obj, level, ...) goo_object_log (GOO_OBJECT_NAME (obj), GOO_OBJECT_PREFIX (obj), (level), __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__);

/**
 * GOO_OBJECT_DEBUG:
 * @obj: The #GooObject instance which generates the message
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_OBJECT_DEBUG(obj, ...)   GOO_OBJECT_LOG(obj, GOO_LOG_DEBUG, __VA_ARGS__)
/**
 * GOO_OBJECT_NOTICE:
 * @obj: The #GooObject instance which generates the message
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_OBJECT_NOTICE(obj, ...)  GOO_OBJECT_LOG(obj, GOO_LOG_NOTICE, __VA_ARGS__)
/**
 * GOO_OBJECT_INFO:
 * @obj: The #GooObject instance which generates the message
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_OBJECT_INFO(obj, ...)    GOO_OBJECT_LOG(obj, GOO_LOG_INFO, __VA_ARGS__)
/**
 * GOO_OBJECT_WARNING:
 * @obj: The #GooObject instance which generates the message
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_OBJECT_WARNING(obj, ...) GOO_OBJECT_LOG(obj, GOO_LOG_WARNING, __VA_ARGS__)
/**
 * GOO_OBJECT_ERROR:
 * @obj: The #GooObject instance which generates the message
 * @...: printf-style message to output
 *
 * Output a logging message.
 */
#define GOO_OBJECT_ERROR(obj, ...)   GOO_OBJECT_LOG(obj, GOO_LOG_ERROR, __VA_ARGS__)

#endif /* __GOO_LOG_H__ */
