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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const String NAMESPACE = "root/SampleProvider";
const String classname = "Sample_MethodProviderClass";
const String methodName = "SayHello";
const String OUTSTRING = "Yoda";
const String GOODREPLY = "Hello, " + OUTSTRING + "!";
const String GOODPARAM = "From Neverland";

int main()
{
    const CIMObjectPath instanceName = CIMObjectPath(
        "Sample_MethodProviderClass.Identifier=1");

    try
    {
        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        CIMClient client;
        client.connectLocal();
        inParams.append(CIMParamValue("Name", CIMValue(OUTSTRING)));
        CIMValue retValue = client.invokeMethod(
            NAMESPACE,
            instanceName,
            methodName,
            inParams,
            outParams);

        if (retValue.toString() != GOODREPLY)
        {
            cerr << "Error: bad reply \"" << retValue.toString() << "\"" <<
                endl;
            exit(1);
        }
        else
        {
            if (outParams.size() > 0)
            {
                String outReply;
                CIMValue paramVal = outParams[0].getValue();
                paramVal.get(outReply);
                if (outReply == GOODPARAM)
                {
                    cout << "+++++ InvokeMethod2 passed all tests" << endl;
                    return 0;
                }
                else
                {
                    cerr << "Error: bad output parameter: \"" << outReply <<
                        "\"" << endl;
                    exit(1);
                }
            }
            else
            {
                cerr << "Error: output parameter missing. Reply: \"" <<
                    retValue.toString() << "\"" << endl;
                exit(1);
            }
        }
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }
}
