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

#include "Message.h"
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

Uint32 MessageMask::ha_request =             0x00100000;
Uint32 MessageMask::ha_reply =               0x00200000;
Uint32 MessageMask::ha_async =               0x00400000;

Message::~Message()
{
}


void Message::print(ostream& os, Boolean printHeader) const
{
    if (printHeader)
    {
        os << "Message\n";
        os << "{";
    }

    os << "    messageType: " << MessageTypeToString(_type) << endl;

    if (printHeader)
    {
        os << "}";
    }
}


static const char* _MESSAGE_TYPE_STRINGS[] =
{
    "HEARTBEAT/REPLY",

    "CIM_GET_CLASS_REQUEST_MESSAGE",
    "CIM_GET_INSTANCE_REQUEST_MESSAGE",
    "CIM_EXPORT_INDICATION_REQUEST_MESSAGE",
    "CIM_DELETE_CLASS_REQUEST_MESSAGE",
    "CIM_DELETE_INSTANCE_REQUEST_MESSAGE",
    "CIM_CREATE_CLASS_REQUEST_MESSAGE",
    "CIM_CREATE_INSTANCE_REQUEST_MESSAGE",
    "CIM_MODIFY_CLASS_REQUEST_MESSAGE",
    "CIM_MODIFY_INSTANCE_REQUEST_MESSAGE",
    "CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE",  // 10
    "CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE",
    "CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE",
    "CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE",
    "CIM_EXEC_QUERY_REQUEST_MESSAGE",
    "CIM_ASSOCIATORS_REQUEST_MESSAGE",
    "CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE",
    "CIM_REFERENCES_REQUEST_MESSAGE",
    "CIM_REFERENCE_NAMES_REQUEST_MESSAGE",
    "CIM_GET_PROPERTY_REQUEST_MESSAGE",
    "CIM_SET_PROPERTY_REQUEST_MESSAGE",  // 20
    "CIM_GET_QUALIFIER_REQUEST_MESSAGE",
    "CIM_SET_QUALIFIER_REQUEST_MESSAGE",
    "CIM_DELETE_QUALIFIER_REQUEST_MESSAGE",
    "CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE",
    "CIM_INVOKE_METHOD_REQUEST_MESSAGE",
    "CIM_PROCESS_INDICATION_REQUEST_MESSAGE",
    "CIM_HANDLE_INDICATION_REQUEST_MESSAGE",
    "CIM_NOTIFY_PROVIDER_REGISTRATION_REQUEST_MESSAGE",
    "CIM_NOTIFY_PROVIDER_TERMINATION_REQUEST_MESSAGE",
    "CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE",  // 30
    "CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE",
    "CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE",
    "CIM_DISABLE_MODULE_REQUEST_MESSAGE",
    "CIM_ENABLE_MODULE_REQUEST_MESSAGE",
    "CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE",
    "CIM_GET_CLASS_RESPONSE_MESSAGE",
    "CIM_GET_INSTANCE_RESPONSE_MESSAGE",
    "CIM_EXPORT_INDICATION_RESPONSE_MESSAGE",
    "CIM_DELETE_CLASS_RESPONSE_MESSAGE",
    "CIM_DELETE_INSTANCE_RESPONSE_MESSAGE",  // 40
    "CIM_CREATE_CLASS_RESPONSE_MESSAGE",
    "CIM_CREATE_INSTANCE_RESPONSE_MESSAGE",
    "CIM_MODIFY_CLASS_RESPONSE_MESSAGE",
    "CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE",
    "CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE",
    "CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE",
    "CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE",
    "CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE",
    "CIM_EXEC_QUERY_RESPONSE_MESSAGE",
    "CIM_ASSOCIATORS_RESPONSE_MESSAGE",  // 50
    "CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE",
    "CIM_REFERENCES_RESPONSE_MESSAGE",
    "CIM_REFERENCE_NAMES_RESPONSE_MESSAGE",
    "CIM_GET_PROPERTY_RESPONSE_MESSAGE",
    "CIM_SET_PROPERTY_RESPONSE_MESSAGE",
    "CIM_GET_QUALIFIER_RESPONSE_MESSAGE",
    "CIM_SET_QUALIFIER_RESPONSE_MESSAGE",
    "CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE",
    "CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE",
    "CIM_INVOKE_METHOD_RESPONSE_MESSAGE",
    "CIM_PROCESS_INDICATION_RESPONSE_MESSAGE", // 60
    "CIM_NOTIFY_PROVIDER_REGISTRATION_RESPONSE_MESSAGE",
    "CIM_NOTIFY_PROVIDER_TERMINATION_RESPONSE_MESSAGE",
    "CIM_HANDLE_INDICATION_RESPONSE_MESSAGE",
    "CIM_CREATE_SUBSCRIPTION_RESPONSE_MESSAGE",
    "CIM_MODIFY_SUBSCRIPTION_RESPONSE_MESSAGE",
    "CIM_DELETE_SUBSCRIPTION_RESPONSE_MESSAGE",
    "CIM_DISABLE_MODULE_RESPONSE_MESSAGE",
    "CIM_ENABLE_MODULE_RESPONSE_MESSAGE",
    "CIM_STOP_ALL_PROVIDERS_RESPONSE_MESSAGE",  // 70
// EXP_PULL_BEGIN
    "CIM_OPEN_ENUMERATE_INSTANCES_REQUEST_MESSAGE",
    "CIM_OPEN_ENUMERATE_INSTANCE_PATHS_REQUEST_MESSAGE",
    "CIM_OPEN_ASSOCIATOR_INSTANCES_REQUEST_MESSAGE",
    "CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_REQUEST_MESSAGE",
    "CIM_OPEN_REFERENCE_INSTANCES_REQUEST_MESSAGE",
    "CIM_OPEN_REFERENCE_INSTANCE_PATHS_REQUEST_MESSAGE",
    "CIM_OPEN_QUERY_INSTANCES_REQUEST_MESSAGE",
    "CIM_PULL_INSTANCES_WITH_PATHS_REQUEST_MESSAGE",
    "CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE",
    "CIM_PULL_INSTANCES_REQUEST_MESSAGE",
    "CIM_CLOSE_ENUMERATION_REQUEST_MESSAGE",  // 80
    "CIM_ENUMERATION_COUNT_REQUEST_MESSAGE",

    "CIM_OPEN_ENUMERATE_INSTANCES_RESPONSE_MESSAGE",
    "CIM_OPEN_ENUMERATE_INSTANCE_PATHS_RESPONSE_MESSAGE",
    "CIM_OPEN_ASSOCIATOR_INSTANCES_RESPONSE_MESSAGE",
    "CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_RESPONSE_MESSAGE",
    "CIM_OPEN_REFERENCES_INSTANCE_RESPONSE_MESSAGE",
    "CIM_OPEN_REFERENCE_INSTANCE_PATHS_RESPONSE_MESSAGE",
    "CIM_OPEN_QUERY_INSTANCES_RESPONSE_MESSAGE",
    "CIM_PULL_INSTANCES_WITH_PATH_RESPONSE_MESSAGE",
    "CIM_PULL_INSTANCE_PATHS_RESPONSE_MESSAGE",  // 90
    "CIM_PULL_INSTANCES_RESPONSE_MESSAGE",
    "CIM_CLOSE_ENUMERATION_RESPONSE_MESSAGE",
    "CIM_ENUMERATION_COUNT_RESPONSE_MESSAGE",
// EXP_PULL_END
    "SOCKET_MESSAGE",
    "CLOSE_CONNECTION_MESSAGE",
    "HTTP_MESSAGE",
    "HTTP_ERROR_MESSAGE",
    "CLIENT_EXCEPTION_MESSAGE",

    "ASYNC::IOCLOSE",
    "ASYNC::CIMSERVICE_START",  // 100
    "ASYNC::CIMSERVICE_STOP",

    "ASYNC::ASYNC_OP_START",
    "ASYNC::ASYNC_OP_RESULT",
    "ASYNC::ASYNC_LEGACY_OP_START",
    "ASYNC::ASYNC_LEGACY_OP_RESULT",

    "ASYNC::ASYNC_MODULE_OP_START",
    "ASYNC::ASYNC_MODULE_OP_RESULT",

    "CIM_NOTIFY_PROVIDER_ENABLE_REQUEST_MESSAGE",
    "CIM_NOTIFY_PROVIDER_ENABLE_RESPONSE_MESSAGE",

    "CIM_NOTIFY_PROVIDER_FAIL_REQUEST_MESSAGE", // 110
    "CIM_NOTIFY_PROVIDER_FAIL_RESPONSE_MESSAGE",

    "CIM_INITIALIZE_PROVIDER_AGENT_REQUEST_MESSAGE",
    "CIM_INITIALIZE_PROVIDER_AGENT_RESPONSE_MESSAGE",

    "CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE",
    "CIM_NOTIFY_CONFIG_CHANGE_RESPONSE_MESSAGE",

    "CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE",
    "CIM_SUBSCRIPTION_INIT_COMPLETE_RESPONSE_MESSAGE",

    "CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE",
    "CIM_INDICATION_SERVICE_DISABLED_RESPONSE_MESSAGE",

    "PROVAGT_GET_SCMOCLASS_REQUEST_MESSAGE",  // 120
    "PROVAGT_GET_SCMOCLASS_RESPONSE_MESSAGE",

    "CIM_NOTIFY_SUBSCRIPTION_NOT_ACTIVE_REQUEST_MESSAGE",
    "CIM_NOTIFY_SUBSCRIPTION_NOT_ACTIVE_RESPONSE_MESSAGE",

    "CIM_NOTIFY_LISTENER_NOT_ACTIVE_REQUEST_MESSAGE",
    "CIM_NOTIFY_LISTENER_NOT_ACTIVE_RESPONSE_MESSAGE",

    "WSMAN_EXPORT_INDICATION_REQUEST_MESSAGE",
    "WSMAN_EXPORT_INDICATION_RESPONSE_MESSAGE"
};

const char* MessageTypeToString(MessageType messageType)
{
    if (messageType < NUMBER_OF_MESSAGES)
    {
        return _MESSAGE_TYPE_STRINGS[messageType];
    }

    PEG_TRACE((TRC_MESSAGEQUEUESERVICE, Tracer::LEVEL2,
        "MessageTypeToString: Unknown message type 0x%04X", messageType));
    return "UNKNOWN";
}


CIMOperationType Message::convertMessageTypetoCIMOpType(MessageType type)
{
    CIMOperationType enum_type = CIMOPTYPE_GET_CLASS;
    switch (type)
    {
        case CIM_GET_CLASS_REQUEST_MESSAGE:
        case CIM_GET_CLASS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_GET_CLASS;
            break;

        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
        case CIM_GET_INSTANCE_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_GET_INSTANCE;
             break;

        case CIM_DELETE_CLASS_REQUEST_MESSAGE:
        case CIM_DELETE_CLASS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_DELETE_CLASS;
             break;

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
        case CIM_DELETE_INSTANCE_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_DELETE_INSTANCE;
             break;

        case CIM_CREATE_CLASS_REQUEST_MESSAGE:
        case CIM_CREATE_CLASS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_CREATE_CLASS;
             break;

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
        case CIM_CREATE_INSTANCE_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_CREATE_INSTANCE;
             break;

        case CIM_MODIFY_CLASS_REQUEST_MESSAGE:
        case CIM_MODIFY_CLASS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_MODIFY_CLASS;
             break;

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
        case CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_MODIFY_INSTANCE;
             break;

        case CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE:
        case CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_ENUMERATE_CLASSES;
             break;

        case CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE:
        case CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_ENUMERATE_CLASS_NAMES;
             break;

        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
        case CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_ENUMERATE_INSTANCES;
             break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
        case CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_ENUMERATE_INSTANCE_NAMES;
             break;

        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
        case CIM_EXEC_QUERY_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_EXEC_QUERY;
             break;

        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
        case CIM_ASSOCIATORS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_ASSOCIATORS;
             break;

        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
        case CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_ASSOCIATORS;
             break;

        case CIM_REFERENCES_REQUEST_MESSAGE:
        case CIM_REFERENCES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_REFERENCES;
             break;

        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
        case CIM_REFERENCE_NAMES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_REFERENCE_NAMES;
             break;

        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
        case CIM_GET_PROPERTY_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_GET_PROPERTY;
             break;

        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
        case CIM_SET_PROPERTY_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_SET_PROPERTY;
             break;

        case CIM_GET_QUALIFIER_REQUEST_MESSAGE:
        case CIM_GET_QUALIFIER_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_GET_QUALIFIER;
             break;

        case CIM_SET_QUALIFIER_REQUEST_MESSAGE:
        case CIM_SET_QUALIFIER_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_SET_QUALIFIER;
             break;

        case CIM_DELETE_QUALIFIER_REQUEST_MESSAGE:
        case CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_DELETE_QUALIFIER;
             break;

        case CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE:
        case CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_ENUMERATE_QUALIFIERS;
             break;

        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
        case CIM_INVOKE_METHOD_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_INVOKE_METHOD;
             break;
 // EXP_PULL_BEGIN
        case CIM_OPEN_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
        case CIM_OPEN_ENUMERATE_INSTANCES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_OPEN_ENUMERATE_INSTANCES;
             break;


        case CIM_OPEN_ENUMERATE_INSTANCE_PATHS_REQUEST_MESSAGE:
        case CIM_OPEN_ENUMERATE_INSTANCE_PATHS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_OPEN_ENUMERATE_INSTANCE_PATHS;
             break;

        case CIM_OPEN_ASSOCIATOR_INSTANCES_REQUEST_MESSAGE:
        case CIM_OPEN_ASSOCIATOR_INSTANCES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_OPEN_ASSOCIATOR_INSTANCES;
             break;

             /// TBD EXP_PULL_ERROR should be ASSOCIATOR_NAMES
        case CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_REQUEST_MESSAGE:
        case CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_OPEN_ASSOCIATOR_INSTANCE_PATHS;
             break;

        case CIM_OPEN_REFERENCE_INSTANCES_REQUEST_MESSAGE:
        case CIM_OPEN_REFERENCE_INSTANCES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_OPEN_REFERENCE_INSTANCES;
             break;

        case CIM_OPEN_REFERENCE_INSTANCE_PATHS_REQUEST_MESSAGE:
        case CIM_OPEN_REFERENCE_INSTANCE_PATHS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_OPEN_REFERENCE_INSTANCE_PATHS;
             break;

        case CIM_OPEN_QUERY_INSTANCES_REQUEST_MESSAGE:
        case CIM_OPEN_QUERY_INSTANCES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_OPEN_QUERY_INSTANCES;
             break;

        case CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE:
        case CIM_PULL_INSTANCES_WITH_PATH_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_PULL_INSTANCES_WITH_PATH;
             break;

        case CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE:
        case CIM_PULL_INSTANCE_PATHS_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_PULL_INSTANCE_PATHS;
             break;

        case CIM_PULL_INSTANCES_REQUEST_MESSAGE:
        case CIM_PULL_INSTANCES_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_PULL_INSTANCES;
             break;

        case CIM_CLOSE_ENUMERATION_REQUEST_MESSAGE:
        case CIM_CLOSE_ENUMERATION_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_CLOSE_ENUMERATION;
             break;

        case CIM_ENUMERATION_COUNT_REQUEST_MESSAGE:
        case CIM_ENUMERATION_COUNT_RESPONSE_MESSAGE:
             enum_type = CIMOPTYPE_ENUMERATION_COUNT;
             break;
 // EXP_PULL_END

        default:
             // exicution should never get to this point
             PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
    }
    return enum_type;
}

/**
 * Validate the magic object for this Message. This
 * compiles only in debug mode and can be use to validate the
 * CIMResponseData object. This validates the magic number if
 * in debug mode and always the _type.
 *
 * @return Boolean True if valid object.
 */
Boolean Message::valid() const
{
    PEGASUS_DEBUG_ASSERT(_magic);
    return (_type < NUMBER_OF_MESSAGES);
}


////////////////////////////////////////////////////////////////////////////////
//
// QueueIdStack
//
////////////////////////////////////////////////////////////////////////////////

QueueIdStack::QueueIdStack(const QueueIdStack& x) : _size(x._size)
{
    memcpy(_items, x._items, sizeof(_items));
}

QueueIdStack::QueueIdStack(Uint32 x) : _size(0)
{
    push(x);
}

QueueIdStack::QueueIdStack(Uint32 x1, Uint32 x2) : _size(0)
{
    push(x1);
    push(x2);
}

QueueIdStack& QueueIdStack::operator=(const QueueIdStack& x)
{
    if (this != &x)
    {
        memcpy(_items, x._items, sizeof(_items));
        _size = x._size;
    }
    return *this;
}

QueueIdStack QueueIdStack::copyAndPop() const
{
    return QueueIdStack(*this, 0);
}

QueueIdStack::QueueIdStack(const QueueIdStack& x, int) : _size(x._size)
{
    memcpy(_items, x._items, sizeof(_items));
    pop();
}

PEGASUS_NAMESPACE_END
