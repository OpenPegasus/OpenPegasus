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

#ifndef Pegasus_ListenerService_h
#define Pegasus_ListenerService_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Monitor.h>
#include <Pegasus/Common/HTTPAcceptor.h>
#include <Pegasus/DynListener/Linkage.h>

#include <Pegasus/ExportServer/CIMExportResponseEncoder.h>
#include <Pegasus/ExportServer/CIMExportRequestDecoder.h>

#include "DynamicListenerIndicationDispatcher.h"
#include "ConsumerManager.h"

PEGASUS_NAMESPACE_BEGIN


class PEGASUS_DYNLISTENER_LINKAGE ListenerService
{
public:

    ListenerService(ConsumerManager* consumerManager);

    ~ListenerService();

    ListenerService(const ListenerService& x);

    Boolean initializeListener(
        Uint32 portNumber,
        Boolean useSSL,
        SSLContext* sslContext,
        ReadWriteSem*  sslContextObjectLock);

    Boolean runListener();

    Boolean shutdownListener();

    Boolean isAlive() const throw();

    Uint32 getPortNumber() const;

    static ThreadReturnType PEGASUS_THREAD_CDECL _listener_routine(void *param);

    static ThreadReturnType PEGASUS_THREAD_CDECL _polling_routine(void *param);


private:

    ConsumerManager* _consumerManager;
    DynamicListenerIndicationDispatcher* _dispatcher;

    Uint32 _portNumber;
    Boolean _useSSL;
    SSLContext* _sslContext;
    ReadWriteSem*  _sslContextObjectLock;

    //ATTN: do we need to mutex the status?  The consumer mgr takes
    // care of synchronization ... but,
    //maybe we should just in case someone uses it differently
    Boolean _initialized;
    Boolean _running;
    Boolean _dieNow; //AtomicInt???
    Semaphore* _shutdownSem;

    Monitor*        _monitor;

    HTTPAcceptor *_ip6Acceptor;
    HTTPAcceptor *_ip4Acceptor;

    CIMExportResponseEncoder* _responseEncoder;
    CIMExportRequestDecoder*  _requestDecoder;

    //listener thread
    Thread* _listening_thread;

    //polling thread
    Thread* _polling_thread;

};

PEGASUS_NAMESPACE_END

#endif //Pegasus_ListenerService_h





