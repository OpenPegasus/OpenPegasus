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

#include "CimomMessage.h"

PEGASUS_NAMESPACE_BEGIN

const Uint32 async_results::OK =                        0x00000000;
const Uint32 async_results::CIM_NAK =                   0x00000001;
const Uint32 async_results::CIM_SERVICE_STOPPED =       0x00000002;


AsyncMessage::AsyncMessage(
    MessageType type,
    Uint32 destination,
    Uint32 mask,
    AsyncOpNode* operation)
    : Message(type, destination, mask | MessageMask::ha_async),
      op(operation)
{
}

AsyncRequest::AsyncRequest(
    MessageType type,
    Uint32 mask,
    AsyncOpNode* operation,
    Uint32 destination)
    : AsyncMessage(
          type, destination, mask | MessageMask::ha_request, operation)
{
    if (op != 0)
        op->setRequest(this);
}

AsyncReply::AsyncReply(
    MessageType type,
    Uint32 mask,
    AsyncOpNode* operation,
    Uint32 resultCode)
    : AsyncMessage(
          type,0, mask | MessageMask::ha_reply, operation),
      result(resultCode)
{
    if (op != 0)
        op->setResponse(this);
}

AsyncIoClose::AsyncIoClose(
    AsyncOpNode* operation,
    Uint32 destination)
    : AsyncRequest(
          ASYNC_IOCLOSE,
          0,
          operation,
          destination)
{
}

CimServiceStart::CimServiceStart(
    AsyncOpNode* operation,
    Uint32 destination)
    : AsyncRequest(
          ASYNC_CIMSERVICE_START,
          0, operation, destination)
{
}


CimServiceStop::CimServiceStop(
    AsyncOpNode* operation,
    Uint32 destination)
    : AsyncRequest(
          ASYNC_CIMSERVICE_STOP,
          0, operation, destination)
{
}

AsyncOperationStart::AsyncOperationStart(
    AsyncOpNode* operation,
    Uint32 destination,
    Message* action)
    : AsyncRequest(
          ASYNC_ASYNC_OP_START,
          0,
          operation,
          destination),
      _act(action)
{
}

Message* AsyncOperationStart::get_action()
{
    Message* ret = _act;
    _act = 0;
    ret->put_async(0);
    return ret;
}

AsyncOperationResult::AsyncOperationResult(
    AsyncOpNode* operation,
    Uint32 resultCode)
    : AsyncReply(
          ASYNC_ASYNC_OP_RESULT,
          0,
          operation,
          resultCode)
{
}


AsyncModuleOperationStart::AsyncModuleOperationStart(
    AsyncOpNode* operation,
    Uint32 destination,
    const String& targetModule,
    Message* action)
    : AsyncRequest(
          ASYNC_ASYNC_MODULE_OP_START,
          0,
          operation,
          destination),
      _target_module(targetModule),
      _act(action)
{
    _act->put_async(this);
}


Message* AsyncModuleOperationStart::get_action()
{
    Message* ret = _act;
    _act = 0;
    ret->put_async(0);
    return ret;
}

AsyncModuleOperationResult::AsyncModuleOperationResult(
    AsyncOpNode* operation,
    Uint32 resultCode,
    const String& targetModule,
    Message* result)
    : AsyncReply(
          ASYNC_ASYNC_MODULE_OP_RESULT,
          0,
          operation, resultCode),
      _targetModule(targetModule),
      _res(result)
{
    _res->put_async(this);
}

Message* AsyncModuleOperationResult::get_result()
{
    Message* ret = _res;
    _res = 0;
    ret->put_async(0);
    return ret;
}


AsyncLegacyOperationStart::AsyncLegacyOperationStart(
    AsyncOpNode* operation,
    Uint32 destination,
    Message* action)
    : AsyncRequest(
          ASYNC_ASYNC_LEGACY_OP_START,
          0,
          operation, destination),
      _act(action)
{
    _act->put_async(this);
}


Message* AsyncLegacyOperationStart::get_action()
{
    Message* ret = _act;
    _act = 0;
//    ret->put_async(0);
    return ret;

}

void AsyncLegacyOperationStart::put_action(Message *act_)
{
    _act = act_;
}

AsyncLegacyOperationResult::AsyncLegacyOperationResult(
    AsyncOpNode* operation,
    Message* result)
    : AsyncReply(
          ASYNC_ASYNC_LEGACY_OP_RESULT,
          0, operation, 0),
      _res(result)
{
    _res->put_async(this);
}

Message* AsyncLegacyOperationResult::get_result()
{
    Message* ret = _res;
    _res = 0;
//    ret->put_async(0);
    return ret;
}

AsyncMessage::~AsyncMessage()
{
}

AsyncRequest::~AsyncRequest()
{
}

PEGASUS_NAMESPACE_END
