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

#ifndef _GOO_UTILS_H_
#define _GOO_UTILS_H_

#include <glib.h>
#include <OMX_Core.h>

#include <goo-log.h>

G_BEGIN_DECLS

/**
 * GOO_BASE_PORT_BUFFER_PADDING:
 *
 * The padding of the buffers data when the port allocates its own buffers
 **/
#define GOO_BASE_PORT_BUFFER_PADDING 256

/** @todo The OMX Version */
#define OMX_VERSION_MAJOR    1
#define OMX_VERSION_MINOR    1
#define OMX_VERSION_REVISION 0
#define OMX_VERSION_STEP     0

/**
 * GOO_RUN:
 * @cmd: the OMX sentence to execute
 *
 * Execute and OMX's API instruction and assert it
 */
#define GOO_RUN(cmd) \
	{ \
		OMX_ERRORTYPE err = cmd; \
		if (err != OMX_ErrorNone) { \
			GOO_ERROR (goo_strerror (err)); \
			g_assert (err == OMX_ErrorNone); \
		} \
	}

/**
 * RETURN_GOO_RUN:
 * @cmd: the OMX sentence to execute
 *
 * Execute and OMX's API instruction
 *
 * Returns a boolean value if the there's no error
 */
#define RETURN_GOO_RUN(cmd) \
	{ \
		OMX_ERRORTYPE err = cmd; \
		if (err != OMX_ErrorNone) { \
			GOO_ERROR (goo_strerror (err)); \
			GOO_OBJECT_UNLOCK (self); \
			return (err == OMX_ErrorNone); \
		} \
	}

/* #define GOO_RUN(cmd) { OMXg_assert (cmd == OMX_ErrorNone) } */

/**
 * GOO_INIT_PARAM:
 * @s: The pointer to an OMX's structure
 * @t: The type of the OMX's structure
 *
 * Initializes the OMX structure setting it nSize and nVersion fields
 */
#define GOO_INIT_PARAM(st, t)					\
	g_assert (st != NULL);					\
	(st)->nSize = sizeof (t);				\
	(st)->nVersion.s.nVersionMajor = OMX_VERSION_MAJOR;	\
	(st)->nVersion.s.nVersionMinor = OMX_VERSION_MINOR;	\
	(st)->nVersion.s.nRevision = OMX_VERSION_REVISION;	\
	(st)->nVersion.s.nStep = OMX_VERSION_STEP

/**
 * GOO_ARRAY_SIZE:
 * @X: the array to measure
 *
 * Returns: the number of elements in a static array
 */
#define GOO_ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

/**
 * GOO_ROUND_UP_16:
 * @num: The number to round up
 *
 * Make number divideable by 16 without a rest.
 */
#define GOO_ROUND_UP_16(num) (((num)+15)&~15)


/**
 * GOO_STR_NULL:
 * @str: The string to check.
 *
 * Macro to use when a string must not be NULL, but may be NULL. If the string
 * is NULL, "(NULL)" is printed instead.
 */
#define GOO_STR_NULL(str) ((str) ? (str) : "(NULL)")

typedef struct _StateDesc StateDesc;

/**
 * StateDesc:
 * @state: The #OMX_STATETYPE index
 * @desc: The description of the index
 *
 * Explanation of each state index type
 **/
struct _StateDesc
{
	const OMX_STATETYPE state;
	const gchar *desc;
};

const static StateDesc statedesc[] =
{
	{ OMX_StateInvalid,	     "Invalid"		},
	{ OMX_StateLoaded,	     "Loaded"		},
	{ OMX_StateIdle,	     "Idle"		},
	{ OMX_StateExecuting,	     "Executing"	},
	{ OMX_StatePause,	     "Pause"		},
	{ OMX_StateWaitForResources, "WaitForResources" }
};

typedef struct _ErrorDesc ErrorDesc;

/**
 * ErrorDesc:
 * @error: The #OMX_ERRORTYPE index
 * @desc: The description of the index
 *
 * Explanation of each error index type
 **/
struct _ErrorDesc
{
	const OMX_ERRORTYPE error;
	const gchar *desc;
};

const static ErrorDesc errordesc[] =
{
	{ OMX_ErrorNone, "None error" },
	{ OMX_ErrorInsufficientResources, "Insufficent resources" },
	{ OMX_ErrorUndefined, "Undefined error" },
	{ OMX_ErrorInvalidComponentName, "Invalid component name" },
	{ OMX_ErrorComponentNotFound, "Component not found" },
	{ OMX_ErrorInvalidComponent, "Invalid component" },
	{ OMX_ErrorBadParameter, "Bad parameter" },
	{ OMX_ErrorNotImplemented, "Not implemented" },
	{ OMX_ErrorUnderflow, "Underflow error" },
	{ OMX_ErrorOverflow, "Overflow error" },
	{ OMX_ErrorHardware, "Hardware error" },
	{ OMX_ErrorInvalidState, "Invalid state" },
	{ OMX_ErrorStreamCorrupt, "Corrupt stream" },
	{ OMX_ErrorPortsNotCompatible, "Ports not compatible" },
	{ OMX_ErrorResourcesLost, "Resources lost" },
	{ OMX_ErrorNoMore, "\"No more\" error" },
	{ OMX_ErrorVersionMismatch, "Version mismatch" },
	{ OMX_ErrorNotReady, "Not ready" },
	{ OMX_ErrorTimeout, "Timeout" },
	{ OMX_ErrorSameState, "Same state error" },
	{ OMX_ErrorResourcesPreempted, "Resources preempted" },
	{ OMX_ErrorPortUnresponsiveDuringAllocation, "Port unresponsive during allocation" },
	{ OMX_ErrorPortUnresponsiveDuringDeallocation, "Port unresponsive during deallocation" },
	{ OMX_ErrorPortUnresponsiveDuringStop, "Port unresponsive during stop" },
	{ OMX_ErrorIncorrectStateTransition, "Incorrect state transition" },
	{ OMX_ErrorIncorrectStateOperation, "Incorrect state operation" },
	{ OMX_ErrorUnsupportedSetting, "Unsupported setting" },
	{ OMX_ErrorUnsupportedIndex, "Unsupported index" },
	{ OMX_ErrorBadPortIndex, "Bad port index" },
	{ OMX_ErrorPortUnpopulated, "Port unpopulated" }
};

typedef struct _EventDesc EventDesc;

/**
 * EventDesc:
 * @event: The #OMX_EVENTTYPE index
 * @desc: The description of the index
 *
 * Explanation of each event index type
 **/
struct _EventDesc
{
	const OMX_EVENTTYPE event;
	const gchar *desc;
};

const static EventDesc eventdesc[] =
{
	{ OMX_EventCmdComplete,		"Execution command completed" },
	{ OMX_EventError,		"Error condition detected"    },
	{ OMX_EventMark,		"Marked buffer received"      },
	{ OMX_EventPortSettingsChanged, "Port settings changed"	      },
	{ OMX_EventBufferFlag,		"End of stream detected"      },
	{ OMX_EventResourcesAcquired,	"Resources have been granted" }
};

typedef struct _CommandDesc CommandDesc;

/**
 * CommandDesc:
 * @command: The #OMX_COMMANDTYPE index
 * @desc: The description of the index
 *
 * Explanation of each command index type
 **/
struct _CommandDesc
{
	const OMX_COMMANDTYPE command;
	const gchar *desc;
};

const static CommandDesc commanddesc[] =
{
	{ OMX_CommandStateSet,	  "Change component state"  },
	{ OMX_CommandFlush,	  "Flush the buffers queue" },
	{ OMX_CommandPortDisable, "Disable a port"	    },
	{ OMX_CommandPortEnable,  "Enable a port"	    },
	{ OMX_CommandMarkBuffer,  "Mark a buffer"	    },
};

typedef struct _DirectionDesc DirectionDesc;

/**
 * DirectionDesc:
 * @direction: The #OMX_DIRTYPE index
 * @desc: The description of the index
 *
 * Explanation of each direction index type
 **/
struct _DirectionDesc
{
	const OMX_DIRTYPE direction;
	const gchar *desc;
};

const static DirectionDesc directiondesc[] =
{
	{ OMX_DirInput,	 "Input"  },
	{ OMX_DirOutput, "Output" },
};

typedef struct _DomainDesc DomainDesc;

/**
 * DomainDesc:
 * @domain: The #OMX_PORTDOMAINTYPE index
 * @desc: The description of the index
 *
 * Explanation of each port domain index type
 **/
struct _DomainDesc
{
	const OMX_PORTDOMAINTYPE domain;
	const gchar *desc;
};
const static DomainDesc domaindesc[] =
{
	{ OMX_PortDomainAudio, "Audio" },
	{ OMX_PortDomainVideo, "Video" },
	{ OMX_PortDomainImage, "Image" },
	{ OMX_PortDomainOther, "Other" }
};

typedef struct _VideoCompressionDesc VideoCompressionDesc;

struct _VideoCompressionDesc
{
	const OMX_VIDEO_CODINGTYPE compression;
	const gchar *desc;
};
const static VideoCompressionDesc videocompressiondesc[] =
{
	{ OMX_VIDEO_CodingUnused, "Unused" },
	{ OMX_VIDEO_CodingAutoDetect, "Autodetect" },
	{ OMX_VIDEO_CodingMPEG2, "MPEG2" },
	{ OMX_VIDEO_CodingH263, "H263" },
	{ OMX_VIDEO_CodingMPEG4, "MPEG4" },
	{ OMX_VIDEO_CodingWMV, "WMV" },
	{ OMX_VIDEO_CodingRV, "RV" },
	{ OMX_VIDEO_CodingAVC, "H264/AVC" },
	{ OMX_VIDEO_CodingMJPEG, "JPEG" }
};


typedef struct _ColorDesc ColorDesc;

struct _ColorDesc
{
	const OMX_COLOR_FORMATTYPE color;
	const gchar *desc;
};
const static ColorDesc colordesc[] =
{
	{ OMX_COLOR_FormatUnused, "Unused" },
	{ OMX_COLOR_FormatMonochrome, "Monochrome" },
	{ OMX_COLOR_Format8bitRGB332, "8bitRGB332" },
	{ OMX_COLOR_Format12bitRGB444, "12bitRGB444" },
	{ OMX_COLOR_Format16bitARGB4444, "16bitARGB4444" },
	{ OMX_COLOR_Format16bitARGB1555, "16bitARGB1555" },
	{ OMX_COLOR_Format16bitRGB565, "16bitRGB565" },
	{ OMX_COLOR_Format16bitBGR565, "16bitBGR565" },
	{ OMX_COLOR_Format18bitRGB666,
	  "18bitRGB666"
	},
	{ OMX_COLOR_Format18bitARGB1665,
	  "18bitARGB1665"
	},
	{ OMX_COLOR_Format19bitARGB1666 ,
	  "19bitARGB1666 "
	},
	{ OMX_COLOR_Format24bitRGB888,
	  "24bitRGB888"
	},
	{ OMX_COLOR_Format24bitBGR888,
	  "24bitBGR888"
	},
	{ OMX_COLOR_Format24bitARGB1887,
	  "24bitARGB1887"
	},
	{ OMX_COLOR_Format25bitARGB1888,
	  "25bitARGB1888"
	},
	{ OMX_COLOR_Format32bitBGRA8888,
	  "32bitBGRA8888"
	},
	{ OMX_COLOR_Format32bitARGB8888,
	  "32bitARGB8888"
	},
	{ OMX_COLOR_FormatYUV411Planar,
	  "YUV411Planar"
	},
	{ OMX_COLOR_FormatYUV411PackedPlanar,
	  "YUV411PackedPlanar"
	},
	{ OMX_COLOR_FormatYUV420Planar,
	  "YUV420Planar (I420)"
	},
	{ OMX_COLOR_FormatYUV420PackedPlanar,
	  "YUV420PackedPlanar"
	},
	{ OMX_COLOR_FormatYUV420SemiPlanar,
	  "YUV420SemiPlanar"
	},
	{ OMX_COLOR_FormatYUV422Planar,
	  "YUV422Planar"
	},
	{ OMX_COLOR_FormatYUV422PackedPlanar,
	  "YUV422PackedPlanar"
	},
	{ OMX_COLOR_FormatYUV422SemiPlanar,
	  "YUV422SemiPlanar"
	},
	{ OMX_COLOR_FormatYCbYCr,
	  "YCbYCr (YUY2)"
	},
	{ OMX_COLOR_FormatYCrYCb,
	  "YCrYCb"
	},
	{ OMX_COLOR_FormatCbYCrY,
	  "CbYCrY (UYVY)"
	},
	{ OMX_COLOR_FormatCrYCbY,
	  "CrYCbY"
	},
	{ OMX_COLOR_FormatYUV444Interleaved,
	  "YUV444Interleaved"
	},
	{ OMX_COLOR_FormatRawBayer8bit,
	  "RawBayer8bit"
	},
	{ OMX_COLOR_FormatRawBayer10bit,
	  "RawBayer10bit"
	},
	{ OMX_COLOR_FormatRawBayer8bitcompressed,
	  "RawBayer8bitcompressed"
	},
	{ OMX_COLOR_FormatL2 ,
	  "L2 "
	},
	{ OMX_COLOR_FormatL4 ,
	  "L4 "
	},
	{ OMX_COLOR_FormatL8 ,
	  "L8 "
	},
	{ OMX_COLOR_FormatL16 ,
	  "L16 "
	},
	{ OMX_COLOR_FormatL24 ,
	  "L24 "
	},
	{ OMX_COLOR_FormatL32,
	  "L32"
	},
};

typedef struct _ResolutionInfo ResolutionInfo;

/**
 * ResolutionInfo:
 * @size: the name of the resolution
 * @width: the width of the resolution
 * @height: the height of the resolution
 *
 * Structure which define the different kind of resolutions
 */
struct _ResolutionInfo
{
	char *size;
	unsigned int width;
	unsigned int height;
};
const static ResolutionInfo resinfo[] =
{
	{ "qcif",   176,  144  },
	{ "cif",    352,  288  },
	{ "vga",    640,  480  },
	{ "qvga",   320,  240  },
	{ "dvntsc", 720,  480  }, /* digital tv */
	{ "d1ntsc", 720,  486  }, /* digital tv */
	{ "pal",    720,  576  }, /* digital tv */
	{ "svga",   800,  600  },
	{ "wvga",   864,  864  },  /* WVGA Landscape/Portrait */
	{ "xvga",   1024, 768  },
	{ "720p",   1280, 720  },
	{ "sxvga",  1280, 1024 },
	{ "uxvga",  1600, 1200 },
	{ "qxga",   2048, 1536 },
	{ "wqxga",  2560, 1600 },
	{ "qsxga",  2560, 2048 }
};


const gchar* goo_strstate (OMX_STATETYPE state);
const gchar* goo_strerror (OMX_ERRORTYPE error);
const gchar* goo_strevent (OMX_EVENTTYPE event);
const gchar* goo_strcommand (OMX_COMMANDTYPE command);
const gchar* goo_strdirection (OMX_DIRTYPE direction);
const gchar* goo_strdomain (OMX_PORTDOMAINTYPE domain);
const gchar* goo_strcolor (OMX_COLOR_FORMATTYPE color);
const gchar* goo_strvideocompression (OMX_VIDEO_CODINGTYPE compression);

ResolutionInfo goo_get_resolution (const char *size);

gchar* goo_strportdef (OMX_PARAM_PORTDEFINITIONTYPE* param);

G_END_DECLS

#endif /* _GOO_UTILS_H_ */

