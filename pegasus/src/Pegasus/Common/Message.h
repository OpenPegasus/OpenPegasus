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

#ifndef Pegasus_Message_h
#define Pegasus_Message_h

#include <Pegasus/Common/Config.h>
#include <iostream>
#include <cstring>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMOperationType.h>
#include <Pegasus/Common/Linkable.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE MessageMask
{
public:
    // Message handling is indicated by the high order 12 bits.  For example:
    // Uint32 messageHandling = flags & 0xfff00000;
    static Uint32 ha_request;
    static Uint32 ha_reply;
    static Uint32 ha_async;
};

enum HttpMethod
{
    HTTP_METHOD__POST,
    HTTP_METHOD_M_POST
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    ,HTTP_METHOD_GET,
    HTTP_METHOD_HEAD
#endif /* PEGASUS_ENABLE_PROTOCOL_WEB */
};

enum MessageType
{
    DUMMY_MESSAGE,

    // CIM Message types:

    CIM_GET_CLASS_REQUEST_MESSAGE,
    CIM_GET_INSTANCE_REQUEST_MESSAGE,
    CIM_EXPORT_INDICATION_REQUEST_MESSAGE,
    CIM_DELETE_CLASS_REQUEST_MESSAGE,
    CIM_DELETE_INSTANCE_REQUEST_MESSAGE,
    CIM_CREATE_CLASS_REQUEST_MESSAGE,
    CIM_CREATE_INSTANCE_REQUEST_MESSAGE,
    CIM_MODIFY_CLASS_REQUEST_MESSAGE,
    CIM_MODIFY_INSTANCE_REQUEST_MESSAGE,
    CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE, //10
    CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE,
    CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE,
    CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE,
    CIM_EXEC_QUERY_REQUEST_MESSAGE,
    CIM_ASSOCIATORS_REQUEST_MESSAGE,
    CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE,
    CIM_REFERENCES_REQUEST_MESSAGE,
    CIM_REFERENCE_NAMES_REQUEST_MESSAGE,
    CIM_GET_PROPERTY_REQUEST_MESSAGE,
    CIM_SET_PROPERTY_REQUEST_MESSAGE, //20
    CIM_GET_QUALIFIER_REQUEST_MESSAGE,
    CIM_SET_QUALIFIER_REQUEST_MESSAGE,
    CIM_DELETE_QUALIFIER_REQUEST_MESSAGE,
    CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE,
    CIM_INVOKE_METHOD_REQUEST_MESSAGE,
    CIM_PROCESS_INDICATION_REQUEST_MESSAGE,
    CIM_HANDLE_INDICATION_REQUEST_MESSAGE,
    CIM_NOTIFY_PROVIDER_REGISTRATION_REQUEST_MESSAGE,
    CIM_NOTIFY_PROVIDER_TERMINATION_REQUEST_MESSAGE,
    CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE,  // 30
    CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE,
    CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE,
    CIM_DISABLE_MODULE_REQUEST_MESSAGE,
    CIM_ENABLE_MODULE_REQUEST_MESSAGE,
    CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE,

    CIM_GET_CLASS_RESPONSE_MESSAGE,
    CIM_GET_INSTANCE_RESPONSE_MESSAGE,
    CIM_EXPORT_INDICATION_RESPONSE_MESSAGE,
    CIM_DELETE_CLASS_RESPONSE_MESSAGE,
    CIM_DELETE_INSTANCE_RESPONSE_MESSAGE,  // 40
    CIM_CREATE_CLASS_RESPONSE_MESSAGE,
    CIM_CREATE_INSTANCE_RESPONSE_MESSAGE,
    CIM_MODIFY_CLASS_RESPONSE_MESSAGE,
    CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE,
    CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE,
    CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE,
    CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE,
    CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE,
    CIM_EXEC_QUERY_RESPONSE_MESSAGE,
    CIM_ASSOCIATORS_RESPONSE_MESSAGE,  // 50
    CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE,
    CIM_REFERENCES_RESPONSE_MESSAGE,
    CIM_REFERENCE_NAMES_RESPONSE_MESSAGE,
    CIM_GET_PROPERTY_RESPONSE_MESSAGE,
    CIM_SET_PROPERTY_RESPONSE_MESSAGE,
    CIM_GET_QUALIFIER_RESPONSE_MESSAGE,
    CIM_SET_QUALIFIER_RESPONSE_MESSAGE,
    CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE,
    CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE,
    CIM_INVOKE_METHOD_RESPONSE_MESSAGE,  // 60
    CIM_PROCESS_INDICATION_RESPONSE_MESSAGE,
    CIM_NOTIFY_PROVIDER_REGISTRATION_RESPONSE_MESSAGE,
    CIM_NOTIFY_PROVIDER_TERMINATION_RESPONSE_MESSAGE,
    CIM_HANDLE_INDICATION_RESPONSE_MESSAGE,
    CIM_CREATE_SUBSCRIPTION_RESPONSE_MESSAGE,
    CIM_MODIFY_SUBSCRIPTION_RESPONSE_MESSAGE,
    CIM_DELETE_SUBSCRIPTION_RESPONSE_MESSAGE,
    CIM_DISABLE_MODULE_RESPONSE_MESSAGE,
    CIM_ENABLE_MODULE_RESPONSE_MESSAGE,
    CIM_STOP_ALL_PROVIDERS_RESPONSE_MESSAGE,  // 70

// EXP_PULL_BEGIN
    // CIM Pull Operations

    CIM_OPEN_ENUMERATE_INSTANCES_REQUEST_MESSAGE, // 71
    CIM_OPEN_ENUMERATE_INSTANCE_PATHS_REQUEST_MESSAGE,
    CIM_OPEN_ASSOCIATOR_INSTANCES_REQUEST_MESSAGE,
    CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_REQUEST_MESSAGE,
    CIM_OPEN_REFERENCE_INSTANCES_REQUEST_MESSAGE,
    CIM_OPEN_REFERENCE_INSTANCE_PATHS_REQUEST_MESSAGE,
    CIM_OPEN_QUERY_INSTANCES_REQUEST_MESSAGE,
    CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE,
    CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE,
    CIM_PULL_INSTANCES_REQUEST_MESSAGE,  // 80
    CIM_CLOSE_ENUMERATION_REQUEST_MESSAGE,
    CIM_ENUMERATION_COUNT_REQUEST_MESSAGE,

    CIM_OPEN_ENUMERATE_INSTANCES_RESPONSE_MESSAGE,  // 83
    CIM_OPEN_ENUMERATE_INSTANCE_PATHS_RESPONSE_MESSAGE,
    CIM_OPEN_ASSOCIATOR_INSTANCES_RESPONSE_MESSAGE,
    CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_RESPONSE_MESSAGE,
    CIM_OPEN_REFERENCE_INSTANCES_RESPONSE_MESSAGE,
    CIM_OPEN_REFERENCE_INSTANCE_PATHS_RESPONSE_MESSAGE,
    CIM_OPEN_QUERY_INSTANCES_RESPONSE_MESSAGE,
    CIM_PULL_INSTANCES_WITH_PATH_RESPONSE_MESSAGE,  // 90
    CIM_PULL_INSTANCE_PATHS_RESPONSE_MESSAGE,
    CIM_PULL_INSTANCES_RESPONSE_MESSAGE,
    CIM_CLOSE_ENUMERATION_RESPONSE_MESSAGE,
    CIM_ENUMERATION_COUNT_RESPONSE_MESSAGE,  // 94
// EXP_PULL_END

    // Monitor-related messages:

    SOCKET_MESSAGE,

    // Connection-oriented messages:

    CLOSE_CONNECTION_MESSAGE,

    // HTTP messages:

    HTTP_MESSAGE,
    HTTP_ERROR_MESSAGE,

    // Exception messages to be passed to a CIM client application:

    CLIENT_EXCEPTION_MESSAGE,

    ASYNC_IOCLOSE,  // 100
    ASYNC_CIMSERVICE_START,
    ASYNC_CIMSERVICE_STOP,

    ASYNC_ASYNC_OP_START,
    ASYNC_ASYNC_OP_RESULT, // 105
    ASYNC_ASYNC_LEGACY_OP_START,
    ASYNC_ASYNC_LEGACY_OP_RESULT,

    ASYNC_ASYNC_MODULE_OP_START,
    ASYNC_ASYNC_MODULE_OP_RESULT,

    CIM_NOTIFY_PROVIDER_ENABLE_REQUEST_MESSAGE,  //110
    CIM_NOTIFY_PROVIDER_ENABLE_RESPONSE_MESSAGE,

    CIM_NOTIFY_PROVIDER_FAIL_REQUEST_MESSAGE,
    CIM_NOTIFY_PROVIDER_FAIL_RESPONSE_MESSAGE,

    CIM_INITIALIZE_PROVIDER_AGENT_REQUEST_MESSAGE,
    CIM_INITIALIZE_PROVIDER_AGENT_RESPONSE_MESSAGE, // 115

    CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE,
    CIM_NOTIFY_CONFIG_CHANGE_RESPONSE_MESSAGE,

    CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE,
    CIM_SUBSCRIPTION_INIT_COMPLETE_RESPONSE_MESSAGE,

    CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE,  // 120
    CIM_INDICATION_SERVICE_DISABLED_RESPONSE_MESSAGE,

    PROVAGT_GET_SCMOCLASS_REQUEST_MESSAGE,
    PROVAGT_GET_SCMOCLASS_RESPONSE_MESSAGE,

    CIM_NOTIFY_SUBSCRIPTION_NOT_ACTIVE_REQUEST_MESSAGE,
    CIM_NOTIFY_SUBSCRIPTION_NOT_ACTIVE_RESPONSE_MESSAGE,

    CIM_NOTIFY_LISTENER_NOT_ACTIVE_REQUEST_MESSAGE,
    CIM_NOTIFY_LISTENER_NOT_ACTIVE_RESPONSE_MESSAGE,

    WSMAN_EXPORT_INDICATION_REQUEST_MESSAGE,
    WSMAN_EXPORT_INDICATION_RESPONSE_MESSAGE,

    NUMBER_OF_MESSAGES
};

PEGASUS_COMMON_LINKAGE const char* MessageTypeToString(MessageType messageType);


/** The Message class and derived classes are used to pass messages between
    modules. Messages are passed between modules using the message queues
    (see MessageQueue class). Derived classes may add their own fields.
    This base class defines a common type field, which is the type of
    the message.
*/
class PEGASUS_COMMON_LINKAGE Message : public Linkable
{
public:

    Message(
        MessageType type,
        Uint32 destination = 0,
        Uint32 mask = 0)
        :
        dest(destination),
        _type(type),
        _mask(mask),
        _httpMethod(HTTP_METHOD__POST),
        _httpCloseConnect(false),
        _isComplete(true),
        _index(0),
        _async(0)
    {
    }

    virtual ~Message();

    // NOTE: The compiler default implementation of the copy constructor
    // is used for this class.

    Boolean getCloseConnect() const { return _httpCloseConnect; }
    void setCloseConnect(Boolean httpCloseConnect)
    {
        _httpCloseConnect = httpCloseConnect;
    }

    MessageType getType() const { return _type; }

    Uint32 getMask() const { return _mask; }

    void setMask(Uint32 mask) { _mask = mask; }

    HttpMethod getHttpMethod() const { return _httpMethod; }

    void setHttpMethod(HttpMethod httpMethod) {_httpMethod = httpMethod;}

    static CIMOperationType convertMessageTypetoCIMOpType(MessageType type);


    virtual void print(
        PEGASUS_STD(ostream)& os,
        Boolean printHeader = true) const;


    Message* get_async()
    {
        Message *ret = _async;
        _async = 0;
        return ret;
    }

    void put_async(Message* msg)
    {
        _async = msg;
    }

    // set the message index indicating what piece (or sequence) this is
    // message indexes start at zero
    void setIndex(Uint32 index) { _index = index; }

    // get the message index (or sequence number)
    Uint32 getIndex() const { return _index; }

    // is this the first piece of the message ?
    Boolean isFirst() const { return _index == 0; }

    // set the complete flag indicating whether this message piece is the last
    void setComplete(Boolean isComplete)
    {
        _isComplete = isComplete;
    }

    // is this message complete? (i.e the last in a one or more sequence)
    Boolean isComplete() const { return _isComplete; }

    // diagnostic tests magic number in context to see if valid object
    Boolean valid() const;


    Uint32 dest;

private:

    Message& operator=(const Message& msg);

    MessageType _type;
    Uint32 _mask;

    HttpMethod _httpMethod;
    Boolean _httpCloseConnect;

    Boolean _isComplete;
    Uint32 _index;

    Message* _async;
};


/** This class implements a stack of queue-ids. Many messages must keep a
    stack of queue-ids of queues which they must be returned to. This provides
    a light efficient stack for this purpose.
*/
class PEGASUS_COMMON_LINKAGE QueueIdStack
{
public:

    QueueIdStack() : _size(0)
    {
    }

    QueueIdStack(const QueueIdStack& x);

    explicit QueueIdStack(Uint32 x);

    explicit QueueIdStack(Uint32 x1, Uint32 x2);

    ~QueueIdStack()
    {
    }

    QueueIdStack& operator=(const QueueIdStack& x);

    Uint32 size() const
    {
        return _size;
    }

    Boolean isEmpty() const
    {
        return _size == 0;
    }

    void push(Uint32 x)
    {
#ifdef PEGASUS_DEBUG
        if (_size == MAX_SIZE)
            throw StackOverflow();
#endif
        _items[_size++] = x;
    }

    Uint32& top()
    {
#ifdef PEGASUS_DEBUG
        if (_size == 0)
            throw StackUnderflow();
#endif
        return _items[_size-1];
    }

    Uint32 top() const
    {
        return ((QueueIdStack*)this)->top();
    }

    void pop()
    {
#ifdef PEGASUS_DEBUG
        if (_size == 0)
            throw StackUnderflow();
#endif
        _size--;
    }

    /** Make a copy of this stack and then pop the top element. */
    QueueIdStack copyAndPop() const;

    Uint32 operator[](Uint32 i) const { return _items[i]; }

private:

    // Copy the given stack but then pop the top element:
    QueueIdStack(const QueueIdStack& x, int);

    enum { MAX_SIZE = 5 };
    Uint32 _items[MAX_SIZE];
    Uint32 _size;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Message_h */
