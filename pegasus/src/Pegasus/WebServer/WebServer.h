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

#ifndef Pegasus_WebServer_h
#define Pegasus_WebServer_h

#include <Pegasus/WebServer/Linkage.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/MessageQueue.h>

#include <Pegasus/WebServer/WebProcessor.h>
#include <Pegasus/WebServer/WebRequest.h>


PEGASUS_NAMESPACE_BEGIN


/**
 *
 * Entry point for requests not handled by any of the default handlers
 * registered at class HTTPAuthenticationDelegator.
 *
 * Parses the httpMessages and passes the values of interest
 * wrapped in a WebRequest-object to the WebProcessor which will generate the
 * response. The response will be received by method handleResponse and send
 * via the HTTPConnection to the requester.
 */
class PEGASUS_WEBSERVER_LINKAGE WebServer : public MessageQueue
{
public:

    /**
     *
     */
    WebServer();

    /**
     *
     */
    ~WebServer();

    /**
     * Overridden for tracing purposes.
     */
    virtual void enqueue(Message* message);

    /**
     * Dequeues enqueued messages.
     */
    virtual void handleEnqueue();

    /**
     * Casts the message to an HTTPMessage.
     */
    virtual void handleEnqueue(Message* message);

    /**
     * Handles the HTTPMessage requests.
     */
    void handleHTTPMessage(HTTPMessage* httpMessage);

    /**
     * Sends the HTTPMessage response to the requester.
     */
    void handleResponse(HTTPMessage* response);

private:

    /**
     * Processes the parsed requests and generates the response messages.
     */
    WebProcessor _webProcessor;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WebServer_h */
