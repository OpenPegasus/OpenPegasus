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

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

static const CMPIBroker *_broker;

/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/

CMPIStatus TestCMPIKeyReturnedProviderCleanup(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean term)
{
  CMReturn(CMPI_RC_OK);
}

CMPIStatus TestCMPIKeyReturnedProviderEnumInstanceNames(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

/* Test Instance A
   - Build a new Instance from scratch grabbing namespace and classname from
   classPath, but not adding the keys to the new CMPIObjectPath.
*/
void test_Instance_A(
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
    CMPIValue num;
    CMPIValue flag;
    CMPIObjectPath * myObjPath=NULL;
    CMPIInstance * myInst=NULL;

    num.uint32=39;
    flag.boolean=1;

    myObjPath= CMNewObjectPath(
        _broker,
        CMGetCharPtr(CMGetNameSpace(ref,0)),
        "TestCMPI_KeyReturned",
        0);

    myInst=CMNewInstance(_broker,myObjPath,0);

    CMSetProperty(myInst,"Name", "TestInstanceA", CMPI_chars);
    CMSetProperty(myInst,"Number", &num, CMPI_uint32);
    CMSetProperty(myInst,"Flag", &flag, CMPI_boolean);

    CMReturnInstance(rslt, myInst);
    CMRelease(myObjPath);
    CMRelease(myInst);
}

/* Test Instance B1
   - Build a new instance from a complete new CMPIObjectPath with the key
   properties being set, as well as instance properties
*/
void test_Instance_B1(
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
    CMPIValue num;
    CMPIValue flag;
    CMPIObjectPath * myObjPath=NULL;
    CMPIInstance * myInst=NULL;

    num.uint32=40;
    flag.boolean=1;


    myObjPath=CMNewObjectPath(
        _broker,
        CMGetCharPtr(CMGetNameSpace(ref,0)),
        "TestCMPI_KeyReturned",
        0);

    CMAddKey(myObjPath,"Name", "TestInstanceB1", CMPI_chars);
    CMAddKey(myObjPath,"Number", &num, CMPI_uint32);
    CMAddKey(myObjPath,"Flag", &flag, CMPI_boolean);

    myInst=CMNewInstance(_broker,myObjPath,0);

    CMSetProperty(myInst,"Name", "TestInstanceB1", CMPI_chars);
    CMSetProperty(myInst,"Number", &num, CMPI_uint32);
    CMSetProperty(myInst,"Flag", &flag, CMPI_boolean);

    CMReturnInstance(rslt, myInst);
    CMRelease(myObjPath);
    CMRelease(myInst);
}

/* Test Instance B2
   - Build a new instance from a complete new CMPIObjectPath with the key
   properties being set, but NOT the instance properties
*/
void test_Instance_B2(
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
    CMPIValue num;
    CMPIValue flag;
    CMPIObjectPath * myObjPath=NULL;
    CMPIInstance * myInst=NULL;

    num.uint32=40;
    flag.boolean=1;


    myObjPath=CMNewObjectPath(
        _broker,
        CMGetCharPtr(CMGetNameSpace(ref,0)),
        "TestCMPI_KeyReturned",
        0);

    CMAddKey(myObjPath,"Name", "TestInstanceB2", CMPI_chars);
    CMAddKey(myObjPath,"Number", &num, CMPI_uint32);
    CMAddKey(myObjPath,"Flag", &flag, CMPI_boolean);

    myInst=CMNewInstance(_broker,myObjPath,0);

    CMReturnInstance(rslt, myInst);
    CMRelease(myObjPath);
    CMRelease(myInst);
}

/* Test Instance C1
   - Build a new instance from the given classPath and do add the keybindings in
   the provider as well as the properties
*/
void test_Instance_C1(
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
    CMPIValue num;
    CMPIValue flag;
    CMPIObjectPath * myObjPath=NULL;
    CMPIInstance * myInst=NULL;

    num.uint32=41;
    flag.boolean=1;

    /* Using ref requires a clone here */
    myObjPath=CMClone(ref,0);

    CMAddKey(myObjPath,"Name", "TestInstanceC2", CMPI_chars);
    CMAddKey(myObjPath,"Number", &num, CMPI_uint32);
    CMAddKey(myObjPath,"Flag", &flag, CMPI_boolean);

    myInst=CMNewInstance(_broker,myObjPath,0);

    CMSetProperty(myInst,"Name", "TestInstanceC1", CMPI_chars);
    CMSetProperty(myInst,"Number", &num, CMPI_uint32);
    CMSetProperty(myInst,"Flag", &flag, CMPI_boolean);

    CMReturnInstance(rslt, myInst);
    CMRelease(myObjPath);
    CMRelease(myInst);
}

/* Test Instance C2
   - Build a new instance from the given classPath and do add the keybindings in
   the provider but NOT the instance properties
*/
void test_Instance_C2(
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
    CMPIValue num;
    CMPIValue flag;
    CMPIObjectPath * myObjPath=NULL;
    CMPIInstance * myInst=NULL;

    num.uint32=41;
    flag.boolean=1;

    /* Using ref requires a clone here */
    myObjPath=CMClone(ref,0);

    CMAddKey(myObjPath,"Name", "TestInstanceC2", CMPI_chars);
    CMAddKey(myObjPath,"Number", &num, CMPI_uint32);
    CMAddKey(myObjPath,"Flag", &flag, CMPI_boolean);

    myInst=CMNewInstance(_broker,myObjPath,0);

    CMReturnInstance(rslt, myInst);
    CMRelease(myObjPath);
    CMRelease(myInst);
}

/* Test Instance D
   - test instance is directly built using the supplied classPath (ref)
   but without setting the keys, instead only supplying values in the properties
*/
void test_Instance_D(
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
    CMPIValue num;
    CMPIValue flag;
    CMPIInstance * myInst=NULL;

    num.uint32=42;
    flag.boolean=1;

    myInst=CMNewInstance(_broker,ref,0);

    CMSetProperty(myInst,"Name", "TestInstanceD", CMPI_chars);
    CMSetProperty(myInst,"Number", &num, CMPI_uint32);
    CMSetProperty(myInst,"Flag", &flag, CMPI_boolean);

    CMReturnInstance(rslt, myInst);
    CMRelease(myInst);
}

CMPIStatus TestCMPIKeyReturnedProviderEnumInstances(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
/* - Build a new Instance from scratch grabbing namespace and classname from
   classPath, but not adding the keys to the new CMPIObjectPath. */
    test_Instance_A(rslt,ref,properties);

/* - Build a new instance from a complete new CMPIObjectPath with the key
   properties being set, as well as instance properties */
    test_Instance_B1(rslt,ref,properties);

/* - Build a new instance from a complete new CMPIObjectPath with the key
   properties being set, but NOT the instance properties */
    test_Instance_B2(rslt,ref,properties);

/* - Build a new instance from the given classPath and do add the keybindings in
   the provider as well as the properties */
    test_Instance_C1(rslt,ref,properties);

/* - Build a new instance from the given classPath and do add the keybindings in
   the provider but NOT the instance properties */
    test_Instance_C2(rslt,ref,properties);

/* - test instance is directly built using the supplied classPath (ref) but
   without setting the keys, instead only supplying values in the properties */
    test_Instance_D(rslt,ref,properties);

    CMReturnDone(rslt);
    CMReturn(CMPI_RC_OK);
}

CMPIStatus TestCMPIKeyReturnedProviderGetInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const char **properties)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIKeyReturnedProviderCreateInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIKeyReturnedProviderModifyInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci,
    const char **properties)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIKeyReturnedProviderDeleteInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIKeyReturnedProviderExecQuery(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *lang,
    const char *query)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}


/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/
CMInstanceMIStub(
    TestCMPIKeyReturnedProvider,
    TestCMPIKeyReturnedProvider,
    _broker,
    CMNoHook);

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIKeyReturnedProvider                             */
/* ---------------------------------------------------------------------------*/
