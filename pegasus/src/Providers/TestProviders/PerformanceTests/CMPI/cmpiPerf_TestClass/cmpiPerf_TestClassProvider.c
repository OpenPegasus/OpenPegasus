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

#include "cmpiPerf_TestClass.h"
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
#include <windows.h>
#define setenv(A,B,C) SetEnvironmentVariable(A,B)
#endif

static const CMPIBroker * _broker;

/* ----------------------------------------------------------*/
/* private declarations                                      */
/* ----------------------------------------------------------*/

unsigned int getNumberOfInstances()
{
    char *           env = NULL;
    unsigned int numInst = 1;

    //
    // The number of instances is determined from an environment variable
    //
    env = getenv("PERF_TEST");
    if (env != NULL)
    {
        numInst = atoi(env);
    }
    return numInst;
}

void setNumberOfInstances(unsigned int numInst)
{
    char * env = NULL;
    env = calloc(1,30);
    sprintf(env, "PERF_TEST=%u", numInst);

    putenv(env);
    free(env);
    return;
}

/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus cmpiPerf_TestClassProviderCleanup(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean terminating)
{
    CMReturn(CMPI_RC_OK);
}

CMPIStatus cmpiPerf_TestClassProviderEnumInstanceNames(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref)
{
    CMPIObjectPath  * op = NULL;
    CMPIStatus        rc = {CMPI_RC_OK, NULL};
    unsigned int   index = 0;
    unsigned int numInst = 1;

    //
    // While the instance itself is hard code, the number of instances
    // is determined from an environment variable
    //
    numInst = getNumberOfInstances();

    for (index=0; index < numInst; index++)
    {
        op = _makePath_TestClass( _broker, ctx, ref, index );

        if (op == NULL)
        {
            return rc;
        }

        CMReturnObjectPath( rslt, op );
        CMRelease( op );
    }

    CMReturnDone( rslt );
    return rc;
}

CMPIStatus cmpiPerf_TestClassProviderEnumInstances(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char ** properties)
{
    CMPIInstance * ci = NULL;
    CMPIStatus     rc = {CMPI_RC_OK, NULL};
    unsigned int numInst = 1;
    unsigned int   index = 0;


    //
    // While the instance itself is hard code, the number of instances
    // is determined from an environment variable
    //
    numInst = getNumberOfInstances();

    for (index=0; index < numInst; index++)
    {
        ci = _makeInst_TestClass( _broker, ctx, ref, properties, index );

        if (ci == NULL)
        {
            return rc;
        }

        CMReturnInstance( rslt, ci );
        CMRelease( ci );
    }
    CMReturnDone( rslt );
    return rc;
}

CMPIStatus cmpiPerf_TestClassProviderGetInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const char ** properties)
{
    CMPIInstance * ci = NULL;
    CMPIStatus     rc = {CMPI_RC_OK, NULL};


    ci = _makeInst_TestClass(_broker,ctx,cop,properties,1);

    if (ci == NULL)
    {
        return rc;
    }

    CMReturnInstance( rslt, ci );
    CMReturnDone(rslt);

    return rc;
}

CMPIStatus cmpiPerf_TestClassProviderCreateInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci)
{
    CMPIObjectPath  * op = NULL;
    CMPIStatus rc = {CMPI_RC_OK, NULL};
    unsigned int numInst = 1;


    numInst = getNumberOfInstances()+1;

    op = _makePath_TestClass( _broker, ctx, cop, numInst );

    CMReturnObjectPath( rslt, op );
    CMReturnDone( rslt );

    setNumberOfInstances(numInst);


    return rc;
}

CMPIStatus cmpiPerf_TestClassProviderModifyInstance(
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

CMPIStatus cmpiPerf_TestClassProviderDeleteInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop)
{
    CMPIStatus rc = {CMPI_RC_OK, NULL};
    unsigned int numInst = 1;


    numInst = getNumberOfInstances();

    if (numInst > 0)
    {
        numInst--;
    }

    setNumberOfInstances(numInst);

    return rc;
}

// Support for extrinsic ExecQuery
CMPIStatus cmpiPerf_TestClassProviderExecQuery(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char * lang,
    const char * query)
{
    CMPIStatus rc = {CMPI_RC_OK, NULL};
    CMPISelectExp *   sel = NULL;
    CMPIInstance  *   ci = NULL;
    CMPIBoolean       evalRslt;
    unsigned int      index = 0,
    numInst = getNumberOfInstances();

    // The CMNewSelectExp will return CMPI_RC_ERR_QUERY_LANGUAGE_NOT_SUPPORTED
    // if the language is not supported, therefore no specific check of the
    // lang supplied by the caller is necessary.

    // Create a CMPISelectExp (Select Expression) from the query string,
    // specifying a NULL projection (whatever that is...)
    sel = CMNewSelectExp(_broker, query, lang, NULL, &rc);
    if (sel == NULL)
    {
        return rc;
    }

    // Loop through all the instances,
    // evaluating them against the CMPI SelectExp.
    for (index=0; index<numInst; index++)
    {
        ci = _makeInst_TestClass( _broker, ctx, ref, NULL, index );

        // Check that the CMPIInstance was successfully retrieved, otherwise
        // return with an error.  One might consider not failing the query just
        // because one or more instances failed to be obtained and instead use
        // the rc to indicate that the results are incomplete due to instance
        // failures.
        if (ci == NULL)
        {
            return rc;
        }

        // Evaluate the instance against the CMPI Select Expression
        evalRslt = CMEvaluateSelExp(sel, ci, &rc);
        if ((CMPIBoolean) 0 != evalRslt)
        {
            CMReturnInstance( rslt, ci );
        }
    }

    return rc;
}


/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub(cmpiPerf_TestClassProvider,
                 cmpiPerf_TestClassProvider,
                 _broker,
                 CMNoHook)

/* ---------------------------------------------------------------------------*/
/*                end of cmpiPerf_TestClassProvider                         */
/* ---------------------------------------------------------------------------*/
