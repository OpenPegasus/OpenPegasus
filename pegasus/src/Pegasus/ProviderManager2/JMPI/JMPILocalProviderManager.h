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

#ifndef Pegasus_JMPILocalProviderManager_h
#define Pegasus_JMPILocalProviderManager_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/HashTable.h>

#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_JMPIPM_LINKAGE JMPILocalProviderManager
{

public:
    JMPILocalProviderManager(void);
    virtual ~JMPILocalProviderManager(void);

public:
    JMPIProvider::OpProviderHolder getProvider(
        const String & fileName,
        const String & providerName,
        const String & interfaceName = String::EMPTY);

    void unloadProvider(const String & fileName, const String & providerName);

    void shutdownAllProviders(void);

    Boolean hasActiveProviders();
    void unloadIdleProviders();

    /**
         Gets list of indication providers to be enabled.
         Once IndicationService initialization has been completed, the
         enableIndications() method must be called on each indication provider
         that has current subscriptions.

         @return list of providers whose enableIndications() method must be
                 called
     */
    Array <JMPIProvider *> getIndicationProvidersToEnable ();

private:
    enum CTRL
    {
        INSERT_PROVIDER,
        INSERT_MODULE,
        REMOVE_PROVIDER,
        REMOVE_MODULE,
        LOOKUP_PROVIDER,
        LOOKUP_MODULE,
        GET_PROVIDER,
        UNLOAD_PROVIDER,
        UNLOAD_ALL_PROVIDERS,
        UNLOAD_IDLE_PROVIDERS,
        UNLOAD_IDLE_MODULES
    };

    typedef HashTable<String, JMPIProvider *,
        EqualFunc<String>,  HashFunc<String> > ResolverTable;

    typedef HashTable<String, JMPIProvider *,
        EqualFunc<String>,  HashFunc<String> > ProviderTable;

    typedef HashTable<String, JMPIProviderModule *,
        EqualFunc<String>, HashFunc<String> > ModuleTable;

    typedef struct
    {
        const String *providerName;
        const String *fileName;
        const String *interfaceName;
    } CTRL_STRINGS;

    ResolverTable _resolvers;
    ProviderTable _providers;
    ModuleTable _modules;
    Uint32 _idle_timeout;

    JMPIProvider *_getResolver(
        const String & fileName,
        const String & interfaceType);
    Sint32 _provider_ctrl(CTRL code, void *parm, void *ret);

    Mutex _providerTableMutex;

    static int trace;
protected:

};

PEGASUS_NAMESPACE_END

#endif

