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

#include "Network.h"
#include <iostream>
#include <cctype>
#include <cstdlib>
#include "Signal.h"
#include "Socket.h"
#include "TLS.h"
#include "HTTPConnection.h"
#include "HTTPAcceptor.h"
#include "Monitor.h"
#include "HTTPMessage.h"
#include "Tracer.h"
#include "Buffer.h"
#include "LanguageParser.h"

#include <Pegasus/Common/XmlWriter.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// Local routines:
//
////////////////////////////////////////////////////////////////////////////////

/*
 * string and number constants for HTTP sending/receiving
 */

// buffer size for sending receiving
static const Uint32 httpTcpBufferSize = 8192;

// string constants for HTTP header. "Name" represents strings on the left
// side of headerNameTerminator and "Value" represents strings on the right
// side of headerNameTerminator

#define headerNameTrailer "Trailer"
#undef CRLF
#define CRLF "\r\n"

static const char headerNameTransferTE[] = "TE";
static const char headerNameTransferEncoding[] = "transfer-encoding";
static const char headerNameContentLength[] = "content-length";
static const char headerValueTransferEncodingChunked[] = "chunked";
static const char headerValueTransferEncodingIdentity[] = "identity";
static const char headerValueTEchunked[] = "chunked";
static const char headerValueTEtrailers[] = "trailers";
static const char headerNameError[] = "CIMError";
static const char headerNameCode[] = "CIMStatusCode";
static const char headerNameDescription[] = "CIMStatusCodeDescription";
static const char headerNameOperation[] = "CIMOperation";
static const char headerNameContentLanguage[] = "Content-Language";

// the names comes from the HTTP specification on chunked transfer encoding

static const char headerNameTerminator[] = ": ";
static const char headerValueSeparator[] = ", ";
static const char headerLineTerminator[] = CRLF;
static const char chunkLineTerminator[] = CRLF;
static const char chunkTerminator[] = CRLF;
static const char chunkBodyTerminator[] = CRLF;
static const char trailerTerminator[] = CRLF;
static const char chunkExtensionTerminator[] = ";";

// string sizes

static const Uint32 headerNameContentLengthLength =
    sizeof(headerNameContentLength) - 1;
static const Uint32 headerValueTransferEncodingChunkedLength =
    sizeof(headerValueTransferEncodingChunked) - 1;
static const Uint32 headerNameTransferEncodingLength =
    sizeof(headerNameTransferEncoding) - 1;
static const Uint32 headerNameTerminatorLength =sizeof(headerNameTerminator)-1;
static const Uint32 headerLineTerminatorLength =sizeof(headerLineTerminator)-1;
static const Uint32 chunkLineTerminatorLength = sizeof(chunkLineTerminator)-1;
static const Uint32 chunkTerminatorLength = sizeof(chunkTerminator)-1;
static const Uint32 chunkBodyTerminatorLength = sizeof(chunkBodyTerminator)-1;
static const Uint32 trailerTerminatorLength = sizeof(trailerTerminator)-1;
static const Uint32 chunkExtensionTerminatorLength =
    sizeof(chunkExtensionTerminator) - 1;

// the number of bytes it takes to place a Uint32 into a string (minus null)
static const Uint32 numberAsStringLength = 10;

/*
 * given an HTTP status code, return the description. not all codes are listed
 * here. Unmapped codes result in the internal error string.
 * Add any required future codes here.
 */

static const String httpDetailDelimiter = headerValueSeparator;
static const String httpStatusInternal = HTTP_STATUS_INTERNALSERVERERROR;

static const char INTERNAL_SERVER_ERROR_CONNECTION_CLOSED_KEY[] =
    "Common.HTTPConnection.INTERNAL_SERVER_ERROR_CONNECTION_CLOSED";
static const char INTERNAL_SERVER_ERROR_CONNECTION_CLOSED[] =
    "Internal server error. Connection with IP address $0 closed.";

/*
 * throw given http code with detail, file, line
 * This is shared client/server code. The caller will decide what to do
 * with the thrown message
 */

static void _throwEventFailure(
    const String &status,
    const String &detail,
    const char *file,
    Uint32 line)
{
    String message = status + httpDetailDelimiter + detail;
    PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL1,
         (const char*)message.getCString());
    if (status == httpStatusInternal)
        throw AssertionFailureException(file, line, message);
    else throw Exception(message);
}

// throw a http exception. This is used for both client and server common code.
// The macro allows is used for file, line inclusion for debugging

#define _throwEventFailure(status, detail) \
  _throwEventFailure(status, String(detail), __FILE__, __LINE__)

#define _socketWriteError()                                                   \
    do                                                                        \
    {                                                                         \
        String failureText = PEGASUS_SYSTEM_NETWORK_ERRORMSG;                 \
                                                                              \
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,                        \
            "Could not write response to client. Client may have timed out. " \
            "Socket write failed with error: %s ",                            \
            (const char*)failureText.getCString()));                          \
                                                                              \
        throw SocketWriteError(failureText);                                  \
    }                                                                         \
    while (0)

static inline Uint32 _Min(Uint32 x, Uint32 y)
{
    return x < y ? x : y;
}


////////////////////////////////////////////////////////////////////////////////
//
// HTTPConnection
//
////////////////////////////////////////////////////////////////////////////////

Uint32 HTTPConnection::_idleConnectionTimeoutSeconds = 0;

#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
Mutex HTTPConnection::_idleConnectionTimeoutSecondsMutex;
#endif

void HTTPConnection::setIdleConnectionTimeout(
    Uint32 idleConnectionTimeoutSeconds)
{
#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
    AutoMutex lock(_idleConnectionTimeoutSecondsMutex);
#endif
    _idleConnectionTimeoutSeconds = idleConnectionTimeoutSeconds;
}

Uint32 HTTPConnection::getIdleConnectionTimeout()
{
#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
    AutoMutex lock(_idleConnectionTimeoutSecondsMutex);
#endif
    return _idleConnectionTimeoutSeconds;
}

/*
    Note: This method is called in client code for reconnecting with the Server
    and can also be used in the server code to check the connection status  and
    take appropriate actions.it checks whether the connection is alive by
    attempting to read 1 byte from the socket.This method MUST not be used when
    incoming data is expected from the connection.

    Returns TRUE when there is no data and peer has closed the connection
    gracefully or there is an unanticipated incoming data, returns FALSE
    otherwise. Note that this method does not consider the errors returned
    from read().
*/

Boolean HTTPConnection::needsReconnect()
{
    char buffer;

    int n =  _socket->peek(&buffer, sizeof(buffer));

    return n >= 0;
}

HTTPConnection::HTTPConnection(
    Monitor* monitor,
    SharedPtr<MP_Socket>& socket,
    const String& ipAddress,
    HTTPAcceptor* owningAcceptor,
    MessageQueue* outputMessageQueue)
    :
    Base(PEGASUS_QUEUENAME_HTTPCONNECTION),
    _monitor(monitor),
    _socket(socket),
    _ipAddress(ipAddress),
    _owningAcceptor(owningAcceptor),
    _outputMessageQueue(outputMessageQueue),
    _contentOffset(-1),
    _contentLength(-1),
    _connectionClosePending(false),
    _acceptPending(false),
    _httpMethodNotChecked(true),
    _internalError(false)
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPConnection::HTTPConnection");

    _authInfo.reset(new AuthenticationInfo(true));

#ifndef PEGASUS_OS_ZOS
    // Add SSL verification information to the authentication information
    if (_socket->isSecure())
    {
        if (_socket->isPeerVerificationEnabled() &&
            _socket->isCertificateVerified())
        {
            _authInfo->setConnectionAuthenticated(true);
            _authInfo->setAuthType(AuthenticationInfoRep::AUTH_TYPE_SSL);
            _authInfo->setClientCertificateChain(
                _socket->getPeerCertificateChain());
        }
    }
#else
        if (_socket->isClientAuthenticated())
        {
            _authInfo->setAuthType(_socket->getAuthType());
            _authInfo->setConnectionAuthenticated(true);
            _authInfo->setConnectionUser(_socket->getAuthenticatedUser());
        }
#endif

    _responsePending = false;
    _connectionRequestCount = 0;
    _transferEncodingChunkOffset = 0;

    PEG_TRACE((TRC_HTTP, Tracer::LEVEL3,
        "Connection IP address = %s",(const char*)_ipAddress.getCString()));

    _authInfo->setIpAddress(_ipAddress);

    PEG_METHOD_EXIT();
}

HTTPConnection::~HTTPConnection()
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPConnection::~HTTPConnection");

    // We need to acquire this mutex in order to give handleEnqueue()
    // a chance to finish processing. If we don't, we may run into a
    // situation where the connection is marked to be closed by the
    // idle connection timeout mechanism and there are no pending
    // responses (the _responsePending flag is cleared in
    // _handleWriteEvent()). This causes the monitor to clean up the
    // connection. But if processing is not out of
    // HTTPConnection::handleEnqueue(), we are running a risk of
    // accessing a deleted object and crashing cimserver.
    AutoMutex connectionLock(_connection_mut);
    _socket->close();

    PEG_METHOD_EXIT();
}

void HTTPConnection::enqueue(Message *message)
{
    handleEnqueue(message);
}

Boolean HTTPConnection::isActive()
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPConnection::isActive");
    if(needsReconnect())
    {
        PEG_METHOD_EXIT();
        return false;
    }
    else
    {
        PEG_METHOD_EXIT();
        return true;
    }
}


void HTTPConnection::handleInternalServerError(
    Uint32 respMsgIndex,
    Boolean isComplete)
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPConnection::handleInternalServerError");

    PEG_TRACE((TRC_HTTP, Tracer::LEVEL1,
        "Internal server error. Connection queue id : %u, IP address :%s, "
            "Response Index :%u, Response is Complete :%u.",
        getQueueId(),
        (const char*)_ipAddress.getCString(),
        respMsgIndex,
        isComplete));

    _internalError = true;
    Buffer buffer;
    HTTPMessage message(buffer);
    message.setIndex(respMsgIndex);
    message.setComplete(isComplete);
    AutoMutex connectionLock(_connection_mut);
    _handleWriteEvent(message);
    PEG_METHOD_EXIT();
}

void HTTPConnection::handleEnqueue(Message *message)
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPConnection::handleEnqueue");

    if (! message)
    {
        PEG_METHOD_EXIT();
        return;
    }
    // Lock monitor mutex before executing the message because, as part of the
    // processing, HTTPConnection calls back to the monitor to set
    // status (monitor::setStatus(...).
    // See bug 10044
    // monitor::setStatus() sets the monitor lock which, without this prior
    // monitor lock could result in a deadlock for HTTPConnection calls from
    // other threads. For example:
    //
    // monitor->_entriesLockMutex->dst->run-> httpConnection::run()
    // creates SocketMsg-> handleEnqueue-> _handleReadEvent->
    // Monitor::setstate(...)
    //
    // CIMOperationResponseEncoder::sendResponse->handleEnqueue->
    // _handleWriteEvent->(lock HTTPconnection->_closeConnection()
    //     Monitor::setState->_entriesMutex -- Deadlock
    //
    // TODO: There may be a more efficient way to handle this interaction than
    // completely mutual exclusion of the monitor and HTTPConnection but this
    // does remove the chance of deadlock. Today we are not sure what the
    // effect would be of another way to handle the setState
    AutoMutex monitorLock(_monitor->getLock());

    AutoMutex connectionLock(_connection_mut);

    switch (message->getType())
    {
        case SOCKET_MESSAGE:
        {
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                "HTTPConnection::handleEnqueue - SOCKET_MESSAGE");
            SocketMessage* socketMessage = (SocketMessage*)message;
            if (socketMessage->events & SocketMessage::READ)
                _handleReadEvent();
            break;
        }

        case HTTP_MESSAGE:
        {
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                "HTTPConnection::handleEnqueue - HTTP_MESSAGE");

            HTTPMessage* httpMessage = dynamic_cast<HTTPMessage*>(message);
            PEGASUS_ASSERT(httpMessage);

#ifdef PEGASUS_ENABLE_SESSION_COOKIES
            // inject cookie to responses
            String cookie = _authInfo->getCookie();
            if (!_isClient() && cookie.size() > 0)
            {
                String header = "\r\nSet-Cookie: " + cookie;
                httpMessage->injectHeader(header);
                // don't send cookie in subsequent messages
                _authInfo->setCookie("");
            }
#endif

            _handleWriteEvent(*httpMessage);
            break;
        }

        default:
            // ATTN: need unexpected message error!
            break;
    } // switch

    delete message;

    PEG_METHOD_EXIT();
}

/*
 * Used on Server side to close outstanding connections waiting for SSL
 * handshake to complete if timeout expired or to close idle connections if
 * idleConnectionTimeout config property value has specified.
 * Returns 'true' if connection is closed (or is closePending).
 * timeNow will be updated to current time if connection's _idleStartTime
 * is greater than timeNow.
*/

Boolean HTTPConnection::closeConnectionOnTimeout(struct timeval* timeNow)
{
    // if SSL Handshake is not complete.
    if (_acceptPending)
    {
        PEGASUS_ASSERT(!_isClient());
        if ((timeNow->tv_sec - _acceptPendingStartTime.tv_sec >
                 PEGASUS_SSL_ACCEPT_TIMEOUT_SECONDS) &&
            (timeNow->tv_sec > _acceptPendingStartTime.tv_sec))
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL4,
                "HTTPConnection: close acceptPending connection for timeout");
            _closeConnection();
            return true;  // return 'true' to indicate connection was closed
        }
    }
    // else if connection timeout is active
    else if (getIdleConnectionTimeout())
    {
        // For performance reasons timeNow is calculated only once in
        // Monitor. Update timeNow if connection's _idleStartTime has
        // more recent time.
        if (timeNow->tv_sec < _idleStartTime.tv_sec)
        {
            Time::gettimeofday(timeNow);
        }
        else if ((Uint32)(timeNow->tv_sec - _idleStartTime.tv_sec) >
            getIdleConnectionTimeout())
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL3,
                "HTTPConnection: close idle connection for timeout "
                "of %d seconds\n", getIdleConnectionTimeout()));
            _closeConnection();
            return true;  // return 'true' to indicate connection was closed
        }
    }
    return false;  // connection was not closed
}

/*
 * handle the message coming down from the above. This is shared client and
 * server code. If the message is coming in chunks, then validate the chunk
 * sequence number. If the message is being processed on the server side,
 * make sure the client has requested transfer encodings and/or trailers before
 * sending them. If not, the message must be queued up until a complete flag
 * has arrived.
 */

Boolean HTTPConnection::_handleWriteEvent(HTTPMessage& httpMessage)
{
    String httpStatusString;
    Buffer& buffer = httpMessage.message;
    Boolean isFirst = httpMessage.isFirst();
    Boolean isLast = httpMessage.isComplete();
    Sint32 totalBytesWritten = 0;
    Uint32 messageLength = buffer.size();

    try
    {
        // delivery behavior:
        // 1 handler.processing() : header + optional body?
        // 2 handler.deliver()    : 1+ fully XML encoded object(s)
        // 3 handler.complete()   : deliver() + isLast = true

        Uint32 bytesRemaining = messageLength;
        char *messageStart = (char *) buffer.getData();
        Uint32 bytesToWrite = httpTcpBufferSize;
        Uint32 messageIndex = httpMessage.getIndex();
        Boolean isChunkResponse = false;
        Boolean isChunkRequest = false;
        Boolean isFirstException = false;

        if (_isClient() == false)
        {
            if (isFirst == true)
            {
                _outgoingBuffer.clear();
                // tracks the message coming from above
                _transferEncodingChunkOffset = 0;
                _mpostPrefix.clear();
                cimException = CIMException();
            }
            else
            {
                // this is coming from our own internal code, therefore it is an
                // internal error. somehow the chunks came out of order.
                if (_transferEncodingChunkOffset+1 != messageIndex)
                    _throwEventFailure(
                        httpStatusInternal, "chunk sequence mismatch");
                _transferEncodingChunkOffset++;
            }

            // If there is an internal error on this connection, just return
            // from here if the current message is not the last message because
            // this connection will be closed once all messages are received.
            if (_internalError)
            {
                if (isLast)
                {
                    _responsePending = false;
                    _closeConnection();
                    Logger::put_l(
                        Logger::ERROR_LOG,
                        System::CIMSERVER,
                        Logger::SEVERE,
                        MessageLoaderParms(
                            INTERNAL_SERVER_ERROR_CONNECTION_CLOSED_KEY,
                            INTERNAL_SERVER_ERROR_CONNECTION_CLOSED,
                            _ipAddress));
                }

                // Cleanup Authentication Handle
                // currently only PAM implemented, see Bug#9642
                _authInfo->getAuthHandle().destroy();
                return true;
            }

            // save the first error
            if (httpMessage.cimException.getCode() != CIM_ERR_SUCCESS)
            {
                httpStatusString = httpMessage.cimException.getMessage();
                if (cimException.getCode() == CIM_ERR_SUCCESS)
                {
                    cimException = httpMessage.cimException;
                    // set language to first error language (overriding
                    // anything there)
                    contentLanguages = cimException.getContentLanguages();
                    isFirstException = true;
                }
            }
            else if (cimException.getCode() == CIM_ERR_SUCCESS)
            {
                if (isFirst == true)
                    contentLanguages = httpMessage.contentLanguages;
                else if (httpMessage.contentLanguages != contentLanguages)
                    contentLanguages.clear();
                else contentLanguages = httpMessage.contentLanguages;
            }
            // check to see if the client requested chunking OR trailers.
            // trailers are tightly integrated with chunking, so it can also
            // be used.

            if (isChunkRequested() == true)
            {
                isChunkRequest = true;
            }
            else
            {
                // we are not sending chunks because the client did not
                // request it

                // save the entire FIRST error response for non-chunked error
                // responses this will be used as the error message

                if (isFirstException)
                {
                    PEGASUS_ASSERT(messageLength != 0);
                    cimException = CIMException(cimException.getCode(),
                        String(messageStart, messageLength));
                }

                if (isFirst == false)
                {
                    // subsequent chunks from the server, just append

                    messageLength += _outgoingBuffer.size();
                    _outgoingBuffer.reserveCapacity(messageLength+1);
                    _outgoingBuffer.append(buffer.getData(), buffer.size());
                    buffer.clear();
                    // null terminate
                    messageStart = (char *) _outgoingBuffer.getData();
                    messageStart[messageLength] = 0;
                    // put back in buffer, so the httpMessage parser can work
                    // below
                    _outgoingBuffer.swap(buffer);
                }

                if (isLast == false)
                {
                    // this tells the send loop below to do nothing until we
                    // are at the last response
                    bytesRemaining = 0;
                }
                else
                {
                    if (cimException.getCode() != CIM_ERR_SUCCESS)
                    {
                        buffer.clear();
                        // discard all data collected to this point
                        _outgoingBuffer.clear();
                        String messageS = cimException.getMessage();
                        CString messageC = messageS.getCString();
                        messageStart = (char *) (const char *) messageC;
                        messageLength = (Uint32)strlen(messageStart);
                        buffer.reserveCapacity(messageLength+1);
                        buffer.append(messageStart, messageLength);
                        // null terminate
                        messageStart = (char *) buffer.getData();
                        messageStart[messageLength] = 0;
                        // Error messages are always encoded non-binary
                        httpMessage.binaryResponse = false;
                    }
                    bytesRemaining = messageLength;
                }

            } // if not sending chunks

            // We now need to adjust the contentLength line.
            // If chunking was requested and this is the first chunk, then
            // we need to enter this block so we can adjust the header and
            // send to the client the first set of bytes right away.
            // If chunking was NOT requested, we have to wait for the last
            // chunk of the message to get (and set) the size of the content
            // because we are going to send it the traditional (i.e
            // non-chunked) way

            if ((isChunkRequest && isFirst) || (!isChunkRequest && isLast))
            {
                // need to find the end of the header
                String startLine;
                Array<HTTPHeader> headers;
                Uint32 contentLength = 0;

                // Note: this gets the content length from subtracting the
                // header length from the messageLength, not by parsing the
                // content length header field

                httpMessage.parse(startLine, headers, contentLength);
                Uint32 httpStatusCode = 0;
                String httpVersion;
                String reasonPhrase;
                Boolean isValid = httpMessage.parseStatusLine(
                    startLine, httpVersion, httpStatusCode, reasonPhrase);

                Uint32 headerLength = messageLength - contentLength;
                if (!isChunkRequest)
                {
                    if (contentLanguages.size() != 0)
                    {
                        // we must insert the content-language into the
                        // header
                        Buffer contentLanguagesString;

                        // this is the keyword:value(s) + header line
                        // terminator
                        contentLanguagesString <<
                            headerNameContentLanguage <<
                            headerNameTerminator <<
                            LanguageParser::buildContentLanguageHeader(
                                contentLanguages).getCString() <<
                            headerLineTerminator;

                        Uint32 insertOffset =
                            headerLength - headerLineTerminatorLength;
                        messageLength =
                            contentLanguagesString.size() + buffer.size();

                        // Adding 8 bytes to capacity, since in the
                        // binary case we might add up to 7 null bytes
                        buffer.reserveCapacity(messageLength+8);
                        messageLength = contentLanguagesString.size();
                        messageStart=(char *)contentLanguagesString.getData();

                        // insert the content language line before end
                        // of header
                        // note: this can be expensive on large payloads

                        if (!httpMessage.binaryResponse)
                        {
                            buffer.insert(
                                insertOffset, messageStart, messageLength);
                        }
                        else
                        {
                            // Need to fixup the binary alignment 0 bytes
                            // delete bytes if new is smaller than old
                            // add bytes if old is smaller than new
                            // if new and old amount equal -> do nothing

                            // ((a+7) & ~7) <- round up to the next highest
                            // number dividable by eight
                            Uint32 extraNullBytes =
                                ((headerLength + 7) & ~7) - headerLength;
                            Uint32 newHeaderSize =
                                headerLength+contentLanguagesString.size();
                            Uint32 newExtraNullBytes =
                                ((newHeaderSize + 7) & ~7) - newHeaderSize;

                            if (extraNullBytes > newExtraNullBytes)
                            {
                                buffer.insertWithOverlay(
                                    insertOffset,
                                    messageStart,
                                    messageLength,
                                    extraNullBytes-newExtraNullBytes);

                                contentLength -=
                                    (extraNullBytes-newExtraNullBytes);
                            }
                            else
                            {
                                Uint32 reqNullBytes =
                                    newExtraNullBytes - extraNullBytes;
                                contentLanguagesString << headerLineTerminator;
                                messageLength += headerLineTerminatorLength;
                                // Cleverly attach the extra bytes upfront
                                // to the contentLanguagesString
                                for (Uint32 i = 0; i < reqNullBytes; i++)
                                {
                                    contentLanguagesString.append('\0');
                                }
                                messageLength+=reqNullBytes;
                                buffer.insertWithOverlay(
                                    insertOffset,
                                    messageStart,
                                    messageLength,
                                    headerLineTerminatorLength);

                                contentLength+=reqNullBytes;
                            }
                        }
                        // null terminate
                        messageLength = buffer.size();
                        messageStart = (char *) buffer.getData();
                        messageStart[messageLength] = 0;
                        bytesRemaining = messageLength;
                    } // if there were any content languages

                } // if chunk request is false

                headerLength = messageLength - contentLength;

                char save = messageStart[headerLength];
                messageStart[headerLength] = 0;

                char *contentLengthStart =
                    strstr(messageStart, headerNameContentLength);

                char* contentLengthEnd = contentLengthStart ?
                    strstr(contentLengthStart, headerLineTerminator) : 0;

                messageStart[headerLength] = save;

                // the message may or may not have the content length specified
                // depending on the type of request it is

                if (contentLengthStart)
                {
                    // the message has the content length specified.
                    // If we are NOT sending a chunked response, then we need
                    // to overlay the contentLength number to reflect the
                    // actual byte count of the content (i.e message body).
                    // If we ARE sending a chunked response, then we will
                    // overlay the transferEncoding keyword name and value
                    // on top of the contentLength keyword and value.

                    // Important note:
                    // for performance reasons, the contentLength and/or
                    // transferEncoding strings are being overlayed
                    // DIRECTLY inside the message buffer WITHOUT changing
                    // the actual length in bytes of the message.
                    // The XmlWriter has been modified to pad out the
                    // maximum number in zeros to accomodate any number.
                    // The maximum contentLength name and value is identical
                    // to the transferEncoding name and value and can
                    // be easily interchanged. By doing this, we do not have
                    // to piece together the header (and more importantly,
                    // the lengthy body) all over again!
                    // This is why the http line lengths are validated below

                    Uint32 transferEncodingLineLengthExpected =
                        headerNameTransferEncodingLength +
                        headerNameTerminatorLength +
                        headerValueTransferEncodingChunkedLength;

                    Uint32 contentLengthLineLengthExpected =
                        headerNameContentLengthLength +
                        headerNameTerminatorLength + numberAsStringLength;

                    Uint32 contentLengthLineLengthFound =
                        (Uint32)(contentLengthEnd - contentLengthStart);

                    if (isValid == false || ! contentLengthEnd ||
                            contentLengthLineLengthFound !=
                            transferEncodingLineLengthExpected ||
                            transferEncodingLineLengthExpected !=
                            contentLengthLineLengthExpected)
                    {
                        // these should match up since this is coming
                        // directly from our code in XmlWriter! If not,
                        // some code changes have got out of sync

                        _throwEventFailure(httpStatusInternal,
                            "content length was incorrectly formatted");
                    }

                    // we will be sending a chunk response if:
                    // 1. chunking has been requested AND
                    // 2. contentLength has been set
                    //    (meaning a non-bodyless message has come in) OR
                    // 3. this is not the last message
                    //    (meaning the data is coming in pieces and we should
                    //    send chunked)

                    if (isChunkRequest == true &&
                        (contentLength > 0 || isLast == false))
                    {
                        isChunkResponse = true;
                    }

                    save = contentLengthStart[contentLengthLineLengthExpected];
                    contentLengthStart[contentLengthLineLengthExpected] = 0;

                    // overlay the contentLength value
                    if (isChunkResponse == false)
                    {
                        // overwrite the content length number with the actual
                        // byte count
                        char *contentLengthNumberStart = contentLengthStart +
                            headerNameContentLengthLength +
                            headerNameTerminatorLength;
                        char format[6];
                        sprintf (format, "%%.%uu", numberAsStringLength);
                        // overwrite the bytes in buffer with the content
                        //encoding length
                        sprintf(contentLengthNumberStart,
                            format, contentLength);
                        contentLengthStart[contentLengthLineLengthExpected] =
                            save;
                    }
                    else
                    {
                        // overlay the contentLength name and value with the
                        // transferEncoding name and value

                        sprintf(contentLengthStart, "%s%s%s",
                            headerNameTransferEncoding,
                            headerNameTerminator,
                            headerValueTransferEncodingChunked);
                        bytesToWrite = messageLength - contentLength;

                        contentLengthStart[contentLengthLineLengthExpected] =
                            save;
                        // look for 2-digit prefix (if mpost was use)
                        HTTPMessage::lookupHeaderPrefix(
                            headers,
                            headerNameOperation,
                            _mpostPrefix);
                    } // else chunk response is true

                } // if content length was found

            } // if this is the first chunk containing the header
            else
            {
                // if chunking was requested, then subsequent messages that are
                // received need to turn response chunking on

                if (isChunkRequest == true && messageIndex > 0)
                {
                    isChunkResponse = true;
                    bytesToWrite = messageLength;
                }
            }

            // the data is sitting in buffer, but we want to cache it in
            // _outgoingBuffer because there may be more chunks to append
            if (isChunkRequest == false)
                _outgoingBuffer.swap(buffer);

        } // if not a client

        PEG_TRACE_CSTRING(TRC_HTTP,Tracer::LEVEL4,
                "HTTPConnection::_handleWriteEvent: Server write event.");

        // All possible fix ups have been done, trace the result
        PEG_TRACE((TRC_XML_IO, Tracer::LEVEL4,
            "<!-- Response: queue id: %u -->\n%s",
            getQueueId(),
            Tracer::traceFormatChars(
                Buffer(messageStart,bytesRemaining),
                httpMessage.binaryResponse).get()));

        SignalHandler::ignore(PEGASUS_SIGPIPE);

        const char *sendStart = messageStart;
        Sint32 bytesWritten = 0;

        if (isFirst == true && isChunkResponse == true && bytesToWrite > 0)
        {
            // send the header first for chunked reponses.

            // dont include header terminator yet
            Uint32 headerLength = bytesToWrite;
            bytesToWrite -= headerLineTerminatorLength;
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                "HTTPConnection::_handleWriteEvent: "
                    "Sending header for chunked reponses.");

            bytesWritten = _socket->write(sendStart, bytesToWrite);
            if (bytesWritten < 0)
                _socketWriteError();
            totalBytesWritten += bytesWritten;
            bytesRemaining -= bytesWritten;

            // put in trailer header.
            Buffer trailer;
            trailer << headerNameTrailer << headerNameTerminator <<
                _mpostPrefix << headerNameCode <<    headerValueSeparator <<
                _mpostPrefix << headerNameDescription << headerValueSeparator <<
                headerNameContentLanguage << headerLineTerminator;

            sendStart = trailer.getData();
            bytesToWrite = trailer.size();

            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                "HTTPConnection::_handleWriteEvent: "
                    "Sending trailer header for chunked responses.");

            bytesWritten = _socket->write(sendStart, bytesToWrite);

            if (bytesWritten < 0)
                _socketWriteError();
            totalBytesWritten += bytesWritten;
            // the trailer is outside the header buffer, so dont include in
            // tracking variables

            // now send header terminator
            bytesToWrite = headerLineTerminatorLength;
            sendStart = messageStart + headerLength - bytesToWrite;

            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                "HTTPConnection::_handleWriteEvent: "
                    "Sending header terminator for chunked responses.");

            bytesWritten = _socket->write(sendStart, bytesToWrite);
            if (bytesWritten < 0)
                _socketWriteError();
            totalBytesWritten += bytesWritten;
            bytesRemaining -= bytesWritten;

            messageStart += headerLength;
            messageLength -= headerLength;
            sendStart = messageStart;
            bytesWritten = 0;
            bytesToWrite = bytesRemaining;
        } // if first chunk of chunked response

        // room enough for hex string representing chunk length and terminator
        char chunkLine[sizeof(Uint32)*2 + chunkLineTerminatorLength+1];

        for (; bytesRemaining > 0; )
        {
            if (isChunkResponse == true)
            {
                // send chunk line containing hex string and chunk line
                // terminator
                sprintf(chunkLine, "%x%s", bytesToWrite, chunkLineTerminator);
                sendStart = chunkLine;
                Sint32 chunkBytesToWrite = (Sint32)strlen(sendStart);

                PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                    "HTTPConnection::_handleWriteEvent: "
                        "Sending chunk with chunk line terminator.");

                bytesWritten = _socket->write(sendStart, chunkBytesToWrite);
                if (bytesWritten < 0)
                    _socketWriteError();
                totalBytesWritten += bytesWritten;
            }

            // for chunking, we will send the entire chunk data in one send, but
            // for non-chunking, we will send incrementally
            else bytesToWrite = _Min(bytesRemaining, bytesToWrite);

            // send non-chunked data
            //
            // Socket writes larger than 64K cause some platforms to return
            //  errors. When the socket write can't send the full buffer at
            //  one time, subtract the bytes sent and loop until the whole
            //  buffer has gone.  Use httpTcpBufferSize for maximum send size.
            //
            for (; bytesRemaining > 0; )
            {
              sendStart = messageStart + messageLength - bytesRemaining;
              bytesToWrite = _Min(httpTcpBufferSize, bytesRemaining);

              PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                  "HTTPConnection::_handleWriteEvent: "
                      "Sending non-chunked data.");

              bytesWritten = _socket->write(sendStart, bytesToWrite);
              if (bytesWritten < 0)
                  _socketWriteError();
              totalBytesWritten += bytesWritten;
              bytesRemaining -= bytesWritten;
            }

            if (isChunkResponse == true)
            {
                // send chunk terminator, on the last chunk, it is the chunk
                // body terminator
                Buffer trailer;
                Boolean traceTrailer = false;
                trailer << chunkLineTerminator;

                // on the last chunk, attach the last chunk termination
                // sequence: 0 + last chunk terminator + optional trailer +
                // chunkBodyTerminator

                if (isLast == true)
                {
                    if (bytesRemaining > 0)
                        _throwEventFailure(httpStatusInternal,
                            "more bytes after indicated last chunk");
                    trailer << "0" << chunkLineTerminator;
                    Uint32 httpStatus = cimException.getCode();

                    if (httpStatus != 0)
                    {
                        char httpStatusP[11];
                        sprintf(httpStatusP, "%u",httpStatus);

                        traceTrailer = true;
                        trailer << _mpostPrefix << headerNameCode <<
                            headerNameTerminator << httpStatusP <<
                            headerLineTerminator;
                        const String& httpDescription =
                            cimException.getMessage();
                        if (httpDescription.size() != 0)
                            trailer << _mpostPrefix << headerNameDescription <<
                                headerNameTerminator << httpDescription <<
                                headerLineTerminator;
                    }

                    // Add Content-Language to the trailer if requested
                    if (contentLanguages.size() != 0)
                    {
                        traceTrailer = true;
                        trailer << _mpostPrefix
                            << headerNameContentLanguage << headerNameTerminator
                            << LanguageParser::buildContentLanguageHeader(
                                   contentLanguages)
                            << headerLineTerminator;
                    }

                    // now add chunkBodyTerminator
                    trailer << chunkBodyTerminator;
                } // if isLast

                if (traceTrailer)
                {
                    PEG_TRACE((TRC_XML_IO, Tracer::LEVEL4,
                        "<!-- Trailer: queue id: %u -->\n%s",
                        getQueueId(),
                        Tracer::traceFormatChars(
                            trailer,
                            httpMessage.binaryResponse).get()));
                }
                sendStart = trailer.getData();
                Sint32 chunkBytesToWrite = (Sint32) trailer.size();

                PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                    "HTTPConnection::_handleWriteEvent: "
                        "Sending the last chunk with chunk body terminator");

                bytesWritten = _socket->write(sendStart, chunkBytesToWrite);
                if (bytesWritten < 0)
                    _socketWriteError();
                totalBytesWritten += bytesWritten;
            } // isChunkResponse == true

        } // for all bytes in message

    } // try

    catch (SocketWriteError &e)
    {
        // On the server side, the socket write error is suppressed
        // and not handled as an internal error.
        httpStatusString = e.getMessage();
    }
    catch (Exception &e)
    {
        httpStatusString = e.getMessage();
        _internalError = true;
    }
    catch (PEGASUS_STD(bad_alloc)&)
    {
        httpStatusString = "Out of memory";
        _internalError = true;
    }
    catch (...)
    {
        httpStatusString = "Unknown error";
        _internalError = true;
    }

    if (httpStatusString.size())
    {
        PEG_TRACE((TRC_HTTP, Tracer::LEVEL1,
            "Internal error: %s, connection queue id: %u",
            (const char*)httpStatusString.getCString(),
            getQueueId()));
    }

    if (isLast == true)
    {
        _outgoingBuffer.clear();
        _transferEncodingTEValues.clear();

        // Reset the transfer encoding chunk offset. If it is not reset here,
        // then a request sent with chunked encoding may not be properly read
        // off of a connection in which a chunked response has been sent.
        _transferEncodingChunkOffset = 0;

        //
        // decrement request count
        //

        _responsePending = false;

        if (httpStatusString.size() == 0)
        {
            PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                "A response has been sent (%d of %u bytes have been written). "
                    "A total of %u requests have been processed on this "
                    "connection.",
                totalBytesWritten,
                messageLength,
                _connectionRequestCount));
        }

        //
        // Since we are done writing, update the status of entry to IDLE
        // and notify the Monitor.
        //
        if (_isClient() == false)
        {
            // Cleanup Authentication Handle
            // currently only PAM implemented, see Bug#9642
            _authInfo->getAuthHandle().destroy();

            if (_internalError)
            {
                _closeConnection();
                Logger::put_l(
                    Logger::ERROR_LOG,
                    System::CIMSERVER,
                    Logger::SEVERE,
                    MessageLoaderParms(
                        INTERNAL_SERVER_ERROR_CONNECTION_CLOSED_KEY,
                        INTERNAL_SERVER_ERROR_CONNECTION_CLOSED,
                        _ipAddress));
            }
            // Check for message to close
            else if (httpMessage.getCloseConnect())
            {
                PEG_TRACE((TRC_HTTP, Tracer::LEVEL3,
                    "HTTPConnection::_handleWriteEvent: \"Connection: Close\" "
                        "in client message."));
                _closeConnection();
            }
            else
            {
                // Update connection idle time.
                if (getIdleConnectionTimeout())
                {
                    Time::gettimeofday(&_idleStartTime);
                }
                PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                    "Now setting state to %d", MonitorEntry::STATUS_IDLE));
                _monitor->setState(_entry_index, MonitorEntry::STATUS_IDLE);
                _monitor->tickle();
            }
            cimException = CIMException();
        }
    }

    return httpStatusString.size() == 0 ? false : true;

}

void HTTPConnection::handleEnqueue()
{
   Message* message = dequeue();

    if (!message)
        return;
    handleEnqueue(message);
}

Boolean _IsBodylessMessage(const char* line)
{
    //ATTN: Make sure this is the right place to check for HTTP/1.1 and
    //      HTTP/1.0 that is part of the authentication challenge header.
    // ATTN-RK-P2-20020305: How do we make sure we have the complete list?

    // List of request methods which do not have message bodies
    const char* METHOD_NAMES[] =
    {
        "GET",
        "HEAD",
        "OPTIONS",
        "DELETE"
    };

    // List of response codes which the client accepts and which should not
    // (normally) have message bodies.  The RFC is vague regarding which
    // response codes support or require bodies.  These are being reported by
    // class (4xx, 5xx, etc) because the CIM client should be able to handle
    // any status code, including those not explicitly defined in RFC 2616.
    // Therefore, listing codes individually will not work because the client
    // socket will hang on a code not in this list if no content length is
    // specified.
    // See bugzilla 1586
    const char* RESPONSE_CODES[] =
    {
        "HTTP/1.1 3XX",
        "HTTP/1.0 3XX",
        "HTTP/1.1 4XX",
        "HTTP/1.0 4XX",
        "HTTP/1.1 5XX",
        "HTTP/1.0 5XX"
    };

    // Check for bodyless HTTP request method
    const Uint32 METHOD_NAMES_SIZE = sizeof(METHOD_NAMES) / sizeof(char*);

    for (Uint32 i = 0; i < METHOD_NAMES_SIZE; i++)
    {
        Uint32 n = (Uint32)strlen(METHOD_NAMES[i]);

        if (strncmp(line, METHOD_NAMES[i], n) == 0 && isspace(line[n]))
            return true;
    }

    // Check for bodyless HTTP status code
    const Uint32 RESPONSE_CODES_SIZE = sizeof(RESPONSE_CODES) / sizeof(char*);

    for (Uint32 i = 0; i < RESPONSE_CODES_SIZE; i++)
    {
        Uint32 n = (Uint32)strlen(RESPONSE_CODES[i]);

        if (strncmp(line, RESPONSE_CODES[i], n - 2) == 0 && isspace(line[n]))
                return true;
            }

    return false;
}

/*
Boolean _IsBodylessMessage(const char* line)
{
    //ATTN: Make sure this is the right place to check for HTTP/1.1 and
    //      HTTP/1.0 that is part of the authentication challenge header.
    // ATTN-RK-P2-20020305: How do we make sure we have the complete list?
    const char* METHOD_NAMES[] =
    {
        "GET",
        "HTTP/1.1 400",
        "HTTP/1.0 400",
        "HTTP/1.1 401",
        "HTTP/1.0 401",
        "HTTP/1.1 413",
        "HTTP/1.0 413",
        "HTTP/1.1 500",
        "HTTP/1.0 500",
        "HTTP/1.1 501",
        "HTTP/1.0 501",
        "HTTP/1.1 503",
        "HTTP/1.0 503"
    };

    const Uint32 METHOD_NAMES_SIZE = sizeof(METHOD_NAMES) / sizeof(char*);

    for (Uint32 i = 0; i < METHOD_NAMES_SIZE; i++)
    {
        Uint32 n = strlen(METHOD_NAMES[i]);

        if (strncmp(line, METHOD_NAMES[i], n) == 0 && isspace(line[n]))
            return true;
    }

    return false;
}*/

void HTTPConnection::_getContentLengthAndContentOffset()
{
    Uint32 size = _incomingBuffer.size();
    if (size == 0)
        return;
    char* data = (char*)_incomingBuffer.getData();
    char* line = (char*)data;
    char* sep;
    Uint32 lineNum = 0;
    Boolean bodylessMessage = false;
    Boolean gotContentLength = false;
    Boolean gotTransferEncoding = false;
    Boolean gotContentLanguage = false;
    Boolean gotTransferTE = false;

    while ((sep = HTTPMessage::findSeparator(line)))
    {
        char save = *sep;
        *sep = '\0';

        // Did we find the double separator which terminates the headers?

        if (line == sep)
        {
            *sep = save;
            line = sep + ((save == '\r') ? 2 : 1);
            _contentOffset = (Sint32)(line - _incomingBuffer.getData());

            // reserve space for entire non-chunked message
            if (_contentLength > 0)
            {
                try
                {
                    Uint32 capacity = (Uint32)(_contentLength +
                        _contentOffset + 1);
                    _incomingBuffer.reserveCapacity(capacity);
                    data = (char *)_incomingBuffer.getData();
                    data[capacity-1] = 0;
                }
                catch (const PEGASUS_STD(bad_alloc)&)
                {
                    _throwEventFailure(HTTP_STATUS_REQUEST_TOO_LARGE,
                        "Error reserving space for non-chunked message");
                }
                catch (...)
                {
                    _throwEventFailure(
                        httpStatusInternal, "unexpected exception");
                }
            }

            break;
        }

        // If this is one of the bodyless methods, then we can assume the
        // message is complete when the "\r\n\r\n" is encountered.

        if (lineNum == 0 && _IsBodylessMessage(line))
            bodylessMessage = true;

        // Look for the content-length if not already found:

        char* colon = strchr(line, ':');

        if (colon)
        {
            *colon  = '\0';

            // remove whitespace after colon before value
            char *valueStart = colon + 1;
            while (*valueStart == ' ' || *valueStart == '\t')
                valueStart++;

            // we found some non-whitespace token
            if (valueStart != sep)
            {
                char *valueEnd = sep - 1;

                // now remove whitespace from end of line back to last byte
                // of value
                while (*valueEnd == ' ' || *valueEnd == '\t')
                    valueEnd--;

                char valueSave = *(valueEnd+1);

                if (System::strcasecmp(line, headerNameContentLength) == 0)
                {
                    if (gotContentLength)
                    {
                        _throwEventFailure(HTTP_STATUS_BADREQUEST,
                            "Duplicate Content-Length header detected");
                    }
                    gotContentLength = true;

                    if (_transferEncodingValues.size() == 0)
                    {
                        // Use a dummy character conversion to catch an
                        // invalid character in the value.
                        char dummy;
                        if (sscanf(valueStart, "%d%c",
                                &_contentLength, &dummy) != 1)
                        {
                            _throwEventFailure(HTTP_STATUS_BADREQUEST,
                                "Invalid Content-Length header detected");
                        }
                    }
                    else
                    {
                        _contentLength = -1;
                    }
                }
                else if (System::strcasecmp(
                             line, headerNameTransferEncoding) == 0)
                {
                    if (gotTransferEncoding)
                    {
                        _throwEventFailure(HTTP_STATUS_BADREQUEST,
                            "Duplicate Transfer-Encoding header detected");
                    }
                    gotTransferEncoding = true;

                    _transferEncodingValues.clear();

                    if (strcmp(valueStart,
                            headerValueTransferEncodingChunked) == 0)
                        _transferEncodingValues.append(
                            headerValueTransferEncodingChunked);
                    else if (strcmp(valueStart,
                                 headerValueTransferEncodingIdentity) == 0)
                        ; // do nothing
                    else _throwEventFailure(HTTP_STATUS_NOTIMPLEMENTED,
                             "unimplemented transfer-encoding value");
                    _contentLength = -1;
                }
                else if (System::strcasecmp(
                             line, headerNameContentLanguage) == 0)
                {
                    // note: if this is a chunked header, then this will be
                    // ignored later
                    String contentLanguagesString(
                        valueStart, (Uint32)(valueEnd - valueStart + 1));
                    try
                    {
                        ContentLanguageList contentLanguagesValue =
                            LanguageParser::parseContentLanguageHeader(
                                contentLanguagesString);

                        if (gotContentLanguage)
                        {
                            // Append these content languages to the existing
                            // list.
                            for (Uint32 i = 0;
                                 i < contentLanguagesValue.size(); i++)
                            {
                                contentLanguages.append(
                                    contentLanguagesValue.getLanguageTag(i));
                            }
                        }
                        else
                        {
                            contentLanguages = contentLanguagesValue;
                            gotContentLanguage = true;
                        }
                    }
                    catch (...)
                    {
                        PEG_TRACE((TRC_HTTP, Tracer::LEVEL2,
                            "HTTPConnection: ERROR: contentLanguages had "
                                "parsing failure. clearing languages. error "
                                "data=%s",
                            (const char *)contentLanguagesString.getCString()));
                        contentLanguages.clear();
                    }
                }
                else if (System::strcasecmp(line, headerNameTransferTE) == 0)
                {
                    if (gotTransferTE)
                    {
                        _throwEventFailure(HTTP_STATUS_BADREQUEST,
                            "Duplicate TE header detected");
                    }
                    gotTransferTE = true;

                    _transferEncodingTEValues.clear();
                    static const char valueDelimiter = ',';
                    char *valuesStart = valueStart;

                    // now tokenize the values
                    while (*valuesStart)
                    {
                        // strip off whitepsace from the front
                        while (*valuesStart == ' ' || *valuesStart == '\t')
                            valuesStart++;

                        if (valuesStart == valueEnd)
                            break;

                        char *v = strchr(valuesStart, valueDelimiter);
                        if (v)
                        {
                            if (v == valuesStart)
                            {
                                valuesStart++;
                                continue;
                            }
                            v--;
                            // strip off whitespace from the end
                            while (*v == ' ' || *v == '\t')
                                v--;
                            v++;
                            *v = 0;
                        }

                        _transferEncodingTEValues.append(valuesStart);

                        if (v)
                        {
                            *v = valueDelimiter;
                            valuesStart = v+1;
                        }
                        else break;
                    }
                }

                *(valueEnd+1) = valueSave;
            } // if some value tokens

            *colon = ':';
        }

        *sep = save;
        line = sep + ((save == '\r') ? 2 : 1);
        lineNum++;
    }

    if (_contentOffset != -1 && bodylessMessage)
        _contentLength = 0;
}

void HTTPConnection::_clearIncoming()
{
    _contentOffset = -1;
    _contentLength = -1;
    _incomingBuffer.clear();
    _mpostPrefix.clear();
    contentLanguages.clear();
}

void HTTPConnection::_closeConnection()
{
    // return - don't send the close connection message.
    // let the monitor dispatch function do the cleanup.
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPConnection::_closeConnection");
    _connectionClosePending = true;

    // NOTE: if there is a response pending while a close connection request
    // occurs, then this indicates potential activity on this connection apart
    // from this thread of execution (although this code here is locked, other
    // threads may be waiting on this one.)
    // The caller MUST check this value before attempting a delete of this
    // connnection, otherwise the delete may occur while other chunked responses
    // are waiting to be delivered through this connection.
    // This condition may happen on a client error/timeout/interrupt/disconnect

    if (_isClient() == false)
    {
        if (_responsePending == true)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
                "HTTPConnection::_closeConnection - Close connection "
                    "requested while responses are still expected on this "
                    "connection. connection=0x%p, socket=%d\n",
                (void*)this, getSocket()));

        }

        // still set to DYING
        PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
            "Now setting state to %d", MonitorEntry::STATUS_DYING));
        _monitor->setState(_entry_index, MonitorEntry::STATUS_DYING);
        _monitor->tickle();
    }

    if (_connectionRequestCount == 0)
    {
        PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
            "HTTPConnection::_closeConnection - Connection being closed "
                "without receiving any requests.");
    }

    PEG_METHOD_EXIT();
}

Boolean HTTPConnection::isChunkRequested()
{
    Boolean answer = false;
    if (_transferEncodingTEValues.size() > 0 &&
        (Contains(_transferEncodingTEValues, String(headerValueTEchunked)) ||
         Contains(_transferEncodingTEValues, String(headerValueTEtrailers))))
        answer = true;
    return answer;
}

void HTTPConnection::setSocketWriteTimeout(Uint32 socketWriteTimeout)
{
        _socket->setSocketWriteTimeout(socketWriteTimeout);
}


// determine if the current code being executed is on the client side

Boolean HTTPConnection::_isClient()
{
    return (_owningAcceptor == 0);
}

/*
 * determine if the data read in should be treated as transfer encoded data.
 * If so, proceed to strip out the transfer encoded meta data within the body
 * but the headers relating to transfer encoding will remain unchanged.
 * One should refer to the transfer encoding section of the HTTP protocol
 * specification to understand the parsing semantics below.
 * NOTE: this function is coded as a syncronous read! The entire message will
 * be read in before the message leaves this class and is passed up to the
 * client application.
 */

void HTTPConnection::_handleReadEventTransferEncoding()
{
    PEG_METHOD_ENTER(TRC_HTTP,
        "HTTPConnection::_handleReadEventTransferEncoding");

    Uint32 messageLength = _incomingBuffer.size();
    Uint32 headerLength = (Uint32) _contentOffset;

    // return immediately under these conditions:

    // - Header terminator has not been reached yet (_contentOffset < 0)
    // - This is a non-transfer encoded message because the content length
    //   has been set from the given header value (_contentLength > 0)
    //   (_contentLength == 0 means bodyless, so return too - section 4.3)
    // - The message read in so far is <= to the header length
    // - No transfer encoding has been declared in the header.

    if (_contentOffset < 0 || _contentLength >= 0 ||
        messageLength <= headerLength || _transferEncodingValues.size() == 0)
    {
        PEG_METHOD_EXIT();
        return;
    }

    // on the first chunk in the message, set the encoding offset to the content
    // offset

    if (_transferEncodingChunkOffset == 0)
        _transferEncodingChunkOffset = (Uint32) _contentOffset;

    char *headerStart = (char *) _incomingBuffer.getData();
    char *messageStart = headerStart;

    // loop thru the received data (so far) and strip out all chunked meta data.
    // this logic assumes that the data read in may be only partial at any point
    // during the parsing. the variable _transferEncodingChunkOffset represents
    // the byte offset (from the start of the message) of the last NON completed
    // chunk parsed within the message. Remember that the tcp reader has padded
    // the buffer with a terminating null for easy string parsing.

    for (;;)
    {
        // we have parsed the length, but not all bytes of chunk have been read
        // in yet
        if (_transferEncodingChunkOffset >= messageLength)
            break;

        // this is the length from _transferEncodingChunkOffset to the end
        // of the message (so far). It represents the bytes that have not been
        // processed yet

        Uint32 remainderLength = messageLength - _transferEncodingChunkOffset;

        // the start of the first fully non-parsed chunk of this interation
        char *chunkLineStart = messageStart + _transferEncodingChunkOffset;
        char *chunkLineEnd = chunkLineStart;

        // Find the end of the hex string representing the data portion
        // length of the current chunk. Note that we must hit at least one
        // non-hexdigit (except null) to know we have read in the complete
        // number

        while (isxdigit(*chunkLineEnd))
            chunkLineEnd++;

        if (! *chunkLineEnd)
            break;

        // This is the parsed chunk length in hex. From here on, this many bytes
        // plus the chunk terminator (AFTER this chunk line is done) must be
        // read in to constitute a complete chunk in which
        // _transferEncodingChunkOffset can be incremented to the next chunk

        Uint32 chunkLengthParsed =
            (Uint32) strtoul((const char *)chunkLineStart, 0, 16);

        // this also covers strings stated even larger
        if (chunkLengthParsed == PEG_NOT_FOUND)
            _throwEventFailure(HTTP_STATUS_REQUEST_TOO_LARGE,
                "stated chunk length too large");

        char *chunkExtensionStart = chunkLineEnd;
        chunkLineEnd = strstr(chunkLineEnd, chunkLineTerminator);

        // If we have not received the chunk line terminator yet, then
        // return and wait for the next iteration. This is done because the
        // hex length given only represents the non-meta data, not the chunk
        // line itself.

        if (!chunkLineEnd)
            break;

        // the token after the hex digit must be either the chunk line
        // terminator or the chunk extension terminator. If not, the sender
        // has sent an illegal chunked encoding syntax.

        if (strncmp(chunkExtensionStart, chunkExtensionTerminator,
                chunkExtensionTerminatorLength) != 0 &&
            strncmp(chunkExtensionStart, chunkLineTerminator,
                chunkLineTerminatorLength) != 0)
        {
            _throwEventFailure(
                HTTP_STATUS_BADREQUEST, "missing chunk extension");
        }

        chunkLineEnd += chunkLineTerminatorLength;
        Uint32 chunkLineLength = (Uint32)(chunkLineEnd - chunkLineStart);
        Uint32 chunkMetaLength = chunkLineLength;

        // Always add chunkTerminatorLength since last-chunk should also
        // contain chunkTerminator (CRLF)
        chunkMetaLength += chunkTerminatorLength;

        Uint32 chunkTerminatorOffset = _transferEncodingChunkOffset +
            chunkLineLength + chunkLengthParsed;

        // The parsed length represents the non-meta data bytes which starts
        // after the chunk line terminator has been received.
        // If we dont have enough remainder bytes to process from the length
        // parsed then return and wait for the next iteration.

        //
        // Also, if this is the last chunk, then we have to know if there
        // is enough data in here to be able to verify that meta crlf for
        // the end of the whole chunked message is present.
        // If chunkLengthParsed + chunkMetaLenght > reminderLength, it
        // means that there is a space only for meta crlf of the last chunk.
        // Therefore go back and re-read socket until you get enough data
        // for at least 2 crlfs.  One for the end of the last chunk or
        // the end of the optional trailer, and one for the end of whole
        // message.

        if (chunkLengthParsed + chunkMetaLength > remainderLength)
            break;

        // at this point we have a complete chunk. proceed and strip out
        // meta-data
        // NOTE: any time "remove" is called on the buffer, many variables
        // must be recomputed to reflect the data removed.

        // remove the chunk length line
        _incomingBuffer.remove(_transferEncodingChunkOffset, chunkLineLength);
        messageLength = _incomingBuffer.size();
        // always keep the byte after the last data byte null for easy string
        // processing.
        messageStart[messageLength] = 0;

        // recalculate since we just removed the chunk length line
        chunkTerminatorOffset -= chunkLineLength;

        // is this the last chunk ?
        if (chunkLengthParsed == 0)
        {
            // We are at the last chunk. The only remaining data should be:
            // 1. optional trailer first
            // 2. message terminator (will remain on incoming buffer and
            //    passed up)

            remainderLength -= chunkLineLength;

            CIMStatusCode cimStatusCode = CIM_ERR_SUCCESS;
            Uint32 httpStatusCode = HTTP_STATUSCODE_OK;
            String httpStatus;
            String cimErrorValue;

            // is there an optional trailer ?
            if (remainderLength > chunkBodyTerminatorLength)
            {
                Uint32 trailerLength =
                    remainderLength - chunkBodyTerminatorLength;
                Uint32 trailerOffset = _transferEncodingChunkOffset;
                char *trailerStart = messageStart + trailerOffset;
                char *trailerTerminatorStart = trailerStart + trailerLength -
                    trailerTerminatorLength;

                // no trailer terminator before end of chunk body ?
                if (strncmp(trailerTerminatorStart, trailerTerminator,
                        trailerTerminatorLength) != 0)
                    _throwEventFailure(HTTP_STATUS_BADREQUEST,
                        "No chunk trailer terminator received");

                Buffer trailer;
                // add a dummy startLine so that the parser works
                trailer << " " << headerLineTerminator;

                char save = trailerStart[trailerLength];
                trailerStart[trailerLength] = 0;
                trailer << trailerStart;
                trailerStart[trailerLength] = save;

                _incomingBuffer.remove(trailerOffset, trailerLength);
                messageLength = _incomingBuffer.size();
                messageStart[messageLength] = 0;
                remainderLength -= trailerLength;

                // parse the trailer looking for the code and description
                String trailerStartLine;
                Array<HTTPHeader> trailers;
                Uint32 trailerContentLength = 0;
                HTTPMessage httpTrailer(trailer);
                httpTrailer.parse(
                    trailerStartLine, trailers, trailerContentLength);

                // first look for cim error. this is an http level error
                Boolean found = false;

                found = httpTrailer.lookupHeader(
                    trailers, headerNameError, cimErrorValue, true);

                if (found == true)
                {
                    // we have a cim error. parse the header to get the
                    // original http level error if any, otherwise, we have
                    // to make one up.

                    Buffer header(messageStart, headerLength);
                    String headerStartLine;
                    Array<HTTPHeader> headers;
                    Uint32 headerContentLength = 0;
                    HTTPMessage httpHeader(header);
                    httpHeader.parse(
                        headerStartLine, headers, headerContentLength);
                    String httpVersion;
                    Boolean isValid = httpHeader.parseStatusLine(
                        headerStartLine,
                        httpVersion,
                        httpStatusCode,
                        httpStatus);
                    if (isValid == false || httpStatusCode == 0 ||
                            httpStatusCode == HTTP_STATUSCODE_OK)
                    {
                        // ATTN: make up our own http code if not given ?
                        httpStatusCode = (Uint32) HTTP_STATUSCODE_BADREQUEST;
                        httpStatus = HTTP_STATUS_BADREQUEST;
                    }
                }
                else
                {
                    const char* codeValue;
                    found = httpTrailer.lookupHeader(
                        trailers, headerNameCode, codeValue, true);
                    if (found && *codeValue &&
                        (cimStatusCode = (CIMStatusCode)atoi(codeValue)) > 0)
                    {
                        HTTPMessage::lookupHeaderPrefix(
                            trailers, headerNameCode, _mpostPrefix);
                        httpStatus = _mpostPrefix + headerNameCode +
                            headerNameTerminator + codeValue +
                            headerLineTerminator;

                        // look for cim status description
                        String descriptionValue;
                        found = httpTrailer.lookupHeader(
                            trailers,
                            headerNameDescription,
                            descriptionValue,
                            true);
                        if (descriptionValue.size() == 0)
                        {
                            descriptionValue =
                                cimStatusCodeToString(cimStatusCode);
                        }

                        httpStatus = httpStatus + _mpostPrefix +
                            headerNameDescription + headerNameTerminator +
                            descriptionValue + headerLineTerminator;

                    } // if found a cim status code

                    // Get Content-Language out of the trailer, if it is there
                    String contentLanguagesString;
                    found = httpTrailer.lookupHeader(trailers,
                        headerNameContentLanguage,
                        contentLanguagesString,
                        true);

                    contentLanguages.clear();
                    if (found == true && contentLanguagesString.size() > 0)
                    {
                        try
                        {
                            contentLanguages =
                                LanguageParser::parseContentLanguageHeader(
                                    contentLanguagesString);
                        }
                        catch (...)
                        {
                            PEG_TRACE((TRC_HTTP, Tracer::LEVEL2,
                                "HTTPConnection: ERROR: contentLanguages had "
                                    "parsing failure. clearing languages. "
                                    "error data=%s",
                                (const char *)contentLanguagesString.
                                    getCString()));
                            contentLanguages.clear();
                        }
                    }

                } // else not a cim error
            } // if optional trailer present

            char *chunkBodyTerminatorStart =
                messageStart + _transferEncodingChunkOffset;

            // look for chunk body terminator
            if (remainderLength != chunkBodyTerminatorLength ||
                strncmp(chunkBodyTerminatorStart, chunkBodyTerminator,
                    chunkBodyTerminatorLength) != 0)
                _throwEventFailure(HTTP_STATUS_BADREQUEST,
                    "No chunk body terminator received");

            // else the remainder is just the terminator, which we will leave
            // on the incoming buffer and pass up
            // (as if a non-transfer message arrived)

            _transferEncodingChunkOffset = 0;
            _contentLength = messageLength - headerLength;

            if (httpStatusCode != HTTP_STATUSCODE_OK)
            {
                _handleReadEventFailure(httpStatus, cimErrorValue);
            }
            else if (cimStatusCode != CIM_ERR_SUCCESS)
            {
                // discard the XML payload data (body) according to cim
                // operations spec and add code and description to the
                // header so the next layer can interpret the error correctly

                _incomingBuffer.remove(headerLength, _contentLength);
                // remove the header line terminator
                _incomingBuffer.remove(
                    headerLength - headerLineTerminatorLength,
                    headerLineTerminatorLength);
                // append new status
                _incomingBuffer.append(
                    httpStatus.getCString(), httpStatus.size());

                _incomingBuffer.append(
                    headerLineTerminator, headerLineTerminatorLength);
                // null terminate - the buffer is at least as long after
                // removing
                char *data = (char *)_incomingBuffer.getData();
                data[_incomingBuffer.size()] = 0;
                _contentLength = 0;
                _contentOffset = 0;
            }

            break;
        } // if last chunk

        // we are NOT on the last chunk! validate that the offset where the
        // chunk terminator was found matches what the parsed chunk length
        // claimed.

        if (strncmp(messageStart + chunkTerminatorOffset, chunkTerminator,
                chunkTerminatorLength) != 0)
            _throwEventFailure(HTTP_STATUS_BADREQUEST, "Bad chunk terminator");

        // now remove the chunk terminator
        _incomingBuffer.remove(chunkTerminatorOffset, chunkTerminatorLength);
        messageLength = _incomingBuffer.size();
        messageStart[messageLength] = 0;

        // jump to the start of the next chunk (which may not have been
        // read yet)
        _transferEncodingChunkOffset = chunkTerminatorOffset;
    } // for all remaining bytes containing chunks

    PEG_METHOD_EXIT();
}

/*
 * Handle a failure on the read or an HTTP error. This is NOT meant for
 * errors found in the cim response or the trailer.
 * The http status MAY have the detailed message attached to it using the
 * detail delimiter.
 */

void HTTPConnection::_handleReadEventFailure(
    const String& httpStatusWithDetail,
    const String& cimError)
{
    Uint32 delimiterFound = httpStatusWithDetail.find(httpDetailDelimiter);
    String httpDetail;
    String httpStatus = httpStatusWithDetail.subString(0, delimiterFound);

    if (delimiterFound != PEG_NOT_FOUND)
    {
        httpDetail = httpStatusWithDetail.subString(
            delimiterFound + httpDetailDelimiter.size());
    }

    PEG_TRACE((TRC_HTTP, Tracer::LEVEL2,"%s%s%s%s%s",
        (const char*)httpStatus.getCString(),
        (const char*)httpDetailDelimiter.getCString(),
        (const char*)httpDetail.getCString(),
        (const char*)httpDetailDelimiter.getCString(),
        (const char*)cimError.getCString()));

    Buffer message;
    message = XmlWriter::formatHttpErrorRspMessage(httpStatus, cimError,
        httpDetail);
    HTTPMessage* httpMessage = new HTTPMessage(message);

    // this is common error code. If we are the server side, we want to send
    // back the error to the client, but if we are the client side, then we
    // simply want to queue up this error locally so the client app can receive
    // the error. The client side's own message queue name will be the same
    // as the connector name (the server would be acceptor)

    if (_isClient() == true)
    {
        httpMessage->dest = _outputMessageQueue->getQueueId();

        _outputMessageQueue->enqueue(httpMessage);

        _clearIncoming();
    }
    else
    {
        // else server side processing error - send back to client
        PEG_TRACE((TRC_XML_IO, Tracer::LEVEL2,
            "<!-- Error response: queue id: %u -->\n%s",
            getQueueId(),
            httpMessage->message.getData()));
        handleEnqueue(httpMessage);
    }
    _closeConnection();
}

void HTTPConnection::_handleReadEvent()
{
    PEG_METHOD_ENTER(TRC_HTTP, "HTTPConnection::_handleReadEvent");

    if (_acceptPending)
    {
        PEGASUS_ASSERT(!_isClient());

        Sint32 socketAcceptStatus = _socket->accept();

        if (socketAcceptStatus < 0)
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "HTTPConnection: SSL_accept() failed");
            _closeConnection();
            PEG_METHOD_EXIT();
            return;
        }
        else if (socketAcceptStatus == 0)
        {
            // Not enough data yet to complete the SSL handshake
            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                "HTTPConnection: SSL_accept() pending");
            PEG_METHOD_EXIT();
            return;
        }
        else
        {
#ifndef PEGASUS_OS_ZOS
        // Add SSL verification information to the authentication information
            if (_socket->isSecure())
            {
                if (_socket->isPeerVerificationEnabled() &&
                    _socket->isCertificateVerified())
                {
                    _authInfo->setConnectionAuthenticated(true);
                    _authInfo->setAuthType(
                        AuthenticationInfoRep::AUTH_TYPE_SSL);
                    _authInfo->setClientCertificateChain(
                        _socket->getPeerCertificateChain());
                }
            }
#else
            if (_socket->isClientAuthenticated())
            {
                _authInfo->setAuthType(_socket->getAuthType());
                _authInfo->setConnectionAuthenticated(true);
                _authInfo->setConnectionUser(_socket->getAuthenticatedUser());
            }
#endif

            // Go back to the select() and wait for data on the connection
            _acceptPending = false;
            PEG_METHOD_EXIT();
            return;
        }
    }

    // -- Append all data waiting on socket to incoming buffer:

    Sint32 bytesRead = 0;
    Boolean incompleteSecureReadOccurred = false;

    for (;;)
    {
        char buffer[httpTcpBufferSize];

        Sint32 n = _socket->read(buffer, sizeof(buffer)-1);

        if (n <= 0)
        {
            // It is possible that SSL_read was not able to
            // read the entire SSL record.  This could happen
            // if the record was send in multiple packets
            // over the network and only some of the packets
            // are available.  Since SSL requires the entire
            // record to successfully decrypt, the SSL_read
            // operation will return "0 bytes" read.
            // Once all the bytes of the SSL record have been read,
            // SSL_read will return the entire record.
            // The following test was added to allow
            // handleReadEvent to distinguish between a
            // disconnect and partial read of an SSL record.
            //
            incompleteSecureReadOccurred =
                _socket->incompleteSecureReadOccurred(n);
            break;
        }

        try
        {
            _incomingBuffer.reserveCapacity(_incomingBuffer.size() + n);
            _incomingBuffer.append(buffer, n);
        }
        catch (...)
        {
            static const char detailP[] =
                "Unable to append the request to the input buffer";
            String httpStatus =
                HTTP_STATUS_REQUEST_TOO_LARGE + httpDetailDelimiter + detailP;
            _handleReadEventFailure(httpStatus);
            PEG_METHOD_EXIT();
            return;
        }

        bytesRead += n;

        // Check if this was the first read of a connection to the server.
        // This has to happen inside the read loop, because there can be
        // an incomplete SSL read.
        if (_httpMethodNotChecked && (bytesRead > 5) && !_isClient())
        {
            char* buf = _incomingBuffer.getContentPtr();
            // The first bytes of a connection to the server have to contain
            // a valid HTTP Method.
            if ((strncmp(buf, "POST", 4) != 0) &&
                            (strncmp(buf, "PUT", 3) != 0) &&
                            (strncmp(buf, "OPTIONS", 7) != 0) &&
                            (strncmp(buf, "DELETE", 6) != 0) &&
#if defined(PEGASUS_ENABLE_PROTOCOL_WEB)
                            (strncmp(buf, "GET", 3) != 0) &&
                            (strncmp(buf, "HEAD", 4) != 0) &&
#endif
                (strncmp(buf, "M-POST", 6) != 0))
            {
                _clearIncoming();

                PEG_TRACE((TRC_HTTP, Tracer::LEVEL2,
                      "This Request has an unknown HTTP Method: "
                      "%02X %02X %02X %02X %02X %02X",
                      buf[0],buf[1],buf[2],
                      buf[3],buf[4],buf[5]));

                // Try to send message to client.
                // This function also closes the connection.
                _handleReadEventFailure(HTTP_STATUS_NOTIMPLEMENTED);

                PEG_METHOD_EXIT();
                return;
            }
            _httpMethodNotChecked = false;
        }

#if defined (PEGASUS_OS_VMS)
        if (n < sizeof(buffer))
        {
            //
            // Read is smaller than the buffer size.
            // No more to read, continue.
            //
            break;
        }
#endif
    }

    PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
        "Total bytesRead = %d; Bytes read this iteration = %d",
        _incomingBuffer.size(), bytesRead));

    try
    {
        if (_contentOffset == -1)
            _getContentLengthAndContentOffset();
        _handleReadEventTransferEncoding();
    }
    catch (Exception& e)
    {
        _handleReadEventFailure(e.getMessage());
        PEG_METHOD_EXIT();
        return;
    }

    // -- See if the end of the message was reached (some peers signal end of
    // -- the message by closing the connection; others use the content length
    // -- HTTP header and then there are those messages which have no bodies
    // -- at all).

    if ((bytesRead == 0 && !incompleteSecureReadOccurred) ||
        (_contentLength != -1 && _contentOffset != -1 &&
        (Sint32(_incomingBuffer.size()) >= _contentLength + _contentOffset)))
    {
        // If no message was received, just close the connection
        if (_incomingBuffer.size() == 0)
        {
            _clearIncoming();

            PEG_TRACE((TRC_XML_IO, Tracer::LEVEL2,
                "<!-- No request message received; connection closed: "
                    "queue id: %u -->",
                getQueueId()));
            _closeConnection();

            //
            // If we are executing on the server side, the connection
            // is closed, return. Do not forward an empty HTTP message.
            //
            if (!_isClient())
            {
                PEG_METHOD_EXIT();
                return;
            }
        }

        // If the connection was closed and we are executing on the client
        // side send an empty HTTP message. Otherwise, a message was
        // received, so process it.

        HTTPMessage* message = new HTTPMessage(_incomingBuffer, getQueueId());
        message->authInfo = _authInfo.get();
        message->ipAddress = _ipAddress;
        message->contentLanguages = contentLanguages;
        message->dest = _outputMessageQueue->getQueueId();

        //
        // The _closeConnection method sets the _connectionClosePending flag.
        // If we are executing on the client side and the
        // _connectionClosePending flag is set, send an empty HTTP message.
        //
        if (_connectionClosePending)
        {
            _outputMessageQueue->enqueue(message);
            PEG_METHOD_EXIT();
            return;
        }

        if (_isClient() == false)
        {
            PEG_TRACE((
                TRC_XML_IO,
                Tracer::LEVEL4,
                "<!-- Request: queue id: %u -->\n%s",
                getQueueId(),
                Tracer::getHTTPRequestMessage(
                    _incomingBuffer).get()));
        }

        //
        // increment request count
        //
        _connectionRequestCount++;
        _responsePending = true;

        //
        // Set the entry status to BUSY.
        //
        if (_isClient() == false)
        {
            PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                "Now setting state to %d", MonitorEntry::STATUS_BUSY));
            _monitor->setState(_entry_index, MonitorEntry::STATUS_BUSY);
            _monitor->tickle();
        }

        try
        {
            _outputMessageQueue->enqueue(message);
        }
        catch(TooManyHTTPHeadersException& e)
        {
            String httpStatus(HTTP_STATUS_REQUEST_TOO_LARGE);
            httpStatus.append(httpDetailDelimiter);
            httpStatus.append(e.getMessage());
            _handleReadEventFailure(httpStatus);
        }
        catch (Exception& e)
        {
            String httpStatus =
                HTTP_STATUS_BADREQUEST + httpDetailDelimiter + e.getMessage();
            _handleReadEventFailure(httpStatus);
        }
        catch (const exception& e)
        {
            String httpStatus =
                HTTP_STATUS_BADREQUEST + httpDetailDelimiter + e.what();
            _handleReadEventFailure(httpStatus);
        }
        catch (...)
        {
            MessageLoaderParms
                mlParms("Common.HTTPConnection.UNKNOWN_EXCEPTION",
                        "Unknown exception caught while parsing HTTP "
                        "message.");
            String mlString(MessageLoader::getMessage(mlParms));
            String httpStatus =
                HTTP_STATUS_BADREQUEST + httpDetailDelimiter + mlString;
            _handleReadEventFailure(httpStatus);
        }

        _clearIncoming();
    }
    PEG_METHOD_EXIT();
}

Boolean HTTPConnection::isResponsePending()
{
    return _responsePending;
}

Boolean HTTPConnection::run()
{
    Boolean handled_events = false;
    int events = 0;
    fd_set fdread;
    struct timeval tv = { 0, 1 };
    FD_ZERO(&fdread);
    FD_SET(getSocket(), &fdread);
    events = select(FD_SETSIZE, &fdread, NULL, NULL, &tv);

    if (events == PEGASUS_SOCKET_ERROR)
        return false;

    if (events)
    {
        events = 0;
        if (FD_ISSET(getSocket(), &fdread))
        {
            events |= SocketMessage::READ;
            Message *msg = new SocketMessage(getSocket(), events);
            try
            {
                handleEnqueue(msg);
            }
            catch (...)
            {
                PEG_TRACE_CSTRING(
                    TRC_DISCARDED_DATA,
                    Tracer::LEVEL1,
                    "HTTPConnection::run handleEnqueue(msg) failure");
                return true;
            }
            handled_events = true;
        }
    }

    return handled_events;
}

PEGASUS_NAMESPACE_END
