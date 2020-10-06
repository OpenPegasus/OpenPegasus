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
#ifndef Pegasus_CIMClientConnection_h
#define Pegasus_CIMClientConnection_h

#include <Pegasus/Client/CIMClientRep.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Client/Linkage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/HostAddress.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_CLIENT_LINKAGE CIMClientConnection
{

public:

    // class constructor
    CIMClientConnection();

    CIMClientConnection(
        const String& host,
        const String& port,
        const String& userid,
        const String& passwd);

    CIMClientConnection(
        const String& host,
        const String& port,
        const String& userid,
        const String& passwd,
        const SSLContext& sslcontext);

    Boolean equals(void *binIPAddress, int af, const String& port);

    CIMClientRep *  getConnectionHandle(void);

    String getUser(void);
    String getPass(void);
    SSLContext* getSSLContext(void);

private:
    AutoPtr<CIMClientRep> _connectionHandle;
    String  _hostname;
    String  _port;
    String  _userid;
    String  _passwd;
    AutoPtr<SSLContext> _sslcontext;

    char  _resolvedIP[PEGASUS_INET6_ADDRSTR_LEN];
};

PEGASUS_NAMESPACE_END

#endif  // Pegasus_CIMClientConnection_h
