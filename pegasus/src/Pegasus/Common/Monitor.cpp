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

#include "Network.h"
#include <Pegasus/Common/Config.h>
#include <cstring>
#include "Monitor.h"
#include "MessageQueue.h"
#include "Socket.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/HTTPAcceptor.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/Exception.h>
#include "ArrayIterator.h"
#include "HostAddress.h"
#include <errno.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// Tickler
//
////////////////////////////////////////////////////////////////////////////////

Tickler::Tickler()
    : _listenSocket(PEGASUS_INVALID_SOCKET),
      _clientSocket(PEGASUS_INVALID_SOCKET),
      _serverSocket(PEGASUS_INVALID_SOCKET)
{
    try
    {
        _initialize();
    }
    catch (...)
    {
        _uninitialize();
        throw;
    }
}

Tickler::~Tickler()
{
    _uninitialize();
}

void Tickler::notify()
{
    Socket::write(_clientSocket, "\0", 1);
}

void Tickler::reset()
{
    // Clear all bytes from the tickle socket
    char buffer[32];
    while (Socket::read(_serverSocket, buffer, 32) > 0)
    {
    }
}

#if defined(PEGASUS_OS_TYPE_UNIX)

// Use an anonymous pipe for the tickle connection.

void Tickler::_initialize()
{
    int fds[2];

    if (pipe(fds) == -1)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_CREATE",
            "Received error number $0 while creating the internal socket.",
            getSocketError());
        throw Exception(parms);
    }

    _serverSocket = fds[0];
    _clientSocket = fds[1];

    Socket::disableBlocking(_serverSocket);
}

#else

// Use an external loopback socket connection to allow the tickle socket to
// be included in the select() array on non-Unix platforms.

void Tickler::_initialize()
{
    //
    // Set up the addresses for the listen, client, and server sockets
    // based on whether IPv6 is enabled.
    //

    Socket::initializeInterface();

# ifdef PEGASUS_ENABLE_IPV6
    struct sockaddr_storage listenAddress;
    struct sockaddr_storage clientAddress;
    struct sockaddr_storage serverAddress;
# else
    struct sockaddr_in listenAddress;
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;
# endif

    int addressFamily;
    SocketLength addressLength;

    memset(&listenAddress, 0, sizeof (listenAddress));

# ifdef PEGASUS_ENABLE_IPV6
    if (System::isIPv6StackActive())
    {
        // Use the IPv6 loopback address for the listen sockets
        HostAddress::convertTextToBinary(
            HostAddress::AT_IPV6,
            "::1",
            &reinterpret_cast<struct sockaddr_in6*>(&listenAddress)->sin6_addr);
        listenAddress.ss_family = AF_INET6;
        reinterpret_cast<struct sockaddr_in6*>(&listenAddress)->sin6_port = 0;

        addressFamily = AF_INET6;
        addressLength = sizeof(struct sockaddr_in6);
    }
    else
# endif
    {
        // Use the IPv4 loopback address for the listen sockets
        HostAddress::convertTextToBinary(
            HostAddress::AT_IPV4,
            "127.0.0.1",
            &reinterpret_cast<struct sockaddr_in*>(
                &listenAddress)->sin_addr.s_addr);
        reinterpret_cast<struct sockaddr_in*>(&listenAddress)->sin_family =
            AF_INET;
        reinterpret_cast<struct sockaddr_in*>(&listenAddress)->sin_port = 0;

        addressFamily = AF_INET;
        addressLength = sizeof(struct sockaddr_in);
    }

    // Use the same address for the client socket as the listen socket
    clientAddress = listenAddress;

    //
    // Set up a listen socket to allow the tickle client and server to connect
    //

    // Create the listen socket
    if ((_listenSocket = Socket::createSocket(addressFamily, SOCK_STREAM, 0)) ==
             PEGASUS_INVALID_SOCKET)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_CREATE",
            "Received error number $0 while creating the internal socket.",
            getSocketError());
        throw Exception(parms);
    }

    // Bind the listen socket to the loopback address
    if (::bind(
            _listenSocket,
            reinterpret_cast<struct sockaddr*>(&listenAddress),
            addressLength) < 0)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_BIND",
            "Received error number $0 while binding the internal socket.",
            getSocketError());
        throw Exception(parms);
    }

    // Listen for a connection from the tickle client
    if ((::listen(_listenSocket, 3)) < 0)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_LISTEN",
            "Received error number $0 while listening to the internal socket.",
            getSocketError());
        throw Exception(parms);
    }

    // Verify we have the correct listen socket
    SocketLength tmpAddressLength = addressLength;
    int sock = ::getsockname(
        _listenSocket,
        reinterpret_cast<struct sockaddr*>(&listenAddress),
        &tmpAddressLength);
    if (sock < 0)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_SOCKNAME",
            "Received error number $0 while getting the internal socket name.",
            getSocketError());
        throw Exception(parms);
    }

    //
    // Set up the client side of the tickle connection.
    //

    // Create the client socket
    if ((_clientSocket = Socket::createSocket(addressFamily, SOCK_STREAM, 0)) ==
             PEGASUS_INVALID_SOCKET)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_CLIENT_CREATE",
            "Received error number $0 while creating the internal client "
                "socket.",
            getSocketError());
        throw Exception(parms);
    }

    // Bind the client socket to the loopback address
    if (::bind(
            _clientSocket,
            reinterpret_cast<struct sockaddr*>(&clientAddress),
            addressLength) < 0)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_CLIENT_BIND",
            "Received error number $0 while binding the internal client "
                "socket.",
            getSocketError());
        throw Exception(parms);
    }

    // Connect the client socket to the listen socket address
    if (::connect(
            _clientSocket,
            reinterpret_cast<struct sockaddr*>(&listenAddress),
            addressLength) < 0)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_CLIENT_CONNECT",
            "Received error number $0 while connecting the internal client "
                "socket.",
            getSocketError());
        throw Exception(parms);
    }

    //
    // Set up the server side of the tickle connection.
    //

    tmpAddressLength = addressLength;

    // Accept the client socket connection.
    _serverSocket = ::accept(
        _listenSocket,
        reinterpret_cast<struct sockaddr*>(&serverAddress),
        &tmpAddressLength);

    if (_serverSocket == PEGASUS_SOCKET_ERROR)
    {
        MessageLoaderParms parms(
            "Common.Monitor.TICKLE_ACCEPT",
            "Received error number $0 while accepting the internal socket "
                "connection.",
            getSocketError());
        throw Exception(parms);
    }

    //
    // Close the listen socket and make the other sockets non-blocking
    //

    Socket::close(_listenSocket);
    Socket::disableBlocking(_serverSocket);
    Socket::disableBlocking(_clientSocket);
}

#endif

void Tickler::_uninitialize()
{
    PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4, "uninitializing interface");

    try
    {
        Socket::close(_serverSocket);
        Socket::close(_clientSocket);
        Socket::close(_listenSocket);
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL2,
            "Failed to close tickle sockets");
    }
    Socket::uninitializeInterface();
}


////////////////////////////////////////////////////////////////////////////////
//
// Monitor
//
////////////////////////////////////////////////////////////////////////////////

#define MAX_NUMBER_OF_MONITOR_ENTRIES  32
Monitor::Monitor()
   : _stopConnections(0),
     _stopConnectionsSem(0),
     _solicitSocketCount(0)
{
    int numberOfMonitorEntriesToAllocate = MAX_NUMBER_OF_MONITOR_ENTRIES;
    _entries.reserveCapacity(numberOfMonitorEntriesToAllocate);

    // Create a MonitorEntry for the Tickler and set its state to IDLE so the
    // Monitor will watch for its events.
    _entries.append(MonitorEntry(
        _tickler.getReadHandle(),
        1,
        MonitorEntry::STATUS_IDLE,
        MonitorEntry::TYPE_TICKLER));

    // Start the count at 1 because _entries[0] is the Tickler
    for (int i = 1; i < numberOfMonitorEntriesToAllocate; i++)
    {
        _entries.append(MonitorEntry());
    }
}

Monitor::~Monitor()
{
    PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                  "returning from monitor destructor");
}

void Monitor::tickle()
{
    _tickler.notify();
}

void Monitor::setState(
    Uint32 index,
    MonitorEntry::Status status)
{
    AutoMutex autoEntryMutex(_entriesMutex);
    // Set the state to requested state
    _entries[index].status = status;
}

void Monitor::run(Uint32 milliseconds)
{
    struct timeval tv = {milliseconds/1000, milliseconds%1000*1000};

    fd_set fdread;
    FD_ZERO(&fdread);

    AutoMutex autoEntryMutex(_entriesMutex);

    ArrayIterator<MonitorEntry> entries(_entries);

    // Check the stopConnections flag.  If set, clear the Acceptor monitor
    // entries
    if (_stopConnections.get() == 1)
    {
        for (Uint32 indx = 0; indx < entries.size(); indx++)
        {
            if (entries[indx].type == MonitorEntry::TYPE_ACCEPTOR)
            {
                if (entries[indx].status != MonitorEntry::STATUS_EMPTY)
                {
                    if (entries[indx].status == MonitorEntry::STATUS_IDLE ||
                        entries[indx].status == MonitorEntry::STATUS_DYING)
                    {
                        // remove the entry
                        entries[indx].status = MonitorEntry::STATUS_EMPTY;
                    }
                    else
                    {
                        // set status to DYING
                        entries[indx].status = MonitorEntry::STATUS_DYING;
                    }
                }
            }
        }
        _stopConnections = 0;
        _stopConnectionsSem.signal();
    }

    for (Uint32 indx = 0; indx < entries.size(); indx++)
    {
        const MonitorEntry& entry = entries[indx];

        if ((entry.status == MonitorEntry::STATUS_DYING) &&
            (entry.type == MonitorEntry::TYPE_CONNECTION))
        {
            MessageQueue *q = MessageQueue::lookup(entry.queueId);
            PEGASUS_ASSERT(q != 0);
            HTTPConnection &h = *static_cast<HTTPConnection *>(q);

            if (h._connectionClosePending == false)
                continue;

            // NOTE: do not attempt to delete while there are pending responses
            // coming thru. The last response to come thru after a
            // _connectionClosePending will reset _responsePending to false
            // and then cause the monitor to rerun this code and clean up.
            // (see HTTPConnection.cpp)

            if (h._responsePending == true)
            {
                PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                    "Monitor::run - Ignoring connection delete request "
                        "because responses are still pending. "
                        "connection=0x%p, socket=%d\n",
                    (void *)&h, h.getSocket()));
                continue;
            }
            h._connectionClosePending = false;
            HTTPAcceptor &o = h.getOwningAcceptor();
            Message* message= new CloseConnectionMessage(entry.socket);
            message->dest = o.getQueueId();

            // HTTPAcceptor is responsible for closing the connection.
            // The lock is released to allow HTTPAcceptor to call
            // unsolicitSocketMessages to free the entry.
            // Once HTTPAcceptor completes processing of the close
            // connection, the lock is re-requested and processing of
            // the for loop continues.  This is safe with the current
            // implementation of the entries object.  Note that the
            // loop condition accesses the entries.size() on each
            // iteration, so that a change in size while the mutex is
            // unlocked will not result in an ArrayIndexOutOfBounds
            // exception.

            _entriesMutex.unlock();
            o.enqueue(message);
            _entriesMutex.lock();

            // After enqueue a message and the autoEntryMutex has been
            // released and locked again, the array of _entries can be
            // changed. The ArrayIterator has be reset with the original
            // _entries.
            entries.reset(_entries);
        }
    }

    Uint32 _idleEntries = 0;

    /*
        We will keep track of the maximum socket number and pass this value
        to the kernel as a parameter to SELECT.  This loop seems like a good
        place to calculate the max file descriptor (maximum socket number)
        because we have to traverse the entire array.
    */
    SocketHandle maxSocketCurrentPass = 0;
    for (Uint32 indx = 0; indx < entries.size(); indx++)
    {
        if (maxSocketCurrentPass < entries[indx].socket)
            maxSocketCurrentPass = entries[indx].socket;

        if (entries[indx].status == MonitorEntry::STATUS_IDLE)
        {
            _idleEntries++;
            FD_SET(entries[indx].socket, &fdread);
        }
    }

    /*
        Add 1 then assign maxSocket accordingly. We add 1 to account for
        descriptors starting at 0.
    */
    maxSocketCurrentPass++;

    _entriesMutex.unlock();

    //
    // The first argument to select() is ignored on Windows and it is not
    // a socket value.  The original code assumed that the number of sockets
    // and a socket value have the same type.  On Windows they do not.
    //
#ifdef PEGASUS_OS_TYPE_WINDOWS
    int events = select(0, &fdread, NULL, NULL, &tv);
#else
    int events = select(maxSocketCurrentPass, &fdread, NULL, NULL, &tv);
#endif

    _entriesMutex.lock();

    struct timeval timeNow;
    Time::gettimeofday(&timeNow);

    // After enqueue a message and the autoEntryMutex has been released and
    // locked again, the array of _entries can be changed. The ArrayIterator
    // has be reset with the original _entries
    entries.reset(_entries);

    if (events == PEGASUS_SOCKET_ERROR)
    {
        int selectErrno = 0;
        selectErrno = getSocketError();

        PEG_TRACE((TRC_HTTP, Tracer::LEVEL1,
            "Monitor::run - select() returned error %d.", selectErrno));
        // The EBADF error indicates that one or more or the file
        // descriptions was not valid. This could indicate that
        // the entries structure has been corrupted or that
        // we have a synchronization error.

        PEGASUS_ASSERT(selectErrno != EBADF);
    }
    else if (events)
    {
        PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
            "Monitor::run select event received events = %d, monitoring %d "
                "idle entries",
            events, _idleEntries));
        for (Uint32 indx = 0; indx < entries.size(); indx++)
        {
            // The Monitor should only look at entries in the table that are
            // IDLE (i.e., owned by the Monitor).
            if ((entries[indx].status == MonitorEntry::STATUS_IDLE) &&
                (FD_ISSET(entries[indx].socket, &fdread)))
            {
                MessageQueue* q = MessageQueue::lookup(entries[indx].queueId);
                PEGASUS_ASSERT(q != 0);
                PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                    "Monitor::run indx = %d, queueId = %d, q = %p",
                    indx, entries[indx].queueId, q));

                try
                {
                    if (entries[indx].type == MonitorEntry::TYPE_CONNECTION)
                    {
                        PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                            "entries[%d].type is TYPE_CONNECTION",
                            indx));

                        HTTPConnection *dst =
                            reinterpret_cast<HTTPConnection *>(q);
                        dst->_entry_index = indx;

                        // Update idle start time because we have received some
                        // data. Any data is good data at this point, and we'll
                        // keep the connection alive, even if we've exceeded
                        // the idleConnectionTimeout, which will be checked
                        // when we call closeConnectionOnTimeout() next.
                        Time::gettimeofday(&dst->_idleStartTime);

                        // Check for accept pending (ie. SSL handshake pending)
                        // or idle connection timeouts for sockets from which
                        // we received data (avoiding extra queue lookup below).
                        if (!dst->closeConnectionOnTimeout(&timeNow))
                        {
                            PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                                "Entering HTTPConnection::run() for "
                                    "indx = %d, queueId = %d, q = %p",
                                indx, entries[indx].queueId, q));

                            try
                            {
                                dst->run();
                            }
                            catch (...)
                            {
                                PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL1,
                                    "Caught exception from "
                                    "HTTPConnection::run()");
                            }
                            PEG_TRACE_CSTRING(TRC_HTTP, Tracer::LEVEL4,
                                "Exited HTTPConnection::run()");
                        }
                    }
                    else if (entries[indx].type == MonitorEntry::TYPE_TICKLER)
                    {
                        _tickler.reset();
                    }
                    else
                    {
                        PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
                            "Non-connection entry, indx = %d, has been "
                                "received.",
                            indx));
                        Message* msg = new SocketMessage(
                            entries[indx].socket, SocketMessage::READ);
                        entries[indx].status = MonitorEntry::STATUS_BUSY;
                        _entriesMutex.unlock();
                        q->enqueue(msg);
                        _entriesMutex.lock();

                        // After enqueue a message and the autoEntryMutex has
                        // been released and locked again, the array of
                        // entries can be changed. The ArrayIterator has to be
                        // reset with the latest _entries.
                        entries.reset(_entries);
                        entries[indx].status = MonitorEntry::STATUS_IDLE;
                    }
                }
                catch (...)
                {
                }
            }
            // else check for accept pending (ie. SSL handshake pending) or
            // idle connection timeouts for sockets from which we did not
            // receive data.
            else if ((entries[indx].status == MonitorEntry::STATUS_IDLE) &&
                entries[indx].type == MonitorEntry::TYPE_CONNECTION)

            {
                MessageQueue* q = MessageQueue::lookup(entries[indx].queueId);
                PEGASUS_ASSERT(q != 0);
                HTTPConnection *dst = reinterpret_cast<HTTPConnection *>(q);
                dst->_entry_index = indx;
                dst->closeConnectionOnTimeout(&timeNow);
            }
        }
    }
    // else if "events" is zero (ie. select timed out) then we still need
    // to check if there are any pending SSL handshakes that have timed out.
    else
    {
        for (Uint32 indx = 0; indx < entries.size(); indx++)
        {
            if ((entries[indx].status == MonitorEntry::STATUS_IDLE) &&
                entries[indx].type == MonitorEntry::TYPE_CONNECTION)
            {
                MessageQueue* q = MessageQueue::lookup(entries[indx].queueId);
                PEGASUS_ASSERT(q != 0);
                HTTPConnection *dst = reinterpret_cast<HTTPConnection *>(q);
                dst->_entry_index = indx;
                dst->closeConnectionOnTimeout(&timeNow);
            }
        }
    }
}

void Monitor::stopListeningForConnections(Boolean wait)
{
    PEG_METHOD_ENTER(TRC_HTTP, "Monitor::stopListeningForConnections()");
    // set boolean then tickle the server to recognize _stopConnections
    _stopConnections = 1;
    tickle();

    if (wait)
    {
      // Wait for the monitor to notice _stopConnections.  Otherwise the
      // caller of this function may unbind the ports while the monitor
      // is still accepting connections on them.
      _stopConnectionsSem.wait();
    }

    PEG_METHOD_EXIT();
}


int Monitor::solicitSocketMessages(
    SocketHandle socket,
    Uint32 queueId,
    Uint32 type)
{
    PEG_METHOD_ENTER(TRC_HTTP, "Monitor::solicitSocketMessages");
    AutoMutex autoMut(_entriesMutex);

    // Check to see if we need to dynamically grow the _entries array
    // We always want the _entries array to be 2 bigger than the
    // current connections requested
    _solicitSocketCount++;  // bump the count

    for (Uint32 i = _entries.size(); i < _solicitSocketCount + 1; i++)
    {
        _entries.append(MonitorEntry());
    }

    for (Uint32 index = 1; index < _entries.size(); index++)
    {
        try
        {
            if (_entries[index].status == MonitorEntry::STATUS_EMPTY)
            {
                _entries[index].socket = socket;
                _entries[index].queueId  = queueId;
                _entries[index].type = type;
                _entries[index].status = MonitorEntry::STATUS_IDLE;

                PEG_METHOD_EXIT();
                return (int)index;
            }
        }
        catch (...)
        {
        }
    }
    // decrease the count, if we are here we didn't do anything meaningful
    _solicitSocketCount--;
    PEG_METHOD_EXIT();
    return -1;
}

void Monitor::unsolicitSocketMessages(SocketHandle socket)
{
    PEG_METHOD_ENTER(TRC_HTTP, "Monitor::unsolicitSocketMessages");
    AutoMutex autoMut(_entriesMutex);

    /*
        Start at index = 1 because _entries[0] is the tickle entry which
        never needs to be reset to EMPTY;
    */
    for (Uint32 index = 1; index < _entries.size(); index++)
    {
        if (_entries[index].socket == socket)
        {
            _entries[index].reset();
            _solicitSocketCount--;
            break;
        }
    }

    /*
        Dynamic Contraction:
        To remove excess entries we will start from the end of the _entries
        array and remove all entries with EMPTY status until we find the
        first NON EMPTY.  This prevents the positions, of the NON EMPTY
        entries, from being changed.
    */
    for (Uint32 index = _entries.size() - 1;
         (_entries[index].status == MonitorEntry::STATUS_EMPTY) &&
             (index >= MAX_NUMBER_OF_MONITOR_ENTRIES);
         index--)
    {
        _entries.remove(index);
    }

    PEG_METHOD_EXIT();
}

Mutex& Monitor::getLock()
{
    return _entriesMutex;
}

PEGASUS_NAMESPACE_END
