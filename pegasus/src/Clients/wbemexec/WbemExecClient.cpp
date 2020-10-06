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

#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/AutoPtr.h>

#include "HttpConstants.h"
#include "WbemExecClient.h"
#include <Pegasus/Common/Network.h>
#include <iostream>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


static const char PASSWORD_PROMPT []  =
                     "Please enter your password: ";

static const char PASSWORD_BLANK []  =
                     "Password cannot be blank. Please re-enter your password.";

static const Uint32 MAX_PW_RETRIES =  3;


#ifdef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
// This callback is used when using SSL for a "local" connection.
static Boolean verifyServerCertificate(SSLCertificateInfo &certInfo)
{
    //
    // If server certificate was found in CA trust store and validated, then
    // return 'true' to accept the certificate, otherwise return 'false'.
    //
    if (certInfo.getResponseCode() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

WbemExecClient::WbemExecClient(Uint32 timeoutMilliseconds)
    :
    MessageQueue(PEGASUS_QUEUENAME_WBEMEXECCLIENT),
    _timeoutMilliseconds(timeoutMilliseconds),
    _connected(false),
    _isRemote(false)
{
    // CAUTION:
    //    Using private AutoPtr<> data members for these objects causes linker
    //    errors on some SOLARIS_SPARC_CC platforms.

    _monitor = 0;
    _httpConnector = 0;

    AutoPtr<Monitor> monitor(new Monitor());
    AutoPtr<HTTPConnector> httpConnector(new HTTPConnector(monitor.get()));

    _monitor = monitor.release();
    _httpConnector = httpConnector.release();
}

WbemExecClient::~WbemExecClient()
{
    disconnect();
    delete _httpConnector;
    delete _monitor;
}

void WbemExecClient::handleEnqueue()
{
}

void WbemExecClient::_connect()
{
    //
    // Attempt to establish a connection:
    //
    _httpConnection = _httpConnector->connect(
        _connectHost,
        _connectPortNumber,
        _connectSSLContext.get(),
        _timeoutMilliseconds,
        this);

    _connected = true;
    _httpConnection->setSocketWriteTimeout(_timeoutMilliseconds/1000+1);
}

void WbemExecClient::connect(
    const String& host,
    Uint32 portNumber,
    const SSLContext* sslContext,
    const String& userName,
    const String& password)
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    //
    //  If the host is empty, set hostName to "localhost"
    //
    String hostName = host;
    if (host == String::EMPTY)
    {
        hostName = "localhost";
    }

    //
    // Set authentication information
    //
    _authenticator.clear();

    if (userName.size())
    {
        _authenticator.setUserName(userName);
    }

    if (password.size())
    {
        _authenticator.setPassword(password);
        _password = password;
    }

    if (sslContext)
    {
        _connectSSLContext.reset(new SSLContext(*sslContext));
    }
    else
    {
        _connectSSLContext.reset();
    }
    _connectHost = hostName;
    _connectPortNumber = portNumber;

    _connect();
    _isRemote = true;
}

void WbemExecClient::connectLocal()
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    //
    // Set authentication type
    //
    _authenticator.clear();
    _authenticator.setAuthType(ClientAuthenticator::LOCAL);

#ifndef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    _connectSSLContext.reset();
    _connectHost = String::EMPTY;
    _connectPortNumber = 0;
    _connect();
#else

    try
    {
        //
        // Look up the WBEM HTTP port number for the local system
        //
        _connectPortNumber = System::lookupPort(WBEM_HTTP_SERVICE_NAME,
            WBEM_DEFAULT_HTTP_PORT);

        //
        //  Assign host
        //
        _connectHost.assign(System::getHostName());

        _connectSSLContext.reset();

        _connect();
    }
    catch (CannotConnectException&)
    {
#ifdef PEGASUS_HAS_SSL
        //
        // Look up the WBEM HTTPS port number for the local system
        //
        _connectPortNumber = System::lookupPort(WBEM_HTTPS_SERVICE_NAME,
            WBEM_DEFAULT_HTTPS_PORT);

        //
        //  Assign host
        //
        _connectHost.assign(System::getHostName());

        //
        // Create SSLContext
        //

        const char* pegasusHome = getenv("PEGASUS_HOME");

        String certpath = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_CERTIFICATEFILE);

        String randFile;

#ifdef PEGASUS_SSL_RANDOMFILE
        randFile = FileSystem::getAbsolutePath(
            pegasusHome, PEGASUS_SSLCLIENT_RANDOMFILE);
#endif

        _connectSSLContext.reset(
            new SSLContext(certpath, verifyServerCertificate, randFile));

        _connect();
#else
        throw;
#endif // PEGASUS_HAS_SSL
    }
#endif // PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
    _isRemote = false;
}

void WbemExecClient::disconnect()
{
    if (_connected)
    {
        //
        // Close the connection
        //
        _httpConnector->disconnect(_httpConnection);
        _httpConnection = 0;

        _authenticator.clear();

        _connected = false;
    }
}

void WbemExecClient::_reconnect()
{
    PEGASUS_ASSERT(_connected);
    _httpConnector->disconnect(_httpConnection);
    _httpConnection = 0;
    _connected = false;
    _connect();
}

/**

    Prompt for password.

    @return  String value of the user entered password

 */
String WbemExecClient::_promptForPassword()
{
    //
    // Password is not set, prompt for the old password once
    //
    String pw;
    Uint32 retries = 1;
    do
    {
        pw = System::getPassword(PASSWORD_PROMPT);

        if (pw == String::EMPTY)
        {
            if (retries < MAX_PW_RETRIES)
            {
                retries++;
            }
            else
            {
                break;
            }
            cerr << PASSWORD_BLANK << endl;
            continue;
        }
    }
    while (pw == String::EMPTY);
    return pw;
}


Buffer WbemExecClient::issueRequest(const Buffer& request)
{
    if (!_connected)
    {
        throw NotConnectedException();
    }

    HTTPMessage* httpRequest = new HTTPMessage(request);

    // Note:  A historical defect in the calculation of the Content-Length
    // header makes it possible that wbemexec input files exist with a
    // Content-Length value one larger than the actual content size.  Adding
    // and extra newline character to the end of the message keeps those old
    // scripts working.
    httpRequest->message << "\n";

    _authenticator.setRequestMessage(httpRequest);

    Boolean haveBeenChallenged = false;
    HTTPMessage* httpResponse;

    while (1)
    {
        HTTPMessage* httpRequestCopy =
            new HTTPMessage(*(HTTPMessage*)_authenticator.getRequestMessage());
        _addAuthHeader(httpRequestCopy);

        Message* response = _doRequest(httpRequestCopy);
        PEGASUS_ASSERT(response->getType() == HTTP_MESSAGE);
        httpResponse = (HTTPMessage*)response;

        // If we've already been challenged or if the response does not
        // contain a challenge, there is nothing more to do.

        String startLine;
        Array<HTTPHeader> headers;
        Uint32 contentLength;

        httpResponse->parse(startLine, headers, contentLength);

        if (haveBeenChallenged || !_checkNeedToResend(headers))
        {
            break;
        }

        // If the challenge contains a Connection: Close header, reestablish
        // the connection.

        String connectionHeader;

        if (HTTPMessage::lookupHeader(
                headers, "Connection", connectionHeader, false))
        {
            if (String::equalNoCase(connectionHeader, "Close"))
            {
                _reconnect();
            }
        }

        // Prompt for a password, if necessary

        if ((_password == String::EMPTY) && _isRemote)
        {
            _password = _promptForPassword();
            _authenticator.setPassword(_password);
        }
        haveBeenChallenged = true;
        delete httpResponse;
    }

    AutoPtr<HTTPMessage> origRequest(
        (HTTPMessage*)_authenticator.releaseRequestMessage());

    AutoPtr<HTTPMessage> destroyer(httpResponse);

    return httpResponse->message;
}

Message* WbemExecClient::_doRequest(HTTPMessage * request)
{
    // ATTN-RK-P2-20020416: We should probably clear out the queue first.
    PEGASUS_ASSERT(getCount() == 0);  // Shouldn't be any messages in our queue

    _httpConnection->enqueue(request);

    Uint64 startMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    Uint64 nowMilliseconds = startMilliseconds;
    Uint64 stopMilliseconds = nowMilliseconds + _timeoutMilliseconds;

    while (nowMilliseconds < stopMilliseconds)
    {
        //
        // Wait until the timeout expires or an event occurs:
        //
        _monitor->run(Uint32(stopMilliseconds - nowMilliseconds));

        //
        // Check to see if incoming queue has a message
        //

        Message* response = dequeue();

        if (response)
        {
            return response;
        }

        nowMilliseconds = TimeValue::getCurrentTime().toMilliseconds();
    }

    //
    // Throw timed out exception:
    //

    throw ConnectionTimeoutException();
}

void WbemExecClient::_addAuthHeader(HTTPMessage*& httpMessage)
{
    //
    // Add authentication headers to the message
    //
    String authHeader = _authenticator.buildRequestAuthHeader();
    if (authHeader != String::EMPTY)
    {
        // Find the separator between the start line and the headers, so we
        // can add the authorization header there.

        const char* messageStart = httpMessage->message.getData();
        char* headerStart =
            (char*)memchr(messageStart, '\n', httpMessage->message.size());

        if (headerStart)
        {
            // Handle a CRLF or LF separator
            if ((headerStart != messageStart) && (headerStart[-1] == '\r'))
            {
                headerStart[-1] = 0;
            }
            else
            {
                *headerStart = 0;
            }

            headerStart++;
            int remainingSize = httpMessage->message.size() -
                (headerStart - (char *) messageStart);

            // Build a new message with the original start line, the new
            // authorization header, and the original headers and content.

            Buffer newMessageBuffer;
            newMessageBuffer << messageStart << HTTP_CRLF;
            newMessageBuffer << authHeader << HTTP_CRLF;
            newMessageBuffer.append(headerStart, remainingSize);

            HTTPMessage* newMessage = new HTTPMessage(newMessageBuffer);
            delete httpMessage;
            httpMessage = newMessage;
        }
    }
}

Boolean WbemExecClient::_checkNeedToResend(const Array<HTTPHeader>& httpHeaders)
{
    try
    {
        return _authenticator.checkResponseHeaderForChallenge(httpHeaders);
    }
    catch(InvalidAuthHeader&)
    {
        // We're done, send (garbage) response back to the user.
        return false;
    }
}

PEGASUS_NAMESPACE_END
