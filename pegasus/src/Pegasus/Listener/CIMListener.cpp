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

#include "CIMListener.h"
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Common/HTTPAcceptor.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/ExportServer/CIMExportResponseEncoder.h>
#include <Pegasus/ExportServer/CIMExportRequestDecoder.h>
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Listener/CIMListenerIndicationDispatcher.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMListenerService
//
////////////////////////////////////////////////////////////////////////////////

class CIMListenerService
{
public:
    CIMListenerService(Uint32 portNumber, SSLContext * sslContext = NULL);
    CIMListenerService(CIMListenerService & svc);
    ~CIMListenerService();

    void init();

    /** bind to the port
    */
    void bind();

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
    Boolean terminated() const
    {
        return _dieNow;
    };

    /** Call to resume the sever.
    */
    void resume();

    /** Call to set the CIMServer state.  Also inform the appropriate
        message queues about the current state of the CIMServer.
    */
    void setState(Uint32 state);

    Uint32 getOutstandingRequestCount();

    /** Returns the indication listener dispatcher
    */
    CIMListenerIndicationDispatcher *getIndicationDispatcher() const;

    /** Returns the indication listener dispatcher
    */
    void setIndicationDispatcher(CIMListenerIndicationDispatcher* dispatcher);

    /** Returns the port number being used.
    */
    Uint32 getPortNumber() const;

    static ThreadReturnType PEGASUS_THREAD_CDECL
    _listener_routine(void *param);

private:
    Uint32 _portNumber;
    SSLContext *_sslContext;
    ReadWriteSem _sslContextObjectLock;
    Monitor *_monitor;
    Mutex _monitorMutex;
    HTTPAcceptor *_ip6Acceptor;
    HTTPAcceptor *_ip4Acceptor;
    Boolean _dieNow;
    CIMListenerIndicationDispatcher *_dispatcher;
    CIMExportResponseEncoder *_responseEncoder;
    CIMExportRequestDecoder *_requestDecoder;
};

CIMListenerService::CIMListenerService(
    Uint32 portNumber,
    SSLContext * sslContext)
    :
    _portNumber(portNumber),
    _sslContext(sslContext),
    _monitor(NULL),
    _ip6Acceptor(NULL),
    _ip4Acceptor(NULL),
    _dieNow(false),
    _dispatcher(NULL),
    _responseEncoder(NULL),
    _requestDecoder(NULL)
{
}

CIMListenerService::CIMListenerService(CIMListenerService & svc) :
    _portNumber(svc._portNumber),
    _sslContext(svc._sslContext),
    _monitor(NULL),
    _ip6Acceptor(NULL),
    _ip4Acceptor(NULL),
    _dieNow(svc._dieNow),
    _dispatcher(NULL),
    _responseEncoder(NULL),
    _requestDecoder(NULL)
{
}

CIMListenerService::~CIMListenerService()
{
    delete _responseEncoder;
    delete _requestDecoder;
    delete _ip6Acceptor;
    delete _ip4Acceptor;
    delete _monitor;
}

void CIMListenerService::init()
{
    PEG_METHOD_ENTER(TRC_LISTENER, "CIMListenerService::init");

    if (NULL == _monitor)
        _monitor = new Monitor();

    // _dispatcher = new CIMListenerIndicationDispatcher();

    if (NULL == _responseEncoder)
        _responseEncoder = new CIMExportResponseEncoder();

    if (NULL == _requestDecoder)
    {
        _requestDecoder = new CIMExportRequestDecoder(
            _dispatcher, _responseEncoder->getQueueId());
    }
#ifdef PEGASUS_ENABLE_IPV6
    if (System::isIPv6StackActive())
    {
        if (NULL == _ip6Acceptor)
        {
            if (NULL == _sslContext)
            {
                _ip6Acceptor = new HTTPAcceptor(
                        _monitor, _requestDecoder,
                        HTTPAcceptor::IPV6_CONNECTION,
                        _portNumber, 0, 0);
            }
            else
            {
                _ip6Acceptor = new HTTPAcceptor(
                        _monitor, _requestDecoder,
                        HTTPAcceptor::IPV6_CONNECTION,
                        _portNumber, _sslContext, &_sslContextObjectLock);
            }
        }
    }
#ifndef PEGASUS_OS_TYPE_WINDOWS
    else
#endif
#endif
    if (NULL == _ip4Acceptor)
    {
        if (NULL == _sslContext)
        {
            _ip4Acceptor = new HTTPAcceptor(
                    _monitor, _requestDecoder,
                    HTTPAcceptor::IPV4_CONNECTION,
                    _portNumber, 0, 0);
        }
        else
        {
            _ip4Acceptor = new HTTPAcceptor(
                    _monitor, _requestDecoder,
                    HTTPAcceptor::IPV4_CONNECTION,
                    _portNumber, _sslContext, &_sslContextObjectLock);
        }
    }
    bind();

    PEG_METHOD_EXIT();
}

void CIMListenerService::bind()
{
    if (_ip6Acceptor != NULL)
    {
        _ip6Acceptor->bind();

        Logger::put(
            Logger::STANDARD_LOG,
            System::CIMLISTENER,
            Logger::INFORMATION,
            "IPV6, Listening on HTTP port $0.",
            _portNumber);
    }
    if (_ip4Acceptor != NULL)
    {
        _ip4Acceptor->bind();

        Logger::put(
            Logger::STANDARD_LOG,
            System::CIMLISTENER,
            Logger::INFORMATION,
            "IPV4, Listening on HTTP for port $0.",
            _portNumber);
    }
}

void CIMListenerService::runForever()
{
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
                MessageQueueService::get_thread_pool()->cleanupIdleThreads();
            }
            catch(...)
            {
                // Ignore!
            }
        }
    }
}

void CIMListenerService::shutdown()
{
    PEG_METHOD_ENTER(TRC_LISTENER, "CIMListenerService::shutdown()");

    // This logic signals the thread currently executing _listener_routine()
    // to exit. That function deletes this instance of CIMListenerService,
    // which deletes the _monitor member. We use a mutex to keep it from
    // deleting the monitor until after tickle has been called.
    {
        AutoMutex am(_monitorMutex);
        _dieNow = true;
        _monitor->tickle();
    }

    PEG_METHOD_EXIT();
}

void CIMListenerService::resume()
{
    PEG_METHOD_ENTER(TRC_LISTENER, "CIMListenerService::resume()");
    if (_ip6Acceptor != NULL)
    {
        _ip6Acceptor->reopenConnectionSocket();
    }
    if (_ip4Acceptor != NULL)
    {
        _ip4Acceptor->reopenConnectionSocket();
    }
    PEG_METHOD_EXIT();
}

void CIMListenerService::stopClientConnection()
{
    PEG_METHOD_ENTER(
        TRC_LISTENER,
        "CIMListenerService::stopClientConnection()");

    // tell Monitor to stop listening for client connections
    _monitor->stopListeningForConnections(true);
    if (_ip6Acceptor != NULL)
    {
        _ip6Acceptor->closeConnectionSocket();
    }
    if (_ip4Acceptor != NULL)
    {
        _ip4Acceptor->closeConnectionSocket();
    }
    PEG_METHOD_EXIT();
}

Uint32 CIMListenerService::getOutstandingRequestCount()
{
    Uint32 cnt = 0;

    if (_ip6Acceptor)
    {
        cnt = _ip6Acceptor->getOutstandingRequestCount();
    }

    if (_ip4Acceptor)
    {
        cnt += _ip4Acceptor->getOutstandingRequestCount();
    }

    return cnt;
}

CIMListenerIndicationDispatcher*
CIMListenerService::getIndicationDispatcher() const
{
    return _dispatcher;
}

void CIMListenerService::setIndicationDispatcher(
    CIMListenerIndicationDispatcher* dispatcher)
{
    _dispatcher = dispatcher;
}

Uint32 CIMListenerService::getPortNumber() const
{
    Uint32 portNumber = _portNumber;

    if ((portNumber == 0) && (_ip6Acceptor != 0))
    {
        portNumber = _ip6Acceptor->getPortNumber();
    }
    else if ((portNumber == 0) && (_ip4Acceptor != 0))
    {
        portNumber = _ip4Acceptor->getPortNumber();
    }

    return (portNumber);
}

ThreadReturnType PEGASUS_THREAD_CDECL
CIMListenerService::_listener_routine(void *param)
{
    CIMListenerService *svc = reinterpret_cast < CIMListenerService * >(param);

    try
    {
        // svc->init(); bug 1394
        while (!svc->terminated())
        {
#if defined(PEGASUS_OS_DARWIN)
            pthread_testcancel();
#endif
            svc->runForever();
        }
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL1,
                      "Unknown exception thrown in _listener_routine.");
    }

    // CAUTION: deleting the service also deletes the monitor whose tickle()
    // method may still be executing in another thread. This line of code was
    // most likely reached when the CIMListenerService::shutdown() method set
    // _dieNow to true and called Monitor::tickle(). We must wait until we
    // can obtain the _monitorMutex, indicating that we are no longer inside
    // Monitor::tickle().
    svc->_monitorMutex.lock();
    svc->_monitorMutex.unlock();
    delete svc;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// CIMListenerRep
//
////////////////////////////////////////////////////////////////////////////////

class CIMListenerRep
{
public:
    CIMListenerRep(Uint32 portNumber, SSLContext * sslContext = NULL);
    ~CIMListenerRep();

    Uint32 getPortNumber() const;

    SSLContext *getSSLContext() const;
    void setSSLContext(SSLContext * sslContext);

    void start();
    void stop();

    Boolean isAlive();

    Boolean addConsumer(CIMIndicationConsumer * consumer);
    Boolean removeConsumer(CIMIndicationConsumer * consumer);

private:
    Boolean waitForPendingRequests(Uint32 shutdownTimeout);

    Uint32 _portNumber;
    SSLContext *_sslContext;

    CIMListenerIndicationDispatcher *_dispatcher;
    ThreadPool *_thread_pool;
    CIMListenerService *_svc;
    Semaphore *_listener_sem;
};

CIMListenerRep::CIMListenerRep(
    Uint32 portNumber,
    SSLContext * sslContext)
    :
    _portNumber(portNumber),
    _sslContext(sslContext),
    _dispatcher(new CIMListenerIndicationDispatcher()),
    _thread_pool(NULL),
    _svc(NULL),
    _listener_sem(NULL)
{
}

CIMListenerRep::~CIMListenerRep()
{
    stop();

    delete _sslContext;
    delete _dispatcher;
    delete _thread_pool;
    delete _listener_sem;

    // don't delete _svc, this is deleted by _listener_routine
}

Uint32 CIMListenerRep::getPortNumber() const
{
    Uint32 portNumber;

    if (_svc == 0)
        portNumber = _portNumber;
    else
        portNumber = _svc->getPortNumber();

    return portNumber;
}

SSLContext *CIMListenerRep::getSSLContext() const
{
    return _sslContext;
}

void CIMListenerRep::setSSLContext(SSLContext * sslContext)
{
    delete _sslContext;
    _sslContext = sslContext;
}

void CIMListenerRep::start()
{
    // spawn a thread to do this
    if (_thread_pool == 0)
    {
        AutoPtr < CIMListenerService >
            svc(new CIMListenerService(_portNumber, _sslContext));

        svc->setIndicationDispatcher(_dispatcher);
        svc->init();

        struct timeval deallocateWait = { 15, 0 };
        AutoPtr < ThreadPool >
            threadPool(new ThreadPool(0, "Listener", 0, 1, deallocateWait));
        AutoPtr < Semaphore > sem(new Semaphore(0));

        if (threadPool->allocate_and_awaken(
            svc.get(), CIMListenerService::_listener_routine, sem.get())
            != PEGASUS_THREAD_OK)
        {
            PEG_TRACE_CSTRING(
                TRC_SERVER,
                Tracer::LEVEL1,
                "Could not allocate thread for "
                "CIMListenerService::_listener_routine.");
            throw
                Exception(MessageLoaderParms(
                    "Listener.CIMListener.CANNOT_ALLOCATE_THREAD",
                    "Could not allocate thread."));
        }

        Logger::put(
            Logger::STANDARD_LOG,
            System::CIMLISTENER,
            Logger::INFORMATION,
            "CIMListener started");

        _svc = svc.release();
        _thread_pool = threadPool.release();
        _listener_sem = sem.release();
    }
}

void CIMListenerRep::stop()
{
    if (_thread_pool != NULL)
    {
        //
        // Graceful shutdown of the listener service
        //

        // Block incoming export requests and unbind the port
        _svc->stopClientConnection();

        // Wait until pending export requests in the server are done.
        waitForPendingRequests(10);

        // Shutdown the CIMListenerService
        _svc->shutdown();

        // Wait for the _listener_routine thread to exit.
        // The thread could be delivering an export, so give it 3sec.
        // Note that _listener_routine deletes the CIMListenerService,
        // so no need to delete _svc.
        if (!_listener_sem->time_wait(3000))
        {
            // No need to do anything, the thread pool will be deleted below
            // to cancel the _listener_routine thread if it is still running.
        }

        delete _listener_sem;
        _listener_sem = NULL;

        // Delete the thread pool.  This cancels the listener thread if it is
        // still
        // running.
        delete _thread_pool;
        _thread_pool = NULL;

        Logger::put(
            Logger::STANDARD_LOG, System::CIMLISTENER,
            Logger::INFORMATION, "CIMListener stopped");
    }
}

Boolean CIMListenerRep::isAlive()
{
    return (_thread_pool != NULL) ? true : false;
}

Boolean CIMListenerRep::addConsumer(CIMIndicationConsumer * consumer)
{
    return _dispatcher->addConsumer(consumer);
}

Boolean CIMListenerRep::removeConsumer(CIMIndicationConsumer * consumer)
{
    return _dispatcher->removeConsumer(consumer);
}

Boolean CIMListenerRep::waitForPendingRequests(Uint32 shutdownTimeout)
{
    // Wait for 10 sec max
    Uint32 reqCount;
    Uint32 countDown = shutdownTimeout * 10;

    for (; countDown > 0; countDown--)
    {
        reqCount = _svc->getOutstandingRequestCount();
        if (reqCount > 0)
            Threads::sleep(100);
        else
            return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// CIMListener
//
/////////////////////////////////////////////////////////////////////////////

CIMListener::CIMListener(
    Uint32 portNumber,
    SSLContext * sslContext)
    :
    _rep(new CIMListenerRep(portNumber, sslContext))
{
}

CIMListener::~CIMListener()
{
    if (_rep != NULL)
        delete static_cast < CIMListenerRep * >(_rep);
    _rep = NULL;
}

Uint32 CIMListener::getPortNumber() const
{
    return static_cast < CIMListenerRep * >(_rep)->getPortNumber();
}

SSLContext *CIMListener::getSSLContext() const
{
    return static_cast < CIMListenerRep * >(_rep)->getSSLContext();
}

void CIMListener::setSSLContext(SSLContext * sslContext)
{
    static_cast < CIMListenerRep * >(_rep)->setSSLContext(sslContext);
}

void CIMListener::start()
{
    static_cast < CIMListenerRep * >(_rep)->start();
}

void CIMListener::stop()
{
    static_cast < CIMListenerRep * >(_rep)->stop();
}

Boolean CIMListener::isAlive() const
{
    return static_cast < CIMListenerRep * >(_rep)->isAlive();
}

Boolean CIMListener::addConsumer(CIMIndicationConsumer * consumer)
{
    return static_cast < CIMListenerRep * >(_rep)->addConsumer(consumer);
}

Boolean CIMListener::removeConsumer(CIMIndicationConsumer * consumer)
{
    return static_cast < CIMListenerRep * >(_rep)->removeConsumer(consumer);
}

PEGASUS_NAMESPACE_END
