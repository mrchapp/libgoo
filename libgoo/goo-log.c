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

#include <goo-log.h>
#include <goo-utils.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <glib/gprintf.h>

typedef struct _LogDesc LogDesc;
struct _LogDesc
{
	GooLogLevel level;
	gchar *desc;
};

static LogDesc logdesc[] =
{
	{ GOO_LOG_UNDEFINED, "UNDEFINED" },
	{ GOO_LOG_NONE,	     "NONE"	 },
	{ GOO_LOG_ERROR,     "ERROR"	 },
	{ GOO_LOG_WARNING,   "WARNING"	 },
	{ GOO_LOG_INFO,	     "INFO"	 },
	{ GOO_LOG_NOTICE,    "NOTICE"	 },
	{ GOO_LOG_DEBUG,     "DEBUG"	 }
};

static const gchar *
get_log_level_str (GooLogLevel level)
{
	int i;

	for (i = 0; i < GOO_ARRAY_SIZE(logdesc); i++)
	{
		if (level == logdesc[i].level)
		{
			return logdesc[i].desc;
		}
	}

	return "Unknown";
}

/* @todo find a better way */
static guint
get_log_level ()
{
	guint level;
	gchar *tail = NULL;
	const gchar *levelstr = getenv (GOO_DEBUG_LEVEL);

	if (levelstr == NULL)
	{
		return GOO_LOG_NONE;
	}

	while (g_ascii_isspace (*levelstr))
	{
		levelstr++;
	}

	if (*levelstr == 0)
	{
		return GOO_LOG_NONE;
	}

	errno = 0;
	level = strtol (levelstr, &tail, 0);

	if (errno)
	{
		return GOO_LOG_NONE;
	}

	if (level < GOO_LOG_NONE || level > GOO_LOG_DEBUG)
	{
		return GOO_LOG_NONE;
	}

	return level;
}

/**
 * goo_log:
 * @level: log level
 * @file: the file name
 * @function: the function name
 * @line: the code line number
 * @fmt: the message format
 * @...: the message
 *
 * Writes a log message to the stdout
 *
 **/
void
goo_log (guint level, const gchar *file, const gchar *function,
	 gint line, const gchar *fmt, ...)
{
	static gint curlevel = GOO_LOG_UNDEFINED;

	if (curlevel == GOO_LOG_UNDEFINED)
	{
		curlevel = get_log_level ();
	}

	if (level != GOO_LOG_ERROR && curlevel < level)
	{
		return;
	}

	va_list args;
	va_start (args, fmt);

	gchar* tmp = g_strdup_vprintf (fmt, args);
	g_fprintf (stderr, "%s [%d] %s(%d):%s\t%s\n" ,
		   get_log_level_str (level),
		   getpid (), file, line, function, tmp);
	fflush (stderr);

	g_free (tmp);
	va_end (args);
}

/**
 * goo_object_log:
 * @objname: the object's name
 * @level: log level
 * @file: the file name
 * @function: the function name
 * @line: the code line number
 * @fmt: the message format
 * @...: the message
 *
 * Writes a log message to the stdout
 *
 **/
void
goo_object_log (const gchar* objname, const gchar* prefix, guint level,
		const gchar* file, const gchar* function, gint line,
		const gchar* fmt, ...)
{
	static gint curlevel = GOO_LOG_UNDEFINED;

	if (curlevel == GOO_LOG_UNDEFINED)
	{
		curlevel = get_log_level ();
	}

	if (level != GOO_LOG_ERROR && curlevel < level)
	{
		return;
	}

	gchar *objectname = NULL;

	if (prefix != NULL && strlen (prefix) > 0)
	{
		objectname = g_strdup_printf ("%s:%s", prefix, objname);
	}
	else
	{
		objectname = g_strdup (objname);
	}

	va_list args;
	va_start (args, fmt);

	gchar* tmp = g_strdup_vprintf (fmt, args);
	g_fprintf (stderr, "%s [%d] <%s>%s(%d):%s\t%s\n",
		   get_log_level_str (level), getpid (),
		   GOO_STR_NULL (objectname), file, line, function, tmp);
	fflush (stderr);

	if (G_LIKELY (objectname))
	{
		g_free (objectname);
	}

	if (G_LIKELY (tmp))
	{
		g_free (tmp);
	}

	va_end (args);

	return;
}
