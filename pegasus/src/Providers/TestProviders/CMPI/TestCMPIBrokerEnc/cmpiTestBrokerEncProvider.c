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

#define _ClassName "TestCMPI_BrokerEnc"
#define _Namespace    "test/TestProvider"

#define _ProviderLocation "/src/Providers/TestProviders/CMPI/" \
    "TestCMPIBrokerEnc/tests/"

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
// To increase the coverage in CMPIBrokerEnc.cpp
static int _testBrokerEnc (const CMPIContext * ctx,
    const CMPIResult * rslt)
{
    int flag = 1 ;
    char* illegal;
    char path[100];
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIString *type;
    CMPIString *cmpiStr;
    CMPIBoolean bol=0;
    CMPIArray* cmpiArray = NULL ;
    CMPIInstance* instance = NULL;
    CMPIInstance* instance1 = NULL;
    CMPIInstance* instanceErr = NULL;
    CMPIObjectPath* objPath = NULL;
    CMPIObjectPath* objPath1 = NULL;
    CMPICount cmpiCnt = 5;
    CMPIType cmpiType = CMPI_uint64;
    CMPIDateTime* cmpiDateTime = NULL ;
    CMPIMsgFileHandle msgFileHandle;
    CMPIEnumeration* cmpiEnum;
    CMPISelectExp* selExp;
    CMPIValue val;
    CMPIError* err;
    CMPIStatus* rc1 = NULL;
    CMPIArgs* args = NULL;
    void* handle;
    CMPIInstanceFT* funcTable;

    PROV_LOG ("++++ _testBrokerEnc ");

    PROV_LOG ("++++ ObjectPath testing");

    objPath1 = CMNewObjectPath (_broker, NULL, _ClassName, &rc);
    PROV_LOG ("++++ New object path with NULL namespace creation : (%s)",
        strCMPIStatus (rc));
    CMRelease(objPath1);

    objPath1 = CMNewObjectPath (_broker, _Namespace, "abc", &rc);
    PROV_LOG ("++++ New object path with wrong classname : (%s)",
        strCMPIStatus (rc));

    instance = CMNewInstance(_broker, NULL, &rc);
    PROV_LOG ("++++ New Instance with NULL object path : (%s)",
        strCMPIStatus (rc));

    handle = objPath1->hdl;
    objPath1->hdl=NULL;
    instance = CMNewInstance(_broker, objPath1, &rc);
    PROV_LOG ("++++ New Instance with object path handle set to NULL: (%s)",
        strCMPIStatus (rc));
    objPath1->hdl = handle;

    objPath = make_ObjectPath(_broker, _Namespace, _ClassName);
    instance = CMNewInstance(_broker, objPath, &rc);
    PROV_LOG ("++++ New Instance creation status: (%s)",
        strCMPIStatus (rc));
    instance1 = CMClone(instance, &rc);
    instanceErr = CMClone(instance, &rc);

    cmpiStr = CMNewString(_broker, NULL, &rc);
    PROV_LOG ("++++ Status of mbNewString with NULL parameter:(%s)",
        strCMPIStatus (rc));

    err = CMNewCMPIError(_broker, "abc", "abc", "abc", 4, 5, 6, rc1);
    if (err)
    {
        PROV_LOG ("++++ CMNewCMPIError called with null rc");
    }
    cmpiDateTime = CMNewDateTimeFromChars(_broker, "datetime", &rc);
    PROV_LOG ("++++ Status of CMNewDateTimeFromChars with junk parameter:(%s)",
        strCMPIStatus (rc));

    if (!CMClassPathIsA(_broker, NULL, NULL, &rc))
    {
        PROV_LOG ("++++ Error Status of CMClassPathIsA:(%s)",
            strCMPIStatus (rc));
    }

    if (!CMClassPathIsA(_broker, objPath, NULL, &rc))
    {
        PROV_LOG ("++++ Error Status of CMClassPathIsA:(%s)",
            strCMPIStatus (rc));
    }
    illegal = "$";
    if (!CMClassPathIsA(_broker, objPath, illegal, &rc))
    {
        PROV_LOG ("++++ Error Status of CMClassPathIsA:(%s)",
            strCMPIStatus (rc));
    }


    rc = CMLogMessage(_broker,
        2,
        "TestProvider",
        "Testing Severity 2 in TestCMPIBrokerEnc",
        NULL);
    PROV_LOG ("++++ Status of CMLogMessage with Severity 2:(%s)",
        strCMPIStatus (rc));

    rc = CMLogMessage(_broker,
        3,
        "TestProvider",
        "Testing Severity 3 in TestCMPIBrokerEnc",
        NULL);
    PROV_LOG ("++++ Status of CMLogMessage with Severity 3:(%s)",
        strCMPIStatus (rc));

    rc = CMLogMessage(_broker,
        4,
        "TestProvider",
        "Testing Severity 4 in TestCMPIBrokerEnc",
        NULL);
    PROV_LOG ("++++ Status of CMLogMessage with Severity 4:(%s)",
        strCMPIStatus (rc));

    rc = CMTraceMessage(_broker,
        2,
        "CMPIProvider",
        "Tracing for level 2",
        NULL);
    PROV_LOG ("++++ Status of CMTraceMessage with level 2:(%s)",
        strCMPIStatus (rc));

    rc = CMTraceMessage(_broker,
        3,
        "CMPIProvider",
        "Tracing for level 3",
        NULL);
    PROV_LOG ("++++ Status of CMTraceMessage with level 3:(%s)",
        strCMPIStatus (rc));

    rc = CMTraceMessage(_broker,
        4,
        "CMPIProvider",
        "Tracing for level 4",
        NULL);
    PROV_LOG ("++++ Status of CMTraceMessage with level 4:(%s)",
        strCMPIStatus (rc));

    cmpiStr = CMNewString(_broker, "Tracing for level 5", &rc);
    rc = CMTraceMessage(_broker,
        5,
        "CMPIProvider",
        NULL,
        cmpiStr);
    PROV_LOG ("++++ Status of CMTraceMessage with level 5:(%s)",
        strCMPIStatus (rc));
    CMRelease(cmpiStr);

    PROV_LOG ("++++ Calling mbEncToString function ");

    type = CDToString (_broker, _broker, &rc);
    PROV_LOG ("++++ Error status of mbEncToString with wrong object path:"
        " (%s)",
        strCMPIStatus (rc));

    type = CDToString (_broker, NULL, &rc);
    PROV_LOG ("++++ Error status of mbEncToString with NULL parameter:"
        " (%s)",
        strCMPIStatus (rc));

    type = CDToString (_broker, type, &rc);
    PROV_LOG ("++++ Passing CMPIString to mbEncToString successes : (%s)",
        strCMPIStatus (rc));

    type = CDToString (_broker, objPath, &rc);
    PROV_LOG ("++++ Passing CMPIObjectPath to mbEncToString successes : (%s)",
        strCMPIStatus (rc));
    handle = objPath1->hdl;
    objPath1->hdl = NULL;
    type = CDToString (_broker, objPath1, &rc);
    if ( rc.rc == CMPI_RC_ERR_INVALID_PARAMETER)
    {
        PROV_LOG ("++++ Error status of mbEncToString with object path with "
            "NULL handle: (%s)",
            strCMPIStatus (rc));
    }
    objPath1->hdl = handle;

    val.boolean = 1;
    rc = CMSetProperty (instance, "Boolean", &val, CMPI_boolean);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_boolean : (%s)",
        strCMPIStatus (rc));

    val.uint8 = 8;
    rc = CMSetProperty (instance, "Uint8", &val, CMPI_uint8);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_uint8 : (%s)",
        strCMPIStatus (rc));

    val.uint16 = 16;
    rc = CMSetProperty (instance, "Uint16", &val, CMPI_uint16);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_uint16 : (%s)",
        strCMPIStatus (rc));

    val.uint32 = 32;
    rc = CMSetProperty (instance, "Uint32", &val, CMPI_uint32);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_uint32 : (%s)",
        strCMPIStatus (rc));

    val.uint64 = 64;
    rc = CMSetProperty (instance, "Uint64", &val, CMPI_uint64);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_uint64 : (%s)",
        strCMPIStatus (rc));

    val.sint8 = -8;
    rc = CMSetProperty (instance, "Sint8", &val, CMPI_sint8);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_sint8 : (%s)",
        strCMPIStatus (rc));

    val.sint16 = -16;
    rc = CMSetProperty (instance, "Sint16", &val, CMPI_sint16);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_sint16 : (%s)",
        strCMPIStatus (rc));

    val.sint32 = -32;
    rc = CMSetProperty (instance, "Sint32", &val, CMPI_sint32);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_sint32 : (%s)",
        strCMPIStatus (rc));

    val.sint64 = -64;
    rc = CMSetProperty (instance, "Sint64", &val, CMPI_sint64);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_sint64 : (%s)",
        strCMPIStatus (rc));

    val.real32 = 32.32f;
    rc = CMSetProperty (instance, "Real32", &val, CMPI_real32);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_real32 : (%s)",
        strCMPIStatus (rc));

    val.real64 = 64.64;
    rc = CMSetProperty (instance, "Real64", &val, CMPI_real64);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_real64 : (%s)",
        strCMPIStatus (rc));

    val.char16 = 'c';
    rc = CMSetProperty (instance, "Char16", &val, CMPI_char16);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_char16 : (%s)",
        strCMPIStatus (rc));

    cmpiDateTime = CMNewDateTime (_broker, &rc);
    val.dateTime = cmpiDateTime;
    rc = CMSetProperty (instance, "DateTime", &val, CMPI_dateTime);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_dateTime : (%s)",
        strCMPIStatus (rc));

    val.ref = objPath;
    rc = CMSetProperty (instance, "Ref", &val, CMPI_ref);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_ref : (%s)",
        strCMPIStatus (rc));

    val.inst = instance1;
    rc = CMSetProperty (instance, "Instance", &val, CMPI_instance);
    PROV_LOG ("++++ Status of CMSetProperty of type CMPI_instance : (%s)",
        strCMPIStatus (rc));

    type = CDToString (_broker, instance, &rc);
    PROV_LOG ("++++ Passing CMPIInstance to mbEncToString successes : (%s)",
        strCMPIStatus (rc));

    args = CMNewArgs(_broker, &rc);
    PROV_LOG ("++++ Status of CMNewArgs : (%s)", strCMPIStatus (rc));
    type = CDToString (_broker, args, &rc);
    PROV_LOG ("++++ Passing CMPIArgs to mbEncToString successes : (%s)",
        strCMPIStatus (rc));

    PROV_LOG ("++++ Calling mbEncGetType function  ");
    rc1 = NULL;
    type = CDGetType (_broker, NULL, rc1);
    if (!type)
    {
        PROV_LOG ("++++ Error calling mbEncGetType with NULL rc");
    }

    rc1 = NULL;
    funcTable = instanceErr->ft;
    instanceErr->ft = NULL;
    type = CDGetType (_broker, instanceErr, rc1);
    if (!type)
    {
        PROV_LOG ("++++ Error calling mbEncGetType with wrong parameter and"
            " NULL rc");
    }
    type = CDGetType (_broker, instanceErr, &rc);
    PROV_LOG ("++++ Error status of mbEncGetType with wrong input : (%s)",
        strCMPIStatus (rc));

    instanceErr->ft = funcTable;

    type = CDGetType (_broker, NULL, &rc);
    PROV_LOG ("++++ Error status of mbEncGetType with NULL input : (%s)",
        strCMPIStatus (rc));

    type = CDGetType (_broker, instance, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIInstance"
        " : (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CDGetType (_broker, objPath, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIObjectPath:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    cmpiStr = CMNewString(_broker, "Test1", & rc);
    type = CDGetType (_broker, cmpiStr, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIString:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CDGetType (_broker, _broker, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIBroker:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    cmpiArray = CMNewArray(_broker, cmpiCnt,cmpiType, &rc);
    type = CDGetType (_broker, cmpiArray, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIArray:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CDGetType (_broker, cmpiDateTime, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIDateTime:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));
    type = CDGetType (_broker, ctx, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIContext:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CDGetType (_broker, rslt, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIResult:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    rc1 = NULL;
    selExp = CMNewSelectExp(_broker,
        "SELECT c,b FROM TestCMPI_Indication WHERE c='c' OR b=1",
        "WQL",
        NULL,
        rc1);
    PROV_LOG ("++++ CMNewSelectExp with rc set to NULL Called");

    selExp = CMNewSelectExp(_broker,
        "SELECT c,b FROM TestCMPI_Indication",
        "WQL",
        NULL,
        &rc);
    PROV_LOG ("++++ Status of CMNewSelectExp: (%s) ",
        strCMPIStatus (rc));
    type = CDGetType (_broker, selExp, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPISelectExp:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    handle = objPath1->hdl;
    objPath1->hdl = NULL;
    rc = CMRelease(objPath1);
    PROV_LOG ("++++ Status of CMRelease(objPath1): (%s)",
        strCMPIStatus (rc));
    objPath1->hdl = handle;
    rc = CMRelease(objPath1);
    PROV_LOG ("++++ Status of CMRelease(objPath1): (%s)",
        strCMPIStatus (rc));

    objPath1 = CMNewObjectPath(_broker,
        "test/TestProvider",
        "TestCMPI_Instance",
        &rc);
    PROV_LOG ("++++ Status of CMNewObjectPath for TestCMPI_Instance : (%s)",
        strCMPIStatus (rc));

    PROV_LOG_CLOSE();
    cmpiEnum = CBEnumInstances(_broker, ctx, objPath1, NULL, &rc);
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);

    PROV_LOG ("++++ Status of CBEnumInstances : (%s)",
        strCMPIStatus (rc));
    type = CDGetType (_broker, cmpiEnum, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIEnumeration:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CDGetType (_broker, args, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIArgs:"
        " (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));


    PROV_LOG ("++++ Calling mbEncIsOfType function ");
    bol = CDIsOfType (_broker, NULL, "No object", rc1);
    if(!bol)
    {
        PROV_LOG ("++++Calling mbEncIsOfType with NULL rc and wrong"
            " parameters");
    }
    bol = CDIsOfType (_broker, instance, "CMPIInstance", rc1);
    if(!bol)
    {
        PROV_LOG ("++++ Calling mbEncIsOfType with NULL rc");
    }
    bol = CDIsOfType (_broker, NULL, "No object", &rc);
    PROV_LOG ("++++ Error Status of mbEncIsOfType : (%s)", strCMPIStatus (rc));

    bol = CDIsOfType (_broker, instance, "CMPIInstance", &rc);
    if ( bol )
    {
        PROV_LOG ("++++ CDIsOfType for CMPIInstance status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, instance, "CMPI", &rc);
    if ( !bol )
    {
        PROV_LOG ("++++ CDIsOfType for CMPIInstance status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, objPath, "CMPIObjectPath", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIObjectPath status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, objPath, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIObjectPath status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, cmpiDateTime, "CMPIDateTime", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIDateTime status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, cmpiDateTime, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIDateTime status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, cmpiStr, "CMPIString", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIString status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, cmpiStr, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIString status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, _broker, "CMPIBroker", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIBroker status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, _broker, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIBroker status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }


    bol = CDIsOfType (_broker, cmpiArray, "CMPIArray", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIArray status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, cmpiArray, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIArray status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, rslt, "CMPIResult", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIResult status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, rslt, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIResult status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, ctx, "CMPIContext", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIContext status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }
    bol = CDIsOfType (_broker, ctx, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIContext status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, cmpiEnum, "CMPIEnumeration", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIEnumeration status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, cmpiEnum, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIEnumeration status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    /* Testcase to increase coverage in CMPI_Enumeration.cpp*/
    rc = CMRelease(cmpiEnum);
    PROV_LOG ("++++ Status of CMRelease on cmpiEnum :(%s)",
        strCMPIStatus (rc));

    /**********************************************************/

    bol = CDIsOfType (_broker, selExp, "CMPISelectExp", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPISelectExp status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, selExp, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPISelectExp status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, args, "CMPIArgs", &rc);
    if (bol)
    {
        PROV_LOG ("++++ CDIsOfType for CMPIArgs status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    bol = CDIsOfType (_broker, args, "CMPI", &rc);
    if (!bol)
    {
        PROV_LOG ("++++ CdIsOfType for CMPIArgs status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    type = CMGetMessage (_broker,
        "msgid",
        "Test 0 arguments",
        &rc,
        0);
    PROV_LOG ("++++ Status of GetMessage with 0 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0",
        &rc,
        CMFmtArgs1(CMFmtChars("one argument")));
    PROV_LOG ("++++ Status of GetMessage with 1 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0 $1",
        &rc,
        CMFmtArgs2(CMFmtChars("Sint32 ::"),
            CMFmtSint(-1)));
    PROV_LOG ("++++ Status of GetMessage with 2 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0 $1, $2",
        &rc,
        CMFmtArgs3(CMFmtChars("Sint32, Uint32 ::"),
            CMFmtSint(-1),
            CMFmtUint(1)));
    PROV_LOG ("++++ Status of GetMessage with 3 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0 $1, $2, $3",
        &rc,
        CMFmtArgs4(CMFmtChars("Sint32, Uint32, Sint64 ::"),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64)));
    PROV_LOG ("++++ Status of GetMessage with 4 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0 $1, $2, $3, $4",
        &rc,
        CMFmtArgs5(CMFmtChars("Sint32, Uint32, Sint64, Uint64 ::"),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64)));
    PROV_LOG ("++++ Status of GetMessage with 5 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0 $1, $2, $3, $4, $5",
        &rc,
        CMFmtArgs6(CMFmtChars("Sint32, Uint32, Sint64, Uint64, Real64 ::"),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64),
            CMFmtReal(64.64)));
    PROV_LOG ("++++ Status of GetMessage with 6 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0 $1, $2, $3, $4, $5, $6",
        &rc,
        CMFmtArgs7(CMFmtChars("Sint32, Uint32, Sint64, Uint64, Real64,"
            " Boolean:: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64),
            CMFmtReal(64.64),
            CMFmtBoolean(1)));
    PROV_LOG ("++++ Status of GetMessage with 7 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0 $1, $2, $3, $4, $5, $6, $7",
        &rc,
        CMFmtArgs8(CMFmtChars("Sint32, Uint32, Sint64, Uint64, Real64,"
            " Boolean, chars:: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64),
            CMFmtReal(64.64),
            CMFmtBoolean(1),
            CMFmtChars("chars")));
    PROV_LOG ("++++ Status of GetMessage with 8 arguments(%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage (_broker,
        "msgid",
        "Test $0 $1, $2, $3, $4, $5, $6, $7, $8",
        &rc,
        CMFmtArgs9(CMFmtChars("Sint32, Uint32, Sint64, Uint64, Real64,"
            " Boolean, chars, String :: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64),
            CMFmtReal(64.64),
            CMFmtBoolean(1),
            CMFmtChars("chars"),
            CMFmtString(cmpiStr)));
    PROV_LOG ("++++ Status of GetMessage with 9 arguments (%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    strcpy(path,"path");
    rc = CMOpenMessageFile(_broker, path, &msgFileHandle);
    PROV_LOG ("++++ Status of CMOpenMessageFile (%s) ",
        strCMPIStatus (rc));

    type = CMGetMessage2(_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test zero arguments",
        &rc,
        0);
    PROV_LOG ("++++ Status of GetMessage2 with 0 argument (%s) type (%s)",
            strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2(_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0",
        &rc,
        CMFmtArgs1(CMFmtChars("Single argument")));
    PROV_LOG ("++++ Status of GetMessage2 with 1 argument (%s) type (%s)",
            strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2(_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0 $1",
        &rc,
        CMFmtArgs2(CMFmtChars("Sint32 ::"),CMFmtSint(1)));
    PROV_LOG ("++++ Status of GetMessage2 with 2 arguments (%s) type (%s)",
            strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2 (_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0 $1, $2",
        &rc,
        CMFmtArgs3(CMFmtChars("Sint32, Uint32 :: "),
            CMFmtSint(-1),
            CMFmtUint(1)));
    PROV_LOG ("++++ Status of GetMessage2 with 3 arguments (%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2 (_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0 $1, $2, $3",
        &rc,
        CMFmtArgs4(CMFmtChars("Sint32, Uint32, Sint64 :: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64)));
    PROV_LOG ("++++ Status of GetMessage2 with 4 arguments (%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2 (_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0 $1, $2, $3, $4",
        &rc,
        CMFmtArgs5(CMFmtChars("Sint32, Uint32, Sint64, Uint64 :: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64)));
    PROV_LOG ("++++ Status of GetMessage2 with 5 arguments (%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2 (_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0 $1, $2, $3, $4, $5",
        &rc,
        CMFmtArgs6(CMFmtChars("Sint32, Uint32, Sint64, Uint64, Real64 :: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64),
            CMFmtReal(64.64)));
    PROV_LOG ("++++ Status of GetMessage2 with 6 arguments (%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2 (_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0 $1, $2, $3, $4, $5, $6",
        &rc,
        CMFmtArgs7(CMFmtChars("Sint32, Uint32, Sint64, Uint64, Real64,"
            " Boolean :: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64),
            CMFmtReal(64.64),
            CMFmtBoolean(1)));
    PROV_LOG ("++++ Status of GetMessage2 with 7 arguments (%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2 (_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0 $1, $2, $3, $4, $5, $6, $7",
        &rc,
        CMFmtArgs8(CMFmtChars("Sint32, Uint32, Sint64, Uint64, Real64,"
            " Boolean, chars :: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64),
            CMFmtReal(64.64),
            CMFmtBoolean(1),
            CMFmtChars("chars")));
    PROV_LOG ("++++ Status of GetMessage2 with 8 arguments (%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    type = CMGetMessage2 (_broker,
        "Common.CIMStatusCode.CIM_ERR_SUCCESS",
        msgFileHandle,
        "Test $0 $1, $2, $3, $4, $5, $6, $7, $8",
        &rc,
        CMFmtArgs9(CMFmtChars("Sint32, Uint32, Sint64, Uint64, Real64,"
            " Boolean, chars, String :: "),
            CMFmtSint(-1),
            CMFmtUint(1),
            CMFmtSint64(-64),
            CMFmtUint64(64),
            CMFmtReal(64.64),
            CMFmtBoolean(1),
            CMFmtChars("chars"),
            CMFmtString(cmpiStr)));
    PROV_LOG ("++++ Status of GetMessage2 with 9 arguments (%s) type (%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    rc = CMCloseMessageFile(_broker, msgFileHandle);
    PROV_LOG ("++++ Status of CMCloseMessageFile (%s) ",
        strCMPIStatus (rc));

    CMRelease(instance1);
    CMRelease(instanceErr);
    return flag ;


}

/* ---------------------------------------------------------------------------*/
/*                        Method Provider Interface                           */
/* ---------------------------------------------------------------------------*/
CMPIStatus TestCMPIBrokerEncProviderMethodCleanup (CMPIMethodMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIBrokerEncProviderInvokeMethod (CMPIMethodMI * mi,
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

    CMPIUint32 oper_rc = 1;

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
        if (strncmp("testBrokerEnc",
            methodName,
            strlen ("testBrokerEnc"))== 0)
        {
            oper_rc = _testBrokerEnc(ctx, rslt);
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

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/


CMMethodMIStub (TestCMPIBrokerEncProvider,
    TestCMPIBrokerEncProvider,
    _broker,
    CMNoHook)

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
