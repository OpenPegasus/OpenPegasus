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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const CIMNamespaceName NAMESPACE = CIMNamespaceName("test/TestProvider");

static Boolean verbose;

void callMethod(const CIMName& methodName)
{
    try
    {
        CIMClient client;
        client.connectLocal();

        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        CIMObjectPath instName =
            CIMObjectPath("Test_MethodProviderClass.Id=1");

        inParams.append(CIMParamValue("InParam1", Uint32(1)));
        inParams.append(CIMParamValue("InParam2", Uint32(2)));

        CIMValue returnValue = client.invokeMethod(
            NAMESPACE,
            instName,
            methodName,
            inParams,
            outParams);

        Uint32 rc;
        returnValue.get(rc);
        PEGASUS_TEST_ASSERT(rc == 10);

        PEGASUS_TEST_ASSERT(outParams.size() == 2);

        Uint32 outParam1 = 0;
        Uint32 outParam2 = 0;

        for (Uint32 i = 0; i < 2; i++)
        {
            if (outParams[i].getParameterName() == "OutParam1")
            {
                outParams[i].getValue().get(outParam1);
            }
            else if (outParams[i].getParameterName() == "OutParam2")
            {
                outParams[i].getValue().get(outParam2);
            }
            else
            {
                PEGASUS_TEST_ASSERT(0);
            }
        }
        PEGASUS_TEST_ASSERT(outParam1 == 21);
        PEGASUS_TEST_ASSERT(outParam2 == 32);
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}

void  callMethodOutputParam(const CIMName& methodName)
{
    try
    {
        CIMClient client;
        client.connectLocal();

        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        CIMObjectPath instName =
            CIMObjectPath("Test_MethodProviderClass.Id=1");

        CIMValue returnValue = client.invokeMethod(
            NAMESPACE,
            instName,
            methodName,
            inParams,
            outParams);

        Uint32 rc;
        returnValue.get(rc);
        PEGASUS_TEST_ASSERT(rc == 0);

        PEGASUS_TEST_ASSERT(outParams.size() == 101);
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}


/* Test for method invoke and response that handles ref parameter
   This functions sends two ref  in parameters and expects exactly the
   same references back in the two out parameters.
*/
void callMethodRefParam(const CIMName& methodName)
{
    try
    {
        CIMClient client;
        client.connectLocal();

        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        CIMObjectPath instName =
            CIMObjectPath("Test_MethodProviderClass.Id=1");

        CIMObjectPath inParam1(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Patrick\""));

        CIMObjectPath inParam2(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Rafter\""));

        inParams.append(CIMParamValue("InParam1", inParam1));
        inParams.append(CIMParamValue("InParam2", inParam2));
        inParams.append(CIMParamValue("Recursed", Boolean(false)));

        CIMValue returnValue = client.invokeMethod(
            NAMESPACE,
            instName,
            methodName,
            inParams,
            outParams);

        Uint32 rc;
        returnValue.get(rc);
        PEGASUS_TEST_ASSERT(rc == 10);

        PEGASUS_TEST_ASSERT(outParams.size() == 3);

        CIMObjectPath outParam1;
        CIMObjectPath outParam2;

        for (Uint32 i = 0; i < 2; i++)
        {
            if (outParams[i].getParameterName() == "OutParam1")
            {
                outParams[i].getValue().get(outParam1);
            }
            else if (outParams[i].getParameterName() == "OutParam2")
            {
                outParams[i].getValue().get(outParam2);
            }
            else
            {
                PEGASUS_TEST_ASSERT(0);
            }
        }
        PEGASUS_TEST_ASSERT(outParam1 == inParam1);
        PEGASUS_TEST_ASSERT(outParam2 == inParam2);
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}


/* Test for method invoke and response that handles ref parameter arrays.
   This functions sends two ref array in parameters and expects exactly the
   same reference arrays back in the two out parameters.
*/
void callMethodRefParamArray(const CIMName& methodName)
{
    try
    {
        CIMClient client;
        client.connectLocal();

        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        CIMObjectPath instName =
            CIMObjectPath("Test_MethodProviderClass.Id=1");

        CIMObjectPath inParamA1(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Patrick\""));

        CIMObjectPath inParamA2(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Rafter\""));

        CIMObjectPath inParamB1(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"Fred\""));

        CIMObjectPath inParamB2(
            String("//atp:77/root/cimv2:CIM_ComputerSystem."
                "last=\"Rafter\",first=\"John\""));

        Array<CIMObjectPath> InParamAArray;
        InParamAArray.append(inParamA1);
        InParamAArray.append(inParamA2);
        CIMValue inParam1(InParamAArray);

        Array<CIMObjectPath> InParamBArray;
        InParamBArray.append(inParamB1);
        InParamBArray.append(inParamB2);
        CIMValue inParam2(InParamBArray);

        inParams.append(CIMParamValue("InParam1", inParam1));
        inParams.append(CIMParamValue("InParam2", inParam2));

        CIMValue returnValue = client.invokeMethod(
            NAMESPACE,
            instName,
            methodName,
            inParams,
            outParams);

        Uint32 rc;
        returnValue.get(rc);
        PEGASUS_TEST_ASSERT(rc == 10);

        PEGASUS_TEST_ASSERT(outParams.size() == 2);

        Array<CIMObjectPath> outParam1;
        Array<CIMObjectPath> outParam2;

        for (Uint32 i = 0; i < 2; i++)
        {
            if (outParams[i].getParameterName() == "OutParam1")
            {
                outParams[i].getValue().get(outParam1);
            }
            else if (outParams[i].getParameterName() == "OutParam2")
            {
                outParams[i].getValue().get(outParam2);
            }
            else
            {
                PEGASUS_TEST_ASSERT(0);
            }
        }
        PEGASUS_TEST_ASSERT(outParam1 == inParam1);
        PEGASUS_TEST_ASSERT(outParam2 == inParam2);
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}
int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        if (verbose)
        {
            cout << "Calling test2" << endl;
        }
        callMethod("test2");

        if (verbose)
        {
            cout << "Calling test1" << endl;
        }
        callMethod("test1");

        if (verbose)
        {
            cout << "Calling test3" << endl;
        }
        callMethodRefParam("test3");
        if (verbose)
        {
            cout << "Calling test4" << endl;
        }
        callMethodRefParamArray("test4");
        if (verbose)
        {
            cout << "Calling test5" << endl;
        }
        callMethodOutputParam("test5");
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
