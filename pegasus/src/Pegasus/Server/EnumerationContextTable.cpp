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

#include "EnumerationContextTable.h"

#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/General/Guid.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/Condition.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Server/EnumerationContext.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/IDFactory.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Config/ConfigManager.h>
// Used only for the single static function call to issueSavedResponses
#include <Pegasus/Server/CIMOperationRequestDispatcher.h>

#include <Pegasus/Server/CIMServer.h>
#include <Pegasus/ProviderManagerService/ProviderManagerService.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// define conversion between sec and usec
#define PEG_MICROSEC 1000000

// General class to process various objects that are made up of Pegaus
// Strings back to the String and more directly to the const char* ...
// used for display. This can be used for
// String, CIMName, CIMNamespaceName, Exception, CIMDateTime, CIMObjectPath
// The same general class exists in several places in OpenPegasus.
// TODO: make this a general part of Pegasus so it is not duplicated in
// many different files.
class Str
{
public:
    Str(const String& s) : _cstr(s.getCString()) { }
    Str(const CIMName& n) : _cstr(n.getString().getCString()) { }
    Str(const CIMNamespaceName& n) : _cstr(n.getString().getCString()) { }
    Str(const Exception& e) : _cstr(e.getMessage().getCString()) { }
    Str(const CIMDateTime& x) : _cstr(x.toString().getCString()) { }
    Str(const CIMObjectPath& x) : _cstr(x.toString().getCString()) { }
    const char* operator*() const { return (const char*)_cstr; }
    operator const char*() const { return (const char*)_cstr; }
private:
    CString _cstr;
};

// Definition of static variable that can be set by  config manager
Uint32 EnumerationContextTable::_defaultOperationTimeoutSec =
    PEGASUS_DEFAULT_PULL_OPERATION_TIMEOUT_SEC;

#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
Mutex EnumerationContextTable::_defaultOperationTimeoutSecMutex;
#endif

// Define the table instance that will contain enumeration contexts for Open,
// Pull, Close, and countEnumeration operations.  The default interoperation
// timeout and max cache size are set as part of creating the table.
//
// TODO FUTURE: There are several system parameters here that should be
// set globally.  We need somewhere common to be able to define this and
// while it could be the config, it could also be a systemLimits file or
// something common.
#define PEGASUS_MAX_OPEN_ENUMERATE_CONTEXTS 256

    // Define the maximum size for the response cache in each
    // enumerationContext.  As responses are returned from providers this is the
    // maximum number that can be placed in the CIMResponseData cache waiting
    // for pull operations to send them as responses before responses
    // start backing up to the providers (i.e. delaying return from the provider
    // deliver calls.
    // FUTURE: As we develop more flexible resource management this value should
    // be modified for each context creation in terms of the object sizes
    // expected and the memory usage of the CIMServer.  Thus, it would be
    // logical to allow caching many more path responses than instance
    // responses because they are probably much smaller.
    // This variable is not externalized because we are not sure
    // if that is logical.
#define PEGASUS_PULL_RESPONSE_CACHE_DEFAULT_MAX_SIZE 1000

Uint32 responseCacheDefaultMaximumSize =
        PEGASUS_PULL_RESPONSE_CACHE_DEFAULT_MAX_SIZE;

EnumerationContextTable* EnumerationContextTable::pInstance = NULL;

// Thread execution function for timeoutThread(). This thread executes
// regular timeout tests on active contexts and closes them or marks them
// for close if timed out.  This is required for those cases where a
// pull sequence is terminated without either completing or closing the
// sequence.

ThreadReturnType PEGASUS_THREAD_CDECL operationContextTimeoutThread(void* parm)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContextTable::operationContextTimerThread");

    Thread *myself = reinterpret_cast<Thread *>(parm);
    EnumerationContextTable* et =
        reinterpret_cast<EnumerationContextTable*>(myself->get_parm());

    PEGASUS_DEBUG_ASSERT(et->valid());

    Uint32 nextTimeoutMsec = et->getTimeoutIntervalMsec();

    // execute loop at regular intervals until no more contexts or
    // stop flag set
    while (!et->_stopTimeoutThreadFlag.get() )
    {
        et->_timeoutThreadWaitSemaphore.time_wait(nextTimeoutMsec);

        // true return indicates table empty which terminates loop.
        if (et->processExpiredContexts())
        {
            break;
        }
    }

    // reset the timeout value to indicate that the thread is quiting
    et->_timeoutThreadRunningFlag = 0;

    PEG_METHOD_EXIT();
    return ThreadReturnType(0);
}

/************************************************************************
**
**  Implementation of EnumerationContextTable Class
**
************************************************************************/
/*
    Constructor.  Note that the hashtable size is 1/2 the maximum number
    of simultaneous open contexts.  This was a guess based on notes in
    the Hashtable code indicating that 1/3 might be logical choice.
*/
EnumerationContextTable::EnumerationContextTable()
    :
    _timeoutIntervalMsec(0),
    // Defines the Context Hash table size as 1/2 the max number of entries
    _enumContextTable(PEGASUS_MAX_OPEN_ENUMERATE_CONTEXTS / 2),
    _operationContextTimeoutThread(operationContextTimeoutThread, this, true),
    _responseCacheMaximumSize(PEGASUS_PULL_RESPONSE_CACHE_DEFAULT_MAX_SIZE),
    _cacheHighWaterMark(0),
    _responseObjectCountHighWaterMark(0),
    _totalObjectsReturned(0),
    _enumerationContextsOpened(0),
    _enumerationsTimedOut(0),
    _maxOpenContexts(0),
    _maxOpenContextsLimit(PEGASUS_MAX_OPEN_ENUMERATE_CONTEXTS),
    _highWaterRequestsPerSequence(0),
    _totalRequestsPerSequence(0),
    _sequencesClosed(0),
    _requestedSize(0),
    _requestCount(0),
    _totalZeroLenDelayedResponses(0),
    // Initial Value for ContextId counter.  500000 is arbitary
    _contextIdCounter(500000)
{
    // Setup the default value for the operation timeout value if the value
    // received  in a request is NULL.  This is the server defined default.
    ConfigManager* configManager = ConfigManager::getInstance();

    _defaultOperationTimeoutSec = ConfigManager::parseUint32Value(
    configManager->getCurrentValue("pullOperationsDefaultTimeout"));
}

// Create the singleton instance of the enumerationContextTable and return
// pointer to that instance. If created, return pointer to existing singleton
// instance
EnumerationContextTable* EnumerationContextTable::getInstance()
{
    if (!pInstance)
    {
        pInstance = new EnumerationContextTable();
    }
    return pInstance;
}

/* Remove all contexts and delete them. Only used on system shutdown.
*/
EnumerationContextTable::~EnumerationContextTable()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContextTable::~EnumerationContextTable");

    _stopTimeoutThread();

    // Show shutdown statistics for EnumerationContextTable
    displayStatistics();

    removeContextTable();

    PEG_METHOD_EXIT();
}

/*
    Create a new context entry and return it. This includes information
    required to process the pull and close operations for the enumeration
    sequence controlled by this context. The context instance will remain
    active for the life of the enumeration sequence.
    Returns pointer to the new context except if:
       - Size exceeds system limit.
*/

EnumerationContext* EnumerationContextTable::createContext(
    const CIMOpenOperationRequestMessage* request,
    MessageType pullRequestType,
    CIMResponseData::ResponseDataContent contentType)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,"EnumerationContextTable::createContext");

    AutoMutex autoMut(_tableLock);

    // Test for Max Number of simultaneous contexts.
    if (_enumContextTable.size() > _maxOpenContextsLimit)
    {
        PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL1,
            "Error EnumerationContext Table exceeded Max limit of %u",
             _maxOpenContextsLimit));
        return NULL;
    }
    // set the operation timeout to either the default or current
    // value
    Uint32 operationTimeout = (request->operationTimeout.isNull())?
        _defaultOperationTimeoutSec
        :
        request->operationTimeout.getValue();

    // Create new contextId
    Uint32 rtnSize;
    char scratchBuffer[22];
    const char* contextId = Uint32ToString(scratchBuffer,
        _getNextId(), rtnSize);

    // Create new context, Context name is monolithically increasing counter.
    EnumerationContext* en = new EnumerationContext(contextId,
        request->nameSpace,
        operationTimeout,
        request->continueOnError,
        pullRequestType,
        contentType);

    // Set the maximum size for the response Cache from the default
    // value in the table. This is for future where we could adjust the
    // size dynamically for each operation depending on resource utilization.
    // or expected response sizes (ex. paths vs instances)
    en->_responseCacheMaximumSize = _responseCacheMaximumSize;

    // Pointer back to this object
    en->_enumerationContextTable = this;

    // insert new context into the table. Failure to insert is a
    // system failure
    if(!_enumContextTable.insert(contextId, en))
    {
        PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL1,
            "Error Creating Enumeration Context ContextId=%s. System Failed",
            contextId ));
        PEGASUS_ASSERT(false);  // This is a system failure
    }

    _enumerationContextsOpened++;

    // set new highwater mark for maxOpenContexts if necessary
    if (_enumContextTable.size() >_maxOpenContexts )
    {
        _maxOpenContexts = _enumContextTable.size();
    }

    PEG_METHOD_EXIT();
    return en;
}

void EnumerationContextTable::displayStatistics(bool clearStats)
{
    // Show statistics for EnumerationContextTable
    // Should add avg size of requests.  Maybe some other info.
    cout << buildStatistics(clearStats) << endl;
}

// build a string with the statistics info.
String EnumerationContextTable::buildStatistics(bool clearStats)
{
    String str;

    AutoMutex autoMut(_tableLock);
    if (_enumerationContextsOpened != 0)
    {
        str.appendPrintf("EnumerationTable Statistics:"
            "\n  EnumerationCache highwater mark=%u"
            "\n  Max simultaneous enumerations=%u"
            "\n  Total enumerations opened=%llu",
            _cacheHighWaterMark,
            _maxOpenContexts,
            _enumerationContextsOpened);

        str.appendPrintf(
            "\n  Enumerations timed out=%u"
            "\n  Current open enumerations=%u"
            "\n  Avg request maxObjectCount=%u"
            "\n  Max objects/response=%u"
            "\n  Avg objects/response=%u"
            "\n  Avg requests/sequence=%u"
            "\n  Max requests/sequence=%u",
            _enumerationsTimedOut,
            size(),
            _getAvgRequestSize(),
            _responseObjectCountHighWaterMark,
            _getAvgResponseSize(),
            _getAvgRequestsPerSequence(),
            _highWaterRequestsPerSequence);

        str.appendPrintf(
            "\n  Total zero Length delayed responses=%llu",
            _totalZeroLenDelayedResponses);
    }
    if (clearStats)
    {
        _cacheHighWaterMark = 0;
        _maxOpenContexts = 0;
        _enumerationContextsOpened = 0;
        _enumerationsTimedOut = 0;
        _requestCount = 0;
        _requestedSize = 0;
        _responseObjectCountHighWaterMark = 0;
        _totalZeroLenDelayedResponses = 0;
        _totalObjectsReturned = 0;
        _totalRequestsPerSequence = 0;
        _highWaterRequestsPerSequence = 0;
        _sequencesClosed = 0;
    }
    return str;
}

void EnumerationContextTable::removeContextTable()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContextTable::removeContextTable");

    AutoMutex autoMut(_tableLock);
    // Clear out any existing enumerations.
    for (EnumContextTableType::Iterator i = _enumContextTable.start(); i; i++)
    {
        EnumerationContext* en = i.value();

        PEG_TRACE(( TRC_ENUMCONTEXT, Tracer::LEVEL4,
            "EnumerationTable Delete. "
                " ContextId=%s. Existed for %llu milliseconds",
             *Str(en->getContextId()),
             ((TimeValue::getCurrentTime().toMilliseconds()
               - en->_startTimeUsec)/1000) ));
        delete en;
    }
    _enumContextTable.clear();
    PEG_METHOD_EXIT();
}

bool EnumerationContextTable::releaseContext(EnumerationContext* en)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,"EnumerationContextTable::releaseContext");

    AutoMutex autoMut(_tableLock);

    PEGASUS_DEBUG_ASSERT(valid());
    PEGASUS_DEBUG_ASSERT(en->valid());

    String contextId = en->getContextId();

    en->unlockContext();

    // Check to assure that the context ID is in the table.
    if (!find(contextId))
    {
        PEG_METHOD_EXIT();
        return false;
    }
    _removeContext(en);

    PEG_METHOD_EXIT();
    return true;
}

// Private remove function with no lock protection. The _tableLock must
// be set before this function is called to protect the table. This
// removes the context from the context table.
// Boolean return: Returns false if attempt to remove context that is
// not closed and providers complete. However, for now it then asserts since
// this is really an internal failure.

bool EnumerationContextTable::_removeContext(EnumerationContext* en)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,"EnumerationContextTable::_removeContext");

    PEGASUS_DEBUG_ASSERT(en->valid());

    // If context is valid and providers are complete, remove
    // the enumerationContext.  If providers not complete, only
    // completion of provider deliveries can initiate removal of
    // the enumeration context.

    // Any functions that set _ClientClosed and _ProvidersComplete
    // must insure that they block until finished with context.
    if (en->_clientClosed && en->_providersComplete)
    {
        PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL3,
            "EnumerationContext Remove. ContextId=%s",
            *Str(en->getContextId()) ));
        //// KS_TODO Remove this trace output.
        en->trace();
       // Should never get here with _savedRequest NULL. Choice was to
       // release or assert during tests.
       PEGASUS_DEBUG_ASSERT(en->_savedRequest == NULL);

        // test/set the highwater mark for the table
        // KS_TODO confirm that statistics below always get accumulated

        if (en->_cacheHighWaterMark > _cacheHighWaterMark)
        {
            _cacheHighWaterMark = en->_cacheHighWaterMark;
        }

        if (en->_responseObjectsCount > _responseObjectCountHighWaterMark)
        {
            _responseObjectCountHighWaterMark = en->_responseObjectsCount;
        }

        if (en->_requestCount > _highWaterRequestsPerSequence)
        {
            _highWaterRequestsPerSequence = en->_requestCount;
        }

        _totalObjectsReturned += en->_responseObjectsCount;
        _totalZeroLenDelayedResponses+= en->_totalZeroLenObjectResponseCounter;

        _totalRequestsPerSequence += en->_requestCount;
        _sequencesClosed++;

        _enumContextTable.remove(en->getContextId());

        delete en;

        PEG_METHOD_EXIT();
        return true;
    }

    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL3,
            "EnumerationContext Remove Ignored. ContextId=%s not complete.",
            *Str(en->getContextId()) ));
    en->trace();

    // This assert is part of testing. KS_TODO Remove before release
    PEGASUS_DEBUG_ASSERT(false);

    PEG_METHOD_EXIT();
    return false;
}

// Get the number of entries in the hash table of EnumerationContexts.
Uint32 EnumerationContextTable::size()
{
    AutoMutex autoMut(_tableLock);
    return(_enumContextTable.size());
}

// If context name found, return pointer to that context.  Otherwise
// return 0
EnumerationContext* EnumerationContextTable::find(const String& contextId)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT, "EnumerationContextTable::find");

    AutoMutex autoMut(_tableLock);

    EnumerationContext* en = 0;

    if(_enumContextTable.lookup(contextId, en))
    {
        PEGASUS_DEBUG_ASSERT(en->valid());
    }
    // Return pointer or pointer = 0 if not found.
    PEG_METHOD_EXIT();
    return en;
}

/* Test all table entries and remove the ones timed out.
   Returns true if the enumeration table is empty when finished.

*/
bool EnumerationContextTable::processExpiredContexts()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContextTable::processExpiredContexts");

    PEGASUS_DEBUG_ASSERT(valid());

    if (size() == 0)
    {
        PEG_METHOD_EXIT();
        return true;
    }

    //Timed out entry context Ids will be placed in one of these lists for
    // processing so we do not remove entries from table while processing
    // The cleanActiveList is also processed after the table lock is removed.
    Array<String> removeList;
    Array<String> cleanActiveList;

    Uint64 currentTimeUsec = System::getCurrentTimeUsec();

    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
                   "processExpiredContexts Start table size=%u", size() ));
    // Lock the EnumerationContextTable so no operations can be accepted
    // during the scan process
    {    /// Context for automutex
        AutoMutex autoMut(_tableLock);

        // Search enumeration table for entries timed out. Sets any entries
        // that have timed out into a secondary lists for processing
        for (EnumContextTableType::Iterator i = _enumContextTable.start();i;i++)
        {
            EnumerationContext* en = i.value();
            PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
                       "Timeout Scan Processing ContextId=%s",
                       *Str(en->getContextId()) ));
            //// KS_TODO temporary trace. Remove before release
            en->trace();

            PEGASUS_DEBUG_ASSERT(en->valid());     // diagnostic. KS_TEMP
            if (en->valid())
            {
                if (en->_operationTimerUsec != 0)
                {
                    // Only set lock if there is a chance the timer is active.
                    // redoes the above test after setting lock. Bypass this
                    // enumerationContext if locked
                    if (en->tryLockContext())
                    {
                        // test if entry is timed out
                        if (en->isTimedOut(currentTimeUsec))
                        {
                            en->stopTimer();

                            // if active wait for provider timeout then send
                            // saved response. This restarts timer
                            if (en->isProcessing())
                            {
                                // KS_TODO temp test. This should always
                                // be true.
                                PEGASUS_DEBUG_ASSERT(!en->isClientClosed());

                                PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
                                   "Timeout while IsProcessing ContextId=%s",
                                   *Str(en->getContextId()) ));

                                cleanActiveList.append(
                                    en->getContextId());
                            }
                            // Otherwise this is the interoperation timer
                            // Close client and cleanup based on whether
                            // providers are complete
                            else
                            {
                                // set ClientClosed since there has been an
                                // interoperation timeout
                                en->setClientClosed();

                                // If providers are complete we can remove the
                                // context.
                                if (en->providersComplete())
                                {
                                    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                                       "TimeoutProvidersComplete ContextId=%s",
                                       *Str(en->getContextId())));

                                    removeList.append(en->getContextId());
                                }
                                // Providers not complete but client is closed
                                // This kicks off cleanup effort as well as
                                // signal to be sure providers continue to
                                // deliver  responses.
                                else
                                {
                                    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
                                       "Timeout Providers NOTComplete "
                                           "ContextId=%s",
                                       *Str(en->getContextId()) ));

                                    en->signalProviderWaitCondition();

                                    cleanActiveList.append(en->getContextId());
                                }
                            }
                        }
                        else  // not timed out
                        {
                            en->unlockContext();
                        }
                    } // end trylock
                }  // end timer != 0
            }  // en not valid
            else
            {
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Invalid EnumerationContext discarded, ContextId=%s"
                    " Ignored", *Str(en->getContextId()) ));

                // KS_TODO should we consider trying to remove the invalid
                // entry?
            }
        } // end for loop

        // remove any contexts in the remove list. This should be only
        // contexts with client closed and providers complete
        for (Uint32 i = 0; i < removeList.size(); i++)
        {
            // unlock before removing the context
            EnumerationContext* en = find(removeList[i]);
            PEGASUS_DEBUG_ASSERT(en->valid());
            _enumerationsTimedOut++;
            en->unlockContext();
            _removeContext(en);
        }
    }   // release the EnumerationContextTable lock

    // Process entries in the cleanActiveList. At this point
    // the table is unlocked but all contexts in this list are locked
    // These are all  entries for which the providers are incomplete
    // but not responding.
    // This may be either:
    //     - Client open Entries for which the zero len saved response is
    //       sent
    //     - Client closed entries where the provider chaing
    //       should be kicked or closed.

    for (Uint32 i = 0; i < cleanActiveList.size(); i++)
    {
        // recheck by finding, validating and checking EnumerationContext
        // state again
        //
        EnumerationContext* en = find(cleanActiveList[i]);
        PEGASUS_DEBUG_ASSERT(en->valid());

        // clean up one context
        _cleanActiveContext(en);
    }
    PEG_METHOD_EXIT();

    // Return true if table is now empty.
    return (size() == 0);
}

// Attempt to clean up the enumerationContext provided.
// Assumes the context is locked when called.
// If client open, issue any saved response and test for consecutive
// retries that pass the defined limits.
//     Consecutive resends of Zero Len responses >
//       PEGASUS_MAX_CONSECUTIVE_WAITS_BEFORE_ERR
//     results in setting the providers closed and issuing message.
//
// After 4 of these messages sent, just close the enumeration.
//
// If  client closed, start timer and do the consecutive resend tests
// to see if the provider should just be closed.

void EnumerationContextTable::_cleanActiveContext(EnumerationContext* en)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
                     "EnumerationContextTable::cleanActiveContext");
    // Increment the counter for sending zero length responses to the
    // client. Used to determine when providers stuck incomplete
    Uint32 ctr = en->incConsecutiveZeroLenObjectResponseCounter();

    Uint32 targetCount = PEGASUS_MAX_CONSECUTIVE_WAITS_BEFORE_ERR;
    Uint32 finalTargetCount = targetCount + 3;

    // If past final target count, set providers complete so we issue
    // next msg with EOS set. That closes the client. At this
    // point we have given up completely on the providers but are trying
    // to send the last message to the client
    if (ctr >= finalTargetCount)
    {
        PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
                   "IssueSaveResponseList Set ProvidersComplete "
                   "ContextId=%s ctr=%u finalTargetCount=%u",
                   *Str(en->getContextId()), ctr, finalTargetCount));

        en->setProvidersComplete();
        // Set an error.
        CIMException e = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                         "Provider Responses Failed.");
        // Force continueOnError to false since we
        // really do want to close the client
        en->setContinueOnError(false);
        en->setErrorState(e);
    }

    // If client not closed, issue an empty response to the dispatcher.
    // Otherwise just restart timer to timeout providers
    // Set the count to zero so can issue with no responses in
    // cache.
    en->_savedOperationMaxObjectCount = 0;
    if (!en->isClientClosed())
    {
        PEGASUS_DEBUG_ASSERT(en->isProcessing());
        CIMOperationRequestDispatcher::issueSavedResponse(en);
    }
    else
    {
        if (!en->providersComplete())
        {
            // restart timer to time cleanup of providers
            en->startTimer(PEGASUS_PULL_MAX_OPERATION_WAIT_SEC * PEG_MICROSEC);
        }
    }

    // Repeat test after issuing saved response.
    // If we have tried the clean up several times and it did not work,
    // discard the enumeration context. We just picked the number 4
    // as the number of retries for the cleanup before we close the
    // whole thing as broken.
    if (ctr >= finalTargetCount)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
              "Enumeration Context removed after providers did not"
                  "respond for at least %u min. ContextId=%s",
              ((finalTargetCount * PEGASUS_PULL_MAX_OPERATION_WAIT_SEC)/60),
              *Str(en->getContextId()) ));

        en->setClientClosed();
        en->unlockContext();
        _removeContext(en);
        PEG_METHOD_EXIT();
        return;
    }
    // The initial test and cleanup.  Started at targetCount and
    // repeated at least 3 times before we discard the provider
    // This calls the providerManager cleanup assuming that
    // providers have stopped delivering without sending the
    // complete flag.
    if (ctr >= targetCount)
    {
        PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
            "%u Consecutive 0 length responses issued for ContextId=%s"
            " Issuing provider cleanup.",
            ctr, *Str(en->getContextId()) ));

        // send cleanup message to provider manager.  Need pointer to
        // this service.
        CIMServer * cimserver = CIMServer::getInstance();
        // Ignore false return
        cimserver->_providerManager->enumerationContextCleanup(
            en->getContextId());
        en->unlockContext();
    }
    else
    {
        PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
            "%u Consecutive 0 length responses issued for ContextId=%s",
            ctr, *Str(en->getContextId()) ));
        en->unlockContext();
    }

    PEG_METHOD_EXIT();
}

// Validate every entry in the table.This is a diagnostic that should only
// be used in testing changes during development.
#ifdef ENUMERATIONTABLE_DEBUG
void EnumerationContextTable::tableValidate()
{
    AutoMutex autoMut(_tableLock);
    for (EnumContextTableType::Iterator i = _enumContextTable.start(); i;
        i++)
    {
        EnumerationContext* en = i.value();
        if (!en->valid())
        {
            en->trace();
            PEGASUS_DEBUG_ASSERT(en->valid());
        }
    }
}
#endif

// interval is the timeout for the current operation that
// initiated this call.  It helps the timer thread decide how often
// to scan for timeouts.
void EnumerationContextTable::dispatchTimerThread()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
                     "EnumerationContextTable::dispatchTimerThread");

    PEGASUS_DEBUG_ASSERT(valid());

    AutoMutex autoMut(_tableLock);

    // If timeout thread not running, start it
    if (_timeoutThreadRunningFlag.get() == 0)
    {
        // convert second timer to milliseconds and set it for double
        // the input defined interval. Set for the timer thread to
        // loop through the table every PEGASUS_PULL_MAX_OPERATION_WAIT_SEC
        // seconds.

        _timeoutIntervalMsec = PEGASUS_PULL_MAX_OPERATION_WAIT_SEC*1000;

        // Start the detached thread to execute the timeout tests.
        // Thread runs until the timer is cleared or there are
        // no more contexts.
        if (_operationContextTimeoutThread.run() != PEGASUS_THREAD_OK)
        {
            MessageLoaderParms parms(
                "Server.EnumerationContextTable.THREAD_ERROR",
                "Failed to start pull operation timer thread.");

            Logger::put_l(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
                parms);
            PEGASUS_DEBUG_ASSERT(false); // This is system failure
        }

        _timeoutThreadRunningFlag++;
    }
    PEG_METHOD_EXIT();
}

void EnumerationContextTable::_stopTimeoutThread()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContextTable::_stopTimeoutThread");

    PEGASUS_DEBUG_ASSERT(valid());

    if (_timeoutThreadRunningFlag.get() != 0)
    {
        _stopTimeoutThreadFlag++;
        _timeoutThreadWaitSemaphore.signal();

        while (_timeoutThreadRunningFlag.get())
        {
            Threads::yield();
            Threads::sleep(PEGASUS_PULL_MAX_OPERATION_WAIT_SEC);
        }
    }
    PEG_TRACE_CSTRING(TRC_ENUMCONTEXT, Tracer::LEVEL4,
        "EnumerationContextTable timeout thread stopped");
    PEG_METHOD_EXIT();
}


void EnumerationContextTable::setDefaultOperationTimeoutSec(Uint32 seconds)
{
#ifndef PEGASUS_INTEGERS_BOUNDARY_ALIGNED
    AutoMutex lock(_defaultOperationTimeoutSecMutex);
#endif
    _defaultOperationTimeoutSec = seconds;
}


void EnumerationContextTable::trace()
{
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
        "EnumerationContextTable Trace. size=%u", _enumContextTable.size()));

    AutoMutex autoMut(_tableLock);

    for (EnumContextTableType::Iterator i = _enumContextTable.start(); i; i++)
    {
        PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
            "ContextTable Entry: key [%s]",
                   (const char*)i.key().getCString() ));
        EnumerationContext* enumeration = i.value();
        enumeration->trace();
    }
}

PEGASUS_NAMESPACE_END
