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

#include <Pegasus/Common/Config.h>

#include <cstdio>
#include <cctype>
#include <ctime>

#if (defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_LINUX)) \
    && defined(PEGASUS_USE_RELEASE_DIRS)
# include <unistd.h>
# include <errno.h>
#endif

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/HTTPAcceptor.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Cimom.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/AuditLogger.h>
#include <Pegasus/Common/StringConversion.h>

#include <Pegasus/General/SSLContextManager.h>

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/ExportServer/CIMExportRequestDispatcher.h>
#include <Pegasus/ExportServer/CIMExportResponseEncoder.h>
#include <Pegasus/ExportServer/CIMExportRequestDecoder.h>
#include <Pegasus/Config/ConfigManager.h>
#ifndef PEGASUS_PAM_AUTHENTICATION
# include <Pegasus/Security/UserManager/UserManager.h>
# include <Pegasus/ControlProviders/UserAuthProvider/UserAuthProvider.h>
#endif
#include <Pegasus/HandlerService/IndicationHandlerService.h>
#include <Pegasus/IndicationService/IndicationService.h>
#include <Pegasus/ProviderManagerService/ProviderManagerService.h>
#include <Pegasus/ProviderManager2/Default/DefaultProviderManager.h>
#include "reg_table.h"  // Location of DynamicRoutingTable

#if defined PEGASUS_OS_ZOS
# include "ConsoleManager_zOS.h"
#endif

#ifdef PEGASUS_ENABLE_SLP
# include <Pegasus/Client/CIMClient.h>
#endif

#include "CIMServer.h"
#include "CIMOperationRequestDispatcher.h"
#include "CIMOperationResponseEncoder.h"
#include "CIMOperationRequestDecoder.h"
#include "CIMOperationRequestAuthorizer.h"
#include "HTTPAuthenticatorDelegator.h"
#include "ShutdownProvider.h"
#include "ShutdownService.h"
#include <Pegasus/Common/ModuleController.h>
#include <Pegasus/ControlProviders/ConfigSettingProvider/\
ConfigSettingProvider.h>
#include <Pegasus/ControlProviders/ProviderRegistrationProvider/\
ProviderRegistrationProvider.h>
#include <Pegasus/ControlProviders/NamespaceProvider/NamespaceProvider.h>

#ifndef PEGASUS_DISABLE_PERFINST
# include <Pegasus/ControlProviders/Statistic/CIMOMStatDataProvider.h>
#endif

#ifdef PEGASUS_HAS_SSL
# include <Pegasus/ControlProviders/CertificateProvider/CertificateProvider.h>
#endif

#ifdef PEGASUS_ENABLE_CQL
# include <Pegasus/ControlProviders/QueryCapabilitiesProvider/\
CIMQueryCapabilitiesProvider.h>
#endif

#if defined PEGASUS_ENABLE_INTEROP_PROVIDER
# include <Pegasus/ControlProviders/InteropProvider/InteropProvider.h>
#endif

#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
# include <Pegasus/WsmServer/WsmProcessor.h>
#endif

# include <Pegasus/RSServer/RsProcessor.h>

#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
# include <Pegasus/WebServer/WebServer.h>
#endif

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_SLP_REG_TIMEOUT
ThreadReturnType PEGASUS_THREAD_CDECL registerPegasusWithSLP(void *parm);
// Configurable SLP port to be handeled in a separate bug.
# define SLP_DEFAULT_PORT 427
# define LOCALHOST_IP "127.0.0.1"
#endif

static CIMServer* _cimserver = NULL;

// Need a static method to act as a callback for the control provider.
// This doesn't belong here, but I don't have a better place to put it.
static Message* controlProviderReceiveMessageCallback(
    Message* message,
    void* instance)
{
    CIMRequestMessage* request = dynamic_cast<CIMRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    Thread::setLanguages(
        ((AcceptLanguageListContainer) request->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages());

    ProviderMessageHandler* pmh =
        reinterpret_cast<ProviderMessageHandler*>(instance);

    MessageType reqType = request->getType();
    if (reqType == CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE)
    {
        pmh->terminate();
        return 0;
    }
    else if (reqType == CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE)
    {
        pmh->subscriptionInitComplete();
        return 0;
    }
    else if (reqType == CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE)
    {
        pmh->indicationServiceDisabled();
        return 0;
    }
    return pmh->processMessage(request);
}

//
// Signal handler for shutdown signals, currently SIGHUP and SIGTERM
//
Boolean handleShutdownSignal = false;
void shutdownSignalHandler(int s_n, PEGASUS_SIGINFO_T* s_info, void* sig)
{
    PEG_METHOD_ENTER(TRC_SERVER, "shutdownSignalHandler");
    PEG_TRACE((TRC_SERVER, Tracer::LEVEL3, "Signal %d received.", s_n));

    CIMServer::shutdownSignal();

    PEG_METHOD_EXIT();
}

void CIMServer::shutdownSignal()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::shutdownSignal()");
    handleShutdownSignal = true;
    _cimserver->tickle_monitor();
    PEG_METHOD_EXIT();
}

#ifdef PEGASUS_OS_PASE
static void _synchronousSignalHandler(int s_n, PEGASUS_SIGINFO_T* s_info,
                                      void* sig)
{
    static bool mark = false;
    if (mark)
        return;

    mark = true;

    Logger::put_l(Logger::ERROR_LOG, "CIMServer", Logger::SEVERE,
        MessageLoaderParms(
            "Pegasus.Server.CIMServer.RECEIVE_SYN_SIGNAL.PEGASUS_OS_PASE",
            "Synchronous signal received."));

    CIMServer::shutdownSignal();
}

static void _asynchronousSignalHandler(int s_n, PEGASUS_SIGINFO_T* s_info,
                                       void* sig)
{
    static bool mark = false;
    if (mark)
        return;

    mark = true;

    Logger::put_l(Logger::ERROR_LOG, "CIMServer", Logger::SEVERE,
        MessageLoaderParms(
            "Pegasus.Server.CIMServer.RECEIVE_ASYN_SIGNAL.PEGASUS_OS_PASE",
            "Asynchronous signal received."));

    CIMServer::shutdownSignal();
}
#endif

CIMServer::CIMServer()
    : _dieNow(false)
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::CIMServer()");
    _cimserver = this;
    _init();

    PEG_METHOD_EXIT();
}

void CIMServer::tickle_monitor()
{
    _monitor->tickle();
}

SCMOClass CIMServer::_scmoClassCache_GetClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    CIMClass cc;

    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::_scmoClassCache_GetClass()");
    try
    {
        cc = _cimserver->_repository->getClass(
            nameSpace,
            className,
            false, // localOnly
            true, // includeQualifiers
            true, // includeClassOrigin
            CIMPropertyList());
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
                   "The class '%s' in the name space '%s' was not found. "
                       "The repository throws the following exception: %s",
                   (const char*)className.getString().getCString(),
                   (const char*)nameSpace.getString().getCString(),
                   (const char*)e.getMessage().getCString()));
        // Return a empty class.
        PEG_METHOD_EXIT();
        return SCMOClass("","");
    }

    if (cc.isUninitialized())
    {
        // The requested class was not found !
        // Return a empty class.
        PEG_METHOD_EXIT();
        return SCMOClass("","");
    }
    PEG_METHOD_EXIT();
    return SCMOClass(cc,(const char*)nameSpace.getString().getCString());

}


void CIMServer::_init()
{

    // pre-initialize the hostname.
    System::getHostName();

    _monitor.reset(new Monitor());

#if (defined(PEGASUS_OS_HPUX) || defined(PEGASUS_OS_LINUX)) \
    && defined(PEGASUS_USE_RELEASE_DIRS)
    if (chdir(PEGASUS_CORE_DIR) != 0)
    {
        PEG_TRACE((TRC_SERVER, Tracer::LEVEL2,
            "chdir(\"%s\") failed with errno %d.", PEGASUS_CORE_DIR, errno));
    }
#endif

    // -- Create a repository:

    String repositoryRootPath =
        ConfigManager::getHomedPath(
            ConfigManager::getInstance()->getCurrentValue("repositoryDir"));

#ifdef DO_NOT_CREATE_REPOSITORY_ON_STARTUP
    // If this code is enable, the CIMServer will fail to start
    // if the repository directory does not exist. If called,
    // the Repository will create an empty repository.

    // This check has been disabled to allow cimmof to call
    // the CIMServer to build the initial repository.
    if (!FileSystem::isDirectory(repositoryRootPath))
    {
        throw NoSuchDirectory(repositoryRootPath);
    }
#endif

    _repository = new CIMRepository(repositoryRootPath);

    // -- Create a UserManager object:
#ifndef PEGASUS_PAM_AUTHENTICATION
    UserManager::getInstance(_repository);
#endif


    // -- Create a SCMOClass Cache and set call back for the repository

    SCMOClassCache::getInstance()->setCallBack(_scmoClassCache_GetClass);

    // -- Create a CIMServerState object:

    _serverState.reset(new CIMServerState());

    _providerRegistrationManager = new ProviderRegistrationManager(_repository);

    // -- Create queue inter-connections:

    _providerManager = new ProviderManagerService(
        _providerRegistrationManager,
        _repository,
        DefaultProviderManager::createDefaultProviderManagerCallback);

    // Create IndicationHandlerService:

    _handlerService = new IndicationHandlerService(_repository);

    _cimOperationRequestDispatcher = new CIMOperationRequestDispatcher(
        _repository, _providerRegistrationManager);

    // Create the control service
    _controlService = new ModuleController(PEGASUS_QUEUENAME_CONTROLSERVICE);

    // Jump this number up when there are more control providers.
    _controlProviders.reserveCapacity(16);

    // Create the Configuration control provider
    ProviderMessageHandler* configProvider = new ProviderMessageHandler(
        "CIMServerControlProvider", "ConfigSettingProvider",
        new ConfigSettingProvider(), 0, 0, false);

    _controlProviders.append(configProvider);
    _controlService->register_module(
        PEGASUS_MODULENAME_CONFIGPROVIDER,
        configProvider,
        controlProviderReceiveMessageCallback);

#ifndef PEGASUS_PAM_AUTHENTICATION
    // Create the User/Authorization control provider
    ProviderMessageHandler* userAuthProvider = new ProviderMessageHandler(
        "CIMServerControlProvider", "UserAuthProvider",
        new UserAuthProvider(_repository), 0, 0, false);
    _controlProviders.append(userAuthProvider);
    _controlService->register_module(
        PEGASUS_MODULENAME_USERAUTHPROVIDER,
        userAuthProvider,
        controlProviderReceiveMessageCallback);
#endif

    // Create the Provider Registration control provider
    ProviderMessageHandler* provRegProvider = new ProviderMessageHandler(
        "CIMServerControlProvider", "ProviderRegistrationProvider",
        new ProviderRegistrationProvider(_providerRegistrationManager),
        ModuleController::indicationCallback, 0, false);
    // Warning: The ProviderRegistrationProvider destructor deletes
    // _providerRegistrationManager
    _controlProviders.append(provRegProvider);
    _controlService->register_module(
        PEGASUS_MODULENAME_PROVREGPROVIDER,
        provRegProvider,
        controlProviderReceiveMessageCallback);

    // Create the Shutdown control provider
    ProviderMessageHandler* shutdownProvider = new ProviderMessageHandler(
        "CIMServerControlProvider", "ShutdownProvider",
        new ShutdownProvider(this), 0, 0, false);
    _controlProviders.append(shutdownProvider);
    _controlService->register_module(
        PEGASUS_MODULENAME_SHUTDOWNPROVIDER,
        shutdownProvider,
        controlProviderReceiveMessageCallback);

    // Create the namespace control provider
    ProviderMessageHandler* namespaceProvider = new ProviderMessageHandler(
        "CIMServerControlProvider", "NamespaceProvider",
        new NamespaceProvider(_repository), 0, 0, false);
    _controlProviders.append(namespaceProvider);
    _controlService->register_module(
        PEGASUS_MODULENAME_NAMESPACEPROVIDER,
        namespaceProvider,
        controlProviderReceiveMessageCallback);

    //
    // Create a SSLContextManager object
    //
    _sslContextMgr = new SSLContextManager();

#ifdef PEGASUS_HAS_SSL
    // Because this provider allows management of the cimserver truststore
    // it needs to be available regardless of the value
    // of sslClientVerificationMode config property.
    ProviderMessageHandler* certificateProvider = new ProviderMessageHandler(
        "CIMServerControlProvider", "CertificateProvider",
        new CertificateProvider(_repository, _sslContextMgr),
        0, 0, false);
    _controlProviders.append(certificateProvider);
    _controlService->register_module(
        PEGASUS_MODULENAME_CERTIFICATEPROVIDER,
        certificateProvider,
        controlProviderReceiveMessageCallback);
#endif

#ifndef PEGASUS_DISABLE_PERFINST
    // Create the Statistical Data control provider
    ProviderMessageHandler* cimomstatdataProvider = new ProviderMessageHandler(
        "CIMServerControlProvider", "CIMOMStatDataProvider",
        new CIMOMStatDataProvider(), 0, 0, false);
    _controlProviders.append(cimomstatdataProvider);
    _controlService->register_module(
        PEGASUS_MODULENAME_CIMOMSTATDATAPROVIDER,
        cimomstatdataProvider,
        controlProviderReceiveMessageCallback);
#endif

#ifdef PEGASUS_ENABLE_CQL

    // Create the Query Capabilities control provider
    ProviderMessageHandler* cimquerycapprovider = new ProviderMessageHandler(
        "CIMServerControlProvider", "CIMQueryCapabilitiesProvider",
        new CIMQueryCapabilitiesProvider(),
        0, 0, false);
    _controlProviders.append(cimquerycapprovider);
    _controlService->register_module(
        PEGASUS_MODULENAME_CIMQUERYCAPPROVIDER,
        cimquerycapprovider,
        controlProviderReceiveMessageCallback);
#endif

#if defined PEGASUS_ENABLE_INTEROP_PROVIDER

    // Create the interop control provider
    ProviderMessageHandler* interopProvider = new ProviderMessageHandler(
        "CIMServerControlProvider", "InteropProvider",
        new InteropProvider(
                _repository,
                _providerRegistrationManager),
        0, 0, false);

    _controlProviders.append(interopProvider);
    _controlService->register_module(
        PEGASUS_MODULENAME_INTEROPPROVIDER,
        interopProvider,
        controlProviderReceiveMessageCallback);
#endif

    _cimOperationResponseEncoder = new CIMOperationResponseEncoder;

    //
    // get the configured authentication and authorization flags
    //
    ConfigManager* configManager = ConfigManager::getInstance();

    Boolean enableAuthentication = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableAuthentication"));

    MessageQueue* cimOperationProcessorQueue = 0;

    //
    // Create Authorization queue only if authentication is enabled
    //
    if ( enableAuthentication )
    {
        _cimOperationRequestAuthorizer = new CIMOperationRequestAuthorizer(
            _cimOperationRequestDispatcher);
        cimOperationProcessorQueue = _cimOperationRequestAuthorizer;
    }
    else
    {
        _cimOperationRequestAuthorizer = 0;
        cimOperationProcessorQueue = _cimOperationRequestDispatcher;
    }

    _cimOperationRequestDecoder = new CIMOperationRequestDecoder(
        cimOperationProcessorQueue,
        _cimOperationResponseEncoder->getQueueId());

    _cimExportRequestDispatcher = new CIMExportRequestDispatcher();

    _cimExportResponseEncoder = new CIMExportResponseEncoder;

    _cimExportRequestDecoder = new CIMExportRequestDecoder(
        _cimExportRequestDispatcher,
        _cimExportResponseEncoder->getQueueId());

    _httpAuthenticatorDelegator = new HTTPAuthenticatorDelegator(
        _cimOperationRequestDecoder->getQueueId(),
        _cimExportRequestDecoder->getQueueId(),
        _repository);

    _rsProcessor = new RsProcessor(
        cimOperationProcessorQueue,
        _repository);
    _httpAuthenticatorDelegator->setRsQueueId(
        _rsProcessor->getRsRequestDecoderQueueId());

#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    _webServer = new WebServer();
    _httpAuthenticatorDelegator->setWebQueueId(
        _webServer->getQueueId());
#endif

#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
    _wsmProcessor = new WsmProcessor(
        cimOperationProcessorQueue,
        _repository);
    _httpAuthenticatorDelegator->setWsmQueueId(
        _wsmProcessor->getWsmRequestDecoderQueueId());
#endif

    // IMPORTANT-NU-20020513: Indication service must start after ExportService
    // otherwise HandlerService started by indicationService will never
    // get ExportQueue to export indications for existing subscriptions

    _indicationService = new IndicationService(
        _repository,
        _providerRegistrationManager);

    // Build the Control Provider and Service internal routing table. This must
    // be called after MessageQueueService initialized and ServiceQueueIds
    // installed.

    DynamicRoutingTable::buildRoutingTable();

    // Enable the signal handler to shutdown gracefully on SIGHUP and SIGTERM
    getSigHandle()->registerHandler(PEGASUS_SIGHUP, shutdownSignalHandler);
    getSigHandle()->activate(PEGASUS_SIGHUP);
    getSigHandle()->registerHandler(PEGASUS_SIGTERM, shutdownSignalHandler);
    getSigHandle()->activate(PEGASUS_SIGTERM);
#ifdef PEGASUS_OS_PASE
    getSigHandle()->registerHandler(SIGFPE, _synchronousSignalHandler);
    getSigHandle()->activate(SIGFPE);
    getSigHandle()->registerHandler(SIGILL, _synchronousSignalHandler);
    getSigHandle()->activate(SIGILL);
    getSigHandle()->registerHandler(SIGSEGV, _synchronousSignalHandler);
    getSigHandle()->activate(SIGSEGV);
    getSigHandle()->registerHandler(SIGIO, _asynchronousSignalHandler);
    getSigHandle()->activate(SIGIO);
#endif
#ifdef PEGASUS_OS_ZOS
    // Establish handling signal send to us on USS shutdown
    getSigHandle()->registerHandler(PEGASUS_SIGDANGER, shutdownSignalHandler);
    getSigHandle()->activate(PEGASUS_SIGDANGER);
    // enable process to receive SIGDANGER on USS shutdown
    __shutdown_registration(_SDR_NOTIFY, _SDR_REGPROCESS, _SDR_SENDSIGDANGER);
#endif

    //
    // Set up an additional thread waiting for commands from the
    // system console
    //
#if defined PEGASUS_OS_ZOS
    ZOSConsoleManager::startConsoleWatchThread();
#endif

#ifdef PEGASUS_ENABLE_AUDIT_LOGGER

    // Register audit logger initialize callback
    AuditLogger::setInitializeCallback(auditLogInitializeCallback);

    Boolean enableAuditLog = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableAuditLog"));

    if (enableAuditLog)
    {
        AuditLogger::setEnabled(enableAuditLog);
    }

#endif
}

void CIMServer::initComplete()
{
    // CIMServer initialization is now complete. Inform indication service
    // to send CIMSubscriptionInitCompleteRequestMessage.
    if (_indicationService)
    {
        _indicationService->sendSubscriptionInitComplete();
    }
    _providerRegistrationManager->setInitComplete();
}

CIMServer* CIMServer::getInstance()
{
    return _cimserver;
}

CIMServer::~CIMServer ()
{
    PEG_METHOD_ENTER (TRC_SERVER, "CIMServer::~CIMServer()");

    // Wait until the Shutdown provider request has cleared through the
    // system.
    ShutdownService::getInstance(this)->waitUntilNoMoreRequests(false);

    // Make sure no providers are running (since they might depend on the
    // services we are about to delete).
    ShutdownService::getInstance(this)->shutdownProviders();

    // Ok, shutdown all the MQSs. This shuts their communication channel.
    ShutdownService::getInstance(this)->shutdownCimomServices();

    // Start deleting the objects.
    // The order is very important.

    // The HTTPAcceptor depends on HTTPAuthenticationDelegator
    for (Uint32 i = 0, n = _acceptors.size (); i < n; i++)
    {
        HTTPAcceptor* p = _acceptors[i];
        delete p;
    }

    // IndicationService depends on ProviderManagerService,
    // IndicationHandlerService, and ProviderRegistrationManager, and thus
    // should be deleted before the ProviderManagerService,
    // IndicationHandlerService, and ProviderRegistrationManager are deleted.
    delete _indicationService;

    // HTTPAuthenticationDelegator depends on CIMRepository,
    // CIMOperationRequestDecoder and CIMExportRequestDecoder
    delete _httpAuthenticatorDelegator;

    delete _cimExportRequestDecoder;

    delete _cimExportResponseEncoder;

    delete _cimExportRequestDispatcher;

    // CIMOperationRequestDecoder depends on CIMOperationRequestAuthorizer
    // and CIMOperationResponseEncoder
    delete _cimOperationRequestDecoder;
    delete _cimOperationResponseEncoder;

    delete _rsProcessor;
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    delete _webServer;
#endif
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
    // WsmProcessor depends on CIMOperationRequestAuthorizer/Dispatcher and
    // CIMRepository
    delete _wsmProcessor;
#endif

    // CIMOperationRequestAuthorizer depends on
    // CIMOperationRequestDispatcher
    delete _cimOperationRequestAuthorizer;

    // IndicationHandlerService uses CIMOperationRequestDispatcher
    delete _handlerService;

    // CIMOperationRequestDispatcher depends on
    // CIMRepository and ProviderRegistrationManager.
    // CIMOperationRequestDispatcher keeps an internal list of control
    // providers. Must delete this before ModuleController.
    delete _cimOperationRequestDispatcher;

    // ModuleController takes care of deleting all wrappers around
    // the control providers.
    delete _controlService;

    // Find all of the control providers (module)
    // Must delete CIMOperationRequestDispatcher _before_ deleting each
    // of the control provider. The CIMOperationRequestDispatcher keeps
    // its own table of the internal providers (pointers).
    for (Uint32 i = 0, n = _controlProviders.size(); i < n; i++)
    {
        ProviderMessageHandler* p = _controlProviders[i];
        delete p->getProvider();
        delete p;
    }

    // The SSL control providers use the SSL context manager.
    delete _sslContextMgr;

    // ProviderManagerService depends on ProviderRegistrationManager.
    // Note that deleting the ProviderManagerService causes the
    // DefaultProviderManager (if loaded) to get unloaded.  Dynamically
    // unloading the DefaultProviderManager library affects (on HP-UX, at
    // least) the statically loaded version of this library used by the
    // ProviderMessageHandler wrapper for the control providers.  Deleting
    // the ProviderManagerService after the control providers is a
    // workaround for this problem.
    delete _providerManager;

    delete _providerRegistrationManager;

    // Almost everybody uses the CIMRepository.
    delete _repository;

    // Destroy the singleton services
    SCMOClassCache::destroy();
#ifndef PEGASUS_PAM_AUTHENTICATION
    UserManager::destroy();
#endif
    ShutdownService::destroy();

    PEG_METHOD_EXIT ();
}

void CIMServer::addAcceptor(
    Uint16 connectionType,
    Uint32 portNumber,
    Boolean useSSL,
    HostAddress *ipAddress)
{
    HTTPAcceptor* acceptor;

    acceptor = new HTTPAcceptor(
        _monitor.get(),
        _httpAuthenticatorDelegator,
        connectionType,
        portNumber,
        useSSL ? _getSSLContext() : 0,
        useSSL ? _sslContextMgr->getSSLContextObjectLock() : 0,
        ipAddress);

    _acceptors.append(acceptor);
}

void CIMServer::bind()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::bind()");

    if (_acceptors.size() == 0)
    {
        MessageLoaderParms mlp = MessageLoaderParms(
            "Server.CIMServer.BIND_FAILED",
            "No CIM Server connections are enabled.");

        throw BindFailedException(mlp);
    }

    for (Uint32 i=0; i<_acceptors.size(); i++)
    {
        _acceptors[i]->bind();
    }

    PEG_METHOD_EXIT();
}

// This is the primary server runloop.  It loops until a shutdown
// received
void CIMServer::runForever()
{
    // Note: Trace code in this method will be invoked frequently.

    if (!_dieNow)
    {
        _monitor->run(500000);

        static struct timeval lastIdleCleanupTime = {0, 0};
        struct timeval now;
        Time::gettimeofday(&now);

        if (now.tv_sec - lastIdleCleanupTime.tv_sec > 300)
        {
            lastIdleCleanupTime.tv_sec = now.tv_sec;

            try
            {
                // clean up expired sessions
                _httpAuthenticatorDelegator->idleTimeCleanup();

                // clean up idle providers. This starts separate thread
                _providerManager->idleTimeCleanup();

                MessageQueueService::get_thread_pool()->cleanupIdleThreads();

#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
                _wsmProcessor->cleanupExpiredContexts();
#endif
            }
            catch (...)
            {
            }
        }

        if (handleShutdownSignal)
        {
            PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
                "CIMServer::runForever - signal received.  Shutting down.");

            //same statement as that of getShutdownTimeout
            //TODO: DL consolidate in one place
            ConfigManager *cfgManager = ConfigManager::getInstance();
            String shutdnTimestr = cfgManager ->getCurrentValue(
                "shutdownTimeout");

            Uint64 shutdownTimeout = 0;

            StringConversion::decimalStringToUint64(
                    shutdnTimestr.getCString(), shutdownTimeout);

            ShutdownService::getInstance(this)->shutdown(
                true,
                (Uint32)shutdownTimeout,
                false);


            // Set to false must be after call to shutdown.  See
            // stopClientConnection.
            handleShutdownSignal = false;
        }
    }
}

void CIMServer::stopClientConnection()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::stopClientConnection()");

    // tell Monitor to stop listening for client connections
    if (handleShutdownSignal)
        // If shutting down, this is in the same thread as runForever.
        // No need to wait for the thread to see the stop flag.
        _monitor->stopListeningForConnections(false);
    else
        // If not shutting down, this is not in the same thread as runForever.
        // Need to wait for the thread to see the stop flag.
        _monitor->stopListeningForConnections(true);

    //
    // Wait 150 milliseconds to allow time for the Monitor to stop
    // listening for client connections.
    //
    // This wait time is the timeout value for the select() call
    // in the Monitor's run() method (currently set to 100
    // milliseconds) plus a delta of 50 milliseconds.  The reason
    // for the wait here is to make sure that the Monitor entries
    // are updated before closing the connection sockets.
    //
    // PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL4, "Wait 150 milliseconds.");
    //  Threads::sleep(150);  not needed anymore due to the semaphore
    // in the monitor

    for (Uint32 i=0; i<_acceptors.size(); i++)
    {
        _acceptors[i]->closeConnectionSocket();
    }

    PEG_METHOD_EXIT();
}

void CIMServer::shutdown()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::shutdown()");

#ifdef PEGASUS_DEBUG
    _repository->DisplayCacheStatistics();
    SCMOClassCache::getInstance()->DisplayCacheStatistics();
#endif

    _dieNow = true;
    _cimserver->tickle_monitor();

    PEG_METHOD_EXIT();
}

void CIMServer::resume()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::resume()");

    for (Uint32 i=0; i<_acceptors.size(); i++)
    {
        _acceptors[i]->reopenConnectionSocket();
    }

    PEG_METHOD_EXIT();
}

void CIMServer::setState(Uint32 state)
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::setState()");

    _serverState->setState(state);

    //
    // get the configured authentication and authorization flags
    //
    ConfigManager* configManager = ConfigManager::getInstance();

    Boolean enableAuthentication = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableAuthentication"));
    Boolean enableNamespaceAuthorization = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableNamespaceAuthorization"));

    if (state == CIMServerState::TERMINATING)
    {
        // tell decoder that CIMServer is terminating
        _cimOperationRequestDecoder->setServerTerminating(true);
        _cimExportRequestDecoder->setServerTerminating(true);
        _rsProcessor->setServerTerminating(true);
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
        _wsmProcessor->setServerTerminating(true);
#endif

        // tell authorizer that CIMServer is terminating ONLY if
        // authentication and authorization are enabled
        //
        if ( enableAuthentication && enableNamespaceAuthorization )
        {
            _cimOperationRequestAuthorizer->setServerTerminating(true);
        }
    }
    else
    {
        // tell decoder that CIMServer is not terminating
        _cimOperationRequestDecoder->setServerTerminating(false);
        _cimExportRequestDecoder->setServerTerminating(false);

        _rsProcessor->setServerTerminating(false);
#ifdef PEGASUS_ENABLE_PROTOCOL_WSMAN
        _wsmProcessor->setServerTerminating(false);
#endif

        // tell authorizer that CIMServer is terminating ONLY if
        // authentication and authorization are enabled
        //
        if ( enableAuthentication && enableNamespaceAuthorization )
        {
            _cimOperationRequestAuthorizer->setServerTerminating(false);
        }
    }
    PEG_METHOD_EXIT();
}

Uint32 CIMServer::getOutstandingRequestCount()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::getOutstandingRequestCount()");

    Uint32 requestCount = 0;

    for (Uint32 i=0; i<_acceptors.size(); i++)
    {
        requestCount += _acceptors[i]->getOutstandingRequestCount();
    }

    PEG_METHOD_EXIT();
    return requestCount;
}

//
SSLContext* CIMServer::_getSSLContext()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::_getSSLContext()");

    static const String PROPERTY_NAME__SSL_CERT_FILEPATH =
        "sslCertificateFilePath";
    static const String PROPERTY_NAME__SSL_KEY_FILEPATH = "sslKeyFilePath";
    static const String PROPERTY_NAME__SSL_TRUST_STORE = "sslTrustStore";
    static const String PROPERTY_NAME__SSL_CRL_STORE = "crlStore";
    static const String PROPERTY_NAME__SSL_CLIENT_VERIFICATION =
        "sslClientVerificationMode";
    static const String PROPERTY_NAME__SSL_AUTO_TRUST_STORE_UPDATE =
        "enableSSLTrustStoreAutoUpdate";
    static const String PROPERTY_NAME__SSL_TRUST_STORE_USERNAME =
        "sslTrustStoreUserName";
    static const String PROPERTY_NAME__HTTP_ENABLED =
        "enableHttpConnection";
    static const String PROPERTY_NAME__SSL_CIPHER_SUITE = "sslCipherSuite";
    static const String PROPERTY_NAME__SSL_COMPATIBILITY =
        "sslBackwardCompatibility";

    String verifyClient;
    String trustStore;
    SSLContext* sslContext = 0;

    //
    // Get a config manager instance
    //
    ConfigManager* configManager = ConfigManager::getInstance();

    // Note that if invalid values were set for either sslKeyFilePath,
    // sslCertificateFilePath, crlStore or sslTrustStore, the invalid
    // paths would have been detected in SecurityPropertyOwner and
    // terminated the server startup. This happens regardless of whether
    // or not HTTPS is enabled (not a great design, but that seems to
    // be how other properties are validated as well)
    //
    // Get the sslClientVerificationMode property from the Config
    // Manager.
    //
    verifyClient = configManager->getCurrentValue(
        PROPERTY_NAME__SSL_CLIENT_VERIFICATION);

    //
    // Get the sslTrustStore property from the Config Manager.
    //
    trustStore = configManager->getCurrentValue(
        PROPERTY_NAME__SSL_TRUST_STORE);

    if (trustStore != String::EMPTY)
    {
        trustStore = ConfigManager::getHomedPath(trustStore);
    }

    PEG_TRACE((TRC_SERVER, Tracer::LEVEL4,"Server trust store name: %s",
        (const char*)trustStore.getCString()));

    //
    // Get the sslTrustStoreUserName property from the Config Manager.
    //
    String trustStoreUserName;
    trustStoreUserName = configManager->getCurrentValue(
        PROPERTY_NAME__SSL_TRUST_STORE_USERNAME);

    if (!String::equal(verifyClient, "disabled"))
    {
        //
        // 'required' and 'optional' settings must have a valid truststore
        //
        if (trustStore == String::EMPTY)
        {
            MessageLoaderParms parms(
                "Pegasus.Server.SSLContextManager."
                    "SSL_CLIENT_VERIFICATION_EMPTY_TRUSTSTORE",
                "The \"sslTrustStore\" configuration property must be set "
                    "if \"sslClientVerificationMode\" is 'required' or "
                    "'optional'.");
            PEG_METHOD_EXIT();
            throw SSLException(parms);
        }

#ifdef PEGASUS_DISABLE_LOCAL_DOMAIN_SOCKET
        //
        // ATTN: 'required' setting must have http port enabled.
        // If only https is enabled, and a call to shutdown the
        // cimserver is given, the call will hang and a forced shutdown
        // will ensue. This is because the CIMClient::connectLocal call
        // cannot specify a certificate for authentication against
        // the local server.  This limitation is being investigated.
        // See Bugzilla 2995.
        //
        if (String::equal(verifyClient, "required"))
        {
            if (!ConfigManager::parseBooleanValue(
                configManager->getCurrentValue(
                    PROPERTY_NAME__HTTP_ENABLED)))
            {
                MessageLoaderParms parms(
                    "Pegasus.Server.SSLContextManager."
                        "INVALID_CONF_HTTPS_REQUIRED",
                    "The \"sslClientVerificationMode\" property cannot be "
                        "set to \"required\" if HTTP is disabled, as the "
                        "cimserver will be unable to properly shutdown.  "
                        "The recommended course of action is to change "
                        "the property value to \"optional\".");
                PEG_METHOD_EXIT();
                throw SSLException(parms);
            }
        }
#endif
        //
        // A truststore username must be specified if
        // sslClientVerificationMode is enabled and the truststore is a
        // single CA file.  If the truststore is a directory, then the
        // CertificateProvider should be used to register users with
        // certificates.
        //

        if (trustStore != String::EMPTY)
        {
            if (!FileSystem::exists(trustStore))
            {
                MessageLoaderParms parms(
                    "Pegasus.Server.SSLContextManager."
                        "COULD_NOT_ACCESS_TRUST_STORE",
                    "Could not access the trust store."
                        "Check the permissions of the truststore path \"$0\".",
                        trustStore);
                PEG_METHOD_EXIT();
                throw SSLException(parms);
            }

            if (!FileSystem::isDirectory(trustStore))
            {
                if (trustStoreUserName == String::EMPTY)
                {
                    MessageLoaderParms parms(
                        "Pegasus.Server.SSLContextManager."
                            "SSL_CLIENT_VERIFICATION_EMPTY_USERNAME",
                        "The \"sslTrustStoreUserName\" property must specify a "
                            "valid username if \"sslClientVerificationMode\" "
                            "is 'required' or 'optional' and the truststore is "
                            "a single CA file. To register individual "
                            "certificates to users, you must use a truststore "
                            "directory along with the CertificateProvider.");
                    PEG_METHOD_EXIT();
                    throw SSLException(parms);
                }
            }
        }
    }

#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    //
    // Get the crlStore property from the Config Manager.
    //
    String crlStore = configManager->getCurrentValue(
        PROPERTY_NAME__SSL_CRL_STORE);

    if (crlStore != String::EMPTY)
    {
        crlStore = ConfigManager::getHomedPath(crlStore);
    }
#else
    String crlStore;
#endif

    //
    // Get the sslCertificateFilePath property from the Config Manager.
    //
    String certPath;
    certPath = ConfigManager::getHomedPath(
        configManager->getCurrentValue(PROPERTY_NAME__SSL_CERT_FILEPATH));

    //
    // Get the sslKeyFilePath property from the Config Manager.
    //
    String keyPath;
    keyPath = ConfigManager::getHomedPath(
        configManager->getCurrentValue(PROPERTY_NAME__SSL_KEY_FILEPATH));

    String randFile;

#ifdef PEGASUS_SSL_RANDOMFILE
    // NOTE: It is technically not necessary to set up a random file on
    // the server side, but it is easier to use a consistent interface
    // on the client and server than to optimize out the random file on
    // the server side.
    randFile = ConfigManager::getHomedPath(PEGASUS_SSLSERVER_RANDOMFILE);
#endif

    //
    // Get the cipherSuite property from the Config Manager.
    //
    String cipherSuite = configManager->getCurrentValue(
        PROPERTY_NAME__SSL_CIPHER_SUITE);
    PEG_TRACE((TRC_SERVER, Tracer::LEVEL4, "Cipher suite is %s",
        (const char*)cipherSuite.getCString()));

    Boolean sslCompatibility = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue(
        PROPERTY_NAME__SSL_COMPATIBILITY));
    //
    // Create the SSLContext defined by the configuration properties
    //
    if (String::equal(verifyClient, "required"))
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
            "SSL Client verification REQUIRED.");

        _sslContextMgr->createSSLContext(
            trustStore, certPath, keyPath, crlStore, false, randFile,
            cipherSuite,sslCompatibility);
    }
    else if (String::equal(verifyClient, "optional"))
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
            "SSL Client verification OPTIONAL.");

        _sslContextMgr->createSSLContext(
            trustStore, certPath, keyPath, crlStore, true, randFile,
            cipherSuite,sslCompatibility);
    }
    else if (String::equal(verifyClient, "disabled") ||
             verifyClient == String::EMPTY)
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
            "SSL Client verification DISABLED.");

        _sslContextMgr->createSSLContext(
            String::EMPTY, certPath, keyPath, crlStore, false, randFile,
            cipherSuite,sslCompatibility);
    }
    sslContext = _sslContextMgr->getSSLContext();

    try
    {
        sslContext->_validateCertificate();
    }
    catch (SSLException& e)
    {
        Logger::put(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            e.getMessage());
    }

    PEG_METHOD_EXIT();
    return sslContext;
}

void CIMServer::auditLogInitializeCallback()
{
#ifdef PEGASUS_ENABLE_AUDIT_LOGGER

    Array<String> propertyNames;
    Array<String> propertyValues;

    // Get all current property names and values
    ConfigManager* configManager = ConfigManager::getInstance();

    configManager->getAllPropertyNames(propertyNames, false);

    for (Uint32 i = 0; i < propertyNames.size(); i++)
    {
        propertyValues.append(configManager->getCurrentValue(propertyNames[i]));
    }

    AuditLogger::logCurrentConfig(propertyNames, propertyValues);

    // get currently registered provider module instances
    Array<CIMInstance> moduleInstances;

    moduleInstances =
        _cimserver->_providerRegistrationManager->enumerateInstancesForClass(
        CIMObjectPath("PG_ProviderModule"));

    AuditLogger::logCurrentRegProvider(moduleInstances);

    AuditLogger::logCurrentEnvironmentVar();

#endif
}



#ifdef PEGASUS_ENABLE_SLP

ThreadReturnType PEGASUS_THREAD_CDECL _callSLPProvider(void* parm);

// This is a control function that starts a new thread which issues a
// cim operation to start the slp provider. Used for both External and
// Embedded SLP registrations.
//

void CIMServer::startSLPProvider()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::startSLPProvider");

    // Get Config parameter to determine if we should start SLP.
    ConfigManager* configManager = ConfigManager::getInstance();
    Boolean _runSLP = ConfigManager::parseBooleanValue(
         configManager->getCurrentValue("slp"));

    // If false, do not start slp provider
    if (!_runSLP)
    {
        PEG_METHOD_EXIT();
        return;
    }
    // Create a separate thread, detach and call function to execute the
    // startup.
    Thread t( _callSLPProvider, 0, true );
    t.run();

    PEG_METHOD_EXIT();
    return;
}



// startSLPProvider is a function to get the slp provider kicked off
// during startup.  It is placed in the provider manager simply because
// the provider manager is the only component of the system is
// driven by a timer after startup.  It should never be here and must be
// moved to somewhere more logical or really replaced. We simply needed
// something that was run shortly after system startup.
// This function is assumed to operate in a separate thread and
// KS 15 February 2004.

ThreadReturnType PEGASUS_THREAD_CDECL _callSLPProvider(void* parm)
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::_callSLPProvider()");

    CIMClient client;
    String hostStr = System::getHostName();

    try
    {
        client.connectLocal();

        String timeoutStr=
            ConfigManager::getInstance()->getCurrentValue(
                "slpProviderStartupTimeout");
        Uint64 timeOut;

        StringConversion::decimalStringToUint64(
            timeoutStr.getCString(),
            timeOut);
        client.setTimeout(timeOut & 0xFFFFFFFF);

        String referenceStr = "//";
        referenceStr.append(hostStr);
        referenceStr.append("/");
        referenceStr.append(PEGASUS_NAMESPACENAME_INTERNAL.getString());
        referenceStr.append(":");
        referenceStr.append(PEGASUS_CLASSNAME_WBEMSLPTEMPLATE.getString());
        CIMObjectPath reference(referenceStr);
        //
        // issue the invokeMethod request on the register method
        //
        Array<CIMParamValue> inParams;
        Array<CIMParamValue> outParams;

        Uint32 retries = 3;

        while (retries > 0)
        {
            try
            {
                CIMValue retValue = client.invokeMethod(
                    PEGASUS_NAMESPACENAME_INTERNAL,
                    reference,
                    CIMName("register"),
                    inParams,
                    outParams);
                break;
            }
            catch(ConnectionTimeoutException&)
            {
                retries--;
                if (retries == 0)
                {
                    throw;
                }
            }
        }

        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::INFORMATION,
            MessageLoaderParms(
                "Pegasus.Server.SLP.SLP_REGISTRATION_INITIATED",
                "SLP Registration Initiated"));
    }

    catch(Exception& e)
    {
#ifdef PEGASUS_SLP_REG_TIMEOUT
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "Pegasus.Server.SLP.EXTERNAL_SLP_REGISTRATION_FAILED_EXCEPTION",
                "CIM Server registration with External SLP Failed. "
                    "Exception: $0",
                e.getMessage()));
#else
        Logger::put_l(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "Pegasus.Server.SLP.INTERNAL_SLP_REGISTRATION_FAILED_EXCEPTION",
                "CIM Server registration with Internal SLP Failed. "
                    "Exception: $0",
                e.getMessage()));
#endif
    }

    catch(...)
    {
#ifdef PEGASUS_SLP_REG_TIMEOUT
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "Pegasus.Server.SLP.EXTERNAL_SLP_REGISTRATION_FAILED_ERROR",
                "CIM Server registration with External SLP Failed."));
#else
        Logger::put_l(Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            MessageLoaderParms(
                "Pegasus.Server.SLP.INTERNAL_SLP_REGISTRATION_FAILED_ERROR",
                "CIM Server registration with Internal SLP Failed."));
#endif
    }

    client.disconnect();

    PEG_METHOD_EXIT();
    return( (ThreadReturnType)32 );
}
#endif

PEGASUS_NAMESPACE_END
