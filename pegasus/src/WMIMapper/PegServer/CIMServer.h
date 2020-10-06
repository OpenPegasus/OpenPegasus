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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Server_h
#define Pegasus_Server_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Server/CIMServerState.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/AutoPtr.h>

#include "HTTPAuthenticatorDelegator.h"

PEGASUS_NAMESPACE_BEGIN

struct ServerRep;

class CIMOperationRequestDispatcher;
class CIMOperationResponseEncoder;
class CIMOperationRequestDecoder;
class CIMOperationRequestAuthorizer;
class CIMExportRequestDispatcher;
class CIMExportResponseEncoder;
class CIMExportRequestDecoder;
class HTTPAcceptor;
class CIMRepository;

/*class ModuleController;
class IndicationHandlerService;
class IndicationService;
class ProviderManagerService;
class ProviderRegistrationManager;
class BinaryMessageHandler;*/
class SSLContextManager;


class PEGASUS_SERVER_LINKAGE CIMServer
{
public:

    enum Protocol { PROPRIETARY, STANDARD };

    /** Constructor - Creates a CIMServer object.
    */
    CIMServer();

    ~CIMServer();

    /** Adds a connection acceptor for the specified listen socket.
        @param connectionType Uint16 specifying whether the acceptor should
               listen on LOCAL_CONNECTION or IPV4_CONNECTION or
               IPV6_CONNECTION socket. portNumer is ignored if
               connectionType is LOCAL_CONNECTION.
        @param portNumber Port number that should be used by the listener.
               This parameter is ignored if localConnection=true.
        @param useSSL Boolean specifying whether SSL should be used for
               connections created by this acceptor.
        @param exportConnection Boolean indicating whether this acceptor is
               only for export connections. If true, client SSL certificate
               verification is enabled on the export connection created by
               this acceptor. Ignored when useSSL is false.
    */
    void addAcceptor(
        Uint16 connectionType,
        Uint32 portNumber,
        Boolean useSSL);

    /** Bind the acceptors to the specified listen sockets.
    @exception - This function may receive exceptions from
    Channel specific subfunctions.
    */
    void bind();

    void tickle_monitor();

    /** runForever Main runloop for the server.
    */
    void runForever();

    /** Call to gracefully shutdown the server.  The server connection socket
        will be closed to disable new connections from clients.
    */
    void stopClientConnection();

    /** Call to gracefully shutdown the server.  It is called when the server
        has been stopped and is ready to be shutdown.  Next time runForever()
        is called, the server shuts down.
    */
    void shutdown();

    /** Return true if the server has shutdown, false otherwise.
    */
    Boolean terminated() { return _dieNow; };

    /** Call to resume the sever.
    */
    void resume();

    /** Call to set the CIMServer state.  Also inform the appropriate
        message queues about the current state of the CIMServer.
    */
    void setState(Uint32 state);

    Uint32 getOutstandingRequestCount();

    /** Signal to shutdown
    */
    static void shutdownSignal();

private:

    Boolean _dieNow;

    AutoPtr<Monitor> _monitor;
    CIMRepository* _repository;
    CIMOperationRequestDispatcher* _cimOperationRequestDispatcher;
    CIMOperationResponseEncoder* _cimOperationResponseEncoder;
    CIMOperationRequestDecoder* _cimOperationRequestDecoder;
    CIMOperationRequestAuthorizer* _cimOperationRequestAuthorizer;

    CIMExportRequestDispatcher* _cimExportRequestDispatcher;
    CIMExportResponseEncoder* _cimExportResponseEncoder;
    CIMExportRequestDecoder* _cimExportRequestDecoder;
    HTTPAuthenticatorDelegator* _httpAuthenticatorDelegator;

    Array<HTTPAcceptor*> _acceptors;
    AutoPtr<CIMServerState> _serverState;

    /*ModuleController* _controlService;
    IndicationHandlerService* _handlerService;
    IndicationService* _indicationService;
    ProviderManagerService* _providerManager;
    ProviderRegistrationManager* _providerRegistrationManager;
    BinaryMessageHandler *_binaryMessageHandler;*/
    SSLContextManager* _sslContextMgr;

    void _init(void);
    SSLContext* _getSSLContext();
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Server_h */
