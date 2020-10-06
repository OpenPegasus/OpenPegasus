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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main()
{
    const CIMNamespaceName NAMESPACE =
        CIMNamespaceName("test/testprovider");
    const CIMName CLASSNAME = CIMName("IndicationStressTestClass");

    try
    {
        Array<CIMKeyBinding> kbArray;
        CIMKeyBinding        kb;
        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;
        String               hostName;
        CIMClient            client;
        Uint32 sendCount = 1;


        //
        // Connect to CIM Server.
        //
        // The connectLocal API creates a connection to the server for
        // Local clients. The connection is automatically authenticated
        // for the current user. This sample uses the connectLocal API.
        //
        // Also available is the connect API, used to create
        // an HTTP connection with the server defined by the URL in address.
        // User name and password information can be passed
        // using the connect interface.
        //
        client.connectLocal();

        //
        // Build the params.
        //
        kb.setName(CIMName("Identifier"));
        kb.setValue("1");
        kb.setType(CIMKeyBinding::NUMERIC);

        kbArray.append(kb);

        //inParams is the list of parameters for the method that 
        //we want to invoke. In our case the method is 
        //"SendTestIndicationNormal".
        //The parameters that need to be passed are the name and value.

        inParams.append(
            CIMParamValue(String("indicationSendCount"),
            CIMValue(sendCount)));

        CIMObjectPath reference(hostName, NAMESPACE, CLASSNAME, kbArray);

        //
        // Specify the method that needs to be invoked. 
        //
        CIMValue retValue = client.invokeMethod(
            NAMESPACE,
            reference,
            CIMName ("SendTestIndicationNormal"),
            inParams,
            outParams);

        cout << "Output : " << retValue.toString() << endl;
        for (Uint8 i = 0; i < outParams.size(); i++)
            cout << outParams[i].getParameterName()
                << " : "
                << outParams[i].getValue().toString()
                << endl;
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    return 0;
}
