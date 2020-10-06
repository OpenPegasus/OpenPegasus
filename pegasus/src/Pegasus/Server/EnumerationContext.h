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

#ifndef PegasusEnumerationContext_h
#define PegasusEnumerationContext_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/UintArgs.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Magic.h>
#include <Pegasus/Common/Condition.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Server/EnumerationContextTable.h>

PEGASUS_NAMESPACE_BEGIN

#define LOCAL_MIN(a, b) ((a < b) ? a : b)

/******************************************************************************
**
**    Class that caches each outstanding enumeration sequence. Contains
**    the parameters and current status of existing enumerations
**    that the server is processing.  Enumerations are those
**    sequences of operations starting with an Open... operation
**    and proceeding through Pull... and possible close Enumeration
**    operations.  The enumerationContext is the glue information
**    that ties the sequence of operations together.  This struct
**    defines the information that is maintained througout the
**    life of the sequence.
**    This structure also contains the queue of CIMOperationData objects
**    that is fed from provider returns and accessed by the operation requests
**
******************************************************************************/

/*
    Keep total and average statistics on a 32bit integer. Used by
    enumeration to keep statistics on info in the enumeration.
*/
class PEGASUS_SERVER_LINKAGE Uint32Stats
{
public:
    Uint32Stats();
    void reset();
    // Add an entry to the statistics
    void add(Uint32 newInfo);
    // get the various pieces of intformation.
    Uint32 getHighWaterMark();
    Uint32 getAverage();
    Uint32 getCounter();
private:
    Uint32 _highWaterMark;
    Uint32 _counter;
    Uint32 _average;
    Uint64 _total;
    bool _overflow;
};

/*
    Controls the EnumerationContext for pull operations.  An instance
    of this class is the controller for the sequence of operations representing
    a pull sequence from open to completion. This instance provides the
    tools for maintaining interoperation information including:
      - the cache of objects to be returned as received from providers,
      - request information that is used by pull and close operations
        (timers, open request parameters, etc.),
      - and the state of the pull operation sequence.

    The key variables are:
     - client state(clientClosed)  - whether the client is open or has been
       closed
     - providersComplete - Set true when last provider response received.
     - The responseData cache where responses are stored when received
       from providers.
     - contextID - The name for this context which is the client
       and EnumerationContextTable identification for the sequence.
     - Processing which is the active.inactive state where the context
       is active any time it is processing a client response
     - interoperationTimer - set non-zero with a timeout value when the
       processing state is inactive.
*/

class EnumerationContextTable;

class PEGASUS_SERVER_LINKAGE EnumerationContext
{
public:

    /** Construct a single instance of Enumeration Context. This is
    to be used only from the EnumerationContextTable CreateContext function.

    @param contextId String identifier for this context. These must
    be unique.

    @param nameSpace CIMNamespaceName for this operation and
    therefore or this enumeration sequence.

    @param operationTimeoutSec - Number of seconds for the interoperation
    timeout for this pull sequence.  Set with the open and
    used by the startTimer to set the timeout value between operations

    @param continueOnError - Boolean Flag from open request

    @param interoperationTimeout - microsecond timer that defines
    the next timeout.  If zero,there is no timeout in process

    @param pullRequestType MessageType for all pull requests for
    this sequence

    @param  contentType ResponseDataContent Type used to define the
    responseData cache for this context.

    */
    EnumerationContext(const String& contextId,
        const CIMNamespaceName& nameSpace,
        Uint32 interOperationTimeoutValue,
        Boolean continueOnError,
        MessageType pullRequestType,
        CIMResponseData::ResponseDataContent contentType);

    ~EnumerationContext();

    /**
       Get the ContextId of this enumeration context. The ContextId
       is the key to access the context entry in the enumeration
       context table.
       @return String containing the contextId.
     */
    String& getContextId();

    /**
        Set the request properties that must be moved to subsequent
        pull operations.
    */
    void setRequestProperties(
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList);

    /** Start the interOperation timer for this context. Uses the
     *  interoperation timeout value as basis for setting timeout
     *  // KS_TODO Should these be private?? start and stop timer
    */
    void startTimer();

    /* Start timer with the defined timeout*/
    void startTimer(Uint64 timeoutUsec);

    /** Stop the interOperation timer for this context
    */
    void stopTimer();

    /** Test if this context timed out given the current time
        @param currentTime
        @return true if interoperation timer timed out
    */
    bool isTimedOut(Uint64 currentTime);

    /** Test if this context timed out. Gets current time and tests
        against the timeout in the enumeration context entry

        @return true if interoperation timer timed out
    */
    bool isTimedOut();

    // diagnostic tests magic number in context to see if context valid
    bool valid() const;

    /** Test if client is closed for this enumeration.
       @return bool true if closed
    */
    bool isClientClosed();

    /** Test if this enumeration context has received an error response
       from providers, etc.
       @return true if errors received and not processed.
    */
    bool isErrorState();

    /**Set the error state flag and set the current cimException
       into the context object. This indicates that an exception
       was received side.
        @param cimException
     */
    void setErrorState(CIMException cimException);

    /** Test the Message type for the open message saved in the
        context against the type parameter.  This provides a test
        that insures that Pull message types match the open type.
        Ex. PullPaths can only be used with OpenPath contexts
        @param type MessageType for (for pull operation)
        @return Returns true if type match is correct. Returns false
        if not correct type.
     */
    bool isValidPullRequestType(MessageType type) const;

    /** Test context to determine if it is active (i.e an operation
        is in process in the CIMOperationRequestDispatcher.
    */
    bool isProcessing();

    /**Put the CIMResponseData from the response message into the
       enumerationContext cache and if providersComplete is true,
       set the enumeration state to providersComplete. This function
       signals the getCache function conditional variable. This
       function may also wait if the cache is full.

       @param response CIMResponseMessage containing CIMResponseData
          to be inserted into the cache.
       @param providersComplete bool indicating that this is the
          last response from providers.
       @return true if data set into cache, false if enumeration
               closed this is last response from providers.
     */
    bool putCache(CIMResponseMessage*& response,
        Boolean providersComplete);

    /** Wait for the cache to drop below defined size before
        responding to the provider.
    */
    void waitCacheSize();

    /**Get up to the number of objects defined by count from the
       CIMResponseData cache in the enumerationContext into the rtn
       CIMResponseData object. This function waits for a number of
       possible events as defined below and returns only when one of
       these events is true.
       This function also executes a ProviderLimitCondition signal
       before returning to tell the ProviderLimit condition variable
       that the size of the cache may have changed.

       @param count Uint32 count of max number of objects to return
       @param rtnData CIMResponseData containing the
           objects returned. Count of objects le count argument
       @return true if data acquired from cache. False if error from
       providers encountered
     */
    bool getCache(Uint32 count, CIMResponseData& rtnData);

    /** Test cache to see if there are responses that could be used
        for an immediate response. Returns immediatly with true or false
        indicating that a response should be issued.  The algorithm for
        the response is
             if request is for zero objects
                return true
            If requiresAll
               return true if
                    cache has enough objects for response (ge count) ||
                    the error flag is set ||
                    the providersComplete flag is set
            else
                return true if
                    Cache has some objects int (do not return zero objects) ||
                    the error flag is set ||
                    the providersComplete flag is set
        @param count Uint32 count of objects that the requests set as
        max number for response

        @param return parameter indicating that the errorFlag is set for this
        enumeration.
        @return True if passes tests for something to send or error flag
        set.
    */
    bool testCacheForResponses(Uint32 operationMaxObjectCount,
                                  Boolean requiresAll);

    /* Setup the request and response information for a response to
        be issued later as part of putting provider info into the
        cache. This saves the request, response, and count
        information and starts the timer so that the delayed
        response will be issued after a defined time if there are no
        provider events

        @param request - The CIMOperationRequestMessage for the
        response that will be issued later. This could be either a
        pull or open response

        @param response - The CIMOpenOrPullResponseMessage that will
        be issued at a later time. Required by the function that
        issues the response.

        @param operationMaxObjecCount Uint32 that defines the
        maximum number of objects to be returned.
    */
    void setupDelayedResponse(CIMOperationRequestMessage* request,
         CIMOpenOrPullResponseDataMessage* response,
         Uint32 operationMaxObjectCount);

    /** Returns count of objects in the EnumerationContext CIMResponseData
        cache.
        @return  Uint32 count of objects currently in the cache
    */
    Uint32 responseCacheSize();

     /**
        Set the ProvidersComplete state.  This should be set from provider
        responses when all responses processed.
    */
    void setProvidersComplete();

    /**
        Closed the client side of the EnumerationContext. From this
        point on, any client side requests should be rejected. Note
        that the providers may still be delivering CIMResponseData
        to the enumerationContextQueue. The
        CIMOperationRequestDispatcher uses the closed state to
        refuse pull/close operations. Once the EnumerationContext is
        closed, it may be removed from the enumeration context table
        (normally this happens when closed and providersComplete are
        set).
    */
    void setClientClosed();

    /**
        Sets the active state (i.e. Request being processed).
        Setting processing = true stops the interOperation timer.
        Otherwise the interoperation timer is started
        @param state bool defines whether to set processing or
        !processing. Processing means request being processed.
     */
    void setProcessingState(bool state);

    const char* processingState();

    /**
        Test if the provider processing is complete.
        @return true if provider processing is complete.
     */
    bool providersComplete() const ;

    /**
        Called by the Dispatcher Client operation when the
        processing of a Request is complete, this function
        sets the next state for the operation,
        either back to wait for the next operation or complete.
         @param errorFound bool indicating that an error was
         encountered which, if continueOnError = false, forces
         the operation to be closed and the true response returned.
        @return Boolean true if the enumeration is complete.
    */
    bool setNextEnumerationState(bool errorFound);

    /**
        Increment the count of the number of pull operations executed
        for this context. This method also controls the counting
        of operations with zero length through the input parameter.
        The zero length counter is reset for each call with the
        input parameter != zero so that this function counts total
        operations and also counts consecutive maxObjectCount zero
        length requests.

        @param isZeroLength bool indicating if this operation is a
        request for zero objects which is used to count consecutive
        zero length pull operations.
        @return true if the count of consecutive zero length pull operations
        exceeds a predefined maximum.
    */
    bool incAndTestPullCounters(bool isZeroLength);


    /** Diagnostic to display the current context into the trace
        file  KS_TODO eliminate or compile in debug mode only
    */
    void trace();

    //
    //  EnumeratonContext Data
    //

    // Exception placed here in case of error. This is set by the operation
    // aggregate with any CIMException recieved from providers.  Note that
    // Only one is allowed.
    // NO  multiple errors until we get continueOnError
    // or really get way to return multiple  errors.
    CIMException _cimException;

    // This mutex locks the entire Enumeration context for some
    // critical sections. the following function control this
    // mutex
    void lockContext();
    void unlockContext();
    bool tryLockContext();

    Mutex _contextLock;

    /* Increment count of requests processed for this enumeration
    */
    void incrementRequestCount();

    void setContinueOnError(bool x);

    // get the namespace stored during the consruction
    CIMNamespaceName& getNamespace();

    // Parameters for requests saved for future send.
    //
    CIMOperationRequestMessage* _savedRequest;
    CIMOpenOrPullResponseDataMessage* _savedResponse;
    Uint32 _savedOperationMaxObjectCount;

    // Pointer to Enumeration Table. Set during Create of enumeration
    // context
    EnumerationContextTable* _enumerationContextTable;

    /**
        Increment and return the zeroLenObjectResponseCounter which is a
        counter of the number of zero-len responses sent to the client
        consecetivily.  The counter is cleared each time a response is
        recevied from a provider
    */
    Uint32 incConsecutiveZeroLenObjectResponseCounter();
    void clearConsecutiveZeroLenObjectResponseCounter();

private:
    // Default constructor not used
    EnumerationContext();
    // hide assignment and copy constructor
    EnumerationContext(const EnumerationContext& x);
    EnumerationContext& operator=(const EnumerationContext&);

    friend class EnumerationContextTable;

    // Name(Id) of this EnumerationContext.
    String _contextId;

    // Namespace for this pull sequence.  Set by open and used by
    // pull and close.
    CIMNamespaceName _nameSpace;

    // Interoperation timeout value in seconds.  From open request operation
    // parameters.
    Uint32 _operationTimeoutSec;

    // ContinueOnError request flag.Set by open...
    Boolean _continueOnError;

    // Timeout absolute time value microseconds) for operation timeout.
    // 0 indicates  timer not active. Used to time both interoperation
    // timeouts (not processing) and timeouts when enumerationContext is
    // active (There is a response waiting).
    Uint64 _operationTimerUsec;

    // Request Type for pull operations for this pull sequence.
    // Set by open. All pulls must match this type.
    MessageType _pullRequestType;

    // Status flags.
    // Set true when context closed from client side
    bool _clientClosed;

    // Set to true when input from providers complete. The context
    // cannot be removed while this is false
    bool _providersComplete;

    // set true when CIMServer is processing a request within the
    // enumeration context
    bool _processing;

    // Set true when error received from Providers.
    bool _error;

    // Object cache for this context.  All pull responses feed their
    // CIMResponseData into this cache using putCache(..) and all
    // Open and Pull responses get data from the cache using getCache()
    // Simultaneous access to the cache is controlled with _responseCacheMutex
    // mutex.
    Mutex _responseCacheMutex;
    CIMResponseData _responseCache;

    // Condition variable and mutex for the provider wait
    // condition.  This is a hold on returns from putcache when cache
    // reaches a defined limit that is cleared when the cache level drops
    // to below the defined level
    Condition _providerWaitCondition;
    Mutex _providerWaitConditionMutex;

    // Signal the condition variable that it should test the
    // providerWaitCondition condition variable.
    void signalProviderWaitCondition();

    // Statistics for waiting.  Total for this context and max for wait
    Uint64 _totalWaitTimeUsec;
    Uint64 _maxWaitTimeUsec;

    // Count Of pull operations executed in this context.  Used for statistics
    Uint32 _pullOperationCounter;

    // Counter of consecutive requests with maxObjectCount == 0. Used to limit
    // client stalling by executing excessive number of zero return pull
    // operations.
    Uint32 _consecutiveZeroLenMaxObjectRequestCounter;

    // Counter of consecutive zero length  client responses sent by the
    // dispatcher. Used to  limit condition where providers never close
    // When this reaches a defined limit, a message is sent to the
    // OOPProviderAgent to close out the defined enumerationContext and
    // eventually the enumeration Context is just closed as a error.
    Uint32 _consecutiveZeroLenObjectResponseCounter;

    // Maximum number of objects that can be placed in the response Cache
    // before blocking providers.
    Uint32 _responseCacheMaximumSize;

    // number of requests for this EnumerationContext.
    Uint32 _requestCount;

    // Count of Objects actually returned
    Uint32 _responseObjectsCount;
    // Accumulation of count of request maxObjectCounts
    Uint32 _requestedResponseObjectsCount;

    // Counts all zero length responses sent to Client.  This is
    // for statistics. This is, in effect the number of times during
    // this enumeation that the providers responded so late that we
    // sent a zero response.  This is an indicator of provider overload,
    // or a  provider somewhere that is responding very slowly. The timers
    // are such that there should be at least one response from a provider
    // about each 15 seconds which is the current timer for this timeout.
    Uint32 _totalZeroLenObjectResponseCounter;

    // Enumeration startTime in microseconds
    Uint64 _startTimeUsec;
    // Max number of objects in the cache.
    Uint32 _cacheHighWaterMark;

    Magic<0x57D11474> _magic;
};

/******************************************************
   Inline functions
******************************************************/
inline  CIMNamespaceName& EnumerationContext::getNamespace()
{
    return _nameSpace;
}

inline String& EnumerationContext::getContextId()
{
    return _contextId;
}

inline void EnumerationContext::incrementRequestCount()
{
    _requestCount++;
}

inline bool EnumerationContext::isProcessing()
{
    return _processing;
}

inline bool EnumerationContext::isClientClosed()
{
    return _clientClosed;
}

inline bool EnumerationContext::isErrorState()
{
    return _error;
}

/*
    Test the current pull message against the type set on the create
    context. they must match
*/
inline bool EnumerationContext::isValidPullRequestType(
   MessageType type) const
{
    return(type == _pullRequestType);
}

inline bool EnumerationContext::providersComplete() const
{
    return _providersComplete;
}

inline Uint32 EnumerationContext::responseCacheSize()
{
    return _responseCache.size();
}

inline void EnumerationContext::lockContext() { _contextLock.lock(); }

inline bool EnumerationContext::tryLockContext()
{
    return _contextLock.try_lock();
}
inline void EnumerationContext::unlockContext() { _contextLock.unlock(); }
inline void EnumerationContext::setContinueOnError(Boolean x)
{
    _continueOnError = x;
}
// Increment both the consecutive and total counters and return the value
// of the consecutive counter.
inline Uint32 EnumerationContext::incConsecutiveZeroLenObjectResponseCounter()
{
    _totalZeroLenObjectResponseCounter++;
    return ++_consecutiveZeroLenObjectResponseCounter;
}
inline void EnumerationContext::clearConsecutiveZeroLenObjectResponseCounter()
{
    _consecutiveZeroLenObjectResponseCounter = 0;
}

PEGASUS_NAMESPACE_END

#endif /* PegasusEnumerationContext_h */
