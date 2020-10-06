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

#include <cstdlib>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Print.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Client/CIMClientRep.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

static char * verbose;

//==============================================================================
//
// TestBinaryClient host port
//
//     This program enumerates instances of CIM_ManagedElement using the
//     OpenPegasus binary protocol.
//
//==============================================================================

static void _SetBinaryResponse(CIMClient& client, Boolean flag)
{
    CIMClientRep* rep = *(reinterpret_cast<CIMClientRep**>(&client));
    rep->setBinaryResponse(flag);
}

void printInstances(Array<CIMInstance> instances)
{
    for (Uint32 i = 0; i < instances.size(); i++)
    {
        PrintInstance(cout, instances[i]);
    }
}

int main(int argc, char** argv)
{

    verbose = getenv("PEGASUS_TEST_VERBOSE");
    // Check args:

    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " host port" << endl;
        exit(1);
    }

    // Extract args:

    String host = argv[1];
    Uint32 port = atoi(argv[2]);

    if (port == 0)
    {
        cerr << argv[0] << ": illegal value for port number" << endl;
        exit(1);
    }

    // Connect and enumerate instances.

    CIMClient client;
    _SetBinaryResponse(client, true);

    try
    {
        client.connect(host, port, String(), String());

        Array<CIMInstance> result = client.enumerateInstances("root/cimv2",
            "CIM_ManagedElement");

        if (verbose)
        {
            printInstances(result);
        }

        for (Uint32 i = 0; i < result.size(); i++)
        {
            PrintInstance(cout, result[i]);
        }

        CIMEnumerationContext enumerationContext;
        Boolean endOfSequence;
        result = client.openEnumerateInstances(
           enumerationContext,
           endOfSequence,
           "root/cimv2",
           "CIM_ManagedElement");

        if (verbose)
        {
            printInstances(result);
        }

        while (!endOfSequence)
        {
            result = client.pullInstancesWithPath(
               enumerationContext,
               endOfSequence,
               100);

            if (verbose)
            {
                printInstances(result);
            }
        }

    }
    catch (Exception& e)
    {
        cerr << e.getMessage() << endl;
        exit(1);
    }

    cout << "TestBinaryClient +++++ passed all tests" << endl;

    return 0;
}
