//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

/*!
    \file value.c
    \brief Native CMPIValue helper module.

    This module provides means to clone and release CMPIValues.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "native.h"
#include "debug.h"


void native_release_CMPIValue ( CMPIType type, CMPIValue * val )
{
    switch (type)
    {
        case CMPI_instance:
            CMRelease ( val->inst );
            break;

        case CMPI_ref:
            CMRelease ( val->ref );
            break;

        case CMPI_args:
            CMRelease ( val->args );
            break;

        case CMPI_filter:
            CMRelease ( val->filter );
            break;

        case CMPI_enumeration:
            CMRelease ( val->Enum );
            break;

        case CMPI_string:
            CMRelease ( val->string );
            break;

        case CMPI_chars:
            tool_mm_add ( val->chars );
            break;

        case CMPI_dateTime:
            CMRelease ( val->dateTime );
            break;

        case CMPI_charsptr:
            free (val->dataPtr.ptr);
            break;

        default:
            if (type & CMPI_ARRAY)
            {
                CMRelease ( val->array );
            }
    }
}


CMPIValue native_clone_CMPIValue (
    CMPIType type,
    CONST CMPIValue * val,
    CMPIStatus * rc )
{
    CMPIValue v;

    if (type & CMPI_ENC)
    {

        switch (type)
        {
            case CMPI_instance:
                v.inst = CMClone ( val->inst, rc );
                break;

            case CMPI_ref:
                v.ref = CMClone ( val->ref, rc );
                break;

            case CMPI_args:
                v.args = CMClone ( val->args, rc );
                break;

            case CMPI_filter:
                v.filter = CMClone ( val->filter, rc );
                break;

            case CMPI_enumeration:
                v.Enum = CMClone ( val->Enum, rc );
                break;

            case CMPI_string:
                v.string = CMClone ( val->string, rc );
                break;

            case CMPI_chars:
                v.chars = strdup ( val->chars );
                CMSetStatus ( rc, CMPI_RC_OK );
                break;

            case CMPI_dateTime:
                v.dateTime = CMClone ( val->dateTime, rc );
                break;

            case CMPI_charsptr:
                v.dataPtr.length = val->dataPtr.length;
                v.dataPtr.ptr = malloc (val->dataPtr.length);
                strcpy(v.dataPtr.ptr, val->dataPtr.ptr);
                break;
        }

    }
    else if (type & CMPI_ARRAY)
    {

        v.array = CMClone ( val->array, rc );
    }
    else
    {

        v = *val;
        CMSetStatus ( rc, CMPI_RC_OK );
    }

    return v;
}

extern CMPIString *__oft_toString( CMPIObjectPath * cop, CMPIStatus *rc);

char * value2Chars ( CMPIType type, CMPIValue * value )
{
    char str[256],*p;
    unsigned int size;
    CMPIString *cStr;
    TRACE_NORMAL(("value2string type 0x%x CMPIValue.", type));

    if (type & CMPI_ARRAY)
    {

//      TRACE_INFO(("serializing array object type."));
//      return __serialize_CMPIArray ( fd, value->array );

    }
    else if (type & CMPI_ENC)
    {

        TRACE_INFO(("trying to stringify encapsulated data type."));

        switch (type)
        {
            case CMPI_instance:
                break;

            case CMPI_ref:
                cStr=__oft_toString(value->ref,NULL);
                return strdup((char*)cStr->hdl);
                break;

            case CMPI_args:
                break;

            case CMPI_filter:
                break;

            case CMPI_string:
            case CMPI_numericString:
            case CMPI_booleanString:
            case CMPI_dateTimeString:
            case CMPI_classNameString:
                if (value->string->hdl)
                {
                    size = strlen((char *) value->string->hdl);
                    p = malloc(size + 8);
                    sprintf(p, "\"%s\"", (char *) value->string->hdl);
                    return p;
                }
                break;

            case CMPI_dateTime:
                break;

        }
        TRACE_CRITICAL(("non-supported encapsulated data type."));


    }
    else if (type & CMPI_SIMPLE)
    {

        TRACE_INFO(("serializing simple value."));

        switch (type)
        {
            case CMPI_boolean:
                return strdup(value->boolean ? "true" : "false" );

            case CMPI_char16:
                break;
        }

        TRACE_CRITICAL(("non-supported simple data type."));

    }
    else if (type & CMPI_INTEGER)
    {

        TRACE_INFO(("serializing integer value."));

        switch (type)
        {
            case CMPI_uint8:
                sprintf(str,"%u",value->uint8);
                return strdup(str);
            case CMPI_sint8:
                sprintf(str,"%d",value->sint8);
                return strdup(str);
            case CMPI_uint16:
                sprintf(str,"%u",value->uint16);
                return strdup(str);
            case CMPI_sint16:
                sprintf(str,"%d",value->sint16);
                return strdup(str);
            case CMPI_uint32:
                sprintf(str,"%u",value->uint32);
                return strdup(str);
            case CMPI_sint32:
                sprintf(str,"%d",value->sint32);
                return strdup(str);
            case CMPI_uint64:
#if defined CMPI_PLATFORM_WIN32_IX86_MSVC
                sprintf(str,"I64u",value->uint64);
#else
                sprintf(str,"%llu",value->uint64);
#endif
                return strdup(str);
            case CMPI_sint64:

#if defined CMPI_PLATFORM_WIN32_IX86_MSVC
                sprintf(str,"%I64d",value->sint64);
#else
                sprintf(str,"%lld",value->sint64);
#endif
                return strdup(str);
        }

        TRACE_CRITICAL(("non-supported integer data type."));

    }
    else if (type & CMPI_REAL)
    {

        switch (type)
        {
            case CMPI_real32:
                sprintf(str,"%g",value->real32);
                return strdup(str);
            case CMPI_real64:
                sprintf(str,"%g",value->real64);
                return strdup(str);
        }

        TRACE_CRITICAL(("non-supported floating point data type."));
    }
    sprintf(str,"*NOT RECOGNIZED VALUE TYPE %d*",type);
    return strdup(str);
}


/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
