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
//%///////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Async_RequestExecutor_h
#define Pegasus_Async_RequestExecutor_h

#include <Pegasus/Common/Condition.h>
#include <Pegasus/Common/ThreadPool.h>
#include <Pegasus/Common/CIMMessage.h>

#include <Pegasus/ProviderManager2/Linkage.h>
#include <iostream>

PEGASUS_NAMESPACE_BEGIN

/**
    A response callback class that can be used to wait on request handler 
    threads.The different request handler threads use the shared
    callback instance to signal completion which in turn aggregates the
    responses and finally signals upon reaching the expected responses 
    thus retuning the aggregated response
**/
class PEGASUS_COMMON_LINKAGE ResponseAggregationCallback
{
public:
    ResponseAggregationCallback();
    ~ResponseAggregationCallback();

    /*
        This function waits till the received response count reaches the 
        expected response count and return the aggregated response.
    */
    CIMException waitForCompletion(Uint32 expectedResponseCount);

    /* 
        This function incremet the response count and signals completion
        once the count reaches the excpected count.
    */
    void signalCompletion(CIMException& responseException);

private:
    CIMException _responseException;
    Uint32 _expectedResponseCount;
    Uint32 _currResponseCount;
    Mutex _mutex;
    Condition _pollingCond;
};

/**
    A multithreaded executor that processes the submitted requests on multiple
    threads using the thread pool by invoking the request handler per request
    and returns the aggregated response.
**/
class PEGASUS_COMMON_LINKAGE AsyncRequestExecutor
{
public:
    //base class representing the request parameter
    class AsyncRequestMsg
    {
     public:
         AsyncRequestMsg() {}
         virtual ~AsyncRequestMsg() {}
    };

    /**
    Input :
    1) Function pointer to the function that needs to be called on
       different threads.
    2) The pointer to an object which can be used in the 
       asyncRequestCallback function.
    3) The max number of threads which the Thread Pool can run 
       while executing the requests.
    **/    
    AsyncRequestExecutor(
        CIMException (*asyncRequestCallback)(void *, AsyncRequestMsg* ),
        void *callbackPtr,
        Uint32 numberOfThreads=5);

    ~AsyncRequestExecutor();

    //execute the  requests on multiple threads by invoking the
    //request handler and return the aggregated response.
    CIMException executeRequests(Array<AsyncRequestMsg*> requests);

private:

    static ThreadReturnType PEGASUS_THREAD_CDECL
        _requestProcessor(void* arg);


    CIMException (*_asyncRequestCallback)(void *callbackPtr, 
        AsyncRequestMsg* request);

    void *_callbackPtr;

    ThreadPool* _threadPool;

    ResponseAggregationCallback _responseCallback;

    /* 
        Structure used to pass a request to the executor consisting of a
        request handler and the request msg.
    */
    struct ReqThreadParam 
    {
    public:
        ReqThreadParam() {}

        ReqThreadParam( 
            CIMException (*asyncRequestCallback)(void *callbackPtr,
                AsyncRequestMsg *request),
            void *callbackPtr,
            AsyncRequestMsg* request,
            ResponseAggregationCallback* respCallback):
                _asyncRequestCallback(asyncRequestCallback),
                _callbackPtr(callbackPtr),
                _request(request),
                _responseCallback(respCallback) 
        {}

        ~ReqThreadParam() 
        { }

        CIMException (*_asyncRequestCallback)(void *,AsyncRequestMsg*);
        void *_callbackPtr;
        AsyncRequestMsg* _request;
        ResponseAggregationCallback*  _responseCallback;
    };
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Async_RequestExecutor_h */
