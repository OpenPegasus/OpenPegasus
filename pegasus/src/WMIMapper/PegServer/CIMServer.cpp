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

#include <Pegasus/Common/Config.h>

#include <iostream>
#include <cstdio>
#include <cctype>
#include <ctime>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/HTTPAcceptor.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Cimom.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Signal.h>
#include <Pegasus/General/SSLContextManager.h>

#include <Pegasus/Repository/CIMRepository.h>
//#include "ProviderMessageFacade.h"
#include <Pegasus/ExportServer/CIMExportRequestDispatcher.h>
#include <Pegasus/ExportServer/CIMExportResponseEncoder.h>
#include <Pegasus/ExportServer/CIMExportRequestDecoder.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Security/UserManager/UserManager.h>
//#include <Pegasus/HandlerService/IndicationHandlerService.h>
//#include <Pegasus/IndicationService/IndicationService.h>
//#include <Pegasus/ProviderManager2/ProviderManagerService.h>


#include "CIMServer.h"
#include "CIMOperationRequestDispatcher.h"
#include "CIMOperationResponseEncoder.h"
#include "CIMOperationRequestDecoder.h"
#include "CIMOperationRequestAuthorizer.h"
#include "HTTPAuthenticatorDelegator.h"

#include <Pegasus/Common/MessageLoader.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static CIMServer *_cimserver = NULL;
Boolean handleShutdownSignal = false;
void CIMServer::shutdownSignal()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::shutdownSignal()");
    handleShutdownSignal = true;
    _cimserver->tickle_monitor();
    PEG_METHOD_EXIT();
}


CIMServer::CIMServer()
  : _dieNow(false)
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::CIMServer()");
    _init();
    _cimserver = this;
    PEG_METHOD_EXIT();
}


void CIMServer::tickle_monitor()
{
    _monitor->tickle();
}

void CIMServer::_init()
{
    _monitor.reset(new Monitor());

    String repositoryRootPath;

#if defined(PEGASUS_OS_HPUX) && defined(PEGASUS_USE_RELEASE_DIRS)
    chdir( PEGASUS_CORE_DIR );
#endif
    // -- Save the monitor or create a new one:
    repositoryRootPath = ConfigManager::getHomedPath(
        ConfigManager::getInstance()->getCurrentValue("repositoryDir"));

    // -- Create a repository:

/*
#ifdef DO_NOT_CREATE_REPOSITORY_ON_STARTUP
    // If this code is enable, the CIMServer will fail to start
    // if the repository directory does not exit. If called,
    // the Repository will create an empty repository.

    // This check has been disabled to allow cimmof to call
    // the CIMServer to build the initial repository.
    if (!FileSystem::isDirectory(repositoryRootPath))
    {
        PEG_METHOD_EXIT();
        throw NoSuchDirectory(repositoryRootPath);

    }
#endif
*/
    _repository = new CIMRepository(repositoryRootPath);

    // -- Create a UserManager object:

    UserManager* userManager = UserManager::getInstance(_repository);

    // -- Create a CIMServerState object:

    _serverState.reset(new CIMServerState());



     //
     // Create a SSLContextManager object
     //
     _sslContextMgr = new SSLContextManager();

    _cimOperationRequestDispatcher =
        new CIMOperationRequestDispatcher;

    _cimOperationResponseEncoder
        = new CIMOperationResponseEncoder;

    //
    // get the configured authentication and authorization flags
    //
    ConfigManager* configManager = ConfigManager::getInstance();

    Boolean enableAuthentication = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableAuthentication"));

    //
    // Create Authorization queue only if authentication is enabled
    //
    if ( enableAuthentication )
    {
        _cimOperationRequestAuthorizer = new CIMOperationRequestAuthorizer(
            _cimOperationRequestDispatcher);

        _cimOperationRequestDecoder = new CIMOperationRequestDecoder(
            _cimOperationRequestAuthorizer,
            _cimOperationResponseEncoder->getQueueId());
    }
    else
    {
        _cimOperationRequestAuthorizer = 0;

        _cimOperationRequestDecoder = new CIMOperationRequestDecoder(
            _cimOperationRequestDispatcher,
            _cimOperationResponseEncoder->getQueueId());

    }

    _cimExportRequestDispatcher
        = new CIMExportRequestDispatcher();

    _cimExportResponseEncoder
        = new CIMExportResponseEncoder;

    _cimExportRequestDecoder = new CIMExportRequestDecoder(
        _cimExportRequestDispatcher,
        _cimExportResponseEncoder->getQueueId());

    _httpAuthenticatorDelegator = new HTTPAuthenticatorDelegator(
        _cimOperationRequestDecoder->getQueueId(),
        _cimExportRequestDecoder->getQueueId(),
        _repository);

    // IMPORTANT-NU-20020513: Indication service must start after ExportService
    // otherwise HandlerService started by indicationService will never
    // get ExportQueue to export indications for existing subscriptions

/*
    _indicationService = 0;
    if (ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableIndicationService")))
    {
        _indicationService = new IndicationService
            (_repository, _providerRegistrationManager);
    }

    // Enable the signal handler to shutdown gracefully on SIGHUP and SIGTERM
    getSigHandle()->registerHandler(PEGASUS_SIGHUP, shutdownSignalHandler);
    getSigHandle()->activate(PEGASUS_SIGHUP);
    getSigHandle()->registerHandler(PEGASUS_SIGTERM, shutdownSignalHandler);
    getSigHandle()->activate(PEGASUS_SIGTERM);
*/
}



CIMServer::~CIMServer()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::~CIMServer()");

    // Start deleting the objects.
     for (Uint32 i = 0, n = _acceptors.size (); i < n; i++)
       {
         HTTPAcceptor *p = _acceptors[i];
         delete p;
       }
     // The order is very important.

     // 13
     /*if (_indicationService)
       {
         delete _indicationService;
       } */

     // 12
     // HTTPAuthenticationDelegaor depends on
     // CIMRepository, CIMOperationRequestDecode and
     // CIMExportRequestDecoder
     if (_httpAuthenticatorDelegator)
       {
         delete _httpAuthenticatorDelegator;
       }
     // 11
     if (_cimExportRequestDecoder)
       {
         delete _cimExportRequestDecoder;
       }
     // 10
     if (_cimExportResponseEncoder)
       {
         delete _cimExportResponseEncoder;
       }
     // 9
     if (_cimExportRequestDispatcher)
       {
         delete _cimExportRequestDispatcher;
       }
     // 8
     // CIMOperationRequestDecoder depends on CIMOperationRequestAuthorizer
     // and CIMOperationResponseEncoder
     if (_cimOperationRequestDecoder)
       {
         delete _cimOperationRequestDecoder;
       }
     // 7
     if (_cimOperationResponseEncoder)
       {
         delete _cimOperationResponseEncoder;
       }
     // BinaryMessageHandler depends on CIMOperationRequestDispatcher
     /*if (_binaryMessageHandler)    //6
       {
         delete _binaryMessageHandler;
       } */
     // CIMOperationRequestAuthorizer depends on
     // CIMOperationRequestDispatcher
     if (_cimOperationRequestAuthorizer)
       {
         delete _cimOperationRequestAuthorizer;
       }
     // IndicationHandlerService , 3. It uses CIMOperationRequestDispatcher
     /*if (_handlerService)
       {
         delete _handlerService;
       } */
     // CIMOperationRequestDispatcher depends on
     // CIMRepository and ProviderRegistrationManager
     if (_cimOperationRequestDispatcher)
       {
         // Keeps an internal list of control providers. Must
         // delete this before ModuleController.
         delete _cimOperationRequestDispatcher;
       }

     // The SSL control providers used the SSL context manager.
     if (_sslContextMgr)
       {
         delete _sslContextMgr;
         _sslContextMgr = 0;
       }
     // ConfigManager. Really weird way of doing it.
     ConfigManager *configManager = ConfigManager::getInstance ();
     if (configManager)
       {
         // Refer to Bug #3537. It will soon have a fix.
         //delete configManager;
       }
     UserManager *userManager = UserManager::getInstance (_repository);
     if (userManager)
       {
         // Bug #3537 will soon fix this
         //delete userManager;
       }
     // Lastly the repository.
     if (_repository)
       {
         delete _repository;
       }

    PEG_METHOD_EXIT();
}

void CIMServer::addAcceptor(
    Uint16 connectionType,
    Uint32 portNumber,
    Boolean useSSL)
{
    HTTPAcceptor* acceptor;

    acceptor = new HTTPAcceptor(
          _monitor.get(),
          _httpAuthenticatorDelegator,
          connectionType,
          portNumber,
          useSSL ? _getSSLContext() : 0,
          useSSL ? _sslContextMgr->getSSLContextObjectLock() : 0 );

    ConfigManager* configManager = ConfigManager::getInstance();
    String socketWriteConfigTimeout =
        configManager->getCurrentValue("socketWriteTimeout");
    // Set timeout value for server socket timeouts
    // depending on config option
    Uint32 socketWriteTimeout =
        strtol(socketWriteConfigTimeout.getCString(), (char **)0, 10);
    // equal what went wrong, there has to be a timeout
    if (socketWriteTimeout == 0)
        socketWriteTimeout = PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS;
    acceptor->setSocketWriteTimeout(socketWriteTimeout);

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

void CIMServer::runForever()
{
    struct timeval now;

    // Note: Trace code in this method will be invoked frequently.
    if(!_dieNow)
      {
          _monitor->run(500000);

        static struct timeval lastIdleCleanupTime = {0, 0};
        Time::gettimeofday(&now);

        if (now.tv_sec - lastIdleCleanupTime.tv_sec > 300)
        {
            lastIdleCleanupTime.tv_sec = now.tv_sec;

            try
            {
                //_providerManager->unloadIdleProviders();
                MessageQueueService::get_thread_pool()->cleanupIdleThreads();
            }
            catch (...)
            {
            }
        }

        if (handleShutdownSignal)
        {
            shutdown();
            PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
                "CIMServer::runForever - signal received.  Shutting down.");
            //ShutdownService::getInstance(this)->shutdown(true, 10, false);
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
    _monitor->stopListeningForConnections(false);

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

SSLContext* CIMServer::_getSSLContext()
{
    PEG_METHOD_ENTER(TRC_SERVER, "CIMServer::_getSSLContext()");

    static const String PROPERTY_NAME__SSL_CERT_FILEPATH =
                                           "sslCertificateFilePath";
    static const String PROPERTY_NAME__SSL_KEY_FILEPATH  = "sslKeyFilePath";
    static const String PROPERTY_NAME__SSL_TRUST_STORE  = "sslTrustStore";
    static const String PROPERTY_NAME__SSL_CRL_STORE  = "crlStore";
    static const String PROPERTY_NAME__SSL_CLIENT_VERIFICATION =
                                           "sslClientVerificationMode";
    static const String PROPERTY_NAME__SSL_AUTO_TRUST_STORE_UPDATE =
                                           "enableSSLTrustStoreAutoUpdate";
    static const String PROPERTY_NAME__SSL_TRUST_STORE_USERNAME =
                                           "sslTrustStoreUserName";
    static const String PROPERTY_NAME__HTTP_ENABLED =
                                           "enableHttpConnection";

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
        // 'required' setting must have a valid truststore
        // 'optional' setting can be used with or without a truststore;
        // log a warning if a truststore is not specified
        //
        if (trustStore == String::EMPTY)
        {
            if (String::equal(verifyClient, "required"))
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
            else if (String::equal(verifyClient, "optional"))
            {
                Logger::put(Logger::STANDARD_LOG, System::CIMSERVER,
                    Logger::WARNING, "SSL client verification is "
                    "enabled but no truststore was specified.");
            }
        }
        //
        // ATTN: 'required' setting must have http port enabled.
        // If only https is enabled, and a call to shutdown the
        // cimserver is given, the call will hang and a forced shutdown
        // will ensue. This is because the CIMClient::connectLocal call
        // cannot (and should not) specify a truststore to validate the
        // local server against.  This limitation is being investigated.
        //
        if (String::equal(verifyClient, "required"))
        {
            if (!ConfigManager::parseBooleanValue(
                    configManager->getCurrentValue(
                        PROPERTY_NAME__HTTP_ENABLED)))
            {
                MessageLoaderParms parms(
                    "Pegasus.Server.SSLContextManager."
                      "SSL_CLIENT_VERIFICATION_HTTP_NOT_ENABLED_WITH_REQUIRED",
                    "The HTTP port must be enabled if "
                        "\"sslClientVerificationMode\" is 'required' in order "
                        "for the cimserver to properly shutdown.");
                PEG_METHOD_EXIT();
                throw SSLException(parms);
            }
        }

        //
        // A truststore username must be specified if
        // sslClientVerificationMode is enabled and the truststore is a
        // single CA file.  If the truststore is a directory, then the
        // CertificateProvider should be used to register users with
        // certificates.
        //
        if ((trustStore != String::EMPTY) &&
            (!FileSystem::isDirectory(trustStore)))
        {
            if (trustStoreUserName == String::EMPTY)
            {
                MessageLoaderParms parms(
                    "Pegasus.Server.SSLContextManager."
                        "SSL_CLIENT_VERIFICATION_EMPTY_USERNAME",
                    "The \"sslTrustStoreUserName\" property must specify a "
                        "valid username if \"sslClientVerificationMode\" is "
                        "'required' or 'optional' and the truststore is a "
                        "single CA file. To register individual certificates "
                        "to users, you must use a truststore directory along "
                        "with the CertificateProvider.");
                PEG_METHOD_EXIT();
                throw SSLException(parms);
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
    crlStore = String::EMPTY;
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
    // Create the SSLContext defined by the configuration properties
    //
    if (String::equal(verifyClient, "required"))
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
            "SSL Client verification REQUIRED.");

        _sslContextMgr->createSSLContext(
            trustStore, certPath, keyPath, crlStore, false, randFile);
    }
    else if (String::equal(verifyClient, "optional"))
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
            "SSL Client verification OPTIONAL.");

        _sslContextMgr->createSSLContext(
            trustStore, certPath, keyPath, crlStore, true, randFile);
    }
    else if (String::equal(verifyClient, "disabled") ||
             verifyClient == String::EMPTY)
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
            "SSL Client verification DISABLED.");

        _sslContextMgr->createSSLContext(
            String::EMPTY, certPath, keyPath, crlStore, false, randFile);
    }
    sslContext = _sslContextMgr->getSSLContext();

    PEG_METHOD_EXIT();
    return sslContext;
}

PEGASUS_NAMESPACE_END
