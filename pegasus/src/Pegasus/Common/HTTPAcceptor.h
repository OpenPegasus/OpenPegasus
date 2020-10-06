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

#ifndef Pegasus_HTTPAcceptor_h
#define Pegasus_HTTPAcceptor_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Common/HostAddress.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/TLS.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class HTTPAcceptorRep;
class Monitor;
/** Instances of this class listen on a port and accept conections.
*/
class PEGASUS_COMMON_LINKAGE HTTPAcceptor : public MessageQueue
{
public:
    // Connection types.
    enum { LOCAL_CONNECTION, IPV4_CONNECTION, IPV6_CONNECTION };

    typedef MessageQueue Base;

    /** Constructor.
        @param monitor pointer to monitor object which this class uses to
        solicit SocketMessages on the server port (socket).
        @param outputMessageQueue output message queue for connections
        created by this acceptor.
        @param connectionType indicating the type of connection for
        this acceptor. connectionType can be any one of  LOCAL_CONNECTION,
        IPV4_CONNECTION and IPV6_CONNECTION. If connectionType is
        LOCAL_CONNECTION portNumber is ignored.
        @param portNumber Specifies which port number this acceptor is to
        listen on.  If this value is 0 then a port is automatically selected
        by bind().  In this case, the actual port number used can be retrieved
        using getPortNumber().
        @param sslcontext If null, this acceptor does not create SSL
        connections.  If non-null, the argument specifies an SSL context to
        use for connections established by this acceptor.
        @param listenOn If null, listening for connection is not restricted 
        else it is restricted to listen on specified add in listenOn
    */
    HTTPAcceptor(
        Monitor* monitor,
        MessageQueue* outputMessageQueue,
        Uint16 connectionType,
        Uint32 portNumber,
        SSLContext * sslcontext,
        ReadWriteSem* sslContextObjectLock = 0,
        HostAddress *listenOn = 0);

    /** Destructor. */
    ~HTTPAcceptor();

    /** This method is called whenever a SocketMessage is enqueued
        on the input queue of the HTTPAcceptor object.
    */

    virtual void handleEnqueue(Message *);

    virtual void handleEnqueue();

    /** Bind the specified listen socket.
        @exception throws BindFailedException if unable to bind (either
        because the listen socket is invalid or the socket is in use).
    */
    void bind();

    /** Unbind from the given port.
    */
    void unbind();

    /** Close the connection socket.
    */
    void closeConnectionSocket();

    /** Reopen the connection socket.
    */
    void reopenConnectionSocket();

    /** Destroys all the connections created by this acceptor. */
    void destroyConnections();

    /** Close and Reopen the connection socket.
    */
    void reconnectConnectionSocket();

    /** Returns the number of outstanding requests for connections created by
        this Acceptor.
    */
    Uint32 getOutstandingRequestCount() const;

    /** Returns the port number used for the connection.  If the number
        is 0 that means that the socket is not bound.
    */
    Uint32 getPortNumber() const;

    static void setSocketWriteTimeout(Uint32 socketWriteTimeout);

private:

    void _acceptConnection();
    void _bind();


    Monitor* _monitor;
    MessageQueue* _outputMessageQueue;
    HTTPAcceptorRep* _rep;

    int _entry_index;

    Uint16 _connectionType;
    Uint32 _portNumber;
    SSLContext* _sslcontext;
    ReadWriteSem*  _sslContextObjectLock;
    static Uint32 _socketWriteTimeout;
#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
    static Mutex _socketWriteTimeoutMutex;
#endif
    HostAddress *_listenAddress;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_HTTPAcceptor_h */
