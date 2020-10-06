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

#ifndef Pegasus_Server_h
#define Pegasus_Server_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/ProviderManager2/Default/ProviderMessageHandler.h>
#include <Pegasus/Server/CIMServerState.h>
#include <Pegasus/Server/HTTPAuthenticatorDelegator.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Common/SCMOClassCache.h>

PEGASUS_NAMESPACE_BEGIN

class CIMOperationRequestDispatcher;
class CIMOperationResponseEncoder;
class CIMOperationRequestDecoder;
class CIMOperationRequestAuthorizer;
class CIMExportRequestDispatcher;
class CIMExportResponseEncoder;
class CIMExportRequestDecoder;
class WsmProcessor;
class RsProcessor;
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
class WebServer;
#endif /* PEGASUS_ENABLE_PROTOCOL_WEB */
class HTTPAcceptor;
class CIMRepository;

class ModuleController;
class IndicationHandlerService;
class IndicationService;
class ProviderManagerService;
class ProviderRegistrationManager;
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
        @param connectionType specifying whether the acceptor should
               listen on LOCAL_CONNECTION or IPV4_CONNECTION or
               IPV6_CONNECTION socket. portNumber is ignored if
               connectionType is LOCAL_CONNECTION.
        @param portNumber Port number that should be used by the listener.
               This parameter is ignored if localConnection=true.
        @param useSSL Boolean specifying whether SSL should be used for
               connections created by this acceptor.
    */
    void addAcceptor(
        Uint16 connectionType,
        Uint32 portNumber,
        Boolean useSSL,
        HostAddress *ipAddress = 0);

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

    /** Returns true if the server is in the process of shutting down,
        false otherwise.
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

    /** Performs any initializations required before accepting the requests.
        Currently this method is used for IndicationService to send the
        CIMSubscriptionInitCompleteRequestMessage.
    */
    void initComplete();

    /** Signal to shutdown
    */
    static void shutdownSignal();

    /** startSLPProvider is a temporary function to get this
        provider started as part of system startup.
        This MUST be replaced with something more permanent.
    */
    void startSLPProvider();

    /**
        This function gets the current environment variables, the current
        configuration properties, and the currently registered provider
        modules whent the CIM Server starts with the configuration
        property "enableAuditLog" set to true.
    */
    static void auditLogInitializeCallback();

    /**
        get the singleton instance of the CIMServer object
        @return  pointer to the singleton instance of the CIMServer
                 object. This should NEVER be NULL since that would
                 indicate that the CIMServer object was not
                 initialized so server not alive.
    */
    static CIMServer* getInstance();

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

    RsProcessor* _rsProcessor;
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    WebServer* _webServer;
#endif
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
    WsmProcessor* _wsmProcessor;
#endif

    Array<HTTPAcceptor*> _acceptors;
    Array<ProviderMessageHandler*> _controlProviders;
    AutoPtr<CIMServerState> _serverState;

    ModuleController* _controlService;
    IndicationHandlerService* _handlerService;
    IndicationService* _indicationService;
    ProviderManagerService* _providerManager;
    ProviderRegistrationManager* _providerRegistrationManager;
    SSLContextManager* _sslContextMgr;

    static SCMOClass _scmoClassCache_GetClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    void _init();
    SSLContext* _getSSLContext();

    //Give access to _providerManager
    friend class EnumerationContextTable;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Server_h */
