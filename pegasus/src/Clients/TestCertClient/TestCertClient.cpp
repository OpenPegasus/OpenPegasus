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
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/TLS.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

enum expectedResultType { NONE, PASS, FAIL };
enum expectedErrorType { ERROR_TYPE_NONE, ERROR_TYPE_HTTP_401,
         ERROR_TYPE_CANNOT_CONNECT };

/** This client is primarily used to test the SSL client verification function.
 * The client is used by the poststarttests of the ssltrustmgr CLI.
 * This client was introduced because existing test clients are heavy on
 * function and thus take a long time to return.  We don't care about the
 * response, only whether the request was authenticated.
 */

int main(int argc, char** argv)
{
//   TestCertClient Parameters
//       Parameter 1: Client Certification File
//       Parameter 2: Client Private Key File
//       Parameter 3: Random Key File
//       Parameter 4: User Name
//       Parameter 5: Password
//       Parameter 6: Expected Result
//       Parameter 7: Expected Identity

    if ((argc < 3) || (argc > 8))
    {
        PEGASUS_STD(cout) << "Wrong number of arguments" << PEGASUS_STD(endl);
        exit(1);
    }

    String certpath;
    if (strcmp(argv[1],"NONE") != 0)
    {
        certpath = argv[1];
    }

    String keypath;
    if (strcmp(argv[2],"NONE") != 0)
    {
        keypath = argv[2];
    }

    String randFile;
    if (argc >=4)
    {
        if (strcmp(argv[3],"CONFIG") == 0)
        {
            const char* pegasusHome = getenv("PEGASUS_HOME");
            randFile = FileSystem::getAbsolutePath(
                pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
        }
        else if (strcmp(argv[3],"NONE") != 0)
        {
            randFile = argv[3];
        }
    }

    String userName;
    if (argc >=  5)
    {
        userName = argv[4];
    }

    String password;
    if (argc >=  6)
    {
        password = argv[5];
    }

    expectedResultType expectedResult = NONE;
    expectedErrorType expectedError = ERROR_TYPE_NONE;
    String expectedUserName;
    if (argc >=  7)
    {
        if (strcmp(argv[6],"PASS") == 0)
        {
            expectedResult = PASS;
            if (argc >= 8)
            {
               if (strcmp(argv[7],"NONE") != 0)
               {
                   expectedUserName = argv[7];
               }
            }
        }
        else if (strcmp(argv[6],"FAIL") == 0)
        {
            expectedResult = FAIL;
            if (argc >= 8)
            {
               if (strcmp(argv[7],"NONE") == 0)
               {
                   expectedError = ERROR_TYPE_NONE;
               }
               else if (strcmp(argv[7],"HTTP_401") == 0)
               {
                   expectedError = ERROR_TYPE_HTTP_401;
               }
               else if (strcmp(argv[7],"CANNOT_CONNECT") == 0)
               {
                   expectedError = ERROR_TYPE_CANNOT_CONNECT;
               }
               else
               {
                   PEGASUS_STD(cout) << "Invalid expectedError parameter: "
                        << argv[7] << PEGASUS_STD(endl);
                   exit(1);
               }
            }
        }
        else if (strcmp(argv[6],"NONE") == 0)
        {
            expectedResult = NONE;
        }
        else
        {
            PEGASUS_STD(cout) << "Invalid expectedResult parameter: "
                << argv[6] << PEGASUS_STD(endl);
            exit(1);
        }
    }

    try
    {
        AutoPtr<SSLContext> pCtx;
        if (certpath != String::EMPTY)
        {
            pCtx.reset(
                new SSLContext(String::EMPTY, certpath, keypath, 0, randFile));
        }
        else
        {
            pCtx.reset(new SSLContext(String::EMPTY, 0, randFile));
        }

        PEGASUS_STD(cout)<< "TestCertClient::Connecting to 127.0.0.1:5989"
          << PEGASUS_STD(endl);

        CIMClient client;
        client.connect("127.0.0.1", 5989, *pCtx, userName, password);

        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;
        CIMValue retValue = client.invokeMethod(
            CIMNamespaceName("test/TestProvider"),
            CIMObjectPath("Test_MethodProviderClass"),
            CIMName("getIdentity"),
            inParams,
            outParams);

        if (expectedResult == FAIL)
        {
           throw Exception("Failure: Connection unexpectedly succeeded");
        }

        String retUserName;
        retValue.get(retUserName);

        if (expectedUserName != String::EMPTY)
        {
           if (expectedUserName != retUserName)
           {
              throw Exception("Provider returned unexpected Identity: "
                  + retUserName);
           }
        }
        CIMClass c = client.getClass("root/cimv2",
            "CIM_ComputerSystem", false, false, true);

        PEGASUS_STD(cout) << "Result: " <<  c.getClassName().getString()
            << PEGASUS_STD(endl);
    }
    catch (CIMClientHTTPErrorException& httpException)
    {
        if ((expectedResult == FAIL) &&
                (httpException.getCode() == 401) &&
                (expectedError == ERROR_TYPE_HTTP_401))
        {
            PEGASUS_STD(cout) << "+++++ "<< argv[0] <<
                " +++++ passed all tests" << PEGASUS_STD(endl);
            exit(0);
         }

         PEGASUS_STD(cout) << "Exception: " << httpException.getMessage()
            << PEGASUS_STD(endl);
         exit(1);
    }
    catch (CannotConnectException& connectException)
    {
        if ((expectedResult == FAIL) &&
                (expectedError == ERROR_TYPE_CANNOT_CONNECT))
        {
            PEGASUS_STD(cout) << "+++++ "<< argv[0] <<
                " +++++ passed all tests" << PEGASUS_STD(endl);
            exit(0);
        }
        PEGASUS_STD(cout) << "Exception: " << connectException.getMessage()
            << PEGASUS_STD(endl);
        exit(1);
    }
    catch (Exception& ex)
    {
        PEGASUS_STD(cout) << "Exception: " << ex.getMessage()
             << PEGASUS_STD(endl);
        exit(1);

    }
    catch (...)
    {
        PEGASUS_STD(cout) << "Unknown exception" << PEGASUS_STD(endl);
        exit(1);
    }
    if (expectedResult == PASS)
    {
        PEGASUS_STD(cout) << "+++++ "<< argv[0] <<
             " +++++ passed all tests" << PEGASUS_STD(endl);
    }
    else
    {
        PEGASUS_STD(cout) << "+++++ "<< "TestCertClient"
            << " Terminated Normally" << PEGASUS_STD(endl);
    }
    exit(0);
}

