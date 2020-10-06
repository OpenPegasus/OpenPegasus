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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/AutoPtr.h>


#ifdef PEGASUS_HAS_SSL
#  include <openssl/tls1.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const CIMNamespaceName NAMESPACE = CIMNamespaceName("root/cimv2");

/*
 * This tests the TLSv1.2 support in pegasus
 *
 * The cimserver must be configured as following to test this:
 *
 *  enableHttpsConnection=true and sslCipherSuite = TLSv1.2
 *
 */

#ifdef PEGASUS_HAS_SSL

void _testTLS1_2_support(String &host , Uint32 &port, String &randPath)
{

//A rudimentary way to detect TLS1_2 support on openSSL and hence on pegasus
#ifdef TLS1_2_VERSION

    String trustStorePath;
    String certPath;
    String keyPath;
    String cipherSuite = "SSLv3";
    Boolean sslCompatibility = false;
    AutoPtr<CIMClient> cc(new CIMClient);

    try
    {
        AutoPtr<SSLContext> sslContext(new SSLContext (trustStorePath, 
            certPath, keyPath, String::EMPTY, 0, randPath, cipherSuite,
            sslCompatibility));

        if (sslContext.get())
        {
            cc->connect (host, port, *sslContext, "", "");
            //
            //Unreachable when server is started with correct versions
            //Otherwise it informs that server is not started properly
            //
            PEGASUS_TEST_ASSERT( 0 &&
                (bool)"cimserver not started with sslCipherSuite=TLSv1.2");
        }
    }
    catch(CannotConnectException &e)
    {
        cout << "SSLCipherTest Expected exception:  "<< e.getMessage() << endl;
        cout << "Test passed: Connecting with cipher list: " << cipherSuite
            << " and TLS1.2 as expected" << endl;
    }
    catch(Exception &e)
    {
        cerr << "SSLCipherTest Failed:  "<< e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(0 && (bool)"Got unexpected Exception, Aborting");
    }


    cipherSuite = "TLSv1.2";
    try
    {

        AutoPtr<SSLContext> sslContext(new SSLContext (trustStorePath, 
            certPath, keyPath, String::EMPTY, 0, randPath, cipherSuite,
            sslCompatibility));
        if (sslContext.get())
        {
            cc->connect (host, port, *sslContext, "", "");
            //
            // Do a generic call. We have to do this call to test whether or
            // not we get 401'ed.
            //
            CIMClass cimClass = cc->getClass(CIMNamespaceName("root/cimv2"),
                CIMName ("CIM_ManagedElement"), true, false, false,
                CIMPropertyList());
            cc->disconnect();
        }
    }
    catch(Exception &e)
    {
        cerr << "SSLCipherTest Error: "<< e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(0);
    }

    cout << "Test passed: Connecting with cipher list: " << cipherSuite
        << " and TLS1.2" << endl;
#else
    //Hacks to stop unused param warning while compiling
    PEGASUS_TEST_ASSERT( host == host);
    PEGASUS_TEST_ASSERT( port == port);
    PEGASUS_TEST_ASSERT( randPath == randPath);

    cout << "TLSv1.2 is not supported on this build of pegasus. Upgrade openssl"
        << " skipping TLSv1.2 support tests" << endl;
#endif
}

#endif

int main()
{

#ifdef PEGASUS_HAS_SSL
    String host = System::getHostName();
    Uint32 port = System::lookupPort(
        WBEM_HTTPS_SERVICE_NAME, WBEM_DEFAULT_HTTPS_PORT);

    String randPath;
# ifdef PEGASUS_SSL_RANDOMFILE
    const char* pegasusHome = getenv("PEGASUS_HOME");
    randPath = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
# endif

    _testTLS1_2_support( host ,  port,  randPath);

    cout << "+++++ passed all tests" << endl;

    return 0;

#endif

    //
    // This returns a false positive result.
    // But we should never get here since this test is only run if
    // PEGASUS_HAS_SSL is defined.
    //

    return 0;
}
