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

#include "CIMDefaultClientConnectionManager.h"

PEGASUS_NAMESPACE_BEGIN

// class constructor
CIMDefaultClientConnectionManager::CIMDefaultClientConnectionManager()
{

}

// virtual class destructor has to be implemented by specific implementation
CIMDefaultClientConnectionManager::~CIMDefaultClientConnectionManager()
{
    CIMClientConnection     *remvPointer;
    for (Uint32 i=0;i<_cccm_container.size();i++)
    {
        remvPointer = (CIMClientConnection*) _cccm_container[i];
        delete remvPointer;
    }
    _cccm_container.clear();
}

// this function returns the specified connection
CIMClientRep* CIMDefaultClientConnectionManager::getConnection(
                                const String& host,
                                const String& port,
                                const CIMNamespaceName& nameSpace)

{
    char requestedIP[PEGASUS_INET6_ADDRSTR_LEN];
    Uint32 requestedPort;

        int af;
        System::acquireIP((const char*)host.getCString(), &af, requestedIP);
        if (System::isLoopBack(af, requestedIP))
    {
        // localhost or ip address of 127.0.0.1
        // still for compare we need the real ip address
            System::acquireIP((const char *)
                System::getHostName().getCString(), &af, requestedIP);
    }

    requestedPort = strtoul((const char*) port.getCString(), NULL, 0);

    CIMClientConnection     *ccc;
    CIMClientRep            *returnedConnectionHandle;

    // rotate through list to find correct connection
    for (Uint32 i=0;i<_cccm_container.size();i++)
    {
        ccc = _cccm_container[i];
        if (ccc->equals(requestedIP, af, port))
        {
            // okay, we found the correct connection
            // in case it isn't already connected,
            // lets connect and return the connection to caller
            returnedConnectionHandle = ccc->getConnectionHandle();
            if (returnedConnectionHandle != 0)
            {
                // connecting in case we aren't already connected
                if (!returnedConnectionHandle->isConnected())
                {
                    // is this a ssl connection ?
                    if (ccc->getSSLContext() == 0)
                    {
                        // no, we connect without ssl
                        returnedConnectionHandle->connect(
                            host,
                            requestedPort,
                            ccc->getUser(),
                            ccc->getPass());
                    } else
                    {
                        // yes, ssl is enabled
                        returnedConnectionHandle->connect(
                            host,
                            requestedPort,
                            *ccc->getSSLContext(),
                            ccc->getUser(),
                            ccc->getPass());
                    }
                }
            } else {
                // in case no fitting connection could be found
                // a return of NULL shall be returned
                return 0;
            }
            // at this stage we found an applicable connection
            // and also connected already
            // thus we return the connection handle CIMClientRep
            return returnedConnectionHandle;
        }
    }
    return 0;
}

void CIMDefaultClientConnectionManager::addConnection(
                                            const String& host,
                                            const String& port,
                                            const String& userid,
                                            const String& passwd)
{
    CIMClientConnection     *newConnPointer;
    newConnPointer = new CIMClientConnection(host, port, userid, passwd);
    _cccm_container.append(newConnPointer);
}

void CIMDefaultClientConnectionManager::addConnection(
                                            const String& host,
                                            const String& port,
                                            const String& userid,
                                            const String& passwd,
                                            const SSLContext& sslcontext)
{
    CIMClientConnection     *newConnPointer;
    newConnPointer = new CIMClientConnection(
                             host,
                             port,
                             userid,
                             passwd,
                             sslcontext);

    _cccm_container.append(newConnPointer);
}


PEGASUS_NAMESPACE_END
