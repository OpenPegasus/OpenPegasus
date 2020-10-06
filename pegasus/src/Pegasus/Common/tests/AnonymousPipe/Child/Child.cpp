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
#include <Pegasus/Common/AnonymousPipe.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/InternalException.h>

#if defined (PEGASUS_OS_VMS)
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main (int argc, char * argv [])
{
    Boolean verbose = (getenv ("PEGASUS_TEST_VERBOSE")) ? true : false;

    if (argc != 3)
    {
        cerr << "Usage: " << argv [0] <<
            " child-read-handle child-write-handle" << endl;
        PEGASUS_TEST_ASSERT (0);
    }

    String readHandleStr (argv [1]);
    CString readHandle = readHandleStr.getCString ();
    String writeHandleStr (argv [2]);
    CString writeHandle = writeHandleStr.getCString ();

    try
    {
        //
        //  Create pipe instances
        //
        AutoPtr <AnonymousPipe> pipeFromParent
            (new AnonymousPipe (readHandle, NULL));
        AutoPtr <AnonymousPipe> pipeToParent
            (new AnonymousPipe (NULL, writeHandle));

#if defined (PEGASUS_OS_VMS)
            //
            //  Child: Close parent handles
            //
            pipeFromParent->closeWriteHandle ();
            pipeToParent->closeReadHandle ();
#endif
        //
        //  Test readBuffer and writeBuffer
        //
        Uint32 bufferLength;
        AnonymousPipe::Status readBufferStatus = pipeFromParent->readBuffer
            ((char *) &bufferLength, sizeof (Uint32));

        if (readBufferStatus != AnonymousPipe::STATUS_SUCCESS)
        {
            cerr << "Child failed to read request length: "
                << readBufferStatus << endl;
            PEGASUS_TEST_ASSERT (0);
        }

        if (bufferLength == 0)
        {
            cerr << "Child read request length of 0" << endl;
            PEGASUS_TEST_ASSERT (0);
        }

        AutoArrayPtr <char> requestBuffer (new char [bufferLength + 1]);
        do
        {
            readBufferStatus = pipeFromParent->readBuffer (requestBuffer.get (),
                bufferLength);
        } while (readBufferStatus == AnonymousPipe::STATUS_INTERRUPT);

        if (readBufferStatus != AnonymousPipe::STATUS_SUCCESS)
        {
            cerr << "Child failed to read request data: " << readBufferStatus
                << endl;
            PEGASUS_TEST_ASSERT (0);
        }

        requestBuffer.get () [bufferLength] = '\0';
        if (verbose)
        {
            cout << "Request received by child: " << requestBuffer.get ()
                << endl;
        }

        PEGASUS_TEST_ASSERT (strcmp (requestBuffer.get (), "Hello world") == 0);

        Buffer responseBuffer;
        char buffer [16];
        sprintf (buffer, "%s", "Good-bye");
        responseBuffer.append (buffer, strlen (buffer));
        AnonymousPipe::Status writeBufferStatus;
        bufferLength = responseBuffer.size();

        writeBufferStatus = pipeToParent->writeBuffer
            ((const char *) &bufferLength, sizeof (Uint32));

        if (writeBufferStatus == AnonymousPipe::STATUS_SUCCESS)
        {
            writeBufferStatus = pipeToParent->writeBuffer
                (responseBuffer.getData (), bufferLength);
            if (writeBufferStatus != AnonymousPipe::STATUS_SUCCESS)
            {
                cerr << "Child failed to write response data: "
                    << writeBufferStatus << endl;
                PEGASUS_TEST_ASSERT (0);
            }
        }
        else
        {
            cerr << "Child failed to write response length: "
                << writeBufferStatus << endl;
            PEGASUS_TEST_ASSERT (0);
        }

        //
        //  Test readMessage and writeMessage
        //
        CIMMessage * message;
        pipeFromParent->readMessage(message);

        AutoPtr<CIMGetInstanceRequestMessage> request;
        request.reset(dynamic_cast<CIMGetInstanceRequestMessage*>(message));
        PEGASUS_TEST_ASSERT (request.get() != 0);

        if (verbose)
        {
            cout << "CIMGetInstanceRequestMessage received by child" << endl;
        }

        PEGASUS_TEST_ASSERT (request->getType () ==
            CIM_GET_INSTANCE_REQUEST_MESSAGE);
        PEGASUS_TEST_ASSERT (request->messageId == String ("00000001"));
        PEGASUS_TEST_ASSERT (request->nameSpace.equal
              (CIMNamespaceName ("root/test/A")));
        PEGASUS_TEST_ASSERT (request->instanceName ==
              CIMObjectPath ("MCCA_TestClass.theKey=1"));
        PEGASUS_TEST_ASSERT (request->includeQualifiers == false);
        PEGASUS_TEST_ASSERT (request->includeClassOrigin == false);
        PEGASUS_TEST_ASSERT (request->propertyList.isNull ());
        PEGASUS_TEST_ASSERT (request->authType == String::EMPTY);
        PEGASUS_TEST_ASSERT (request->userName == String::EMPTY);
        PEGASUS_TEST_ASSERT (
            ((ContentLanguageListContainer)request->operationContext.get
            (ContentLanguageListContainer::NAME)).getLanguages().size() == 0);
        PEGASUS_TEST_ASSERT (
            ((AcceptLanguageListContainer)request->operationContext.get
            (AcceptLanguageListContainer::NAME)).getLanguages().size() == 0);

        AcceptLanguageListContainer allc1(
            request->operationContext.get(AcceptLanguageListContainer::NAME));
        PEGASUS_TEST_ASSERT ( allc1.getLanguages().size() == 0 );
        AcceptLanguageListContainer allc2(allc1);
        PEGASUS_TEST_ASSERT ( allc2.getLanguages().size() == 0 );
        AcceptLanguageListContainer allc3 = allc2;
        PEGASUS_TEST_ASSERT ( allc3.getLanguages().size() == 0 );

        CIMInstance anInstance;
        AutoPtr <CIMGetInstanceResponseMessage> response
            (new CIMGetInstanceResponseMessage
                (String ("00000002"),
                CIMException (CIM_ERR_FAILED),
                QueueIdStack()));
        response->getResponseData().setInstance(anInstance);

        AnonymousPipe::Status writeMessageStatus =
            pipeToParent->writeMessage (response.get ());
        if (writeMessageStatus != AnonymousPipe::STATUS_SUCCESS)
        {
            cerr << "Child failed to write response message: "
                << writeMessageStatus << endl;
            PEGASUS_TEST_ASSERT (0);
        }
    }
    catch (Exception & e)
    {
        cerr << "Exception occurred in child: " << e.getMessage () << endl;
        PEGASUS_TEST_ASSERT (0);
    }
    catch (...)
    {
        cerr << "Unknown error occurred in child" << endl;
        PEGASUS_TEST_ASSERT (0);
    }

    return 0;
}
