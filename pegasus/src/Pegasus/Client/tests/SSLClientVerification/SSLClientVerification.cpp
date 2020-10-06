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

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const CIMNamespaceName NAMESPACE = CIMNamespaceName("root/cimv2");

/*
 * This is a test for basic SSL Client Verification that was introduced in
 * PEP#165.
 *
 * The cimserver must be configured as following to test this:
 *
 *  enableHttpsConnection=true
 *  enableAuthentication=true
 *  sslClientVerificationMode=optional
 *  sslTrustStore=<directory>
 *  sslTrustStoreUserName=<valid sys user>
 *
 * To test optional truststore automatic update:
 *
 *  PEGASUS_USE_AUTOMATIC_TRUSTSTORE_UPDATE is defined
 *  enableSSLTrustStoreAutoUpdate=true
 *
 * Additionally, the client must have its own certificate and private key to
 * present to the server during the handshake.
 * Finally, the client certificate must be located in the server's truststore
 * in the correct format: "<subject-hash>.0"
 */

int main(int argc, char** argv)
{
    String host;

#ifdef PEGASUS_HAS_SSL
    try
    {
        if (argc == 1)
        {
            host = System::getHostName();
        }
        else
        {
            if (argc == 2)
            {
                 host = argv[1];
            }
            else
            {
                cerr << "Usage of " << argv[0] <<" :" << endl;
                cerr << "No argument: test to localhost" << endl;
                cerr << "1st argument: test to specified host" << endl;
                exit(1);
            }
        }

        Uint32 port = System::lookupPort(
            WBEM_HTTPS_SERVICE_NAME, WBEM_DEFAULT_HTTPS_PORT);

        //
        // Note that these files are separate from the client.pem which
        // represents the client's truststore.
        //
        const char* pegasusHome = getenv("PEGASUS_HOME");
        String certPath = FileSystem::getAbsolutePath(
            pegasusHome, "clientkeystore/client_cert.pem");
        String keyPath = FileSystem::getAbsolutePath(
            pegasusHome, "clientkeystore/client_key.pem");

        cerr << "certPath is " << certPath << endl;
        cerr << "keyPath is " << keyPath << endl;

        String randPath;
# ifdef PEGASUS_SSL_RANDOMFILE
        randPath = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
# endif

        CIMClient client;

        client.connect(
                      host,
                      port,
                      SSLContext("", certPath, keyPath, NULL, randPath),
                      String::EMPTY,
                      String::EMPTY);

        //
        // Do a generic call.  We have to do this call to test whether or not
        // we get 401'ed.
        //
        CIMClass cimClass = client.getClass(
                                           CIMNamespaceName("root/cimv2"),
                                           CIMName ("CIM_ManagedElement"),
                                           true,
                                           false,
                                           false,
                                           CIMPropertyList());

        client.disconnect();
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        cerr << "Root cause could be PEGASUS_HAS_SSL is defined but "
            "enableHttpsConnection=false" << endl;
        exit(1);
    }

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
