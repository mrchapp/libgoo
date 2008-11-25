/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* =====================================================================
 *                Texas Instruments OMAP(TM) Platform Software
 *             Copyright (c) 2005 Texas Instruments, Incorporated
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ===================================================================== */

#ifndef _GOO_TI_SPARKDEC_H_
#define _GOO_TI_SPARKDEC_H_

#include <goo-ti-video-decoder.h>

G_BEGIN_DECLS

#define GOO_TYPE_TI_SPARKDEC \
        (goo_ti_sparkdec_get_type ())
#define GOO_TI_SPARKDEC(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_TI_SPARKDEC, GooTiSparkDec))
#define GOO_TI_SPARKDEC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_TI_SPARKDEC, GooTiSparkDecClass))
#define GOO_IS_TI_SPARKDEC(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_TI_SPARKDEC))
#define GOO_IS_TI_SPARKDEC_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_TI_SPARKDEC))
#define GOO_TI_SPARKDEC_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TI_SPARKDEC_GET_CLASS, GooTiSparkDecClass))

#define GOO_TI_SPARKDEC_IS_SPARK_INPUT	\
	(goo_ti_sparkdec_is_sparkinput_get_type())

typedef enum _GooTiSparkDecIsSparInput
{
	GOO_TI_SPARKDEC_NOT_SPARKINPUT,
	GOO_TI_SPARKDEC_IS_SPARKINPUT
} GooTiSparkDecIsSparInput;


typedef struct _GooTiSparkDec GooTiSparkDec;
typedef struct _GooTiSparkDecClass GooTiSparkDecClass;

struct _GooTiSparkDec
{
        GooTiVideoDecoder parent;
        gboolean bIsSparkInput;
};

struct _GooTiSparkDecClass
{
        GooTiVideoDecoderClass parent_class;
};

/* functions */
GType goo_ti_sparkdec_get_type (void);
GType goo_ti_sparkdec_is_sparkinput_get_type (void);

GooComponent* goo_ti_sparkdec_new ();

G_END_DECLS


#endif /* _GOO_TI_SPARKDEC_H_ */
