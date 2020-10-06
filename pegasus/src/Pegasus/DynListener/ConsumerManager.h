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

#ifndef Pegasus_ConsumerManager_h
#define Pegasus_ConsumerManager_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Common/System.h>

#include <Pegasus/General/OptionManager.h>

#include <Pegasus/DynListener/Linkage.h>

#include "ConsumerModule.h"
#include "DynamicConsumer.h"

PEGASUS_NAMESPACE_BEGIN

/** The ConsumerManager class is responsible for managing all of the consumers.
 *  It is also responsible for synchronizing consumer operations.
 */

class PEGASUS_DYNLISTENER_LINKAGE ConsumerManager
{
public:

    ConsumerManager(
        const String& consumerDir,
        const String& consumerConfigDir,
        Boolean enableConsumerUnload,
        Uint32 idleTimeout);

    virtual ~ConsumerManager();

    String getConsumerDir();

    String getConsumerConfigDir();

    Boolean getEnableConsumerUnload();

    Uint32 getIdleTimeout();

    Boolean hasLoadedConsumers();

    Boolean hasActiveConsumers();

    DynamicConsumer* getConsumer(const String& consumerName);

    void unloadConsumer(const String& consumerName);

    void unloadAllConsumers();

    void unloadIdleConsumers();

private:

    typedef HashTable<
                String,
                DynamicConsumer *,
                EqualFunc<String>,
                HashFunc<String> > ConsumerTable;

    typedef HashTable<
                String,
                ConsumerModule *,
                EqualFunc<String>,
                HashFunc<String> > ModuleTable;

    //consumer queue
    ConsumerTable _consumers;
    Mutex _consumerTableMutex;

    //consumer module queue
    ModuleTable _modules;
    Mutex _moduleTableMutex;

    //config properties
    String _consumerDir;
    String _consumerConfigDir;
    Boolean _enableConsumerUnload;
    Uint32 _idleTimeout; //ms
    Boolean _forceShutdown;

    //ATTN: Bugzilla 3765 - Uncomment when OptionManager has a reset capability
    //OptionManager _optionMgr;

    //global thread pool
    ThreadPool* _thread_pool;

    //worker thread
    static ThreadReturnType PEGASUS_THREAD_CDECL _worker_routine(void *param);


    //methods

    ConsumerModule* _lookupModule(const String & moduleFileName);

    String _getConsumerLibraryName(const String & consumerName);

    void _initConsumer(const String& consumerName, DynamicConsumer* consumer);

    void _unloadConsumers(Array<DynamicConsumer*> consumersToUnload);

    void _init();

    Array<IndicationDispatchEvent> _deserializeOutstandingIndications(
                                        const String& consumerName);

    void _serializeOutstandingIndications(
             const String& consumerName,
             Array<IndicationDispatchEvent> indications);

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ConsumerManager_h */



