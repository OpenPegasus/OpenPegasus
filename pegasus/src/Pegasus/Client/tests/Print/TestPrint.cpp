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
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Print.h>
#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

//==============================================================================
//
// This program obtains instances of the given class when invoked as follows:
//
//     $ TestPrint <CLASSNAME>
//
// It connects locally to the CIM server, enumerates instances of that class
// and then prints each one.
//
//==============================================================================

int main(int argc, char** argv)
{
    // Check usage:

    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " classname" << endl;
        exit(1);
    }

    // Check classname argument:

    CIMName name;

    try
    {
        name = argv[1];
    }
    catch (...)
    {
        cerr << argv[0] << ": illegal CIM classname: " << argv[1] << endl;
        exit(1);
    }


    // Connect:

    CIMClient cc;

    try
    {
        cc.connectLocal();
    }
    catch (...)
    {
        cerr << "failed to connect" << endl;
        exit(1);
    }

    // Enumerate and print:

    try
    {
        Array<CIMInstance> a = cc.enumerateInstances("root/cimv2", name);

        for (Uint32 i = 0; i < a.size(); i++)
        {
            PrintInstance(cout, a[i]);
        }
    }
    catch (...)
    {
        cerr << "failed to enumerate instances of " << name.getString() << endl;
        exit(1);
    }

    return 0;
}
