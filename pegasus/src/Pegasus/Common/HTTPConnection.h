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

#ifndef Pegasus_HTTPConnection_h
#define Pegasus_HTTPConnection_h

#include <iostream>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/AuthenticationInfo.h>
#include <Pegasus/Common/TLS.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SharedPtr.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_NAMESPACE_BEGIN

class Monitor;
class HTTPAcceptor;

class PEGASUS_COMMON_LINKAGE HTTPConnection : public MessageQueue
{
public:
    typedef MessageQueue Base;

    /** Constructor. */
    HTTPConnection(
        Monitor* monitor,
        SharedPtr<MP_Socket>& socket,
        const String& ipAddress,
        HTTPAcceptor * owningAcceptor,
        MessageQueue * outputMessageQueue);

    /** Destructor. */
    ~HTTPConnection();

    virtual void enqueue(Message *);

    /**
        In this specialization of isActive a check is performed on the
        non-blocking socket to see if it is active by reading 1 byte. Since the
        current thread is processing the request, its safe to try to read 1 byte
        from the socket as there should be no data on the socket. If read
        returns a message of size zero, it is an indication that the client has
        closed the connection and the socket at the server end can be closed.
    */
    virtual Boolean isActive();

    /** This method is called whenever a SocketMessage is enqueued
        on the input queue of the HTTPConnection object.
    */
    virtual void handleEnqueue(Message *);

    virtual void handleEnqueue();

    /** Return socket this connection is using. */
    SocketHandle getSocket() { return _socket->getSocket();}

    MP_Socket& getMPSocket() { return *_socket;}

    /** Indicates whether this connection has an outstanding response.
    */
    Boolean isResponsePending();

    Boolean run();

    HTTPAcceptor& getOwningAcceptor()
    {
        PEGASUS_ASSERT(_owningAcceptor);
        return *_owningAcceptor;
    }

    // was the request for chunking ?
    Boolean isChunkRequested();

    void setSocketWriteTimeout(Uint32 socketWriteTimeout);
    static void setIdleConnectionTimeout(Uint32 idleConnectionTimeout);
    static Uint32 getIdleConnectionTimeout();

    Boolean closeConnectionOnTimeout(struct timeval* timeNow);

    // This method is called in Client code to decide reconnection with
    // the Server and can also be used in the server code to check if the
    // connection is still alive and take appropriate action.
    Boolean needsReconnect();

    // This method is called in Server code when response encoders or
    // HTTPAuthenticatorDelegator runs out-of-memory. This method calls
    // _handleWriteEvent() with a dummy HTTPMessage to maintain  response
    // chunk sequence properly. Once all responses are  arrived, connection
    // is closed. Param "respMsgIndex" indicates the response index and
    // isComplete indicates whether the response is complete or not.
    void handleInternalServerError(
        Uint32 respMsgIndex,
        Boolean isComplete);

    // ATTN-RK-P1-20020521: This is a major hack, required to get the CIM
    // server and tests to run successfully.  The problem is that the
    // HTTPAcceptor is deleting an HTTPConnection before all the threads
    // that are queued up to run in that HTTPConnection instance have had
    // a chance to finish.  This hack makes the HTTPAcceptor spin until
    // the HTTPConnection event threads have completed, before deleting
    // the HTTPConnection.  Note that this fix is not perfect, because
    // there is a window between when the HTTPConnection queue lookup is
    // done and when the refcount is incremented.  If the HTTPAcceptor
    // deletes the HTTPConnection in that window, the soon-to-be-launched
    // HTTPConnection event thread will fail (hard).
    AtomicInt refcount;

    CIMException cimException;

    // list of content languages
    ContentLanguageList contentLanguages;

private:

    void _clearIncoming();

    /**
        @exception  Exception   Indicates an error occurred.
    */
    void _getContentLengthAndContentOffset();

    void _closeConnection();

    void _handleReadEvent();

    Boolean _handleWriteEvent(HTTPMessage& httpMessage);

    void _handleReadEventFailure(const String& httpStatusWithDetail,
                                 const String& cimError = String());
    void _handleReadEventTransferEncoding();
    Boolean _isClient();

    Monitor* _monitor;

    SharedPtr<MP_Socket> _socket;
    String _ipAddress;
    HTTPAcceptor* _owningAcceptor;
    MessageQueue* _outputMessageQueue;

    Sint32 _contentOffset;
    Sint32 _contentLength;
    Buffer _incomingBuffer;
    Buffer _outgoingBuffer;
    SharedPtr<AuthenticationInfo> _authInfo;

    // _connectionRequestCount contains the number of
    // requests that have been received on this connection.
    Uint32 _connectionRequestCount;

    /**
        The _responsePending flag indicates whether the a request has been
        received on the connection and is awaiting a response.  It is set to
        true when a request is received on the connection and set to false
        when a response is sent.  The connection object must not be destructed
        while a response is pending, because the CIM Server must route the
        response to the connection object when it becomes available.
    */
    Boolean _responsePending;

    Mutex _connection_mut;

    // The _connectionClosePending flag will be set to true if
    // the connection receives a close connection socket message.
    // This flag is used to set the connection status to
    // either Monitor::IDLE (_connectionClosePending == false)
    // or Monitor::DYING (_connectionClosePending == true) when
    // returning control of the connection to the Monitor.
    Boolean _connectionClosePending;

    // The _acceptPending flag is set to true if a server-side
    // connection is accepted but an SSL handshake has not been
    // completed.
    Boolean _acceptPending;

    // The _httpMethodNotChecked flag is disabled after the first bytes of a
    // request were read and validated to be one of the supported HTTP methods
    // "POST" or "M-POST".
    Boolean _httpMethodNotChecked;

    // Holds time since the accept pending condition was detected.
    struct timeval _acceptPendingStartTime;

    int _entry_index;

    // When used by the client, it is an offset (from start of http message)
    // representing last NON completely parsed chunk of a transfer encoding.
    // When used by the server, it is the message index that comes down
    // from the providers/repository representing each message chunk.
    // WARNING: Due to the duel use of this member variable, modifying code
    // that uses this variable should be done very carefully. Accidental bugs
    // introduced could easily break interoperability with wbem
    // clients/servers that transfer and/or receive data via HTTP chunking.
    Uint32 _transferEncodingChunkOffset;

    // list of transfer encoding values from sender
    Array<String> _transferEncodingValues;

    // list of TE values from client
    Array<String> _transferEncodingTEValues;

    // 2 digit prefix on http header if mpost was used
    String _mpostPrefix;

    // Holds time since this connection is idle.
    struct timeval _idleStartTime;

    // Idle connection timeout in seconds specified by Config property
    // idleConnectionTimeout.
    static Uint32 _idleConnectionTimeoutSeconds;
#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
    static Mutex _idleConnectionTimeoutSecondsMutex;
#endif
    // When this flag is set to true, it indicates that internal error on this
    // connection occured. Currently this flag is used by the Server code when
    // out-of-memory error is occurs and connection is closed by the server
    // once all responses are arrived.
    Boolean _internalError;

    friend class Monitor;
    friend class HTTPAcceptor;
    friend class HTTPConnector;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_HTTPConnection_h */
