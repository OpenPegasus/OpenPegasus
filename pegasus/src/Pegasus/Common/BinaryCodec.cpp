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

#include <Pegasus/Common/StatisticalData.h>
#include "XmlWriter.h"
#include "BinaryCodec.h"
#include "CIMBuffer.h"
#include "StringConversion.h"
#include "Print.h"

#define ENABLE_VALIDATION

#if defined(_EQUAL)
# undef _EQUAL
#endif

#define _EQUAL(X, Y) (X.size() == (sizeof(Y)-1) && String::equalNoCase(X, Y))

// Header flags:
#define LOCAL_ONLY              (1 << 0)
#define INCLUDE_QUALIFIERS      (1 << 1)
#define INCLUDE_CLASS_ORIGIN    (1 << 2)
#define DEEP_INHERITANCE        (1 << 3)
#define CONTINUE_ON_ERROR       (1 << 4)

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// Local definitions:
//
//==============================================================================

static const Uint32 _MAGIC = 0xF00DFACE;
static const Uint32 _REVERSE_MAGIC = 0xCEFA0DF0;
static const Uint32 _VERSION = 1;

enum Operation
{
    OP_Invalid,
    OP_GetClass,
    OP_GetInstance,
    OP_IndicationDelivery,
    OP_DeleteClass,
    OP_DeleteInstance,
    OP_CreateClass,
    OP_CreateInstance,
    OP_ModifyClass,
    OP_ModifyInstance,
    OP_EnumerateClasses,
    OP_EnumerateClassNames,
    OP_EnumerateInstances,
    OP_EnumerateInstanceNames,
    OP_ExecQuery,
    OP_Associators,
    OP_AssociatorNames,
    OP_References,
    OP_ReferenceNames,
    OP_GetProperty,
    OP_SetProperty,
    OP_GetQualifier,
    OP_SetQualifier,
    OP_DeleteQualifier,
    OP_EnumerateQualifiers,
    OP_InvokeMethod,
// EXP_PULL_BEGIN
    OP_OpenEnumerateInstances,
    OP_OpenEnumerateInstancePaths,
    OP_OpenReferenceInstances,
    OP_OpenReferenceInstancePaths,
    OP_OpenAssociatorInstances,
    OP_OpenAssociatorInstancePaths,
    OP_PullInstancesWithPath,
    OP_PullInstancePaths,
    OP_PullInstances,
    OP_CloseEnumeration,
    OP_EnumerationCount,
    OP_OpenQueryInstances,
// EXP_PULL_END
    OP_Count
};

static Operation _NameToOp(const CIMName& name)
{
    const String& s = name.getString();

    switch (s[0])
    {
        case 'A':
            if (_EQUAL(s, "Associators"))
                return OP_Associators;
            if (_EQUAL(s, "AssociatorNames"))
                return OP_AssociatorNames;
            break;
        case 'C':
            if (_EQUAL(s, "CreateInstance"))
                return OP_CreateInstance;
            if (_EQUAL(s, "CreateClass"))
                return OP_CreateClass;
//EXP_PULL_BEGIN
            if (_EQUAL(s, "CloseEnumeration"))
                return OP_CloseEnumeration;
// EXP_PULL_END
            break;
        case 'D':
            if (_EQUAL(s, "DeleteInstance"))
                return OP_DeleteInstance;
            if (_EQUAL(s, "DeleteClass"))
                return OP_DeleteClass;
            if (_EQUAL(s, "DeleteQualifier"))
                return OP_DeleteQualifier;
            break;
        case 'E':
            if (_EQUAL(s, "EnumerateInstances"))
                return OP_EnumerateInstances;
            if (_EQUAL(s, "EnumerateInstanceNames"))
                return OP_EnumerateInstanceNames;
            if (_EQUAL(s, "ExecQuery"))
                return OP_ExecQuery;
            if (_EQUAL(s, "EnumerateClassNames"))
                return OP_EnumerateClassNames;
            if (_EQUAL(s, "EnumerateClasses"))
                return OP_EnumerateClasses;
            if (_EQUAL(s, "EnumerateQualifiers"))
                return OP_EnumerateQualifiers;
            break;
        case 'G':
            if (_EQUAL(s, "GetInstance"))
                return OP_GetInstance;
            if (_EQUAL(s, "GetClass"))
                return OP_GetClass;
            if (_EQUAL(s, "GetQualifier"))
                return OP_GetQualifier;
            if (_EQUAL(s, "GetProperty"))
                return OP_GetProperty;
            break;
        case 'I':
            if (_EQUAL(s, "InvokeMethod"))
                return OP_InvokeMethod;
            if (_EQUAL(s, "IndicationDelivery"))
                return OP_IndicationDelivery;
            break;
        case 'M':
            if (_EQUAL(s, "ModifyInstance"))
                return OP_ModifyInstance;
            if (_EQUAL(s, "ModifyClass"))
                return OP_ModifyClass;
            break;
// EXP_PULL_BEGIN
        case 'O':
            if (_EQUAL(s, "OpenEnumerateInstances"))
                return OP_OpenEnumerateInstances;
            if (_EQUAL(s, "OpenEnumerateInstancePaths"))
                return OP_OpenEnumerateInstancePaths;
            if (_EQUAL(s, "OpenReferenceInstances"))
                return OP_OpenReferenceInstances;
            if (_EQUAL(s, "OpenReferenceInstancePaths"))
                return OP_OpenReferenceInstancePaths;
            if (_EQUAL(s, "OpenAssociatorInstances"))
                return OP_OpenAssociatorInstances;
            if (_EQUAL(s, "OpenAssociatorInstancePaths"))
                return OP_OpenAssociatorInstancePaths;
            if (_EQUAL(s, "OpenQueryInstances"))
                return OP_OpenQueryInstances;;
            break;
        case 'P':
            if (_EQUAL(s, "PullInstancesWithPath"))
                return OP_PullInstancesWithPath;
            if (_EQUAL(s, "PullInstancePaths"))
                return OP_PullInstancePaths;
            if (_EQUAL(s, "PullInstances"))
                return OP_PullInstances;
            break;
// EXP_PULL_END
        case 'R':
            if (_EQUAL(s, "References"))
                return OP_References;
            if (_EQUAL(s, "ReferenceNames"))
                return OP_ReferenceNames;
            break;
        case 'S':
            if (_EQUAL(s, "SetQualifier"))
                return OP_SetQualifier;
            if (_EQUAL(s, "SetProperty"))
                return OP_SetProperty;
            break;
    }

    // Unknown, se we will assume it is an extrinsic method!
    return OP_InvokeMethod;
}

static void _putHeader(
    CIMBuffer& out,
    Uint32 flags,
    const String& messageId,
    Operation operation)
{
    // [MAGIC]
    out.putUint32(_MAGIC);

    // [VERSION]
    out.putUint32(_VERSION);

    // [FLAGS]
    out.putUint32(flags);

    // [MESSAGEID]
    out.putString(messageId);

    // [OPERATION]
    out.putUint32(operation);
}

static bool _getHeader(
    CIMBuffer& in,
    Uint32& flags,
    String& messageId,
    Operation& operation_)
{
    Uint32 magic;
    Uint32 version;

    // [MAGIC]
    if (!in.getUint32(magic))
        return false;

    if (magic != _MAGIC)
    {
        if (magic != _REVERSE_MAGIC)
            return false;

        // Sender has opposite endianess so turn on endian swapping:
        in.setSwap(true);
    }

    // [VERSION]
    if (!in.getUint32(version) || version != _VERSION)
        return false;

    // [FLAGS]
    if (!in.getUint32(flags))
        return false;

    // [MESSAGEID]
    if (!in.getString(messageId))
        return false;

    // [OPERATION]
    {
        Uint32 op;

        if (!in.getUint32(op) || op == OP_Invalid || op >= OP_Count)
            return false;

        operation_ = Operation(op);
    }

    return true;
}

//==============================================================================
//
// EnumerateInstances
//
//==============================================================================

static void _encodeEnumerateInstancesRequest(
    CIMBuffer& buf,
    CIMEnumerateInstancesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("EnumerateInstances");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->deepInheritance)
        flags |= DEEP_INHERITANCE;

    if (msg->includeQualifiers)
        flags |= INCLUDE_QUALIFIERS;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    _putHeader(buf, flags, msg->messageId, OP_EnumerateInstances);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [CLASSNAME]
    buf.putName(msg->className);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);
}

static CIMEnumerateInstancesRequestMessage* _decodeEnumerateInstancesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean deepInheritance = flags & DEEP_INHERITANCE;
#ifndef PEGASUS_DISABLE_INSTANCE_QUALIFIERS
    Boolean includeQualifiers = flags & INCLUDE_QUALIFIERS;
#endif
    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [CLASSNAME]
    CIMName className;

    if (!in.getName(className))
        return 0;

    // [PROPERTY-LIST]
    CIMPropertyList propertyList;
    if (!in.getPropertyList(propertyList))
        return 0;

    AutoPtr<CIMEnumerateInstancesRequestMessage> request(
        new CIMEnumerateInstancesRequestMessage(
            messageId,
            nameSpace,
            className,
            deepInheritance,
#ifdef PEGASUS_DISABLE_INSTANCE_QUALIFIERS
            false,
#else
            includeQualifiers,
#endif
            includeClassOrigin,
            propertyList,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeEnumerateInstancesResponseBody(
    CIMBuffer& out,
    CIMResponseData& data,
    CIMName& name,
    bool isFirst)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("EnumerateInstances");
    name = NAME;

    // Only write the property list on the first provider response
    if (isFirst)
    {
        // [PROPERTY-LIST]
        out.putPropertyList(data.getPropertyList());
    }
    data.encodeBinaryResponse(out);
}

static CIMEnumerateInstancesResponseMessage* _decodeEnumerateInstancesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMEnumerateInstancesResponseMessage* msg;
    CIMException cimException;

    msg = new CIMEnumerateInstancesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstances, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();

    // [PROPERTY-LIST]
    CIMPropertyList propertyList;
    if (!in.getPropertyList(propertyList))
    {
        return 0;
    }
    responseData.setPropertyList(propertyList);

    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}

//==============================================================================
//
// EnumerateInstanceNames
//
//==============================================================================

static CIMEnumerateInstanceNamesRequestMessage*
_decodeEnumerateInstanceNamesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    STAT_GETSTARTTIME

    // [NAMESPACE]
    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [CLASSNAME]
    CIMName className;

    if (!in.getName(className))
        return 0;

    AutoPtr<CIMEnumerateInstanceNamesRequestMessage> request(
        new CIMEnumerateInstanceNamesRequestMessage(
            messageId,
            nameSpace,
            className,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static CIMEnumerateInstanceNamesResponseMessage*
_decodeEnumerateInstanceNamesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMEnumerateInstanceNamesResponseMessage* msg;
    CIMException cimException;

    msg = new CIMEnumerateInstanceNamesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstanceNames, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest = true;
    return msg;
}

static void _encodeEnumerateInstanceNamesRequest(
    CIMBuffer& buf,
    CIMEnumerateInstanceNamesRequestMessage* msg,
    CIMName& name)
{
    static const CIMName NAME("EnumerateInstanceNames");
    name = NAME;

    // [HEADER]

    _putHeader(buf, 0, msg->messageId, OP_EnumerateInstanceNames);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [CLASSNAME]
    buf.putName(msg->className);
}

static void _encodeEnumerateInstanceNamesResponseBody(
    CIMBuffer& out,
    CIMResponseData& data,
    CIMName& name)
{
    static const CIMName NAME("EnumerateInstanceNames");
    name = NAME;

    data.encodeBinaryResponse(out);
}

//==============================================================================
//
// GetInstance
//
//==============================================================================

static CIMGetInstanceRequestMessage* _decodeGetInstanceRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    STAT_GETSTARTTIME

    // [FLAGS]
#ifndef PEGASUS_DISABLE_INSTANCE_QUALIFIERS
    Boolean includeQualifiers = flags & INCLUDE_QUALIFIERS;
#endif
    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;

    // [NAMESPACE]
    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [INSTANCE-NAME]
    CIMObjectPath instanceName;

    if (!in.getObjectPath(instanceName))
        return 0;

    // [PROPERTY-LIST]
    CIMPropertyList propertyList;
    if (!in.getPropertyList(propertyList))
        return 0;

    AutoPtr<CIMGetInstanceRequestMessage> request(
        new CIMGetInstanceRequestMessage(
            messageId,
            nameSpace,
            instanceName,
#ifdef PEGASUS_DISABLE_INSTANCE_QUALIFIERS
            false,
#else
            includeQualifiers,
#endif
            includeClassOrigin,
            propertyList,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static CIMGetInstanceResponseMessage* _decodeGetInstanceResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMGetInstanceResponseMessage* msg;
    CIMException cimException;

    msg = new CIMGetInstanceResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstances, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest = true;
    return msg;
}

static void _encodeGetInstanceRequest(
    CIMBuffer& buf,
    CIMGetInstanceRequestMessage* msg,
    CIMName& name)
{
    static const CIMName NAME("GetInstance");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->includeQualifiers)
        flags |= INCLUDE_QUALIFIERS;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    _putHeader(buf, flags, msg->messageId, OP_GetInstance);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [INSTANCE-NAME]
    buf.putObjectPath(msg->instanceName);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);
}

static void _encodeGetInstanceResponseBody(
    CIMBuffer& out,
    CIMResponseData& data,
    CIMName& name)
{
    static const CIMName NAME("GetInstance");
    name = NAME;

    data.encodeBinaryResponse(out);
}

//==============================================================================
//
// CreateInstance
//
//==============================================================================

static void _encodeCreateInstanceRequest(
    CIMBuffer& buf,
    CIMCreateInstanceRequestMessage* msg,
    CIMName& name)
{
    static const CIMName NAME("CreateInstance");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_CreateInstance);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [NEW-INSTANCE]
    buf.putInstance(msg->newInstance, false);
}

static CIMCreateInstanceRequestMessage* _decodeCreateInstanceRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [NEW-INSTANCE]

    CIMInstance newInstance;

    if (!in.getInstance(newInstance))
        return 0;

    AutoPtr<CIMCreateInstanceRequestMessage> request(
        new CIMCreateInstanceRequestMessage(
            messageId,
            nameSpace,
            newInstance,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeCreateInstanceResponseBody(
    CIMBuffer& out,
    CIMCreateInstanceResponseMessage* msg,
    CIMName& name)
{
    static const CIMName NAME("CreateInstance");
    name = NAME;

    out.putObjectPath(msg->instanceName, false);
}

static CIMCreateInstanceResponseMessage* _decodeCreateInstanceResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMObjectPath instanceName;

    if (!in.getObjectPath(instanceName))
        return 0;

    CIMCreateInstanceResponseMessage* msg;
    CIMException cimException;

    msg = new CIMCreateInstanceResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        instanceName);

    msg->binaryRequest = true;

    return msg;
}

//==============================================================================
//
// ModifyInstance
//
//==============================================================================

static void _encodeModifyInstanceRequest(
    CIMBuffer& buf,
    CIMModifyInstanceRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("ModifyInstance");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->includeQualifiers)
        flags |= INCLUDE_QUALIFIERS;

    _putHeader(buf, flags, msg->messageId, OP_ModifyInstance);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [MODIFIED-INSTANCE]
    buf.putInstance(msg->modifiedInstance, false);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);
}

static CIMModifyInstanceRequestMessage* _decodeModifyInstanceRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean includeQualifiers = flags & INCLUDE_QUALIFIERS;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [MODIFIED-INSTANCE]
    CIMInstance modifiedInstance;

    if (!in.getInstance(modifiedInstance))
        return 0;

    // [PROPERTY-LIST]
    CIMPropertyList propertyList;
    if (!in.getPropertyList(propertyList))
        return 0;

    AutoPtr<CIMModifyInstanceRequestMessage> request(
        new CIMModifyInstanceRequestMessage(
            messageId,
            nameSpace,
            modifiedInstance,
            includeQualifiers,
            propertyList,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeModifyInstanceResponseBody(
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("ModifyInstance");
    name = NAME;
}

static CIMModifyInstanceResponseMessage* _decodeModifyInstanceResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    Array<CIMInstance> instances;

    while (in.more())
    {
        Array<CIMInstance> tmp;

        if (!in.getInstanceA(tmp))
            return 0;

        instances.append(tmp.getData(), tmp.size());
    }

    CIMModifyInstanceResponseMessage* msg;
    CIMException cimException;

    msg = new CIMModifyInstanceResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// DeleteInstance
//
//==============================================================================

static void _encodeDeleteInstanceRequest(
    CIMBuffer& buf,
    CIMDeleteInstanceRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("DeleteInstance");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_DeleteInstance);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [INSTANCE-NAME]
    buf.putObjectPath(msg->instanceName, false);
}

static CIMDeleteInstanceRequestMessage* _decodeDeleteInstanceRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]
    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [INSTANCE-NAME]
    CIMObjectPath instanceName;

    if (!in.getObjectPath(instanceName))
        return 0;

    AutoPtr<CIMDeleteInstanceRequestMessage> request(
        new CIMDeleteInstanceRequestMessage(
            messageId,
            nameSpace,
            instanceName,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeDeleteInstanceResponseBody(
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("DeleteInstance");
    name = NAME;
}

static CIMDeleteInstanceResponseMessage* _decodeDeleteInstanceResponse(
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMDeleteInstanceResponseMessage* msg;
    CIMException cimException;

    msg = new CIMDeleteInstanceResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// Associators
//
//==============================================================================

static void _encodeAssociatorsRequest(
    CIMBuffer& buf,
    CIMAssociatorsRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("Associators");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->includeQualifiers)
        flags |= INCLUDE_QUALIFIERS;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    _putHeader(buf, flags, msg->messageId, OP_Associators);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [OBJECT-NAME]
    buf.putObjectPath(msg->objectName);

    // [ASSOC-CLASS]
    buf.putName(msg->assocClass);

    // [RESULT-CLASS]
    buf.putName(msg->resultClass);

    // [ROLE]
    buf.putString(msg->role);

    // [RESULT-ROLE]
    buf.putString(msg->resultRole);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);
}

static CIMAssociatorsRequestMessage* _decodeAssociatorsRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean includeQualifiers = flags & INCLUDE_QUALIFIERS;
    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [OBJECT-NAME]
    CIMObjectPath objectName;

    if (!in.getObjectPath(objectName))
        return 0;

    // [ASSOC-CLASS]

    CIMName assocClass;

    if (!in.getName(assocClass))
        return 0;

    // [RESULT-CLASS]

    CIMName resultClass;

    if (!in.getName(resultClass))
        return 0;

    // [ROLE]

    String role;

    if (!in.getString(role))
        return 0;

    // [RESULT-ROLE]

    String resultRole;

    if (!in.getString(resultRole))
        return 0;

    // [PROPERTY-LIST]

    CIMPropertyList propertyList;

    if (!in.getPropertyList(propertyList))
        return 0;

    AutoPtr<CIMAssociatorsRequestMessage> request(
        new CIMAssociatorsRequestMessage(
            messageId,
            nameSpace,
            objectName,
            assocClass,
            resultClass,
            role,
            resultRole,
            includeQualifiers,
            includeClassOrigin,
            propertyList,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeAssociatorsResponseBody(
    CIMBuffer& out,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("Associators");
    name = NAME;


    data.encodeBinaryResponse(out);
}

static CIMAssociatorsResponseMessage* _decodeAssociatorsResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMAssociatorsResponseMessage* msg;
    CIMException cimException;

    msg = new CIMAssociatorsResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getObjects, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// AssociatorNames
//
//==============================================================================

static void _encodeAssociatorNamesRequest(
    CIMBuffer& buf,
    CIMAssociatorNamesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("AssociatorNames");
    name = NAME;

    // [HEADER]
    Uint32 flags = 0;

    _putHeader(buf, flags, msg->messageId, OP_AssociatorNames);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [OBJECT-NAME]
    buf.putObjectPath(msg->objectName);

    // [ASSOC-CLASS]
    buf.putName(msg->assocClass);

    // [RESULT-CLASS]
    buf.putName(msg->resultClass);

    // [ROLE]
    buf.putString(msg->role);

    // [RESULT-ROLE]
    buf.putString(msg->resultRole);
}

static CIMAssociatorNamesRequestMessage* _decodeAssociatorNamesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [OBJECT-NAME]
    CIMObjectPath objectName;

    if (!in.getObjectPath(objectName))
        return 0;

    // [ASSOC-CLASS]

    CIMName assocClass;

    if (!in.getName(assocClass))
        return 0;

    // [RESULT-CLASS]

    CIMName resultClass;

    if (!in.getName(resultClass))
        return 0;

    // [ROLE]

    String role;

    if (!in.getString(role))
        return 0;

    // [RESULT-ROLE]

    String resultRole;

    if (!in.getString(resultRole))
        return 0;

    AutoPtr<CIMAssociatorNamesRequestMessage> request(
        new CIMAssociatorNamesRequestMessage(
            messageId,
            nameSpace,
            objectName,
            assocClass,
            resultClass,
            role,
            resultRole,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeAssociatorNamesResponseBody(
    CIMBuffer& out,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("AssociatorNames");
    name = NAME;
    data.encodeBinaryResponse(out);
}

static CIMAssociatorNamesResponseMessage* _decodeAssociatorNamesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMAssociatorNamesResponseMessage* msg;
    CIMException cimException;

    msg = new CIMAssociatorNamesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstanceNames, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// References
//
//==============================================================================

static void _encodeReferencesRequest(
    CIMBuffer& buf,
    CIMReferencesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("References");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->includeQualifiers)
        flags |= INCLUDE_QUALIFIERS;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    _putHeader(buf, flags, msg->messageId, OP_References);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [OBJECT-NAME]
    buf.putObjectPath(msg->objectName);

    // [RESULT-CLASS]
    buf.putName(msg->resultClass);

    // [ROLE]
    buf.putString(msg->role);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);
}

static CIMReferencesRequestMessage* _decodeReferencesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean includeQualifiers = flags & INCLUDE_QUALIFIERS;
    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [OBJECT-NAME]

    CIMObjectPath objectName;

    if (!in.getObjectPath(objectName))
        return 0;

    // [RESULT-CLASS]

    CIMName resultClass;

    if (!in.getName(resultClass))
        return 0;

    // [ROLE]

    String role;

    if (!in.getString(role))
        return 0;

    // [PROPERTY-LIST]

    CIMPropertyList propertyList;

    if (!in.getPropertyList(propertyList))
        return 0;

    AutoPtr<CIMReferencesRequestMessage> request(
        new CIMReferencesRequestMessage(
            messageId,
            nameSpace,
            objectName,
            resultClass,
            role,
            includeQualifiers,
            includeClassOrigin,
            propertyList,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeReferencesResponseBody(
    CIMBuffer& out,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("References");
    name = NAME;

    data.encodeBinaryResponse(out);
}

static CIMReferencesResponseMessage* _decodeReferencesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMReferencesResponseMessage* msg;
    CIMException cimException;

    msg = new CIMReferencesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    // Instead of resolving the binary data right here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to getNamedInstances, which is going to resolve the binary
    // data when the callback function is registered.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// ReferenceNames
//
//==============================================================================

static void _encodeReferenceNamesRequest(
    CIMBuffer& buf,
    CIMReferenceNamesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("ReferenceNames");
    name = NAME;

    // [HEADER]
    Uint32 flags = 0;

    _putHeader(buf, flags, msg->messageId, OP_ReferenceNames);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [OBJECT-NAME]
    buf.putObjectPath(msg->objectName);

    // [RESULT-CLASS]
    buf.putName(msg->resultClass);

    // [ROLE]
    buf.putString(msg->role);
}

static CIMReferenceNamesRequestMessage* _decodeReferenceNamesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [OBJECT-NAME]
    CIMObjectPath objectName;

    if (!in.getObjectPath(objectName))
        return 0;

    // [RESULT-CLASS]

    CIMName resultClass;

    if (!in.getName(resultClass))
        return 0;

    // [ROLE]

    String role;

    if (!in.getString(role))
        return 0;

    AutoPtr<CIMReferenceNamesRequestMessage> request(
        new CIMReferenceNamesRequestMessage(
            messageId,
            nameSpace,
            objectName,
            resultClass,
            role,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeReferenceNamesResponseBody(
    CIMBuffer& out,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("ReferenceNames");
    name = NAME;
    data.encodeBinaryResponse(out);
}

static CIMReferenceNamesResponseMessage* _decodeReferenceNamesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMReferenceNamesResponseMessage* msg;
    CIMException cimException;

    msg = new CIMReferenceNamesResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstanceNames, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest = true;

    return msg;
}

//==============================================================================
//
// GetClass
//
//==============================================================================

static void _encodeGetClassRequest(
    CIMBuffer& buf,
    CIMGetClassRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("GetClass");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->localOnly)
        flags |= LOCAL_ONLY;

    if (msg->includeQualifiers)
        flags |= INCLUDE_QUALIFIERS;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    _putHeader(buf, flags, msg->messageId, OP_GetClass);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [CLASSNAME]
    buf.putName(msg->className);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);
}

static CIMGetClassRequestMessage* _decodeGetClassRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean localOnly = flags & LOCAL_ONLY;
    Boolean includeQualifiers = flags & INCLUDE_QUALIFIERS;
    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [CLASSNAME]
    CIMName className;

    if (!in.getName(className))
        return 0;

    // [PROPERTY-LIST]
    CIMPropertyList propertyList;
    if (!in.getPropertyList(propertyList))
        return 0;

    AutoPtr<CIMGetClassRequestMessage> request(new CIMGetClassRequestMessage(
        messageId,
        nameSpace,
        className,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeGetClassResponseBody(
    CIMBuffer& out,
    CIMGetClassResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("GetClass");
    name = NAME;

    out.putClass(msg->cimClass);
}

static CIMGetClassResponseMessage* _decodeGetClassResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMClass cimClass;

    if (!in.getClass(cimClass))
        return 0;

    CIMGetClassResponseMessage* msg;
    CIMException cimException;

    msg = new CIMGetClassResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        cimClass);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// EnumerateClasses
//
//==============================================================================

static void _encodeEnumerateClassesRequest(
    CIMBuffer& buf,
    CIMEnumerateClassesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("EnumerateClasses");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->localOnly)
        flags |= LOCAL_ONLY;

    if (msg->deepInheritance)
        flags |= DEEP_INHERITANCE;

    if (msg->includeQualifiers)
        flags |= INCLUDE_QUALIFIERS;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    _putHeader(buf, flags, msg->messageId, OP_EnumerateClasses);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [CLASSNAME]
    buf.putName(msg->className);
}

static CIMEnumerateClassesRequestMessage* _decodeEnumerateClassesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean localOnly = flags & LOCAL_ONLY;
    Boolean deepInheritance = flags & DEEP_INHERITANCE;
    Boolean includeQualifiers = flags & INCLUDE_QUALIFIERS;
    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [CLASSNAME]
    CIMName className;

    if (!in.getName(className))
        return 0;

    AutoPtr<CIMEnumerateClassesRequestMessage> request(
        new CIMEnumerateClassesRequestMessage(
            messageId,
            nameSpace,
            className,
            deepInheritance,
            localOnly,
            includeQualifiers,
            includeClassOrigin,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeEnumerateClassesResponseBody(
    CIMBuffer& out,
    CIMEnumerateClassesResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("EnumerateClasses");
    name = NAME;

    out.putClassA(msg->cimClasses);
}

static CIMEnumerateClassesResponseMessage* _decodeEnumerateClassesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    Array<CIMClass> cimClasses;

    while (in.more())
    {
        Array<CIMClass> tmp;

        if (!in.getClassA(tmp))
            return 0;

        cimClasses.append(tmp.getData(), tmp.size());
    }

    CIMEnumerateClassesResponseMessage* msg;
    CIMException cimException;

    msg = new CIMEnumerateClassesResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        cimClasses);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// EnumerateClassNames
//
//==============================================================================

static void _encodeEnumerateClassNamesRequest(
    CIMBuffer& buf,
    CIMEnumerateClassNamesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("EnumerateClassNames");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->deepInheritance)
        flags |= DEEP_INHERITANCE;

    _putHeader(buf, flags, msg->messageId, OP_EnumerateClassNames);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [CLASSNAME]
    buf.putName(msg->className);
}

static CIMEnumerateClassNamesRequestMessage* _decodeEnumerateClassNamesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean deepInheritance = flags & DEEP_INHERITANCE;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [CLASSNAME]
    CIMName className;

    if (!in.getName(className))
        return 0;

    AutoPtr<CIMEnumerateClassNamesRequestMessage> request(
        new CIMEnumerateClassNamesRequestMessage(
            messageId,
            nameSpace,
            className,
            deepInheritance,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeEnumerateClassNamesResponseBody(
    CIMBuffer& out,
    CIMEnumerateClassNamesResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("EnumerateClassNames");
    name = NAME;

    out.putNameA(msg->classNames);
}

static CIMEnumerateClassNamesResponseMessage*
_decodeEnumerateClassNamesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    Array<CIMName> classNames;

    while (in.more())
    {
        Array<CIMName> tmp;

        if (!in.getNameA(tmp))
            return 0;

        classNames.append(tmp.getData(), tmp.size());
    }

    CIMEnumerateClassNamesResponseMessage* msg;
    CIMException cimException;

    msg = new CIMEnumerateClassNamesResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        classNames);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// CreateClass
//
//==============================================================================

static void _encodeCreateClassRequest(
    CIMBuffer& buf,
    CIMCreateClassRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("CreateClass");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_CreateClass);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [NEW-CLASS]
    buf.putClass(msg->newClass);
}

static CIMCreateClassRequestMessage* _decodeCreateClassRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [NEW-CLASS]
    CIMClass newClass;

    if (!in.getClass(newClass))
        return 0;

    AutoPtr<CIMCreateClassRequestMessage> request(
        new CIMCreateClassRequestMessage(
            messageId,
            nameSpace,
            newClass,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeCreateClassResponseBody(
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("CreateClass");
    name = NAME;
}

static CIMCreateClassResponseMessage* _decodeCreateClassResponse(
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMCreateClassResponseMessage* msg;
    CIMException cimException;

    msg = new CIMCreateClassResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// DeleteClass
//
//==============================================================================

static void _encodeDeleteClassRequest(
    CIMBuffer& buf,
    CIMDeleteClassRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("DeleteClass");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_DeleteClass);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [CLASSNAME]
    buf.putName(msg->className);
}

static CIMDeleteClassRequestMessage* _decodeDeleteClassRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [CLASSNAME]
    CIMName className;

    if (!in.getName(className))
        return 0;

    AutoPtr<CIMDeleteClassRequestMessage> request(
        new CIMDeleteClassRequestMessage(
            messageId,
            nameSpace,
            className,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeDeleteClassResponseBody(
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("DeleteClass");
    name = NAME;
}

static CIMDeleteClassResponseMessage* _decodeDeleteClassResponse(
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMDeleteClassResponseMessage* msg;
    CIMException cimException;

    msg = new CIMDeleteClassResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// ModifyClass
//
//==============================================================================

static void _encodeModifyClassRequest(
    CIMBuffer& buf,
    CIMModifyClassRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("ModifyClass");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_ModifyClass);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [NEW-CLASS]
    buf.putClass(msg->modifiedClass);
}

static CIMModifyClassRequestMessage* _decodeModifyClassRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [MODIFIED-CLASS]
    CIMClass modifiedClass;

    if (!in.getClass(modifiedClass))
        return 0;

    AutoPtr<CIMModifyClassRequestMessage> request(
        new CIMModifyClassRequestMessage(
            messageId,
            nameSpace,
            modifiedClass,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeModifyClassResponseBody(
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("ModifyClass");
    name = NAME;
}

static CIMModifyClassResponseMessage* _decodeModifyClassResponse(
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMModifyClassResponseMessage* msg;
    CIMException cimException;

    msg = new CIMModifyClassResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// SetQualifier
//
//==============================================================================

static void _encodeSetQualifierRequest(
    CIMBuffer& buf,
    CIMSetQualifierRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("SetQualifier");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_SetQualifier);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [QUALIFIER-DECLARATION]
    buf.putQualifierDecl(msg->qualifierDeclaration);
}

static CIMSetQualifierRequestMessage* _decodeSetQualifierRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [QUALIFIER.DECLARATION]

    CIMQualifierDecl qualifierDeclaration;

    if (!in.getQualifierDecl(qualifierDeclaration))
        return 0;

    AutoPtr<CIMSetQualifierRequestMessage> request(
        new CIMSetQualifierRequestMessage(
            messageId,
            nameSpace,
            qualifierDeclaration,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeSetQualifierResponseBody(
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("SetQualifier");
    name = NAME;
}

static CIMSetQualifierResponseMessage* _decodeSetQualifierResponse(
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMSetQualifierResponseMessage* msg;
    CIMException cimException;

    msg = new CIMSetQualifierResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// GetQualifier
//
//==============================================================================

static void _encodeGetQualifierRequest(
    CIMBuffer& buf,
    CIMGetQualifierRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("GetQualifier");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_GetQualifier);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [QUALIFIER-NAME]
    buf.putName(msg->qualifierName);
}

static CIMGetQualifierRequestMessage* _decodeGetQualifierRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [QUALIFIER-NAME]
    CIMName qualifierName;

    if (!in.getName(qualifierName))
        return 0;

    AutoPtr<CIMGetQualifierRequestMessage> request(
        new CIMGetQualifierRequestMessage(
            messageId,
            nameSpace,
            qualifierName,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeGetQualifierResponseBody(
    CIMBuffer& out,
    CIMGetQualifierResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("GetQualifier");
    name = NAME;

    out.putQualifierDecl(msg->cimQualifierDecl);
}

static CIMGetQualifierResponseMessage* _decodeGetQualifierResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMQualifierDecl cimQualifierDecl;

    if (!in.getQualifierDecl(cimQualifierDecl))
        return 0;

    CIMGetQualifierResponseMessage* msg;
    CIMException cimException;

    msg = new CIMGetQualifierResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        cimQualifierDecl);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// DeleteQualifier
//
//==============================================================================

static void _encodeDeleteQualifierRequest(
    CIMBuffer& buf,
    CIMDeleteQualifierRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("DeleteQualifier");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_DeleteQualifier);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [QUALIFIER-NAME]
    buf.putName(msg->qualifierName);
}

static CIMDeleteQualifierRequestMessage* _decodeDeleteQualifierRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [QUALIFIER-NAME]
    CIMName qualifierName;

    if (!in.getName(qualifierName))
        return 0;

    AutoPtr<CIMDeleteQualifierRequestMessage> request(
        new CIMDeleteQualifierRequestMessage(
            messageId,
            nameSpace,
            qualifierName,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeDeleteQualifierResponseBody(
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("DeleteQualifier");
    name = NAME;
}

static CIMDeleteQualifierResponseMessage* _decodeDeleteQualifierResponse(
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMDeleteQualifierResponseMessage* msg;
    CIMException cimException;

    msg = new CIMDeleteQualifierResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// EnumerateQualifiers
//
//==============================================================================

static void _encodeEnumerateQualifiersRequest(
    CIMBuffer& buf,
    CIMEnumerateQualifiersRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("EnumerateQualifiers");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_EnumerateQualifiers);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);
}

static CIMEnumerateQualifiersRequestMessage* _decodeEnumerateQualifiersRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    AutoPtr<CIMEnumerateQualifiersRequestMessage> request(
        new CIMEnumerateQualifiersRequestMessage(
            messageId,
            nameSpace,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeEnumerateQualifiersResponseBody(
    CIMBuffer& out,
    CIMEnumerateQualifiersResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("EnumerateQualifiers");
    name = NAME;

    out.putQualifierDeclA(msg->qualifierDeclarations);
}

static CIMEnumerateQualifiersResponseMessage*
    _decodeEnumerateQualifiersResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    Array<CIMQualifierDecl> qualifierDecls;

    while (in.more())
    {
        Array<CIMQualifierDecl> tmp;

        if (!in.getQualifierDeclA(tmp))
            return 0;

        qualifierDecls.append(tmp.getData(), tmp.size());
    }

    CIMEnumerateQualifiersResponseMessage* msg;
    CIMException cimException;

    msg = new CIMEnumerateQualifiersResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        qualifierDecls);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// GetProperty
//
//==============================================================================

static void _encodeGetPropertyRequest(
    CIMBuffer& buf,
    CIMGetPropertyRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("GetProperty");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_GetProperty);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [INSTANCE-NAME]
    buf.putObjectPath(msg->instanceName);

    // [PROPERTY-NAME]
    buf.putName(msg->propertyName);
}

static CIMGetPropertyRequestMessage* _decodeGetPropertyRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [INSTANCE-NAME]
    CIMObjectPath instanceName;

    if (!in.getObjectPath(instanceName))
        return 0;

    // [PROPERTY-NAME]
    CIMName propertyName;

    if (!in.getName(propertyName))
        return 0;

    AutoPtr<CIMGetPropertyRequestMessage> request(
        new CIMGetPropertyRequestMessage(
            messageId,
            nameSpace,
            instanceName,
            propertyName,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeGetPropertyResponseBody(
    CIMBuffer& out,
    CIMGetPropertyResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("GetProperty");
    name = NAME;

    // [VALUE]
    out.putValue(msg->value);
}

static CIMGetPropertyResponseMessage* _decodeGetPropertyResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    // [VALUE]
    CIMValue value;

    if (!in.getValue(value))
        return 0;

    // Unfortunately the CIM GetProperty() method is only able to return
    // a string since there is no containing element that specifies its
    // type. So even though the binary protocol properly transmits the type,
    // we are force to convert that type to string to match the XML protocol
    // behavior.

    if (value.isNull())
        value.setNullValue(CIMTYPE_STRING, false);
    else
        value.set(value.toString());

    CIMGetPropertyResponseMessage* msg;
    CIMException cimException;

    msg = new CIMGetPropertyResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        value);

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// SetProperty
//
//==============================================================================

static void _encodeSetPropertyRequest(
    CIMBuffer& buf,
    CIMSetPropertyRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("SetProperty");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_SetProperty);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [INSTANCE-NAME]
    buf.putObjectPath(msg->instanceName);

    // [PROPERTY-NAME]
    buf.putName(msg->propertyName);

    // [VALUE]
    buf.putValue(msg->newValue);
}

static CIMSetPropertyRequestMessage* _decodeSetPropertyRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [INSTANCE-NAME]

    CIMObjectPath instanceName;

    if (!in.getObjectPath(instanceName))
        return 0;

    // [PROPERTY-NAME]

    CIMName propertyName;

    if (!in.getName(propertyName))
        return 0;

    // [PROPERTY-VALUE]

    CIMValue propertyValue;

    if (!in.getValue(propertyValue))
        return 0;

    AutoPtr<CIMSetPropertyRequestMessage> request(
        new CIMSetPropertyRequestMessage(
            messageId,
            nameSpace,
            instanceName,
            propertyName,
            propertyValue,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeSetPropertyResponseBody(
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("SetProperty");
    name = NAME;
}

static CIMSetPropertyResponseMessage* _decodeSetPropertyResponse(
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    CIMSetPropertyResponseMessage* msg;
    CIMException cimException;

    msg = new CIMSetPropertyResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    msg->binaryRequest = true;
    return msg;
}

//==============================================================================
//
// InvokeMethod
//
//==============================================================================

static void _encodeInvokeMethodRequest(
    CIMBuffer& buf,
    CIMInvokeMethodRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    name = msg->methodName;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_InvokeMethod);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [INSTANCE-NAME]
    buf.putObjectPath(msg->instanceName);

    // [METHOD-NAME]
    buf.putName(msg->methodName);

    // [IN-PARAMETERS]
    buf.putParamValueA(msg->inParameters);
}

static CIMInvokeMethodRequestMessage* _decodeInvokeMethodRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [INSTANCE-NAME]

    CIMObjectPath instanceName;

    if (!in.getObjectPath(instanceName))
        return 0;

    // [METHOD-NAME]

    CIMName methodName;

    if (!in.getName(methodName))
        return 0;

    // [IN-PARAMETERS]

    Array<CIMParamValue> inParameters;

    if (!in.getParamValueA(inParameters))
        return 0;

    AutoPtr<CIMInvokeMethodRequestMessage> request(
        new CIMInvokeMethodRequestMessage(
            messageId,
            nameSpace,
            instanceName,
            methodName,
            inParameters,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeInvokeMethodResponseBody(
    CIMBuffer& out,
    CIMInvokeMethodResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    name = msg->methodName;

    // [METHOD-NAME]
    out.putName(msg->methodName);

    // [RETURN-VALUE]
    out.putValue(msg->retValue);

    // [OUT-PARAMETERS]
    out.putParamValueA(msg->outParameters);
}

static CIMInvokeMethodResponseMessage* _decodeInvokeMethodResponse(
    CIMBuffer& in,
    const String& messageId)
{
    /* See ../Client/CIMOperationResponseDecoder.cpp */

    // [METHOD-NAME]

    CIMName methodName;

    if (!in.getName(methodName))
        return 0;

    // [RETURN-VALUE]

    CIMValue returnValue;

    if (!in.getValue(returnValue))
        return 0;

    // [OUT-PARAMETERS]

    Array<CIMParamValue> outParameters;

    if (!in.getParamValueA(outParameters))
        return 0;

    CIMInvokeMethodResponseMessage* msg;
    CIMException cimException;

    msg = new CIMInvokeMethodResponseMessage(
        messageId,
        cimException,
        QueueIdStack(),
        returnValue,
        outParameters,
        methodName);

    msg->binaryRequest = true;

    return msg;
}

//==============================================================================
//
// ExecQuery
//
//==============================================================================

static void _encodeExecQueryRequest(
    CIMBuffer& buf,
    CIMExecQueryRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("ExecQuery");
    name = NAME;

    // [HEADER]
    _putHeader(buf, 0, msg->messageId, OP_ExecQuery);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [QUERY-LANGUAGE]
    buf.putString(msg->queryLanguage);

    // [QUERY]
    buf.putString(msg->query);
}

static CIMExecQueryRequestMessage* _decodeExecQueryRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;

    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [QUERY-LANGUAGE]]
    String queryLanguage;

    if (!in.getString(queryLanguage))
        return 0;

    // [QUERY]]
    String query;

    if (!in.getString(query))
        return 0;

    AutoPtr<CIMExecQueryRequestMessage> request(
        new CIMExecQueryRequestMessage(
            messageId,
            nameSpace,
            queryLanguage,
            query,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeExecQueryResponseBody(
    CIMBuffer& out,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("ExecQuery");
    name = NAME;

    data.encodeBinaryResponse(out);
}

static CIMExecQueryResponseMessage* _decodeExecQueryResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMExecQueryResponseMessage* msg;
    CIMException cimException;

    msg = new CIMExecQueryResponseMessage(
        messageId,
        cimException,
        QueueIdStack());

    // Instead of resolving the binary data right here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to getNamedInstances, which is going to resolve the binary
    // data when the callback function is registered.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest = true;
    return msg;
}
// EXP_PULL_BEGIN
//==============================================================================
//
// OpenEnumerateInstances
//
//==============================================================================

static void _encodeOpenEnumerateInstancesRequest(
    CIMBuffer& buf,
    CIMOpenEnumerateInstancesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("OpenEnumerateInstances");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->deepInheritance)
        flags |= DEEP_INHERITANCE;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    if (msg->continueOnError)
        flags |= CONTINUE_ON_ERROR;

    _putHeader(buf, flags, msg->messageId, OP_OpenEnumerateInstances);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [CLASSNAME]
    buf.putName(msg->className);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);

    buf.putUint32(msg->maxObjectCount);
    buf.putUint32Arg(msg->operationTimeout);
    buf.putString(msg->filterQueryLanguage);
    buf.putString(msg->filterQuery);
}

static CIMOpenEnumerateInstancesRequestMessage*
    _decodeOpenEnumerateInstancesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean deepInheritance = flags & DEEP_INHERITANCE;
    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;
    Boolean continueOnError = flags & CONTINUE_ON_ERROR;

    // [NAMESPACE]
    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [CLASSNAME]
    CIMName className;
    if (!in.getName(className))
        return 0;

    // [PROPERTY-LIST]
    CIMPropertyList propertyList;
    if (!in.getPropertyList(propertyList))
        return 0;

    // These can all be one common function.
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
       return 0;
    Uint32Arg operationTimeout;
    if (!in.getUint32Arg(operationTimeout))
        return 0;
    String filterQueryLanguage;
    if (!in.getString(filterQueryLanguage))
        return 0;
    String filterQuery;
    if (!in.getString(filterQuery))
        return 0;

    AutoPtr<CIMOpenEnumerateInstancesRequestMessage> request(
        new CIMOpenEnumerateInstancesRequestMessage(
            messageId,
            nameSpace,
            className,
            deepInheritance,
            includeClassOrigin,
            propertyList,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

// For the pull Response messages the interface is the message and the,
// not just the responseData.
static void _encodeOpenEnumerateInstancesResponseBody(
    CIMBuffer& out,
    CIMOpenEnumerateInstancesResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("OpenEnumerateInstances");
    name = NAME;

    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMOpenEnumerateInstancesResponseMessage*
    _decodeOpenEnumerateInstancesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    // KS_TODO Should we set validation???
    // Turn on validation: This is a debugging tool
//#if defined(ENABLE_VALIDATION)
//    buf.setValidate(true);
//#endif

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
    {
        return 0;
    }

    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    CIMOpenEnumerateInstancesResponseMessage* msg =
        new CIMOpenEnumerateInstancesResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstances, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}

//==============================================================================
//
// OpenEnumerateInstancesPaths
//
//==============================================================================

static void _encodeOpenEnumerateInstancePathsRequest(
    CIMBuffer& buf,
    CIMOpenEnumerateInstancePathsRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("OpenEnumerateInstancePaths");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->continueOnError)
        flags |= CONTINUE_ON_ERROR;

    _putHeader(buf, flags, msg->messageId, OP_OpenEnumerateInstancePaths);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [CLASSNAME]
    buf.putName(msg->className);

    buf.putUint32(msg->maxObjectCount);
    buf.putUint32Arg(msg->operationTimeout);
    buf.putString(msg->filterQueryLanguage);
    buf.putString(msg->filterQuery);
}

static CIMOpenEnumerateInstancePathsRequestMessage*
    _decodeOpenEnumerateInstancePathsRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean continueOnError = flags & CONTINUE_ON_ERROR;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [CLASSNAME]
    CIMName className;
    if (!in.getName(className))
        return 0;

    // These can all be one common function.

    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
       return 0;
    Uint32Arg operationTimeout;
    if (!in.getUint32Arg(operationTimeout))
        return 0;
    String filterQueryLanguage;
    if (!in.getString(filterQueryLanguage))
        return 0;
    String filterQuery;
    if (!in.getString(filterQuery))
        return 0;

    AutoPtr<CIMOpenEnumerateInstancePathsRequestMessage> request(
        new CIMOpenEnumerateInstancePathsRequestMessage(
            messageId,
            nameSpace,
            className,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

// For the pull Response messages the interface is the message and the,
// not just the responseData.
static void _encodeOpenEnumerateInstancePathsResponseBody(
    CIMBuffer& out,
    CIMOpenEnumerateInstancePathsResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("OpenEnumerateInstancePaths");
    name = NAME;

    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMOpenEnumerateInstancePathsResponseMessage*
    _decodeOpenEnumerateInstancePathsResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
    {
        return 0;
    }

    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    CIMOpenEnumerateInstancePathsResponseMessage* msg =
        new CIMOpenEnumerateInstancePathsResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstanceNames, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}
//==============================================================================
//
// OpenReferenceInstances
//
//==============================================================================

static void _encodeOpenReferenceInstancesRequest(
    CIMBuffer& buf,
    CIMOpenReferenceInstancesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("OpenReferenceInstances");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    if (msg->continueOnError)
        flags |= CONTINUE_ON_ERROR;

    _putHeader(buf, flags, msg->messageId, OP_OpenReferenceInstances);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [OBJECT-NAME]
    buf.putObjectPath(msg->objectName);

    // [RESULT-CLASS]
    buf.putName(msg->resultClass);

    // [ROLE]
    buf.putString(msg->role);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);

    buf.putUint32(msg->maxObjectCount);
    buf.putUint32Arg(msg->operationTimeout);
    buf.putString(msg->filterQueryLanguage);
    buf.putString(msg->filterQuery);
}

static CIMOpenReferenceInstancesRequestMessage*
    _decodeOpenReferenceInstancesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;
    Boolean continueOnError = flags & CONTINUE_ON_ERROR;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [OBJECT-NAME]

    CIMObjectPath objectName;

    if (!in.getObjectPath(objectName))
        return 0;

    // [RESULT-CLASS]

    CIMName resultClass;

    if (!in.getName(resultClass))
        return 0;

    // [ROLE]

    String role;

    if (!in.getString(role))
        return 0;

    // [PROPERTY-LIST]
    CIMPropertyList propertyList;
    if (!in.getPropertyList(propertyList))
        return 0;

    // These can all be one common function.
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
       return 0;
    Uint32Arg operationTimeout;
    if (!in.getUint32Arg(operationTimeout))
        return 0;
    String filterQueryLanguage;
    if (!in.getString(filterQueryLanguage))
        return 0;
    String filterQuery;
    if (!in.getString(filterQuery))
        return 0;

    AutoPtr<CIMOpenReferenceInstancesRequestMessage> request(
        new CIMOpenReferenceInstancesRequestMessage(
            messageId,
            nameSpace,
            objectName,
            resultClass,
            role,
            includeClassOrigin,
            propertyList,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}
// For the pull Response messages the interface is the message and the,
// not just the responseData.
static void _encodeOpenReferenceInstancesResponseBody(
    CIMBuffer& out,
    CIMOpenReferenceInstancesResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("OpenReferenceInstances");
    name = NAME;

    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMOpenReferenceInstancesResponseMessage*
    _decodeOpenReferenceInstancesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
        return 0;

    String enumerationContext;
    if (!in.getString(enumerationContext))
        return 0;

    CIMOpenReferenceInstancesResponseMessage* msg =
        new CIMOpenReferenceInstancesResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstances, which
    // resolves the binary data is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}
//==============================================================================
//
// OpenReferenceInstancePaths
//
//==============================================================================

static void _encodeOpenReferenceInstancePathsRequest(
    CIMBuffer& buf,
    CIMOpenReferenceInstancePathsRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("OpenReferenceInstancePaths");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;
    if (msg->continueOnError)
        flags |= CONTINUE_ON_ERROR;

    _putHeader(buf, flags, msg->messageId, OP_OpenReferenceInstancePaths);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [OBJECT-NAME]
    buf.putObjectPath(msg->objectName);

    // [RESULT-CLASS]
    buf.putName(msg->resultClass);

    // [ROLE]
    buf.putString(msg->role);

    buf.putUint32(msg->maxObjectCount);
    buf.putUint32Arg(msg->operationTimeout);
    buf.putString(msg->filterQueryLanguage);
    buf.putString(msg->filterQuery);
}

static CIMOpenReferenceInstancePathsRequestMessage*
    _decodeOpenReferenceInstancePathsRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../
Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean continueOnError = flags & CONTINUE_ON_ERROR;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [OBJECT-NAME]

    CIMObjectPath objectName;
    if (!in.getObjectPath(objectName))
        return 0;

    // [RESULT-CLASS]

    CIMName resultClass;
    if (!in.getName(resultClass))
        return 0;

    // [ROLE]

    String role;
    if (!in.getString(role))
        return 0;

    // These can all be one common function.
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
       return 0;
    Uint32Arg operationTimeout;
    if (!in.getUint32Arg(operationTimeout))
        return 0;
    String filterQueryLanguage;
    if (!in.getString(filterQueryLanguage))
        return 0;
    String filterQuery;
    if (!in.getString(filterQuery))
        return 0;

    AutoPtr<CIMOpenReferenceInstancePathsRequestMessage> request(
        new CIMOpenReferenceInstancePathsRequestMessage(
            messageId,
            nameSpace,
            objectName,
            resultClass,
            role,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}
// For the pull Response messages the interface is the message and the,
// not just the responseData.
static void _encodeOpenReferenceInstancePathsResponseBody(
    CIMBuffer& out,
    CIMOpenReferenceInstancePathsResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("OpenReferenceInstancePaths");
    name = NAME;

    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMOpenReferenceInstancePathsResponseMessage*
    _decodeOpenReferenceInstancePathsResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
        return 0;

    String enumerationContext;
    if (!in.getString(enumerationContext))
        return 0;

    CIMOpenReferenceInstancePathsResponseMessage* msg =
        new CIMOpenReferenceInstancePathsResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data right here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to getNamedInstances, which is going to resolve the binary
    // data when the callback function is registered.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}

//==============================================================================
//
// OpenAssociatorInstances
//
//==============================================================================

static void _encodeOpenAssociatorInstancesRequest(
    CIMBuffer& buf,
    CIMOpenAssociatorInstancesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("OpenAssociatorInstances");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->includeClassOrigin)
        flags |= INCLUDE_CLASS_ORIGIN;

    if (msg->continueOnError)
        flags |= CONTINUE_ON_ERROR;

    _putHeader(buf, flags, msg->messageId, OP_OpenAssociatorInstances);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [OBJECT-NAME]
    buf.putObjectPath(msg->objectName);

    // [ASSOC-CLASS]
    buf.putName(msg->assocClass);

    // [RESULT-CLASS]
    buf.putName(msg->resultClass);

    // [ROLE]
    buf.putString(msg->role);

    // [RESULT-ROLE]
    buf.putString(msg->resultRole);

    // [PROPERTY-LIST]
    buf.putPropertyList(msg->propertyList);

    buf.putUint32(msg->maxObjectCount);
    buf.putUint32Arg(msg->operationTimeout);
    buf.putString(msg->filterQueryLanguage);
    buf.putString(msg->filterQuery);
}

static CIMOpenAssociatorInstancesRequestMessage*
    _decodeOpenAssociatorInstancesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean includeClassOrigin = flags & INCLUDE_CLASS_ORIGIN;
    Boolean continueOnError = flags & CONTINUE_ON_ERROR;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [OBJECT-NAME]

    CIMObjectPath objectName;

    if (!in.getObjectPath(objectName))
        return 0;

    // [ASSOC-CLASS]

    CIMName assocClass;

    if (!in.getName(assocClass))
        return 0;

    // [RESULT-CLASS]

    CIMName resultClass;

    if (!in.getName(resultClass))
        return 0;

    // [ROLE]

    String role;

    if (!in.getString(role))
        return 0;

    // [RESULT-ROLE]

    String resultRole;

    if (!in.getString(resultRole))
        return 0;

    // [PROPERTY-LIST]
    CIMPropertyList propertyList;
    if (!in.getPropertyList(propertyList))
        return 0;

    // These can all be one common function.
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
       return 0;
    Uint32Arg operationTimeout;
    if (!in.getUint32Arg(operationTimeout))
        return 0;
    String filterQueryLanguage;
    if (!in.getString(filterQueryLanguage))
        return 0;
    String filterQuery;
    if (!in.getString(filterQuery))
        return 0;

    AutoPtr<CIMOpenAssociatorInstancesRequestMessage> request(
        new CIMOpenAssociatorInstancesRequestMessage(
            messageId,
            nameSpace,
            objectName,
            assocClass,
            resultClass,
            role,
            resultRole,
            includeClassOrigin,
            propertyList,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}
// For the pull Response messages the interface is the message and the,
// not just the responseData.
static void _encodeOpenAssociatorInstancesResponseBody(
    CIMBuffer& out,
    CIMOpenAssociatorInstancesResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("OpenAssociatorInstances");
    name = NAME;

    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMOpenAssociatorInstancesResponseMessage*
    _decodeOpenAssociatorInstancesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
        return 0;

    String enumerationContext;
    if (!in.getString(enumerationContext))
        return 0;

    CIMOpenAssociatorInstancesResponseMessage* msg =
        new CIMOpenAssociatorInstancesResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext );

    // Instead of resolving the binary data right here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to getNamedInstances, which is going to resolve the binary
    // data when the callback function is registered.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}

//==============================================================================
//
// OpenAssociatorInstancePaths
//
//==============================================================================

static void _encodeOpenAssociatorInstancePathsRequest(
    CIMBuffer& buf,
    CIMOpenAssociatorInstancePathsRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("OpenAssociatorInstancePaths");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    if (msg->continueOnError)
        flags |= CONTINUE_ON_ERROR;

    _putHeader(buf, flags, msg->messageId, OP_OpenAssociatorInstancePaths);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [OBJECT-NAME]
    buf.putObjectPath(msg->objectName);

    // [ASSOC-CLASS]
    buf.putName(msg->assocClass);

    // [RESULT-CLASS]
    buf.putName(msg->resultClass);

    // [ROLE]
    buf.putString(msg->role);

    // [RESULT-ROLE]
    buf.putString(msg->resultRole);

    buf.putUint32(msg->maxObjectCount);
    buf.putUint32Arg(msg->operationTimeout);
    buf.putString(msg->filterQueryLanguage);
    buf.putString(msg->filterQuery);
}

static CIMOpenAssociatorInstancePathsRequestMessage*
    _decodeOpenAssociatorInstancePathsRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean continueOnError = flags & CONTINUE_ON_ERROR;

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [OBJECT-NAME]

    CIMObjectPath objectName;
    if (!in.getObjectPath(objectName))
        return 0;

    // [ASSOC-CLASS]

    CIMName assocClass;
    if (!in.getName(assocClass))
        return 0;

    // [RESULT-CLASS]

    CIMName resultClass;
    if (!in.getName(resultClass))
        return 0;

    // [ROLE]

    String role;
    if (!in.getString(role))
        return 0;

    // [RESULT-ROLE]

    String resultRole;
    if (!in.getString(resultRole))
        return 0;

    // These can all be one common function.
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
       return 0;

    Uint32Arg operationTimeout;
    if (!in.getUint32Arg(operationTimeout))
        return 0;

    String filterQueryLanguage;
    if (!in.getString(filterQueryLanguage))
        return 0;

    String filterQuery;
    if (!in.getString(filterQuery))
        return 0;

    AutoPtr<CIMOpenAssociatorInstancePathsRequestMessage> request(
        new CIMOpenAssociatorInstancePathsRequestMessage(
            messageId,
            nameSpace,
            objectName,
            assocClass,
            resultClass,
            role,
            resultRole,
            filterQueryLanguage,
            filterQuery,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeOpenAssociatorInstancePathsResponseBody(
    CIMBuffer& out,
    CIMOpenAssociatorInstancePathsResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("OpenAssociatorInstancePaths");
    name = NAME;

    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMOpenAssociatorInstancePathsResponseMessage*
    _decodeOpenAssociatorInstancePathsResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
    {
        return 0;
    }

    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    CIMOpenAssociatorInstancePathsResponseMessage* msg =
        new CIMOpenAssociatorInstancePathsResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data right here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to getNamedInstances, which is going to resolve the binary
    // data when the callback function is registered.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}

//==============================================================================
//
// PullInstancesWithPath
//
//==============================================================================

static void _encodePullInstancesWithPathRequest(
    CIMBuffer& buf,
    CIMPullInstancesWithPathRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("PullInstancesWithPath");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    _putHeader(buf, flags, msg->messageId, OP_PullInstancesWithPath);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [EnumerationContext]
    buf.putString(msg->enumerationContext);

    buf.putUint32(msg->maxObjectCount);
}

static CIMPullInstancesWithPathRequestMessage*
    _decodePullInstancesWithPathRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
    {
        return 0;
    }

    // [EnumerationContext]
    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    // [MAXCOUNT]
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
    {
       return 0;
    }

    AutoPtr<CIMPullInstancesWithPathRequestMessage> request(
        new CIMPullInstancesWithPathRequestMessage(
            messageId,
            nameSpace,
            enumerationContext,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

// For the pull Response messages the interface is the message and the,
// not just the responseData.
static void _encodePullInstancesWithPathResponseBody(
    CIMBuffer& out,
    CIMPullInstancesWithPathResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("PullInstancesWithPath");
    name = NAME;
    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMPullInstancesWithPathResponseMessage*
    _decodePullInstancesWithPathResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
    {
        return 0;
    }

    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    CIMPullInstancesWithPathResponseMessage* msg =
        new CIMPullInstancesWithPathResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data right here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to getNamedInstances, which is going to resolve the binary
    // data when the callback function is registered.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}
//==============================================================================
//
// PullInstancePaths
//
//==============================================================================

static void _encodePullInstancePathsRequest(
    CIMBuffer& buf,
    CIMPullInstancePathsRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("PullInstancePaths");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    _putHeader(buf, flags, msg->messageId, OP_PullInstancePaths);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [EnumerationContext]
    buf.putString(msg->enumerationContext);

    buf.putUint32(msg->maxObjectCount);
}

static CIMPullInstancePathsRequestMessage*
    _decodePullInstancePathsRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
    {
        return 0;
    }

    // [EnumerationContext]
    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    // [MAXOBJECTCOUNT]
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
       return 0;

    AutoPtr<CIMPullInstancePathsRequestMessage> request(
        new CIMPullInstancePathsRequestMessage(
            messageId,
            nameSpace,
            enumerationContext,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

// For the pull Response messages the interface is the message and the,
// not just the responseData.
static void _encodePullInstancePathsResponseBody(
    CIMBuffer& out,
    CIMPullInstancePathsResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("PullInstancePaths");
    name = NAME;

    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMPullInstancePathsResponseMessage*
    _decodePullInstancePathsResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
    {
        return 0;
    }

    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    CIMPullInstancePathsResponseMessage* msg =
        new CIMPullInstancePathsResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data right here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to getNamedInstances, which is going to resolve the binary
    // data when the callback function is registered.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}


//==============================================================================
//
// PullInstances
//
//==============================================================================

static void _encodePullInstancesRequest(
    CIMBuffer& buf,
    CIMPullInstancesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("PullInstances");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    _putHeader(buf, flags, msg->messageId, OP_PullInstances);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [EnumerationContext]
    buf.putString(msg->enumerationContext);

    buf.putUint32(msg->maxObjectCount);
}

static CIMPullInstancesRequestMessage*
    _decodePullInstancesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
    {
        return 0;
    }

    // [EnumerationContext]
    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    // [MAXCOUNT]
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
    {
       return 0;
    }

    AutoPtr<CIMPullInstancesRequestMessage> request(
        new CIMPullInstancesRequestMessage(
            messageId,
            nameSpace,
            enumerationContext,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

// For the pull Response messages the interface is the message and the,
// not just the responseData.
static void _encodePullInstancesResponseBody(
    CIMBuffer& out,
    CIMPullInstancesResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("PullInstances");
    name = NAME;
    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMPullInstancesResponseMessage*
    _decodePullInstancesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
    {
        return 0;
    }

    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    CIMPullInstancesResponseMessage* msg =
        new CIMPullInstancesResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data right here, we delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to getNamedInstances, which is going to resolve the binary
    // data when the callback function is registered.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}
//==============================================================================
//
// CloseEnumeration
//
//==============================================================================

static void _encodeCloseEnumerationRequest(
    CIMBuffer& buf,
    CIMCloseEnumerationRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("CloseEnumeration");
    name = NAME;

    // [HEADER]

    Uint32 flags = 0;

    _putHeader(buf, flags, msg->messageId, OP_CloseEnumeration);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [EnumerationContext]
    buf.putString(msg->enumerationContext);
}

static CIMCloseEnumerationRequestMessage* _decodeCloseEnumerationRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [EnumerationContext]
    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    AutoPtr<CIMCloseEnumerationRequestMessage> request(
        new CIMCloseEnumerationRequestMessage(
            messageId,
            nameSpace,
            enumerationContext,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeCloseEnumerationResponseBody(
    CIMBuffer& out,
    CIMCloseEnumerationResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("CloseEnumeration");
    name = NAME;
}

static CIMCloseEnumerationResponseMessage*
    _decodeCloseEnumerationResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    CIMCloseEnumerationResponseMessage* msg =
        new CIMCloseEnumerationResponseMessage(
            messageId,
            cimException,
            QueueIdStack());

    msg->binaryRequest=true;
    return msg;
}
//==============================================================================
//
// EnumerationCount
//
//==============================================================================

static void _encodeEnumerationCountRequest(
    CIMBuffer& buf,
    CIMEnumerationCountRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("EnumerationCount");
    name = NAME;

    // [HEADER]
    Uint32 flags = 0;

    _putHeader(buf, flags, msg->messageId, OP_EnumerationCount);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [EnumerationContext]
    buf.putString(msg->enumerationContext);
}

static CIMEnumerationCountRequestMessage* _decodeEnumerationCountRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    // [NAMESPACE]

    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [EnumerationContext]
    String enumerationContext;
    if (!in.getString(enumerationContext))
    {
        return 0;
    }

    AutoPtr<CIMEnumerationCountRequestMessage> request(
        new CIMEnumerationCountRequestMessage(
            messageId,
            nameSpace,
            enumerationContext,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeEnumerationCountResponseBody(
    CIMBuffer& out,
    CIMEnumerationCountResponseMessage* msg,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    static const CIMName NAME("EnumerationCount");
    name = NAME;

    // [count]
    out.putUint64Arg(msg->count);
}

static CIMEnumerationCountResponseMessage*
    _decodeEnumerationCountResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;
    Uint64 count;
    if (!in.getUint64(count))
        return 0;

    CIMEnumerationCountResponseMessage* msg =
        new CIMEnumerationCountResponseMessage(
            messageId,
            cimException,
            QueueIdStack(),
            count);

    msg->binaryRequest=true;
    return msg;
}
//==============================================================================
//
// OpenQueryInstances
//
//==============================================================================

static void _encodeOpenQueryInstancesRequest(
    CIMBuffer& buf,
    CIMOpenQueryInstancesRequestMessage* msg,
    CIMName& name)
{
    /* See ../Client/CIMOperationRequestEncoder.cpp */

    static const CIMName NAME("OpenQueryInstances");

    // [HEADER]

    Uint32 flags = 0;

    if (msg->continueOnError)
        flags |= CONTINUE_ON_ERROR;

    _putHeader(buf, flags, msg->messageId, OP_OpenQueryInstances);

    // [NAMESPACE]
    buf.putNamespaceName(msg->nameSpace);

    // [filterQuery and filterQueryLanguage]
    buf.putString(msg->query);
    buf.putString(msg->queryLanguage);

    // [returnQueryResultClass]
    buf.putBoolean(msg->returnQueryResultClass);

    buf.putUint32(msg->maxObjectCount);
    buf.putUint32Arg(msg->operationTimeout);
}

static CIMOpenQueryInstancesRequestMessage*
    _decodeOpenQueryInstancesRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId,
    Uint32 flags,
    const String& messageId)
{
    /* See ../Server/CIMOperationRequestDecoder.cpp */

    STAT_GETSTARTTIME

    Boolean continueOnError = flags & CONTINUE_ON_ERROR;

    // [NAMESPACE]
    CIMNamespaceName nameSpace;
    if (!in.getNamespaceName(nameSpace))
        return 0;

    // [FILTERQUERY and FILTERQUERYSTRING]
    String filterQuery;
    if (!in.getString(filterQuery))
        return 0;
    String filterQueryLanguage;
    if (!in.getString(filterQueryLanguage))
        return 0;

    // [RETURNQUERYRESULTCLASS]
    Boolean returnQueryResultClass;
    if (!in.getBoolean(returnQueryResultClass))
        return 0;

    // These can all be one common function.
    Uint32 maxObjectCount;
    if (!in.getUint32(maxObjectCount))
       return 0;
    Uint32Arg operationTimeout;
    if (!in.getUint32Arg(operationTimeout))
        return 0;

    AutoPtr<CIMOpenQueryInstancesRequestMessage> request(
        new CIMOpenQueryInstancesRequestMessage(
            messageId,
            nameSpace,
            filterQuery,
            filterQueryLanguage,
            returnQueryResultClass,
            operationTimeout,
            continueOnError,
            maxObjectCount,
            QueueIdStack(queueId, returnQueueId)));

    request->binaryRequest = true;

    STAT_SERVERSTART

    return request.release();
}

static void _encodeOpenQueryInstancesResponseBody(
    CIMBuffer& out,
    CIMOpenQueryInstancesResponseMessage* msg,
    CIMResponseData& data,
    CIMName& name)
{
    /* See ../Server/CIMOperationResponseEncoder.cpp */

    out.putClass(msg->queryResultClass);

    // [endOfSequence]
    out.putBoolean(msg->endOfSequence);

    // [enumerationContext]
    out.putString(msg->enumerationContext);

    data.encodeBinaryResponse(out);
}

static CIMOpenQueryInstancesResponseMessage*
    _decodeOpenQueryInstancesResponse(
    CIMBuffer& in,
    const String& messageId)
{
    CIMException cimException;

    // KS_TODO Should we set validation???
    // Turn on validation: This is a debugging tool
//#if defined(ENABLE_VALIDATION)
//    buf.setValidate(true);
//#endif

    Boolean endOfSequence;
    if (!in.getBoolean(endOfSequence))
        return 0;

    String enumerationContext;
    if (!in.getString(enumerationContext))
        return 0;

    CIMOpenQueryInstancesResponseMessage* msg =
        new CIMOpenQueryInstancesResponseMessage(
            messageId,
            cimException,
            CIMClass(),
            QueueIdStack(),
            endOfSequence,
            enumerationContext);

    // Instead of resolving the binary data here, delegate this
    // to a later point in time when the data is actually retrieved through
    // a call to CIMResponseData::getInstances, which
    // resolves the binary data as it is passed to the next interface.
    // This allows an alternate client implementation to gain direct access
    // to the binary data and pass this for example to the JNI implementation
    // of the JSR48 CIM Client for Java.
    CIMResponseData& responseData = msg->getResponseData();
    responseData.setRemainingBinaryData(in);

    msg->binaryRequest=true;
    return msg;
}

// EXP_PULL_END


//==============================================================================
//
// BinaryCodec::hexDump()
//
//==============================================================================

#if defined(PEGASUS_DEBUG)

//    TODO - There are 3 hex dump functions in pegasus today,
//        here, SCMO.cpp and Tracer.  We should consolidate them

void BinaryCodec::hexDump(const void* data, size_t size)
{
    unsigned char* p = (unsigned char*)data;
    unsigned char buf[16];
    size_t n = 0;

    for (size_t i = 0, col = 0; i < size; i++)
    {
        unsigned char c = p[i];
        buf[n++] = c;

        if (col == 0)
            printf("%06X ", (unsigned int)i);

        printf("%02X ", c);

        if (col + 1 == sizeof(buf) || i + 1 == size)
        {
            for (size_t j = col + 1; j < sizeof(buf); j++)
                printf("   ");

            for (size_t j = 0; j < n; j++)
            {
                c = buf[j];

                if (c >= ' ' && c <= '~')
                    printf("%c", buf[j]);
                else
                    printf(".");
            }

            printf("\n");
            n = 0;
        }

        if (col + 1 == sizeof(buf))
            col = 0;
        else
            col++;
    }

    printf("\n");
}

#endif /* defined(PEGASUS_DEBUG) */

//==============================================================================
//
// BinaryCodec::decodeRequest()
//
//==============================================================================

CIMOperationRequestMessage* BinaryCodec::decodeRequest(
    CIMBuffer& in,
    Uint32 queueId,
    Uint32 returnQueueId)
{

    // Turn on validation:
#if defined(ENABLE_VALIDATION)
    in.setValidate(true);
#endif

    Uint32 flags;
    String messageId;
    Operation operation;


    if (!_getHeader(in, flags, messageId, operation))
    {
        return 0;
    }

    switch (operation)
    {
        case OP_EnumerateInstances:
            return _decodeEnumerateInstancesRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_EnumerateInstanceNames:
            return _decodeEnumerateInstanceNamesRequest(
                in, queueId, returnQueueId, messageId);
        case OP_GetInstance:
            return _decodeGetInstanceRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_CreateInstance:
            return _decodeCreateInstanceRequest(
                in, queueId, returnQueueId, messageId);
        case OP_ModifyInstance:
            return _decodeModifyInstanceRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_DeleteInstance:
            return _decodeDeleteInstanceRequest(
                in, queueId, returnQueueId, messageId);
        case OP_Associators:
            return _decodeAssociatorsRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_AssociatorNames:
            return _decodeAssociatorNamesRequest(
                in, queueId, returnQueueId, messageId);
        case OP_References:
            return _decodeReferencesRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_ReferenceNames:
            return _decodeReferenceNamesRequest(
                in, queueId, returnQueueId, messageId);
        case OP_GetClass:
            return _decodeGetClassRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_EnumerateClasses:
            return _decodeEnumerateClassesRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_EnumerateClassNames:
            return _decodeEnumerateClassNamesRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_CreateClass:
            return _decodeCreateClassRequest(
                in, queueId, returnQueueId, messageId);
        case OP_DeleteClass:
            return _decodeDeleteClassRequest(
                in, queueId, returnQueueId, messageId);
        case OP_ModifyClass:
            return _decodeModifyClassRequest(
                in, queueId, returnQueueId, messageId);
        case OP_SetQualifier:
            return _decodeSetQualifierRequest(
                in, queueId, returnQueueId, messageId);
        case OP_GetQualifier:
            return _decodeGetQualifierRequest(
                in, queueId, returnQueueId, messageId);
        case OP_DeleteQualifier:
            return _decodeDeleteQualifierRequest(
                in, queueId, returnQueueId, messageId);
        case OP_EnumerateQualifiers:
            return _decodeEnumerateQualifiersRequest(
                in, queueId, returnQueueId, messageId);
        case OP_GetProperty:
           return _decodeGetPropertyRequest(
                in, queueId, returnQueueId, messageId);
        case OP_SetProperty:
           return _decodeSetPropertyRequest(
                in, queueId, returnQueueId, messageId);
        case OP_InvokeMethod:
           return _decodeInvokeMethodRequest(
                in, queueId, returnQueueId, messageId);
        case OP_ExecQuery:
           return _decodeExecQueryRequest(
                in, queueId, returnQueueId, messageId);
//EXP_PULL_BEGIN
        case OP_OpenEnumerateInstances:
            return _decodeOpenEnumerateInstancesRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_OpenEnumerateInstancePaths:
            return _decodeOpenEnumerateInstancePathsRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_OpenReferenceInstances:
            return _decodeOpenReferenceInstancesRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_OpenReferenceInstancePaths:
            return _decodeOpenReferenceInstancePathsRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_OpenAssociatorInstances:
            return _decodeOpenAssociatorInstancesRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_OpenAssociatorInstancePaths:
            return _decodeOpenAssociatorInstancePathsRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_PullInstancesWithPath:
            return _decodePullInstancesWithPathRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_PullInstancePaths:
            return _decodePullInstancePathsRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_CloseEnumeration:
            return _decodeCloseEnumerationRequest(
                in, queueId, returnQueueId, flags, messageId);
        case OP_EnumerationCount:
            return _decodeEnumerationCountRequest(
                in, queueId, returnQueueId, flags, messageId);
//EXP_PULL_END
        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            return 0;
    }
}

//==============================================================================
//
// BinaryCodec::decodeResponse()
//
//==============================================================================


CIMResponseMessage* BinaryCodec::decodeResponse(
    CIMBuffer& buf)
{
    // Turn on validation: This is a debugging tool
#if defined(ENABLE_VALIDATION)
    buf.setValidate(true);
#endif

    Uint32 flags;
    String messageId;
    Operation operation;

    if (!_getHeader(buf, flags, messageId, operation))
    {
        throw CIMException(CIM_ERR_FAILED, "Corrupt binary message header");
        return 0;
    }

    CIMResponseMessage* msg = 0;

    switch (operation)
    {
        case OP_EnumerateInstances:
            msg = _decodeEnumerateInstancesResponse(buf, messageId);
            break;
        case OP_EnumerateInstanceNames:
            msg = _decodeEnumerateInstanceNamesResponse(buf, messageId);
            break;
        case OP_GetInstance:
            msg = _decodeGetInstanceResponse(buf, messageId);
            break;
        case OP_CreateInstance:
            msg = _decodeCreateInstanceResponse(buf, messageId);
            break;
        case OP_ModifyInstance:
            msg = _decodeModifyInstanceResponse(buf, messageId);
            break;
        case OP_DeleteInstance:
            msg = _decodeDeleteInstanceResponse(messageId);
            break;
        case OP_Associators:
            msg = _decodeAssociatorsResponse(buf, messageId);
            break;
        case OP_AssociatorNames:
            msg = _decodeAssociatorNamesResponse(buf, messageId);
            break;
        case OP_References:
            msg = _decodeReferencesResponse(buf, messageId);
            break;
        case OP_ReferenceNames:
            msg = _decodeReferenceNamesResponse(buf, messageId);
            break;
        case OP_GetClass:
            msg = _decodeGetClassResponse(buf, messageId);
            break;
        case OP_EnumerateClasses:
            msg = _decodeEnumerateClassesResponse(buf, messageId);
            break;
        case OP_EnumerateClassNames:
            msg = _decodeEnumerateClassNamesResponse(buf, messageId);
            break;
        case OP_CreateClass:
            msg = _decodeCreateClassResponse(messageId);
            break;
        case OP_DeleteClass:
            msg = _decodeDeleteClassResponse(messageId);
            break;
        case OP_ModifyClass:
            msg = _decodeModifyClassResponse(messageId);
            break;
        case OP_SetQualifier:
            msg = _decodeSetQualifierResponse(messageId);
            break;
        case OP_GetQualifier:
            msg = _decodeGetQualifierResponse(buf, messageId);
            break;
        case OP_DeleteQualifier:
            msg = _decodeDeleteQualifierResponse(messageId);
            break;
        case OP_EnumerateQualifiers:
            msg = _decodeEnumerateQualifiersResponse(buf, messageId);
            break;
        case OP_GetProperty:
            msg = _decodeGetPropertyResponse(buf, messageId);
            break;
        case OP_SetProperty:
            msg = _decodeSetPropertyResponse(messageId);
            break;
        case OP_InvokeMethod:
            msg = _decodeInvokeMethodResponse(buf, messageId);
            break;
        case OP_ExecQuery:
            msg = _decodeExecQueryResponse(buf, messageId);
            break;
//EXP_PULL_BEGIN
        case OP_OpenEnumerateInstances:
            msg = _decodeOpenEnumerateInstancesResponse(buf, messageId);
            break;
        case OP_OpenEnumerateInstancePaths:
            msg = _decodeOpenEnumerateInstancePathsResponse(buf, messageId);
            break;
        case OP_OpenReferenceInstances:
            msg = _decodeOpenReferenceInstancesResponse(buf, messageId);
            break;
        case OP_OpenReferenceInstancePaths:
            msg = _decodeOpenReferenceInstancePathsResponse(buf, messageId);
            break;
        case OP_OpenAssociatorInstances:
            msg = _decodeOpenAssociatorInstancesResponse(buf, messageId);
            break;
        case OP_OpenAssociatorInstancePaths:
            msg = _decodeOpenAssociatorInstancePathsResponse(buf, messageId);
            break;
        case OP_PullInstancesWithPath:
            msg = _decodePullInstancesWithPathResponse(buf, messageId);
            break;
        case OP_PullInstancePaths:
            msg = _decodePullInstancePathsResponse(buf, messageId);
            break;
        case OP_CloseEnumeration:
            msg = _decodeCloseEnumerationResponse(buf, messageId);
            break;
        case OP_OpenQueryInstances:
            msg = _decodeCloseEnumerationResponse(buf, messageId);
            break;
//EXP_PULL_END
        default:
            // Unexpected message type
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
    }

    if (!msg)
        throw CIMException(CIM_ERR_FAILED, "Received corrupted binary message");

    return msg;
}

//==============================================================================
//
// BinaryCodec::formatSimpleIMethodRspMessage()
//
//==============================================================================

Buffer BinaryCodec::formatSimpleIMethodRspMessage(
    const CIMName& iMethodName,
    const String& messageId,
    HttpMethod httpMethod,
    const ContentLanguageList& httpContentLanguages,
    const Buffer& rtnParams,
    const Buffer& body,
    Uint64 serverResponseTime,
    Boolean isFirst,
    Boolean)
{
    Buffer out;

    if (isFirst == true)
    {
        // Write HTTP header:
        XmlWriter::appendMethodResponseHeader(out, httpMethod,
            httpContentLanguages, 0, serverResponseTime, true);

        for (size_t i=out.size(), k=CIMBuffer::round(i); i<k;i++)
        {
            out.append('\0');
        }
        // Binary message header:
        CIMBuffer cb(128);
        _putHeader(cb, 0, messageId, _NameToOp(iMethodName));
        out.append(cb.getData(), cb.size());
    }

    if (body.size() != 0)
    {
        out.append(body.getData(), body.size());
    }
    // EXP_PULL_BEGIN
    // If there are any output parameters include them here.
    // Assumes that it is prebuilt with all components
    //
    if (rtnParams.size() != 0)
    {
        out << rtnParams;
    }
    //
    // EXP_PULL_END

    return out;
}

//==============================================================================
//
// BinaryCodec::encodeRequest()
//
//==============================================================================

bool BinaryCodec::encodeRequest(
    Buffer& out,
    const char* host,
    const String& authHeader,
    CIMOperationRequestMessage* msg,
    bool binaryResponse)
{
    CIMBuffer buf;
    CIMName name;

    switch (msg->getType())
    {
        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
        {
            _encodeEnumerateInstancesRequest(buf,
                (CIMEnumerateInstancesRequestMessage*)msg, name);
            break;
        }

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
        {
            _encodeEnumerateInstanceNamesRequest(buf,
                (CIMEnumerateInstanceNamesRequestMessage*)msg, name);
            break;
        }

        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
        {
            _encodeGetInstanceRequest(buf,
                (CIMGetInstanceRequestMessage*)msg, name);
            break;
        }

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
        {
            _encodeCreateInstanceRequest(buf,
                (CIMCreateInstanceRequestMessage*)msg, name);
            break;
        }

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
        {
            _encodeModifyInstanceRequest(buf,
                (CIMModifyInstanceRequestMessage*)msg, name);
            break;
        }

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
        {
            _encodeDeleteInstanceRequest(buf,
                (CIMDeleteInstanceRequestMessage*)msg, name);
            break;
        }

        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
        {
            _encodeAssociatorsRequest(buf,
                (CIMAssociatorsRequestMessage*)msg, name);
            break;
        }

        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
        {
            _encodeAssociatorNamesRequest(buf,
                (CIMAssociatorNamesRequestMessage*)msg, name);
            break;
        }

        case CIM_REFERENCES_REQUEST_MESSAGE:
        {
            _encodeReferencesRequest(buf,
                (CIMReferencesRequestMessage*)msg, name);
            break;
        }

        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
        {
            _encodeReferenceNamesRequest(buf,
                (CIMReferenceNamesRequestMessage*)msg, name);
            break;
        }

        case CIM_GET_CLASS_REQUEST_MESSAGE:
        {
            _encodeGetClassRequest(buf,
                (CIMGetClassRequestMessage*)msg, name);
            break;
        }

        case CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE:
        {
            _encodeEnumerateClassesRequest(buf,
                (CIMEnumerateClassesRequestMessage*)msg, name);
            break;
        }

        case CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE:
        {
            _encodeEnumerateClassNamesRequest(buf,
                (CIMEnumerateClassNamesRequestMessage*)msg, name);
            break;
        }

        case CIM_CREATE_CLASS_REQUEST_MESSAGE:
        {
            _encodeCreateClassRequest(buf,
                (CIMCreateClassRequestMessage*)msg, name);
            break;
        }

        case CIM_DELETE_CLASS_REQUEST_MESSAGE:
        {
            _encodeDeleteClassRequest(buf,
                (CIMDeleteClassRequestMessage*)msg, name);
            break;
        }

        case CIM_MODIFY_CLASS_REQUEST_MESSAGE:
        {
            _encodeModifyClassRequest(buf,
                (CIMModifyClassRequestMessage*)msg, name);
            break;
        }

        case CIM_SET_QUALIFIER_REQUEST_MESSAGE:
        {
            _encodeSetQualifierRequest(buf,
                (CIMSetQualifierRequestMessage*)msg, name);
            break;
        }

        case CIM_GET_QUALIFIER_REQUEST_MESSAGE:
        {
            _encodeGetQualifierRequest(buf,
                (CIMGetQualifierRequestMessage*)msg, name);
            break;
        }

        case CIM_DELETE_QUALIFIER_REQUEST_MESSAGE:
        {
            _encodeDeleteQualifierRequest(buf,
                (CIMDeleteQualifierRequestMessage*)msg, name);
            break;
        }

        case CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE:
        {
            _encodeEnumerateQualifiersRequest(buf,
                (CIMEnumerateQualifiersRequestMessage*)msg, name);
            break;
        }

        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
        {
            _encodeGetPropertyRequest(buf,
                (CIMGetPropertyRequestMessage*)msg, name);
            break;
        }

        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
        {
            _encodeSetPropertyRequest(buf,
                (CIMSetPropertyRequestMessage*)msg, name);
            break;
        }

        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
        {
            _encodeInvokeMethodRequest(buf,
                (CIMInvokeMethodRequestMessage*)msg, name);
            break;
        }

        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
        {
            _encodeExecQueryRequest(buf,
                (CIMExecQueryRequestMessage*)msg, name);
            break;
        }

//EXP_PULL_BEGIN
        case CIM_OPEN_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
        {
            _encodeOpenEnumerateInstancesRequest(buf,
                (CIMOpenEnumerateInstancesRequestMessage*)msg, name);
            break;
        }

        case CIM_OPEN_ENUMERATE_INSTANCE_PATHS_REQUEST_MESSAGE:
        {
            _encodeOpenEnumerateInstancePathsRequest(buf,
                (CIMOpenEnumerateInstancePathsRequestMessage*)msg, name);
            break;
        }
        case CIM_OPEN_ASSOCIATOR_INSTANCES_REQUEST_MESSAGE:
        {
            _encodeOpenAssociatorInstancesRequest(buf,
                (CIMOpenAssociatorInstancesRequestMessage*)msg, name);
            break;
        }
        case CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_REQUEST_MESSAGE:
        {
            _encodeOpenAssociatorInstancePathsRequest(buf,
                (CIMOpenAssociatorInstancePathsRequestMessage*)msg, name);
            break;
        }
        case CIM_OPEN_REFERENCE_INSTANCES_REQUEST_MESSAGE:
        {
            _encodeOpenReferenceInstancesRequest(buf,
                (CIMOpenReferenceInstancesRequestMessage*)msg, name);
            break;
        }
        case CIM_OPEN_REFERENCE_INSTANCE_PATHS_REQUEST_MESSAGE:
        {
            _encodeOpenReferenceInstancePathsRequest(buf,
                (CIMOpenReferenceInstancePathsRequestMessage*)msg, name);
            break;
        }
        case CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE:
        {
            _encodePullInstancesWithPathRequest(buf,
                (CIMPullInstancesWithPathRequestMessage*)msg, name);
            break;
        }
        case CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE:
        {
            _encodePullInstancePathsRequest(buf,
                (CIMPullInstancePathsRequestMessage*)msg, name);
            break;
        }
        case CIM_CLOSE_ENUMERATION_REQUEST_MESSAGE:
        {
            _encodeCloseEnumerationRequest(buf,
                (CIMCloseEnumerationRequestMessage*)msg, name);
            break;
        }
        case CIM_ENUMERATION_COUNT_REQUEST_MESSAGE:
        {
            _encodeEnumerationCountRequest(buf,
                (CIMEnumerationCountRequestMessage*)msg, name);
            break;
        }
        case CIM_OPEN_QUERY_INSTANCES_REQUEST_MESSAGE:
        {
            _encodeOpenQueryInstancesRequest(buf,
                (CIMOpenQueryInstancesRequestMessage*)msg, name);
            break;
        }
//EXP_PULL_END

        default:
            // Unexpected message type
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            return false;
    }

    // [HTTP-HEADERS]
    XmlWriter::appendMethodCallHeader(
        out,
        host,
        name,
        msg->nameSpace.getString(),
        authHeader,
        msg->getHttpMethod(),
        AcceptLanguageListContainer(msg->operationContext.get(
            AcceptLanguageListContainer::NAME)).getLanguages(),
        ContentLanguageListContainer(msg->operationContext.get(
            ContentLanguageListContainer::NAME)).getLanguages(),
        buf.size(),
        true, /* binaryRequest */
        binaryResponse);

    // Need to pad the Buffer to the 64bit border since CIMBuffer is 64bit
    // aligned, but Buffer only 8bit
    Uint32 extraAlignBytes = CIMBuffer::round(out.size()) - out.size();
    for (Uint32 i=0; i < extraAlignBytes;i++)
    {
        out.append('\0');
    }
    // Need fix-up Content-length value...
    char * contentLengthValueStart =
        (char*) strstr(out.getData(), "content-length");
    contentLengthValueStart += sizeof("content-length: ")-1;
    // using sprintf to stay equal to the macro OUTPUT_CONTENTLENGTH definition
    // defined in XMLGenerator.h
    char contentLengthP[11];
    sprintf(contentLengthP,"%.10u", (unsigned int)buf.size()+extraAlignBytes);
    memcpy(contentLengthValueStart,contentLengthP,10);

    out.append(buf.getData(), buf.size());

    return true;
}

//==============================================================================
//
// BinaryCodec::encodeResponseBody()
//
//==============================================================================

bool BinaryCodec::encodeResponseBody(
    Buffer& out,
    const CIMResponseMessage* msg,
    CIMName& name)
{
    CIMBuffer buf;

    switch (msg->getType())
    {
        case CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE:
        {
            _encodeEnumerateInstancesResponseBody(
                buf,
                ((CIMEnumerateInstancesResponseMessage*)msg)->getResponseData(),
                name,
                (msg->getIndex() == 0));
            break;
        }

        case CIM_ENUMERATE_INSTANCE_NAMES_RESPONSE_MESSAGE:
        {
            _encodeEnumerateInstanceNamesResponseBody(
            buf,
            ((CIMEnumerateInstanceNamesResponseMessage*)msg)->getResponseData(),
            name);
            break;
        }

        case CIM_GET_INSTANCE_RESPONSE_MESSAGE:
        {
            _encodeGetInstanceResponseBody(
                buf,
                ((CIMGetInstanceResponseMessage*)msg)->getResponseData(),
                name);
            break;
        }

        case CIM_CREATE_INSTANCE_RESPONSE_MESSAGE:
        {
            _encodeCreateInstanceResponseBody(buf,
                (CIMCreateInstanceResponseMessage*)msg, name);
            break;
        }

        case CIM_MODIFY_INSTANCE_RESPONSE_MESSAGE:
        {
            _encodeModifyInstanceResponseBody(name);
            break;
        }

        case CIM_DELETE_INSTANCE_RESPONSE_MESSAGE:
        {
            _encodeDeleteInstanceResponseBody(name);
            break;
        }

        case CIM_ASSOCIATORS_RESPONSE_MESSAGE:
        {
            _encodeAssociatorsResponseBody(
                buf,
                ((CIMAssociatorsResponseMessage*)msg)->getResponseData(),
                name);
            break;
        }

        case CIM_ASSOCIATOR_NAMES_RESPONSE_MESSAGE:
        {
            _encodeAssociatorNamesResponseBody(
                buf,
                ((CIMAssociatorNamesResponseMessage*)msg)->getResponseData(),
                name);
            break;
        }

        case CIM_REFERENCES_RESPONSE_MESSAGE:
        {
            _encodeReferencesResponseBody(
                buf,
                ((CIMReferencesResponseMessage*)msg)->getResponseData(),
                name);
            break;
        }

        case CIM_REFERENCE_NAMES_RESPONSE_MESSAGE:
        {
            _encodeReferenceNamesResponseBody(
                buf,
                ((CIMReferenceNamesResponseMessage*)msg)->getResponseData(),
                name);
            break;
        }

        case CIM_GET_CLASS_RESPONSE_MESSAGE:
        {
            _encodeGetClassResponseBody(buf,
                (CIMGetClassResponseMessage*)msg, name);
            break;
        }

        case CIM_ENUMERATE_CLASSES_RESPONSE_MESSAGE:
        {
            _encodeEnumerateClassesResponseBody(buf,
                (CIMEnumerateClassesResponseMessage*)msg, name);
            break;
        }

        case CIM_ENUMERATE_CLASS_NAMES_RESPONSE_MESSAGE:
        {
            _encodeEnumerateClassNamesResponseBody(buf,
                (CIMEnumerateClassNamesResponseMessage*)msg, name);
            break;
        }

        case CIM_CREATE_CLASS_RESPONSE_MESSAGE:
        {
            _encodeCreateClassResponseBody(name);
            break;
        }

        case CIM_DELETE_CLASS_RESPONSE_MESSAGE:
        {
            _encodeDeleteClassResponseBody(name);
            break;
        }

        case CIM_MODIFY_CLASS_RESPONSE_MESSAGE:
        {
            _encodeModifyClassResponseBody(name);
            break;
        }

        case CIM_SET_QUALIFIER_RESPONSE_MESSAGE:
        {
            _encodeSetQualifierResponseBody(name);
            break;
        }

        case CIM_GET_QUALIFIER_RESPONSE_MESSAGE:
        {
            _encodeGetQualifierResponseBody(buf,
                (CIMGetQualifierResponseMessage*)msg, name);
            break;
        }

        case CIM_DELETE_QUALIFIER_RESPONSE_MESSAGE:
        {
            _encodeDeleteQualifierResponseBody(name);
            break;
        }

        case CIM_ENUMERATE_QUALIFIERS_RESPONSE_MESSAGE:
        {
            _encodeEnumerateQualifiersResponseBody(buf,
                (CIMEnumerateQualifiersResponseMessage*)msg, name);
            break;
        }

        case CIM_GET_PROPERTY_RESPONSE_MESSAGE:
        {
            _encodeGetPropertyResponseBody(buf,
                (CIMGetPropertyResponseMessage*)msg, name);
            break;
        }

        case CIM_SET_PROPERTY_RESPONSE_MESSAGE:
        {
            _encodeSetPropertyResponseBody(name);
            break;
        }

        case CIM_INVOKE_METHOD_RESPONSE_MESSAGE:
        {
            _encodeInvokeMethodResponseBody(buf,
                (CIMInvokeMethodResponseMessage*)msg, name);
            break;
        }

        case CIM_EXEC_QUERY_RESPONSE_MESSAGE:
        {
            _encodeExecQueryResponseBody(
                buf,
                ((CIMExecQueryResponseMessage*)msg)->getResponseData(),
                name);
            break;
        }

//EXP_PULL_BEGIN
        case CIM_OPEN_ENUMERATE_INSTANCES_RESPONSE_MESSAGE:
        {
            _encodeOpenEnumerateInstancesResponseBody(buf,
                (CIMOpenEnumerateInstancesResponseMessage*)msg,
                ((CIMOpenEnumerateInstancesResponseMessage*)msg)->
                                                      getResponseData(),
                name);
            break;
        }

        case CIM_OPEN_ENUMERATE_INSTANCE_PATHS_RESPONSE_MESSAGE:
        {
            _encodeOpenEnumerateInstancePathsResponseBody(buf,
                (CIMOpenEnumerateInstancePathsResponseMessage*)msg,
                ((CIMOpenEnumerateInstancePathsResponseMessage*)msg)->
                                                          getResponseData(),
                name);
            break;
        }

        case CIM_OPEN_REFERENCE_INSTANCES_RESPONSE_MESSAGE:
        {
            _encodeOpenReferenceInstancesResponseBody(buf,
                (CIMOpenReferenceInstancesResponseMessage*)msg,
                ((CIMOpenReferenceInstancesResponseMessage*)msg)->
                                                      getResponseData(),
                name);
            break;
        }

        case CIM_OPEN_REFERENCE_INSTANCE_PATHS_RESPONSE_MESSAGE:
        {
            _encodeOpenReferenceInstancePathsResponseBody(buf,
                (CIMOpenReferenceInstancePathsResponseMessage*)msg,
                ((CIMOpenReferenceInstancePathsResponseMessage*)msg)->
                                                          getResponseData(),
                name);
            break;
        }
        case CIM_OPEN_ASSOCIATOR_INSTANCES_RESPONSE_MESSAGE:
        {
            _encodeOpenAssociatorInstancesResponseBody(buf,
                (CIMOpenAssociatorInstancesResponseMessage*)msg,
                ((CIMOpenAssociatorInstancesResponseMessage*)msg)->
                                                      getResponseData(),
                name);
            break;
        }

        case CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_RESPONSE_MESSAGE:
        {
            _encodeOpenAssociatorInstancePathsResponseBody(buf,
                (CIMOpenAssociatorInstancePathsResponseMessage*)msg,
                ((CIMOpenAssociatorInstancePathsResponseMessage*)msg)->
                                                          getResponseData(),
                name);
            break;
        }

        case CIM_PULL_INSTANCES_WITH_PATH_RESPONSE_MESSAGE:
        {
            _encodePullInstancesWithPathResponseBody(buf,
                (CIMPullInstancesWithPathResponseMessage*)msg,
                ((CIMPullInstancesWithPathResponseMessage*)msg)->
                                                      getResponseData(),
                name);
            break;
        }

        case CIM_PULL_INSTANCE_PATHS_RESPONSE_MESSAGE:
        {
            _encodePullInstancePathsResponseBody(buf,
                (CIMPullInstancePathsResponseMessage*)msg,
                ((CIMPullInstancePathsResponseMessage*)msg)->
                                                      getResponseData(),
                name);
            break;
        }
        case CIM_CLOSE_ENUMERATION_RESPONSE_MESSAGE:
        {
            _encodeCloseEnumerationResponseBody(buf,
                (CIMCloseEnumerationResponseMessage*)msg,
                name);
            break;
        }
        case CIM_ENUMERATION_COUNT_RESPONSE_MESSAGE:
        {
            _encodeEnumerationCountResponseBody(buf,
                (CIMEnumerationCountResponseMessage*)msg,
                name);
            break;
        }
//EXP_PULL_END

        default:
            // Unexpected message type
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            return false;
    }

    out.append(buf.getData(), buf.size());
    return true;
}

PEGASUS_NAMESPACE_END
