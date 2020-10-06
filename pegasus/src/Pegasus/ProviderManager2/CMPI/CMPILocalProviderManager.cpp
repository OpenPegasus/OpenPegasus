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

#include "CMPI_Version.h"

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/ProviderManager2/CMPI/CMPIProvider.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIProviderModule.h>
#include <Pegasus/ProviderManager2/CMPI/CMPILocalProviderManager.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

/* Thread deletion specific */
Semaphore CMPILocalProviderManager::_pollingSem(0);
AtomicInt CMPILocalProviderManager::_stopPolling(0);
Thread *CMPILocalProviderManager::_reaperThread = 0;
List<CMPILocalProviderManager::cleanupThreadRecord,Mutex>
    CMPILocalProviderManager::_finishedThreadList;
Mutex CMPILocalProviderManager::_reaperMutex;

CMPILocalProviderManager::CMPILocalProviderManager ():
_idle_timeout (PEGASUS_PROVIDER_IDLE_TIMEOUT_SECONDS)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::CMPILocalProviderManager()");
    PEG_METHOD_EXIT();
}

CMPILocalProviderManager::~CMPILocalProviderManager ()
{
    Uint32 ccode;
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "MPILocalProviderManager::~CMPILocalProviderManager()");

    _provider_ctrl (UNLOAD_ALL_PROVIDERS, this, &ccode);
    // Since all of the providers are deleted we can delete the
    //  modules too.
    for (ModuleTable::Iterator j = _modules.start (); j != 0; j++)
    {
        CMPIProviderModule *module = j.value ();
        delete module;
    }

    if (_reaperThread)
    {
        AutoMutex lock(_reaperMutex);
        _stopPolling++;
        _pollingSem.signal();
        // Wait until it finishes itself.
        _reaperThread->join();
        delete _reaperThread;
        _reaperThread = 0;
    }
    PEGASUS_ASSERT(_finishedThreadList.size() == 0);
    PEG_METHOD_EXIT();
}

Sint32 CMPILocalProviderManager::_provider_ctrl (
    CTRL code,
    void *parm,
    void *ret)
{

    static Uint32 quantum;
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_provider_ctrl()");

    Sint32 ccode = 0;
    CTRL_STRINGS *ctrlParms = reinterpret_cast < CTRL_STRINGS * >(parm);

    switch (code)
    {
        case GET_PROVIDER:
            {
                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL3,
                    "CMPILocalProviderManager::_provider_ctrl:GET_PROVIDER()");

                String providerName = *(ctrlParms->providerName);
                String providerModuleName = *(ctrlParms->providerModuleName);
                String moduleFileName = *(ctrlParms->fileName);
                String location = *(ctrlParms->location);

                CMPIProvider *pr = 0;
                OpProviderHolder * ph =
                    reinterpret_cast < OpProviderHolder * >(ret);

                pr = _lookupProvider (providerName, providerModuleName);

                if (pr->getStatus () != CMPIProvider::INITIALIZED)
                {
                    pr->setLocation (location);
                    _initProvider (pr, moduleFileName);

                    if (pr->getStatus () != CMPIProvider::INITIALIZED)
                    {
                        MessageLoaderParms parms(
                            "ProviderManager.CMPI.CMPILocalProviderManager."
                            "CANNOT_INIT_PROVIDER",
                            "Failed to initialize the provider $0.",
                            pr->getName());
                        PEG_METHOD_EXIT ();
                        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, parms);
                    }
                }


                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                    "Returning Provider %s",
                    (const char*)providerName.getCString()));


                ph->SetProvider (pr);
                break;
            }

        case UNLOAD_PROVIDER:
            {

                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL3,
                    "CMPILocalProviderManager::_provider_ctrl:\
                    UNLOAD_PROVIDER");
                CMPIProvider *pr = 0;
                pr = _lookupProvider (
                    *ctrlParms->providerName,
                    *ctrlParms->providerModuleName);

                // The provider table must be locked before unloading.
                AutoMutex lock (_providerTableMutex);
                if ((pr->getStatus () == CMPIProvider::INITIALIZED))
                {
                    PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                        "Unloading CMPIProvider: %s",
                        (const char*)pr->getName().getCString()));
                    _unloadProvider(pr);
                }
                if (pr->getStatus () == CMPIProvider::UNINITIALIZED)
                {
                    _removeProvider(pr->getNameWithType(), pr->getModuleName());
                    delete pr;
                }
                else
                {
                    // Provider could not be unloaded due to pending operations.
                    ccode = -1;
                }
                break;
            }

        case UNLOAD_ALL_PROVIDERS:
            {
                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL3,
                    "CMPILocalProviderManager::_provider_ctrl: \
                    UNLOAD_ALL_PROVIDERS");
                CMPILocalProviderManager *myself =
                    reinterpret_cast < CMPILocalProviderManager * >(parm);
                CMPIProvider *provider = 0;
                // Locked provider mutex.
                AutoMutex lock (_providerTableMutex);
                Array<CMPIProvider*> unloadPendingProviders;

                PEG_TRACE((
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL3,
                    "providers in cache = %d", myself->_providers.size ()));
                ProviderTable::Iterator i = myself->_providers.start ();
                try
                {
                    for (; i != 0; i++)
                    {

                        provider = i.value ();
                        PEGASUS_ASSERT (provider != 0);
                        if (provider->getStatus () ==
                            CMPIProvider::UNINITIALIZED)
                        {
                            // Delete the skeleton.
                            delete provider;
                            continue;
                        }
                        else
                        {
                            // Force unload.
                            _unloadProvider (provider, true);
                            if (provider->getStatus () ==
                                CMPIProvider::UNINITIALIZED)
                            {
                                delete provider;
                            }
                        }
                    }

                    // All the providers are removed. Clear the hash-table
                    _providers.clear ();
                }
                catch (...)
                {
                    PEG_TRACE_CSTRING(
                        TRC_PROVIDERMANAGER,
                        Tracer::LEVEL2,
                        "Unexpected Exception in UNLOAD_ALL_PROVIDERS.");
                }
                break;
            }
        case UNLOAD_IDLE_PROVIDERS:
            {
                PEG_TRACE_CSTRING(
                    TRC_PROVIDERMANAGER,
                    Tracer::LEVEL3,
                    "CMPILocalProviderManager::_provider_ctrl: \
                    UNLOAD_IDLE_PROVIDERS");
                AutoMutex lock (_providerTableMutex);

                quantum++;
                CMPILocalProviderManager *myself =
                    reinterpret_cast < CMPILocalProviderManager * >(parm);
                Uint32 numProviders = myself->_providers.size ();

                if (numProviders)
                {
                    // Rather than removing the provider immediately while
                    // iterating through the list we remember all candidates
                    // for unload in a simple array.
                    CMPIProvider **unloadProviderArray =
                        new CMPIProvider *[numProviders];
                    Uint32 upaIndex = 0;

                    try
                    {
                        struct timeval now;
                        Time::gettimeofday (&now);
                        ProviderTable::Iterator i = myself->_providers.start();

                        for (; i != 0; i++)
                        {
                            CMPIProvider* provider = i.value();
                            PEGASUS_ASSERT (provider != 0);

                            if (provider->getStatus () ==
                                CMPIProvider::UNINITIALIZED)
                            {
                                continue;
                            }

                            if (provider->getQuantum() == quantum)
                            {
                                continue;
                            }

                            provider->setQuantum(quantum);

                            if (provider->getCurrentOperations())
                            {
                                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                                    "CMPIProvider has pending operations: %s",
                                    (const char*)
                                         provider->getName().getCString()));

                                continue;
                            }

                            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                                "Checking timeout data for CMPIProvider: %s",
                                (const char*)provider->getName().getCString()));
                            struct timeval timeout = { 0, 0};
                            provider->get_idle_timer (&timeout);

                            PEG_TRACE_CSTRING(
                                TRC_PROVIDERMANAGER,
                                Tracer::LEVEL4,
                                provider->unload_ok ()?
                                " provider->unload_ok() returns: true" :
                                " provider->unload_ok() returns: false");

                            if (provider->unload_ok () == true &&
                                (now.tv_sec - timeout.tv_sec) >
                                ((Sint32) myself->_idle_timeout))
                            {
                                // Remember this provider to be unloaded
                                unloadProviderArray[upaIndex] = provider;
                                upaIndex++;
                            }
                            //else cout<<"--- NOT unloaded: "+
                            // provider->getName()<<endl;
                        }

                        // Now finally we unload all providers that we
                        // identified as candidates above.
                        for (Uint32 index = 0; index < upaIndex; index++)
                        {
                            CMPIProvider *provider =
                                unloadProviderArray[index];
                            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                                "Now trying to unload CMPIProvider %s",
                                (const char*)provider->getName().getCString()));
                            {
                                // lock the provider mutex

                                AutoMutex pr_lock (provider->getStatusMutex());

                                if (provider->tryTerminate () == false)
                                {
                                    // provider not unloaded -- we are
                                    // respecting this!
                                    PEG_TRACE((TRC_PROVIDERMANAGER,
                                        Tracer::LEVEL4,
                                        "Provider refused to unload: %s",
                                        (const char*)
                                            unloadProviderArray[index]
                                                ->getName().getCString()));
                                    continue;
                                }

                                PEGASUS_ASSERT (provider->getModule() != 0);

                                // unload provider module
                                provider->getModule()->unloadModule ();
                                PEG_TRACE((
                                    TRC_PROVIDERMANAGER,
                                    Tracer::LEVEL3,
                                    "CMPILocalProviderManager::_provider_crtl"
                                        " -  Unload provider %s",
                                    (const char*)
                                        provider->getName().getCString()));

                                // Note: The deleting of the cimom handle is
                                // being moved after the call to
                                // unloadModule() based on a previous fix for
                                // bug 3669 and consistency with other
                                // provider managers. Do not move it back
                                // before the call to unloadModule().
                                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                                    "Destroying CMPIProvider's CIMOM Handle %s",
                                    (const char*)
                                        provider->getName().getCString()));

                                delete provider->getCIMOMHandle();

                                // set provider status to UNINITIALIZED
                                provider->reset ();
                            }
                        }
                    }
                    catch (...)
                    {
                        PEG_TRACE_CSTRING(
                            TRC_PROVIDERMANAGER,
                            Tracer::LEVEL1,
                            "Unexpected Exception in UNLOAD_IDLE_PROVIDERS.");
                    }
                    delete [] unloadProviderArray;
                }                     // if there are any providers
                break;
            }

        default:
            ccode = -1;
            break;
    }
    PEG_METHOD_EXIT ();
    return(ccode);
}



/*
 * The reaper function polls out the threads from the global list
 * (_finishedThreadList), joins them deletes them, and removes them from the
 * CMPIProvider specific list.
*/
ThreadReturnType PEGASUS_THREAD_CDECL CMPILocalProviderManager::_reaper(
    void *parm)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_reaper()");
    do
    {
        _pollingSem.wait();
        // All of the threads are finished working. We just need to reap 'em
        cleanupThreadRecord *rec = 0;

        while (_finishedThreadList.size() >0)
        {
            // Pull of the the threads from the global list.
            rec = _finishedThreadList.remove_front();
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                "-- Reaping the thread from %s",
                (const char*)rec->provider->getName().getCString()));

            rec->thread->join();

            // Remove the thread for the CMPIProvider.
            rec->provider->threadDelete(rec->thread);

            // Delete the thread.
            delete rec->thread;
            delete rec;
        }
    }
    while (_stopPolling.get() == 0);

    PEG_METHOD_EXIT();
    return ThreadReturnType(0);
}

Boolean CMPILocalProviderManager::isProviderActive(
    const String &providerName,
    const String &providerModuleName)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::isProviderActive()");

    AutoMutex mtx(_providerTableMutex);
    String lProviderName("L");
    lProviderName.append(providerName);
    String rProviderName("R");
    rProviderName.append(providerName);

    ProviderKey lpKey(lProviderName, providerModuleName);
    ProviderKey rpKey(rProviderName, providerModuleName);
    Boolean active = _providers.contains(lpKey) || _providers.contains(rpKey);

    PEG_METHOD_EXIT();

    return active;
}

/*
 // Cleanup the thread and upon deletion of it, call the CMPIProvider'
 // "threadDeleted". to not, all the CMPIProvider '
 // Note that this function is called from the thread that finished with
 // running the providers function, and returns immediately while scheduling
 // the a cleanup procedure. If you want to wait until the thread is truly
 // deleted, call 'waitUntilThreadsDone' - but DO NOT do it in the the thread
 // that the Thread owns - you will wait forever.
 //
 // @argument t Thread that is not NULL and finished with running the provider
 // function.
 // @argument p CMPIProvider in which the 't' Thread was running.
 */
void CMPILocalProviderManager::cleanupThread(Thread *t, CMPIProvider *p)
{
    PEG_METHOD_ENTER(
         TRC_PROVIDERMANAGER,
         "CMPILocalProviderManager::cleanupThread()");

    PEGASUS_ASSERT( t != 0 && p != 0 );

    PEGASUS_ASSERT ( p->isThreadOwner(t) );
    // The mutex guards against a race condition for _reaperThread creation.
    AutoMutex lock(_reaperMutex);

    // Put the Thread and the CMPIProvider on the global list.
    cleanupThreadRecord *record = new cleanupThreadRecord(t, p);
    _finishedThreadList.insert_back(record);

    if (_reaperThread == 0)
    {
        _reaperThread = new Thread(_reaper, NULL, false);

        if (_reaperThread->run() != PEGASUS_THREAD_OK)
        {
            PEG_TRACE_CSTRING(
                TRC_PROVIDERMANAGER,
                Tracer::LEVEL1,
                "Could not allocate thread to take care of deleting "
                    "user threads, will be cleaned up later.");

            delete _reaperThread; 
            _reaperThread = 0;
            PEG_METHOD_EXIT();
            return;
        }
    }
    // Wake up the reaper.
    _pollingSem.signal();
    PEG_METHOD_EXIT();

}

OpProviderHolder CMPILocalProviderManager::getRemoteProvider (
    const String & location,
    const String & providerName,
    const String & providerModuleName)
{
    OpProviderHolder ph;
    CTRL_STRINGS
        strings;
    Sint32
        ccode;
    const String
        proxy ("CMPIRProxyProvider");

    String
        rproviderName ("R");
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "ProvidertManager::getRemoteProvider()");

    rproviderName.append (providerName);

    strings.fileName = &proxy;
    strings.providerName = &rproviderName;
    strings.providerModuleName = &providerModuleName;
    strings.location = &location;

    try
    {
        ccode = _provider_ctrl (GET_PROVIDER, &strings, &ph);
    }
    catch (const Exception & e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
            "--- Exception loading proxy: %s",
            (const char*)e.getMessage().getCString()));

        PEG_METHOD_EXIT ();
        throw;
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL1,
            "--- Unexpected exception in loading proxy provider: ---");
        PEG_METHOD_EXIT ();
        throw;
    }

    PEG_METHOD_EXIT ();
    return ph;
}

OpProviderHolder CMPILocalProviderManager::getProvider (
    const String & fileName,
    const String & providerName,
    const String & providerModuleName)
{
    OpProviderHolder ph;
    CTRL_STRINGS strings;
    Sint32 ccode;

    String lproviderName ("L");

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "ProviderManager::getProvider()");

    if (fileName.size() == 0)
    {
        throw Exception(MessageLoaderParms(
            "ProviderManager.CMPI.CMPILocalProviderManager.CANNOT_FIND_LIBRARY",
            "For provider $0 the library name was empty. Check provider "
            "registered location.",
            providerName));

    }
    lproviderName.append (providerName);
    strings.fileName = &fileName;
    strings.providerName = &lproviderName;
    strings.providerModuleName = &providerModuleName;
    strings.location = &String::EMPTY;

    try
    {
        ccode = _provider_ctrl (GET_PROVIDER, &strings, &ph);
    }
    catch (const Exception & e)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
            "--- Exception loading local provider: %s",
            (const char*)e.getMessage().getCString()));
        PEG_METHOD_EXIT ();
        throw;
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL1,
            "--- Unexpected exception in loading local provider ---");
        PEG_METHOD_EXIT ();
        throw;
    }


    PEG_METHOD_EXIT ();
    return(ph);

}

Boolean CMPILocalProviderManager::unloadProvider(
    const String & fileName,
    const String & providerName,
    const String & providerModuleName)
{
    CTRL_STRINGS strings;
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "ProviderManager::unloadProvider()");

    String lproviderName ("L");
    String rproviderName ("R");
    lproviderName.append (providerName);
    rproviderName.append (providerName);

    strings.fileName = &fileName;
    strings.providerName = &lproviderName;
    strings.providerModuleName = &providerModuleName;
    strings.location = &String::EMPTY;

    int lproviderStatus = 0;
    int rproviderStatus = 0;
    lproviderStatus = _provider_ctrl (UNLOAD_PROVIDER, &strings, (void *) 0);

    strings.providerName = &rproviderName;

    rproviderStatus = _provider_ctrl (UNLOAD_PROVIDER, &strings, (void *) 0);

    PEG_METHOD_EXIT ();
    return lproviderStatus != -1 && rproviderStatus != -1;
}

void CMPILocalProviderManager::shutdownAllProviders ()
{
    _provider_ctrl (UNLOAD_ALL_PROVIDERS, (void *) this, (void *) 0);
}


Boolean CMPILocalProviderManager::hasActiveProviders ()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "ProviderManager::hasActiveProviders()");

    try
    {
        AutoMutex lock (_providerTableMutex);
        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "providers in _providers table = %d", _providers.size ()));

        // Iterate through the _providers table looking for an active provider
        for (ProviderTable::Iterator i = _providers.start (); i != 0; i++)
        {
            if (i.value ()->getStatus () == CMPIProvider::INITIALIZED)
            {
                PEG_METHOD_EXIT ();
                return true;
            }
        }
    }
    catch (...)
    {
        // Unexpected exception; do not assume that no providers are loaded
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL1,
            "Unexpected Exception in hasActiveProviders.");
        PEG_METHOD_EXIT ();
        return true;
    }

    // No active providers were found in the _providers table
    PEG_METHOD_EXIT ();
    return false;
}

void CMPILocalProviderManager::unloadIdleProviders ()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "ProviderManager::unloadIdleProviders()");

    try
    {
        _provider_ctrl (UNLOAD_IDLE_PROVIDERS, this, (void *) 0);
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL2,
            "Caught unexpected exception from UNLOAD_IDLE_PROVIDERS.");
    }

    PEG_METHOD_EXIT ();
}

Array <CMPIProvider *>CMPILocalProviderManager::
    getIndicationProvidersToEnable()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::getIndicationProvidersToEnable()");

    Array < CMPIProvider * >enableProviders;

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "Number of providers in _providers table = %d",
        _providers.size ()));

    try
    {
        AutoMutex
            lock (_providerTableMutex);

        //
        // Iterate through the _providers table
        //
        for (ProviderTable::Iterator i = _providers.start (); i != 0; i++)
        {
            //
            //  Enable any indication provider with current subscriptions
            //
            CMPIProvider *
                provider = i.value ();
            if (provider->testSubscriptions ())
            {
                enableProviders.append (provider);
            }
        }

    }
    catch (const CIMException & e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL1,
            "CIMException in getIndicationProvidersToEnable: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (const Exception & e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL1,
            "Exception in getIndicationProvidersToEnable: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(
            TRC_DISCARDED_DATA,
            Tracer::LEVEL1,
            "Unexpected error in getIndicationProvidersToEnable");
    }

    PEG_TRACE((
        TRC_PROVIDERMANAGER,
        Tracer::LEVEL4,
        "Number of indication providers to enable = %d",
        enableProviders.size ()));

    PEG_METHOD_EXIT ();
    return enableProviders;
}

CMPIProvider *CMPILocalProviderManager::_initProvider(
    CMPIProvider * provider,
    const String & moduleFileName)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_initProvider()");

    CMPIProviderModule *module = 0;
    ProviderVector base;

    {
        // lock the providerTable mutex
        AutoMutex lock (_providerTableMutex);

        // lookup provider module
        module = _lookupModule (moduleFileName);
    }                             // unlock the providerTable mutex

    Boolean moduleLoaded = false;
    Boolean deleteProvider = false;
    String exceptionMsg = moduleFileName;
    {
        // lock the provider status mutex
        AutoMutex lock (provider->getStatusMutex());

        if (provider->getStatus() == CMPIProvider::INITIALIZED)
        {
            PEG_METHOD_EXIT();
            // Initialization is already complete
            return provider;
        }

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "Loading/Linking Provider Module %s",
            (const char*)moduleFileName.getCString()));

        // load the provider
        try
        {
            base = module->load (provider->getNameWithType());
            moduleLoaded = true;
        }
        catch (const Exception &e)
        {
            exceptionMsg = e.getMessage();
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                "Exception caught Loading/Linking Provider Module %s"
                " error is: %s",
                (const char*)moduleFileName.getCString(),
                (const char*)exceptionMsg.getCString()));
            deleteProvider =true;
        }
        catch (...)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                "Unknown exception caught Loading/Linking Provider Module %s",
                (const char*)moduleFileName.getCString()));
            exceptionMsg = moduleFileName;
            deleteProvider = true;
        }

        if (!deleteProvider)
        {
            // initialize the provider
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                "Initializing Provider %s",
                (const char*)provider->getName().getCString()));

            CIMOMHandle *cimomHandle = new CIMOMHandle ();
            provider->set (module, base, cimomHandle);
            provider->setQuantum(0);

            try
            {
                provider->initialize (*(provider->getCIMOMHandle()));
            }
            catch (const Exception &e)
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                    "Problem initializing %s: %s",
                    (const char*)provider->getName().getCString(),
                    (const char*)e.getMessage().getCString()));
                deleteProvider = true;
                exceptionMsg = e.getMessage();
            }
            catch (...)
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                    "Unknown problem initializing %s",
                    (const char*)provider->getName().getCString()));
                deleteProvider = true;
                exceptionMsg = provider->getName();
            }
        }                             // unlock the provider status mutex
    }
    /* We wait until this moment to delete the provider b/c we need
       be outside the scope for the AutoMutex for the provider. */
    if (deleteProvider)
    {
        // Note: The deleting of the cimom handle is being
        // moved after the call to unloadModule() based on
        // a previous fix for bug 3669 and consistency with
        // other provider managers. Do not move it back before
        // the call to unloadModule().

        // unload provider module
        if (moduleLoaded)
        {
            module->unloadModule();
        }

        // delete the cimom handle
        delete provider->getCIMOMHandle();
        // set provider status to UNINITIALIZED
        provider->reset ();

        AutoMutex lock (_providerTableMutex);
        _removeProvider(provider->getNameWithType(), provider->getModuleName());
        delete provider;

        PEG_METHOD_EXIT ();
        throw Exception(exceptionMsg);
    }

    PEG_METHOD_EXIT ();
    return(provider);
}

void CMPILocalProviderManager::_unloadProvider (
    CMPIProvider * provider,
    Boolean forceUnload)
{
    //
    // NOTE:  It is the caller's responsibility to make sure that
    // the ProviderTable mutex is locked before calling this method.
    //
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_unloadProvider()");

    PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
        "Unloading Provider %s",(const char*)provider->getName().getCString()));

    if (provider->getCurrentOperations() && !forceUnload)
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "Provider cannot be unloaded due to pending operations: %s",
            (const char*)provider->getName().getCString()));
    }
    else
    {
        if (provider->getCurrentOperations())
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                "Terminating Provider with pending operations %s",
                (const char*)provider->getName().getCString()));
        }
        else
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                "Terminating Provider %s",
                (const char*)provider->getName().getCString()));
         }

        // lock the provider mutex
        AutoMutex pr_lock (provider->getStatusMutex());

        try
        {
            provider->terminate ();
        }
        catch (...)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                "Error occured terminating CMPI provider %s",
                (const char*)provider->getName().getCString()));
        }

        if (provider->getStatus() == CMPIProvider::UNINITIALIZED)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                "Unload provider module %s for provider %s",
                (const char*)
                     provider->getModule()->getFileName().getCString(),
                (const char*)provider->getName().getCString()));
            // unload provider module
            provider->getModule()->unloadModule();

            // delete the cimom handle
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                "Destroying CMPIProvider's CIMOM Handle %s",
                (const char*)provider->getName().getCString()));

            delete provider->getCIMOMHandle();
            PEGASUS_ASSERT (provider->getModule() != 0);


            // set provider status to UNINITIALIZED
            provider->reset ();

            // Do not delete the provider. The function calling this function
            // takes care of that.
        }
    }

    PEG_METHOD_EXIT ();
}

Boolean CMPILocalProviderManager::_removeProvider(
    const String & providerName,
    const String & providerModuleName)
{
    ProviderKey providerKey(providerName, providerModuleName);

    AutoMutex lock (_providerTableMutex);
    return _providers.remove(providerKey);
}

CMPIProvider * CMPILocalProviderManager::_lookupProvider(
    const String & providerName,
    const String & providerModuleName)

{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_lookupProvider()");

    ProviderKey providerKey(providerName, providerModuleName);

    // lock the providerTable mutex
    AutoMutex lock (_providerTableMutex);
    // look up provider in cache
    CMPIProvider *pr = 0;

    if (true == _providers.lookup (providerKey, pr))
    {
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "Found Provider %s in CMPI Provider Manager Cache",
            (const char*)providerName.getCString()));
    }
    else
    {
        // create provider
        pr = new CMPIProvider (providerName, providerModuleName, 0, 0);
        // insert provider in provider table
        _providers.insert (providerKey, pr);

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "Created provider %s",(const char*)pr->getName().getCString()));
    }
    pr->update_idle_timer();

    PEG_METHOD_EXIT ();
    return(pr);
}


CMPIProviderModule * CMPILocalProviderManager::_lookupModule(
    const String & moduleFileName)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "CMPILocalProviderManager::_lookupModule()");

    // look up provider module in cache
    CMPIProviderModule *module = 0;

    if (true == _modules.lookup (moduleFileName, module))
    {
        // found provider module in cache
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "Found Provider Module %s in Provider Manager Cache",
            (const char*)moduleFileName.getCString()));

    }
    else
    {
        // provider module not found in cache, create provider module
        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "Creating CMPI Provider Module %s",
            (const char*)moduleFileName.getCString()));

        module = new CMPIProviderModule (moduleFileName);

        // insert provider module in module table
        _modules.insert (moduleFileName, module);
    }

    PEG_METHOD_EXIT ();
    return(module);
}

PEGASUS_NAMESPACE_END

