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

#ifndef Pegasus_ModuleController_h
#define Pegasus_ModuleController_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/AsyncOpNode.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/CIMMessage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE RegisteredModuleHandle : public Linkable
{
public:
    RegisteredModuleHandle(
        const String& name,
        void* module_address,
        Message* (*receive_message)(Message *, void *));

    virtual ~RegisteredModuleHandle();

    const String& get_name() const;

private:

    RegisteredModuleHandle();
    RegisteredModuleHandle(const RegisteredModuleHandle&);
    RegisteredModuleHandle& operator=(const RegisteredModuleHandle&);

    Message* _receive_message(Message* msg);
    String _name;
    void* _module_address;
    Message* (*_module_receive_message)(Message *, void *);

    friend class ModuleController;
};


class PEGASUS_COMMON_LINKAGE ModuleController : public MessageQueueService
{
public:
    typedef MessageQueueService Base;
    ModuleController(const char *name);

    ~ModuleController();

    void register_module(
        const String& module_name,
        void* module_address,
        Message* (*receive_message)(Message *, void *));

    static ModuleController* getModuleController();

    static  void indicationCallback(
        CIMProcessIndicationRequestMessage* request);

    // send a message to another service
    AsyncReply* ClientSendWait(
        Uint32 destination_q,
        AsyncRequest* request);

    Boolean ClientSendForget(
        Uint32 destination_q,
        AsyncRequest* message);

protected:
    // ATTN-RK-P2-20010322:  These methods are pure virtual in superclass
    virtual void handleEnqueue() {}
    virtual void handleEnqueue(Message *message);
    virtual void _handle_async_request(AsyncRequest *rq);
    virtual void _handle_async_callback(AsyncOpNode *op);

private:
    static void _indicationDeliveryRoutine(
        CIMProcessIndicationRequestMessage* request);
    typedef List<RegisteredModuleHandle, Mutex> RegisteredModulesList;
    RegisteredModulesList _modules;
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_ModuleController_h
