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

#ifndef Pegasus_Monitor_h
#define Pegasus_Monitor_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/ModuleController.h>
#include <Pegasus/Common/Socket.h>
#include <Pegasus/Common/Sharable.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE MonitorEntry
{
public:
    enum Status
    {
        STATUS_IDLE,
        STATUS_BUSY,
        STATUS_DYING,
        STATUS_EMPTY
    };

    enum Type
    {
        TYPE_ACCEPTOR,
        TYPE_CONNECTION,
        TYPE_TICKLER
    };

    MonitorEntry()
    {
        reset();
    }

    MonitorEntry(
        SocketHandle socket_,
        Uint32 queueId_,
        Uint32 status_,
        Uint32 type_)
        : socket(socket_),
          queueId(queueId_),
          status(status_),
          type(type_)
    {
    }

    // NOTE: Using the default implementation of the copy constructor and
    // assignment operator.

    void reset()
    {
        socket = PEGASUS_INVALID_SOCKET;
        queueId = 0;
        status = STATUS_EMPTY;
        type = TYPE_TICKLER;
    }

    SocketHandle socket;
    Uint32 queueId;
    Uint32 status;
    Uint32 type;
};

/** This message occurs when there is activity on a socket. */
class SocketMessage : public Message
{
public:

    enum Events { READ = 1, WRITE = 2, EXCEPTION = 4 };

    SocketMessage(SocketHandle socket_, Uint32 events_)
        : Message(SOCKET_MESSAGE), socket(socket_), events(events_)
    {
    }

    SocketHandle socket;
    Uint32 events;
};

/**
    This message is sent to a connection owner (HTTPAcceptor) so it can do
    any necessary cleanup of the connection.
*/
class CloseConnectionMessage : public Message
{
public:

    CloseConnectionMessage(SocketHandle socket_)
        : Message(CLOSE_CONNECTION_MESSAGE), socket(socket_)
    {
    }

    SocketHandle socket;
};

/**
    The Tickler class provides a loopback socket connection that can be
    included in a select() socket array to allow the select() call to return
    on demand.
*/
class Tickler
{
public:
    /**
        Constructs a Tickler object and initializes its connection.
        @exception Exception if the initialization fails.
    */
    Tickler();

    ~Tickler();

    /**
        Causes a read event on the tickle socket.
    */
    void notify();

    /**
        Consumes all read events on the tickle socket.
    */
    void reset();

    SocketHandle getReadHandle()
    {
        return _serverSocket;
    }

private:
    /**
        Initializes the Tickler connection.
        @exception Exception if the initialization fails.
    */
    void _initialize();

    /**
        Uninitializes the Tickler connection.
    */
    void _uninitialize();

    SocketHandle _listenSocket;
    SocketHandle _clientSocket;
    SocketHandle _serverSocket;
};

/** This class monitors system-level events and notifies its clients of these
    events by posting messages to their queues.

    The monitor generates following message types:

    <ul>
    <li> SocketMessage - occurs when activity on a socket </li>
    </ul>

    Clients solicit these messages by calling one of the following methods:

    <ul>
    <li> solicitSocketMessages() </li>
    </ul>

    The following example shows how to solicit socket messages:

    <pre>
    Monitor monitor;
    Sint32 socket;
    Uint32 queueId;


    ...

    monitor.solicitSocketMessages(
    socket,
    SocketMessage::READ | SocketMessage::WRITE,
    queueId);
    </pre>

    Each time activity occurs on the given socket, a SocketMessage is
    enqueued on the given queue.

    In order the monitor to generate messages, it must be run by calling
    the run() method as shown below:

    <pre>
    Monitor monitor;

    ...

    Uint32 milliseconds = 5000;
    monitor.run(milliseconds);
    </pre>

    In this example, the monitor is run for five seconds. The run method
    returns after the first message is occurs or five seconds has transpired
    (whichever occurs first).
*/
class PEGASUS_COMMON_LINKAGE Monitor
{
public:
    /** Default constructor. */
    Monitor();

    /** This destruct deletes all handlers which were installed. */
    ~Monitor();

    /** Sets the state of the monitor entry to the specified state.
        This is used to synchronize the monitor and the worker
        thread. Bug# 2057 */
    void setState(
        Uint32 index,
        MonitorEntry::Status status);

    void tickle();

    /** Monitor system-level for the given number of milliseconds. Post a
        message to the corresponding queue when such an event occurs.
        Return after the time has elapsed or a single event has occurred,
        whichever occurs first.

        @param timeoutMsec the number of milliseconds to wait for an event.
    */
    void run(Uint32 timeoutMsec);

    /** Solicit interest in SocketMessages. Note that there may only
        be one solicitor per socket.

        @param socket the socket to monitor for activity.
        @param queueId of queue on which to post socket messages.
        @return false if messages have already been solicited on this socket.
    */
    int solicitSocketMessages(
        SocketHandle socket,
        Uint32 queueId,
        Uint32 type);

    /** Unsolicit messages on the given socket.

        @param socket on which to unsolicit messages.
        @return false if no such solicitation has been made on the given socket.
    */
    void unsolicitSocketMessages(SocketHandle);

    /** stop listening for client connections
     */
    void stopListeningForConnections(Boolean wait);

    Mutex& getLock();
private:

    Array<MonitorEntry> _entries;
    /**
        This mutex must be locked when accessing the _entries array or any
        of its MonitorEntry objects.
    */
    Mutex _entriesMutex;

    AtomicInt _stopConnections;
    Semaphore _stopConnectionsSem;

    /** tracks how many times solicitSocketCount() has been called */
    Uint32 _solicitSocketCount;

    Tickler _tickler;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Monitor_h */
