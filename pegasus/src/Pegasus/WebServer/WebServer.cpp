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


#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>

#include <Pegasus/WebServer/WebServer.h>


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

WebServer::WebServer()
    : MessageQueue("WebServer"),
      _webProcessor(this)
{

}


WebServer::~WebServer()
{
}

void WebServer::enqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER,
        "WebServer::enqueue()");
    handleEnqueue(message);
    PEG_METHOD_EXIT();
}

void WebServer::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_WEBSERVER,
        "WebServer::handleEnqueue()");

    Message* message = dequeue();
    handleEnqueue(message);

    PEG_METHOD_EXIT();
}

void WebServer::handleEnqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebServer::handleEnqueue()");

    if ( !message )
    {
        PEG_METHOD_EXIT();
        return;
    }

    switch (message->getType())
    {
        case HTTP_MESSAGE:
            handleHTTPMessage((HTTPMessage*)message);
            break;

        //Handles only the HTTP_MESSAGE, So this is unrechable
        default:
            PEGASUS_UNREACHABLE( PEGASUS_ASSERT(0);)
            break;
    }

    delete message;
    PEG_METHOD_EXIT();
}

void WebServer::handleHTTPMessage(HTTPMessage* httpMessage)
{
    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebServer::handleHTTPMessage()");

    if (!httpMessage)
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Parse the HTTP message:
    String startLine;
    Uint32 contentLength;
    Array<HTTPHeader> headers;

    httpMessage->parse(startLine, headers, contentLength);

    // ex.: 'Accept: text/html,application/xhtml+xml,
    //       application/xml;q=0.9,*/*;q=0.8'
    const char* acceptMimeTypesHeader;
    Boolean acceptMimeTypesHeaderFound = HTTPMessage::lookupHeader(
            headers, "Accept", acceptMimeTypesHeader, false);

    //ex.: 'Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7'
    const char* charSetHeader;
    Boolean charSetHeaderFound = HTTPMessage::lookupHeader(
        headers, "Accept-Charset", charSetHeader, false);

    //ex.: 'Accept-Encoding: gzip,deflate'
    //Not supported yet
    const char* encHeader = NULL;
    Boolean encHeaderFound = HTTPMessage::lookupHeader(
        headers, "Accept-Encoding", encHeader, false);


    // Parse the request line:
    String methodName;
    String requestUri;
    String httpVersion;

    HTTPMessage::parseRequestLine(
        startLine, methodName, requestUri, httpVersion);

    PEG_TRACE((TRC_WEBSERVER, Tracer::LEVEL4,
            "WebServer::handleHTTPMessage(HTTPMessage* httpMessage) - QueueID"
            " %d: methodName [%s], requestUri [%s], httpVersion [%s],"
            "mimeTypes [%s], charSets [%s], encoding [%s]",
            httpMessage->queueId,
            (const char*)methodName.getCString(),
            (const char*)requestUri.getCString(),
            (const char*)httpVersion.getCString(),
            (acceptMimeTypesHeaderFound ? acceptMimeTypesHeader : "NOT FOUND"),
            (charSetHeaderFound ? charSetHeader : "NOT FOUND"),
            (encHeaderFound ? encHeader : "NOT FOUND")));

    WebRequest* webRequest = new WebRequest(httpMessage->queueId);

    // set method name
    if ("GET" == methodName)
    {
        webRequest->httpMethod = HTTP_METHOD_GET;
    }
    else if ("HEAD" == methodName)
    {
        webRequest->httpMethod = HTTP_METHOD_HEAD;
    }

    if (acceptMimeTypesHeaderFound)
    {
        webRequest->mimeTypes = String(acceptMimeTypesHeader);
    }

    if (charSetHeaderFound)
    {
        webRequest->charSets = String(charSetHeader);
    }

    if (encHeaderFound)
    {
        webRequest->encodings = String(encHeader);
    }

    webRequest->requestURI = String(requestUri);
    webRequest->httpVersion = httpVersion;

    // Save userName and authType:
    webRequest->userName = httpMessage->authInfo->getAuthenticatedUser();
    webRequest->authType = httpMessage->authInfo->getAuthType();
    webRequest->ipAddress = httpMessage->ipAddress;
    
    //for ex.: 'Accept-Language: de-de,de;q=0.8,en-us;q=0.5,en;q=0.3'
    webRequest->acceptLanguages = httpMessage->acceptLanguages;

    /*
     * TODO async processing in thread.
     * Take care of userid/authorization
     */
    _webProcessor.handleWebRequest(webRequest);

    PEG_METHOD_EXIT();
}


void WebServer::handleResponse(HTTPMessage* response)
{

    PEG_METHOD_ENTER(TRC_WEBSERVER, "WebServer::handleResponse()");

    Uint32 queueId = response->queueId;
    MessageQueue* queue = MessageQueue::lookup(queueId);
    AutoPtr<HTTPMessage> httpMessage(response);

    if (!queue)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "ERROR: non-existent queueId = %u, response not sent.", queueId));
        PEG_METHOD_EXIT();
        return;
    }
    PEGASUS_ASSERT(dynamic_cast<HTTPConnection*>(queue) != 0);

    httpMessage->dest = queue->getQueueId();
    queue->enqueue(httpMessage.release());

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
