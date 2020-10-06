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


#define _ClassName "TestCMPIInstance_Method"
#define _Namespace    "test/TestProvider"

#define _ProviderLocation "/src/Providers/TestProviders/CMPI/" \
    "TestCMPIInstance/tests/"

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

CMPIInstance * make_Instance (const CMPIObjectPath * op)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIInstance *ci = NULL;

    PROV_LOG ("++++ make_Instance: CMNewInstance");
    ci = CMNewInstance (_broker, op, &rc);
    PROV_LOG ("++++  CMNewInstance : (%s)", strCMPIStatus (rc));
    if (rc.rc == CMPI_RC_ERR_NOT_FOUND)
    {
        PROV_LOG (" ---- Class %s is not found in the %s namespace!",
            _ClassName, _Namespace);
        return NULL;
    }
    return ci;
}

// For Increasing the Coverage of CMPIInstance
static int _testInstance ()
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIInstance* instance = NULL;
    CMPIObjectPath* objPath = NULL;
    CMPIObjectPath* objPathFake = NULL;

    CMPIValue value;
    CMPIType type = CMPI_uint64;

    CMPIData retData;
    CMPIData returnedData1;

    const char* property_name = "s";
    const char* origin = "origin";

    void* handle;

    int flag = 1;

    value.uint64 = PEGASUS_UINT64_LITERAL(5000000000);

    PROV_LOG("++++ _testInstance");

    objPath = make_ObjectPath(_broker, _Namespace, "TestCMPI_Instance");
    instance = make_Instance(objPath);

    /* Test cases to increase coverage in instSetPropertyWithOrigin function */

    /* Setting n64 property in class TestCMPI_Instance. So origin has been
       set to TestCMPI_Instance.*/
    rc = CMSetPropertyWithOrigin(instance,
        "n64",
        &value,
        type,
        "TestCMPI_Instance");
    if ( rc.rc == CMPI_RC_OK )
    {
        PROV_LOG("++++  SetPropertyWithOrigin for n64 property status : (%s)",
            strCMPIStatus(rc));
    }

    /* CMGetProperty is called to verify the value of the property just being
       set. */
    retData = CMGetProperty(instance, "n64", &rc);

    if (retData.type == CMPI_uint64 &&
           retData.value.uint64 == PEGASUS_UINT64_LITERAL(5000000000))
    {
         PROV_LOG("++++  CMGetProperty for n64 property status : (%s),"
             "value (%"PEGASUS_64BIT_CONVERSION_WIDTH"u)",
             strCMPIStatus(rc),
             retData.value.uint64);
    }

    /* Setting  non-member property of type CMPI_ref*/
    type= CMPI_ref;
    value.ref = objPath;

    rc = CMSetPropertyWithOrigin(instance, "objPath", &value, type, origin);
    if ( rc.rc == CMPI_RC_OK )
    {
        PROV_LOG("++++  SetPropertyWithOrigin status : (%s)",strCMPIStatus(rc));
    }

    /* Setting non-memeber property of type other than CMPI_ref*/
    value.uint32 = 32;
    type = CMPI_uint32;
    rc = CMSetPropertyWithOrigin(instance, "n32", &value, type, NULL);
    if ( rc.rc == CMPI_RC_OK )
    {
        PROV_LOG("++++  SetPropertyWithOrigin status : (%s)",strCMPIStatus(rc));
    }

    /* Testing error path for instGetProperty with NULL property name*/
    returnedData1 = CMGetProperty(instance, NULL, &rc);
    if (rc.rc == CMPI_RC_ERR_INVALID_PARAMETER )
    {
        PROV_LOG("++++ CMGetProperty with NULL property name status: (%s) ",
            strCMPIStatus(rc));
    }
    /*-----------------------------------------------------------------------*/

    /* Test cases to cover error paths for instSetObjectPath and
       instSetPropertyFilter*/

    /* Testing error path by passing CMPIObjectPath with class name different
       from the classname for CMPIInstance in CMSetObjectPath*/

    objPathFake = CMNewObjectPath(_broker,
        _Namespace,
        "TestCMPIInstance_Method",
        NULL);
    rc = CMSetObjectPath(instance, objPathFake);
    if ( rc.rc == CMPI_RC_ERR_FAILED || rc.rc == CMPI_RC_ERR_NOT_SUPPORTED)
    {
        PROV_LOG("++++  Test for CMSetObjectPath with wrong input passed");
    }

    /* Testing error path by passing NULL to instSetObjectPath*/
    rc = CMSetObjectPath(instance, NULL);
    if ( rc.rc == CMPI_RC_ERR_INVALID_PARAMETER ||
        rc.rc == CMPI_RC_ERR_NOT_SUPPORTED)
    {
        PROV_LOG("++++  Test for CMSetObjectPath with NULL input passed ");
    }

    /* Testing error paths by setting handle to CMPIInstance to NULL and using
       that in calls to instSetObjectPath and instSetPropertyFilter*/
    handle = instance->hdl;
    instance->hdl = NULL ;
    rc = CMSetObjectPath(instance, NULL);
    if ( rc.rc == CMPI_RC_ERR_INVALID_HANDLE ||
        rc.rc == CMPI_RC_ERR_NOT_SUPPORTED)
    {
        PROV_LOG("++++  CMSetObjectPath with NULL handle for"
            " CMPIInstance passed");
    }
    PROV_LOG("++++ CMSetObjectPath : (%s) ",strCMPIStatus(rc));

    rc = CMSetPropertyFilter(instance,&property_name,NULL);
    if( rc.rc == CMPI_RC_ERR_INVALID_HANDLE )
    {
        PROV_LOG("++++  CMSetPropertyFilter with NULL handle for CMPIInstance"
            " status : (%s)", strCMPIStatus (rc));
    }
    instance->hdl = handle;
    /*-----------------------------------------------------------------------*/
    return flag;

} // _testInstance end


/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/
CMPIStatus TestCMPIInstanceProviderMethodCleanup (CMPIMethodMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIInstanceProviderInvokeMethod (CMPIMethodMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *methodName,
    const CMPIArgs * in,
    CMPIArgs * out)
{
    CMPIString *class = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    CMPIData data;

    CMPIString *argName = NULL;

    unsigned int arg_cnt = 0, index = 0;

    CMPIValue value;

    PROV_LOG_OPEN ("TestCMPI_Instance", _ProviderLocation);

    PROV_LOG ("--- %s CMPI InvokeMethod() called", _ClassName);

    class = CMGetClassName (ref, &rc);

    PROV_LOG ("InvokeMethod: checking for correct classname [%s]",
        CMGetCharsPtr (class,NULL));

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
        CMGetCharsPtr (class,NULL),
        _ClassName,
        strlen (_ClassName)) == 0)
    {
        if (strncmp("testInstance",
            methodName,
            strlen ("testInstance"))== 0)
        {
            value.uint32 = _testInstance();
            CMReturnData (rslt, &value, CMPI_uint32);
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

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/


CMMethodMIStub (TestCMPIInstanceProvider,
    TestCMPIInstanceProvider,
    _broker,
    CMNoHook)

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
