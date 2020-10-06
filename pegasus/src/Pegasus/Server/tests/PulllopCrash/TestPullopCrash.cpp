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


// This tests calls openEnumerateInstances(), followed by closeEnumeration
// on PG_ComputerSystem.
// It tests bug http://bugzilla.openpegasus.org/show_bug.cgi?id=9973
// - Pegasus used to crash when close was too quick after open, with no pull in
// between.

#include <Pegasus/Common/Config.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main(int argc, char** argv)
{
    const CIMNamespaceName NAMESPACE =
        CIMNamespaceName("root/cimv2");
    const CIMName CLASSNAME = CIMName("PG_ComputerSystem");

    try
    {
        Boolean                 deepInheritance = true;
        Boolean                 includeClassOrigin = false;
        Array<CIMInstance>      cimInstances;
        CIMClient               client;

        client.connectLocal();

        //
        // 100x try to open/close enumeration on PG_ComputerSystem.
        //
        for (int i=0; i<100; i++)
        {
            CIMEnumerationContext  enumCtx;
            Boolean endOfSeq;
            cimInstances = client.openEnumerateInstances(
                    enumCtx,
                    endOfSeq,
                    NAMESPACE,
                    CLASSNAME,
                    deepInheritance,
                    includeClassOrigin);

            client.closeEnumeration(enumCtx);
        }
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    cout << "TestPullopCrash passed" << endl;
    return 0;
}
