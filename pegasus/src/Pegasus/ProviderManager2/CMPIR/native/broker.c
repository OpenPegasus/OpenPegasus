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
    \file broker.c
    \brief Native CMPI broker encapsulated functionality.

    This module implements a complete CMPI broker encapsulated function table
    (CMPIBrokerEncFT) natively. Thus, CMPI data types may be created remotely
    without the need to connect to the CIMOM.

    \author Frank Scheffler
*/

#include "cmpir_common.h"
#include "tool.h"
#include "native.h"
#include "debug.h"
#include "mm.h"

static CMPIInstance * __beft_newInstance (
    CONST CMPIBroker * broker,
    CONST CMPIObjectPath * cop,
    CMPIStatus * rc )
{
    CMPIStatus rc1 = checkArgsReturnStatus(cop);
    if (rc1.rc != CMPI_RC_OK)
    {
        CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
        return 0;
    }
    TRACE_NORMAL(("Creating new native CMPIInstance."));
    return native_new_CMPIInstance ( cop, rc );
}


static CMPIObjectPath * __beft_newObjectPath (
    CONST CMPIBroker * broker,
    const char * namespace,
    const char * classname,
    CMPIStatus * rc )
{
    if (!classname)
    {
        CMSetStatus (rc, CMPI_RC_ERR_NOT_FOUND);
        return 0;
    }
    TRACE_NORMAL(("Creating new native CMPIObjectPath."));
    return native_new_CMPIObjectPath ( namespace, classname, rc );
}



static CMPIArgs * __beft_newArgs ( CONST CMPIBroker * broker, CMPIStatus * rc )
{
    TRACE_NORMAL(("Creating new native CMPIArgs."));
    return native_new_CMPIArgs ( rc );
}


static CMPIString * __beft_newString (
    CONST CMPIBroker * broker,
    const char * str,
    CMPIStatus * rc )
{
    if (!str)
    {
        CMSetStatus (rc, CMPI_RC_ERR_INVALID_PARAMETER);
        return 0;
    }
    TRACE_NORMAL(("Creating new native CMPIString."));
    TRACE_INFO(("String: %s", str ));
    return native_new_CMPIString ( str, rc );
}


static CMPIArray * __beft_newArray (
    CONST CMPIBroker * broker,
    CMPICount size,
    CMPIType type,
    CMPIStatus * rc )
{
    TRACE_NORMAL(("Creating new native CMPIArray."));
    TRACE_INFO(("type: 0x%x\nsize: %d", type, size ));
    return native_new_CMPIArray ( size, type, rc );
}



static CMPIDateTime * __beft_newDateTime (
    CONST CMPIBroker * broker,
    CMPIStatus * rc )
{
    TRACE_NORMAL(("Creating new native CMPIDateTime."));
    return native_new_CMPIDateTime ( rc );
}


static CMPIDateTime * __beft_newDateTimeFromBinary (
    CONST CMPIBroker * broker,
    CMPIUint64 time,
    CMPIBoolean interval,
    CMPIStatus * rc )
{
    TRACE_NORMAL(("Creating new native CMPIDateTime."));
#if defined CMPI_PLATFORM_WIN32_IX86_MSVC
    TRACE_INFO(("time: %I64\ninterval: %d", time, interval ));
#else
    TRACE_INFO(("time: %lld\ninterval: %d", time, interval ));
#endif
    return native_new_CMPIDateTime_fromBinary ( time, interval, rc );
}



static CMPIDateTime * __beft_newDateTimeFromChars (
    CONST CMPIBroker * broker,
    CONST char * string,
    CMPIStatus * rc )
{
    TRACE_NORMAL(("Creating new native CMPIDateTime."));
    TRACE_INFO(("time: %s", string ));
    return native_new_CMPIDateTime_fromChars ( string, rc );
}


static CMPISelectExp * __beft_newSelectExp (
    CONST CMPIBroker * broker,
    const char * queryString,
    const char * language,
    CMPIArray ** projection,
    CMPIStatus * rc )
{
    CMPIBroker *brk;
    CMPIContext *ctx;

    if (!queryString || !language)
    {
        CMSetStatus (rc, CMPI_RC_ERR_INVALID_PARAMETER);
        return 0;
    }
    TRACE_NORMAL(("Creating new native CMPISelectExp."));
    brk = tool_mm_get_broker ( (void**)&ctx);

    return( ( (NativeCMPIBrokerFT*)broker->bft) )->selExp_newSelExp(
        queryString,
        language,
        projection,
        rc );
}

static CMPIBoolean __beft_classPathIsA (
    CONST CMPIBroker * broker,
    CONST CMPIObjectPath * cop,
    const char * type,
    CMPIStatus * rc )
{

    CMPIString *clsn;

    CMPIStatus rc1 = checkArgsReturnStatus(cop);
    if (rc1.rc != CMPI_RC_OK || !type)
    {
        CMSetStatus (rc, CMPI_RC_ERR_INVALID_PARAMETER);
        return 0;
    }

    CMSetStatus(rc,CMPI_RC_OK);

    clsn=CMGetClassName(cop,NULL);

    if (clsn && clsn->hdl)
    {
        if (PEGASUS_CMPIR_STRCASECMP (type,(char*)clsn->hdl)==0)
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }

    return((NativeCMPIBrokerFT*)(broker->bft))->classPathIsA(
        broker,
        cop,
        type,
        rc);
}

extern CMPIObjectPathFT *CMPI_ObjectPath_FT;
extern CMPIInstanceFT *CMPI_Instance_FT;
extern CMPIArgsFT *CMPI_Args_FT;

extern CMPIString *__oft_toString( CONST CMPIObjectPath * cop, CMPIStatus *rc);
extern CMPIString *instance2String( CONST CMPIInstance *inst, CMPIStatus *rc);
extern CMPIString *args2String( CONST CMPIArgs *args, CMPIStatus *rc);

static CMPIString * __beft_toString (
    CONST CMPIBroker * broker,
    CONST void * object,
    CMPIStatus * rc )
{
    if (object)
    {
        if (((CMPIInstance*)object)->ft)
        {
            if (((CMPIObjectPath*)object)->ft==CMPI_ObjectPath_FT)
            {
                return __oft_toString((CMPIObjectPath*)object,rc);
            }
            if (((CMPIInstance*)object)->ft==CMPI_Instance_FT)
            {
                return instance2String((CMPIInstance*)object,rc);
            }
            if (((CMPIArgs*)object)->ft==CMPI_Args_FT)
            {
                return args2String((CMPIArgs*)object,rc);
            }
        }
    }
    TRACE_CRITICAL(("This operation is not yet supported."));
    CMSetStatus ( rc, CMPI_RC_ERR_NOT_SUPPORTED );
    return NULL;
}


static CMPIBoolean __beft_isOfType (
    CONST CMPIBroker * broker,
    CONST void * object,
    const char * type,
    CMPIStatus * rc )
{
    char *t;
    if (!object || !type)
    {
        CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
        return 0;
    }
    t =  object == broker ? "CMPIBroker" : *(char **)object;
    TRACE_NORMAL(("Verifying encapsulated object type."));

    CMSetStatus ( rc, CMPI_RC_OK );
    return( strcmp ( t, type ) == 0 );
}


static CMPIString * __beft_getType (
    CONST CMPIBroker * broker,
    CONST void * object,
    CMPIStatus * rc )
{
    char *t;

    if (!object)
    {
        CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
        return 0;
    }
    TRACE_NORMAL(("Returning encapsulated object type."));
    t = object == broker ? "CMPIBroker" : *(char **)object;

    return __beft_newString ( broker, t, rc );
}


static CMPIString*  __beft_getMessage (
    CONST CMPIBroker* broker,
    const char *msgId,
    const char *defMsg,
    CMPIStatus* rc,
    unsigned int count,
    ...)
{
    CMPIStatus nrc;
    CMPIString *msg;
    va_list argptr;
    va_start(argptr,count);

    msg = ((NativeCMPIBrokerFT*)(broker->bft))->getMessage(
        broker,
        msgId,
        defMsg,
        &nrc,
        count,
        argptr);

    va_end(argptr);
    if (rc) *rc=nrc;
    return msg;
}

#if defined CMPI_VER_100

static CMPIStatus __beft_logMessage(
    const CMPIBroker*broker,
    CMPISeverity severity ,
    const char *id,
    const char *text,
    const CMPIString *string)
{
    return((NativeCMPIBrokerFT*)(broker->bft))->logMessage(
        broker,
        severity,
        id,
        text,
        string);
}


static CMPIStatus __beft_traceMessage(
    const CMPIBroker* broker,
    CMPILevel level,
    const char *component,
    const char *text,
    const CMPIString *string)
{
    return((NativeCMPIBrokerFT*)(broker->bft))->trace(
        broker,
        level,
        component,
        text,
        string);
}

#else

static CMPIArray *__beft_getKeyNames(
    CMPIBroker * broker,
    CMPIContext * context,
    CMPIObjectPath * cop,
    CMPIStatus * rc)
{
    return((NativeCMPIBrokerFT*)(broker->bft))->getKeyNames(
        broker,
        context,
        cop,
        rc);
}

#endif

#if defined CMPI_VER_200

static CMPIError* __beft_newCMPIError (
    const CMPIBroker* broker,
    const char* owner,
    const char* msgID,
    const char* msg,
    const CMPIErrorSeverity sev,
    const CMPIErrorProbableCause pc,
    const CMPIrc cimStatusCode,
    CMPIStatus* rc)
{
    CMPIStatus nrc;
    CMPIError *nerr;

    nerr = ((NativeCMPIBrokerFT*)(broker->bft))->newCMPIError(
        broker,
        owner,
        msgID,
        msg,
        sev,
        pc,
        cimStatusCode,
        &nrc);
    if (rc)
    {
        *rc=nrc;
    }
    return nerr;
}


static CMPIStatus __beft_openMessageFile(
    const CMPIBroker *broker,
    const char* msgFile,
    CMPIMsgFileHandle* msgFileHandle)
{
    return((NativeCMPIBrokerFT*)(broker->bft))->openMessageFile(
        broker,
        msgFile,
        msgFileHandle);
}

static CMPIStatus __beft_closeMessageFile(
    const CMPIBroker *broker,
    const CMPIMsgFileHandle msgFileHandle)
{
    return((NativeCMPIBrokerFT*)(broker->bft))->closeMessageFile(
        broker,
        msgFileHandle);
}

static CMPIString* __beft_getMessage2(
    const CMPIBroker *broker,
    const char *msgId,
    const CMPIMsgFileHandle msgFileHandle,
    const char *defMsg,
    CMPIStatus* rc,
    CMPICount count,
    ...)
{
    CMPIStatus nrc;
    CMPIString *msg;
    va_list argptr;
    va_start(argptr,count);

    msg = ((NativeCMPIBrokerFT*)(broker->bft))->getMessage2(
        broker,
        msgId,
        msgFileHandle,
        defMsg,
        &nrc,
        count,
        argptr);

    va_end(argptr);
    if (rc)
    {
        *rc=nrc;
    }
    return msg;
}

#endif

/****************************************************************************/


PEGASUS_EXPORT CMPIBrokerEncFT native_brokerEncFT =
{
    NATIVE_FT_VERSION,
    __beft_newInstance,
    __beft_newObjectPath,
    __beft_newArgs,
    __beft_newString,
    __beft_newArray,
    __beft_newDateTime,
    __beft_newDateTimeFromBinary,
    __beft_newDateTimeFromChars,
    __beft_newSelectExp,
    __beft_classPathIsA,
    __beft_toString,
    __beft_isOfType,
    __beft_getType,
    __beft_getMessage,
#ifdef CMPI_VER_100
    __beft_logMessage,
    __beft_traceMessage,
#else
    __beft_getKeyNames,
#endif
#ifdef CMPI_VER_200
    __beft_newCMPIError,
    __beft_openMessageFile,
    __beft_closeMessageFile,
    __beft_getMessage2,
#endif
};

/*
   Utility functions that check for Args and hdl pointers.
*/

CMPIStatus checkArgsReturnStatus(const void *ptr)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL};

    if (!ptr || !(ssize_t*)(*( (ssize_t*)ptr) ))
    {
        rc.rc = CMPI_RC_ERR_INVALID_HANDLE;
    }

    return rc;
}

CMPIData checkArgsReturnData(const void *ptr, CMPIStatus *rc)
{
    CMPIStatus status = checkArgsReturnStatus(ptr);
    CMPIData data;
    data.state = CMPI_nullValue;

    if (status.rc != CMPI_RC_OK)
    {
        if (rc)
        {
            rc->rc = status.rc;
        }
        data.state = CMPI_badValue;
    }

    return data;
}

void* checkArgs(const void *ptr, CMPIStatus *rc)
{
    CMPIStatus status = checkArgsReturnStatus(ptr);

    if (status.rc != CMPI_RC_OK)
    {
        if (rc)
        {
            rc->rc = status.rc;
        }
        return 0;
    }

    return(void*)ptr; // Args are good, return the pointer we got
}



/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
