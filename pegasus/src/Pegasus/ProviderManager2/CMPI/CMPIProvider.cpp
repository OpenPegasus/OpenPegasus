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

#include "CMPIProvider.h"

#include "CMPI_Object.h"
#include "CMPI_Broker.h"
#include "CMPI_ContextArgs.h"
#include "CMPI_Ftabs.h"

#include <Pegasus/Common/Time.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIProvider.h>
#include <Pegasus/ProviderManager2/CMPI/CMPIProviderModule.h>
#include <Pegasus/ProviderManager2/CMPI/CMPILocalProviderManager.h>
#include <Pegasus/ProviderManager2/CMPI/CMPI_ThreadContext.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

static const char _MSG_CANNOT_INIT_API_KEY[] =
    "ProviderManager.CMPI.CMPIProvider.CANNOT_INIT_API";
static const char _MSG_CANNOT_INIT_API[] =
    "Error initializing CMPI MI $0, the following MI"
        " factory function(s) returned an error: $1";


// set current operations to 1 to prevent an unload
// until the provider has had a chance to initialize
CMPIProvider::CMPIProvider(
    const String & name,
    const String & moduleName,
    CMPIProviderModule *module,
    ProviderVector *mv)
    : _status(UNINITIALIZED), _module(module), _cimom_handle(0), _name(name),
    _moduleName(moduleName), _no_unload(0),  _threadWatchList(),
    _cleanedThreads()
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPIProvider::CMPIProvider()");
    _current_operations = 1;
    _currentSubscriptions = 0;
    _broker.hdl =0;
    _broker.provider = this;
    if (mv)
    {
        _miVector = *mv;
    }
    unloadStatus = CMPI_RC_DO_NOT_UNLOAD;
    Time::gettimeofday(&_idleTime);
    PEG_METHOD_EXIT();
}

CMPIProvider::~CMPIProvider(void)
{
}

CMPIProvider::Status CMPIProvider::getStatus()
{
    AutoMutex lock(_statusMutex);
    return(_status);
}

void CMPIProvider::set(
    CMPIProviderModule *&module,
    ProviderVector cmpiProvider,
    CIMOMHandle *&cimomHandle)
{
    _module = module;
    _miVector = cmpiProvider;
    _cimom_handle = cimomHandle;
}

void CMPIProvider::reset()
{
    
    _currentSubscriptions = 0;
    _module = 0;
    _cimom_handle = 0;
    _no_unload = 0;
    _status = UNINITIALIZED;
    unloadStatus = CMPI_RC_DO_NOT_UNLOAD;
}

CMPIProviderModule *CMPIProvider::getModule() const
{
    return(_module);
}

String CMPIProvider::getModuleName() const
{
    return _moduleName;
}

String CMPIProvider::getName() const
{
    return(_name.subString(1,PEG_NOT_FOUND));
}

String CMPIProvider::getNameWithType() const
{
    return(_name);
}

void setError(
    ProviderVector &miVector,
    String &errorMessage,
    const String &realProviderName,
    const char *generic,
    const char *spec,
    const CMPIString *optMsg)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPIProvider:setError()");
    if (errorMessage.size() > 0)
    {
        errorMessage.append("; ");
    }

    String MItype;
    if (miVector.genericMode)
    {
        MItype.append(generic);
    }
    else
    {
        MItype.append(realProviderName);
        MItype.append(spec);
    }

    if (optMsg && CMGetCharsPtr(optMsg,NULL))
    {
        MessageLoaderParms mlp(
            "ProviderManager.CMPI.CMPIProvider.MESSAGE_WAS",
            "$0, message was: $1",
            MItype,
            CMGetCharsPtr(optMsg,NULL));

        errorMessage.append(MessageLoader::getMessage(mlp));
    }
    else
    {
        errorMessage.append(MItype);
    }
    PEG_METHOD_EXIT();
}

void CMPIProvider::initialize(
    CIMOMHandle & cimom,
    ProviderVector & miVector,
    String & name,
    CMPI_Broker & broker)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPIProvider::initialize()");
    broker.hdl=&cimom;
    broker.bft=CMPI_Broker_Ftab;
    broker.eft=CMPI_BrokerEnc_Ftab;
    broker.xft=CMPI_BrokerExt_Ftab;
    broker.mft=NULL;    // CMPI memory services not supported

    broker.name=name;

    miVector.instMI = NULL;
    miVector.assocMI = NULL;
    miVector.methMI = NULL;
    miVector.propMI = NULL;
    miVector.indMI = NULL;

    PEG_METHOD_EXIT();
}

void CMPIProvider::initialize(CIMOMHandle & cimom)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPIProvider::initialize()");
    String providername(getName());

    if (_status == UNINITIALIZED)
    {
        String compoundName;
        if (_location.size() == 0)
        {
            compoundName = providername;
        }
        else
        {
            compoundName = _location + ":" + providername;
        }
        CMPIProvider::initialize(cimom,_miVector,compoundName,_broker);
        _status = INITIALIZED;
        _current_operations = 0;
    }
    PEG_METHOD_EXIT();
}

Boolean CMPIProvider::tryTerminate()
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPIProvider::tryTerminate()");

    Boolean terminated = false;

    if (_status == INITIALIZED)
    {
        if (false == unload_ok())
        {
            PEG_METHOD_EXIT();
            return false;
        }

        Status savedStatus=_status;

        try
        {
            if (unloadStatus != CMPI_RC_OK)
            {
                // False means that the CIMServer is not shutting down.
                _terminate(false);
                if (unloadStatus != CMPI_RC_OK)
                {
                    _status=savedStatus;
                    PEG_METHOD_EXIT();
                    return false;
                }
                terminated=true;
            }
        }
        catch (...)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                "Exception caught in CMPIProviderFacade::tryTerminate() for %s",
                (const char*)getName().getCString()));
            terminated = false;

        }
        if (terminated == true)
        {
            _status = UNINITIALIZED;
        }
    }
    PEG_METHOD_EXIT();
    return terminated;
}

/*
 Terminates the CMPIProvider by cleaning its class cache and
 calling its cleanup funtions.

 @argument terminating When set to false, the provider may resist terminating.
      If true, provider MUST clean up.
*/
void CMPIProvider::_terminate(Boolean terminating)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPIProvider::_terminate()");
    const OperationContext opc;
    CMPIStatus rc={CMPI_RC_OK,NULL};
    CMPI_ContextOnStack eCtx(opc);
    CMPI_ThreadContext thr(&_broker,&eCtx);
/*
 @param terminating When true, the terminating argument indicates that the MB
     is in the process of terminating and that cleanup must be done. When
     set to false, the MI may respond with
     CMPI_IRC_DO_NOT_UNLOAD, or CMPI_IRC_NEVER_UNLOAD,
     indicating that unload will interfere with current MI processing.
     @return Function return status. The following CMPIrc codes shall
     be recognized:
        CMPI_RC_OK Operation successful.
        CMPI_RC_ERR_FAILED Unspecific error occurred.
        CMPI_RC_DO_NOT_UNLOAD Operation successful - do not unload now.
        CMPI_RC_NEVER_UNLOAD Operation successful - never unload.
*/
    unloadStatus = CMPI_RC_OK;
    if (_miVector.instMI)
    {
        rc=_miVector.instMI->ft->cleanup(_miVector.instMI,&eCtx, terminating);
        unloadStatus = rc.rc;
    }
    if (_miVector.assocMI)
    {
        rc=_miVector.assocMI->ft->cleanup(_miVector.assocMI,&eCtx, terminating);
        if (unloadStatus == CMPI_RC_OK)
        {
            unloadStatus = rc.rc;
        }
    }
    if (_miVector.methMI)
    {
        rc=_miVector.methMI->ft->cleanup(_miVector.methMI,&eCtx, terminating);
        if (unloadStatus == CMPI_RC_OK)
        {
            unloadStatus = rc.rc;
        }
    }
    if (_miVector.propMI)
    {
        rc=_miVector.propMI->ft->cleanup(_miVector.propMI,&eCtx, terminating);
        if (unloadStatus == CMPI_RC_OK)
        {
            unloadStatus = rc.rc;
        }
    }
    if (_miVector.indMI)
    {
        rc=_miVector.indMI->ft->cleanup(_miVector.indMI,&eCtx, terminating);
        if (unloadStatus == CMPI_RC_OK)
        {
            unloadStatus = rc.rc;
        }
    }

    if (unloadStatus == CMPI_RC_OK || terminating)
    {
        // Check the thread list to make sure the thread has been de-allocated
        if (_threadWatchList.size() != 0)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "There are %d provider threads in %s that have to be cleaned "
                "up.",
                _threadWatchList.size(), (const char *)getName().getCString()));

            // Walk through the list and terminate the threads. After they are
            // terminated, put them back on the watch list, call the cleanup
            //  function and wait until the cleanup is completed.
            while (_threadWatchList.size() > 0)
            {
                // Remove the thread from the watch list and kill it.
                Thread *t = _threadWatchList.remove_front();

                /* If this a non-production build, DO NOT do the cancellation.
                   This is done so that the provider developer will notice
                   incorrect behaviour when unloading his/her provider and
                   hopefully fix that.
                */
#if !defined(PEGASUS_DEBUG)
# if defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
                Logger::put(Logger::STANDARD_LOG, System::CIMSERVER,
                    Logger::WARNING,
                    "Provider thread in $0 did not exit after cleanup function."
                    " Attempting to terminate it.",
                    (const char *)getName().getCString());
                t->cancel();
# else
                // Every other OS that we do not want to do cancellation for.
                Logger::put(Logger::STANDARD_LOG, System::CIMSERVER,
                    Logger::WARNING,
                    "Provider thread in $0 did not exit after cleanup"
                    " function. Ignoring it.",
                    (const char *)getName().getCString());
# endif
#else
                // For the non-release
                Logger::put(Logger::STANDARD_LOG, System::CIMSERVER,
                    Logger::WARNING,
                    "Provider thread in $0 did not exit after cleanup"
                    " function. Ignoring it.",
                    (const char *)getName().getCString());
                // The cancellation is disabled so that when the join happends
                // the CIMServer will hang. This should help the provider
                //   writer to fix his/her providers.
                //t->cancel();
#endif
                //  and perform the normal  cleanup procedure
                _threadWatchList.insert_back(t);
                removeThreadFromWatch(t);
            }
        }
        // threadWatchList size ZERO doesn't mean that all threads have
        //  been cleaned-up. While unloading communication libraries,
        //  Threads waiting for MB UP calls might have
        // just got removed from watchlist and not cleaned.

        // Wait until all of the threads have been cleaned.
        waitUntilThreadsDone();

    }
    // We have killed all threads running in provider forcibly. Set
    // unloadStatus of provider to OK.
    if (terminating)
    {
        unloadStatus = CMPI_RC_OK;
    }
    PEG_METHOD_EXIT();
}


void CMPIProvider::terminate()
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPIProvider::terminate()");
    if (_status == INITIALIZED)
    {
        try
        {

            _terminate(true);
            PEGASUS_ASSERT(unloadStatus == CMPI_RC_OK);
        }
        catch (...)
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                "Exception caught in CMPIProviderFacade::Terminate for %s",
                (const char*)getName().getCString()));
            throw;
        }
    }

    // Provider's cleanup method called successfully, if there are still any
    // pending operations with provider then we were asked to cleanup forcibly,
    // don't uninitialize provider.
    if (_current_operations.get() == 0)
    {
        _status = UNINITIALIZED;
    }

    PEG_METHOD_EXIT();
}

/*
 * Wait until all finished provider threads have been cleaned and deleted.
 * Note: This should NEVER be called from the thread that
 * IS the Thread object that was is finished and called
 * 'removeThreadFromWatch()' . If you do it, you will
 * wait forever.
 */
void CMPIProvider::waitUntilThreadsDone()
{
    while (_cleanedThreads.size() > 0)
    {
        Threads::yield();
    }
}
/*
 * Check if the Thread is owner by this CMPIProvider object.
 *
 * @argument t Thread that is not NULL.
 */
Boolean CMPIProvider::isThreadOwner(Thread *t)
{
    PEGASUS_ASSERT ( t != NULL );
    if (_cleanedThreads.contains(t))
    {
        return true;
    }
    if (!_threadWatchList.contains(t))
    {
        return true;
    }
    return false;
}
/*
 * Remove the thread from the list of threads that are being deleted
 * by the CMPILocalProviderManager.
 *
 * @argument t Thread which has been previously provided
 * to 'removeThreadFromWatch' function.
 */
void CMPIProvider::threadDelete(Thread *t)
{
    PEGASUS_ASSERT ( _cleanedThreads.contains(t) );
    PEGASUS_ASSERT ( !_threadWatchList.contains(t) );
    _cleanedThreads.remove( t );
}

/*
// Removes the thread from the watch list and schedule the
// CMPILocalProviderManager to delete the thread. The
// CMPILocalProviderManager after deleting the thread calls
// the CMPIProvider' "cleanupThread". The CMPILocalProviderManager
// notifies this CMPIProvider object when the thread
// is truly dead by calling "threadDeleted" function.
//
// Note that this function is called from the thread that finished with
// running the providers function, and returns immediately while scheduling
// the a cleanup procedure. If you want to wait until the thread is truly
//  deleted, call 'waitUntilThreadsDone' - but DO NOT do it in the the thread
// that the Thread owns - you will wait forever.
//
// @argument t Thread that is not NULL and finished with running
//  the provider function.
*/
void CMPIProvider::removeThreadFromWatch(Thread *t)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPIProvider::removeThreadFromWatch()");
    PEGASUS_ASSERT( t != 0 );

    // Note: After MI returned true from cleanup() method , there might be some
    // threads running in MI. CMPILocalProviderManager::cleanupThread() called
    // below will take care of joining the running threads in MI.
    {
        AutoMutex mtx(_removeThreadMutex);
        if (_threadWatchList.contains(t))
        {
            // Remove it from the watched list
            _threadWatchList.remove(t);
        }
        else
        {
            // This thread already has been removed from watch list.
            PEG_METHOD_EXIT();
            return;
        }
    }

    PEGASUS_ASSERT (!_cleanedThreads.contains (t));


    // Add the thread to the CMPIProvider's list.
    // We use this list to keep track of threads that are
    // being cleaned (this way 'waitUntilThreadsDone' can stall until the
    // threads are truly deleted).
    _cleanedThreads.insert_back(t);

    CMPILocalProviderManager::cleanupThread(t, this);
    PEG_METHOD_EXIT();
}

/*
 * Adds the thread to the watch list. The watch list is monitored when the
 * provider is terminated and if any of the threads have not cleaned up by
 * that time, they are forcifully terminated and cleaned up.
 *
 * @argument t Thread is not NULL.
*/
void CMPIProvider::addThreadToWatch(Thread *t)
{
    PEGASUS_ASSERT( t != 0 );

    _threadWatchList.insert_back(t);
}

void CMPIProvider::get_idle_timer(struct timeval *t)
{
    PEGASUS_ASSERT(t != 0);
    AutoMutex lock(_idleTimeMutex);
    memcpy(t, &_idleTime, sizeof(struct timeval));
}

void CMPIProvider::update_idle_timer()
{
    AutoMutex lock(_idleTimeMutex);
    Time::gettimeofday(&_idleTime);
}

/*
 * This method returns "false" if there are any requests pending with
 * the provider or Provider has returned CMPI_RC_NEVER_UNLOAD in the last
 * cleanup() invocation cyle.
*/
Boolean CMPIProvider::unload_ok()
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPIProvider::unload_ok()");
    if (unloadStatus == CMPI_RC_NEVER_UNLOAD)
    {
        PEG_METHOD_EXIT();
        return false;
    }
    if (_no_unload.get())
    {
        PEG_METHOD_EXIT();
        return false;
    }
    // Do not call CIMOMHandle::unload_ok here.
    // CIMOMHandle::unload_ok method tests for _providerUnloadProtect
    // and if zero returns true. _providerUnloadProtect is
    // incremented when CIMOMHandle::disallowProviderUnload()
    // is called and decremented when
    // CIMOMHandle::allowProviderUnload() is called. There is
    // no way these functions are called from CMPI Providers.(Bug 6642)
    PEG_METHOD_EXIT();
    return true;
}

//   force provider manager to keep in memory
void CMPIProvider::protect()
{
    _no_unload++;
}

// allow provider manager to unload when idle
void CMPIProvider::unprotect()
{
    _no_unload--;
}

Boolean CMPIProvider::testIfZeroAndIncrementSubscriptions ()
{
    AutoMutex lock (_currentSubscriptionsMutex);
    Boolean isZero = (_currentSubscriptions == 0);
    _currentSubscriptions++;

    return isZero;
}

Boolean CMPIProvider::decrementSubscriptionsAndTestIfZero ()
{
    AutoMutex lock (_currentSubscriptionsMutex);
    _currentSubscriptions--;
    Boolean isZero = (_currentSubscriptions == 0);

    return isZero;
}

Boolean CMPIProvider::testSubscriptions ()
{
    AutoMutex lock (_currentSubscriptionsMutex);
    Boolean currentSubscriptions = (_currentSubscriptions > 0);

    return currentSubscriptions;
}

void CMPIProvider::setProviderInstance (const CIMInstance & instance)
{
    _providerInstance = instance;
}

CIMInstance CMPIProvider::getProviderInstance ()
{
    return _providerInstance;
}

void CMPIProvider::incCurrentOperations ()
{
    _current_operations++;
}

int CMPIProvider::getCurrentOperations ()
{
    return _current_operations.get();
}

void CMPIProvider::decCurrentOperations ()
{
    _current_operations--;
}

CIMOMHandle *CMPIProvider::getCIMOMHandle()
{
    return _cimom_handle;
}

CMPI_Broker *CMPIProvider::getBroker()
{
    return &_broker;
}

CMPIInstanceMI *CMPIProvider::getInstMI()
{
    if (_miVector.instMI == NULL)
    {
        AutoMutex mtx(_statusMutex);
        if (_miVector.instMI == NULL)
        {
            const OperationContext opc;
            CMPI_ContextOnStack eCtx(opc);
            CMPIStatus rc = {CMPI_RC_OK, NULL};
            String providerName = _broker.name;
            CMPIInstanceMI *mi = NULL;

            if (_miVector.genericMode && _miVector.createGenInstMI)
            {
                mi = _miVector.createGenInstMI(
                    &_broker,
                    &eCtx,
                    (const char *)providerName.getCString(),
                    &rc);
            }
            else if (_miVector.createInstMI)
            {
                mi = _miVector.createInstMI(&_broker, &eCtx, &rc);
            }

            if (!mi || rc.rc != CMPI_RC_OK)
            {
                String error;
                setError(
                    _miVector,
                    error,
                    getName(),
                    _Generic_Create_InstanceMI,
                    _Create_InstanceMI,
                    rc.msg);

                throw Exception(
                    MessageLoaderParms(
                        _MSG_CANNOT_INIT_API_KEY,
                        _MSG_CANNOT_INIT_API,
                        getName(),
                        error));
            }
            _miVector.instMI = mi;
        }
    }

    return _miVector.instMI;
}

CMPIMethodMI *CMPIProvider::getMethMI()
{
    if (_miVector.methMI == NULL)
    {
        AutoMutex mtx(_statusMutex);
        if (_miVector.methMI == NULL)
        {
            const OperationContext opc;
            CMPI_ContextOnStack eCtx(opc);
            CMPIStatus rc = {CMPI_RC_OK, NULL};
            String providerName = _broker.name;
            CMPIMethodMI *mi = 0;

            if (_miVector.genericMode && _miVector.createGenMethMI)
            {
                mi = _miVector.createGenMethMI(
                    &_broker,
                    &eCtx,
                    (const char *)providerName.getCString(),
                    &rc);
            }
            else if (_miVector.createMethMI)
            {
                mi = _miVector.createMethMI(&_broker, &eCtx, &rc);
            }
            if (!mi || rc.rc != CMPI_RC_OK)
            {
                String error;
                setError(
                    _miVector,
                    error,
                    getName(),
                    _Generic_Create_MethodMI,
                    _Create_MethodMI,
                    rc.msg);

                throw Exception(
                    MessageLoaderParms(
                        _MSG_CANNOT_INIT_API_KEY,
                        _MSG_CANNOT_INIT_API,
                        getName(),
                        error));
            }
            _miVector.methMI = mi;
        }
    }

    return _miVector.methMI;
}

CMPIAssociationMI *CMPIProvider::getAssocMI()
{
    if (_miVector.assocMI == NULL)
    {
        AutoMutex mtx(_statusMutex);
        if (_miVector.assocMI == NULL)
        {
            const OperationContext opc;
            CMPI_ContextOnStack eCtx(opc);
            CMPIStatus rc = {CMPI_RC_OK, NULL};
            String providerName = _broker.name;
            CMPIAssociationMI *mi = 0;

            if (_miVector.genericMode && _miVector.createGenAssocMI)
            {
                mi = _miVector.createGenAssocMI(
                    &_broker,
                    &eCtx,
                    (const char *)providerName.getCString(),
                    &rc);
            }
            else if (_miVector.createAssocMI)
            {
                mi = _miVector.createAssocMI(&_broker, &eCtx, &rc);
            }

            if (!mi || rc.rc != CMPI_RC_OK)
            {
                String error;
                setError(
                    _miVector,
                    error,
                    getName(),
                    _Generic_Create_AssociationMI,
                    _Create_AssociationMI,
                    rc.msg);

                throw Exception(
                    MessageLoaderParms(
                        _MSG_CANNOT_INIT_API_KEY,
                        _MSG_CANNOT_INIT_API,
                        getName(),
                        error));
            }
            _miVector.assocMI = mi;
        }
    }

    return _miVector.assocMI;
}

CMPIPropertyMI *CMPIProvider::getPropMI()
{
    if (_miVector.propMI == NULL)
    {
        AutoMutex mtx(_statusMutex);
        if (_miVector.propMI == NULL)
        {
            const OperationContext opc;
            CMPI_ContextOnStack eCtx(opc);
            CMPIStatus rc = {CMPI_RC_OK, NULL};
            String providerName = _broker.name;
            CMPIPropertyMI *mi = 0;

            if (_miVector.genericMode && _miVector.createGenPropMI)
            {
                mi = _miVector.createGenPropMI(
                    &_broker,
                    &eCtx,
                    (const char *)providerName.getCString(),
                    &rc);
            }
            else if (_miVector.createPropMI)
            {
                mi = _miVector.createPropMI(&_broker, &eCtx, &rc);
            }

            if (!mi || rc.rc != CMPI_RC_OK)
            {
                String error;
                setError(
                    _miVector,
                    error,
                    getName(),
                    _Generic_Create_PropertyMI,
                    _Create_PropertyMI,
                    rc.msg);

                throw Exception(
                    MessageLoaderParms(
                        _MSG_CANNOT_INIT_API_KEY,
                        _MSG_CANNOT_INIT_API,
                        getName(),
                        error));
            }
            _miVector.propMI = mi;
        }
    }

    return _miVector.propMI;
}

CMPIIndicationMI *CMPIProvider::getIndMI()
{
    if (_miVector.indMI == NULL)
    {
        AutoMutex mtx(_statusMutex);
        if (_miVector.indMI == NULL)
        {
            const OperationContext opc;
            CMPI_ContextOnStack eCtx(opc);
            CMPIStatus rc = {CMPI_RC_OK, NULL};
            String providerName = _broker.name;
            CMPIIndicationMI *mi = 0;

            if (_miVector.genericMode && _miVector.createGenIndMI)
            {
                mi = _miVector.createGenIndMI(
                    &_broker,
                    &eCtx,
                    (const char *)providerName.getCString(),
                    &rc);
            }
            else if (_miVector.createIndMI)
            {
                mi = _miVector.createIndMI(&_broker, &eCtx, &rc);
            }

            if (!mi || rc.rc != CMPI_RC_OK)
            {
                String error;
                setError(
                    _miVector,
                    error,
                    getName(),
                    _Generic_Create_IndicationMI,
                    _Create_IndicationMI,
                    rc.msg);

                throw Exception(
                    MessageLoaderParms(
                        _MSG_CANNOT_INIT_API_KEY,
                        _MSG_CANNOT_INIT_API,
                        getName(),
                        error));
            }
            _miVector.indMI = mi;
        }
    }

    return _miVector.indMI;
}

CMPIProviderModule *CMPIProvider::getModule()
{
    return _module;
}

Uint32 CMPIProvider::getQuantum()
{
    AutoMutex mutex(_statusMutex);
    return _quantum;
}

void CMPIProvider::setQuantum(Uint32 quantum)
{
    AutoMutex mutex(_statusMutex);
    _quantum = quantum;
}

Mutex &CMPIProvider::getStatusMutex()
{
    return _statusMutex;
}

PEGASUS_NAMESPACE_END
