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
    \file serialization.c
    \brief Binary serializer component for Remote CMPI.

    This file implements the functions defined in the function table contained
    in serialization.h. There are function pairs for each CMPI data type that
    can be fully serialized and deserialized using the CMPI APIs. The
    functions except file descriptors to be given as arguments, i.e. socket
    file descriptors to read from or write to.

    The serialization functions return the length of a successfully serialized
    object or data type, whereas the deserialization parts return the
    recreated data.

    \sa serialization.h
    \sa io.c

    \todo Implement CMPISelectExp serialization.
*/

#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# include <windows.h>
# include <winsock2.h>
#else
# if defined (PEGASUS_OS_ZOS)
#  include <arpa/inet.h>
# else
#  include <error.h>
# endif
# include <unistd.h>
# include <strings.h>
# include <netinet/in.h>
#endif

#include "debug.h"
#include "io.h"
#include "serialization.h"
#include "tool.h"
#include "indication_objects.h"
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>

extern int nativeSide;

/****************************************************************************/
static ssize_t __serialize_UINT8 ( int, CMPIUint8 );
static CMPIUint8 __deserialize_UINT8 ( int );

static ssize_t __serialize_UINT16 ( int, CMPIUint16 );
static CMPIUint16 __deserialize_UINT16 ( int );

static ssize_t __serialize_UINT32 ( int, CMPIUint32 );
static CMPIUint32 __deserialize_UINT32 ( int );

static ssize_t __serialize_UINT64 ( int, CMPIUint64 );
static CMPIUint64 __deserialize_UINT64 ( int );

static int __serialized_NULL ( int fd, const void * ptr );
static int __deserialized_NULL ( int fd );

static ssize_t __serialize_CMPIType ( int, CMPIType );
static CMPIType __deserialize_CMPIType ( int );

static ssize_t __serialize_CMPIValue (int, CONST CMPIType, CONST CMPIValue *);
static CMPIValue __deserialize_CMPIValue (int, CMPIType, CONST CMPIBroker *);

static ssize_t __serialize_CMPIData ( int, CMPIData );
static CMPIData __deserialize_CMPIData ( int, CONST CMPIBroker * );

static ssize_t __serialize_CMPIStatus ( int, CMPIStatus * );
static CMPIStatus __deserialize_CMPIStatus ( int, CONST CMPIBroker * );

static ssize_t __serialize_CMPIArgs ( int, CONST CMPIArgs * );
static CMPIArgs * __deserialize_CMPIArgs ( int, CONST CMPIBroker * );

static ssize_t __serialize_string ( int, const char * );
static char * __deserialize_string ( int, CONST CMPIBroker * );

static ssize_t __serialize_CMPIString ( int, CONST CMPIString * );
static CMPIString * __deserialize_CMPIString ( int, CONST CMPIBroker * );

static ssize_t __serialize_CMPIObjectPath ( int, CONST CMPIObjectPath * );
static CMPIObjectPath * __deserialize_CMPIObjectPath (
    int,
    CONST CMPIBroker * );

static ssize_t __serialize_CMPIArray ( int, CONST CMPIArray * );
static CMPIArray * __deserialize_CMPIArray ( int, CONST CMPIBroker * );

static ssize_t __serialize_CMPIInstance ( int, CONST CMPIInstance * );
static CMPIInstance * __deserialize_CMPIInstance ( int, CONST CMPIBroker * ,
    CONST CMPIObjectPath *);

static ssize_t __serialize_CMPISelectExp (
    int,
    CONST CMPISelectExp *,
    CMPIUint32);
static CMPISelectExp * __deserialize_CMPISelectExp(
    int,
    CONST CMPIBroker *,
    CMPIUint32);

static ssize_t __serialize_CMPIDateTime ( int, CMPIDateTime * );
static CMPIDateTime * __deserialize_CMPIDateTime ( int, CONST CMPIBroker * );

#ifdef CMPI_VER_200
static ssize_t __serialize_CMPIError ( int, CMPIError * );
static CMPIError * __deserialize_CMPIError ( int, CONST CMPIBroker * );

static ssize_t __serialize_CMPIErrorSeverity ( int, CMPIErrorSeverity );
static CMPIErrorSeverity __deserialize_CMPIErrorSeverity (
    int,
    CONST CMPIBroker * );

static ssize_t __serialize_CMPIErrorProbableCause (
    int,
    CMPIErrorProbableCause );
static CMPIErrorProbableCause __deserialize_CMPIErrorProbableCause (
    int,
    CONST CMPIBroker * );

static ssize_t __serialize_CMPIrc ( int, CMPIrc );
static CMPIrc __deserialize_CMPIrc ( int, CONST CMPIBroker * );

static ssize_t __serialize_CMPIMsgFileHandle ( int, CMPIMsgFileHandle );
static CMPIMsgFileHandle __deserialize_CMPIMsgFileHandle (
    int,
    CONST CMPIBroker * );
#endif

/****************************************************************************/



const struct BinarySerializerFT binarySerializerFT =
{
    __serialize_UINT8,
    __deserialize_UINT8,
    __serialize_UINT16,
    __deserialize_UINT16,
    __serialize_UINT32,
    __deserialize_UINT32,
    __serialize_UINT64,
    __deserialize_UINT64,
    __serialize_CMPIValue,
    __deserialize_CMPIValue,
    __serialize_CMPIType,
    __deserialize_CMPIType,
    __serialize_CMPIData,
    __deserialize_CMPIData,
    __serialize_CMPIStatus,
    __deserialize_CMPIStatus,
    __serialize_string,
    __deserialize_string,
    __serialize_CMPIString,
    __deserialize_CMPIString,
    __serialize_CMPIArgs,
    __deserialize_CMPIArgs,
    __serialize_CMPIObjectPath,
    __deserialize_CMPIObjectPath,
    __serialize_CMPIArray,
    __deserialize_CMPIArray,
    __serialize_CMPIInstance,
    __deserialize_CMPIInstance,
    __serialize_CMPISelectExp,
    __deserialize_CMPISelectExp,
    __serialize_CMPIDateTime,
    __deserialize_CMPIDateTime,
#ifdef CMPI_VER_200
    __serialize_CMPIError,
    __deserialize_CMPIError,
    __serialize_CMPIErrorSeverity,
    __deserialize_CMPIErrorSeverity,
    __serialize_CMPIErrorProbableCause,
    __deserialize_CMPIErrorProbableCause,
    __serialize_CMPIrc,
    __deserialize_CMPIrc,
    __serialize_CMPIMsgFileHandle,
    __deserialize_CMPIMsgFileHandle,
#endif

};

PEGASUS_EXPORT const struct BinarySerializerFT *binarySerializerFTptr =
    &binarySerializerFT;
/****************************************************************************/


static ssize_t __serialize_UINT8 ( int fd, CMPIUint8 uint8 )
{
    if (io_write_fixed_length (
        fd,
        &uint8,
        sizeof ( CMPIUint8 ) ))
    {
        return -1;
    }
    return sizeof ( CMPIUint8 );
}


static CMPIUint8  __deserialize_UINT8 ( int fd )
{
    CMPIUint8 i;

    if (io_read_fixed_length ( fd, &i, sizeof ( CMPIUint8 ) ))
    {
        return 0;
    }

    return i;
}


static ssize_t __serialize_UINT16 ( int fd, CMPIUint16 uint16 )
{
    return __serialize_UINT32 ( fd, uint16 );
}


static CMPIUint16 __deserialize_UINT16 ( int fd )
{
    return __deserialize_UINT32 ( fd );
}


static ssize_t __serialize_UINT32 ( int fd, CMPIUint32 uint32 )
{
    unsigned long int i = htonl ( uint32 );

    if (io_write_fixed_length ( fd, &i, sizeof ( unsigned long int ) ))
    {
        return -1;
    }
    return sizeof ( unsigned long int );
}


static CMPIUint32 __deserialize_UINT32 ( int fd )
{
    unsigned long int i;

    if (io_read_fixed_length ( fd, &i, sizeof ( unsigned long int ) ))
    {
        return 0;
    }

    return ntohl ( i );
}


static ssize_t __serialize_UINT64 ( int fd, CMPIUint64 uint64 )
{
    ssize_t sl, sh;

    unsigned long int l = uint64 & 0xFFFFFFFF;
    unsigned long int h = uint64 >> 32;

    if (( sl = __serialize_UINT32 ( fd, l ) ) < 0)
    {
        return -1;
    }

    if (( sh = __serialize_UINT32 ( fd, h ) ) < 0)
    {
        return -1;
    }

    return sl + sh;
}


static CMPIUint64 __deserialize_UINT64 ( int fd )
{
    CMPIUint64 l = __deserialize_UINT32 ( fd );
    CMPIUint64 h = __deserialize_UINT32 ( fd );

    return( h << 32 ) | l;
}

/****************************************************************************/


static int __serialized_NULL ( int fd, const void * ptr )
{
    char is_null = ( ptr == NULL );

    TRACE_VERBOSE(("entered function ser NULL. leaving function."));

    __serialize_UINT8 ( fd, is_null );

    return is_null;
}


static int __deserialized_NULL ( int fd )
{
    char is_null;

    TRACE_VERBOSE(("entered function des NULL. leaving function."));

    is_null = __deserialize_UINT8 ( fd );

    return is_null;
}


static ssize_t __serialize_CMPIType ( int fd, CMPIType type )
{
    return __serialize_UINT32 ( fd, type );
}


static CMPIType __deserialize_CMPIType ( int fd )
{
    return __deserialize_UINT32 ( fd );
}


static ssize_t __serialize_CMPIValue ( int fd,
    CONST CMPIType type,
    CONST CMPIValue * value )
{
    TRACE_NORMAL(("serializing type 0x%x CMPIValue.", type));

    if (type & CMPI_ARRAY)
    {

        TRACE_INFO(("serializing array object type."));
        return __serialize_CMPIArray ( fd, value->array );

    }
    else if (type & CMPI_ENC)
    {

        TRACE_INFO(("trying to serialize encapsulated data type."));

        switch (type)
        {
            case CMPI_instance:
                return __serialize_CMPIInstance ( fd, value->inst);

            case CMPI_ref:
                return __serialize_CMPIObjectPath ( fd, value->ref );

            case CMPI_args:
                return __serialize_CMPIArgs ( fd, value->args );

            case CMPI_filter:
                return __serialize_CMPISelectExp (
                    fd,
                    value->filter,
                    PEGASUS_INDICATION_GLOBAL_CONTEXT);

            case CMPI_string:
            case CMPI_numericString:
            case CMPI_booleanString:
            case CMPI_dateTimeString:
            case CMPI_classNameString:
                return __serialize_CMPIString ( fd, value->string );

            case CMPI_dateTime:
                return __serialize_CMPIDateTime ( fd, value->dateTime);
        }

        TRACE_CRITICAL(("non-supported encapsulated data type."));


    }
    else if (type & CMPI_SIMPLE)
    {

        TRACE_INFO(("serializing simple value."));

        switch (type)
        {
            case CMPI_boolean:
                return __serialize_UINT8 ( fd, value->boolean );

            case CMPI_char16:
                return __serialize_UINT16 ( fd, value->char16 );
        }

        TRACE_CRITICAL(("non-supported simple data type."));

    }
    else if (type & CMPI_INTEGER)
    {

        TRACE_INFO(("serializing integer value."));

        switch (type)
        {
            case CMPI_uint8:
            case CMPI_sint8:
                return __serialize_UINT8 ( fd, value->uint8 );

            case CMPI_uint16:
            case CMPI_sint16:
                return __serialize_UINT16 ( fd, value->uint16 );

            case CMPI_uint32:
            case CMPI_sint32:
                return __serialize_UINT32 ( fd, value->uint32 );

            case CMPI_uint64:
            case CMPI_sint64:
                return __serialize_UINT64 ( fd, value->uint64 );
        }

        TRACE_CRITICAL(("non-supported integer data type."));

    }
    else if (type & CMPI_REAL)
    {

        char real_str[256];

        switch (type)
        {
            case CMPI_real32:
            case CMPI_real64:
                sprintf ( real_str,
                    "%.100e",
                    ( type == CMPI_real32 )?
                    (double) value->real32:
                    value->real64 );
                return __serialize_string ( fd, real_str );
        }

        TRACE_CRITICAL(("non-supported floating point data type."));
    }

    error_at_line ( -1, 0, __FILE__, __LINE__,
        "Unable to serialize requested data type: 0x%x",
        type );

    return -1;
}


static CMPIValue __deserialize_CMPIValue (
    int fd,
    CMPIType type,
    CONST CMPIBroker * broker )
{
    int failed = 0;
    CMPIValue v;

    TRACE_VERBOSE(("entered function."));

    memset( &v, 0, sizeof (CMPIValue) );
    TRACE_NORMAL(("deserializing type 0x%x CMPIValue.", type));

    if (type & CMPI_ARRAY)
    {

        TRACE_INFO(("deserializing array object type."));
        v.array = __deserialize_CMPIArray ( fd, broker );

    }
    else if (type & CMPI_ENC)
    {

        TRACE_INFO(("trying to deserialize encapsulated data type."));
        switch (type)
        {
            case CMPI_instance:
                v.inst = __deserialize_CMPIInstance ( fd, broker , 0);
                break;

            case CMPI_ref:
                v.ref = __deserialize_CMPIObjectPath ( fd, broker );
                break;

            case CMPI_args:
                v.args = __deserialize_CMPIArgs ( fd, broker );
                break;

            case CMPI_filter:
                v.filter = __deserialize_CMPISelectExp (
                    fd,
                    broker,
                    PEGASUS_INDICATION_GLOBAL_CONTEXT);
                break;

            case CMPI_string:
            case CMPI_numericString:
            case CMPI_booleanString:
            case CMPI_dateTimeString:
            case CMPI_classNameString:
                v.string = __deserialize_CMPIString ( fd, broker );
                break;

            case CMPI_dateTime:
                v.dateTime = __deserialize_CMPIDateTime ( fd, broker );
                break;

            default:
                TRACE_CRITICAL(("non-supported encapsulated data."));
                failed = 1;
        }

    }
    else if (type & CMPI_SIMPLE)
    {

        TRACE_INFO(("deserializing simple value."));

        switch (type)
        {
            case CMPI_boolean:
                v.boolean = __deserialize_UINT8 ( fd );
                break;

            case CMPI_char16:
                v.char16 = __deserialize_UINT16 ( fd );
                break;

            default:
                TRACE_CRITICAL(("non-supported simple data type."));
                failed = 1;
        }

    }
    else if (type & CMPI_INTEGER)
    {

        TRACE_INFO(("deserializing integer value."));

        switch (type)
        {
            case CMPI_uint8:
            case CMPI_sint8:
                v.uint8 = __deserialize_UINT8 ( fd );
                break;

            case CMPI_uint16:
            case CMPI_sint16:
                v.uint16 = __deserialize_UINT16 ( fd );
                break;

            case CMPI_uint32:
            case CMPI_sint32:
                v.uint32 = __deserialize_UINT32 ( fd );
                break;

            case CMPI_uint64:
            case CMPI_sint64:
                v.uint64 = __deserialize_UINT64 ( fd );
                break;

            default:
                TRACE_CRITICAL(("non-supported integer data type."));
                failed = 1;
        }

    }
    else if (type & CMPI_REAL)
    {

        char * real_str;
        double r;

        switch (type)
        {
            case CMPI_real32:
            case CMPI_real64:
                real_str = __deserialize_string ( fd, broker );
                sscanf ( real_str, "%le", (double *) &r );

                if (type == CMPI_real32)
                {
                    v.real32 =  (CMPIReal32) r;
                }
                else  v.real64 = r;
                break;

            default:
                TRACE_CRITICAL(("non-supported floating point "
                    "data type."));
                failed = 1;
        }
    }

    if (failed)
    {
        error_at_line (
            -1,
            0,
            __FILE__,
            __LINE__,
            "Unable to deserialize requested "
            "data type: 0x%x", type );
    }

    TRACE_VERBOSE(("leaving function."));
    return v;
}


static ssize_t __serialize_CMPIData ( int fd, CMPIData data )
{
    ssize_t tmp, out;

    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("serializing CMPIData."));

    TRACE_INFO(("state: 0x%x\ntype: 0x%x", data.state, data.type ));

    out = __serialize_UINT16 ( fd, data.state );
    if (out < 0)
    {
        return -1;
    }

    if (( tmp = __serialize_CMPIType ( fd, data.type ) ) < 0)
    {
        return -1;
    }

    out += tmp;

    if (! ( data.state & CMPI_nullValue ))
    {

        out += __serialize_CMPIValue ( fd, data.type, &data.value );
    }

    TRACE_VERBOSE(("leaving function."));
    return out;
}


static CMPIData __deserialize_CMPIData ( int fd, CONST CMPIBroker * broker )
{
    CMPIData data = {0,0,{0}};

    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("deserializing CMPIData."));

    data.state = __deserialize_UINT16 ( fd );
    data.type  = __deserialize_CMPIType ( fd );

    TRACE_INFO(("state: 0x%x\ntype: 0x%x", data.state, data.type ));

    if (! ( data.state & CMPI_nullValue ))
    {

        data.value = __deserialize_CMPIValue (
            fd,
            data.type,
            broker );
//                if (data.type & CMPI_ENC)
//                        data.state |= CMPI_nullValue;
    }

    TRACE_VERBOSE(("leaving function."));
    return data;
}


static ssize_t __serialize_CMPIStatus ( int fd, CMPIStatus * rc )
{
    ssize_t out, tmp;

    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("serializing CMPIStatus."));

    tmp = __serialize_UINT32 ( fd, rc->rc );
    if (tmp < 0)
    {
        return -1;
    }

    TRACE_INFO(("rc: %d\nmsg: %s",
        rc->rc,
        rc->msg ? (CMGetCharsPtr ( rc->msg, NULL )) : "" ));

    out = __serialize_CMPIString ( fd, rc->msg );
    if (out)
    {
        return -1;
    }

    TRACE_VERBOSE(("leaving function."));
    return out + tmp;
}


static CMPIStatus __deserialize_CMPIStatus (int fd, CONST CMPIBroker * broker)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};

    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("deserializing CMPIStatus."));

    rc.rc  = (CMPIrc) __deserialize_UINT32 ( fd );
    rc.msg = __deserialize_CMPIString ( fd, broker );

    TRACE_INFO(("rc: %d\nmsg: %s",
        rc.rc,
        rc.msg ? (CMGetCharsPtr ( rc.msg, NULL )) : "" ));

    TRACE_VERBOSE(("leaving function."));
    return rc;
}


static ssize_t __serialize_string ( int fd, const char * str )
{
    size_t len;
    ssize_t out;

    if (__serialized_NULL ( fd, str ))
    {
        return 0;
    }

    len = strlen ( str );
    out = __serialize_UINT32 ( fd, (CMPIUint32) len );
    if (out < 0)
    {
        return -1;
    }

    if (io_write_fixed_length ( fd, str, len ))
    {
        return -1;
    }

    return len + out;
}


static char * __deserialize_string_alloc ( int fd )
{
    size_t len;
    char * str;

    if (__deserialized_NULL ( fd ))
    {
        return NULL;
    }

    len = __deserialize_UINT32 ( fd );

    str = (char *) calloc ( len + 1, sizeof ( char ) );

    if (io_read_fixed_length ( fd, str, len ))
    {
        free ( str );
        return NULL;
    }
    return str;
}


static char * __deserialize_string ( int fd, CONST CMPIBroker * broker )
{
    char * tmp = __deserialize_string_alloc ( fd );

    if (tmp)
    {
        CMPIString * str = CMNewString ( broker, tmp, NULL );
        free ( tmp );
        return (char*)CMGetCharsPtr ( str, NULL );
    }
    return NULL;
}


static ssize_t __serialize_CMPIString ( int fd, CONST CMPIString * string )
{
    if (__serialized_NULL ( fd, string ))
    {
        return 0;
    }
    return __serialize_string ( fd, CMGetCharsPtr ( string, NULL ) );
}


static CMPIString * __deserialize_CMPIString (int fd, CONST CMPIBroker * broker)
{
    CMPIString * str;
    char * tmp;

    if (__deserialized_NULL ( fd ))
    {
        return NULL;
    }

    tmp = __deserialize_string_alloc ( fd );
    str = CMNewString ( broker, tmp, NULL );

    free ( tmp );
    return str;
}


static ssize_t __serialize_CMPIArgs ( int fd, CONST CMPIArgs * args )
{
    ssize_t out;
    unsigned int i, arg_count;

    TRACE_VERBOSE(("entered function."));

    if (__serialized_NULL ( fd, args ))
    {
        return 0;
    }

    TRACE_NORMAL(("serializing non-NULL CMPIArgs."));

    arg_count = CMGetArgCount ( args, NULL );
    out = __serialize_UINT32 ( fd, arg_count );
    if (out < 0)
    {
        return -1;
    }

    TRACE_INFO(("arg count: %d", arg_count ));

    for (i = 0; i < arg_count; i++)
    {
        CMPIString * argName;
        CMPIData data = CMGetArgAt ( args, i, &argName, NULL );

        TRACE_INFO(("arg(%d):\nname: %s\ntype: 0x%x\nstate: 0x%x.",
            i, CMGetCharsPtr ( argName, NULL ),
            data.type, data.state ));

        out += __serialize_CMPIString ( fd, argName );
        out += __serialize_CMPIData ( fd, data );
    }

    TRACE_VERBOSE(("leaving function."));
    return out;
}


static CMPIArgs * __deserialize_CMPIArgs ( int fd, CONST CMPIBroker * broker )
{
    CMPIArgs * args;
    unsigned int arg_count;

    TRACE_VERBOSE(("entered function."));

    if (__deserialized_NULL ( fd ))
    {
        return NULL;
    }

    TRACE_NORMAL(("deserializing non-NULL CMPIArgs."));

    args = CMNewArgs ( broker, NULL );
    arg_count = __deserialize_UINT32 ( fd );

    TRACE_INFO(("arg count: %d", arg_count ));

    while (arg_count--)
    {
        CMPIString * argName = __deserialize_CMPIString ( fd, broker );
        CMPIData data        = __deserialize_CMPIData ( fd, broker );

        TRACE_INFO(("arg:\nname: %s\ntype: 0x%x\nstate: 0x%x.",
            CMGetCharsPtr ( argName, NULL ),
            data.type, data.state ));

        CMAddArg ( args,
            CMGetCharsPtr ( argName, NULL ),
            &data.value,
            data.type );
    }

    TRACE_VERBOSE(("leaving function."));
    return args;
}


static ssize_t __serialize_CMPIObjectPath ( int fd, CONST CMPIObjectPath * cop )
{
    ssize_t out, tmp;
    unsigned int key_count, i;
    CMPIString * classname, * namespace;

    TRACE_VERBOSE(("entered function."));

    if (__serialized_NULL ( fd, cop ))
    {
        return 0;
    }

    TRACE_NORMAL(("serializing non-NULL CMPIObjectPath."));

    classname = CMGetClassName ( cop, NULL );
    namespace = CMGetNameSpace ( cop, NULL );
    key_count = CMGetKeyCount ( cop, NULL );

    TRACE_INFO(("classname: %s.", CMGetCharsPtr ( classname, NULL )));
    TRACE_INFO(("namespace: %s.", CMGetCharsPtr ( namespace, NULL )));
    TRACE_INFO(("key count: %d", key_count ));

    out =__serialize_CMPIString ( fd, classname );
    if (out < 0)
    {
        TRACE_CRITICAL(("failed to serialize classname."));
        return -1;
    }

    tmp = __serialize_CMPIString ( fd, namespace );
    if (tmp < 0)
    {
        TRACE_CRITICAL(("failed to serialize namespace."));
        return -1;
    }

    out += tmp;

    tmp = __serialize_UINT32 ( fd, key_count );
    if (tmp < 0)
    {
        TRACE_CRITICAL(("failed to serialize keycount."));
        return -1;
    }
    out += tmp;

    for (i = 0; i < key_count; i++)
    {
        CMPIString * keyName;
        CMPIData data = CMGetKeyAt ( cop, i, &keyName, NULL );

        TRACE_INFO(("key(%d):\nname: %s\ntype: 0x%x\nstate: 0x%x.",
            i, CMGetCharsPtr ( keyName, NULL ),
            data.type, data.state ));

        out += __serialize_CMPIString ( fd, keyName );
        out += __serialize_CMPIData ( fd, data );
    }

    TRACE_VERBOSE(("leaving function."));
    return out;
}


static CMPIObjectPath * __deserialize_CMPIObjectPath ( int fd,
    CONST CMPIBroker * broker )
{
    const char * namespace, * classname;
    CMPIObjectPath * cop;
    unsigned int i;
    CMPIString * tmp;

    TRACE_VERBOSE(("entered function."));

    if (__deserialized_NULL ( fd ))
    {
        return NULL;
    }

    TRACE_NORMAL(("deserializing non-NULL CMPIObjectPath."));

    tmp = __deserialize_CMPIString ( fd, broker );
    classname = CMGetCharsPtr ( tmp, NULL );
    TRACE_INFO(("classname: %s.", classname));

    tmp = __deserialize_CMPIString ( fd, broker );
    namespace = CMGetCharsPtr ( tmp, NULL );
    TRACE_INFO(("namespace: %s.", namespace));

    cop = CMNewObjectPath ( broker, namespace, classname, NULL );

    i = __deserialize_UINT32 ( fd );

    TRACE_INFO(("key count: %d", i ));

    while (i--)
    {
        CMPIString * keyName = __deserialize_CMPIString ( fd, broker );
        CMPIData data        = __deserialize_CMPIData ( fd, broker );

        TRACE_INFO(("key:\nname: %s\ntype: 0x%x\nstate: 0x%x",
            CMGetCharsPtr ( keyName, NULL ), data.type, data.state ));

        CMAddKey ( cop,
            CMGetCharsPtr ( keyName, NULL ),
            &data.value,
            data.type );
    }

    TRACE_VERBOSE(("leaving function."));
    return cop;
}


static ssize_t __serialize_CMPIArray ( int fd, CONST CMPIArray * array )
{
    CMPICount i, size;
    CMPIType type;
    ssize_t out, tmp;

    TRACE_VERBOSE(("entered function."));

    if (__serialized_NULL ( fd, array ))
    {
        return 0;
    }

    TRACE_NORMAL(("serializing non-NULL CMPIArray."));

    size = CMGetArrayCount ( array, NULL );
    out =__serialize_UINT16 ( fd, size );

    if (out < 0)
    {
        return -1;
    }

    TRACE_INFO(("element count: %d", size ));

    type = CMGetArrayType ( array, NULL );
    out += tmp = __serialize_CMPIType ( fd, type );

    if (tmp < 0)
    {
        return -1;
    }

    TRACE_INFO(("element type: 0x%x", type ));

    for (i = 0; i < size; i++)
    {

        CMPIData data = CMGetArrayElementAt ( array, i, NULL);
        out += __serialize_CMPIData ( fd, data );
    }

    TRACE_VERBOSE(("leaving function."));
    return out;
}


static CMPIArray * __deserialize_CMPIArray (int fd, CONST CMPIBroker * broker)
{
    CMPIArray * a;
    CMPICount size, i;
    CMPIType type;

    TRACE_VERBOSE(("entered function."));

    if (__deserialized_NULL ( fd ))
    {
        return NULL;
    }

    TRACE_NORMAL(("deserializing non-NULL CMPIArray."));

    size = __deserialize_UINT16 ( fd );
    TRACE_INFO(("element count: %d", size ));

    type = __deserialize_CMPIType ( fd );

    TRACE_INFO(("element type: 0x%x", type ));

    a = CMNewArray ( broker, size, type, NULL );

    for (i = 0; i < size; i++)
    {

        CMPIData data = __deserialize_CMPIData ( fd, broker );

        if (data.state & CMPI_nullValue)
        {
            CMSetArrayElementAt ( a, i, NULL, CMPI_null );
            TRACE_INFO(("added NULL value."));
        }
        else
        {
            CMSetArrayElementAt ( a, i, &data.value, data.type );
            TRACE_INFO(("added non-NULL value."));
        }
    }

    TRACE_VERBOSE(("leaving function."));
    return a;
}


static ssize_t __serialize_CMPIInstance ( int fd, CONST CMPIInstance * inst )
{
    ssize_t out, tmp;
    CMPIObjectPath * cop;
    unsigned int props, i;

    TRACE_VERBOSE(("entered function."));

    if (__serialized_NULL ( fd, inst ))
    {
        return 0;
    }

    TRACE_NORMAL(("serializing non-NULL CMPIInstance."));

    cop   = CMGetObjectPath ( inst, NULL );
    props = CMGetPropertyCount ( inst, NULL );

    out = __serialize_CMPIObjectPath ( fd, cop );
    if (out < 0)
    {
        return -1;
    }

    if (( tmp =__serialize_UINT16 ( fd, props ) ) < 0)
    {
        return -1;
    }
    out += tmp;

    TRACE_INFO(("property count: %d", props ));

    for (i = 0; i < props; i++)
    {
        CMPIString * propName;
        CMPIData data = CMGetPropertyAt ( inst,
            i,
            &propName,
            NULL );

        TRACE_INFO(("property(%d)\nname: %s\n"
            "type: 0x%x\nstate: 0x%x.",
            i, CMGetCharsPtr ( propName, NULL ),
            data.type, data.state ));

        out += __serialize_CMPIString ( fd, propName );
        out += __serialize_CMPIData ( fd, data );
    }

    TRACE_VERBOSE(("leaving function."));
    return out;
}


static CMPIInstance * __deserialize_CMPIInstance (
    int fd,
    CONST CMPIBroker * broker ,
    CONST CMPIObjectPath *qop)
{
    CMPIObjectPath * cop;
    CMPIInstance * inst;
    unsigned int i;

    TRACE_VERBOSE(("entered function."));

    if (__deserialized_NULL ( fd ))
    {
        return NULL;
    }

    TRACE_NORMAL(("deserializing non-NULL CMPIInstance."));

    cop = __deserialize_CMPIObjectPath ( fd, broker );
    // If CMPIObjectPath is passed, set the namespace from passed ObjectPath.
    if (qop)
    {
        CMSetNameSpaceFromObjectPath(cop, qop);
    }
    inst = CMNewInstance ( broker, cop, NULL );

    i = __deserialize_UINT16 ( fd );

    while (i--)
    {
        CMPIString * propName =
            __deserialize_CMPIString ( fd, broker );

        CMPIData data =
            __deserialize_CMPIData ( fd, broker );

        TRACE_INFO(("property:\nname: %s\ntype: 0x%x\nstate: 0x%x",
            CMGetCharsPtr ( propName, NULL ), data.type, data.state));

        CMSetProperty ( inst,
            CMGetCharsPtr ( propName, NULL ),
            &data.value,
            data.type );
    }

    TRACE_VERBOSE(("leaving function."));
    return inst;
}

static ssize_t __serialize_CMPISelectExp (
    int fd,
    CONST CMPISelectExp * sexp,
    CMPIUint32 ctx_id)
{

    if (__serialized_NULL (fd, sexp))
    {
        return 0;
    }

    return (ssize_t)__serialize_UINT64 (
        fd,
        create_indicationObject (
        (CMPISelectExp*)sexp,
        ctx_id,
        PEGASUS_INDICATION_OBJECT_TYPE_CMPI_SELECT_EXP) );
}

static CMPISelectExp * __deserialize_CMPISelectExp (
    int fd,
    CONST CMPIBroker *broker,
    CMPIUint32 ctx_id)
{
    if (__deserialized_NULL (fd))
    {
        return NULL;
    }

    return(CMPISelectExp*) get_indicationObject (
        __deserialize_UINT64 (fd),
        ctx_id);
}

static ssize_t __serialize_CMPIDateTime ( int fd, CMPIDateTime * dt )
{
    CMPIUint64 msecs;
    CMPIBoolean i;
    ssize_t out, tmp;

    TRACE_VERBOSE(("entered function."));

    if (__serialized_NULL ( fd, dt ))
    {
        return 0;
    }

    TRACE_NORMAL(("serializing non-NULL CMPIDateTime."));

    msecs = CMGetBinaryFormat ( dt, NULL );
    i = CMIsInterval ( dt, NULL );
#if defined CMPI_PLATFORM_WIN32_IX86_MSVC
    TRACE_INFO(("msecs: %I64d, interval: %d", msecs, i ));
#else
    TRACE_INFO(("msecs: %lld, interval: %d", msecs, i ));
#endif
    out = __serialize_UINT64 ( fd, msecs );
    if (out < 0)
    {
        return -1;
    }

    tmp = __serialize_UINT8 ( fd, i );
    if (tmp < 0)
    {
        return -1;
    }

    TRACE_VERBOSE(("leaving function."));
    return( out + tmp );
}


static CMPIDateTime * __deserialize_CMPIDateTime (
    int fd,
    CONST CMPIBroker * broker )
{
    CMPIUint64 msecs;
    CMPIBoolean i;

    TRACE_VERBOSE(("entered function."));

    if (__deserialized_NULL ( fd ))
    {
        return NULL;
    }

    TRACE_NORMAL(("deserializing non-NULL CMPIDateTime."));

    msecs = __deserialize_UINT64 ( fd );
    i     = __deserialize_UINT8 ( fd );

#if defined CMPI_PLATFORM_WIN32_IX86_MSVC
    TRACE_INFO(("msecs: %I64d, interval: %d", msecs, i ));
#else
    TRACE_INFO(("msecs: %lld, interval: %d", msecs, i ));
#endif
    TRACE_VERBOSE(("leaving function."));
    return CMNewDateTimeFromBinary ( broker, msecs, i, NULL );
}

#ifdef CMPI_VER_200

static ssize_t __serialize_CMPIError ( int fd, CMPIError * err )
{

    TRACE_VERBOSE(("entered function."));

    if (__serialized_NULL ( fd, err ))
    {
        return 0;
    }

    TRACE_NORMAL(("serializing non-NULL CMPIError."));

    TRACE_VERBOSE(("leaving function."));
    return( 0 );
}


static CMPIError * __deserialize_CMPIError (
    int fd,
    CONST CMPIBroker * broker )
{
    CMPIError* nerr=0;

    TRACE_VERBOSE(("entered function."));

    if (__deserialized_NULL ( fd ))
    {
        return NULL;
    }

    TRACE_NORMAL(("deserializing non-NULL CMPIError."));

    return nerr;
}

static ssize_t __serialize_CMPIErrorSeverity ( int fd, CMPIErrorSeverity sev )
{
    return 0;
}

static CMPIErrorSeverity __deserialize_CMPIErrorSeverity (
    int fd,
    CONST CMPIBroker * broker )
{
    
    CMPIErrorSeverity sev = ErrorSevUnknown ;
    return sev;
}

static ssize_t __serialize_CMPIErrorProbableCause (
    int fd,
    CMPIErrorProbableCause pc )
{
    return 0;
}

static CMPIErrorProbableCause __deserialize_CMPIErrorProbableCause (
    int fd,
    CONST CMPIBroker * broker )
{
    CMPIErrorProbableCause pc= ErrorProbCauseUnknown;
    return pc;
}

static ssize_t __serialize_CMPIrc ( int fd, CMPIrc cimStatusCode )
{
    return 0;
}

static CMPIrc __deserialize_CMPIrc ( int fd, CONST CMPIBroker * broker )
{
    CMPIrc cimStatusCode = CMPI_RC_OK ;
    return cimStatusCode;
}

static ssize_t __serialize_CMPIMsgFileHandle (
    int fd,
    CMPIMsgFileHandle msgFileHandle )
{
    ssize_t handle = (ssize_t)msgFileHandle;
    return(__serialize_UINT32 ( fd, (CMPIUint32)handle));
}

static CMPIMsgFileHandle __deserialize_CMPIMsgFileHandle (
    int fd,
    CONST CMPIBroker * broker )
{
    ssize_t handle = __deserialize_UINT32(fd);
    return((CMPIMsgFileHandle)handle);
}


#endif /* CMPI_VER_200 */


/****************************************************************************/


/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
