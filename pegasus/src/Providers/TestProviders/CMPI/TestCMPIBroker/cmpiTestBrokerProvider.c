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
#include <Providers/TestProviders/CMPI/TestUtilLib/cmpiUtilLib.h>

#define _ClassName "TestCMPI_Broker"
#define _Namespace    "test/TestProvider"

#define _ProviderLocation "/src/Providers/TestProviders/CMPI/" \
    "TestCMPIBroker/tests/"

static const CMPIBroker *_broker;

/* ---------------------------------------------------------------------------*/
/*                       CMPI Helper function                        */
/* ---------------------------------------------------------------------------*/

CMPIObjectPath * make_ObjectPath (
    const CMPIBroker *broker,
    const char *ns,
    const char *className)
{
    CMPIObjectPath *objPath = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    PROV_LOG ("++++  make_ObjectPath: CMNewObjectPath");
    objPath = CMNewObjectPath (broker, ns, className, &rc);

    PROV_LOG ("++++  CMNewObjectPath : (%s)", strCMPIStatus (rc));
    CMAddKey (objPath, "ElementName", (CMPIValue *) className, CMPI_chars);

    return objPath;
}
static int _testBrokerServices(const CMPIContext * ctx,
    const CMPIArgs * in,
    CMPIArgs * out)
{
    CMPIObjectPath* objPath;
    CMPIObjectPath* retObjPath;
    CMPIObjectPath* obj;
    CMPIString* retNamespace = NULL;
    CMPIString* retClassname = NULL;
    CMPIString* name = NULL;
    const char* str1;
    const char* str2;
    CMPIData retData;
    CMPIData data;
    CMPIInstance *inst = NULL;
    CMPIInstance *retInst = NULL;
    CMPIStatus rc = {CMPI_RC_OK, NULL};
    CMPIStatus rc1 = {CMPI_RC_OK, NULL};
    CMPIEnumeration* enumeration;
    CMPIArray* arr_ptr;
    CMPICount returnedArraySize;
    CMPIValue value;
    int flag;
    unsigned int initCount = 0;

    flag = 1;

    PROV_LOG("Test CMPI_Broker.cpp");

    PROV_LOG("++++ Testing with TestCMPIExecQueryProvider ++++");
    objPath = CMNewObjectPath (_broker,
        "test/TestProvider",
        "TestCMPI_ExecQuery",
        &rc);
    PROV_LOG("++++ Status of CMNewObjectPath : (%s)", strCMPIStatus(rc));
    inst = CMNewInstance(_broker, objPath, &rc);
    PROV_LOG("++++ Status of CMNewInstance : (%s)", strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    retObjPath = CBCreateInstance(_broker, ctx, objPath, inst, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    if (retObjPath == NULL)
    {
        PROV_LOG("++++ Error Status of CBCreateInstance : (%s)",
            strCMPIStatus(rc));
    }

    PROV_LOG_CLOSE();
    retInst = CBGetInstance(_broker, ctx, objPath, NULL, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    if (retInst == NULL)
    {
        PROV_LOG("++++ Error Status of CBGetInstance : (%s)",
            strCMPIStatus(rc));
    }

    PROV_LOG_CLOSE();
    rc = CBModifyInstance(_broker, ctx, objPath, inst, NULL);
    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Error Status of CBModifyInstance : (%s)",
        strCMPIStatus(rc));

     PROV_LOG_CLOSE();
     rc = CBSetProperty(_broker,
        ctx,
        objPath,
        "c",
        (CMPIValue *) "c",
        CMPI_char16);
    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Error Status of CBSetProperty : (%s)",
        strCMPIStatus(rc));

    PROV_LOG_CLOSE();

    retData = CBGetProperty(_broker,
        ctx,
        objPath,
        "c",
        &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Error Status of CBGetProperty : (%s)",
        strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    enumeration = CBEnumInstanceNames(_broker,
        ctx,
        objPath,
        &rc);
    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBEnumInstanceNames : (%s)",
        strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    enumeration = CBEnumInstances(_broker,
        ctx,
        objPath,
        NULL,
        &rc);
    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBEnumInstances : (%s)",
        strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    rc = CBDeleteInstance(_broker, ctx, objPath);
    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Error Status of CBDeleteInstance : (%s)",
        strCMPIStatus(rc));

    rc = CMRelease(objPath);
    PROV_LOG("++++ Status of CMRelease(objPath) : (%s)",
        strCMPIStatus(rc));

    PROV_LOG("++++ CBReferences ++++");
    objPath = CMNewObjectPath (_broker,
        "test/TestProvider",
        "CMPI_TEST_Person",
        &rc);
    PROV_LOG("++++ Status of CMNewObjectPath for CMPI_TEST_Person : (%s)",
        strCMPIStatus(rc));

    name = CMNewString(_broker, "Melvin", &rc);
    PROV_LOG("++++ Status of CMNewString name with value Melvin : (%s) (%s)",
        strCMPIStatus(rc),
        CMGetCharsPtr(name, NULL));

    value.string = name;
    rc = CMAddKey(objPath, "name", &value, CMPI_string);
    PROV_LOG("++++ Status of CMAddKey : (%s)", strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    enumeration = CBReferences (_broker,
        ctx,
        objPath,
        NULL,
        NULL,
        NULL,
        &rc);
    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBReferences : (%s)",
        strCMPIStatus(rc));
    if (enumeration)
    {
        arr_ptr = CMToArray(enumeration, &rc);
        PROV_LOG ("CMToArray : (%s)", strCMPIStatus (rc));
        if (arr_ptr == NULL)
        {
            PROV_LOG("---- CMToArray failed ");
        }

        returnedArraySize = CMGetArrayCount(arr_ptr, &rc);
        PROV_LOG (" ReturnedArraySize :%d", returnedArraySize);
        PROV_LOG(" Calling CMHasNext");
        while (CMHasNext(enumeration, &rc))
        {
            PROV_LOG(" Calling CMGetNext");
            data = CMGetNext(enumeration, &rc);
            PROV_LOG (" CMGetNext : (%s)", strCMPIStatus (rc));
            if (data.type != CMPI_instance)
            {
                PROV_LOG(" Returned value is incorrect");
            }
            initCount++;
        }
        PROV_LOG(" Enum count %d ", initCount);
        PROV_LOG("CBReferences is Successful");
    }

    rc = CMRelease(objPath);
    PROV_LOG("++++ Status of CMRelease(objPath) : (%s)",
        strCMPIStatus(rc));

    PROV_LOG("++++ Testing with BrokerInstance provider ++++");
    objPath = CMNewObjectPath (_broker,
        "test/TestProvider",
        "TestCMPI_BrokerInstance",
        &rc);
    PROV_LOG("++++ Status of CMNewObjectPath for TestCMPI_BrokerInstance:(%s)",
        strCMPIStatus(rc));
    inst = CMNewInstance(_broker, objPath, &rc);
    PROV_LOG("++++ Status of CMNewInstance : (%s)", strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    retObjPath = CBCreateInstance(_broker, ctx, objPath, inst, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBCreateInstance : (%s)",
        strCMPIStatus(rc));
    retNamespace = CMGetNameSpace(retObjPath, &rc);
    retClassname = CMGetClassName(retObjPath, &rc1);

    if((rc.rc == CMPI_RC_OK) && (rc1.rc == CMPI_RC_OK))
    {
        str1 = CMGetCharsPtr(retNamespace, &rc);
        str2 = CMGetCharsPtr(retClassname, &rc1);

        if ((rc.rc == CMPI_RC_OK) && (rc1.rc == CMPI_RC_OK))
        {
            if (strcmp(str2, "TestCMPI_BrokerInstance"))
            {
                flag = 0;
            }
        }
        else
        {
            flag = 0;
        }
    }
    else
    {
        flag = 0;
    }
    if(flag)
    {
        PROV_LOG("Test for CBCreateInstance is successful ");
    }

    rc = CMSetNameSpace(retObjPath, "test/TestProvider");
    PROV_LOG("++++ Status of CMSetNameSpace : (%s)",
        strCMPIStatus(rc));

    retNamespace = CMGetNameSpace(retObjPath, &rc);
    retClassname = CMGetClassName(retObjPath, &rc);
    str1 = CMGetCharsPtr(retNamespace, &rc);
    str2 = CMGetCharsPtr(retClassname, &rc);
    PROV_LOG("++++Returned Namespace : (%s) and Class (%s)", str1, str2);

    PROV_LOG("++++ CBGetInstance ++++");

    PROV_LOG_CLOSE();
    retInst = CBGetInstance(_broker, ctx, retObjPath, NULL, &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    obj = CMGetObjectPath (retInst, &rc);
    retNamespace = CMGetNameSpace(obj, &rc);
    retClassname = CMGetClassName(obj, &rc);
    str1 = CMGetCharsPtr(retNamespace, &rc);
    str2 = CMGetCharsPtr(retClassname, &rc);

    if (retInst)
    {
        retData = CMGetProperty(retInst, "n64", &rc);
        PROV_LOG("++++ Status of CMGetProperty : (%s)",
            strCMPIStatus(rc));
        if (retData.value.uint64 == 64)
        {
            PROV_LOG("++++ Status of CBGetInstance : (%s)",
                strCMPIStatus(rc));
        }
    }
    PROV_LOG_CLOSE();
    data = CBGetProperty(_broker,
        ctx,
        retObjPath,
        "n32",
        &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBGetProperty : (%s)",
        strCMPIStatus(rc));
    {
       PROV_LOG("++++ Value is (%s)", CMGetCharsPtr(data.value.string, &rc));
    }

    PROV_LOG_CLOSE();
    rc = CBModifyInstance(_broker, ctx, retObjPath, retInst, NULL);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBModifyInstance : (%s)",
        strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    rc = CBSetProperty(_broker,
        ctx,
        retObjPath,
        "c",
        (CMPIValue *) "c",
        CMPI_char16);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBSetProperty : (%s)",
        strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    enumeration = CBExecQuery(_broker,
        ctx,
        retObjPath,
        "SELECT * FROM TestCMPI_BrokerInstance",
        "WQL",
        &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBExecQuery : (%s)",
        strCMPIStatus(rc));
    initCount = 0;
    if(enumeration)
    {
        arr_ptr = CMToArray(enumeration, &rc);
        PROV_LOG ("  CMToArray : (%s)", strCMPIStatus (rc));
        if (arr_ptr == NULL)
        {
            PROV_LOG("---- CMToArray failed  ----");
        }
        returnedArraySize = CMGetArrayCount(arr_ptr, &rc);
        PROV_LOG (" ReturnedArraySize :%d", returnedArraySize);
        PROV_LOG(" Calling CMHasNext");
        while (CMHasNext(enumeration, &rc))
        {
            PROV_LOG(" Calling CMGetNext");
            data = CMGetNext(enumeration, &rc);
            PROV_LOG ("  CMGetNext : (%s)", strCMPIStatus (rc));
            if (data.type != CMPI_instance)
            {
                PROV_LOG(" Returned value is incorrect ");
            }
            initCount++;
        }
        PROV_LOG(" Enum count %d ", initCount);
        PROV_LOG("ExecQuery on BrokerInstance Provider - Successful");
    }

    PROV_LOG_CLOSE();
    rc = CBDeliverIndication(_broker, ctx, "test/TestProvider", retInst);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Error Status of CBDeliverIndication : (%s)",
        strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    rc = CBDeleteInstance(_broker, ctx, retObjPath);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++ Status of CBDeleteInstance : (%s)",
        strCMPIStatus(rc));
    objPath = CMNewObjectPath (_broker,
        "test/TestProvider",
        "TestCMPI_BrokerInstance",
        &rc);

    PROV_LOG_CLOSE();
    retData = CBInvokeMethod(_broker,
        ctx,
        objPath,
        "TestCMPIError",
        in,
        out,
        &rc);
    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    if (!retData.value.uint32 && rc.rc != CMPI_RC_OK)
    {
        PROV_LOG("++++ Error Status of CBInvokeMethod: (%s)",
            strCMPIStatus (rc));
    }
    else
    {
        flag = 0;
    }
    rc = CMRelease(objPath);
    PROV_LOG("++++ Status of CMRelease(objPath) : (%s)",
        strCMPIStatus(rc));

    // Call back ourself, this checks whether MB will hang on recursive
    // call back. See bug 6925.
    PROV_LOG("++++ Testing with TestCMPIBrokerProvider ++++");
    objPath = CMNewObjectPath (_broker,
        "test/TestProvider",
        "TestCMPI_Broker",
        &rc);
    PROV_LOG("++++ Status of CMNewObjectPath : (%s)", strCMPIStatus(rc));

    PROV_LOG_CLOSE();
    enumeration = CBEnumInstances(_broker,
        ctx,
        objPath,
        NULL,
        &rc);
    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);
    PROV_LOG("++++ Status of CBEnumInstances : (%s)", strCMPIStatus(rc));

    return flag;
}

/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/
CMPIStatus TestCMPIBrokerProviderMethodCleanup (CMPIMethodMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIBrokerProviderInvokeMethod (CMPIMethodMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *methodName,
    const CMPIArgs * in,
    CMPIArgs * out)
{
    CMPIString *class_name = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIData data;

    CMPIString *argName = NULL;

    unsigned int arg_cnt = 0, index = 0;

    CMPIUint32 oper_rc = 1;

    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG ("--- %s CMPI InvokeMethod() called", _ClassName);

    class_name = CMGetClassName (ref, &rc);

    PROV_LOG ("InvokeMethod: checking for correct classname [%s]",
        CMGetCharsPtr (class_name,NULL));

    PROV_LOG ("Calling CMGetArgCount");
    arg_cnt = CMGetArgCount (in, &rc);
    PROV_LOG ("++++ (%s)", strCMPIStatus (rc));

    PROV_LOG ("InvokeMethod: We have %d arguments for operation [%s]: ",
        arg_cnt,
        methodName);

    if (arg_cnt > 0)
    {
        PROV_LOG ("Calling CMGetArgAt");
        for (index = 0; index < arg_cnt; index++)
        {
            data = CMGetArgAt (in, index, &argName, &rc);
            if (data.type == CMPI_uint32)
            {
                PROV_LOG ("#%d: %s (uint32), value: %d",
                    index,
                    CMGetCharsPtr (argName,NULL),
                    data.value.uint32);
            }
            else if (data.type == CMPI_string)
            {
                PROV_LOG ("#%d: %s (string) value: %s",
                    index,
                    CMGetCharsPtr (argName,NULL),
                    CMGetCharsPtr (data.value.string,NULL));
            }
            else
            {
                PROV_LOG ("#%d: %s (type: %x)",
                    index,
                    CMGetCharsPtr (argName,NULL),
                    data.type);
            }
            CMRelease (argName);
        }
    }
    if (strncmp(
        CMGetCharsPtr(class_name,NULL),
        _ClassName,
         strlen(_ClassName)) == 0)
    {
        if (strncmp("testBrokerServices",
            methodName,
            strlen ("testBrokerServices"))== 0)
        {
            oper_rc = _testBrokerServices(ctx, in, out);
            CMReturnData (rslt, (CMPIValue *) &oper_rc, CMPI_uint32);
            CMReturnDone (rslt);
        }
        else
        {
            PROV_LOG ("++++ Could not find the %s operation", methodName);
            rc.rc = CMPI_RC_ERR_NOT_FOUND;
            rc.msg=_broker->eft->newString(_broker,methodName,0);
        }
    }
    PROV_LOG ("--- %s CMPI InvokeMethod() exited", _ClassName);
    PROV_LOG_CLOSE();
    return rc;
}

CMPIStatus TestCMPIBrokerProviderCleanup(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean term)
{
    CMReturn (CMPI_RC_OK);
}

/* EnumInstanceNames routine for Instance Provider. */
CMPIStatus TestCMPIBrokerProviderEnumInstanceNames(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref)
{
   CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* EnumInstances routine for Instance Provider.*/

CMPIStatus TestCMPIBrokerProviderEnumInstances(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
    // This is called when we call back ourself from the InvokeMethod.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG ("--- %s CMPI EnumInstances() called", _ClassName);
    PROV_LOG ("--- SUCCESS ---");
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIBrokerProviderGetInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const char **properties)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* CreateInstance routine for Instance Provider. */

CMPIStatus TestCMPIBrokerProviderCreateInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ModifyInstance routine for Instance Provider. */

CMPIStatus TestCMPIBrokerProviderModifyInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci,
    const char **properties)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}


CMPIStatus TestCMPIBrokerProviderDeleteInstance(
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop)
{
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/* ExecQuery routine for Instance Provider. */

CMPIStatus TestCMPIBrokerProviderExecQuery(
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
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/


CMMethodMIStub (TestCMPIBrokerProvider,
    TestCMPIBrokerProvider,
    _broker,
    CMNoHook)


CMInstanceMIStub(
    TestCMPIBrokerProvider,
    TestCMPIBrokerProvider,
    _broker,
    CMNoHook);

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
