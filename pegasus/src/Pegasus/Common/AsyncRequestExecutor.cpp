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

#include "AsyncRequestExecutor.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Constants.h>

PEGASUS_NAMESPACE_BEGIN

static struct timeval deallocateWait = {300, 0};

ResponseAggregationCallback::ResponseAggregationCallback()
:
    _responseException(CIM_ERR_SUCCESS),
    _expectedResponseCount(0),
    _currResponseCount(0),
    _mutex(Mutex::NON_RECURSIVE),
    _pollingCond()
{

}

ResponseAggregationCallback::~ResponseAggregationCallback()
{
}

CIMException ResponseAggregationCallback::waitForCompletion(
    Uint32 expectedResponses)
{
    AutoMutex lock(_mutex);
    _expectedResponseCount = expectedResponses;
  
    //This method is called only after all request threads have been 
    //allocated and hence the _currResponseCount might have already 
    //reached the expected count.Then dont wait.
    while(_currResponseCount != _expectedResponseCount)
    {
        _pollingCond.wait(_mutex);
    }
    return _responseException;
}

void ResponseAggregationCallback::signalCompletion(
    CIMException& responseException)
{
    AutoMutex lock(_mutex);
    _currResponseCount++;
    if(_responseException.getCode() != CIM_ERR_SUCCESS)
    { 
        _responseException = responseException;
    }

    //The _expectedResponseCount is set only after all threads have been 
    //allocated.The check here ensures signalling happens only AFTER 
    //expected response count is set AND the recieved response count
    //reaches the expected count.
    if(_expectedResponseCount &&
        _currResponseCount == _expectedResponseCount)
    {
        _pollingCond.signal();
    }
}

AsyncRequestExecutor::AsyncRequestExecutor(
    CIMException (*asyncRequestCallback)(void *callbackPtr, 
        AsyncRequestMsg* request),
    void *callbackPtr,
    Uint32 numberOfThreads) :
        _asyncRequestCallback(asyncRequestCallback),
        _callbackPtr(callbackPtr),
        _threadPool(new ThreadPool(
            numberOfThreads,
                "AsyncRequestExecutorThreadPool", 0,
            numberOfThreads, deallocateWait)),
        _responseCallback()
{
}

AsyncRequestExecutor::~AsyncRequestExecutor()
{
    delete _threadPool;
}

CIMException AsyncRequestExecutor::executeRequests(
    Array<AsyncRequestMsg*> requests)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
            "AsyncRequestExecutor::executeRequests()");

   //if there is just one request process it on current thread itelf.
    if(requests.size() == 1)
    {
        return _asyncRequestCallback(_callbackPtr,requests[0]);
    }

    CIMException responseException;
    ThreadStatus rtn = PEGASUS_THREAD_OK;
    Uint32 expectedResponses = 0;

    bool isExit=false;

    PEGASUS_ASSERT(_threadPool);
    for (Uint32 j = 0; j < requests.size(); j++)
    {
        ReqThreadParam *requestThreadParam = new ReqThreadParam(
            _asyncRequestCallback,
            _callbackPtr,
            requests[j],
            &_responseCallback);

        //set the callback
        while((rtn = _threadPool->
            allocate_and_awaken(requestThreadParam, _requestProcessor))
            != PEGASUS_THREAD_OK)
        {
            if (rtn == PEGASUS_THREAD_INSUFFICIENT_RESOURCES)
            {
                Threads::yield();
            }
            else
            {
                PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL1,
                    "Failed to allocate a thread for processing a request."));

                responseException = PEGASUS_CIM_EXCEPTION_L(
                    CIM_ERR_FAILED,
                    MessageLoaderParms(
                        "Common.AsyncRequestExecutor."
                            "ASYNCREQUEST_THREAD_ALLOCATION_FAILED",
                        "Failed to allocate a thread for processing "
                            "a request."));
                isExit = true;
                break;
            }
        }
        if(isExit) break;
        expectedResponses++;
     }

     CIMException respException =
         _responseCallback.waitForCompletion(expectedResponses);

     //check if there was any exception already during 
     //thread allocation. 
     if(responseException.getCode() == CIM_ERR_SUCCESS)
     {
         responseException = respException;
     }

     PEG_METHOD_EXIT();
     return responseException;
}

ThreadReturnType PEGASUS_THREAD_CDECL
    AsyncRequestExecutor::_requestProcessor(void* arg)
{
    CIMException response;
    ReqThreadParam* req = reinterpret_cast<ReqThreadParam*> (arg);
    response = req->_asyncRequestCallback(req->_callbackPtr,
        req->_request);
    req->_responseCallback->signalCompletion(response);

    delete req;

    return ThreadReturnType(0);
}

PEGASUS_NAMESPACE_END
