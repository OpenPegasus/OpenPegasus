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
    \file tcpcomm_remote.c
    \brief TCP/IP communication layer (remote side module).

    This module implements the remote side functionality to support TCP/IP
    driven remote providers. This comprises setting up a daemon listening
    for MI requests.

    \sa tcpcomm_proxy.c
    \sa remote_broker.c
    \sa socketcomm.h
*/

#define PEGASUS_TCPCOMM_REMOTE 1

#include "cmpir_common.h"


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# include <winsock2.h>
#elif defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
# include <error.h>
#endif

#include "tcpcomm.h"
#include "ip.h"
#include "remote.h"
#include "serialization.h"
#include "io.h"
#include "mm.h"
#include "native.h"
#include "debug.h"
#include "socketcomm.h"
#include "indication_objects.h"

#include <Pegasus/Provider/CMPI/cmpidt.h>

PEGASUS_IMPORT extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
# define lloonngg __int64
#else
# define lloonngg long long int
#endif

int nativeSide=1;

PEGASUS_IMPORT CMPI_MUTEX_TYPE pegthreadOnceMutex;

/****************************************************************************/

static int __broker_connect(
    CONST CMPIBroker *,
    CONST CMPIContext *,
    const char *);

static const struct BinarySerializerFT *__sft = &binarySerializerFT;

/****************************************************************************/

#define GET_INSTANCE_MI(broker,ctx) \
    RBGetInstanceMI ( CMPIBroker2remote_broker ( (broker) ) )
#define GET_ASSOCIATION_MI(broker,ctx) \
    RBGetAssociationMI ( CMPIBroker2remote_broker ( (broker) ) )
#define GET_METHOD_MI(broker,ctx) \
    RBGetMethodMI ( CMPIBroker2remote_broker ( (broker) ) )
#define GET_PROPERTY_MI(broker,ctx) \
    RBGetPropertyMI ( CMPIBroker2remote_broker ( (broker) ) )
#define GET_INDICATION_MI(broker,ctx) \
    RBGetIndicationMI ( CMPIBroker2remote_broker ( (broker) ) )

#define NOCLEAN(mi,ctx)

static const char* mierrMsg =
    "ProviderInitFailure: Error initializing the API's _Create<mi-type>MI";
#define SET_STATUS_INIT_FAILED(rc) \
    rc.msg = native_new_CMPIString (mierrMsg, NULL); \
    rc.rc = CMPI_RC_ERR_FAILED; \
    TRACE_CRITICAL((mierrMsg));

static void TCPCOMM_InstanceMI_enumInstanceNames(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIInstanceMI *mi = RBGetInstanceMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->enumerateInstanceNames(mi, ctx, result, cop);;
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
}


static void TCPCOMM_InstanceMI_enumInstances(
    int socket, CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char **props = socketcomm_deserialize_props(socket, (__sft), broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIInstanceMI *mi = RBGetInstanceMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->
            enumerateInstances(mi, ctx, result, cop, (CONST char **)props);
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
    free(props);
}


static void TCPCOMM_InstanceMI_getInstance(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char **props = socketcomm_deserialize_props(socket, (__sft), broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIInstanceMI *mi = RBGetInstanceMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->getInstance(mi, ctx, result, cop, (CONST char **)props);
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
    free(props);
}


static void TCPCOMM_InstanceMI_createInstance(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPIInstance *inst = (__sft)->deserialize_CMPIInstance(socket, broker, cop);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIInstanceMI *mi = RBGetInstanceMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->createInstance(mi, ctx, result, cop, inst);;
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
}


static void TCPCOMM_InstanceMI_setInstance(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPIInstance *inst = (__sft)->deserialize_CMPIInstance(socket, broker, cop);
    char **props = socketcomm_deserialize_props(socket, (__sft), broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIInstanceMI *mi = RBGetInstanceMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
#ifdef CMPI_VER_100
        rc = mi->ft->modifyInstance(
            mi,
            ctx,
            result,
            cop,
            inst,
            (const char **)props);
#else
        rc = mi->ft->setInstance(
            mi,
            ctx,
            result,
            cop,
            inst,
            (CONST char **)props);
#endif
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
    free(props);
}


static void TCPCOMM_InstanceMI_deleteInstance(
    int socket,
    CONST  CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIInstanceMI *mi = RBGetInstanceMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker,(CMPIContext *) ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->deleteInstance(mi, ctx, result, cop);;
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
}


static void TCPCOMM_InstanceMI_execQuery(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char *lang = (__sft)->deserialize_string(socket, broker);
    char *query = (__sft)->deserialize_string(socket, broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIInstanceMI *mi = RBGetInstanceMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->execQuery(mi, ctx, result, cop, lang, query);;
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
};


static void TCPCOMM_AssociationMI_associators(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char *assocclass = (__sft)->deserialize_string(socket, broker);
    char *resultclass = (__sft)->deserialize_string(socket, broker);
    char *role = (__sft)->deserialize_string(socket, broker);
    char *resultrole = (__sft)->deserialize_string(socket, broker);
    char **props = socketcomm_deserialize_props(socket, (__sft), broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIAssociationMI *mi =
        RBGetAssociationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->associators(
            mi,
            ctx,
            result,
            cop,
            assocclass,
            resultclass,
            role,
            resultrole,
            (CONST char **)props);
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
    free(props);
}


static void TCPCOMM_AssociationMI_associatorNames(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char *assocclass = (__sft)->deserialize_string(socket, broker);
    char *resultclass = (__sft)->deserialize_string(socket, broker);
    char *role = (__sft)->deserialize_string(socket, broker);
    char *resultrole = (__sft)->deserialize_string(socket, broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIAssociationMI *mi =
        RBGetAssociationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->associatorNames(
            mi,
            ctx,
            result,
            cop,
            assocclass,
            resultclass,
            role,
            resultrole);
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
}


static void TCPCOMM_AssociationMI_references(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char *assocclass = (__sft)->deserialize_string(socket, broker);
    char *role = (__sft)->deserialize_string(socket, broker);
    char **props = socketcomm_deserialize_props(socket, (__sft), broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIAssociationMI *mi =
        RBGetAssociationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->references(
            mi,
            ctx,
            result,
            cop,
            assocclass,
            role,
            (CONST char **)props);
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
    free(props);
}


static void TCPCOMM_AssociationMI_referenceNames(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char *assocclass = (__sft)->deserialize_string(socket, broker);
    char *role = (__sft)->deserialize_string(socket, broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIAssociationMI *mi =
        RBGetAssociationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->referenceNames(mi, ctx, result, cop, assocclass, role);
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
};



static void TCPCOMM_MethodMI_invokeMethod(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char *method = (__sft)->deserialize_string(socket, broker);
    CMPIArgs *in = (__sft)->deserialize_CMPIArgs(socket, broker);
    CMPIArgs *out = CMNewArgs(broker, NULL);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIMethodMI *mi = RBGetMethodMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->invokeMethod(mi, ctx, result, cop, method, in, out);
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
    (__sft)->serialize_CMPIArgs(socket, out);
};



static void TCPCOMM_PropertyMI_getProperty(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char *name = (__sft)->deserialize_string(socket, broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIPropertyMI *mi = RBGetPropertyMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->getProperty(mi, ctx, result, cop, name);;
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
}


static void TCPCOMM_PropertyMI_setProperty(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    char *name = (__sft)->deserialize_string(socket, broker);
    CMPIData data = (__sft)->deserialize_CMPIData(socket, broker);
    CMPIStatus rc;
    CMPIArray *r;
    CMPIResult *result = native_new_CMPIResult(NULL);
    CMPIPropertyMI *mi = RBGetPropertyMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (mi)
    {
        rc = mi->ft->setProperty(mi, ctx, result, cop, name, data);
    }
    else
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIArray(socket, r);;
};



static void TCPCOMM_IndicationMI_authorizeFilter(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPISelectExp *filter = (__sft)->deserialize_CMPISelectExp(
        socket,
        broker,
        PEGASUS_INDICATION_GLOBAL_CONTEXT);

    char *indType = (__sft)->deserialize_string(socket, broker);
    char *owner = (__sft)->deserialize_string(socket, broker);
    CMPIStatus rc;
#ifndef CMPI_VER_100
    CMPIResult *result = native_new_CMPIResult(NULL);
#endif
    CMPIIndicationMI *mi =
        RBGetIndicationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
#ifdef CMPI_VER_100
    if (mi)
    {
        rc = mi->ft->authorizeFilter(mi, ctx, filter, indType, cop, owner);
    }
#else
     CMPIArray *r=NULL;
    if (mi)
    {
        rc = mi->ft->authorizeFilter(
            mi,
            ctx,
            result,
            filter,
            indType,
            cop,
            owner);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIArray(socket, r);;
#endif
    if (!mi)
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    (__sft)->serialize_CMPIStatus(socket, &rc);
}


static void TCPCOMM_IndicationMI_mustPoll(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPISelectExp *filter = (__sft)->deserialize_CMPISelectExp(
        socket,
        broker,
        PEGASUS_INDICATION_GLOBAL_CONTEXT);
    char *indType = (__sft)->deserialize_string(socket, broker);
    CMPIStatus rc;
#ifndef CMPI_VER_100
    CMPIResult *result = native_new_CMPIResult(NULL);
#endif
    CMPIIndicationMI *mi =
        RBGetIndicationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
#ifdef CMPI_VER_100
    if (mi)
    {
        rc = mi->ft->mustPoll(mi, ctx,  filter, indType, cop);
    }
#else
    CMPIArray *r=NULL;
    if (mi)
    {
        rc = mi->ft->mustPoll(mi, ctx, result, filter, indType, cop);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIArray(socket, r);
#endif
    if (!mi)
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    (__sft)->serialize_CMPIStatus(socket, &rc);
}


static void TCPCOMM_IndicationMI_activateFilter(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    // This filter is not deleted when the current thread is finished
    // execution. This is the persistent filter on MB side.
    // Delete this filter in deactiveFilter -V 5245
    CMPISelectExp *filter = (__sft)->deserialize_CMPISelectExp(
        socket,
        broker,
        PEGASUS_INDICATION_GLOBAL_CONTEXT);
    char *clsName = (__sft)->deserialize_string(socket, broker);
    CMPIBoolean firstActivation = (__sft)->deserialize_UINT8(socket);
    CMPIStatus rc;
#ifndef CMPI_VER_100
    CMPIResult *result = native_new_CMPIResult(NULL);
#endif
    CMPIIndicationMI *mi =
        RBGetIndicationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
#ifdef CMPI_VER_100
    if (mi)
    {
        rc = mi->ft->activateFilter(
            mi,
            ctx,
            filter,
            clsName,
            cop,
            firstActivation);;
    }
#else
    CMPIArray *r=NULL;
    if (mi)
    {
        rc = mi->ft->activateFilter(
            mi,
            ctx,
            result,
            filter,
            clsName,
            cop,
            firstActivation);;
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIArray(socket, r);
#endif
    if (!mi)
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    (__sft)->serialize_CMPIStatus(socket, &rc);
}


static void TCPCOMM_IndicationMI_deActivateFilter(
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPISelectExp *filter = (__sft)->deserialize_CMPISelectExp(
        socket,
        broker,
        PEGASUS_INDICATION_GLOBAL_CONTEXT);
    char *clsName = (__sft)->deserialize_string(socket, broker);
    CMPIBoolean lastActivation = (__sft)->deserialize_UINT8(socket);
    CMPIStatus rc;
#ifndef CMPI_VER_100
    CMPIResult *result = native_new_CMPIResult(NULL);
#endif
    CMPIIndicationMI *mi =
        RBGetIndicationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
#ifdef CMPI_VER_100
    if (mi)
    {
        rc = mi->ft->deActivateFilter(
            mi,
            ctx,
            filter,
            clsName,
            cop,
            lastActivation);
    }
#else
    CMPIArray *r=NULL;
    if (mi)
    {
        rc = mi->ft->deActivateFilter(
            mi,
            ctx,
            result,
            filter,
            clsName,
            cop,
            lastActivation);
    }
    r = native_result2array(result);
    (__sft)->serialize_CMPIArray(socket, r);
#endif
    if (!mi)
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    // Now delete the original filter we got in activateFilter. -V 5245
    native_release_CMPISelectExp (filter);
    (__sft)->serialize_CMPIStatus(socket, &rc);
};

static void TCPCOMM_IndicationMI_enableIndications (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPIStatus rc = { CMPI_RC_OK , NULL};

    CMPIIndicationMI *mi =
        RBGetIndicationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (!mi)
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    else
    {
        // To make compatibilty with old providers it was decided to discard the
        // returned CMPIStatus here. This will prevent us from breaking existing
        // CMPI Indication providers. -V 5886
#ifdef CMPI_VER_100
        mi->ft->enableIndications(mi, ctx);
#else
        mi->ft->enableIndications(mi);
#endif
    }
    (__sft)->serialize_CMPIStatus(socket, &rc);
}

static void TCPCOMM_IndicationMI_disableIndications (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    CONST CMPIObjectPath * cop)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL};
    CMPIIndicationMI *mi =
        RBGetIndicationMI(CMPIBroker2remote_broker((broker)));
    TRACE_NORMAL(("Executing remote MI request."));
    tool_mm_set_broker((CMPIBroker *)broker, (CMPIContext *)ctx);
    START_DEBUGGER;
    if (!mi)
    {
        SET_STATUS_INIT_FAILED(rc);
    }
    // To make compatibilty with old providers it was decided to discard the
    // returned CMPIStatus here. This will prevent us from breaking existing
    // CMPI Indication providers. -V 5886
#ifdef CMPI_VER_100
    mi->ft->disableIndications(mi, ctx);
#else
    mi->ft->disableIndications(mi);
#endif
    (__sft)->serialize_CMPIStatus(socket, &rc);
}

static struct socket_mi_function __mi_functions[] =
{
    {"TCPCOMM_InstanceMI_enumInstanceNames",
        TCPCOMM_InstanceMI_enumInstanceNames},
    {"TCPCOMM_InstanceMI_enumInstances",
        TCPCOMM_InstanceMI_enumInstances},
    {"TCPCOMM_InstanceMI_getInstance",
        TCPCOMM_InstanceMI_getInstance},
    {"TCPCOMM_InstanceMI_createInstance",
        TCPCOMM_InstanceMI_createInstance},
    {"TCPCOMM_InstanceMI_setInstance",
        TCPCOMM_InstanceMI_setInstance},
    {"TCPCOMM_InstanceMI_deleteInstance",
        TCPCOMM_InstanceMI_deleteInstance},
    {"TCPCOMM_InstanceMI_execQuery",
        TCPCOMM_InstanceMI_execQuery},
    {"TCPCOMM_AssociationMI_associators",
        TCPCOMM_AssociationMI_associators},
    {"TCPCOMM_AssociationMI_associatorNames",
        TCPCOMM_AssociationMI_associatorNames},
    {"TCPCOMM_AssociationMI_references",
        TCPCOMM_AssociationMI_references},
    {"TCPCOMM_AssociationMI_referenceNames",
        TCPCOMM_AssociationMI_referenceNames},
    {"TCPCOMM_MethodMI_invokeMethod",
        TCPCOMM_MethodMI_invokeMethod},
    {"TCPCOMM_PropertyMI_getProperty",
        TCPCOMM_PropertyMI_getProperty},
    {"TCPCOMM_PropertyMI_setProperty",
        TCPCOMM_PropertyMI_setProperty},
    {"TCPCOMM_IndicationMI_authorizeFilter",
        TCPCOMM_IndicationMI_authorizeFilter},
    {"TCPCOMM_IndicationMI_mustPoll",
        TCPCOMM_IndicationMI_mustPoll},
    {"TCPCOMM_IndicationMI_activateFilter",
        TCPCOMM_IndicationMI_activateFilter},
    {"TCPCOMM_IndicationMI_deActivateFilter",
        TCPCOMM_IndicationMI_deActivateFilter},
    {"TCPCOMM_IndicationMI_enableIndications",
        TCPCOMM_IndicationMI_enableIndications},
    {"TCPCOMM_IndicationMI_disableIndications",
        TCPCOMM_IndicationMI_disableIndications}
};

static CMPIContext *TCPCOMM_prepareAttachThread(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context)
{
    CMPIStatus *rc = NULL;
    CMPIContext *r = native_new_CMPIContext(TOOL_MM_NO_ADD);
    int socket = __broker_connect(
        broker,
        context,
        "TCPCOMM_prepareAttachThread");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    socketcomm_deserialize_context(socket, (__sft), broker, r);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return r;
}


static CMPIStatus TCPCOMM_attachThread(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context)
{
    // Add the current thread to memory Management. -V 5245
    tool_mm_set_broker( (CMPIBroker*)broker, (CMPIContext*)context);
    CMReturn(CMPI_RC_OK);
}


static CMPIStatus TCPCOMM_detachThread(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context)
{
    CMPIStatus __rc, *rc = &__rc;
    int socket = __broker_connect(broker, context, "TCPCOMM_detachThread");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    }; {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
            *(rc) = __rc;
    };
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    native_release_CMPIContext(context);
    return __rc;
}



static CMPIStatus TCPCOMM_deliverIndication(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    const char *ns,
    CONST CMPIInstance * ind)
{
    CMPIStatus __rc, *rc = &__rc;
    int socket = __broker_connect(
        broker,
        context,
        "TCPCOMM_deliverIndication");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    };
    (__sft)->serialize_string(socket, ns);
    (__sft)->serialize_CMPIInstance(socket, ind); {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
            *(rc) = __rc;
    };
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return __rc;
}



static CMPIEnumeration *TCPCOMM_enumInstanceNames(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    CMPIStatus * rc)
{
    CMPIArray *r;
    int socket = __broker_connect(
        broker,
        context,
        "TCPCOMM_enumInstanceNames");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop); {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
            *(rc) = __rc;
    };
    r = (__sft)->deserialize_CMPIArray(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return(r) ? native_new_CMPIEnumeration(r, NULL) : NULL;
}



static CMPIInstance *TCPCOMM_getInstance(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    CONST char **properties,
    CMPIStatus * rc)
{
    CMPIInstance *i;
    int socket = __broker_connect(broker, context, "TCPCOMM_getInstance");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    socketcomm_serialize_props(socket, (__sft), (char **)properties);

    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    i = (__sft)->deserialize_CMPIInstance(socket, broker, cop);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return i;
}



static CMPIObjectPath *TCPCOMM_createInstance(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    CONST CMPIInstance * instance,
    CMPIStatus * rc)
{
    CMPIObjectPath *o;
    int socket = __broker_connect(broker, context, "TCPCOMM_createInstance");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_CMPIInstance(socket, instance);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    o = (__sft)->deserialize_CMPIObjectPath(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return o;
}



static CMPIStatus TCPCOMM_setInstance(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    CONST CMPIInstance * instance,
    CONST char **properties)
{
    CMPIStatus __rc, *rc = &__rc;
    int socket = __broker_connect(broker, context, "TCPCOMM_setInstance");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_CMPIInstance(socket, instance);
    socketcomm_serialize_props(socket, (__sft), (char **)properties);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return __rc;
}



static CMPIStatus TCPCOMM_deleteInstance(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop)
{
    CMPIStatus __rc, *rc = &__rc;
    int socket = __broker_connect(broker, context, "TCPCOMM_deleteInstance");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return __rc;
}



static CMPIEnumeration *TCPCOMM_execQuery(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    const char *query,
    const char *lang, CMPIStatus * rc)
{
    CMPIArray *r;
    int socket = __broker_connect(broker, context, "TCPCOMM_execQuery");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket, query);
    (__sft)->serialize_string(socket, lang);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    r = (__sft)->deserialize_CMPIArray(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return(r) ? native_new_CMPIEnumeration(r, NULL) : NULL;
}



static CMPIEnumeration *TCPCOMM_enumInstances(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    CONST char **properties,
    CMPIStatus * rc)
{
    CMPIArray *r;
    int socket = __broker_connect(broker, context, "TCPCOMM_enumInstances");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    socketcomm_serialize_props(socket, (__sft), (char **)properties);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    r = (__sft)->deserialize_CMPIArray(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return(r) ? native_new_CMPIEnumeration(r, NULL) : NULL;
}



static CMPIEnumeration *TCPCOMM_associators(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    const char *assocClass,
    const char *resultClass,
    const char *role,
    const char *resultRole,
    CONST char **properties,
    CMPIStatus * rc)
{
    CMPIArray *r;
    int socket = __broker_connect(broker, context, "TCPCOMM_associators");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket, assocClass);
    (__sft)->serialize_string(socket, resultClass);
    (__sft)->serialize_string(socket, role);
    (__sft)->serialize_string(socket, resultRole);
    socketcomm_serialize_props(socket, (__sft), (char **)properties);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    r = (__sft)->deserialize_CMPIArray(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return(r) ? native_new_CMPIEnumeration(r, NULL) : NULL;
}



static CMPIEnumeration *TCPCOMM_associatorNames(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    const char *assocClass,
    const char *resultClass,
    const char *role,
    const char *resultRole,
    CMPIStatus * rc)
{
    CMPIArray *r;
    int socket = __broker_connect(broker, context, "TCPCOMM_associatorNames");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket, assocClass);
    (__sft)->serialize_string(socket, resultClass);
    (__sft)->serialize_string(socket, role);
    (__sft)->serialize_string(socket, resultRole);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    r = (__sft)->deserialize_CMPIArray(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return(r) ? native_new_CMPIEnumeration(r, NULL) : NULL;
}



static CMPIEnumeration *TCPCOMM_references(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    const char *resultClass,
    const char *role,
    CONST char **properties,
    CMPIStatus * rc)
{
    CMPIArray *r;
    int socket = __broker_connect(broker, context, "TCPCOMM_references");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket, resultClass);
    (__sft)->serialize_string(socket, role);
    socketcomm_serialize_props(socket, (__sft), (char **)properties);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    r = (__sft)->deserialize_CMPIArray(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return(r) ? native_new_CMPIEnumeration(r, NULL) : NULL;
}



static CMPIEnumeration *TCPCOMM_referenceNames(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    const char *resultClass,
    const char *role,
    CMPIStatus * rc)
{
    CMPIArray *r;
    int socket = __broker_connect(broker, context, "TCPCOMM_referenceNames");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(NULL);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket, resultClass);
    (__sft)->serialize_string(socket, role);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    r = (__sft)->deserialize_CMPIArray(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return(r) ? native_new_CMPIEnumeration(r, NULL) : NULL;
}



static CMPIData TCPCOMM_invokeMethod(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    const char *method,
    CONST CMPIArgs * in,
    CMPIArgs * out,
    CMPIStatus * rc)
{
    CMPIArgs *tmp;
    CMPIData result = { CMPI_null, CMPI_badValue, { 0}};
    int socket = __broker_connect(broker, context, "TCPCOMM_invokeMethod");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(result);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket, method);
    (__sft)->serialize_CMPIArgs(socket, in);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    tmp = (__sft)->deserialize_CMPIArgs(socket, broker);
    socketcomm_copy_args(tmp, out);
    result = (__sft)->deserialize_CMPIData(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return result;
}



static CMPIStatus TCPCOMM_setProperty(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    const char *name,
    CONST CMPIValue * value,
    CONST CMPIType type)
{
    CMPIStatus __rc, *rc = &__rc;
    int socket = __broker_connect(broker, context, "TCPCOMM_setProperty");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket, name);
    (__sft)->serialize_CMPIType(socket, type);
    (__sft)->serialize_CMPIValue(socket, type, value);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return __rc;
}



static CMPIData TCPCOMM_getProperty(
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CONST CMPIObjectPath * cop,
    const char *name,
    CMPIStatus * rc)
{
    CMPIData result = { CMPI_null, CMPI_badValue, { 0}};
    int socket = __broker_connect(broker, context, "TCPCOMM_getProperty");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(result);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket, name);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    result = (__sft)->deserialize_CMPIData(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return result;
}


#ifndef CMPI_VER_100
CMPIArray *TCPCOMM_getKeyNames(
    CMPIBroker * broker,
    CMPIContext * context,
    CMPIObjectPath * cop,
    CMPIStatus * rc)
{
    CMPIArray *result;
    int socket = __broker_connect(broker, context, "TCPCOMM_getKeyNames");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(result);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    result = (__sft)->deserialize_CMPIArray(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return result;
}
#endif

CMPIBoolean TCPCOMM_classPathIsA(
    CONST  CMPIBroker * broker,
    CONST CMPIObjectPath * cop,
    const char * type,
    CMPIStatus * rc )
{
    CMPIBoolean b;
    CMPIContext *context;
    CMPIUint8 result;
    int socket;

    TRACE_NORMAL(("Executing remote MB call."));

    tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_classPathIsA");

    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(0);
    };
    (__sft)->serialize_CMPIObjectPath(socket, cop);
    (__sft)->serialize_string(socket,type);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };
    result = (__sft)->deserialize_UINT8(socket);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    b=(result!=0);
    return b;
}
#ifdef CMPI_VER_100

CMPIStatus TCPCOMM_logMessage(
    const CMPIBroker* broker,
    CMPISeverity severity ,
    const char *id,
    const char *text,
    const CMPIString *string)
{
    CMPIStatus __rc, *rc = &__rc;
    CMPIContext *context;
    int socket;

    tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_logMessage");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    };
    (__sft)->serialize_UINT32(socket,severity);
    (__sft)->serialize_string(socket, id);
    (__sft)->serialize_string(socket, text);
    (__sft)->serialize_CMPIString(socket, string);

    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    }
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return __rc;
}

CMPIStatus  TCPCOMM_trace(
    const CMPIBroker* broker,
    CMPILevel level,
    const char *component,
    const char *text,
    const CMPIString *string)
{
    CMPIStatus __rc, *rc = &__rc;
    CMPIContext *context;
    int socket;

    tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_trace");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    };
    (__sft)->serialize_UINT32(socket,level);
    (__sft)->serialize_string(socket, component);
    (__sft)->serialize_string(socket, text);
    (__sft)->serialize_CMPIString(socket, string);

    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    }
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return __rc;

}

#endif

CMPIString* TCPCOMM_getMessage(
    CONST CMPIBroker* broker,
    const char *msgId,
    const char *defMsg,
    CMPIStatus* rc,
    unsigned int count,
    ...)
{
    CMPIString *result=NULL;
    CMPIContext *context;
    int socket;
    unsigned int i;
    CMPIType type;
    va_list argptr;

    tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_getMessage");

    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(result);
    };

    (__sft)->serialize_string(socket,msgId);
    (__sft)->serialize_string(socket,defMsg);
    (__sft)->serialize_UINT32(socket,count);

    va_start(argptr,count);
    for (i=0; i<count; i++)
    {
        type=va_arg(argptr,int);
        (__sft)->serialize_CMPIType(socket,type);

        switch (type)
        {
            case CMPI_sint8:
            case CMPI_sint16:
            case CMPI_sint32:
                {
                    long int s=va_arg(argptr,unsigned long int);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_sint32,
                        (CMPIValue*)&s);
                }
                break;
            case CMPI_uint8:
            case CMPI_uint16:
            case CMPI_uint32:
                {
                    unsigned long int u=va_arg(argptr,unsigned long int);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_uint32,
                        (CMPIValue*)&u);
                }
                break;
            case CMPI_boolean:
                {
                    CMPIBoolean b=(CMPIBoolean)va_arg(argptr,int);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_boolean,
                        (CMPIValue*)&b);
                }
                break;
            case CMPI_real32:
            case CMPI_real64:
                {
                    double d=va_arg(argptr,double);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_real64,
                        (CMPIValue*)&d);
                }
                break;
            case CMPI_sint64:
                {
                    lloonngg l=va_arg(argptr,lloonngg);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_sint64,
                        (CMPIValue*)&l);
                }
                break;
            case CMPI_uint64:
                {
                    unsigned lloonngg l=va_arg(argptr,unsigned lloonngg);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_uint64,
                        (CMPIValue*)&l);
                }
                break;
            case CMPI_chars:
                {
                    char *s=va_arg(argptr,char*);
                    (__sft)->serialize_string(socket,s);
                }
                break;
            case CMPI_string:
                {
                    CMPIString *s=va_arg(argptr,CMPIString*);
                    (__sft)->serialize_string(socket,(char*)s->hdl);
                }
                break;
            default:
                ;
        }
    }
    va_end(argptr);

    CMSetStatus(rc,CMPI_RC_OK);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };

    result = (__sft)->deserialize_CMPIString(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return result;
}

#ifdef CMPI_VER_200

CMPIError* TCPCOMM_newCMPIError(
    const CMPIBroker* broker,
    const char* owner,
    const char* msgID,
    const char* msg,
    const CMPIErrorSeverity sev,
    const CMPIErrorProbableCause pc,
    const CMPIrc cimStatusCode,
    CMPIStatus* rc)
{
    CMPIError *resError=NULL;
    CMPIContext *context;
    int socket;

    tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_newCMPIError");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(resError);
    };
    (__sft)->serialize_string(socket,owner);
    (__sft)->serialize_string(socket, msgID);
    (__sft)->serialize_string(socket, msg);
    (__sft)->serialize_CMPIErrorSeverity(socket, sev);
    (__sft)->serialize_CMPIErrorProbableCause(socket, pc);
    (__sft)->serialize_CMPIrc(socket, cimStatusCode);

    CMSetStatus(rc,CMPI_RC_OK);

    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };

    resError = (__sft)->deserialize_CMPIError(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return resError;
}

CMPIStatus TCPCOMM_openMessageFile(
    const CMPIBroker *broker,
    const char* msgFile,
    CMPIMsgFileHandle* msgFileHandle)
{
    CMPIStatus __rc, *rc = &__rc;
    CMPIContext *context;
    int socket;

    tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_openMessageFile");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    };
    (__sft)->serialize_string(socket, msgFile);

    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    }
    *msgFileHandle = (__sft)->deserialize_CMPIMsgFileHandle(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return __rc;
}

CMPIStatus TCPCOMM_closeMessageFile(
    const CMPIBroker *broker,
    const CMPIMsgFileHandle msgFileHandle)
{
    CMPIStatus __rc, *rc = &__rc;
    CMPIContext *context;
    int socket;

    tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_closeMessageFile");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(__rc);
    };
    (__sft)->serialize_CMPIMsgFileHandle(socket, msgFileHandle);

    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    }
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return __rc;
}

CMPIString* TCPCOMM_getMessage2(
    const CMPIBroker *broker,
    const char *msgId,
    const CMPIMsgFileHandle msgFileHandle,
    const char *defMsg,
    CMPIStatus* rc,
    CMPICount count,
    ...)
{
    CMPIString *result=NULL;
    CMPIContext *context;
    int socket;
    unsigned int i;
    CMPIType type;
    va_list argptr;

    tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_getMessage2");

    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return(result);
    };

    (__sft)->serialize_string(socket,msgId);
    (__sft)->serialize_CMPIMsgFileHandle(socket, msgFileHandle);
    (__sft)->serialize_string(socket,defMsg);
    (__sft)->serialize_UINT32(socket,count);


    va_start(argptr,count);
    for (i=0; i<count; i++)
    {
        type=va_arg(argptr,int);
        (__sft)->serialize_CMPIType(socket,type);

        switch (type)
        {
            case CMPI_sint8:
            case CMPI_sint16:
            case CMPI_sint32:
                {
                    long int s=va_arg(argptr,unsigned long int);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_sint32,
                        (CMPIValue*)&s);
                }
                break;
            case CMPI_uint8:
            case CMPI_uint16:
            case CMPI_uint32:
                {
                    unsigned long int u=va_arg(argptr,unsigned long int);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_uint32,
                        (CMPIValue*)&u);
                }
                break;
            case CMPI_boolean:
                {
                    CMPIBoolean b=(CMPIBoolean)va_arg(argptr,int);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_boolean,
                        (CMPIValue*)&b);
                }
                break;
            case CMPI_real32:
            case CMPI_real64:
                {
                    double d=va_arg(argptr,double);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_real64,
                        (CMPIValue*)&d);
                }
                break;
            case CMPI_sint64:
                {
                    lloonngg l=va_arg(argptr,lloonngg);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_sint64,
                        (CMPIValue*)&l);
                }
                break;
            case CMPI_uint64:
                {
                    unsigned lloonngg l=va_arg(argptr,unsigned lloonngg);
                    (__sft)->serialize_CMPIValue(
                        socket,
                        CMPI_uint64,
                        (CMPIValue*)&l);
                }
                break;
            case CMPI_chars:
                {
                    char *s=va_arg(argptr,char*);
                    (__sft)->serialize_string(socket,s);
                }
                break;
            case CMPI_string:
                {
                    CMPIString *s=va_arg(argptr,CMPIString*);
                    (__sft)->serialize_string(socket,(char*)s->hdl);
                }
                break;
            default:
                ;
        }
    }
    va_end(argptr);

    CMSetStatus(rc,CMPI_RC_OK);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    };

    result = (__sft)->deserialize_CMPIString(socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    return result;
}

#endif /* CMPI_VER_200 */

CMPIBoolean TCPCOMM_selExp_evaluate (
    CONST CMPISelectExp* se,
    CONST CMPIInstance *inst,
    CMPIStatus *rc)
{
    CMPIContext *context;
    CMPIBroker *broker;
    CMPIBoolean res;
    int socket;

    broker = tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_selExp_evaluate");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return 0;
    };
    (__sft)->serialize_CMPISelectExp (
        socket,
        se,
        PEGASUS_INDICATION_GLOBAL_CONTEXT);
    (__sft)->serialize_CMPIInstance (socket, inst);
    res = (__sft)->deserialize_UINT8(socket);
    if (rc)
    {
        rc->rc = res ? CMPI_RC_OK : CMPI_RC_ERR_FAILED;
        rc->msg = NULL;
    }
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);

    return res;
}



CMPISelectCond * TCPCOMM_selExp_getDOC (
    CONST CMPISelectExp * exp,
    CMPIStatus * rc )
{
    if (rc)
    {
        rc->rc = CMPI_RC_ERR_NOT_SUPPORTED;
        rc->msg = NULL;
    }

    return NULL;
}


CMPISelectCond * TCPCOMM_selExp_getCOD (
    CONST CMPISelectExp * exp,
    CMPIStatus * rc )
{
    if (rc)
    {
        rc->rc = CMPI_RC_ERR_NOT_SUPPORTED;
        rc->msg = NULL;
    }

    return NULL;
}

CMPIBoolean TCPCOMM_selExp_evaluateUsingAccessor (
    CONST CMPISelectExp* se,
    CMPIAccessor *accessor,
    void *parm, CMPIStatus* rc)
{
    if (rc)
    {
        rc->rc = CMPI_RC_ERR_NOT_SUPPORTED;
        rc->msg = NULL;
    }

    return 0;
}

CMPISelectExp* TCPCOMM_selExp_clone (CONST CMPISelectExp* se, CMPIStatus* rc)
{
    CMPIContext *context;
    CMPIBroker *broker;
    CMPISelectExp *res;
    int socket;

    broker = tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_selExp_clone");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return NULL;
    }
    (__sft)->serialize_CMPISelectExp (socket, se,
        PEGASUS_INDICATION_GLOBAL_CONTEXT);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus (socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    }
    res = (__sft)->deserialize_CMPISelectExp (socket, broker,
        PEGASUS_INDICATION_GLOBAL_CONTEXT);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);

    return res;
}

CMPIString* TCPCOMM_selExp_getString (CONST CMPISelectExp* se, CMPIStatus* rc)
{
    CMPIContext *context;
    CMPIBroker *broker;
    CMPIString *str;
    int socket;

    broker = tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_selExp_getString");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return NULL;
    }
    (__sft)->serialize_CMPISelectExp (
        socket,
        se,
        PEGASUS_INDICATION_GLOBAL_CONTEXT);
    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus (socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    }
    str = (__sft)->deserialize_CMPIString (socket, broker);
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);

    return str;
}

CMPIStatus TCPCOMM_selExp_release (CONST CMPISelectExp* se)
{
    CMPIStatus __rc ;
    CMPIContext *context;
    CMPIBroker *broker;
    int socket;

    broker = tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_selExp_release");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        __rc.rc = CMPI_RC_ERR_FAILED;
        __rc.msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        TRACE_CRITICAL(("up-call connect to broker failed."));
    }
    else
    {
        (__sft)->serialize_CMPISelectExp (
            socket,
            se,
            PEGASUS_INDICATION_GLOBAL_CONTEXT);
        __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if (__rc.rc == CMPI_RC_OK)
        {
            native_release_CMPISelectExp (se);
        }
        //invokes close(socket) on unix & closesocket(socket) on windows
        PEGASUS_CMPIR_CLOSESOCKET(socket);
    }

    return __rc;
}

CMPISelectExp *TCPCOMM_selExp_newSelExp (
    const char * queryString,
    const char * language,
    CMPIArray ** projection,
    CMPIStatus * rc )
{
    CMPIContext *context;
    CMPIBroker *broker;
    CMPISelectExp *res;
    CMPIArray *projArr;
    int socket;

    broker = tool_mm_get_broker((void**)&context);
    socket = __broker_connect(broker, context, "TCPCOMM_selExp_newSelExp");
    TRACE_NORMAL(("Executing remote MB call."));
    if (socket < 0)
    {
        if ((rc))
        {
            (rc)->rc = CMPI_RC_ERR_FAILED;
            (rc)->msg = CMNewString((broker), "cannot connect to CIMOM", NULL);
        }
        TRACE_CRITICAL(("up-call connect to broker failed."));
        return NULL;
    }
    (__sft)->serialize_string (socket, queryString);
    (__sft)->serialize_string (socket, language);

    {
        CMPIStatus __rc = (__sft)->deserialize_CMPIStatus(socket, broker);
        if ((rc))
        {
            *(rc) = __rc;
        }
    }
    // This CMPISelectExp is added to current thread memory management
    // -V 5245
    res = (__sft)->deserialize_CMPISelectExp (socket, broker,
        PEGASUS_INDICATION_LOCAL_CONTEXT);
    projArr = (__sft)->deserialize_CMPIArray (socket, broker);
    if (projection && *projection)
    {
        *projection = projArr;
    }
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);

    return res;
}

static NativeCMPIBrokerFT(tcpcomm_brokerFT) =
{
    {0,
    0,
    "TCPCOMM_REMOTE_BROKER",
    TCPCOMM_prepareAttachThread,
    TCPCOMM_attachThread,
    TCPCOMM_detachThread,
    TCPCOMM_deliverIndication,
    TCPCOMM_enumInstanceNames,
    TCPCOMM_getInstance,
    TCPCOMM_createInstance,
    TCPCOMM_setInstance,
    TCPCOMM_deleteInstance,
    TCPCOMM_execQuery,
    TCPCOMM_enumInstances,
    TCPCOMM_associators,
    TCPCOMM_associatorNames,
    TCPCOMM_references,
    TCPCOMM_referenceNames,
    TCPCOMM_invokeMethod,
    TCPCOMM_setProperty,
    TCPCOMM_getProperty
    },
#ifndef CMPI_VER_100
    TCPCOMM_getKeyNames,
#endif
    TCPCOMM_getMessage,
#ifdef CMPI_VER_100
    TCPCOMM_logMessage,
    TCPCOMM_trace,
#endif
    TCPCOMM_classPathIsA,
    // These are added for Remote CMPI support for Indications
    // We are not implemented the below functionalities on remote
    // side instead we make UP calls to MB. -V 5245
    TCPCOMM_selExp_evaluate,
    TCPCOMM_selExp_getDOC,
    TCPCOMM_selExp_getCOD,
    TCPCOMM_selExp_evaluateUsingAccessor,
    TCPCOMM_selExp_clone,
    TCPCOMM_selExp_getString,
    TCPCOMM_selExp_release,
    TCPCOMM_selExp_newSelExp
#ifdef CMPI_VER_200
    ,
    TCPCOMM_newCMPIError,
    TCPCOMM_openMessageFile,
    TCPCOMM_closeMessageFile,
    TCPCOMM_getMessage2
#endif
};



/****************************************************************************/


//! Establishes a broker connection.
/*!
    Opens a new connection to the broker address stored in the remote broker
    handle that is hidden behind the regular CMPIBroker. Once the
    connection has been established the proxy ticket, the RCMPI_CTX_ID, and
    the context itself are sent.

    \param broker the remote broker.
    \param ctx the context to be sent.
    \param function the MB call function name.

    \return the connection socket.
*/
static int __broker_connect(
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx,
    const char *function)
{
    remote_broker *rb = CMPIBroker2remote_broker(broker);
    char *broker_address = RBGetBrokerAddress(rb);
    comm_ticket *ticket = RBGetTicket(rb);

    CMPIData d;

    int socket = open_connection(
        broker_address,
        CIMOM_LISTEN_PORT,
        PEGASUS_PRINT_ERROR_MESSAGE);

    if (socket < 0)
    {
        return -1;
    }

    io_write_fixed_length(socket, ticket, sizeof(comm_ticket));

    d = CMGetContextEntry(ctx, RCMPI_CTX_ID, NULL);
    __sft->serialize_UINT32(socket, d.value.uint32);
    __sft->serialize_string(socket, function);
    socketcomm_serialize_context(socket, __sft, ctx);

    return socket;
}

static void __handle_daemon_stop_request()
{
    // TODO: need to close all active connections ???
    // Don't send any response, remote-end already closed the connection.
    exit(0);
}

static void __handle_daemon_is_running_request(int socket)
{
    //As of now do nothing, Don't send any response,
    //remote end already closed the connection...
    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);

    return;
}
//! Sets up the minimal environment for handling an MI request.
/*!
    The function reads initial data such as function name, ticket, provider
    and provider module, resolves the broker address and looks up the
    handler function.

    \param socket the connection socket.

    \sa __mi_functions
*/
static void __handle_MI_call(int socket)
{

    static CMPIBroker __broker =
    {
        NULL,
        NULL,
        NULL, //&native_brokerEncFT,
        NULL, // CMPI_BrokerExt_Ftab
        NULL  // CMPI_BrokerMem_Ftab
    };
    char *provider, *provider_module, *function, broker_address[256];
    comm_ticket ticket;
    remote_broker *rb;
    unsigned int i;
    __broker.eft=&native_brokerEncFT;
    TRACE_VERBOSE(("entered function."));

    get_peer_address(socket, broker_address);

    TRACE_NORMAL(("Handling MI request from: %s", broker_address));
    provider = __sft->deserialize_string(socket, &__broker);
    if (!strcmp (provider, PEGASUS_CMPIR_DAEMON_STOP))
    {
        __handle_daemon_stop_request ();
        return; // we should never reach here...
    }
    if (!strcmp (provider, PEGASUS_CMPIR_DAEMON_IS_RUNNING))
    {
        __handle_daemon_is_running_request (socket);
        return;
    }
    provider_module = __sft->deserialize_string(socket, &__broker);
    io_read_fixed_length(socket, &ticket, sizeof(comm_ticket));
    function = __sft->deserialize_string(socket, &__broker);

    rb = find_remote_broker(
        "TCPComm",
        broker_address,
        provider,
        provider_module,
        &ticket,
        &tcpcomm_brokerFT.brokerFt);

    for (i = 0;
        i < sizeof(__mi_functions) / sizeof(struct socket_mi_function);
        i++)
    {

        if (strcmp(function, __mi_functions[i].name) == 0)
        {

            CMPIBroker *b = remote_broker2CMPIBroker(rb);
            CMPIContext *ctx = native_new_CMPIContext(TOOL_MM_ADD);
            CMPIObjectPath *cop;

            socketcomm_deserialize_context(socket, __sft, b, ctx);
            cop = __sft->deserialize_CMPIObjectPath(socket, b);

            __mi_functions[i].function(socket, b, ctx, cop);

            RBReleaseMI(rb);
            //invokes close(socket) on unix & closesocket(socket) on windows
            PEGASUS_CMPIR_CLOSESOCKET(socket);

            TRACE_VERBOSE(("leaving function."));
            return;
        }
    }

    //invokes close(socket) on unix & closesocket(socket) on windows
    PEGASUS_CMPIR_CLOSESOCKET(socket);
    TRACE_CRITICAL(("Invalid MI call requested: %s", function));
}


//! Waits on incoming MI requests.
/*!
    This function is to be run in a separate thread. There it waits for incoming
    MI calls. These are passed on to __handle_MI_call().

    \sa accept_connections()
*/

static void *  PEGASUS_CMPIR_STDCALL __start_remote_daemon(void* parm)
{
    accept_connections(REMOTE_LISTEN_PORT, __handle_MI_call, 1);
    return NULL;
}



//! Launches __start_remote_daemon() in a separate thread.
static void __launch_remote_daemon()
{
    CMPI_BrokerExt_Ftab->newThread(__start_remote_daemon, NULL,1);

    printf("TCPComm remote daemon launched on port %d ...\n",
        REMOTE_LISTEN_PORT);
}


//! Starts the MI call daemon thread.
/*!
    \sa __handle_MI_call()
*/
PEGASUS_EXPORT int PEGASUS_CMPIR_CDECL start_remote_daemon()
{
    static int __once = 0;

#ifdef PEGASUS_OS_TYPE_WINDOWS
    pegthreadOnceMutex=CreateMutex(NULL,FALSE,NULL);
    if (pegthreadOnceMutex == NULL)
    {
        TRACE_CRITICAL(("CreateMutex failed: Error code: %d",
            GetLastError()));
    }
#endif

    CMPI_BrokerExt_Ftab->threadOnce(&__once, (void*)__launch_remote_daemon);

    return 0;
}


/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
