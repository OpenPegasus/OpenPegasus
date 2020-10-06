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

#ifndef Pegasus_AsyncOpNode_h
#define Pegasus_AsyncOpNode_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Linkable.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/Thread.h>

PEGASUS_NAMESPACE_BEGIN

#define ASYNC_OPFLAGS_UNKNOWN           0x00000000
#define ASYNC_OPFLAGS_FIRE_AND_FORGET   0x00000001
#define ASYNC_OPFLAGS_CALLBACK          0x00000002
#define ASYNC_OPFLAGS_PSEUDO_CALLBACK   0x00000004

#define ASYNC_OPSTATE_UNKNOWN           0x00000000
#define ASYNC_OPSTATE_COMPLETE          0x00000001

class PEGASUS_COMMON_LINKAGE AsyncOpNode : public Linkable
{
public:

    AsyncOpNode();
    ~AsyncOpNode();

    void setRequest(Message* request);
    Message* getRequest();
    Message* removeRequest();

    void setResponse(Message* response);
    Message* getResponse();
    Message* removeResponse();
    void complete();
    Uint32 getState();
private:
    AsyncOpNode(const AsyncOpNode&);
    AsyncOpNode& operator=(const AsyncOpNode&);

    Semaphore _client_sem;
    AutoPtr<Message> _request;
    AutoPtr<Message> _response;

    Uint32 _state;
    Uint32 _flags;
    MessageQueue *_op_dest;

    void (*_async_callback)(AsyncOpNode *, MessageQueue *, void *);

    // pointers for async callbacks  - don't use
    AsyncOpNode *_callback_node;
    MessageQueue *_callback_response_q;
    void *_callback_ptr;

    friend class cimom;
    friend class MessageQueueService;
    friend class ProviderManagerService;
};


inline void AsyncOpNode::setRequest(Message* request)
{
    PEGASUS_ASSERT(_request.get() == 0);
    PEGASUS_ASSERT(request != 0);
    _request.reset(request);
}

inline Message* AsyncOpNode::getRequest()
{
    PEGASUS_ASSERT(_request.get() != 0);
    return _request.get();
}

inline Message* AsyncOpNode::removeRequest()
{
    PEGASUS_ASSERT(_request.get() != 0);
    Message* request = _request.get();
    _request.release();
    return request;
}

inline void AsyncOpNode::setResponse(Message* response)
{
    PEGASUS_ASSERT(_response.get() == 0);
    PEGASUS_ASSERT(response != 0);
    _response.reset(response);
}

inline Message* AsyncOpNode::getResponse()
{
    PEGASUS_ASSERT(_response.get() != 0);
    return _response.get();
}

inline Message* AsyncOpNode::removeResponse()
{
    PEGASUS_ASSERT(_response.get() != 0);
    Message* response = _response.get();
    _response.release();
    return response;
}

inline void AsyncOpNode::complete()
{
    _state = ASYNC_OPSTATE_COMPLETE;
}

inline Uint32 AsyncOpNode::getState()
{
    return _state;
}

PEGASUS_NAMESPACE_END

#endif //Pegasus_AsyncOpNode_h
