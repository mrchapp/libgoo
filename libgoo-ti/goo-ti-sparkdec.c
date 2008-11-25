/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* =====================================================================
 *                Texas Instruments OMAP(TM) Platform Software
 *             Copyright (c) 2005 Texas Instruments, Incorporated
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 * ===================================================================== */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <goo-ti-sparkdec.h>
#include <goo-utils.h>

enum _GooTiVideoDecoderSparkProp
{
	PROP_0,
	PROP_IS_SPARKINPUT,
};

#define DEFAULT_SPARKINPUT GOO_TI_SPARKDEC_IS_SPARKINPUT

GType
goo_ti_sparkdec_is_sparkinput_get_type (void)
{
	static GType goo_ti_sparkdec_is_sparkinput_get_type = 0;	
	
	if(!goo_ti_sparkdec_is_sparkinput_get_type)	
	{
		static const GEnumValue values[] = {
			{GOO_TI_SPARKDEC_NOT_SPARKINPUT, "0", "Not Spark input" },
			{GOO_TI_SPARKDEC_IS_SPARKINPUT, "1", "Not Spark input" },
			{ 0, NULL, NULL },
		};

		goo_ti_sparkdec_is_sparkinput_get_type =
			g_enum_register_static ("GooTiSparkDecIsSparInput",
						values);
	}

	return goo_ti_sparkdec_is_sparkinput_get_type;
}

G_DEFINE_TYPE (GooTiSparkDec, goo_ti_sparkdec, GOO_TYPE_TI_VIDEO_DECODER)

static void
goo_ti_sparkdec_validate_ports_definitions (GooComponent* component)
{
        g_assert (GOO_IS_TI_SPARKDEC (component));
        GooTiSparkDec* self = GOO_TI_SPARKDEC (component);
        g_assert (component->cur_state == OMX_StateLoaded);

        OMX_INDEXTYPE index;

	GOO_RUN (
		OMX_GetExtensionIndex (component->handle,
					"OMX.TI.VideoDecode.Param.IsSparkInput",
				       &index)
		);

	GOO_RUN (
		OMX_SetParameter (component->handle,
				  index, &self->bIsSparkInput)
		);

	GOO_OBJECT_DEBUG (self, "is_spark_input = %d", self->bIsSparkInput);

        OMX_PARAM_PORTDEFINITIONTYPE *param;        
        GOO_OBJECT_DEBUG (self, "Entering");
        
        {	/* input */
            GooIterator* iter =
            	goo_component_iterate_input_ports (component);
            goo_iterator_nth (iter, 0);
            GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
            g_assert (port != NULL);
                
            param = GOO_PORT_GET_DEFINITION (port);
                
            param->format.video.cMIMEType = "SPARK";                
            param->format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
			param->format.video.bFlagErrorConcealment = OMX_FALSE;
			
			switch (param->format.video.eColorFormat)
            {
            	case OMX_COLOR_FormatCbYCrY:                
                    param->nBufferSize =
                            param->format.video.nFrameWidth *
                            param->format.video.nFrameHeight * 2;
                    break;
            	case OMX_COLOR_FormatYUV420Planar:
                    param->nBufferSize =
                            (param->format.video.nFrameWidth *
                            param->format.video.nFrameHeight) * 1.5;
                    break;
            	default:
                    GOO_OBJECT_ERROR (self, "Not valid color format");
                    g_assert (FALSE);
					break;
            }
            
            param->format.video.eColorFormat = OMX_VIDEO_CodingUnused;
                					 
            GOO_OBJECT_DEBUG (self, " Input port nFrameWidth : %d nFrameHeight : %d",
                					    param->format.video.nFrameWidth,
                					    param->format.video.nFrameHeight);
                                               
            g_object_unref (iter);
            g_object_unref (port);
        }

        /* output */
        {
            GooIterator* iter =
                    goo_component_iterate_output_ports (component);
            goo_iterator_nth (iter, 0);
            GooPort* port = GOO_PORT (goo_iterator_get_current (iter));
            g_assert (port != NULL);

            param = GOO_PORT_GET_DEFINITION (port);

			param->format.video.cMIMEType = "video/x-raw-yuv";
            param->format.video.pNativeRender = NULL;
            param->format.video.bFlagErrorConcealment = OMX_FALSE;
            param->format.video.eCompressionFormat =OMX_VIDEO_CodingUnused;
            
            switch (param->format.video.eColorFormat)
            {
	            case OMX_COLOR_FormatCbYCrY:                
                    param->nBufferSize =
                            param->format.video.nFrameWidth *
                            param->format.video.nFrameHeight * 2;
                    break;
    	        case OMX_COLOR_FormatYUV420Planar:
                    param->nBufferSize =
                            (param->format.video.nFrameWidth *
                            param->format.video.nFrameHeight) * 1.5;
                    break;
        	    default:
                    GOO_OBJECT_ERROR (self, "Not valid color format");
                    g_assert (FALSE);
					break;
            }
            
            GOO_OBJECT_DEBUG (self, "Output port nFrameWidth : %d nFrameHeight : %d",
            						param->format.video.nFrameWidth,
            						param->format.video.nFrameHeight);
              
                g_object_unref (iter);
                g_object_unref (port);
        }

        GOO_OBJECT_DEBUG (self, "Exit");
        
        return;
}

static void
goo_ti_sparkdec_get_property (GObject* object, guint prop_id,
				     GValue* value, GParamSpec* spec)
{
	g_assert (GOO_IS_TI_SPARKDEC (object));
	GooTiSparkDec* self = GOO_TI_SPARKDEC (object);

	switch (prop_id)
	{
	case PROP_IS_SPARKINPUT:
		g_value_set_enum (value, self->bIsSparkInput);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
		break;
	}

	return;
}


static void
goo_ti_sparkdec_init (GooTiSparkDec* self)
{
	self->bIsSparkInput = GOO_TI_SPARKDEC_IS_SPARKINPUT;
        return;
}

static void
goo_ti_sparkdec_class_init (GooTiSparkDecClass* klass)
{
        GooComponentClass* o_klass = GOO_COMPONENT_CLASS (klass);
        GObjectClass* g_klass = G_OBJECT_CLASS (klass);

        g_klass->get_property = goo_ti_sparkdec_get_property;

	GParamSpec* spec;
	spec = g_param_spec_enum ("IsSparkInput", "Is Spark input",
				  "Is Spark input",
				  GOO_TI_SPARKDEC_IS_SPARK_INPUT,
				  DEFAULT_SPARKINPUT, G_PARAM_READABLE);
				  
	g_object_class_install_property (g_klass, PROP_IS_SPARKINPUT, spec);
        
	o_klass->validate_ports_definition_func = goo_ti_sparkdec_validate_ports_definitions;

        return;
}


