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

#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Char16.h>
#include <Pegasus/Client/CIMClient.h>

#include <iostream>

#ifdef PEGASUS_OS_ZOS
// This is inline code. No external dependency is created.
#include <unistd.h>
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static Boolean verbose ;


//Get the first non LLA address and used that for testing
String _getSystemInterface(Array<String> list )
{
    for(Uint32 i =0; i < list.size(); ++i)
    {
        //Detect if ip is link-local address
        String tmp = list[i].subString(0,4);
        if( !(String::equalNoCase(tmp, "fe80")))
        {
            return list[i];
        }
    }
    return String::EMPTY;
}

void _ConnectClientAndTest(CIMClient &clnt,
    const Uint32 port,
    const String &ip)
{
        cout << "connecting to "<< ip.getCString() << endl;
        clnt.connect(ip, port,"guest", "guest");

        CIMClass cimClass = clnt.getClass(
            CIMNamespaceName("root/cimv2"),
            CIMName ("CIM_ManagedElement"),
            true,
            false,
            false,
            CIMPropertyList());

            PEGASUS_TEST_ASSERT(String::equal("CIM_ManagedElement",
               cimClass.getClassName().getString().getCString()));

        clnt.disconnect();
}
void  _restrictionTest(String &list)
{
    if (list.size() == 0)
    {
        cerr << "++++ No Usable Interface Detected ++++ " << endl;
        return;
    }

    CIMClient clnt;
    //This should connect
    _ConnectClientAndTest(clnt,5988,"127.0.0.1");

    bool isConnected = false;
    try
    {
        //This should fail to connect
        _ConnectClientAndTest(clnt, 5988,list);
        isConnected = true;
    }
    catch(CannotConnectException &e)
    {
        cout << "Expected exception: " << e.getMessage() << endl;
    }
    PEGASUS_TEST_ASSERT( !isConnected && (bool) "Unexpected connections made");
}

int main(int argc, char *argv[])
{
#ifdef PEGASUS_OS_ZOS
        // For z/OS set stdout and stderr to EBCDIC
        setEBCDICEncoding(STDOUT_FILENO);
        setEBCDICEncoding(STDERR_FILENO);
#endif
    //make compiler not to complaint of unused param
    argc;

    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        Array<String> list = System::getInterfaceAddrs();

        if( list.size() > 0 )
        {
               String ipToTest = _getSystemInterface(list);
               _restrictionTest(ipToTest);
        }
        else
        {
            cerr << "++++ No Interface Detected ++++ " << endl;
            return 0;
        }
    }
    catch(Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        cerr << argv[0] << "+++++ Failed ++" << endl;

        return 1;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}
