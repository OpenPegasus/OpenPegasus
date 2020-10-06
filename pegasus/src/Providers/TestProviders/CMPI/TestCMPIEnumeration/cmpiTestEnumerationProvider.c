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

#define _ClassName "TestCMPI_Enumeration"
#define _Namespace    "test/TestProvider"
#define _PersonClass  "CMPI_TEST_Person"

#define _ProviderLocation "/src/Providers/TestProviders/CMPI/" \
    "TestCMPIEnumeration/tests/"

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

// To increase the coverage in CMPIEnumeration.cpp
static int _testEnumeration (const CMPIContext * ctx,
    const CMPIResult * rslt)
{

    CMPIStatus rc = { CMPI_RC_OK, NULL };

    const char *properties_for_assoc[] = {"name", NULL};
    unsigned int count = 0;
    int flag = 1;

    //Enumeration Variables
    CMPIEnumeration* testEnumerationForAssociators = NULL;
    CMPIEnumeration* testEnumerationForAssociatorsClone = NULL;
    CMPIEnumeration* testEnumerationForAssociatorNames = NULL;
    CMPIEnumeration* testEnumerationForAssociatorNamesClone = NULL;
    CMPIEnumeration* enum_ptr = NULL;

    //data Arrays to store the values
    CMPIData data[10];
    CMPIData dataAN[10];
    CMPIData dataErr;

    //ObjectPaths for Associator-related Functions
    CMPIObjectPath* opForAssociatorFunctions = NULL;
    CMPIObjectPath* opAssociators = NULL;
    CMPIObjectPath* opAssociatorNames = NULL;

    CMPIBoolean bol = 0;

    //String Variables
    CMPIString* objPathAssociators = NULL;
    CMPIString* objPathAssociatorNames = NULL;
    CMPIString* type;

    CMPIArray* array = NULL;

    const char* getInstanceStringAssociators = NULL;
    const char* getInstanceStringAssociatorNames = NULL;

    PROV_LOG("++++ _testCMPIBroker" );
    //Getting ObjectPaths for different Classes
    opForAssociatorFunctions = make_ObjectPath(_broker,
        _Namespace,
        _PersonClass);

    //==============================CBAssociators==============================

    PROV_LOG("++++ Associators-Broker : CBAssociators");

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testEnumerationForAssociators = CBAssociators( _broker,
        ctx,
        opForAssociatorFunctions,
        "CMPI_TEST_Racing",
        NULL,
        NULL,
        NULL,
        properties_for_assoc,
        &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++  CBAssociators : (rc:%s)", strCMPIStatus (rc));

    while(CMHasNext(testEnumerationForAssociators, &rc))
    {
        data[count] = CMGetNext(testEnumerationForAssociators, &rc);
        PROV_LOG("++++  CMGetNext : (rc:%s)", strCMPIStatus (rc));
        opAssociators = CMGetObjectPath(data[count].value.inst, &rc);
        PROV_LOG("++++  CMGetObjectPath : (rc:%s)", strCMPIStatus (rc));
        objPathAssociators = CMObjectPathToString(opAssociators, &rc);
        PROV_LOG("++++  CMObjectPathToString : (rc:%s)", strCMPIStatus (rc));
        getInstanceStringAssociators = CMGetCharsPtr(objPathAssociators,&rc);
        PROV_LOG("++++  CMGetCharsPtr : (rc:%s)", strCMPIStatus (rc));
        count++;
    }

    PROV_LOG("++++ Number of Associators : %d)", count);

    if (count == 2)
    {
        PROV_LOG("++++ Associators success status : %d",
            flag);
    }
    else
    {
        flag = 0;
    }
    dataErr = CMGetNext(testEnumerationForAssociators, &rc);
    PROV_LOG("++++Error status of CMGetNext with CMPI_ObjEnumeration_Ftab:"
        " (rc:%s)", strCMPIStatus (rc));

    testEnumerationForAssociatorsClone = CMClone(testEnumerationForAssociators,
        &rc);
    PROV_LOG("++++  CMClone for testEnumerationForAssociators : (rc:%s)",
        strCMPIStatus (rc));
    rc = CMRelease(testEnumerationForAssociatorsClone);

    type = CDGetType (_broker, testEnumerationForAssociators, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIEnumeration "
        "with CMPI_ObjEnumeration_Ftab: (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, &rc));

    bol = CDIsOfType (_broker,
        testEnumerationForAssociators,
        "CMPIEnumeration",
        &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIEnumeration with"
            " CMPI_ObjEnumeration_Ftab status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    rc = CMRelease(testEnumerationForAssociators);
    PROV_LOG ("++++ CMRelease for testEnumerationForAssociators(%s)",
        strCMPIStatus (rc));

    //============================CBAssociatorNames============================

    PROV_LOG("++++ Associators-Broker : CBAssociatorNames");

    // Close log file before making upcall to provider that uses same PROV_LOG
    // functions.
    PROV_LOG_CLOSE();

    testEnumerationForAssociatorNames = CBAssociatorNames(_broker,
        ctx,
        opForAssociatorFunctions,
        "CMPI_TEST_Racing",
        NULL,
        NULL,
        NULL,
        &rc);

    // Reopen our log file.
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG("++++  CBAssociatorNames : (rc:%s)", strCMPIStatus (rc));
    count = 0;
    while(CMHasNext(testEnumerationForAssociatorNames, &rc))
    {
        dataAN[count] = CMGetNext(testEnumerationForAssociatorNames, &rc);
        PROV_LOG("++++  CMGetNext : (rc:%s)", strCMPIStatus (rc));
        opAssociatorNames = dataAN[count].value.ref;
        objPathAssociatorNames = CMObjectPathToString(opAssociatorNames, &rc);
        PROV_LOG("++++  CMObjectPathToString : (rc:%s)", strCMPIStatus (rc));
        getInstanceStringAssociatorNames = CMGetCharsPtr(
            objPathAssociatorNames,&rc);
        PROV_LOG("++++  CMGetCharsPtr : (rc:%s)", strCMPIStatus (rc));
        count++;
    }

    PROV_LOG("++++ Number of AssociatorNames : %d)", count);
    if (count == 2)
    {
        flag = 1;
        PROV_LOG("++++ AssociatorNames success status : %d",
            flag);
    }
    else
    {
        flag = 0;
    }
    dataErr = CMGetNext(testEnumerationForAssociatorNames, &rc);
    PROV_LOG("++++ Error status of CMGetNext for CMPI_OpEnumeration: (rc:%s)",
        strCMPIStatus (rc));

    testEnumerationForAssociatorNamesClone =
        CMClone(testEnumerationForAssociatorNames,
            &rc);
    PROV_LOG("++++  CMClone for testEnumerationForAssociatorNames : (rc:%s)",
        strCMPIStatus (rc));

    type = CDGetType (_broker, testEnumerationForAssociatorNames, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIEnumeration "
        "with CMPI_OpEnumeration_Ftab: (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, &rc));

    bol = CDIsOfType (_broker,
        testEnumerationForAssociatorNames,
        "CMPIEnumeration",
        &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIEnumeration with"
            " CMPI_OpEnumeration_Ftab status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    array = CMToArray(testEnumerationForAssociatorNamesClone, &rc);
    PROV_LOG("++++  CMToArray for testEnumerationForAssociatorNamesClone"
        ": (rc:%s)",
        strCMPIStatus (rc));

    rc = CMRelease(testEnumerationForAssociatorNamesClone);
    PROV_LOG ("++++ CMRelease for testEnumerationForAssociatorNamesClone (%s)",
        strCMPIStatus (rc));

    rc = CMRelease(testEnumerationForAssociatorNames);
    PROV_LOG ("++++ CMRelease for testEnumerationForAssociatorNames (%s)",
        strCMPIStatus (rc));

    enum_ptr = CBEnumInstances(_broker,
        ctx,
        opForAssociatorFunctions,
        NULL,
        &rc);
    PROV_LOG ("++++  CBEnumInstances : (%s)", strCMPIStatus (rc));

    count = 0;
    while (CMHasNext(enum_ptr, &rc))
    {
        PROV_LOG("++++ Calling CMGetNext");
        dataErr = CMGetNext(enum_ptr, &rc);
        PROV_LOG ("++++  CMGetNext : (%s)", strCMPIStatus (rc));
        count++;
    }
    PROV_LOG("++++ Enum count %d ", count);
    dataErr = CMGetNext(enum_ptr, &rc);
    PROV_LOG("++++ Error status of CMGetNext for CMPI_InstEnumeration_Ftab:"
        " (rc:%s)", strCMPIStatus (rc));
    return flag;
}

/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/
CMPIStatus TestCMPIEnumerationProviderMethodCleanup (CMPIMethodMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIEnumerationProviderInvokeMethod (CMPIMethodMI * mi,
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

    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

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
        if (strncmp("testEnumeration",
            methodName,
            strlen ("testEnumeration"))== 0)
        {
            value.uint32 = _testEnumeration(ctx, rslt);
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


CMMethodMIStub (TestCMPIEnumerationProvider,
    TestCMPIEnumerationProvider,
    _broker,
    CMNoHook)

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
