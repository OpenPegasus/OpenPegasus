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
//%////////////////////////////////////////////////////////////////////////////
//
// This CIM client program is used to test if a static version of the
// CIM Client works. Focus lies espescially on constructor, destructor and
// automatic disconnect from the server.
//
// ==========================================================================

#include <Pegasus/Client/CIMClient.h>
#include <cstring>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

// For the resolution of Bug#4590
#if defined(PEGASUS_OS_DARWIN) || defined(PEGASUS_OS_VMS)
CIMClient& getCIMClientRef()
{
        static CIMClient client;
        return client;
}
#define client getCIMClientRef()
#else
        static CIMClient client;
#endif

////////////////////////////////////////////////////////////////////////////
//  _errorExit
////////////////////////////////////////////////////////////////////////////

void _errorExit(String message)
{
    cerr << "TestStaticClient error: " << message << endl;
    exit(1);
}

// =========================================================================
//    Main
// =========================================================================

int main()
{
    // Connect to server
    try
    {
        client.connectLocal();
    }
    catch (Exception& e)
    {
        _errorExit(e.getMessage());
    }

    try
    {
        cout << "\n+++++ passed all tests" << endl;
    }
    catch(Exception)
    {
        cout << "\n----- Test Static Client Failed" << endl;
    }
    return(0);
}
