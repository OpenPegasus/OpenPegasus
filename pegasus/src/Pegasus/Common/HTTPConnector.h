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

#ifndef Pegasus_HTTPConnector_h
#define Pegasus_HTTPConnector_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/TLS.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

struct HTTPConnectorRep;

class HTTPConnection;

/** This class is used by clients to establish a connection with a
    server. For each established connection, a HTTPConnection object
    is created.
*/
class PEGASUS_COMMON_LINKAGE HTTPConnector
{
public:

    /** Constructor.
        @param monitor pointer to monitor object which this class uses to
        solicit SocketMessages on the server port (socket).
    */
    HTTPConnector(Monitor* monitor);

    HTTPConnector(Monitor* monitor, SSLContext * sslcontext);

    /** Destructor. */
    ~HTTPConnector();

    /** Establishes a new connection and creates an HTTPConnection object
        to represent it.

        @param host indicates host to connect to
        @param portNumber indicates port number to use
        @param sslContext Specifies the SSL context to use for this connection
        @param timeoutMilliseconds Specifies the connection timeout.
            If a socket is non-blocking and a connection is in progress after
            the initial attempt, the connection will be attempted until the
            specified timeout is reached.  If the specified timeout is zero,
            the connection will not be re-attempted.  The timeout is not
            applicable for a blocking socket.
        @param outputMessageQueue output message queue for the HTTPConnection
        that will be created.
        @exception InvalidLocatorException
        @exception CannotCreateSocketException
        @exception CannotConnectException
    */
    HTTPConnection* connect(
        const String& host,
        const Uint32 portNumber,
        SSLContext * sslContext,
        Uint32 timeoutMilliseconds,
        MessageQueue* outputMessageQueue);

    /** Close the specified connection. */
    void disconnect(HTTPConnection* connection);

private:

    Monitor* _monitor;
    HTTPConnectorRep* _rep;

    SSLContext* _sslcontext;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_HTTPConnector_h */
