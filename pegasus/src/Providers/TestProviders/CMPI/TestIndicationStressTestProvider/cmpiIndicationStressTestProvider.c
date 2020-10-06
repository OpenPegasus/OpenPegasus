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
#include <ctype.h>
#include <stdarg.h>
#ifdef PEGASUS_OS_ZOS
#include <pthread.h>
#endif

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#else
#if defined(CMPI_PLATFORM_AIX_RS_IBMCXX) \
    || defined(CMPI_PLATFORM_PASE_ISERIES_IBMCXX)
#include <time.h>
#endif
#include <sys/time.h>
#endif

#define _IndClassName "TestCMPI_IndicationStressTestClass"
#define _IndSubClassName "TestCMPI_IndicationStressTestSubClass"

#ifdef CMPI_VER_100
static const CMPIBroker *_broker;
#else
static CMPIBroker *_broker;
#endif

#define CMPI_true  1
#define CMPI_false  0

static CMPIObjectPath *objPath;
static CMPIBoolean _enabled = CMPI_false;
static CMPIUint32 _nextUID = 1;
static CMPIUint32 _numSubscriptions = 0;
static CMPIArray *correlatedIndications = NULL;
static char *namespaceName;
static CMPIBoolean _baseClassActivated = CMPI_false;
static CMPIBoolean _subClassActivated = CMPI_false;
static int _namespaceIndex = 0;
static CMPI_MUTEX_TYPE subCntMutex;

static char *_sourceNamespaces[] = {
    "test/TestProvider",
    "test/testIndSrcNS1",
    "test/testIndSrcNS2"
};
    

/* ------------------------------------------------------------------------ */
/*                            Helper functions.                             */
/* ------------------------------------------------------------------------ */

static void _createMutex()
{
  subCntMutex = _broker->xft->newMutex (0);
}

static void _incSubCount()
{
  _broker->xft->lockMutex(subCntMutex);
  _numSubscriptions++;
  _broker->xft->unlockMutex(subCntMutex);
}

static void _decSubCount()
{
  _broker->xft->lockMutex(subCntMutex);
  _numSubscriptions--;
  _broker->xft->unlockMutex(subCntMutex);
}

static CMPIUint32 _getSubCount()
{
    CMPIUint32 count; 
    _broker->xft->lockMutex (subCntMutex);
    count =  _numSubscriptions;
    _broker->xft->unlockMutex (subCntMutex);
    return count;
}

CMPIInstance *getIndicationInstance(CMPIUint64 seqNum, char *UIDbuffer)
{
    CMPIInstance *indicationInstance =
                  CMNewInstance (_broker, objPath, NULL);
    CMPIDateTime *currentDateTime =
                  CMNewDateTime (_broker,NULL);
    CMSetProperty (indicationInstance, "IndicationIdentifier",
                   UIDbuffer, CMPI_chars);
    CMSetProperty (indicationInstance, "CorrelatedIndications",
                   &correlatedIndications, CMPI_stringA);
    CMSetProperty (indicationInstance, "IndicationTime",
                   &currentDateTime, CMPI_dateTime);
    CMSetProperty (indicationInstance, "IndicationSequenceNumber",
                   &seqNum, CMPI_uint64);

    return indicationInstance;
}


/* ---------------------------------------------------------------------------*/
/*                       Indication Provider Interface                        */
/* ---------------------------------------------------------------------------*/

CMPIStatus TestCMPIIndicationStressTestProviderIndicationCleanup(
                                           CMPIIndicationMI * mi,
                                           const CMPIContext * ctx,
                                           CMPIBoolean term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderAuthorizeFilter(
                                           CMPIIndicationMI * mi,
                                           const CMPIContext * ctx,
                                           const CMPISelectExp * se,
                                           const char *ns,
                                           const CMPIObjectPath * op,
                                           const char *user)
{
    if (strcmp (ns, _IndClassName) )
    {
        CMReturn (CMPI_RC_ERR_INVALID_CLASS);
    }

    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderMustPoll(
                                    CMPIIndicationMI * mi,
                                    const CMPIContext * ctx,
                                    const CMPISelectExp * se,
                                    const char *ns, const CMPIObjectPath * op)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIIndicationStressTestProviderActivateFilter(
                                          CMPIIndicationMI * mi,
                                          const CMPIContext * ctx,
                                          const CMPISelectExp * se,
                                          const char *clsName,
                                          const CMPIObjectPath * op,
                                          CMPIBoolean firstActivation)
{

    CMPIString *str = CMGetClassName(op, 0);
    const char *className = CMGetCharsPtr(str, 0);

    if (!strcmp(className, _IndClassName))
    {
        _baseClassActivated = CMPI_true;
        _incSubCount();
    }
    else if(!strcmp(className, _IndSubClassName))
    {
        _subClassActivated = CMPI_true;
        _incSubCount();
    }

    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderDeActivateFilter(
                                            CMPIIndicationMI * mi,
                                            const CMPIContext * ctx,
                                            const CMPISelectExp * se,
                                            const char *clsName,
                                            const CMPIObjectPath * op,
                                            CMPIBoolean lastActivation)
{
    CMPIString *str = CMGetClassName(op, 0);
    const char *className = CMGetCharsPtr(str, 0);

    if (!strcmp(className, _IndClassName))
    {
        _baseClassActivated = CMPI_false;
        _decSubCount();
    }
    else if(!strcmp(className, _IndSubClassName))
    {
        _subClassActivated = CMPI_false;
        _decSubCount();
    }

    if (_numSubscriptions == 0)
    {
        _enabled = CMPI_false;
    }

    CMReturn (CMPI_RC_OK);
}


CMPIStatus TestCMPIIndicationStressTestProviderEnableIndications(
                                              CMPIIndicationMI * mi,
                                              const CMPIContext * ctx)
{
    if (_baseClassActivated && _subClassActivated)
    {
        _enabled = CMPI_true;
    }

    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderDisableIndications(
                                               CMPIIndicationMI * mi,
                                               const CMPIContext * ctx)
{
    if (!_baseClassActivated && !_subClassActivated)
    {
        _enabled = CMPI_false;
    }
    CMReturn (CMPI_RC_OK);
}



/* ---------------------------------------------------------------------------*/
/*                       Method Provider Interface                        */
/* ---------------------------------------------------------------------------*/

CMPIStatus TestCMPIIndicationStressTestProviderMethodCleanup(
                                     CMPIMethodMI * mi,
                                     const CMPIContext * ctx,
                                     CMPIBoolean  term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIIndicationStressTestProviderInvokeMethod(
                                    CMPIMethodMI * mi,
                                    const CMPIContext * ctx,
                                    const CMPIResult * rslt,
                                    const CMPIObjectPath * ref,
                                    const char *methodName,
                                    const CMPIArgs * in, CMPIArgs * out)
{
    CMPIUint32 indicationSendCount = 0;
    CMPIInstance *indicationInstance = 0;
    CMPIData data;
    CMPIValue value;
    char UIDbuffer[32];
    CMPIUint32 seqNum;

    if (!strcmp(methodName, "getSubscriptionCount"))
    {
        value.uint32 = _getSubCount();
        CMReturnData (rslt, &value, CMPI_uint32);
        CMReturnDone(rslt);
        CMReturn (CMPI_RC_OK);
    }

    data = CMGetArgAt (in, 0, NULL, NULL);
    if (data.type == CMPI_uint32)
    {
        indicationSendCount = data.value.uint32;
    }

    if (_enabled && _getSubCount() == 12)
    {
        value.uint32 = 0;
        CMReturnData (rslt, &value, CMPI_sint32);
        CMReturnDone (rslt);
    }
    else
    {
        CMReturn (CMPI_RC_ERR_FAILED);
    }

    objPath = CMNewObjectPath (
        _broker,
        _sourceNamespaces[_namespaceIndex++],
        _IndClassName,
        NULL);

    if (_namespaceIndex == 3)
    {
        _namespaceIndex = 0;
    }

    if (indicationSendCount > 0)
    {
        for (seqNum = 1; seqNum < indicationSendCount+1; seqNum++)
        {
             sprintf(UIDbuffer, "%u", _nextUID++);
             indicationInstance = getIndicationInstance (seqNum, UIDbuffer);
             CBDeliverIndication (_broker, ctx, namespaceName,
                                           indicationInstance);
        }
    }
    free (namespaceName);
    CMRelease (objPath);

    CMReturn (CMPI_RC_OK);
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMIndicationMIStub (TestCMPIIndicationStressTestProvider,
                    TestCMPIIndicationStressTestProvider,_broker,_createMutex())


CMMethodMIStub (TestCMPIIndicationStressTestProvider,
                TestCMPIIndicationStressTestProvider, _broker, CMNoHook)

/* ---------------------------------------------------------------------------*/
/*                      end of TestCMPIProvider                               */
/* ---------------------------------------------------------------------------*/


