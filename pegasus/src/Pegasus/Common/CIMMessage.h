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

#ifndef Pegasus_CIMMessage_h
#define Pegasus_CIMMessage_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/Pair.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/Threads.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/UintArgs.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/IndicationRouter.h>

/*   ProviderType should become part of Pegasus/Common     PEP# 99
   #include <Pegasus/ProviderManager2/ProviderType.h>
   #define TYPE_INSTANCE    ProviderType::INSTANCE
   #define TYPE_CLASS       ProviderType::CLASS
   #define TYPE_METHOD      ProviderType::METHOD
   #define TYPE_ASSOCIATION ProviderType::ASSOCIATION
   #define TYPE_QUERY       ProviderType::QUERY
*/
// using these equations instead      PEP# 99
#define TYPE_CLASS       1
#define TYPE_INSTANCE    2
#define TYPE_ASSOCIATION 3
#define TYPE_METHOD      5
#define TYPE_QUERY       7


PEGASUS_NAMESPACE_BEGIN

/*
 * Please DO NOT make any constructor of CIMMessage(s) inline as it bloats code
 * Instead define the constructor in CIMMessage.cpp,
 * Bug 9580 has details
*/

class PEGASUS_COMMON_LINKAGE CIMMessage : public Message
{
public:

    CIMMessage(MessageType type, const String& messageId_);

    /**
        Updates the language context for the thread to the contents of the
        AcceptLanguageContainer in the OperationContext.
    */
    void updateThreadLanguages()
    {
        if (!Threads::equal(_languageContextThreadId, Threads::self()))
        {
            Thread::setLanguages(
                ((AcceptLanguageListContainer)operationContext.get(
                    AcceptLanguageListContainer::NAME)).getLanguages());
            _languageContextThreadId = Threads::self();
        }
    }

#ifndef PEGASUS_DISABLE_PERFINST
    //
    // Needed for performance measurement
    //

    Uint64 getServerStartTime() const
    {
        return _serverStartTimeMicroseconds;
    }

    void setServerStartTime(Uint64 serverStartTimeMicroseconds)
    {
        _serverStartTimeMicroseconds = serverStartTimeMicroseconds;
    }

    void endServer();

    Uint64 getProviderTime() const
    {
        return _providerTimeMicroseconds;
    }

    void setProviderTime(Uint64 providerTimeMicroseconds)
    {
        _providerTimeMicroseconds = providerTimeMicroseconds;
    }

    Uint64 getTotalServerTime() const
    {
        return _totalServerTimeMicroseconds;
    }

    void setTotalServerTime(Uint64 totalServerTimeMicroseconds)
    {
        _totalServerTimeMicroseconds = totalServerTimeMicroseconds;
    }
#endif

    String messageId;
    OperationContext operationContext;

    // This flag indicates that the original request was a binary request.
    // That is the HTTP "Content-Type" header had a value of
    // "application/x-openpegasus". It does not necessarily follow that
    // the response to this request must also be binary. Binary requests
    // may have XML responses.
    Boolean binaryRequest;

    // This flag indications that the ultimate response to this message
    // must be sent as binary response. This means the original request's
    // "Accept" HTTP header had a value of "application/x-openpegasus".
    // Note that a binary response can be sent to an XML request as long
    // as the "Accept" header is "application/x-openpegasus".
    Boolean binaryResponse;
// EXP_PULL_BEGIN
    // Defines Request Operations that were created internally rather than
    // from a client.  For version 2.14 that is the internal provider
    // requests for the Pull Operations (ex. EnumerateInstances when
    // OpenEnumerateInstances is received). This allows the internal
    // code (ex. OOP) to discriminate between external operations and
    // the internal provider operations since there are some behavior
    // differences (ex. EnumerateInstances paths in response, handling
    // of statistics). Normally set by CIMOperationRequestDispatcher.
    // Defaults to false and must be specifically set to true.
    Boolean internalOperation;
// EXP_PULL_END
private:

    ThreadType _languageContextThreadId;

#ifndef PEGASUS_DISABLE_PERFINST
    Uint64 _serverStartTimeMicroseconds;
    Uint64 _providerTimeMicroseconds;
    Uint64 _totalServerTimeMicroseconds;
#endif
};


class CIMResponseMessage;

class PEGASUS_COMMON_LINKAGE CIMRequestMessage : public CIMMessage
{
public:
    CIMRequestMessage(
        MessageType type_,
        const String& messageId_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const = 0;

    QueueIdStack queueIds;
};


class PEGASUS_COMMON_LINKAGE CIMResponseMessage : public CIMMessage
{
public:

    CIMResponseMessage(
        MessageType type_,
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        Boolean isAsyncResponsePending=false);

    /* Sync attributes from the request to the response including:
       Mask, HttpMethod, closeConnect, Response type (binary, xml)
       Server start time.
    */
    void syncAttributes(const CIMRequestMessage* request);

    QueueIdStack queueIds;
    CIMException cimException;

    // This flag indicates if the response will arrive asynchronously.
    Boolean isAsyncResponsePending;
};

//
// CIMRequestMessages
//
class PEGASUS_COMMON_LINKAGE CIMOperationRequestMessage
    : public CIMRequestMessage
{
public:

    CIMOperationRequestMessage(
        MessageType type_,
        const String& messageId_,
        const QueueIdStack& queueIds_,
        const String& authType_,
        const String& userName_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Uint32 providerType_ = TYPE_INSTANCE);

    String authType;
    String userName;
    String ipAddress;
    CIMNamespaceName nameSpace;
    CIMName className;
    Uint32 providerType;
};

class PEGASUS_COMMON_LINKAGE CIMIndicationRequestMessage
    : public CIMRequestMessage
{
public:
    CIMIndicationRequestMessage(
        MessageType type_,
        const String & messageId_,
        const QueueIdStack& queueIds_,
        const String& authType_,
        const String& userName_);

    String authType;
    String userName;
};

class PEGASUS_COMMON_LINKAGE CIMGetClassRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMGetClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Boolean localOnly_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    Boolean localOnly;
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;
};

class PEGASUS_COMMON_LINKAGE CIMGetInstanceRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMGetInstanceRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath instanceName;
    Boolean localOnly;    // deprecated
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;
};

class PEGASUS_COMMON_LINKAGE CIMExportIndicationRequestMessage
    : public CIMRequestMessage
{
public:
    CIMExportIndicationRequestMessage(
        const String& messageId_,
        const String& destinationPath_,
        const CIMInstance& indicationInstance_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    String destinationPath;
    CIMInstance indicationInstance;
    String authType;
    String userName;
    String ipAddress;
};

class PEGASUS_COMMON_LINKAGE CIMDeleteClassRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMDeleteClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;
};

class PEGASUS_COMMON_LINKAGE CIMDeleteInstanceRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMDeleteInstanceRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath instanceName;
};

class PEGASUS_COMMON_LINKAGE CIMCreateClassRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMCreateClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMClass& newClass_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMClass newClass;
};

class PEGASUS_COMMON_LINKAGE CIMCreateInstanceRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMCreateInstanceRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMInstance& newInstance_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMInstance newInstance;
};

class PEGASUS_COMMON_LINKAGE CIMModifyClassRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMModifyClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMClass& modifiedClass_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMClass modifiedClass;
};

class PEGASUS_COMMON_LINKAGE CIMModifyInstanceRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMModifyInstanceRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMInstance& modifiedInstance_,
        Boolean includeQualifiers_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMInstance modifiedInstance;
    Boolean includeQualifiers;
    CIMPropertyList propertyList;
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateClassesRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMEnumerateClassesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Boolean deepInheritance_,
        Boolean localOnly_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    Boolean deepInheritance;
    Boolean localOnly;
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateClassNamesRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMEnumerateClassNamesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Boolean deepInheritance_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    Boolean deepInheritance;
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateInstancesRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMEnumerateInstancesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        Boolean deepInheritance_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    Boolean deepInheritance;
    Boolean localOnly;    // deprecated
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateInstanceNamesRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMEnumerateInstanceNamesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;
};


class PEGASUS_COMMON_LINKAGE CIMExecQueryRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMExecQueryRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& queryLanguage_,
        const String& query_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    String queryLanguage;
    String query;
};

class PEGASUS_COMMON_LINKAGE CIMAssociatorsRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMAssociatorsRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& assocClass_,
        const CIMName& resultClass_,
        const String& role_,
        const String& resultRole_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        Boolean isClassRequest = false,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath objectName;
    CIMName assocClass;
    CIMName resultClass;
    String role;
    String resultRole;
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;
    Boolean isClassRequest;    // is request for classes or instances
};

class PEGASUS_COMMON_LINKAGE CIMAssociatorNamesRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMAssociatorNamesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& assocClass_,
        const CIMName& resultClass_,
        const String& role_,
        const String& resultRole_,
        const QueueIdStack& queueIds_,
        Boolean isClassRequest = false,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath objectName;
    CIMName assocClass;
    CIMName resultClass;
    String role;
    String resultRole;
    Boolean isClassRequest;  // is request for classes or instances
};

class PEGASUS_COMMON_LINKAGE CIMReferencesRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMReferencesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& resultClass_,
        const String& role_,
        Boolean includeQualifiers_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const QueueIdStack& queueIds_,
        Boolean isClassRequest_ = false,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath objectName;
    CIMName resultClass;
    String role;
    Boolean includeQualifiers;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;
    Boolean isClassRequest;            // is request for classes or instances
};

class PEGASUS_COMMON_LINKAGE CIMReferenceNamesRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMReferenceNamesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& resultClass_,
        const String& role_,
        const QueueIdStack& queueIds_,
        Boolean isClassRequest = false,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath objectName;
    CIMName resultClass;
    String role;
    Boolean isClassRequest;          // is request for classes or instances
};

class PEGASUS_COMMON_LINKAGE CIMGetPropertyRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMGetPropertyRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        const CIMName& propertyName_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath instanceName;
    CIMName propertyName;
};

class PEGASUS_COMMON_LINKAGE CIMSetPropertyRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMSetPropertyRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        const CIMName& propertyName_,
        const CIMValue& newValue_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath instanceName;
    CIMName propertyName;
    CIMValue newValue;
};

class PEGASUS_COMMON_LINKAGE CIMGetQualifierRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMGetQualifierRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& qualifierName_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMName qualifierName;
};

class PEGASUS_COMMON_LINKAGE CIMSetQualifierRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMSetQualifierRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMQualifierDecl& qualifierDeclaration_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMQualifierDecl qualifierDeclaration;
};

class PEGASUS_COMMON_LINKAGE CIMDeleteQualifierRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMDeleteQualifierRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& qualifierName_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMName qualifierName;
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateQualifiersRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMEnumerateQualifiersRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;
};

class PEGASUS_COMMON_LINKAGE CIMInvokeMethodRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMInvokeMethodRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& instanceName_,
        const CIMName& methodName_,
        const Array<CIMParamValue>& inParameters_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath instanceName;
    CIMName methodName;
    Array<CIMParamValue> inParameters;
};

//EXP_PULL_BEGIN
// Intermediate message in the hiearchy to capture all of the common
// attributes of the various CIMOpen... messages

class PEGASUS_COMMON_LINKAGE CIMOpenOperationRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMOpenOperationRequestMessage(
        MessageType type_,
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className,
        const String& filterQueryLanguage_,
        const String& filterQuery_,
        const Uint32Arg& operationTimeout_,
        Boolean continueOnError_,
        Uint32 maxObjectCount_,
        Uint32 providerType_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    String filterQueryLanguage;
    String filterQuery;
    Uint32Arg operationTimeout;
    Boolean continueOnError;
    Uint32 maxObjectCount;
};

class PEGASUS_COMMON_LINKAGE CIMOpenEnumerateInstancesRequestMessage
    : public CIMOpenOperationRequestMessage
{
public:
    CIMOpenEnumerateInstancesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        const Boolean deepInheritance_,
        const Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const String& filterQueryLanguage_,
        const String& filterQuery_,
        const Uint32Arg& operationTimeout_,
        Boolean continueOnError_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);
    virtual CIMResponseMessage* buildResponse() const;

    Boolean deepInheritance;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;
};

class PEGASUS_COMMON_LINKAGE CIMOpenEnumerateInstancePathsRequestMessage
    : public CIMOpenOperationRequestMessage
{
public:
    CIMOpenEnumerateInstancePathsRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        const String& filterQueryLanguage_,
        const String& filterQuery_,
        const Uint32Arg& operationTimeout_,
        Boolean continueOnError_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;
};

class PEGASUS_COMMON_LINKAGE CIMOpenReferenceInstancesRequestMessage
    : public CIMOpenOperationRequestMessage
{
public:
    CIMOpenReferenceInstancesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& resultClass_,
        const String& role_,
        Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const String& filterQueryLanguage_,
        const String& filterQuery_,
        const Uint32Arg& operationTimeout_,
        Boolean continueOnError_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath objectName;
    CIMName resultClass;
    String role;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;
};

class PEGASUS_COMMON_LINKAGE CIMOpenReferenceInstancePathsRequestMessage
    : public CIMOpenOperationRequestMessage
{
public:
    CIMOpenReferenceInstancePathsRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& resultClass_,
        const String& role_,
        const String& filterQueryLanguage_,
        const String& filterQuery_,
        const Uint32Arg& operationTimeout_,
        Boolean continueOnError_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath objectName;
    CIMName resultClass;
    String role;
};

class PEGASUS_COMMON_LINKAGE CIMOpenAssociatorInstancesRequestMessage
    : public CIMOpenOperationRequestMessage
{
public:
    CIMOpenAssociatorInstancesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& assocClass_,
        const CIMName& resultClass_,
        const String& role_,
        const String& resultRole_,
        const Boolean includeClassOrigin_,
        const CIMPropertyList& propertyList_,
        const String& filterQueryLanguage_,
        const String& filterQuery_,
        const Uint32Arg& operationTimeout_,
        Boolean continueOnError_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath objectName;
    CIMName assocClass;
    CIMName resultClass;
    String role;
    String resultRole;
    Boolean includeClassOrigin;
    CIMPropertyList propertyList;
};

class PEGASUS_COMMON_LINKAGE CIMOpenAssociatorInstancePathsRequestMessage
    : public CIMOpenOperationRequestMessage
{
public:
    CIMOpenAssociatorInstancePathsRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMObjectPath& objectName_,
        const CIMName& assocClass_,
        const CIMName& resultClass_,
        const String& role_,
        const String& resultRole_,
        const String& filterQueryLanguage_,
        const String& filterQuery_,
        const Uint32Arg& operationTimeout_,
        Boolean continueOnError_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath objectName;
    CIMName assocClass;
    CIMName resultClass;
    String role;
    String resultRole;
};

// Intermediate message in the request class hiearchy to capture the
// common attributes of CIMPull... messages.  Since all of the input parameters
// of all of the pull messages are the same. This captures all of the
// information for the pull request messages except information about the
// response data type
class PEGASUS_COMMON_LINKAGE CIMPullOperationRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMPullOperationRequestMessage(
        MessageType type_,
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& enumerationContext_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    String enumerationContext;
    Uint32 maxObjectCount;
};

class PEGASUS_COMMON_LINKAGE CIMPullInstancesWithPathRequestMessage
    : public CIMPullOperationRequestMessage
{
public:
    CIMPullInstancesWithPathRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& enumerationContext_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;
};

class PEGASUS_COMMON_LINKAGE CIMPullInstancePathsRequestMessage
    : public CIMPullOperationRequestMessage
{
public:
    CIMPullInstancePathsRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& enumerationContext_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;
};
class PEGASUS_COMMON_LINKAGE CIMPullInstancesRequestMessage
    : public CIMPullOperationRequestMessage
{
public:
    CIMPullInstancesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& enumerationContext_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;
};

class PEGASUS_COMMON_LINKAGE CIMCloseEnumerationRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMCloseEnumerationRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& enumerationContext_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    String enumerationContext;
};

class PEGASUS_COMMON_LINKAGE CIMEnumerationCountRequestMessage
    : public CIMOperationRequestMessage
{
public:
    CIMEnumerationCountRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& enumerationContext_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    String enumerationContext;
};

class PEGASUS_COMMON_LINKAGE CIMOpenQueryInstancesRequestMessage
    : public  CIMOpenOperationRequestMessage
{
public:
    CIMOpenQueryInstancesRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const String& queryLanguage_,
        const String& query_,
        Boolean returnQueryResultClass_,
        const Uint32Arg& operationTimeout_,
        Boolean continueOnError_,
        Uint32 maxObjectCount_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    Boolean returnQueryResultClass;

    // WARNING: The queryLanguage and query here are not the same as
    // for other OpenMessages.  For those a filterQuery like FQL is required
    // This is a full Query Language (ex. WQL or CQL)
    String queryLanguage;
    String query;
};
// EXP_PULL_END

class PEGASUS_COMMON_LINKAGE CIMProcessIndicationRequestMessage
    : public CIMRequestMessage
{
public:
    CIMProcessIndicationRequestMessage(
        const String & messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance& indicationInstance_,
        const Array<CIMObjectPath> & subscriptionInstanceNames_,
        const CIMInstance & provider_,
        const QueueIdStack& queueIds_,
        Uint32 timeoutMilliSec_ = 0,
        String oopAgentName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMNamespaceName nameSpace;
    CIMInstance indicationInstance;
    Array<CIMObjectPath> subscriptionInstanceNames;
    CIMInstance provider;
    Uint32 timeoutMilliSec;
    String oopAgentName;
};

class PEGASUS_COMMON_LINKAGE CIMNotifyProviderRegistrationRequestMessage
    : public CIMRequestMessage
{
public:
    enum Operation
    {
        OP_CREATE = 1,
        OP_DELETE = 2,
        OP_MODIFY = 3
    };

    CIMNotifyProviderRegistrationRequestMessage(
        const String & messageId_,
        const Operation operation_,
        const CIMName & className_,
        const Array <CIMNamespaceName> & newNamespaces_,
        const Array <CIMNamespaceName> & oldNamespaces_,
        const CIMPropertyList & newPropertyNames_,
        const CIMPropertyList & oldPropertyNames_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    CIMName className;
    Array <CIMNamespaceName> newNamespaces;
    Array <CIMNamespaceName> oldNamespaces;
    CIMPropertyList newPropertyNames;
    CIMPropertyList oldPropertyNames;
    Operation operation;
};

class PEGASUS_COMMON_LINKAGE CIMNotifyProviderTerminationRequestMessage
    : public CIMRequestMessage
{
public:
    CIMNotifyProviderTerminationRequestMessage(
        const String & messageId_,
        const Array <CIMInstance> & providers_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    Array <CIMInstance> providers;
};

class PEGASUS_COMMON_LINKAGE CIMHandleIndicationRequestMessage
    : public CIMRequestMessage
{
public:
    CIMHandleIndicationRequestMessage(
        const String& messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance& handlerInstance_,
        const CIMInstance& indicationInstance_,
        const CIMInstance& subscriptionInstance_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMNamespaceName nameSpace;
    CIMInstance handlerInstance;
    CIMInstance indicationInstance;
    CIMInstance subscriptionInstance;
    String authType;
    String userName;
    DeliveryStatusAggregator *deliveryStatusAggregator;
};

class PEGASUS_COMMON_LINKAGE CIMCreateSubscriptionRequestMessage
    : public CIMIndicationRequestMessage
{
public:
    CIMCreateSubscriptionRequestMessage(
        const String& messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance & subscriptionInstance_,
        const Array<CIMName> & classNames_,
        const CIMPropertyList & propertyList_,
        const Uint16 repeatNotificationPolicy_,
        const String & query_,
        const QueueIdStack& queueIds_,
        const String & authType_ = String::EMPTY,
        const String & userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMNamespaceName nameSpace;
    CIMInstance subscriptionInstance;
    Array <CIMName> classNames;
    CIMPropertyList propertyList;
    Uint16 repeatNotificationPolicy;
    String query;
};

class PEGASUS_COMMON_LINKAGE CIMModifySubscriptionRequestMessage
    : public CIMIndicationRequestMessage
{
public:
    CIMModifySubscriptionRequestMessage(
        const String& messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance & subscriptionInstance_,
        const Array<CIMName> & classNames_,
        const CIMPropertyList & propertyList_,
        const Uint16 repeatNotificationPolicy_,
        const String & query_,
        const QueueIdStack& queueIds_,
        const String & authType_ = String::EMPTY,
        const String & userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMNamespaceName nameSpace;
    CIMInstance subscriptionInstance;
    Array<CIMName> classNames;
    CIMPropertyList propertyList;
    Uint16 repeatNotificationPolicy;
    String query;
};

class PEGASUS_COMMON_LINKAGE CIMDeleteSubscriptionRequestMessage
    : public CIMIndicationRequestMessage
{
public:
    CIMDeleteSubscriptionRequestMessage(
        const String& messageId_,
        const CIMNamespaceName & nameSpace_,
        const CIMInstance & subscriptionInstance_,
        const Array<CIMName> & classNames_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMNamespaceName nameSpace;
    CIMInstance subscriptionInstance;
    Array<CIMName> classNames;
};

class PEGASUS_COMMON_LINKAGE
    CIMSubscriptionInitCompleteRequestMessage
    : public CIMRequestMessage
{
public:
    CIMSubscriptionInitCompleteRequestMessage(
        const String & messageId_,
        const QueueIdStack & queueIds_);

    virtual CIMResponseMessage* buildResponse() const;
};

class PEGASUS_COMMON_LINKAGE
    CIMIndicationServiceDisabledRequestMessage
    : public CIMRequestMessage
{
public:
    CIMIndicationServiceDisabledRequestMessage(
        const String & messageId_,
        const QueueIdStack & queueIds_);

    virtual CIMResponseMessage* buildResponse() const;
};

class PEGASUS_COMMON_LINKAGE CIMDisableModuleRequestMessage
    : public CIMRequestMessage
{
public:
    CIMDisableModuleRequestMessage(
        const String& messageId_,
        const CIMInstance& providerModule_,
        const Array<CIMInstance>& providers_,
        Boolean disableProviderOnly_,
        const Array<Boolean>& indicationProviders_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMInstance providerModule;
    Array<CIMInstance> providers;
    Boolean disableProviderOnly;
    Array<Boolean> indicationProviders;
    String authType;
    String userName;
};

class PEGASUS_COMMON_LINKAGE CIMEnableModuleRequestMessage
    : public CIMRequestMessage
{
public:
    CIMEnableModuleRequestMessage(
        const String& messageId_,
        const CIMInstance& providerModule_,
        const QueueIdStack& queueIds_,
        const String& authType_ = String::EMPTY,
        const String& userName_ = String::EMPTY);

    virtual CIMResponseMessage* buildResponse() const;

    CIMInstance providerModule;
    String authType;
    String userName;
};

class PEGASUS_COMMON_LINKAGE CIMNotifyProviderEnableRequestMessage
    : public CIMRequestMessage
{
public:
    CIMNotifyProviderEnableRequestMessage(
        const String & messageId_,
        const Array <CIMInstance> & capInstances_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    Array <CIMInstance> capInstances;
};

class PEGASUS_COMMON_LINKAGE CIMNotifyProviderFailRequestMessage
    : public CIMRequestMessage
{
public:
    CIMNotifyProviderFailRequestMessage(
        const String & messageId_,
        const String & moduleName_,
        const String & userName_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    String moduleName;
    String userName;
};

class PEGASUS_COMMON_LINKAGE CIMStopAllProvidersRequestMessage
    : public CIMRequestMessage
{
public:
    CIMStopAllProvidersRequestMessage(
        const String& messageId_,
        const QueueIdStack& queueIds_,
        Uint32 shutdownTimeout_ = 0);

    virtual CIMResponseMessage* buildResponse() const;
    Uint32 shutdownTimeout;
};

// Used to pass initialization data to an Out-of-Process Provider Agent process
class PEGASUS_COMMON_LINKAGE CIMInitializeProviderAgentRequestMessage
    : public CIMRequestMessage
{
public:
    CIMInitializeProviderAgentRequestMessage(
        const String & messageId_,
        const String& pegasusHome_,
        const Array<Pair<String, String> >& configProperties_,
        Boolean bindVerbose_,
        Boolean subscriptionInitComplete_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    String pegasusHome;
    Array<Pair<String, String> > configProperties;
    Boolean bindVerbose;
    Boolean subscriptionInitComplete;
};

class PEGASUS_COMMON_LINKAGE CIMNotifyConfigChangeRequestMessage
    : public CIMRequestMessage
{
public:
    CIMNotifyConfigChangeRequestMessage(
        const String & messageId_,
        const String & propertyName_,
        const String & newPropertyValue_,
        Boolean currentValueModified_, // false - planned value modified
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    String propertyName;
    String newPropertyValue;
    Boolean currentValueModified;
};


class PEGASUS_COMMON_LINKAGE ProvAgtGetScmoClassRequestMessage
    : public CIMRequestMessage
{
public:
    ProvAgtGetScmoClassRequestMessage(
        const String& messageId_,
        const CIMNamespaceName& nameSpace_,
        const CIMName& className_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    CIMNamespaceName nameSpace;
    CIMName className;

};

class PEGASUS_COMMON_LINKAGE CIMNotifySubscriptionNotActiveRequestMessage
    : public CIMRequestMessage
{
public:
    CIMNotifySubscriptionNotActiveRequestMessage(
        const String & messageId_,
        const CIMObjectPath &subscriptionName_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath subscriptionName;
};

class PEGASUS_COMMON_LINKAGE CIMNotifyListenerNotActiveRequestMessage
    : public CIMRequestMessage
{
public:
    CIMNotifyListenerNotActiveRequestMessage(
        const String & messageId_,
        const CIMObjectPath &handlerName_,
        const QueueIdStack& queueIds_);

    virtual CIMResponseMessage* buildResponse() const;

    CIMObjectPath handlerName;
};

//
// CIMResponseMessages
//

class PEGASUS_COMMON_LINKAGE CIMResponseDataMessage
    : public CIMResponseMessage
{
public:

    CIMResponseDataMessage(
        MessageType type_,
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        CIMResponseData::ResponseDataContent rspContent_,
        Boolean isAsyncResponsePending=false);

    CIMResponseData& getResponseData()
    {
        return _responseData;
    }

private:
    CIMResponseData _responseData;
};

class PEGASUS_COMMON_LINKAGE CIMGetClassResponseMessage
    : public CIMResponseMessage
{
public:
    CIMGetClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMClass& cimClass_);

    CIMClass cimClass;
};

class PEGASUS_COMMON_LINKAGE CIMGetInstanceResponseMessage
    : public CIMResponseDataMessage
{
public:
    CIMGetInstanceResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);

private:
    CIMGetInstanceResponseMessage();
    CIMGetInstanceResponseMessage(const CIMGetInstanceResponseMessage&);
    CIMGetInstanceResponseMessage& operator=(
        const CIMGetInstanceResponseMessage&);
};

class PEGASUS_COMMON_LINKAGE CIMExportIndicationResponseMessage
    : public CIMResponseMessage
{
public:
    CIMExportIndicationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMDeleteClassResponseMessage
    : public CIMResponseMessage
{
public:
    CIMDeleteClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMDeleteInstanceResponseMessage
    : public CIMResponseMessage
{
public:
    CIMDeleteInstanceResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMCreateClassResponseMessage
    : public CIMResponseMessage
{
public:
    CIMCreateClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMCreateInstanceResponseMessage
    : public CIMResponseMessage
{
public:
    CIMCreateInstanceResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMObjectPath& instanceName_);

    CIMObjectPath instanceName;
};

class PEGASUS_COMMON_LINKAGE CIMModifyClassResponseMessage
    : public CIMResponseMessage
{
public:
    CIMModifyClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMModifyInstanceResponseMessage
    : public CIMResponseMessage
{
public:
    CIMModifyInstanceResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateClassesResponseMessage
    : public CIMResponseMessage
{
public:
    CIMEnumerateClassesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<CIMClass>& cimClasses_);

    Array<CIMClass> cimClasses;
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateClassNamesResponseMessage
    : public CIMResponseMessage
{
public:
    CIMEnumerateClassNamesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<CIMName>& classNames_);

    Array<CIMName> classNames;
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateInstancesResponseMessage
    : public CIMResponseDataMessage
{
public:
    CIMEnumerateInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateInstanceNamesResponseMessage
    : public CIMResponseDataMessage
{
public:
    CIMEnumerateInstanceNamesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMExecQueryResponseMessage
    : public CIMResponseDataMessage
{
public:
    CIMExecQueryResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMAssociatorsResponseMessage
    : public CIMResponseDataMessage
{
public:
    CIMAssociatorsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMAssociatorNamesResponseMessage
    : public CIMResponseDataMessage
{
public:
    CIMAssociatorNamesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMReferencesResponseMessage
    : public CIMResponseDataMessage
{
public:
    CIMReferencesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMReferenceNamesResponseMessage
    : public CIMResponseDataMessage
{
public:
    CIMReferenceNamesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMGetPropertyResponseMessage
    : public CIMResponseMessage
{
public:
    CIMGetPropertyResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMValue& value_);

    CIMValue value;
};

class PEGASUS_COMMON_LINKAGE CIMSetPropertyResponseMessage
    : public CIMResponseMessage
{
public:
    CIMSetPropertyResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMGetQualifierResponseMessage
    : public CIMResponseMessage
{
public:
    CIMGetQualifierResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMQualifierDecl& cimQualifierDecl_);

    CIMQualifierDecl cimQualifierDecl;
};

class PEGASUS_COMMON_LINKAGE CIMSetQualifierResponseMessage
    : public CIMResponseMessage
{
public:
    CIMSetQualifierResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMDeleteQualifierResponseMessage
    : public CIMResponseMessage
{
public:
    CIMDeleteQualifierResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMEnumerateQualifiersResponseMessage
    : public CIMResponseMessage
{
public:
    CIMEnumerateQualifiersResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<CIMQualifierDecl>& qualifierDeclarations_);

    Array<CIMQualifierDecl> qualifierDeclarations;
};

class PEGASUS_COMMON_LINKAGE CIMInvokeMethodResponseMessage
    : public CIMResponseMessage
{
public:
    CIMInvokeMethodResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const CIMValue& retValue_,
        const Array<CIMParamValue>& outParameters_,
        const CIMName& methodName_);

    CIMValue retValue;
    Array<CIMParamValue> outParameters;
    CIMName methodName;
};

//EXP_PULL_BEGIN
// Extend CIMResponseDataMessage for the common elements on open and
// pull operations. All of the Open and pull response operations
// return endOfSequence and enumerationContext arguments
class PEGASUS_COMMON_LINKAGE CIMOpenOrPullResponseDataMessage
    : public CIMResponseDataMessage
{
public:
    CIMOpenOrPullResponseDataMessage(
        MessageType type_,
        const String& messageId_,
        const CIMException& cimException_,
         const QueueIdStack& queueIds_,
        CIMResponseData::ResponseDataContent rspContent_,
        const Boolean endOfSequence_ = false,
        const String& enumerationContext_ = String::EMPTY);

    Boolean endOfSequence;
    String enumerationContext;
};

class PEGASUS_COMMON_LINKAGE CIMOpenEnumerateInstancesResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    CIMOpenEnumerateInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_ = false,
        const String& enumerationContext_ = String::EMPTY);
};

// All open and pull ResponseData messages derive from CIMPullResponse
// Data message because the response data is common (CIMResponseData,
// endOfSequencd, and enumerationContext.
class PEGASUS_COMMON_LINKAGE CIMOpenEnumerateInstancePathsResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    // Constructor with default endOfSequence and enumeration context optional
    CIMOpenEnumerateInstancePathsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_ = false,
        const String& enumerationContext_ = String::EMPTY);

    Array<CIMObjectPath> cimInstancePaths;
};

class PEGASUS_COMMON_LINKAGE CIMOpenReferenceInstancesResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    CIMOpenReferenceInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_ = false,
        const String& enumerationContext_ = String::EMPTY);
};

class PEGASUS_COMMON_LINKAGE CIMOpenReferenceInstancePathsResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    CIMOpenReferenceInstancePathsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_ = false,
        const String& enumerationContext_ = String::EMPTY);
};

class PEGASUS_COMMON_LINKAGE CIMOpenAssociatorInstancesResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    CIMOpenAssociatorInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_ = false,
        const String& enumerationContext_ = String::EMPTY);
};

class PEGASUS_COMMON_LINKAGE CIMOpenAssociatorInstancePathsResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    // Constructor with defautl endOfSequence and enumerationContext
    CIMOpenAssociatorInstancePathsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_ = false,
        const String& enumerationContext_ = String::EMPTY);
};

class PEGASUS_COMMON_LINKAGE CIMPullInstancesWithPathResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    CIMPullInstancesWithPathResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_);
};

class PEGASUS_COMMON_LINKAGE CIMPullInstancePathsResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    CIMPullInstancePathsResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_);
};

class PEGASUS_COMMON_LINKAGE CIMPullInstancesResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    CIMPullInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Boolean endOfSequence_,
        const String& enumerationContext_);
};

// CIMCloseEnumeration is not a Data message and returns
// only an acknowledgement
class PEGASUS_COMMON_LINKAGE CIMCloseEnumerationResponseMessage
    : public CIMResponseMessage
{
public:
    CIMCloseEnumerationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

// NOTE: This message will be deprecated in the DMTF
// DSP 0200 V1.5 specification and Pegasus will not implement
// it any further than the Dispatcher. (KS)
class PEGASUS_COMMON_LINKAGE CIMEnumerationCountResponseMessage
    : public CIMResponseMessage
{
public:
    CIMEnumerationCountResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Uint64Arg& count_ );

    Uint64Arg count;
};

class PEGASUS_COMMON_LINKAGE CIMOpenQueryInstancesResponseMessage
    : public CIMOpenOrPullResponseDataMessage
{
public:
    CIMOpenQueryInstancesResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const CIMClass& queryResultClass_,
        const QueueIdStack& queueIds_,
        Boolean endOfSequence_ = false,
        const String& enumerationContext_ = String::EMPTY);

    CIMClass queryResultClass;
};

//EXP_PULL_END

class PEGASUS_COMMON_LINKAGE CIMProcessIndicationResponseMessage
    : public CIMResponseMessage
{
public:
    CIMProcessIndicationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        String oopAgentName_ = String::EMPTY,
        CIMInstance subscription_ = CIMInstance());

    String oopAgentName;
    CIMInstance subscription;
};

class PEGASUS_COMMON_LINKAGE CIMNotifyProviderRegistrationResponseMessage
    : public CIMResponseMessage
{
public:
    CIMNotifyProviderRegistrationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMNotifyProviderTerminationResponseMessage
    : public CIMResponseMessage
{
public:
    CIMNotifyProviderTerminationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMHandleIndicationResponseMessage
    : public CIMResponseMessage
{
public:
    CIMHandleIndicationResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMCreateSubscriptionResponseMessage
    : public CIMResponseMessage
{
public:
    CIMCreateSubscriptionResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMModifySubscriptionResponseMessage
    : public CIMResponseMessage
{
public:
    CIMModifySubscriptionResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMDeleteSubscriptionResponseMessage
    : public CIMResponseMessage
{
public:
    CIMDeleteSubscriptionResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE
CIMSubscriptionInitCompleteResponseMessage
    : public CIMResponseMessage
{
public:
    CIMSubscriptionInitCompleteResponseMessage
       (const String & messageId_,
        const CIMException & cimException_,
        const QueueIdStack & queueIds_);
};

class PEGASUS_COMMON_LINKAGE
CIMIndicationServiceDisabledResponseMessage
    : public CIMResponseMessage
{
public:
    CIMIndicationServiceDisabledResponseMessage
       (const String & messageId_,
        const CIMException & cimException_,
        const QueueIdStack & queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMDisableModuleResponseMessage
    : public CIMResponseMessage
{
public:
    CIMDisableModuleResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<Uint16>& operationalStatus_);

    Array<Uint16> operationalStatus;
};

class PEGASUS_COMMON_LINKAGE CIMEnableModuleResponseMessage
    : public CIMResponseMessage
{
public:
    CIMEnableModuleResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const Array<Uint16>& operationalStatus_);

    Array<Uint16> operationalStatus;
};

class PEGASUS_COMMON_LINKAGE CIMNotifyProviderEnableResponseMessage
    : public CIMResponseMessage
{
public:
    CIMNotifyProviderEnableResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMNotifyProviderFailResponseMessage
    : public CIMResponseMessage
{
public:
    CIMNotifyProviderFailResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);

    Uint32 numSubscriptionsAffected;
};

class PEGASUS_COMMON_LINKAGE CIMStopAllProvidersResponseMessage
    : public CIMResponseMessage
{
public:
    CIMStopAllProvidersResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMInitializeProviderAgentResponseMessage
    : public CIMResponseMessage
{
public:
    CIMInitializeProviderAgentResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMNotifyConfigChangeResponseMessage
    : public CIMResponseMessage
{
public:
    CIMNotifyConfigChangeResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE ProvAgtGetScmoClassResponseMessage
    : public CIMResponseMessage
{
public:
    ProvAgtGetScmoClassResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_,
        const SCMOClass& scmoClass_);

    SCMOClass scmoClass;
};

class PEGASUS_COMMON_LINKAGE CIMNotifySubscriptionNotActiveResponseMessage
    : public CIMResponseMessage
{
public:
    CIMNotifySubscriptionNotActiveResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

class PEGASUS_COMMON_LINKAGE CIMNotifyListenerNotActiveResponseMessage
    : public CIMResponseMessage
{
public:
    CIMNotifyListenerNotActiveResponseMessage(
        const String& messageId_,
        const CIMException& cimException_,
        const QueueIdStack& queueIds_);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMMessage_h */
