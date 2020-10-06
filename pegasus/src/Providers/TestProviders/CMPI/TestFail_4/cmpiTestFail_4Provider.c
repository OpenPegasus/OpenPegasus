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


#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/
static const CMPIBroker *_broker;

const char *_msg =  "&quot;&lt;MSGS&gt;&lt;MSG COMP=&quot;CIMVM&quot;" \
    " RC=&quot;400&quot; RS=&quot;65&quot; MSGID=&quot;DNZZMC465E&quot;" \
    " COMM AND=&quot;Transaction: CimVm::CbCProvisioningService::createUser;" \
    " Subtask: Process disk settings&quot;&gt;Fehler beim Erstellen eines" \
    " virtuellen Computersystems (Aktion wurde rückgängig gemacht).&#13;" \
    "&lt;/MSG&gt;&lt;MSG COMP=&quot;CIMVM&quot; RC=&quot;200&quot;" \
    " RS=&quot;5&quot; MSGID=&quot;DNZZMC205E&quot; ROOTCAUSE=&quot;" \
    "true&quot;&gt;&amp;quot;22500&amp;quot; ist keine gültige" \
    " Einheitennummer.&#13;&lt;/MSG&gt;&lt;/MSGS&gt;&quot;";

CMPIStatus TestCMPIFail_4ProviderCleanup(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
  if (!term) // If CIM Server not terminating
  {
      // Test preventing the unload of an idle provider
      CMReturn(CMPI_RC_DO_NOT_UNLOAD);
  }

  CMReturn (CMPI_RC_OK);
}

CMPIStatus
TestCMPIFail_4ProviderEnumInstanceNames (CMPIInstanceMI * mi,
                                         const CMPIContext * ctx,
                                         const CMPIResult * rslt,
                                         const CMPIObjectPath * ref)
{
  CMReturnWithChars (_broker, CMPI_RC_ERR_NOT_FOUND, _msg);
}

CMPIStatus
TestCMPIFail_4ProviderEnumInstances (CMPIInstanceMI * mi,
                                     const CMPIContext * ctx,
                                     const CMPIResult * rslt,
                                     const CMPIObjectPath * ref,
                                     const char **properties)
{
    // This out-of-place macro was added to test bug #4107
    // in which CMReturnDone without a previous CMReturnInstance would
    // crash cimserver.

    CMReturnDone(rslt);
    CMReturnWithChars (_broker, CMPI_RC_ERR_NOT_FOUND, _msg);
}

CMPIStatus
TestCMPIFail_4ProviderGetInstance (CMPIInstanceMI * mi,
                                   const CMPIContext * ctx,
                                   const CMPIResult * rslt,
                                   const CMPIObjectPath * cop,
                                   const char **properties)
{
    // This out-of-place macro was added to test bug #4107
    // in which CMReturnDone without a previous CMReturnInstance would
    // crash cimserver.

    CMReturnDone(rslt);
    CMReturnWithChars (_broker, CMPI_RC_ERR_NOT_FOUND, _msg);
}

CMPIStatus
TestCMPIFail_4ProviderCreateInstance (CMPIInstanceMI * mi,
                                      const CMPIContext * ctx,
                                      const CMPIResult * rslt,
                                      const CMPIObjectPath * cop,
                                      const CMPIInstance * ci)
{
    // This out-of-place macro was added to test bug #4107
    // in which CMReturnDone without a previous CMReturnInstance would
    // crash cimserver.

    CMReturnDone(rslt);
    CMReturnWithChars (_broker, CMPI_RC_ERR_NOT_FOUND, _msg);
}

CMPIStatus
TestCMPIFail_4ProviderModifyInstance (CMPIInstanceMI * mi,
                                      const CMPIContext * ctx,
                                      const CMPIResult * rslt,
                                      const CMPIObjectPath * cop,
                                      const CMPIInstance * ci,
                                      const char **properties)
{
    // This out-of-place macro was added to test bug #4107
    // in which CMReturnDone without a previous CMReturnInstance would
    // crash cimserver.

    CMReturnDone(rslt);
    CMReturnWithChars (_broker, CMPI_RC_ERR_NOT_FOUND, _msg);
}

CMPIStatus
TestCMPIFail_4ProviderDeleteInstance (CMPIInstanceMI * mi,
                                      const CMPIContext * ctx,
                                      const CMPIResult * rslt,
                                      const CMPIObjectPath * cop)
{
    // This out-of-place macro was added to test bug #4107
    // in which CMReturnDone without a previous CMReturnInstance would
    // crash cimserver.

    CMReturnDone(rslt);
    CMReturnWithChars (_broker, CMPI_RC_ERR_NOT_FOUND, _msg);
}

/*
 * We don't support this function.
 */
/*
CMPIStatus
TestCMPIFail_4ProviderExecQuery (CMPIInstanceMI * mi,
                                 const CMPIContext * ctx,
                                 const CMPIResult * rslt,
                                 const CMPIObjectPath * ref,
                                 const char *lang, const char *query)
{
  CMReturnWithChars (_broker, CMPI_RC_ERR_NOT_FOUND, _msg);
}
*/
/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/
/*
CMInstanceMIStub (TestCMPIFail_4Provider,
                  TestCMPIFail_4Provider, _broker, CMNoHook);
*/

static CMPIInstanceMIFT instMIFT__ = {
    100, 100, "instance" "TestCMPIFail_4Provider",
    TestCMPIFail_4ProviderCleanup,
    TestCMPIFail_4ProviderEnumInstanceNames,
    TestCMPIFail_4ProviderEnumInstances,
    TestCMPIFail_4ProviderGetInstance,
    TestCMPIFail_4ProviderCreateInstance,
    TestCMPIFail_4ProviderModifyInstance,
    TestCMPIFail_4ProviderDeleteInstance,
    NULL
};
CMPI_EXTERN_C CMPIInstanceMI *
TestCMPIFail_4Provider_Create_InstanceMI (const CMPIBroker * brkr,
                                          const CMPIContext * ctx,
                                          CMPIStatus * rc)
{
  static CMPIInstanceMI mi = { ((void *) 0), &instMIFT__, };
  _broker = brkr;
  return &mi;
}

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
