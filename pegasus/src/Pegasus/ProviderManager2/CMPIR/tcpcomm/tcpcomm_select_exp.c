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

#include <stdio.h>
#include <stdlib.h>

#ifdef PEGASUS_OS_TYPE_UNIX
# include <netinet/in.h>
#elif defined PEGASUS_OS_TYPE_WINDOWS
# include <winsock2.h>
#endif

#include "tcpcomm_select_exp.h"
#include "indication_objects.h"

/*
    This file contains necessary methods for manipulation of CMPISelectExp
    on MB side.
*/

static const struct BinarySerializerFT *__sft = &binarySerializerFT;

static void TCPCOMM_selExp_evaluate (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CMPIUint32 ctx_id)
{
    CMPISelectExp *se;
    CMPIInstance *inst;
    CMPIBoolean res;
    CMPIStatus rc;

    se = (__sft)->deserialize_CMPISelectExp(socket, broker, ctx_id);
    inst = (__sft)->deserialize_CMPIInstance(socket, broker, 0);

    res = CMEvaluateSelExp(se, inst, &rc);
    (__sft)->serialize_UINT8(socket, res);
}

static void TCPCOMM_selExp_getDOC (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CMPIUint32 ctx_id)
{
    // not implemented
}

static void TCPCOMM_selExp_getCOD (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CMPIUint32 ctx_id)
{
    // not implemented
}

static void TCPCOMM_selExp_evaluateUsingAccessor (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CMPIUint32 ctx_id)
{
    // not implemented
}

static void TCPCOMM_selExp_clone (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CMPIUint32 ctx_id)
{
    CMPISelectExp *se, *new_se;
    CMPIStatus rc;

    se = (__sft)->deserialize_CMPISelectExp (socket, broker, ctx_id);
    new_se = CMClone(se, &rc);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPISelectExp (socket, new_se, ctx_id);
}

static void TCPCOMM_selExp_getString (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CMPIUint32 ctx_id)
{
    CMPISelectExp *se;
    CMPIString *str;
    CMPIStatus rc;

    se = (__sft)->deserialize_CMPISelectExp (socket, broker, ctx_id);
    str = CMGetSelExpString(se, &rc);
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPIString (socket, str);
}

static void TCPCOMM_selExp_release (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CMPIUint32 ctx_id)
{
    CMPISelectExp *se;
    CMPIStatus rc = { CMPI_RC_OK, NULL};
    ;
    se = (__sft)->deserialize_CMPISelectExp (socket, broker, ctx_id);
    // remove object from context list, this will also calls release
    // of CMPISelectExp
    if (remove_indicationObject (se, ctx_id))
    {
        rc.rc = CMPI_RC_ERR_FAILED;
    }
    (__sft)->serialize_CMPIStatus(socket, &rc);
}

static void TCPCOMM_selExp_newSelExp (
    int socket,
    CONST CMPIBroker * broker,
    CONST CMPIContext * context,
    CMPIUint32 ctx_id)
{
    CMPISelectExp *new_se, *se_clone = NULL;
    CMPIStatus rc;
    char *lang, *query;
    CMPIArray *projection = NULL;

    query = (__sft)->deserialize_string (socket, broker);
    lang = (__sft)->deserialize_string (socket, broker);
    new_se = CMNewSelectExp (broker, query, lang, &projection, &rc);
    // Clone the Object so that object will not get deleted when the
    // current thread finishes execution.
    if (new_se)
    {
        se_clone = CMClone (new_se, &rc);
    }
    (__sft)->serialize_CMPIStatus(socket, &rc);
    (__sft)->serialize_CMPISelectExp (socket, se_clone, ctx_id);
    (__sft)->serialize_CMPIArray (socket, projection);
}

struct socket_select_exp_function select_exp_functions [] =
{
    { "TCPCOMM_selExp_evaluate",        TCPCOMM_selExp_evaluate},
    { "TCPCOMM_selExp_getDOC",          TCPCOMM_selExp_getDOC},
    { "TCPCOMM_selExp_getCOD",          TCPCOMM_selExp_getCOD},
    { "TCPCOMM_selExp_evaluateUsingAccessor",
    TCPCOMM_selExp_evaluateUsingAccessor},
    { "TCPCOMM_selExp_clone",           TCPCOMM_selExp_clone},
    { "TCPCOMM_selExp_getString",       TCPCOMM_selExp_getString},
    { "TCPCOMM_selExp_release",         TCPCOMM_selExp_release},
    { "TCPCOMM_selExp_newSelExp",       TCPCOMM_selExp_newSelExp}
};
