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

#ifndef Pegasus_Dynamic_Listener_h
#define Pegasus_Dynamic_Listener_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ReadWriteSem.h>
#include <Pegasus/DynListener/Linkage.h>

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

class SSLContext;

class PEGASUS_DYNLISTENER_LINKAGE DynamicListener
{
public:

    static const Boolean DEFAULT_CONSUMER_UNLOAD;
    static const Uint32 DEFAULT_IDLE_TIMEOUT;
    static const Boolean DEFAULT_FORCE_SHUTDOWN;
    static const Uint32 DEFAULT_SHUTDOWN_TIMEOUT;

    DynamicListener(Uint32 portNumber,
                    const String& consumerDir,
                    const String& consumerConfigDir,
                    Boolean enableConsumerUnload = DEFAULT_CONSUMER_UNLOAD,
                    Uint32 consumerIdleTimeout = DEFAULT_IDLE_TIMEOUT,
                    Uint32 shutdownTimeout = DEFAULT_SHUTDOWN_TIMEOUT);

#ifdef PEGASUS_HAS_SSL
    DynamicListener(Uint32 portNumber,
                    const String& consumerDir,
                    const String& consumerConfigDir,
                    Boolean useSSL,
                    const String& keyPath,
                    const String& certPath,
                    Boolean enableConsumerUnload = DEFAULT_CONSUMER_UNLOAD,
                    Uint32 consumerIdleTimeout = DEFAULT_IDLE_TIMEOUT,
                    Uint32 shutdownTimeout = DEFAULT_SHUTDOWN_TIMEOUT,
                    const String & sslCipherSuite="DEFAULT",
                    const Boolean& sslCompatibility = false);

    DynamicListener(Uint32 portNumber,
                    const String& consumerDir,
                    const String& consumerConfigDir,
                    Boolean useSSL,
                    SSLContext* sslContext,
                    ReadWriteSem*  _sslContextObjectLock,
                    Boolean enableConsumerUnload = DEFAULT_CONSUMER_UNLOAD,
                    Uint32 consumerIdleTimeout = DEFAULT_IDLE_TIMEOUT,
                    Uint32 shutdownTimeout = DEFAULT_SHUTDOWN_TIMEOUT);
#endif

    ~DynamicListener();

    void start();

    void stop(Boolean forceShutdown= DEFAULT_FORCE_SHUTDOWN);

    Boolean isAlive();

    Boolean addConsumer(
                const String& consumerName,
                const String& location = String::EMPTY);

    Boolean removeConsumer(const String& consumerName);

    Uint32 getPortNumber();

    String getConsumerDir();

    String getConsumerConfigDir();

    void setEnableConsumerUnload(const Boolean enableConsumerUnload);

    Boolean getEnableConsumerUnload();

    void setIdleTimeout(Uint32 idleTimeout);

    Uint32 getIdleTimeout();

private:
    void* _rep;

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Dynamic_Listener_h */
