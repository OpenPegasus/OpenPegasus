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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Providers/TestProviders/CMPI/TestUtilLib/cmpiUtilLib.h>

#define _ClassName "TestCMPI_BrokerInstance"
#define _ClassName_size strlen(_ClassName)
#define _Namespace    "test/TestProvider"
#define _ProviderLocation "/src/Providers/TestProviders/CMPI/" \
    "TestCMPIBroker/tests/"

static const CMPIBroker *_broker;
static CMPIInstance* _inst;

/* ---------------------------------------------------------------------------*/
/*                       CMPI Helper function                        */
/* ---------------------------------------------------------------------------*/

int _setProperty1 (CMPIInstance * ci, const char *p)
{
    CMPIValue val;
    const char *property;
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    property = p;
    PROV_LOG ("--- _setProperty: %s -> %s", p, property);
    if ((strncmp (property, "ElementName", 11) == 0)
        && (strlen (property) == 11))
    {
        rc = CMSetProperty (ci,
            "ElementName",
            (CMPIValue *) _ClassName,
            CMPI_chars);
    }
    else if ((strncmp (property, "s", 1) == 0) && (strlen (property) == 1))
    {
        rc = CMSetProperty (ci, "s", (CMPIValue *) "s", CMPI_chars);
    }
    else if ((strncmp (property, "c", 1) == 0) && (strlen (property) == 1))
    {
        rc = CMSetProperty (ci, "c", (CMPIValue *) "c", CMPI_char16);
    }
    else if ((strncmp (property, "n32", 3) == 0) && (strlen (property) == 3))
    {
        val.uint32 = 32;
        rc = CMSetProperty (ci, "n32", &val, CMPI_uint32);
    }
    else if ((strncmp (property, "n64", 3) == 0) && (strlen (property) == 3))
    {
        val.uint64 = 64;
        rc = CMSetProperty (ci, "n64", &val, CMPI_uint64);
    }
    else if ((strncmp (property, "n16", 3) == 0) && (strlen (property) == 3))
    {
        val.uint16 = 16;
        rc = CMSetProperty (ci, "n16", &val, CMPI_uint16);
    }
    else if ((strncmp (property, "n8", 2) == 0) && (strlen (property) == 2))
    {
        val.uint8 = 8;
        rc = CMSetProperty (ci, "n8", &val, CMPI_uint8);
    }
    else if ((strncmp (property, "r32", 3) == 0) && (strlen (property) == 3))
    {
        val.real32 = (CMPIReal32)1.23;
        rc = CMSetProperty (ci, "r32", &val, CMPI_real32);
    }
    else if ((strncmp (property, "r64", 3) == 0) && (strlen (property) == 3))
    {
        val.real64 = 3.1415678928283;
        rc = CMSetProperty (ci, "r64", &val, CMPI_real64);
    }
    else if ((strncmp (property, "s64", 3) == 0) && (strlen (property) == 3))
    {
        val.sint64 = 0xFFFFFFF;
        rc = CMSetProperty (ci, "s64", &val, CMPI_sint64);
    }
    else if ((strncmp (property, "s32", 3) == 0) && (strlen (property) == 3))
    {
        val.sint32 = 0xDEADBEEF;
        rc = CMSetProperty (ci, "s32", &val, CMPI_sint32);
    }
    else if ((strncmp (property, "s16", 3) == 0) && (strlen (property) == 3))
    {
        val.sint16 = (CMPISint16)0xFFFF;
        rc = CMSetProperty (ci, "s16", &val, CMPI_sint16);
    }
    else if ((strncmp (property, "s8", 2) == 0) && (strlen (property) == 2))
    {
        val.sint8 = (CMPISint8)0xFF;
        rc = CMSetProperty (ci, "s8", &val, CMPI_sint8);
    }
    else if ((strncmp (property, "b", 1) == 0) && (strlen (property) == 1))
    {
        val.boolean = 1;
        rc = CMSetProperty (ci, "b", &val, CMPI_boolean);
    }
    else if ((strncmp (property, "d", 1) == 0) && (strlen (property) == 1))
    {
        PROV_LOG ("---- CMNewDateTime");
        val.dateTime = CMNewDateTime (_broker, &rc);
        rc = CMSetProperty (ci, "d", &val, CMPI_dateTime);
    }
    else if ((strncmp (property, "*", 1) == 0) && (strlen (property) == 1))
    {
        PROV_LOG ("---- All properties");
        _setProperty1 (ci, "ElementName");
        _setProperty1 (ci, "s");
        _setProperty1 (ci, "c");
        _setProperty1 (ci, "n64");
        _setProperty1 (ci, "n32");
        _setProperty1 (ci, "n16");
        _setProperty1 (ci, "s64");
        _setProperty1 (ci, "s32");
        _setProperty1 (ci, "s16");
        _setProperty1 (ci, "r64");
        _setProperty1 (ci, "r32");
        _setProperty1 (ci, "d");
        _setProperty1 (ci, "b");
    }
    else
    {
        PROV_LOG ("---- No implementation for property: %s", property);
        return 1;
    }
    return 0;
}
/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/

CMPIStatus TestCMPIBrokerInstanceProviderCleanup (CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean term)
{
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIBrokerInstanceProviderEnumInstanceNames (CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref)
{
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);
    PROV_LOG ("Inside Enumerate Instance Names");
    PROV_LOG_CLOSE ();
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIBrokerInstanceProviderEnumInstances (CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char **properties)
{
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);
    PROV_LOG ("Inside Enumerate Instances");
    PROV_LOG_CLOSE();
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus TestCMPIBrokerInstanceProviderCreateInstance (CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci)
{
    const CMPIString* retNamespace;
    const CMPIString* retClassname;
    CMPIString* type;
    const char* str1;
    const char* str2;
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIData retData;
    CMPIObjectPath *obj;
    CMPIBoolean bol = 0;

    PROV_LOG_OPEN (_ClassName, _ProviderLocation);
    PROV_LOG("CreateInstance");

    retNamespace = CMGetNameSpace(cop, &rc);
    retClassname = CMGetClassName(cop, &rc);
    str1 = CMGetCharsPtr(retNamespace, &rc);
    str2 = CMGetCharsPtr(retClassname, &rc);
    PROV_LOG("++++Obtained Namespace : (%s) Class (%s)", str1, str2);

    if(ci)
    {
        /* Test to increase coverage in Broker_Enc.cpp*/
        type = CDToString (_broker, ci, &rc);
        PROV_LOG ("++++ Passing CMPIInstance with CMPI_InstanceOnStack_Ftab "
            "to mbEncToString successes : "
            "(%s)",
            strCMPIStatus (rc));

        type = CDGetType (_broker, ci, &rc);
        PROV_LOG ("++++ Status of mbEncGetType with input of type CMPIInstance"
            " : (%s) type(%s)",
            strCMPIStatus (rc),
            CMGetCharsPtr(type, NULL));

        bol = CDIsOfType (_broker, ci, "CMPIInstance", &rc);
        if ( bol )
        {
            PROV_LOG ("++++ CDIsOfType for CMPIInstance with "
                "CMPI_InstanceOnStack_Ftab status is (%s) : %d",
                strCMPIStatus (rc),
                bol);
        }

        type = CDToString (_broker, cop, &rc);
        PROV_LOG ("++++ Passing CMPIObjectPath with "
            "CMPI_ObjectPathOnStack_Ftab to mbEncToString successes : (%s)",
            strCMPIStatus (rc));

        type = CDGetType (_broker, cop, &rc);
        PROV_LOG ("++++ Status of mbEncGetType with input of type "
            "CMPIObjectPath : (%s) type(%s)",
            strCMPIStatus (rc),
            CMGetCharsPtr(type, NULL));

        bol = CDIsOfType (_broker, cop, "CMPIObjectPath", &rc);
        if ( bol )
        {
            PROV_LOG ("++++ CDIsOfType for CMPIObjectPath with "
                "CMPI_ObjectPathOnStack_Ftab status is (%s) : %d",
                strCMPIStatus (rc),
                bol);
        }

        type = CDGetType (_broker, rslt, &rc);
        PROV_LOG ("++++ Status of mbEncGetType with input of type "
            "CMPIResult with CMPI_ResultRefOnStack_Ftab : (%s) type(%s)",
            strCMPIStatus (rc),
            CMGetCharsPtr(type, NULL));

        bol = CDIsOfType (_broker, rslt, "CMPIResult", &rc);
        if ( bol )
        {
            PROV_LOG ("++++ CDIsOfType for CMPIResult with "
                "CMPI_ResultRefOnStack_Ftab status is (%s) : %d",
                strCMPIStatus (rc),
                bol);
        }

        _inst = CMClone(ci, &rc);
        PROV_LOG("++++ Status of CMClone(ci) : (%s)",
            strCMPIStatus(rc));
        rc = CMSetObjectPath (_inst, cop);
        PROV_LOG("++++ Status of CMSetObjectPath(_inst) : (%s)",
            strCMPIStatus(rc));
        _setProperty1(_inst, "*");
        retData = CMGetProperty(_inst, "n64", &rc);
        PROV_LOG("++++ Status of CMGetProperty : (%s)",
            strCMPIStatus(rc));
        PROV_LOG("n64 = %" PEGASUS_64BIT_CONVERSION_WIDTH "u",
            retData.value.uint64);
        retData = CMGetProperty(_inst, "n32", &rc);
        PROV_LOG("++++ Status of CMGetProperty : (%s)",
            strCMPIStatus(rc));
        PROV_LOG("n32 = %d", retData.value.uint32);

        obj = CMGetObjectPath(_inst, &rc);
        PROV_LOG("++++ Status of CMGetObjectPath : (%s)",
            strCMPIStatus(rc));
        retNamespace = CMGetNameSpace(obj, &rc);
        PROV_LOG("++++ Status of CMGetNameSpace : (%s)",
            strCMPIStatus(rc));
        retClassname = CMGetClassName(obj, &rc);
        PROV_LOG("++++ Status of CMGetClassName : (%s)",
            strCMPIStatus(rc));
        str1 = CMGetCharsPtr(retNamespace, &rc);
        PROV_LOG("++++ Status of CMGetCharsPtr : (%s)",
            strCMPIStatus(rc));
        str2 = CMGetCharsPtr(retClassname, &rc);
        PROV_LOG("++++ Status of CMGetCharsPtr : (%s)",
            strCMPIStatus(rc));
        PROV_LOG("++++Namespace (_inst): (%s) Class (*%s)", str1, str2);
        retData = CMGetProperty(_inst, "n64", &rc);
        PROV_LOG("++++ Status of CMGetProperty : (%s)",
            strCMPIStatus(rc));
        PROV_LOG("n64 = %" PEGASUS_64BIT_CONVERSION_WIDTH "u",
            retData.value.uint64);
        PROV_LOG_CLOSE ();

        CMReturnObjectPath(rslt, cop);
        CMReturnDone(rslt);
        CMReturn (CMPI_RC_OK);
    }
    else
    {
        PROV_LOG_CLOSE ();
        CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
    }
}

CMPIStatus TestCMPIBrokerInstanceProviderGetInstance (CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const char **properties)
{
    CMPIStatus rc = {CMPI_RC_OK, NULL};
    CMPIString* retNamespace;
    CMPIString* retClassname;
    CMPIString* type;
    CMPIData retData;
    CMPIObjectPath* obj;
    CMPIBoolean bol = 0;
    const char* str1;
    const char* str2;

    PROV_LOG_OPEN (_ClassName, _ProviderLocation);
    PROV_LOG("GetInstance");
    PROV_LOG("++++Namespace");

    obj = CMGetObjectPath(_inst, &rc);
    PROV_LOG("++++ Status of CMGetObjectPath : (%s)",
        strCMPIStatus(rc));
    retNamespace = CMGetNameSpace(obj, &rc);
    PROV_LOG("++++ Status of CMGetNameSpace : (%s)",
        strCMPIStatus(rc));
    retClassname = CMGetClassName(obj, &rc);
    PROV_LOG("++++ Status of CMGetClassName : (%s)",
        strCMPIStatus(rc));
    str1 = CMGetCharsPtr(retNamespace, &rc);
    PROV_LOG("++++ Status of CMGetCharsPtr : (%s)",
        strCMPIStatus(rc));
    str2 = CMGetCharsPtr(retClassname, &rc);
    PROV_LOG("++++ Status of CMGetCharsPtr : (%s)",
        strCMPIStatus(rc));
    retData = CMGetProperty(_inst, "n64", &rc);
    PROV_LOG("++++ Status of CMGetProperty : (%s)",
        strCMPIStatus(rc));
    PROV_LOG("n64 = %" PEGASUS_64BIT_CONVERSION_WIDTH "u",
        retData.value.uint64);

    /* Test cases for increasing coverage in CMPI_BrokerEnc.cpp*/
    type = CDGetType (_broker, rslt, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type "
        "CMPIResult with CMPI_ResultInstOnStack_Ftab : (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    bol = CDIsOfType (_broker, rslt, "CMPIResult", &rc);
    if ( bol )
    {
        PROV_LOG ("++++ CDIsOfType for CMPIResult with "
            "CMPI_ResultInstOnStack_Ftab status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }
    PROV_LOG_CLOSE();

    if (_inst)
    {
        CMReturnInstance(rslt, _inst);
        CMReturnDone(rslt);
        CMReturn (CMPI_RC_OK);
    }
    else
    {
        CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
    }
}

CMPIStatus TestCMPIBrokerInstanceProviderDeleteInstance (CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop)
{
    PROV_LOG_OPEN (_ClassName, _ProviderLocation);
    PROV_LOG ("Inside Delete Instance");
    PROV_LOG_CLOSE ();
    CMRelease (_inst);
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIBrokerInstanceProviderExecQuery (CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *lang,
    const char *query)
{
    CMPIData retData;
    CMPIString* type;
    CMPIBoolean bol=0;
    CMPIStatus rc = {CMPI_RC_OK, NULL};

    PROV_LOG_OPEN (_ClassName, _ProviderLocation);
    PROV_LOG ("ExecQuery");
    retData = CMGetProperty(_inst, "n64", &rc);
    PROV_LOG("++++ Status of CMGetProperty : (%s)",
        strCMPIStatus(rc));
    PROV_LOG("n64 = %" PEGASUS_64BIT_CONVERSION_WIDTH "u",
        retData.value.uint64);

    type = CDGetType (_broker, rslt, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type "
        "CMPIResult with CMPI_ResultExecQueryOnStack_Ftab : (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    bol = CDIsOfType (_broker, rslt, "CMPIResult", &rc);
    if ( bol )
    {
        PROV_LOG ("++++ CDIsOfType for CMPIResult with "
            "CMPI_ResultExecQueryOnStack_Ftab status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }

    PROV_LOG_CLOSE();
    CMReturnInstance(rslt, _inst);
    CMReturnDone(rslt);
    CMReturn (CMPI_RC_OK);
}

CMPIStatus TestCMPIBrokerInstanceProviderModifyInstance (CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci,
    const char **properties)
{
    CMPIString* type;
    CMPIBoolean bol=0;
    CMPIStatus rc = {CMPI_RC_OK, NULL};

    PROV_LOG_OPEN (_ClassName, _ProviderLocation);
    PROV_LOG ("Inside Modify Instance");

    /* Testcases for increasing coverage in CMPI_BrokerEnc.cpp*/
    type = CDGetType (_broker, rslt, &rc);
    PROV_LOG ("++++ Status of mbEncGetType with input of type "
        "CMPIResult with CMPI_ResultResponseOnStack_Ftab : (%s) type(%s)",
        strCMPIStatus (rc),
        CMGetCharsPtr(type, NULL));

    bol = CDIsOfType (_broker, rslt, "CMPIResult", &rc);
    if ( bol )
    {
        PROV_LOG ("++++ CDIsOfType for CMPIResult with "
            "CMPI_ResultResponseOnStack_Ftab status is (%s) : %d",
            strCMPIStatus (rc),
            bol);
    }
    PROV_LOG_CLOSE ();

    CMReturn (CMPI_RC_OK);
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub (TestCMPIBrokerInstanceProvider,
    TestCMPIBrokerInstanceProvider, _broker, CMNoHook)

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
