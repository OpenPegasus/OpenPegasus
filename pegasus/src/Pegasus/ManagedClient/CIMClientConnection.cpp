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

#include "CIMClientConnection.h"

PEGASUS_NAMESPACE_BEGIN

// class constructor
CIMClientConnection::CIMClientConnection()
{
    _connectionHandle.reset(new CIMClientRep());
    _resolvedIP[0] = 0;
}

CIMClientConnection::CIMClientConnection(
    const String& host,
    const String& port,
    const String& userid,
    const String& passwd)
{
    _hostname = String(host);
    _port = String(port);
    _userid = String(userid);
    _passwd = String(passwd);

    _connectionHandle.reset(new CIMClientRep());

        int af;
    System::acquireIP((const char*)host.getCString(), &af, _resolvedIP);
    if (System::isLoopBack(af, _resolvedIP))
    {
        // localhost or ip address of 127.0.0.1
        // still for compare we need the real ip address
        System::acquireIP((const char *)
                    System::getHostName().getCString(), &af, _resolvedIP);
    }
}

CIMClientConnection::CIMClientConnection(
    const String& host,
    const String& port,
    const String& userid,
    const String& passwd,
    const SSLContext& sslcontext)
{
    _hostname = String(host);
    _port = String(port);
    _userid = String(userid);
    _passwd = String(passwd);

    _sslcontext.reset(new SSLContext(sslcontext));

    _connectionHandle.reset(new CIMClientRep());
    int af;
    System::acquireIP((const char*)host.getCString(), &af, _resolvedIP);
    if (System::isLoopBack(af, _resolvedIP))
    {
        // localhost or ip address of 127.0.0.1
        // still for compare we need the real ip address
        System::acquireIP((const char *)
            System::getHostName().getCString(), &af, _resolvedIP);
    }
}

Boolean CIMClientConnection::equals(void *binIPAddress, int af,
    const String& port)
{
    // only if port and resolved ip address are equal,
    // we have the same connection/CIMOM
    if (HostAddress::equal(af, binIPAddress, _resolvedIP)
            && (String::equalNoCase(_port,port)))
    {
        return true;
    }
    return false;
}

CIMClientRep* CIMClientConnection::getConnectionHandle()
{
    return _connectionHandle.get();
}

String CIMClientConnection::getUser()
{
    return _userid;
}

String CIMClientConnection::getPass()
{
    return _passwd;
}

SSLContext* CIMClientConnection::getSSLContext()
{
    return _sslcontext.get();
}


PEGASUS_NAMESPACE_END
