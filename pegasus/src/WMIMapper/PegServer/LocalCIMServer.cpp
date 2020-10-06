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

#include "LocalCIMServer.h"
#include "CIMOperationRequestDecoder.h"
#include "CIMOperationRequestDispatcher.h"
#include "CIMOperationResponseEncoder.h"

#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/HTTPMessage.h>


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


//
// WMILocalResponseQueue Class: Collects and announces the presence of
// WMI Mapper HTTP responses that arrive during local interface transactions.
//
class WMILocalResponseQueue : public MessageQueue
{
public:
    WMILocalResponseQueue(
        char* name)
        : MessageQueue(name),
          hasReceivedResponse(FALSE),
          httpMessage(NULL)
    { 
    }

    const Boolean& HasReceivedResponse() { return hasReceivedResponse; }
    const HTTPMessage* GetHTTPMessage() { return httpMessage; }

protected:
    Boolean hasReceivedResponse;
    HTTPMessage* httpMessage;

    void handleEnqueue(void)
    {
        Message* request = dequeue();

        if(request)
            handleEnqueue(request);
    }

    void handleEnqueue(Message *message)
    { 
        // Verify that an HTTP Message has arrived correctly
        PEGASUS_ASSERT(message->getType() == HTTP_MESSAGE);
        httpMessage = (HTTPMessage*)message;

        // Indicate that a successful response has been received.
        hasReceivedResponse = TRUE;
    }
};


LocalCIMServer::LocalCIMServer()
{
    PEG_METHOD_ENTER(TRC_SERVER, "LocalCIMServer::LocalCIMServer()");

    _cimOperationRequestDispatcher = new CIMOperationRequestDispatcher();
    _cimOperationResponseEncoder = new CIMOperationResponseEncoder();
    _cimOperationRequestDecoder = new CIMOperationRequestDecoder(
        _cimOperationRequestDispatcher,
        _cimOperationResponseEncoder->getQueueId());

    PEG_METHOD_EXIT();
}

LocalCIMServer::~LocalCIMServer()
{
    PEG_METHOD_ENTER(TRC_SERVER, "LocalCIMServer::~LocalCIMServer()");

    delete _cimOperationRequestDecoder;
    delete _cimOperationResponseEncoder;
    delete _cimOperationRequestDispatcher;

    PEG_METHOD_EXIT();
}

Buffer LocalCIMServer::handleHTTPRequest(const Buffer& request) 
//    throw(ConnectionTimeoutException)
{
    PEG_METHOD_ENTER(TRC_SERVER, "LocalCIMServer::handleHTTPRequest()");

    WMILocalResponseQueue localResponseQueue("WMILocalResponseQueue");

    // Create and HTTP Message containing collected input data and using our
    // local response queue to collect and display the response.
    HTTPMessage httpMessage(request, localResponseQueue.getQueueId());

    // Load the HTTP Message with an authInfo section, 
    // indicating local-direct authentication:
    httpMessage.authInfo = new AuthenticationInfo(false);
    httpMessage.authInfo->setAuthType(LOCAL_CALL_AUTH_TYPE);
    httpMessage.authInfo->setConnectionAuthenticated(true);

    _cimOperationRequestDecoder->handleHTTPMessage(&httpMessage);

    // Wait for a response or a timeout
    //
    // Future: use Windows event signalling or Pegasus monitor
    while( !localResponseQueue.HasReceivedResponse() ) 
    {
        Sleep(50); 
    }

    /* Future : Add the timeout logic and  Check for timeout
    if( !localResponseQueue.HasReceivedResponse() )
    {
        throw ConnectionTimeoutException();
    }
   */

    const HTTPMessage* httpResponse = localResponseQueue.GetHTTPMessage();
    PEGASUS_ASSERT(httpResponse != NULL);

    return httpResponse->message;

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
