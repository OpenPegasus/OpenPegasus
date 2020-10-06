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
    \file selectexp.c
    \brief Native CMPISelectExp implementation.

    This is the native CMPISelectExp implementation as used for remote
    providers. It reflects the well-defined interface of a regular
    CMPISelectExp, however, it works independently from the management broker.

    It is part of a native broker implementation that simulates CMPI data
    types rather than interacting with the entities in a full-grown CIMOM.

    \todo THIS IS NOT FULLY IMPLEMENTED YET!!!
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "native.h"


struct native_selectexp
{
    CMPISelectExp exp;
    int mem_state;
    CMPIUint64 id;
};


static struct native_selectexp * __new_exp (
    int mem_state,
    CMPIUint64 id,
    CMPIStatus *rc);



/*****************************************************************************/

static CMPIStatus __eft_release ( CMPISelectExp * exp )
{
    struct native_selectexp * e = (struct native_selectexp *) exp;
    CMPIContext *ctx;
    CMPIBroker *broker;
    CMPIStatus rc = checkArgsReturnStatus(exp);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }
    broker = tool_mm_get_broker ( (void**)&ctx);

    if (e->mem_state == TOOL_MM_NO_ADD)
    {
        rc = ( ( (NativeCMPIBrokerFT*)broker->bft) )->selExp_release (exp);
        if (rc.rc == CMPI_RC_OK)
        {
            tool_mm_add ( e );
        }
    }
    CMReturn ( CMPI_RC_OK );
}


static CMPISelectExp * __eft_clone (CONST CMPISelectExp * exp, CMPIStatus * rc)
{
    CMPIContext *ctx;
    CMPIBroker *broker;

    if (!checkArgs(exp, rc))
    {
        return 0;
    }
    broker = tool_mm_get_broker ( (void**)&ctx);
    return( ( (NativeCMPIBrokerFT*)broker->bft) )->selExp_clone (exp, rc);
}



CMPIBoolean __eft_evaluate (
    CONST CMPISelectExp * exp,
    CONST CMPIInstance * inst,
    CMPIStatus * rc )
{
    CMPIContext *ctx;
    CMPIBroker *broker;

    if (!checkArgs(exp, rc) || !checkArgs(inst, rc))
    {
        return 0;
    }
    broker = tool_mm_get_broker ( (void**)&ctx);

    return(((NativeCMPIBrokerFT*)broker->bft))->selExp_evaluate (exp, inst ,rc);
}


CMPIString * __eft_getString ( CONST CMPISelectExp * exp, CMPIStatus * rc )
{
    CMPIContext *ctx;
    CMPIBroker *broker;

    if (!checkArgs(exp, rc))
    {
        return 0;
    }
    broker = tool_mm_get_broker ( (void**)&ctx);
    return( ( (NativeCMPIBrokerFT*)broker->bft) )->selExp_getString (exp, rc);
}


CMPISelectCond * __eft_getDOC ( CONST CMPISelectExp * exp, CMPIStatus * rc )
{
    CMPIContext *ctx;
    CMPIBroker *broker;

    if (!checkArgs(exp, rc))
    {
        return 0;
    }
    broker = tool_mm_get_broker ( (void**)&ctx);
    return( ( (NativeCMPIBrokerFT*)broker->bft) )->selExp_getDOC (exp, rc);
}


CMPISelectCond * __eft_getCOD ( CONST CMPISelectExp * exp, CMPIStatus * rc )
{
    CMPIContext *ctx;
    CMPIBroker *broker;

    if (!checkArgs(exp, rc))
    {
        return 0;
    }
    broker = tool_mm_get_broker ( (void**)&ctx);
    return( ( (NativeCMPIBrokerFT*)broker->bft) )->selExp_getCOD (exp, rc);
}

CMPIBoolean __eft_evaluateUsingAccessor (
    CONST CMPISelectExp* se,
    CMPIAccessor *accessor,
    void *parm, CMPIStatus* rc)
{
    CMPIContext *ctx;
    CMPIBroker *broker;

    if (!checkArgs(se, rc) || !checkArgs(accessor, rc))
    {
        return 0;
    }
    broker = tool_mm_get_broker ( (void**)&ctx);
    return( ( (NativeCMPIBrokerFT*)broker->bft) )->
        selExp_evaluateUsingAccessor (se, accessor ,parm, rc);
}

static struct native_selectexp * __new_exp (
    int mm_add,
    CMPIUint64 id,
    CMPIStatus * rc )
{
    static CMPISelectExpFT eft = {
        NATIVE_FT_VERSION,
        __eft_release,
        __eft_clone,
        __eft_evaluate,
        __eft_getString,
        __eft_getDOC,
        __eft_getCOD,
        __eft_evaluateUsingAccessor
    };
    static CMPISelectExp e = {
        "CMPISelectExp",
        &eft
    };


    struct native_selectexp * exp =
        (struct native_selectexp *)
        tool_mm_alloc ( mm_add, sizeof ( struct native_selectexp ) );

    exp->exp         = e;
    exp->mem_state   = mm_add;
    exp->id          = id;

    CMSetStatus ( rc, CMPI_RC_OK );
    return exp;
}

CMPISelectExp * native_new_CMPISelectExp(CMPIUint64 id, CMPIStatus * rc)
{
    return(CMPISelectExp *) __new_exp (
        TOOL_MM_NO_ADD,
        id,
        rc );
}

CMPISelectExp * native_new_CMPISelectExp_add(CMPIUint64 id, CMPIStatus * rc)
{
    return(CMPISelectExp *) __new_exp (
        TOOL_MM_ADD,
        id,
        rc );
}

void native_release_CMPISelectExp( CONST CMPISelectExp *filter)
{
    struct native_selectexp * exp = (struct native_selectexp *) filter;

    if (exp->mem_state == TOOL_MM_NO_ADD)
    {
        tool_mm_add(exp);
    }
}

// These functions help in serializing and deserializing the
// CMPISelectExp -V 5245

PEGASUS_EXPORT CMPIUint64 create_indicationObject (
    CMPISelectExp *se,
    CMPIUint32 ctx_id,
    CMPIUint8 type)
{
    struct native_selectexp *e = (struct native_selectexp*)se;

    return e->id;
}

PEGASUS_EXPORT CMPISelectExp *get_indicationObject (
    CMPIUint64 id,
    CMPIUint32 ctx_id)
{
    if (ctx_id)
    {
        return native_new_CMPISelectExp_add (id, NULL);
    }

    return native_new_CMPISelectExp (id, NULL);
}

/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
