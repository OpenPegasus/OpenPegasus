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

#include "EnumerationContext.h"

#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/General/Guid.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/Condition.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/System.h>


PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

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

// defines conversion between sec and usec
#define PEG_MICROSEC 1000000

#define MAX_ZERO_PULL_OPERATIONS 1000

// When set enables the diagnostic traces in this class
// These should only be enabled for development or special testing
#define ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE


// Create a new context. This is called only from the enumerationTable
// createContext function.
EnumerationContext::EnumerationContext(const String& contextId,
    const CIMNamespaceName& nameSpace,
    Uint32 interOperationTimeoutValue,
    Boolean continueOnError,
    MessageType pullRequestType,
    CIMResponseData::ResponseDataContent contentType )
    :
    _cimException(CIMException()),
    _savedRequest(NULL),             // Clear because used as a flag
    _savedResponse(NULL),
    _contextId(contextId),
    _nameSpace(nameSpace),
    _operationTimeoutSec(interOperationTimeoutValue),
    _continueOnError(continueOnError),
    _operationTimerUsec(0),
    _pullRequestType(pullRequestType),
    _clientClosed(false),
    _providersComplete(false),
    _processing(true),    // set true because always created during processing
    _error(false),
    _responseCache(contentType),
    _providerWaitConditionMutex(Mutex::NON_RECURSIVE),
    _totalWaitTimeUsec(0),
    _maxWaitTimeUsec(0),
    _pullOperationCounter(0),
    _consecutiveZeroLenMaxObjectRequestCounter(0),
    _consecutiveZeroLenObjectResponseCounter(0),
    _responseCacheMaximumSize(0),
    _requestCount(1),
    _responseObjectsCount(0),
    _requestedResponseObjectsCount(0),
    _totalZeroLenObjectResponseCounter(0),
    _cacheHighWaterMark(0)
{
    PEGASUS_DEBUG_ASSERT(_responseCache.valid());

    // set start time for this enumeration sequence
    _startTimeUsec = System::getCurrentTimeUsec();

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL3,
        "Create EnumerationContext ContextId=%s operationTimeoutSec %u"
        " responseCacheDataType %u StartTime %lu",
        *Str(getContextId()),
        _operationTimeoutSec,
        _responseCache.getResponseDataContent(),
        (unsigned long int)_startTimeUsec));
#endif
}

void EnumerationContext::setRequestProperties(
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    // Set request properties into the responseCache that are required
    // later for pull operations. Not required for names operations
    // since the attributes defined characteristics of objects returned
    // (qualifiers, classorigin, propertylists).
    // Sets includeQualifiers == false since this attribute
    // not supported for pull operations.
    _responseCache.setRequestProperties(
        false, includeClassOrigin, propertyList);
}

/*
    Set the inter-operation timer for the timeout to the start of the
    next operation of this enumeration sequence. If the operationTimeout
    value = 0 do not set the timer.
*/
void EnumerationContext::startTimer()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,"EnumerationContext::startTimer");
    PEGASUS_DEBUG_ASSERT(valid());

    // Request operation timeout = 0 means do not start timer
    if (_operationTimeoutSec != 0)
    {
        startTimer(_operationTimeoutSec * PEG_MICROSEC);
    }
    PEG_METHOD_EXIT();
}

void EnumerationContext::startTimer(Uint64 timeoutUsec)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,"EnumerationContext::startTimer");

    PEGASUS_DEBUG_ASSERT(valid());

    Uint64 currentTime = System::getCurrentTimeUsec();

    _operationTimerUsec =  currentTime + timeoutUsec;

    _enumerationContextTable->dispatchTimerThread();

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
        "StartTimer, ContextId=%s, This timeoutTime(sec)=%llu"
           " OperationTimeout=%u sec,"
           " next timeout in %ld sec,",
       *Str(getContextId()),
       (timeoutUsec / PEG_MICROSEC),
       _operationTimeoutSec,
       (long signed int)(_operationTimerUsec - currentTime)/PEG_MICROSEC ));
#endif
    PEG_METHOD_EXIT();
}

void EnumerationContext::stopTimer()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,"EnumerationContext::stopTimer");

    PEGASUS_DEBUG_ASSERT(valid());

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
        "StopTimer ContextId=%s,"
           " OperationTimeout=%u sec,"
           " (timerTime - curtime)=%ld sec,",
       *Str(getContextId()), _operationTimeoutSec,
       (long signed int)(_operationTimerUsec -
           System::getCurrentTimeUsec())/PEG_MICROSEC ));
#endif
    _operationTimerUsec = 0;
    PEG_METHOD_EXIT();
}

/*
    Test interoperation timer against current time. Return true if timed out
    or timer set 0 zero indicating that the timer is not active.
    Returns bool true if timer not zero and Interoperation timer
    is greater than interoperation timeout (i.e timed out).
*/
bool EnumerationContext::isTimedOut(Uint64 currentTime)
{
    PEGASUS_DEBUG_ASSERT(valid());

    if (_operationTimerUsec == 0)
    {
            return false;
    }

    bool timedOut = _operationTimerUsec <= currentTime;

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
        "isTimedOut Timer. ContextId=%s timer(sec)=%lu"
           " current(sec)=%lu time to timeout(usec)=%ld isTimedOut=%s",
        *Str(getContextId()),
        (long unsigned int)(_operationTimerUsec / PEG_MICROSEC),
        (long unsigned int)(currentTime / PEG_MICROSEC),
        (long signed int)((_operationTimerUsec - currentTime)),
        boolToString(timedOut) ));
#endif
    // If it is timed out, set timer inactive.
    if (timedOut)
    {
        _operationTimerUsec = 0;
    }
    return(timedOut);
}

bool EnumerationContext::isTimedOut()
{
    Uint64 currentTime = System::getCurrentTimeUsec();
    return isTimedOut(currentTime);
}

// FUTURE: In future consider list of exceptions since there may be
// multiples.  For the moment, last error wins.
void EnumerationContext::setErrorState(CIMException x)
{
    PEGASUS_DEBUG_ASSERT(valid());
    // Until we handle multiple errors, return only the first error
    if (_error)
    {
        return;
    }
    // Set exception first and use flag as indicator to avoid ipc issues.
    _cimException = x;
    _error = true;
}

//// KS_FUTURE make DEBUG compile only
// Diagnostic display of data in the enumeration context object
void EnumerationContext::trace()
{
    PEGASUS_DEBUG_ASSERT(valid());
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL3,
        "EnumerationContextTrace ContextId=%s "
        "requestOperationTimeOut=%u "
        "operationTimer=%lu sec "
        "continueOnError=%s "
        "pullMsgType=%s "
        "processingState=%s "
        "providersComplete=%s "
        "closed=%s "
        "timeOpen=%lu ms "
        "totalPullCount=%u "
        "cacheHighWaterMark=%u "
        "Request count=%u "
        "ResponseObjectCount=%u "
        "totalWaitTimeUsec=%llu "
        "maxWaitTimeUsec=%llu "
        "RequestedResponseObjectCount=%u "
        "consecutiveZeroLenObjectResponseCtr=%u "
        "totalZeroLenObjectResponseCounter=%u"
        "ResponseCacheSize=%u",
        *Str(getContextId()),
        _operationTimeoutSec,
        (long unsigned int)_operationTimerUsec,
        boolToString(_continueOnError),
        MessageTypeToString(_pullRequestType),
        processingState(),
        boolToString(_providersComplete),
        boolToString(_clientClosed),
        (long unsigned int)
            (System::getCurrentTimeUsec() - _startTimeUsec)/1000,
        _pullOperationCounter,
        _cacheHighWaterMark,
        _requestCount,
        _responseObjectsCount,
        _totalWaitTimeUsec,
        _maxWaitTimeUsec,
        _requestedResponseObjectsCount,
        _consecutiveZeroLenObjectResponseCounter,
        _totalZeroLenObjectResponseCounter,
         responseCacheSize()));
}

/**
 * validate the magic object for this context
 *
 * @return bool True if valid object.
 */
bool EnumerationContext::valid() const
{
    _responseCache.valid();
    return _magic;
}

EnumerationContext::~EnumerationContext()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContext::~EnumerationContext()");

    PEG_METHOD_EXIT();
}

/*
    Insert complete CIMResponseData entities into the cache. If the
    cache is at its max size limit, and there are more provider responses
    wait until it the size drops below the full limit.
    If the operation is closed, ignore the response.
    Return true if putCache worked, false if closed and nothing put into
    the cache.
    NOTE: This function assumes that responses for a request are serialized
    in _enqueueResponse See _enqueueResponseMutex.
*/
bool EnumerationContext::putCache(CIMResponseMessage*& response,
    bool providersComplete)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT, "EnumerationContext::putCache");

    PEGASUS_DEBUG_ASSERT(valid());

    // Design error if we ever get here with providers already set complete
    PEGASUS_DEBUG_ASSERT(!_providersComplete);

    CIMResponseDataMessage* localResponse =
        dynamic_cast<CIMResponseDataMessage*>(response);
    CIMResponseData & from = localResponse->getResponseData();
    //// from.traceResponseData();

    // If there is any binary data, reformat it to SCMO.  There are no
    // size counters for the binary data so reformat to generate
    // counters and make it compatible with the cache access mechanisms
    if (from.hasBinaryData())
    {
        from.resolveBinaryToSCMO();
        //// from.traceResponseData();
    }

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
        "putCache, ContextId=%s isComplete=%s cacheResponseDataType=%u "
            " cacheSize=%u putSize=%u putResponseDataType=%u clientClosed=%s",
        *Str(getContextId()),
        boolToString(providersComplete),
        _responseCache.getResponseDataContent(),
        _responseCache.size(), from.size(), from.getResponseDataContent(),
        boolToString(_clientClosed)));
#endif

    // This test should not be required.  Somewhere in the processing there
    // is a rare return of an erronous response from OOP. This covers that
    // case until we find the issue.  It issues an error a discard trace
    if (from.getResponseDataContent()!=_responseCache.getResponseDataContent())
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL2,
            "Pull Provider Response DataContentType in error. cacheType=%u "
            "responseType=%u "
            "ResponseMsgType=%s ContextId=%s",
            _responseCache.getResponseDataContent(),
            from.getResponseDataContent(),
            MessageTypeToString(response->getType()),
            *Str(getContextId()) ));
        trace();
        // This is temp for testing. KS_TODO delete this console display
        //// cout << System::getCurrentASCIITime()
        ////      << "Error CIMResponseDataMismatch "
        ////      << getContextId() << endl;

        CIMException sysErr = CIMException(CIM_ERR_FAILED,
            "Internal Error in EnumerationContext processing");
        setErrorState(sysErr);
        // Output warning log to indicate that this system failure has occurred

        Logger::put(
                Logger::ERROR_LOG, System::CIMSERVER, Logger::WARNING,
                "Response msg data type mismatch from providers."
                "Internal Error in EnumerationContext processing. "
                " ContextId=", *Str(getContextId() ));
        return _clientClosed;

    }
    // If an operation has closed the enumerationContext
    // ignore any received responses until the providersComplete is received
    // and then remove the Context.
    if (_clientClosed)
    {
        PEG_METHOD_EXIT();
        return false;
    }
    else  // client not closed
    {
        // put the current response into the cache. Lock cache for this
        // operation

        _responseCache.appendResponseData(from);

        // set providersComplete flag from flag in call parameter.
        _providersComplete = providersComplete;

        // test and set the high water mark for this cache.
        if (responseCacheSize() > _cacheHighWaterMark)
        {
            _cacheHighWaterMark = responseCacheSize();
        }
    }

    // Return true indicating that input added to cache and cache is still open
    PEG_METHOD_EXIT();
    return true;
}

// Wait until cache size drops below defined limit. Saves time
// in wait in EnumerationContext for statistics and uses
// waitProviderLimitCondition condition variable.
void EnumerationContext::waitCacheSize()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT, "EnumerationContext::waitCacheSize()");

    PEGASUS_DEBUG_ASSERT(valid());

    _providerWaitConditionMutex.lock();

    Uint64 startTime = System::getCurrentTimeUsec();

    while ((!_clientClosed) && (responseCacheSize() > _responseCacheMaximumSize)
           && !_providersComplete)
    {
        _providerWaitCondition.wait(_providerWaitConditionMutex);
    }

    _providerWaitConditionMutex.unlock();

    Uint64 interval = System::getCurrentTimeUsec() - startTime;

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
        "waitCacheSize  ContextId=%s Wait=%lu usec",
       *Str(getContextId()), (unsigned long int)interval ));
#endif

    _totalWaitTimeUsec += interval;
    if (interval > _maxWaitTimeUsec)
    {
        _maxWaitTimeUsec = interval;
    }
    PEG_METHOD_EXIT();
}


/*****************************************************************************
**
**     Methods to support the EnumerationContext CIMResponseData Cache
**
*****************************************************************************/

/*
    Test the cache to see if there is information that could be used
    for an immediate response. Returns immediatly with true or false
    indicating that a response should be issued.
    @param count Uint32 count of objects that the requests set as
    max number for response
    @return True if passes tests for something to send or error flag
    set.
*/
bool EnumerationContext::testCacheForResponses(
    Uint32 operationMaxObjectCount,
    bool requiresAll)
{
    bool rtn = false;

    // Error encountered, must send response. This makes error highest
    // priority.
    if (isErrorState())
    {
        rtn = true;
    }
    // Always allow requests for no objects
    else if (operationMaxObjectCount == 0)
    {
        rtn = true;
    }
    // If cache has enough objects return true
    else if (requiresAll && (responseCacheSize() >= operationMaxObjectCount))
    {
        rtn = true;
    }
    // anything in cache to return
    else if (!requiresAll && responseCacheSize() > 0)
    {
        rtn = true;
    }
    // Nothing more from providers. Must return response
    else if (providersComplete())
    {
        rtn = true;
    }

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
       "testCacheForResponse returns %s for ContextId=%s",
               boolToString(rtn), *Str(getContextId()) ));
#endif
    return rtn;
}

void EnumerationContext::setupDelayedResponse(
    CIMOperationRequestMessage* request,
    CIMOpenOrPullResponseDataMessage* response,
    Uint32 operationMaxObjectCount)
{
    // Since _savedRequest is also flag, it MUST BE empty when this function
    // called.
    PEGASUS_DEBUG_ASSERT(_savedRequest == NULL);

    _savedOperationMaxObjectCount = operationMaxObjectCount;
    _savedResponse = response;
    _savedRequest = request;

    // Start the waiting timeout for this delayed response.
    // At end of this timer, it will send empty response.
    startTimer(PEGASUS_PULL_MAX_OPERATION_WAIT_SEC * PEG_MICROSEC);
}
/*
    Move the number of objects defined by count from the CIMResponseData
    cache for this EnumerationContext to theCIMResponseData object
    defined by the input parameter.

    Returns true if data acquired from cache. Returns false if CIMException
    found (i.e. returned an error).
*/
bool EnumerationContext::getCache(Uint32 count, CIMResponseData& rtnData)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT, "EnumerationContext::getCache");

    PEGASUS_DEBUG_ASSERT(valid());

    // Move attributes from Cache to new CIMResponseData object
    // sets the attributes for propertyList, includeQualifiers,
    // classOrigin
    rtnData.setResponseAttributes(_responseCache);

    // if Error set, return false to signal the error to caller.
    if (isErrorState())
    {
        PEG_METHOD_EXIT();
        return false;
    }

    // Move the defined number of objects from the cache to the return object.
    rtnData.moveObjects(_responseCache, count);

    // add to statistics for this enumerationContext. Counts objects actuallly
    // sent.
    _responseObjectsCount += rtnData.size();
    // Accumulation of count of request maxObjectCounts
    _requestedResponseObjectsCount += count;

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
      "EnumerationContext::getCache ContextId=%s moveObjects expected=%u"
          " actual=%u", *Str(getContextId()), count, rtnData.size()));
#endif

    // Signal the ProviderLimitCondition that the cache size may
    // have changed.
    signalProviderWaitCondition();

    PEG_METHOD_EXIT();
    return true;
}

void EnumerationContext::signalProviderWaitCondition()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContext::signalProviderLimitCondition");

    PEGASUS_DEBUG_ASSERT(valid());

    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
      "EnumerationContext::signalProviderWait Condition ContextId=%s"
          " cacheSize=%u", *Str(getContextId()), responseCacheSize() ));

    AutoMutex autoMut(_providerWaitConditionMutex);

    _providerWaitCondition.signal();

    PEG_METHOD_EXIT();
}

// Update counters for Pull and test for too many consecutive zero
// length requests.
// return true if too many. Else return false.
bool EnumerationContext::incAndTestPullCounters(bool isZeroLength)
{
    PEGASUS_DEBUG_ASSERT(valid());

    _pullOperationCounter++;

    if (isZeroLength)
    {
        _consecutiveZeroLenMaxObjectRequestCounter++;
    }
    else
    {
        _consecutiveZeroLenMaxObjectRequestCounter = 0;
    }
    return (_consecutiveZeroLenMaxObjectRequestCounter >
             MAX_ZERO_PULL_OPERATIONS);
}

// set providers complete flag and signal the CacheSizeCondition.
// This could awaken any wait at the cacheWait.
void EnumerationContext::setProvidersComplete()
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContext::setProvidersComplete");

    PEGASUS_DEBUG_ASSERT(valid());

    _providersComplete = true;

    PEG_METHOD_EXIT();
}

// End of Request operation processing. Set the next enumeration state.
// If providers Complete and  cache = 0. We can now close the enumeration.
// If no more from providers and no more in cache, we set the client closed
//
// Returns true if there is no more to process (providers are complete and
// responseCacheSize = 0). Returns false if providers not complete or
// there is data in the cache

bool EnumerationContext::setNextEnumerationState(bool errorFound)
{
    PEG_METHOD_ENTER(TRC_ENUMCONTEXT,
        "EnumerationContext::setNextEnumerationState");

    PEGASUS_DEBUG_ASSERT(valid());

    // Return true if client closed because of error or all responses complete,
    // else set ProcessingState false and return false
    if ((providersComplete() && (responseCacheSize() == 0)) ||
        (errorFound && !_continueOnError))
    {
        setClientClosed();
        return true;
    }

    // Otherwise, set processing state to inactive and start operation
    // timer
    setProcessingState(false);

    PEG_METHOD_EXIT();
    return false;
}

void EnumerationContext::setClientClosed()
{
    PEGASUS_DEBUG_ASSERT(valid());

    _clientClosed = true;
    _processing = false;

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL3,
        "setClientClosed. ContextId=%s ", *Str(getContextId()) ));
#endif

    // Clear any existing responses out of the cache.  They will never
    // be used.
    _responseCache.clear();

    if (!_providersComplete)
    {
        // Signal that cache size has dropped.
        signalProviderWaitCondition();
    }

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    trace();
#endif
}

const char* EnumerationContext::processingState()
{
    static const char * active = "active";
    static const char * inactive = "inactive";
    return (_processing? active: inactive);
}

/*
    Set the processing state. Processing is true if the Dispatcher is
    actively handling a request. The dispatcher sets processing = true
    at the start of processing and false at the completion of processing.
*/
void EnumerationContext::setProcessingState(bool state)
{
    // Diagnostic to confirm we are changing state
    PEGASUS_DEBUG_ASSERT(valid());
    PEGASUS_DEBUG_ASSERT(_processing != state); // not setting to same state

#ifdef ENUMERATION_CONTEXT_DIAGNOSTIC_TRACE
    PEG_TRACE((TRC_ENUMCONTEXT, Tracer::LEVEL4,
        "setProcessingState. ContextId=%s nextProcessingStat=%s",
        *Str(getContextId()), (state? "active" : "inactive") ));
#endif

    _processing = state;
    if (_processing)
    {
        stopTimer();
    }
    else
    {
        startTimer();
    }
}

PEGASUS_NAMESPACE_END
