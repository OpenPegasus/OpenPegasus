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

/* The Objective of this provider is to cover/execute the Error paths in
   CMPIProviderManager.cpp. To achieve this, the return value expected in the
   CMPIProviderManager.cpp is CMPI_RC_ERR_NOT_SUPPORTED or
   CMPI_RC_DO_NOT_UNLOAD. In this provider all the methods are intentionaly
   returning CMPI_RC_ERR_NOT_SUPPORTED to meet requirement for executing the
   code in the error paths.  With these changes all the
   'if (rc.rc != CMPI_RC_OK)' statements in CMPIProviderManager.cpp evaluates
   to TRUE and, there by improving the code coverage.
*/

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

static const CMPIBroker *_broker;

/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/

/* Clean up routine for Instance Provider.
   This function returns CMPI_RC_OK when cimserver terminating otherwise
   it returns CMPI_RC_DO_NOT_UNLOAD to inform MB this provider should not get
   unload at this time.
*/
CMPIStatus TestCMPIFail_5ProviderCleanup(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean term)
{
    if (term)
    {
        CMReturn (CMPI_RC_OK);
    }
    else
    {
        CMReturn (CMPI_RC_DO_NOT_UNLOAD);
    }
}

/* EnumInstanceNames routine for Instance Provider. */
CMPIStatus TestCMPIFail_5ProviderEnumInstanceNames(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref)
{
   CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* EnumInstances routine for Instance Provider.*/

CMPIStatus TestCMPIFail_5ProviderEnumInstances(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
   CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* GetInstance routine for Instance Provider. */

CMPIStatus TestCMPIFail_5ProviderGetInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const char **properties)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* CreateInstance routine for Instance Provider. */

CMPIStatus TestCMPIFail_5ProviderCreateInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ModifyInstance routine for Instance Provider. */

CMPIStatus TestCMPIFail_5ProviderModifyInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci,
    const char **properties)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* DeleteInstance routine for Instance Provider. */

CMPIStatus TestCMPIFail_5ProviderDeleteInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ExecQuery routine for Instance Provider. */

CMPIStatus TestCMPIFail_5ProviderExecQuery(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *lang,
    const char *query)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ---------------------------------------------------------------------------*/
/*                       Indication Provider Interface                        */
/* ---------------------------------------------------------------------------*/

/* Cleanup routine for Indication Provider.
   This function returns CMPI_RC_OK when cimserver terminating otherwise
   it returns CMPI_RC_DO_NOT_UNLOAD to inform MB this provider should not get
   unload at this time.
*/

CMPIStatus TestCMPIFail_5ProviderIndicationCleanup(
    CMPIIndicationMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean term)
{
    if (term)
    {
        CMReturn (CMPI_RC_OK);
    }
    else
    {
        CMReturn (CMPI_RC_DO_NOT_UNLOAD);
    }
}

/* AuthorizeFilter routine for Indication Provider. */

CMPIStatus TestCMPIFail_5ProviderAuthorizeFilter(
    CMPIIndicationMI * mi,
    const CMPIContext * ctx,
    const CMPISelectExp * se,
    const char *ns,
    const CMPIObjectPath * op,
    const char *user)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* MustPoll routine for Indication Provider. */

CMPIStatus TestCMPIFail_5ProviderMustPoll(
    CMPIIndicationMI * mi,
    const CMPIContext * ctx,
    const CMPISelectExp * se,
    const char *ns,
    const CMPIObjectPath * op)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ActivateFilter routine for Indication Provider. */

CMPIStatus TestCMPIFail_5ProviderActivateFilter(
    CMPIIndicationMI * mi,
    const CMPIContext * ctx,
    const CMPISelectExp * se,
    const char *clsName,
    const CMPIObjectPath * op,
    CMPIBoolean firstActivation)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* DeActivateFilter routine for Indication Provider. */

CMPIStatus TestCMPIFail_5ProviderDeActivateFilter(
    CMPIIndicationMI * mi,
    const CMPIContext * ctx,
    const CMPISelectExp * se,
    const char *clsName,
    const CMPIObjectPath * op,
    CMPIBoolean lastActivation)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* EnableIndications routine for Indication Provider. */

CMPIStatus TestCMPIFail_5ProviderEnableIndications(
    CMPIIndicationMI * mi,
    const CMPIContext * ctx)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* DisableIndications routine for Indication Provider. */

CMPIStatus TestCMPIFail_5ProviderDisableIndications(
    CMPIIndicationMI * mi,
    const CMPIContext * ctx)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ---------------------------------------------------------------------------*/
/*                       Method Provider Interface                            */
/* ---------------------------------------------------------------------------*/

/* Cleanup routine for Method Provider.
   This function returns CMPI_RC_OK when cimserver terminating otherwise
   it returns CMPI_RC_DO_NOT_UNLOAD to inform MB this provider should not get
   unload at this time.
*/

CMPIStatus TestCMPIFail_5ProviderMethodCleanup(
    CMPIMethodMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    if (term)
    {
        CMReturn (CMPI_RC_OK);
    }
    else
    {
        CMReturn (CMPI_RC_DO_NOT_UNLOAD);
    }
}

/* InvokeMethod routine for Method Provider. */

CMPIStatus TestCMPIFail_5ProviderInvokeMethod(
    CMPIMethodMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *methodName,
    const CMPIArgs * in,
    CMPIArgs * out)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* -------------------------------------------------------------------------- */
/*                        Association Provider Interface                      */
/* -------------------------------------------------------------------------- */

/* Cleanup routine for Association Provider.
   This function returns CMPI_RC_OK when cimserver terminating otherwise
   it returns CMPI_RC_DO_NOT_UNLOAD to inform MB this provider should not get
   unload at this time.
*/

CMPIStatus TestCMPIFail_5ProviderAssociationCleanup(
    CMPIAssociationMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    if (term)
    {
        CMReturn (CMPI_RC_OK);
    }
    else
    {
        CMReturn (CMPI_RC_DO_NOT_UNLOAD);
    }

}

/* Associators routine for Association Provider. */

CMPIStatus TestCMPIFail_5ProviderAssociators(
    CMPIAssociationMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char * _RefLeftClass,
    const char * _RefRightClass,
    const char *role,
    const char *resultRole,
    const char** properties)
{
   CMPIInstance *ci = NULL;
   CMReturnInstance(rslt, ci);

   CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

/* AssociatorNames routine for Association Provider.*/

CMPIStatus TestCMPIFail_5ProviderAssociatorNames(
    CMPIAssociationMI* mi,
    const CMPIContext* ctx,
    const CMPIResult* rslt,
    const CMPIObjectPath* ref,
    const char *_RefLeftClass,
    const char *_RefRightClass,
    const char *role,
    const char *resultRole)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* References routine for Association Provider. */

CMPIStatus TestCMPIFail_5ProviderReferences(
    CMPIAssociationMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *resultClass,
    const char *role ,
    const char** properties)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ReferenceNames routine for Association Provider. */

CMPIStatus TestCMPIFail_5ProviderReferenceNames(
    CMPIAssociationMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char* resultClass,
    const char* role)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub(
    TestCMPIFail_5Provider,
    TestCMPIFail_5Provider,
    _broker,
    CMNoHook)

CMIndicationMIStub(
    TestCMPIFail_5Provider,
    TestCMPIFail_5Provider,
    _broker,
    CMNoHook)

CMMethodMIStub(
    TestCMPIFail_5Provider,
    TestCMPIFail_5Provider,
    _broker,
    CMNoHook)

CMAssociationMIStub(
    TestCMPIFail_5Provider,
    TestCMPIFail_5Provider,
    _broker,
    CMNoHook)

