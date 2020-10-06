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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/AnonymousPipe.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/InternalException.h>

#if defined (PEGASUS_OS_TYPE_WINDOWS)
#include <windows.h>
#elif defined (PEGASUS_OS_VMS)
# include <perror.h>
# include <climsgdef.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <processes.h>
# include <unixio.h>
#else
# include <unistd.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main ()
{
    Boolean verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;

    try
    {
        //
        //  Create the anonymous pipe
        //
        AutoPtr <AnonymousPipe> pipeToChild (new AnonymousPipe ());
        AutoPtr <AnonymousPipe> pipeFromChild (new AnonymousPipe ());

        //
        //  Get arguments to pass to child process
        //
        String childPath;
        const char * homeDir = getenv ("PEGASUS_HOME");
        if (homeDir == NULL)
        {
            homeDir = ".";
        }
        childPath.append (homeDir);
        childPath.append ("/bin/TestAnonymousPipeChild");

        CString childPathCStr = childPath.getCString ();

        AutoArrayPtr <char> childReadHandle (new char [32]);
        AutoArrayPtr <char> childWriteHandle (new char [32]);

        pipeToChild->exportReadHandle (childReadHandle.get ());
        pipeFromChild->exportWriteHandle (childWriteHandle.get ());

#if defined (PEGASUS_OS_TYPE_WINDOWS)
        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;

        //
        //  Set up members of the PROCESS_INFORMATION structure
        //
        ZeroMemory (&piProcInfo, sizeof (PROCESS_INFORMATION));

        //
        //  Set up members of the STARTUPINFO structure
        //
        ZeroMemory (&siStartInfo, sizeof (STARTUPINFO));
        siStartInfo.cb = sizeof (STARTUPINFO);

        //
        //  Generate command line
        //
        char childCommandLine [2048];
        sprintf (childCommandLine, "\"%s\" %s %s",
            (const char *) childPath.getCString (),
            childReadHandle.get (), childWriteHandle.get ());

        //
        //  Create the child process
        //
        if (!CreateProcess (
            NULL,              //
            childCommandLine,  //  command line
            NULL,              //  process security attributes
            NULL,              //  primary thread security attributes
            TRUE,              //  handles are inherited
            0,                 //  creation flags
            NULL,              //  use parent's environment
            NULL,              //  use parent's current directory
            &siStartInfo,      //  STARTUPINFO
            &piProcInfo))      //  PROCESS_INFORMATION
        {
            cerr << "Parent failed to CreateProcess:" << GetLastError ()
                << endl;
            PEGASUS_TEST_ASSERT (0);
        }
#elif defined (PEGASUS_OS_VMS)
        //
        //  fork and exec the child process
        //
        int status,
            cstatus;

        status = vfork ();
        switch (status)
        {
          case 0:
            if ((status = execl ((const char *) childPathCStr,
                                 (const char *) childPathCStr,
                                 childReadHandle.get (),
                                 childWriteHandle.get (),
                                 (char *) 0))
                                 == -1)
            {
              //
              //  execl failed: close pipe handles
              //
              pipeToChild->closeReadHandle ();
              pipeToChild->closeWriteHandle ();
              pipeFromChild->closeReadHandle ();
              pipeFromChild->closeWriteHandle ();

              cerr << "Parent failed to execl: " << strerror (errno) << endl;
              PEGASUS_TEST_ASSERT (0);
            }
            break;

          case -1:
            //
            //  fork failed: close pipe handles
            //
            pipeToChild->closeReadHandle ();
            pipeToChild->closeWriteHandle ();
            pipeFromChild->closeReadHandle ();
            pipeFromChild->closeWriteHandle ();

            cerr << "Parent failed to fork: " << strerror (errno) << endl;
            PEGASUS_TEST_ASSERT (0);
            break;

          default:
            //
            //  Parent: Close child handles
            //
            pipeToChild->closeReadHandle ();
            pipeFromChild->closeWriteHandle ();
#else
        //
        //  fork and exec the child process
        //
        pid_t pid;
        if ((pid = fork ()) < 0)
        {
            //
            //  fork failed: close pipe handles
            //
            pipeToChild->closeReadHandle ();
            pipeToChild->closeWriteHandle ();
            pipeFromChild->closeReadHandle ();
            pipeFromChild->closeWriteHandle ();

            cerr << "Parent failed to fork: " << strerror (errno) << endl;
            PEGASUS_TEST_ASSERT (0);
        }
        else if (pid > 0)
        {
            //
            //  Parent: Close child handles
            //
            pipeToChild->closeReadHandle ();
            pipeFromChild->closeWriteHandle ();
        }
        else
        {
            //
            //  Child: Close parent handles
            //
            pipeFromChild->closeReadHandle ();
            pipeToChild->closeWriteHandle ();

            if (execl ((const char *) childPathCStr,
                (const char *) childPathCStr,
                childReadHandle.get (),
                childWriteHandle.get (),
                (char *) 0) < 0)
            {
                //
                //  execl failed: close pipe handles
                //
                pipeToChild->closeReadHandle ();
                pipeToChild->closeWriteHandle ();
                pipeFromChild->closeReadHandle ();
                pipeFromChild->closeWriteHandle ();

                cerr << "Parent failed to execl: " << strerror (errno) << endl;
                PEGASUS_TEST_ASSERT (0);
            }
        }
#endif

        //
        //  Test writeBuffer and readBuffer
        //

        //
        //  Send a request buffer to the child via the anonymous pipe
        //
        Buffer requestBuffer;
        char buffer [32];
        sprintf (buffer, "%s", "Hello world");
        requestBuffer.append (buffer, strlen (buffer));
        Uint32 bufferLength = requestBuffer.size();
        AnonymousPipe::Status writeBufferStatus = pipeToChild->writeBuffer
            ((const char *) &bufferLength, sizeof (Uint32));
        if (writeBufferStatus == AnonymousPipe::STATUS_SUCCESS)
        {
            writeBufferStatus = pipeToChild->writeBuffer
                (requestBuffer.getData(), requestBuffer.size());
            if (writeBufferStatus != AnonymousPipe::STATUS_SUCCESS)
            {
                cerr << "Parent failed to write request data: "
                    << writeBufferStatus << endl;
                PEGASUS_TEST_ASSERT (0);
            }
        }
        else
        {
            cerr << "Parent failed to write request length: "
                << writeBufferStatus << endl;
            PEGASUS_TEST_ASSERT (0);
        }

        //
        //  Read the response buffer from the child via the anonymous pipe
        //
        AnonymousPipe::Status readBufferStatus = pipeFromChild->readBuffer
            ((char *) &bufferLength, sizeof (Uint32));
        if (readBufferStatus != AnonymousPipe::STATUS_SUCCESS)
        {
            cerr << "Parent failed to read response length: "
                << readBufferStatus << endl;
            PEGASUS_TEST_ASSERT (0);
        }
        if (bufferLength == 0)
        {
            cerr << "Parent read response length of 0" << endl;
            PEGASUS_TEST_ASSERT (0);
        }

        AutoArrayPtr <char> responseBuffer (new char [bufferLength + 1]);
        do
        {
            readBufferStatus = pipeFromChild->readBuffer (responseBuffer.get (),
                bufferLength);
        } while (readBufferStatus == AnonymousPipe::STATUS_INTERRUPT);
        if (readBufferStatus != AnonymousPipe::STATUS_SUCCESS)
        {
            cerr << "Parent failed to read response data: "
                << readBufferStatus << endl;
            PEGASUS_TEST_ASSERT (0);
        }
        responseBuffer.get () [bufferLength] = 0;
        if (verbose)
        {
            cout << "Response received by parent: " << responseBuffer.get ()
                << endl;
        }
        PEGASUS_TEST_ASSERT (strcmp (responseBuffer.get (), "Good-bye") == 0);

        //
        //  Test writeMessage and readMessage
        //
        AutoPtr <CIMGetInstanceRequestMessage> request
            (new CIMGetInstanceRequestMessage
                (String ("00000001"),
                CIMNamespaceName ("root/test/A"),
                CIMObjectPath ("MCCA_TestClass.theKey=1"),
                false,
                false,
                CIMPropertyList (),
                QueueIdStack ()));

        AnonymousPipe::Status writeMessageStatus =
            pipeToChild->writeMessage (request.get ());
        if (writeMessageStatus == AnonymousPipe::STATUS_SUCCESS)
        {
            CIMMessage * message;
            AnonymousPipe::Status readMessageStatus = pipeFromChild->readMessage
                (message);
            if (readMessageStatus != AnonymousPipe::STATUS_SUCCESS)
            {
                cerr << "Parent failed to read response message: "
                    << readMessageStatus << endl;
                PEGASUS_TEST_ASSERT (0);
            }

            AutoPtr<CIMGetInstanceResponseMessage> response;
            response.reset(
                dynamic_cast<CIMGetInstanceResponseMessage*>(message));
            PEGASUS_TEST_ASSERT (response.get() != 0);

            if (verbose)
            {
                cout << "CIMGetInstanceResponseMessage received by parent"
                    << endl;
            }

            PEGASUS_TEST_ASSERT (response->getType () ==
                CIM_GET_INSTANCE_RESPONSE_MESSAGE);
            PEGASUS_TEST_ASSERT (response->messageId == String ("00000002"));
            PEGASUS_TEST_ASSERT (response->cimException.getCode () ==
                CIM_ERR_FAILED);
            PEGASUS_TEST_ASSERT (((ContentLanguageListContainer)
                response->operationContext.get
                (ContentLanguageListContainer::NAME)).getLanguages().size() ==
                0);
        }
        else
        {
            cerr << "Parent failed to write request message: "
                << writeMessageStatus << endl;
            PEGASUS_TEST_ASSERT (0);
        }

        //
        //  Error cases
        //

        //
        //  Test invalid use of constructor
        //
        try
        {
            AutoPtr <AnonymousPipe> pipeError1 (new AnonymousPipe ("g", NULL));
            PEGASUS_TEST_ASSERT (false);
        }
        catch (Exception &)
        {
        }

        try
        {
            AutoPtr <AnonymousPipe> pipeError2 (new AnonymousPipe (NULL, "h"));
            PEGASUS_TEST_ASSERT (false);
        }
        catch (Exception &)
        {
        }

        //
        //  Close handles already closed
        //
        pipeToChild->closeWriteHandle ();
        pipeToChild->closeWriteHandle ();
        pipeFromChild->closeReadHandle ();
        pipeFromChild->closeReadHandle ();

        //
        //  Try to read or write after handles have been closed
        //
        writeBufferStatus = pipeToChild->writeBuffer
            ((const char *) &bufferLength, sizeof (Uint32));
        PEGASUS_TEST_ASSERT (writeBufferStatus == AnonymousPipe::STATUS_CLOSED);

        readBufferStatus = pipeFromChild->readBuffer ((char *) &bufferLength,
            sizeof (Uint32));
        PEGASUS_TEST_ASSERT (readBufferStatus == AnonymousPipe::STATUS_CLOSED);

#if defined (PEGASUS_OS_VMS)
          break;
        }
#endif
    }
    catch (Exception & e)
    {
        cerr << "Exception occurred in parent: " << e.getMessage () << endl;
        PEGASUS_TEST_ASSERT (0);
    }
    catch (...)
    {
        cerr << "Unknown error occurred in parent" << endl;
        PEGASUS_TEST_ASSERT (0);
    }
    return 0;
}
