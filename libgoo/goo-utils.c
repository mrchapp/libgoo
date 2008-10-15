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

#include <goo-utils.h>

/**
 * goo_strstate:
 * @state: The #OMX_STATYETYPE
 *
 * Get a description of the specified state
 *
 * Returns: a static string with the description.  This string should not be
 *  modified nor freed.
 */
const gchar*
goo_strstate (OMX_STATETYPE state)
{
        gint i;
        for (i = 0; i < GOO_ARRAY_SIZE(statedesc); i++)
        {
                if (state == statedesc[i].state)
                {
                        return statedesc[i].desc;
                }
        }
        return "Unkown state!";
}

/**
 * goo_strerror:
 * @error: The #OMX_ERRORTYPE
 *
 * Get a description of the specified error
 *
 * Returns: a static string with the description.  This string should not be
 *  modified nor freed.
 */
const gchar*
goo_strerror (OMX_ERRORTYPE error)
{
        gint i;
        for (i = 0; i < GOO_ARRAY_SIZE(errordesc); i++)
        {
                if (error == errordesc[i].error)
                {
                        return errordesc[i].desc;
                }
        }
        return "Unkown error!";
}

/**
 * goo_strevent:
 * @event: The #OMX_EVENTTYPE
 *
 * Get a description of the specified event
 *
 * Returns: a static string with the description.  This string should not be
 *  modified nor freed.
 */
const gchar*
goo_strevent (OMX_EVENTTYPE event)
{
        gint i;
        for (i = 0; i < GOO_ARRAY_SIZE(eventdesc); i++)
        {
                if (event == eventdesc[i].event)
                {
                        return eventdesc[i].desc;
                }
        }
        return "Unkown event!";
}

/**
 * goo_strcommand:
 * @command: The #OMX_COMMANDTYPE
 *
 * Get a description of the specified command
 *
 * Returns: a static string with the description.  This string should not be
 *  modified nor freed.
 */
const gchar*
goo_strcommand (OMX_COMMANDTYPE command)
{
        gint i;
        for (i = 0; i < GOO_ARRAY_SIZE(commanddesc); i++)
        {
                if (command == commanddesc[i].command)
                {
                        return commanddesc[i].desc;
                }
        }
        return "Unkown command!";
}

/**
 * goo_strdirection:
 * @dir: The #OMX_DIRTYPE
 *
 * Get a description of the specified direction
 *
 * Returns: a static string with the description.  This string should not be
 *  modified nor freed.
 */
const gchar*
goo_strdirection (OMX_DIRTYPE dir)
{
        gint i;
        for (i = 0; i < GOO_ARRAY_SIZE(directiondesc); i++)
        {
                if (dir == directiondesc[i].direction)
                {
                        return directiondesc[i].desc;
                }
        }
        return "Unkown direction!";
}

/**
 * goo_strdomain:
 * @domain: The #OMX_PORTDOMAINTYPE
 *
 * Get a description of the specified domain
 *
 * Returns: a static string with the description.  This string should not be
 *  modified nor freed.
 */
const gchar*
goo_strdomain (OMX_PORTDOMAINTYPE domain)
{
        gint i;
        for (i = 0; i < GOO_ARRAY_SIZE(domaindesc); i++)
        {
                if (domain == domaindesc[i].domain)
                {
                        return domaindesc[i].desc;
                }
        }
        return "Unkown domain!";
}

/**
 * goo_strcolor:
 * @color: The #OMX_COLOR_FORMATTYPE
 *
 * Get a description of the specified color
 *
 * Returns: a static string with the description.  This string should not be
 *  modified nor freed.
 */
const gchar*
goo_strcolor (OMX_COLOR_FORMATTYPE color)
{
        gint i;
        for (i = 0; i < GOO_ARRAY_SIZE(colordesc); i++)
        {
                if (color == colordesc[i].color)
                {
                        return colordesc[i].desc;
                }
        }
        return "Unkown color!";
}

/**
 * goo_strvideocompression:
 * @color: The #OMX_VIDEO_CODINGTYPE
 *
 * Get a description of the specified compression format
 *
 * Returns: a static string with the description.  This string should not be
 *  modified nor freed.
 */
const gchar*
goo_strvideocompression (OMX_VIDEO_CODINGTYPE compression)
{
        gint i;
        for (i = 0; i < GOO_ARRAY_SIZE(videocompressiondesc); i++)
        {
                if (compression == videocompressiondesc[i].compression)
                {
                        return videocompressiondesc[i].desc;
                }
        }
        return "Unkown compression!";
}


/**
 * goo_get_resolution:
 * @size: The name of the resolution
 *
 * Search in the resolution types for name of the specified resolution
 * and brings back the information of that resolution
 *
 * Return value: A #ResolutionInfo structure
 */
ResolutionInfo
goo_get_resolution (const char *size)
{
        int i;
        for (i = 0; i < GOO_ARRAY_SIZE(resinfo); i++)
        {
                if (g_ascii_strncasecmp (size, resinfo[i].size, 6) == 0)
                {
                        return resinfo[i];
                }
        }

        ResolutionInfo err = { "Unkown", 0, 0 };
        return err;
}

/**
 * goo_strportdef:
 * @param: A pointer to an #OMX_PARAM_PORTDEFINITIONTYPE structure
 *
 * Generats a printable string with the data int the
 * #OMX_PARAM_PORTDEFINITIONTYPE structure. The string must be g_free after
 * use.
 *
 * Return value: A printable string with a dump of the
 *               #OMX_PARAM_PORTDEFINITIONTYPE structure.
 *
 * The string returned must be g_free.
 */
gchar*
goo_strportdef (OMX_PARAM_PORTDEFINITIONTYPE* param)
{
        g_assert (param != NULL);

        GString *s = g_string_new ("\n");

        g_string_append_printf (s, "nSize = %ld\n", param->nSize);
        g_string_append_printf (s, "nPortIndex = %ld\n", param->nPortIndex);
        g_string_append_printf (s, "eDir = %s\n",
                                goo_strdirection (param->eDir));
        g_string_append_printf (s, "nBufferCountActual = %ld\n",
                                param->nBufferCountActual);
        g_string_append_printf (s, "nBufferCountMin = %ld\n",
                                param->nBufferCountMin);
        g_string_append_printf (s, "nBufferSize = %ld\n", param->nBufferSize);
        g_string_append_printf (s, "bEnabled = %d\n", param->bEnabled);
        g_string_append_printf (s, "bPopulated = %d\n", param->bPopulated);
        g_string_append_printf (s, "eDomain = %s\n",
                                goo_strdomain (param->eDomain));

		if (param->eDomain == OMX_PortDomainAudio)
		{
                g_string_append_printf (s, "\tcMIMEType = %s\n",
                                        param->format.audio.cMIMEType);
                g_string_append_printf (s, "\teEncoding = %d\n",
                                        param->format.audio.eEncoding);
                g_string_append_printf (s, "\tpNativeRender = %p\n",
                                        param->format.audio.pNativeRender);
                g_string_append_printf (s, "\tbFlagErrorConcealment = %d\n",
                                        param->format.audio.bFlagErrorConcealment);
		}

        if (param->eDomain == OMX_PortDomainImage)
        {
                g_string_append_printf (s, "\tcMIMEType = %s\n",
                                        param->format.image.cMIMEType);
                g_string_append_printf (s, "\tnFrameWidth = %ld\n",
                                        param->format.image.nFrameWidth);
                g_string_append_printf (s, "\tnFrameHeight = %ld\n",
                                        param->format.image.nFrameHeight);
                g_string_append_printf (s, "\tnStride = %ld\n",
                                        param->format.image.nStride);
                g_string_append_printf (s, "\tnSliceHeight = %ld\n",
                                        param->format.image.nSliceHeight);
                g_string_append_printf (s, "\teColorFormat = %s\n",
                                        goo_strcolor (param->format.image.eColorFormat));
                g_string_append_printf (s, "\tbFlagErrorConcealment = %d\n",
                                        param->format.image.bFlagErrorConcealment);
                g_string_append_printf (s, "\teCompressionFormat = %d",
                                        param->format.image.eCompressionFormat);
        }

        if (param->eDomain == OMX_PortDomainVideo)
        {
                g_string_append_printf (s, "\tcMIMEType = %s\n",
                                        param->format.video.cMIMEType);
                g_string_append_printf (s, "\tnFrameWidth = %ld\n",
                                        param->format.video.nFrameWidth);
                g_string_append_printf (s, "\tnFrameHeight = %ld\n",
                                        param->format.video.nFrameHeight);
                g_string_append_printf (s, "\tnStride = %ld\n",
                                        param->format.video.nStride);
                g_string_append_printf (s, "\tnSliceHeight = %ld\n",
                                        param->format.video.nSliceHeight);
                g_string_append_printf (s, "\teColorFormat = %s\n",
                                        goo_strcolor (param->format.video.eColorFormat));
                g_string_append_printf (s, "\tBitrate = %ld\n",
                                        param->format.video.nBitrate);
                g_string_append_printf (s, "\teCompression format = %s\n",
                                        goo_strvideocompression (param->format.video.eCompressionFormat));
        }

        return g_string_free (s, FALSE);
}
