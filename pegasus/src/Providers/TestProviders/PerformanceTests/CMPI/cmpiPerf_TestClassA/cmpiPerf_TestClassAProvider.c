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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmpiPerf_TestClassA.h"

#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
#define strcasecmp stricmp
#endif

static const CMPIBroker * _broker;

/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus cmpiPerf_TestClassAProviderCleanup(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean terminating)
{
    CMReturn(CMPI_RC_OK);
}

CMPIStatus cmpiPerf_TestClassAProviderEnumInstanceNames(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref)
{
    CMPIObjectPath  * op = NULL;
    CMPIStatus        rc = {CMPI_RC_OK, NULL};

    op = _makePath_TestClassA( _broker, ctx, ref);

    CMReturnObjectPath( rslt, op );

    CMReturnDone( rslt );
    return rc;
}

CMPIStatus cmpiPerf_TestClassAProviderEnumInstances(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char ** properties)
{
    CMPIInstance * ci = NULL;
    CMPIStatus     rc = {CMPI_RC_OK, NULL};

    ci = _makeInst_TestClassA( _broker, ctx, ref, properties);

    CMReturnInstance( rslt, ci );

    CMReturnDone( rslt );
    return rc;
}

CMPIStatus cmpiPerf_TestClassAProviderGetInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const char ** properties)
{
    CMPIInstance * ci = NULL;
    CMPIStatus     rc = {CMPI_RC_OK, NULL};


    ci = _makeInst_TestClassA( _broker, ctx, cop, properties);

    CMReturnInstance( rslt, ci );
    CMReturnDone(rslt);

    return rc;
}

CMPIStatus cmpiPerf_TestClassAProviderCreateInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci)
{
    CMPIObjectPath  * op = NULL;
    CMPIStatus rc = {CMPI_RC_OK, NULL};

    op = _makePath_TestClassA( _broker, ctx, cop);

    CMReturnObjectPath( rslt, op );
    CMReturnDone( rslt );

    return rc;
}

CMPIStatus cmpiPerf_TestClassAProviderModifyInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci,
    const char **properties)
{
    CMReturnWithChars(
        _broker,
        CMPI_RC_ERR_NOT_SUPPORTED,
        "CIM_ERR_NOT_SUPPORTED");
}

CMPIStatus cmpiPerf_TestClassAProviderDeleteInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop)
{
    CMReturnWithChars(
        _broker,
        CMPI_RC_ERR_NOT_SUPPORTED,
        "CIM_ERR_NOT_SUPPORTED");
}

CMPIStatus cmpiPerf_TestClassAProviderExecQuery(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char * lang,
    const char * query)
{
    CMReturnWithChars(
        _broker,
        CMPI_RC_ERR_NOT_SUPPORTED,
        "CIM_ERR_NOT_SUPPORTED");
}


/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus cmpiPerf_TestClassAProviderMethodCleanup(
    CMPIMethodMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean terminating)
{
    CMReturn(CMPI_RC_OK);
}

CMPIStatus cmpiPerf_TestClassAProviderInvokeMethod(
    CMPIMethodMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char * methodName,
    const CMPIArgs * in,
    CMPIArgs * out)
{
    CMPIString * className = NULL;
    CMPIStatus   rc    = {CMPI_RC_OK, NULL};

    className = CMGetClassName(ref, &rc);

    if (strcasecmp(CMGetCharsPtr(className, NULL), _ClassName) == 0 &&
        strcasecmp("SetPowerState",methodName) == 0)
    {
        rc.rc = CMPI_RC_ERR_NOT_SUPPORTED;
        rc.msg=_broker->eft->newString(_broker,methodName,0);
    }
    else
    {
        rc.rc = CMPI_RC_ERR_NOT_FOUND;
        rc.msg=_broker->eft->newString(_broker,methodName,0);
    }
    return rc;
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub(cmpiPerf_TestClassAProvider,
                 cmpiPerf_TestClassAProvider,
                 _broker,
                 CMNoHook)

CMMethodMIStub(cmpiPerf_TestClassAProvider,
               cmpiPerf_TestClassAProvider,
               _broker,
               CMNoHook)

/* ---------------------------------------------------------------------------*/
/*                end of cmpiPerf_TestClassAProvider                         */
/* ---------------------------------------------------------------------------*/

