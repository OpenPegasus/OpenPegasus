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

#include "CIMOperationRequestDispatcher.h"

#include <Pegasus/Common/Constants.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/Common/XmlReader.h> // stringToValue(), stringArrayToValue()
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/AuditLogger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/ObjectNormalizer.h>
#include <Pegasus/Server/reg_table.h>
#include <Pegasus/General/VersionUtil.h>
#include <ctime>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/Threads.h>

#include <Pegasus/Server/QuerySupportRouter.h>

#include <Pegasus/Server/EnumerationContext.h>
#include <Pegasus/Server/EnumerationContextTable.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PEGASUS_ENABLE_FQL
#include <Pegasus/FQL/FQLQueryExpressionRep.h>
#include <Pegasus/FQL/FQLParser.h>
#include <Pegasus/FQL/FQLInstancePropertySource.h>
#endif

/// KS_TODO delete this
#include <Pegasus/Common/Print.h>

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

#ifndef XCOUT
#    define XCOUT if (true) cout << __FILE__ << ":" << __LINE__ << " "
#endif

// KS_TODO. Replace the CSTRING args with the Str class implementaiton
#define CSTRING(ARG) (const char*) ARG.getCString()

// General class to process various objects that are made up of Pegaus
// Strings back to the String and more directly to the const char* ...
// used for display. This can be used for
// String, CIMName, CIMNamespaceName, Exception, CIMDateTime, CIMObjectPath
// The same general class exists in several places in OpenPegasus.
// TODO: make this a general part of Pegasus so it is not duplicated in
// many different files.
class Str
{
public:
    Str(const String& s) : _cstr(s.getCString()) { }
    Str(const CIMName& n) : _cstr(n.getString().getCString()) { }
    Str(const CIMNamespaceName& n) : _cstr(n.getString().getCString()) { }
    Str(const Exception& e) : _cstr(e.getMessage().getCString()) { }
    Str(const CIMDateTime& x) : _cstr(x.toString().getCString()) { }
    Str(const CIMObjectPath& x) : _cstr(x.toString().getCString()) { }
    const char* operator*() const { return (const char*)_cstr; }
    operator const char*() const { return (const char*)_cstr; }
private:
    CString _cstr;
};

// Set a define based on whether FQL was enabled for this build
#ifdef PEGASUS_ENABLE_FQL
#  define ENABLE_FQL_FILTER true
#else
#  define ENABLE_FQL_FILTER false
#endif

/******************************************************************************
**
**  Static variables outside of object context
**
******************************************************************************/

// Variable to determine the performance if we wait for pull
// response size to match request or simply for response objects to exist.
// This variable is primarily test for the advantages of each of these
// characteristics.
static Boolean requireCompleteResponses = false;

// A helper function that resets the Propagated and ClassOrigin attributes on
// properties of CIMInstance and CIMClass objects. This is used during
// Create/Modify Instance and Create/Modify Class operations, where the
// Propagated and ClassOrigin attributes must be ignored.
template <class ObjectClass>
void removePropagatedAndOriginAttributes(ObjectClass& newObject);

static const char* _getServiceName(Uint32 serviceId)
{
    MessageQueue *queue = MessageQueue::lookup(serviceId);

    return queue ? queue->getQueueName() : "none";
}

CIMOperationRequestDispatcher* _cimOperationRequestDispatcher = NULL;

/****************************************************************************
**
**  Implementation of OperationAggregate Class
**
****************************************************************************/
OperationAggregate::OperationAggregate(
    CIMOperationRequestMessage* request,
    CIMName className,
    CIMNamespaceName nameSpace,
    Uint32 totalIssued,
    Boolean requiresHostnameCompletion,
    Boolean hasPropList,
    QueryExpressionRep* query,
    String queryLanguage)
    : _messageId(request->messageId),
      _filterResponse(false),
      _msgRequestType(request->getType()),
      _dest(request->queueIds.top()),
      _className(className),
      _nameSpace(nameSpace),
      _requiresHostnameCompletion(requiresHostnameCompletion),
      _hasPropList(hasPropList),
      _query(query),
      _queryLanguage(queryLanguage),
      _pullOperation(false),
      _enumerationFinished(false),
      _closeReceived(false),
      _request(request),
      _totalIssued(totalIssued),
      _totalReceived(0), _totalReceivedComplete(0),
      _totalReceivedExpected(0), _totalReceivedErrors(0),
      _totalReceivedNotSupported(0)
{
}

OperationAggregate::~OperationAggregate()
{
    delete _request;
    delete _query;
}

Boolean OperationAggregate::valid() const
{
    // test if valid OperationAggregate object.
    return _magic;
}

/*  Add one response to the responseList
*/
void OperationAggregate::appendResponse(CIMResponseMessage* response)
{
    PEGASUS_DEBUG_ASSERT(valid());
    AutoMutex autoMut(_appendResponseMutex);
    PEGASUS_DEBUG_ASSERT(response != 0);
    _responseList.append(response);
}


/* Returns the count of the current number of responses in the
   responseList.  Note:this is not the Total number of responses,
   just the number currently available for aggregation.
*/

Uint32 OperationAggregate::numberResponses() const
{
    PEGASUS_DEBUG_ASSERT(valid());

    return _responseList.size();
}

CIMResponseMessage* OperationAggregate::getResponse(const Uint32& pos)
{
    PEGASUS_DEBUG_ASSERT(valid());
    AutoMutex autoMut(_appendResponseMutex);
    CIMResponseMessage* msg = _responseList[pos];

    return msg;
}

CIMResponseMessage* OperationAggregate::removeResponse(const Uint32& pos)
{
    PEGASUS_DEBUG_ASSERT(valid());
    AutoMutex autoMut(_appendResponseMutex);
    CIMResponseMessage* msg = _responseList[pos];
    _responseList.remove(pos);

    return msg;
}

void OperationAggregate::deleteResponse(const Uint32&pos)
{
    PEGASUS_ASSERT(valid());
    AutoMutex autoMut(_appendResponseMutex);
    delete _responseList[pos];
    _responseList.remove(pos);
}

// There are many response pieces (chunks) from potentially many
// threads funneling through this function in random order. This isolates a
// single response (by locking) from a given thread and "resequences" the
// response as part of one large response. It is crucial that the first
// response to come through here be sequenced (or indexed) as 0 and the last
// response from the last thread be marked as "isComplete"

// NOTE: Assumes no chunks can come AFTER a "isComplete" message
// of the LAST thread.

void OperationAggregate::resequenceResponse(CIMResponseMessage& response)
{
    // If error, increment error counter.
    // If NOT_SUPPORTED Error, increment NotSupported counter
    CIMStatusCode error = response.cimException.getCode();
    bool notSupportedReceived = false;

    if (error != CIM_ERR_SUCCESS)
    {
        if (error == CIM_ERR_NOT_SUPPORTED)
        {
            notSupportedReceived = true;
            _totalReceivedNotSupported++;
        }
        _totalReceivedErrors++;

        // trace the error including the provider name
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "Provider Response has errorCode=%u.  Namespace=%s, ClassName=%s, "
                "ResponseSequence=%u messageId=%s",
            (Uint32)error,
            CSTRING(_nameSpace.getString()),
            CSTRING(_className.getString()),
            _totalReceived, CSTRING(response.messageId) ));
    }

    Boolean isComplete = response.isComplete();

    // if this provider is complete increment totalReceivedComplete, etc.
    if (isComplete == true)
    {
        _totalReceivedComplete++;
        _totalReceivedExpected += response.getIndex() + 1;
    }

    response.setIndex(_totalReceived++);

    // set to incomplete until ALL completed messages have come in

    isComplete = false;

    // NOTE:
    // _totalReceivedExpected is calculated by adding up every response index
    // count WHEN the message is marked complete. This may differ from the
    // following reasons:
    // 1. An exception occurred in which the correct index could not be set.
    // 2. Somehow the completed response arrived before the other
    //    (non-completed) responses ? (shouldnt happen with the current
    //    synchronous code).
    // In either case, a message will be logged and attempt to continue

    if (_totalReceivedComplete == _totalIssued)
    {
        if (_totalReceivedExpected == _totalReceived)
        {
            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                "Responses Completed. messageid=%s is complete. "
                    "TotalResponses=%u, totalChunks=%u, totalErrors=%u",
                CSTRING(response.messageId),
                _totalReceivedComplete,
                _totalReceived,
                _totalReceivedErrors));
        }
        else
        {
            // KS_TODO this is actually reporting an error but gets found
            // only if trace is enabled. Same issue exists for lots of things
            // that get reported as DISCARDED_DATA
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "All completed responses (%u) for current request"
                    " messageId=%s "
                    "have been accounted for but expected count (%u) does "
                    "not match the received count (%u). error count (%u).",
                _totalReceivedComplete,
                CSTRING(response.messageId),
                _totalReceivedExpected,
                _totalReceived,
                _totalReceivedErrors));
        }

        // If all of the errors received were NOT_SUPPORTED and
        // all of the responses were errors, then keep the last
        // NOT_SUPPORTED error. Otherwise clear the error
        // The condition below is:
        // If there was an error besides NOT_SUPPORTED, or a non-error response
        // was received, and the last response was a NOT_SUPPORTED error,
        // then clear the error
        if ((_totalReceivedErrors != _totalReceivedNotSupported ||
            _totalReceivedErrors != _totalReceived) && notSupportedReceived)
        {
            response.cimException = CIMException();
        }

        isComplete = true;
        _totalReceivedComplete = 0;
        _totalReceivedExpected = 0;
        _totalReceivedErrors = 0;
        _totalReceivedNotSupported = 0;
        _totalReceived = 0;
    }
    else if (notSupportedReceived)
    {
        // Clear the NOT_SUPPORTED exception
        // We ignore it unless it's the only response received
        response.cimException = CIMException();
    }

    response.setComplete(isComplete);

}

//EXP_PULL_BEGIN

/* setPullOperation sets variables in the Operation Aggregate
    specific to the pull operations upon the Open... operations for use by
    subsequent pull operations. This includes:
    _pullOperation - Flag to indicate that this Operation Aggregate is part
    of a pull operation.
    _enumerationContext - Pointer to the Enumeration Context
    _EnumerationContextName - Id for this context Used only to confirm
    error issues on provider responses
*/
void OperationAggregate::setPullOperation(EnumerationContext* enContext)
{
    _pullOperation = true;
    _enumerationContext = enContext;
    _contextId = enContext->getContextId();
}

/*
*
*   Common functions use by the request Handlers
*/
/*
    build an array of CIMNames with all of the properties in the
    class.
    @param class CIMClass with the properties
    @return Array<String> containing the names of all the properties
    in the class.
*/
void _buildPropertyListWithTags(CIMConstClass& thisClass,
    CIMPropertyList& propertyList)
{
    Array<String> propertyNameArray;
    Uint32 numProperties = thisClass.getPropertyCount();

    for (Uint32 i = 0; i < numProperties; i++)
    {
        propertyNameArray.append(
            thisClass.getProperty(i).getName().getString());
    }
    propertyList.append(propertyNameArray);
}
//EXP_PULL_END

/*************************************************************************
**
** Implementation of ProviderInfoList Class.
** NOTE: Much of this class implemented in the header file
**
***************************************************************************/
Boolean ProviderInfoList::hasMore(Boolean isProvider)
{
    ConstArrayIterator<ProviderInfo> iterator(_array);
    while (_index < iterator.size())
    {
        if (iterator[_index].hasProvider == isProvider)
        {
            return true;
        }
        else
        {
            _index++;
        }
    }
    // Reset index when complete. This is so the hasMore can be used
    // multiple times to cycle through the list.
    _index = 0;
    return false;
}

// Commmon function to display routing info for a providerInfo element.
void ProviderInfoList::pegRoutingTrace(ProviderInfo& providerInfo,
    const char * reqMsgName,
    String& messageId)
{
    if (providerInfo.controlProviderName.size() != 0)
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "%s Routing request for class %s to "
                "service \"%s\" for control provider \"%s\".  "
                "Class # %u of %u, messageId=%s",
            reqMsgName,
            CSTRING(providerInfo.className.getString()),
            _getServiceName(providerInfo.serviceId),
            CSTRING(providerInfo.controlProviderName),
            getIndex() + 1, size(),
            CSTRING(messageId) ));
    }
    else
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "%s Routing request for class %s to "
                "service \"%s\".  Class # %u of %u, messageId=%s",
            reqMsgName,
            CSTRING(providerInfo.className.getString()),
            _getServiceName(providerInfo.serviceId),
            getIndex() + 1, size(),
            CSTRING(messageId) ));
    }
}

/*************************************************************************
**
** Implementation of CIMOperationRequestDispatcher Class
**
***************************************************************************/

// Dispatcher Class Constructor. Setup Dispatcher configuration from
// compile and runtime options.  This includes setting up QueueIds,
// limit parameters, etc. that will be used for request operation processing
//
CIMOperationRequestDispatcher::CIMOperationRequestDispatcher(
    CIMRepository* repository,
    ProviderRegistrationManager* providerRegistrationManager)
    : Base(PEGASUS_QUEUENAME_OPREQDISPATCHER),
      _repository(repository),
      _providerRegistrationManager(providerRegistrationManager)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::CIMOperationRequestDispatcher");

    _cimOperationRequestDispatcher = this;

    //
    // Setup enable AssociationTraversal parameter from runtime configuration.
    // Determines whether association operations are allowed.
    //
    ConfigManager* configManager = ConfigManager::getInstance();

    _enableAssociationTraversal = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableAssociationTraversal"));

    //
    //  Set up maximum breadth of Enums parameter from define.
    //  Determines maximum
    //  classes that can particpate in an enumeration.  Can be used to
    //  limit size of enumerations, associations, etc. to a maximum number
    //  of providers
    //  Get the maximum breadth of enum parameter from config if it exists.
    //
    // ATTN: KS 20030602 Bypass param acquistion until we get it into the
    // config manager
    // Config param removed for Pegasus 2.2 and until PEP 66 resolved.
    // In place, we simply allow anything through by setting the breadth too
    // large number.
// #define  MAXENUMBREADTHTESTENABLED
#ifdef MAXENUMBREADTHTESTENABLED
    String maxEnumBreadthOption;
    try
    {
        maxEnumBreadthOption =
            configManager->getCurrentValue("maximumEnumerationBreadth");
        if (maxEnumBreadthOption.size() != 0)
            _maximumEnumerateBreadth =
                atol(maxEnumBreadthOption.getCString());
    }
    catch (...)
    {
        _maximumEnumerateBreadth = 50;
    }
#else
    // As part of disabling the test for now, we set to very large number.
    _maximumEnumerateBreadth = 1000;
#endif

//EXP_PULL_BEGIN
    // setup PULL_OPERATION_MAXIMUM_OBJECT_COUNT
    // Define the maximum number of objects that the server will return for a
    // single pull... or open... operation. (Objects can be instances or
    // CIMObjectPaths  depending on the operation.

    _systemPullOperationMaxObjectCount = ConfigManager::parseUint32Value(
        configManager->getCurrentValue("pullOperationsMaxObjectCount"));

    //
    // Define maximum pull interoperation timeout value.  This sets
    // the maximum value for operationTimeout that will be accepted by
    // Pegasus. Anything larger than this will be rejected with the
    // error CIM_ERR_INVALID_OPERATION_TIMEOUT.
    _pullOperationMaxTimeout = ConfigManager::parseUint32Value(
        configManager->getCurrentValue("pullOperationsMaxTimeout"));

    // Define  variable that controls whether pegasus allows 0 as a pull
    // interoperation timeout value.  Since the behavior for a zero value is
    // that the server maintains no timer for the context, it may be the
    // decision of most implementors to not allow this value.
    // NOTE: Generally this should NEVER be used as it allows the client
    // to open enumerations that will NEVER be closed.

#define PEGASUS_PULL_OPERATION_REJECT_ZERO_TIMEOUT_VALUE
#ifdef PEGASUS_PULL_OPERATION_REJECT_ZERO_TIMEOUT_VALUE
    _rejectZeroOperationTimeoutValue = true;
#else
    // Default setting if nothing supplied externally
    _rejectZeroOperationTimeoutValue = false;
#endif

    // Create the EnumerationContextTable and get pointer to the
    // created Instance
    _enumerationContextTable = EnumerationContextTable::getInstance();
//EXP_PULL_END

    //
    // Setup list of provider modules that will be excluded from normalization
    // List derived from runtime variable with format name, name, ....
    //
#ifdef PEGASUS_ENABLE_OBJECT_NORMALIZATION
    String moduleList =
        configManager->getCurrentValue("excludeModulesFromNormalization");

    for (Uint32 pos = moduleList.find(','); moduleList.size() != 0;
        pos = moduleList.find(','))
    {
        String moduleName = moduleList.subString(0, pos);

        _excludeModulesFromNormalization.append(moduleName);

        moduleList.remove(0, (pos == PEG_NOT_FOUND ? pos : pos + 1));
    }
#endif

    // Get pointer to Internal Provider Routing Table. Note that table
    // probably not actually built at this point since it must be build
    // very late in initialization. See CIMServer.cpp
    _routing_table = DynamicRoutingTable::getRoutingTable();

    // Get ProviderManager QueueId (Used to forward requests
    // providerManager must be initialized before
    // Dispatcher so QueueId set.
    _providerManagerServiceId =
        lookup(PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP)->getQueueId();

    PEG_METHOD_EXIT();
}

//
//  CIMOperationDispatcher object destructor
//
CIMOperationRequestDispatcher::~CIMOperationRequestDispatcher()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::~CIMOperationRequestDispatcher");

    // Delete EnumerationContextTable
    _enumerationContextTable->removeContextTable();
    delete _enumerationContextTable;

    PEG_METHOD_EXIT();
}

//  Get Provider name and moduleName from OperationContext.
//  Used only by the AuditLog functions
void CIMOperationRequestDispatcher::_getProviderName(
    const OperationContext& context,
    String& moduleName,
    String& providerName)
{
    moduleName.clear();
    providerName.clear();

    if (context.contains(ProviderIdContainer::NAME))
    {
        const ProviderIdContainer pidc =
            (const ProviderIdContainer) context.get(ProviderIdContainer::NAME);

        CIMConstInstance module = pidc.getModule();
        Uint32 pos = module.findProperty(_PROPERTY_PROVIDERMODULE_NAME);
        if (pos != PEG_NOT_FOUND)
        {
            module.getProperty(pos).getValue().get(moduleName);
        }

        CIMConstInstance provider = pidc.getProvider();
        pos = provider.findProperty(PEGASUS_PROPERTYNAME_NAME);
        if (pos != PEG_NOT_FOUND)
        {
            provider.getProperty(pos).getValue().get(providerName);
        }
    }
}

void CIMOperationRequestDispatcher::_logOperation(
    const CIMOperationRequestMessage* request,
    const CIMResponseMessage* response)
{
#ifdef PEGASUS_ENABLE_AUDIT_LOGGER

    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_logOperation");

    String moduleName;
    String providerName;

    if (AuditLogger::isEnabled())
    {
        switch (request->getType())
        {
            case CIM_CREATE_CLASS_REQUEST_MESSAGE:
            {
                const CIMCreateClassRequestMessage* req =
                    reinterpret_cast<const CIMCreateClassRequestMessage*>(
                        request);
                AuditLogger::logUpdateClassOperation(
                    "CreateClass",
                    AuditLogger::EVENT_CREATE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->className,
                    response->cimException.getCode());
                break;
            }

            case CIM_MODIFY_CLASS_REQUEST_MESSAGE:
            {
                const CIMModifyClassRequestMessage* req =
                    reinterpret_cast<const CIMModifyClassRequestMessage*>(
                        request);
                AuditLogger::logUpdateClassOperation(
                    "ModifyClass",
                    AuditLogger::EVENT_UPDATE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->className,
                    response->cimException.getCode());
                break;
            }

            case CIM_DELETE_CLASS_REQUEST_MESSAGE:
            {
                const CIMDeleteClassRequestMessage* req =
                    reinterpret_cast<const CIMDeleteClassRequestMessage*>(
                        request);
                AuditLogger::logUpdateClassOperation(
                    "DeleteClass",
                    AuditLogger::EVENT_DELETE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->className,
                    response->cimException.getCode());
                break;
            }

            case CIM_SET_QUALIFIER_REQUEST_MESSAGE:
            {
                const CIMSetQualifierRequestMessage* req =
                    reinterpret_cast<const CIMSetQualifierRequestMessage*>(
                        request);
                AuditLogger::logUpdateQualifierOperation(
                    "SetQualifier",
                    AuditLogger::EVENT_UPDATE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->qualifierDeclaration.getName(),
                    response->cimException.getCode());
                break;
            }

            case CIM_DELETE_QUALIFIER_REQUEST_MESSAGE:
            {
                const CIMDeleteQualifierRequestMessage* req =
                    reinterpret_cast<const CIMDeleteQualifierRequestMessage*>(
                        request);
                AuditLogger::logUpdateQualifierOperation(
                    "DeleteQualifier",
                    AuditLogger::EVENT_DELETE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->qualifierName,
                    response->cimException.getCode());
                break;
            }

            case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            {
                const CIMCreateInstanceRequestMessage* req =
                    reinterpret_cast<const CIMCreateInstanceRequestMessage*>(
                        request);

                const CIMCreateInstanceResponseMessage* resp =
                    reinterpret_cast<const CIMCreateInstanceResponseMessage*>(
                        response);

                _getProviderName(
                    req->operationContext, moduleName, providerName);

                AuditLogger::logUpdateInstanceOperation(
                    "CreateInstance",
                    AuditLogger::EVENT_CREATE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    (response->cimException.getCode() == CIM_ERR_SUCCESS) ?
                        resp->instanceName :
                        CIMObjectPath(req->className.getString()),
                    moduleName,
                    providerName,
                    response->cimException.getCode());
                break;
            }

            case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            {
                const CIMModifyInstanceRequestMessage* req =
                    reinterpret_cast<const CIMModifyInstanceRequestMessage*>(
                        request);

                _getProviderName(
                    req->operationContext, moduleName, providerName);

                AuditLogger::logUpdateInstanceOperation(
                    "ModifyInstance",
                    AuditLogger::EVENT_UPDATE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->modifiedInstance.getPath(),
                    moduleName,
                    providerName,
                    response->cimException.getCode());
                break;
            }

            case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            {
                const CIMDeleteInstanceRequestMessage* req =
                    reinterpret_cast<const CIMDeleteInstanceRequestMessage*>(
                        request);

                _getProviderName(
                    req->operationContext, moduleName, providerName);

                AuditLogger::logUpdateInstanceOperation(
                    "DeleteInstance",
                    AuditLogger::EVENT_DELETE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->instanceName,
                    moduleName,
                    providerName,
                    response->cimException.getCode());
                break;
            }

            case CIM_SET_PROPERTY_REQUEST_MESSAGE:
            {
                const CIMSetPropertyRequestMessage* req =
                    reinterpret_cast<const CIMSetPropertyRequestMessage*>(
                        request);

                _getProviderName(
                    req->operationContext, moduleName, providerName);

                AuditLogger::logUpdateInstanceOperation(
                    "SetProperty",
                    AuditLogger::EVENT_UPDATE,
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->instanceName,
                    moduleName,
                    providerName,
                    response->cimException.getCode());
                break;
            }

            case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            {
                const CIMInvokeMethodRequestMessage* req =
                    reinterpret_cast<const CIMInvokeMethodRequestMessage*>(
                        request);

                _getProviderName(
                    req->operationContext, moduleName, providerName);

                AuditLogger::logInvokeMethodOperation(
                    req->userName,
                    req->ipAddress,
                    req->nameSpace,
                    req->instanceName,
                    req->methodName,
                    moduleName,
                    providerName,
                    response->cimException.getCode());
                break;
            }

            default:
                // Other operations are not logged.
                break;
        }
    }

    PEG_METHOD_EXIT();

#endif
}

/*  Send the given response synchronously using the given aggregation object.
    return whether the sent message was complete or not. The parameters are
    pointer references because they can be come invalid from external deletes
    if the message is complete after queueing. They can be zeroed in this
    function preventing the caller from referencing deleted pointers.
    If pull operation, send to EnumerationContext queue.  If not pull sent
    directly to output queue.
    Operations for internal clients are gathered completely since there is
    no chunking.
*/
Boolean CIMOperationRequestDispatcher::_enqueueResponse(
    OperationAggregate*& poA,
    CIMResponseMessage*& response)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_enqueueResponse");

    PEGASUS_DEBUG_ASSERT(poA->valid());
    PEGASUS_DEBUG_ASSERT(response->valid());

    // Obtain the _enqueueResponseMutex mutex for this chunked request.
    // This mutex serializes chunked responses from all incoming
    // provider threads. It is imperative that the sequencing done by the
    // resequenceResponse() method and the writing of the chunked response
    // to the connection socket (done as a synchronous enqueue at the end
    // of this method) are atomic to ensure that the chunk that is marked
    // as isComplete for the overall response is indeed the last chunk
    // returned to the client. See PEP 140 for details.
    // This mutex was moved into the OperationAggregate class as part of
    // bug 5157 because we only need to serialize on a per-request basis.
    // This prevents serializing independent requests on separate connections.
    AutoMutex autoMut(poA->_enqueueResponseMutex);

    Boolean isComplete = false;

    try
    {
        // get the completion status of the incoming response on this thread.
        isComplete = response->isComplete();

        poA->appendResponse(response);
        Uint32 type = poA->getRequestType();

        // There should never be more than one object in the list for async
        // queues these functions are called for their jobs other than
        // aggregating.
        // All CIMENUM.., CIM_REF,, and CIM_ASSOC... messages
        if (type != CIM_EXEC_QUERY_REQUEST_MESSAGE)
        {
            // Hostname completion required for the four assoc-like operations
            handleOperationResponseAggregation(poA);
        }
        else
        {
            handleExecQueryResponseAggregation(poA);
        }

        // now take the aggregated response. This is now the one we will
        // work with
        response = poA->removeResponse(Uint32(0));

        PEGASUS_DEBUG_ASSERT(response->valid());

        // reset the completion flag of the last response added to the list.
        // This only makes a difference when there was at least two on the list
        // to begin with before aggregation methods (above) were called.
        // Typically, there will be more than two on the list when a non-async
        // destination queue keeps appending the responses until the completion
        // of all threads/responses has occurred.

        response->setComplete(isComplete);

        response->dest = poA->_dest;
        poA->resequenceResponse(*response);

        // now get the completion status of the response after it has been
        // resequenced. This will reset the completion status of the entire
        // message, not just this one thread coming through here.

        isComplete = response->isComplete();

        // can the destination service queue handle async responses ?
        // (i.e multiple responses from one request). Certain known ones
        // cannot handle it. Most notably, the internal client.

        MessageQueue* q = lookup(poA->_dest);
        const char* name = q ? q->getQueueName() : 0;
        Boolean isDestinationQueueAsync = !(name &&
            (strcmp(name, PEGASUS_QUEUENAME_INTERNALCLIENT) == 0 ||
             strcmp(name, PEGASUS_QUEUENAME_WSMPROCESSOR) == 0));

        // for non-async queues, we'll just keep appending until all responses
        // have come in

        if (isDestinationQueueAsync == false)
        {
            if (isComplete == false)
            {
                poA->appendResponse(response);
                return isComplete;
            }

            // need to reset the first response to complete if the
            // last one that came in was complete

            response->setComplete(true);
            response->setIndex(0);
        }

        // Log only if this is the last of the aggregated response
        if (isComplete)
        {
            _logOperation(poA->getRequest(), response);
        }

        // If it is a pull operation response, send to the output caching queue
        // in the enumeration context.  If not a pull operation,
        // directly queue.
        if (poA->_pullOperation)
        {
            // pull operation. get and validate the enumerationContext
            EnumerationContext* en = poA->_enumerationContext;
            if (!en->valid())
            {
                EnumerationContext *en = _enumerationContextTable->find(
                    poA->_contextId);

                if (en == 0)
                {
                    // Did not find one means that the enumcontext cleaned up
                    // before provider responded. Just issue discard trace
                    PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                        "Provider Response for non-Existent ContextId=%s"
                        " providers. Response Discarded",
                        (const char *)poA->_contextId.getCString() ));
                    // KS_TODO should we discard the response here and what
                    // about the poA. Not really sure but it is a very rare
                    // condition, probably when we have closed enum before
                    // providers complete. Probably only if providers delay
                    // several minutes before response.
                }
                // If we found one but it is invalid, we have a system error
                else
                {
                    PEGASUS_ASSERT(false);
                    // Concluded that an invalid enum on response is really
                    // a system error and should cause failure.  We may want
                    // to rethink that in the future
////                  PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
////                      "Provider Response for invalid EnumerationContext %s"
////                      "providers. System Error",
////                      (const char *)poA->_contextId.getCString() ));

                }
                // return true, just discards everything.
                return true;
            }
            else
            {
                EnumerationContext* ent = _enumerationContextTable->find(
                    poA->_contextId);
                PEGASUS_DEBUG_ASSERT(ent == en);
            }

            PEGASUS_DEBUG_ASSERT(poA->_contextId == en->getContextId());

            // When provider response receved, clear this counter.
            en->clearConsecutiveZeroLenObjectResponseCounter();

            // If this is an exception set the error in EnumerationContext
            if (response->cimException.getCode())
            {
                en->setErrorState(response->cimException);
            }

            // Send to the EnumerationContext cache with the
            // isComplete indicator. Return indicates whether cache is closed
            // and providers complete for cleanup.
            // The enumerationContext could also be cleaned up by the
            // timer thread once the providers are marked complete in
            // the enumerationContext (if the client had timed out )

            en->lockContext();

            // putCache returns true if client not closed
            if (en->putCache(response, isComplete))
            {
                // if there are responses and there is a
                // waiting future response, issue the response
                // If request pending, test to see if responses exist to
                // be sent.
                if (en->_savedRequest != NULL)
                {
                    // If there are responses or error to send, send response
                    if ( (en->testCacheForResponses(
                        en->_savedOperationMaxObjectCount,
                        requireCompleteResponses)) )
                    {
                        PEGASUS_DEBUG_ASSERT(en->_savedRequest->valid());
                        PEGASUS_DEBUG_ASSERT(en->_savedResponse->valid());

                        // Issue response. This may mark context closed.
                        _issueImmediateOpenOrPullResponseMessage(
                            en->_savedRequest,
                            en->_savedResponse,
                            en,
                            en->_savedOperationMaxObjectCount);

                        // Delete the request; it was allocated for
                        // this delayed response.
                        delete en->_savedRequest;

                        // clear the saved request to indicate it was used
                        en->_savedRequest = NULL;
                        en->_savedResponse = NULL;
                        en->_savedOperationMaxObjectCount = 0;
                    }
                }
            }
            else
            {
                if (en->_savedRequest != NULL)
                {
                    delete en->_savedRequest;
                    delete en->_savedResponse;

                    en->_savedRequest = NULL;
                    en->_savedResponse = NULL;
                }
            }

            if (isComplete)
            {
                en->setProvidersComplete();
            }
            PEGASUS_DEBUG_ASSERT(response->valid());

            delete response;

            // If closed  on client side, we can release
            // This MUST BE last operation on the en since
            // the en could be removed by one of the other threads
            // if conditions are correct.
            if (en->isClientClosed() && isComplete)
            {
                // en may be deleted after this call
                _enumerationContextTable->releaseContext(en);
            }
            else
            {
                en->unlockContext();

                // If providers not complete and client open, test for cache
                // overload  before continuing. Do not wait if client is
                // closed since goal is to remove any existing
                // provider responses
                if (!isComplete && !en->isClientClosed())
                {
                    // Wait here if the cache is too large. Sending
                    // requests will reduce the size of the cache and
                    // signal this wait function when size returns below
                    // limit.
                    en->waitCacheSize();
                }
            }
        }
        // Otherwise not a pull operation; queue response
        // Send it syncronously so that multiple responses will show up in the
        // receiving queue according to the order that we have set the response
        // index. If this was a single complete response, we could in theory
        // send it async (i.e SendForget), however, there is no need to make a
        // condition point based off this.
        else
        {
            if (q)
            {
                q->enqueue(response);
            }
            else
            {
                PEG_METHOD_EXIT();
                throw UninitializedObjectException();
            }
        }
    }
    catch (...)
    {
        static const char failMsg[] =
            "Failed to resequence/aggregate/forward response";
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "%s%s",
            "CIMOperationRequestDispatcher::_enqueueResponse",
            failMsg));

        if (response->cimException.getCode() != CIM_ERR_SUCCESS)
            response->cimException =
                CIMException(CIM_ERR_FAILED, String(failMsg));
    }

    // after sending, the response has been deleted externally
    response = 0;

    PEG_METHOD_EXIT();
    return isComplete;
}

void CIMOperationRequestDispatcher::_handle_async_request(AsyncRequest* req)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_handle_async_request");

    // pass legacy operations to handleEnqueue
    // unless it is a CIM_NOTIFY_CONFIG_CHANGE
    if (req->getType() == ASYNC_ASYNC_LEGACY_OP_START)
    {
        AutoPtr<Message> legacy(
            static_cast<AsyncLegacyOperationStart *>(req)->get_action());

        AutoPtr<CIMResponseMessage> response;
        if (legacy->getType() == CIM_NOTIFY_CONFIG_CHANGE_REQUEST_MESSAGE)
        {
            try
            {
                response.reset(
                   handlePropertyUpdateRequest(
                       (CIMNotifyConfigChangeRequestMessage*)
                           legacy.get()));
            }
            catch (Exception& e)
            {
                response.reset(((CIMRequestMessage*)
                                legacy.get())->buildResponse());
                response->cimException =
                    PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, e.getMessage());
            }
            catch (...)
            {
                response.reset(((CIMRequestMessage*)
                                legacy.get())->buildResponse());
                response->cimException =
                    PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, "Exception: Unknown");
            }

            AutoPtr<AsyncLegacyOperationResult> result(
                new AsyncLegacyOperationResult(
                    req->op,
                    response.get()));
            response.release();
            result.release();
            _complete_op_node(req->op);
        }
        else
        {
            Message *message =
                static_cast<AsyncLegacyOperationStart*>(req)->get_action();

            handleEnqueue(message);

        }
        PEG_METHOD_EXIT();
        return;
    }

    // pass all other operations to the default handler
    Base::_handle_async_request(req);
    PEG_METHOD_EXIT();
}

//
// Update the DeliveryRetryAttempts & DeliveryRetryInterval
// with the new property value
//
CIMNotifyConfigChangeResponseMessage*
    CIMOperationRequestDispatcher::handlePropertyUpdateRequest(
        CIMNotifyConfigChangeRequestMessage *message)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
        "CIMOperationRequestDispatcher::_handlePropertyUpdateRequest");
    CIMNotifyConfigChangeRequestMessage * notifyRequest=
           dynamic_cast<CIMNotifyConfigChangeRequestMessage*>(message);

    Uint64 v;

    StringConversion::decimalStringToUint64(
        notifyRequest->newPropertyValue.getCString(),v);

    if (String::equal(
        notifyRequest->propertyName, "pullOperationsMaxObjectCount"))
    {
        _systemPullOperationMaxObjectCount = (Uint32)v;
    }
    else if(String::equal(
        notifyRequest->propertyName, "pullOperationsMaxTimeout"))
    {
         _pullOperationMaxTimeout = (Uint32)v;
    }
    else if (String::equal(
        notifyRequest->propertyName, "pullOperationsDefaultTimeout"))
    {
        EnumerationContextTable::setDefaultOperationTimeoutSec(v);
    }
     else
    {
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }
    CIMNotifyConfigChangeResponseMessage *response =
        dynamic_cast<CIMNotifyConfigChangeResponseMessage*>(
            message->buildResponse());
    PEG_METHOD_EXIT();
    return response;
}

/*
    Get an internal provider that matches the namespace/classname pair
    @parameter nameSpace CIMNamespaceName of desired provider
    $param className CIMName input with className of desired provider
    @providerInfo ProviderInfo with registration information for registered
    provider if provider found
    @return True if internal provider found.  If found, registration
    information is in provider.  False if no internal provider found.
*/
Boolean CIMOperationRequestDispatcher::_lookupInternalProvider(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    ProviderInfo& providerInfo)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_lookupInternalProvider");

    // Clear the parameters since used as test after getRouting call
    Uint32 serviceId = 0;
    String controlProviderName;

    providerInfo.hasProvider = _routing_table->getRouting(
        className,
        nameSpace,
        controlProviderName,
        serviceId);

    if (providerInfo.hasProvider)
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "Internal provider Service = %s provider %s found.",
            _getServiceName(serviceId),
            CSTRING(controlProviderName) ));

        providerInfo.serviceId = serviceId;
        providerInfo.controlProviderName = controlProviderName;
    }

    PEG_METHOD_EXIT();
    return providerInfo.hasProvider;
}

/**
    _getSubClassNames - Gets the names of all subclasses of the defined
    class (including the class) and returns it in an array of strings. Uses a
    similar function in the repository class to get the names.  Note that
    this prepends the result with the input classname.
    @param namespace
    @param className
    @return Array of strings with class names.  Note that there should be at
    least one classname in the array (the input name)
    Note that there is a special exception to this function, the __namespace
    class which does not have any representation in the class repository.
    @exception CIMException(CIM_ERR_INVALID_CLASS)
*/
Array<CIMName> CIMOperationRequestDispatcher::_getSubClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_getSubClassNames");

    Array<CIMName> subClassNames;
    //
    // Get names of descendent classes:
    // Ignore if the special class __Namespace because it works in very
    // strange way.  NOTE: This class is also deprecated in DSP0200.
    //
    if (!className.equal (PEGASUS_CLASSNAME___NAMESPACE))
    {
        // Get the complete list of subclass names
        // getSubClassNames throws an exception if the class does not exist
        _repository->getSubClassNames(nameSpace,
             className, true, subClassNames);
    }

    // Prepend the array with the classname from the request so
    // return is this class and all subclasses
    subClassNames.prepend(className);

    PEG_METHOD_EXIT();
    return subClassNames;
}

///////////////////////////////////////////////////////////////////////////
//
// Provider Lookup Functions - Gets list of providers for operation types
//       (ex. associations, enumerate, get, invokeMethod
//
//////////////////////////////////////////////////////////////////////////
//
/* lookupAllInstanceProviders - Returns the list of all subclasses of this
   class along with information about whether it is an instance provider.
   @param nameSpace - Namespace for the lookup.
   @param className - className for which we are finding providers
   @param providerCount - Returns count of providers in the response array.
   This is because the array returned is all subclasses, not simply those
   with providers.
   @return Returns an array of ProviderInfo, one entry for each subclass.
   Each ProviderInfo instance defines whether a provider exists and the
   information on the provider so that the operation can be forwarded to the
   provider.
   @exception - Throws exception if the className is in error or the request
   is to broad.  The className exception should NEVER occur since
   the className should have already been validated in the operation
   validation code.
*/

ProviderInfoList CIMOperationRequestDispatcher::_lookupAllInstanceProviders(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_lookupAllInstanceProviders");

    ProviderInfoList providerList;

    Array<CIMName> classNames = _getSubClassNames(nameSpace, className);

    // Loop for all classNames found
    ConstArrayIterator<CIMName> iterator(classNames);
    for (Uint32 i = 0, n = iterator.size(); i < n; i++)
    {
        // Lookup any instance providers and add to send list
        ProviderInfo providerInfo =
            _lookupInstanceProvider(nameSpace, iterator[i]);

        if (providerInfo.hasProvider)
        {
            providerList.providerCount++;

            PEG_TRACE((TRC_DISPATCHER,Tracer::LEVEL4,
                "Provider found for class = %s servicename = %s "
                "controlProviderName = %s",
                CSTRING(providerInfo.className.getString()),
                _getServiceName(providerInfo.serviceId),
                CSTRING(providerInfo.controlProviderName)));
        }

        // Always add providerInfo to be sure we include classes without
        // providers. This does not increment provider count
        providerList.appendClass(providerInfo);
   }

    // Reject if the request is too broad
    _rejectEnumerateTooBroad(
        nameSpace, className, providerList.providerCount);

   // Diagnostic to display list of classes returned. Enable the following
   // line to see what this function returns.
   // providerList.trace(nameSpace, className);


   PEG_METHOD_EXIT();
   return providerList;
}

/* _lookupInstanceProvider - Looks up the internal or registered instance
    provider for the classname and namespace.
    @return ProviderInfo struct with information about the provider found.

    If ProviderInfo.hasProvider returns false, no provider for this
    classname/namespace combination.

    This function calls both the _lookupInternalProvider function and if
    nothing found, the providerRegistration _lookupInstanceProvider function.

*/
ProviderInfo CIMOperationRequestDispatcher::_lookupInstanceProvider(
   const CIMNamespaceName& nameSpace,
   const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_lookupInstanceProvider");

    ProviderInfo providerInfo(className);

    // Check for class provided by an internal provider
    Boolean hasControlProvider = _lookupInternalProvider(
        nameSpace,
        className,
        providerInfo);

    // If no internal provider, try registered providers
    if (!hasControlProvider)
    {
        CIMInstance pInstance;
        CIMInstance pmInstance;
        Boolean hasNoQuery = true;

        Boolean hasProvider =
            _providerRegistrationManager->lookupInstanceProvider(
                nameSpace,
                className,
                pInstance,
                pmInstance,
                false,
                &hasNoQuery);

        if (hasProvider)
        {
            providerInfo.addProviderInfo(_providerManagerServiceId, true,
                                         hasNoQuery);

           ProviderIdContainer* pi = _updateProviderContainer(
               nameSpace, pInstance, pmInstance);

           providerInfo.providerIdContainer.reset(pi);

#ifdef PEGASUS_ENABLE_OBJECT_NORMALIZATION
            if (ObjectNormalizer::getEnableNormalization())
            {
                // normalization is enabled for all providers unless they
                // have an old interface version or are explicity excluded by
                // the excludeModulesFromNormalization configuration option.

                // check interface type and version

                String interfaceType;
                String interfaceVersion;
                Uint32 pos = PEG_NOT_FOUND;

                // get the provder interface type
                if ((pos = pmInstance.findProperty("InterfaceType")) !=
                        PEG_NOT_FOUND)
                {
                    pmInstance.getProperty(pos).getValue().get(interfaceType);
                }

                // get the provder interface version
                if ((pos = pmInstance.findProperty("InterfaceVersion")) !=
                        PEG_NOT_FOUND)
                {
                    pmInstance.getProperty(pos).
                        getValue().get(interfaceVersion);
                }

                // compare the interface type and vesion
                if (String::equalNoCase(interfaceType, "C++Default"))
                {
                    // version must be greater than 2.5.0
                    if (VersionUtil::isVersionGreaterOrEqual(
                        interfaceVersion, 2, 5, 0))
                    {
                        providerInfo.hasProviderNormalization = true;
                    }
                }
                else if (String::equalNoCase(interfaceType, "CMPI"))
                {
                    // version must be greater than 2.0.0
                    if (VersionUtil::isVersionGreaterOrEqual(
                        interfaceVersion, 2, 0, 0))
                    {
                        providerInfo.hasProviderNormalization = true;
                    }
                }
                else if (String::equalNoCase(interfaceType, "JMPI"))
                {
                    // version must be greater than 1.0.0
                    if (VersionUtil::isVersionGreaterOrEqual(
                        interfaceVersion, 1, 0, 0))
                    {
                        providerInfo.hasProviderNormalization = true;
                    }
                }

                // check for module exclusion

                String moduleName;

                // get the provider module name
                if ((pos = pmInstance.findProperty(
                    PEGASUS_PROPERTYNAME_NAME)) != PEG_NOT_FOUND)
                {
                    pmInstance.getProperty(pos).getValue().get(moduleName);
                }

                // check if module name is on excludeModulesFromNormalization
                // list
                for (Uint32 i = 0, n = _excludeModulesFromNormalization.size();
                     i < n; i++)
                {
                    if (String::equalNoCase(
                            moduleName, _excludeModulesFromNormalization[i]))
                    {
                        providerInfo.hasProviderNormalization = false;
                        break;
                    }
                }

                PEG_TRACE((TRC_DISPATCHER,Tracer::LEVEL4,
                    "Normalization for provider module %s is %s.",
                    CSTRING(moduleName),
                    (providerInfo.hasProviderNormalization ?
                         "enabled" : "disabled")));
            }
#endif

        }  // no provider or control provider
        else
        {
            PEG_TRACE((TRC_DISPATCHER,Tracer::LEVEL2,
                "Provider for %s not found.",
                CSTRING(className.getString()) ));
        }
    }

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "CIMOperationRequestDispatcher::_lookupInstanceProvider - "
            "Namespace: %s  Class Name: %s  Service Name: %s  "
            "Provider Name: %s found. hasProvider = %s",
        CSTRING(nameSpace.getString()),
        CSTRING(className.getString()),
        _getServiceName(providerInfo.serviceId),
        CSTRING(providerInfo.controlProviderName),
        (providerInfo.hasProvider ? "true" : "false")));

    PEG_METHOD_EXIT();
    return providerInfo;
}

/*
    Lookup the method provider for the namespace, classname, and methodname
    provided.
    NOTE: Lookup of internal providers is handled by the method request handler
    today.  This should probably be fixed to provide exactly the
    same lookup for all types in the future.
*/
String CIMOperationRequestDispatcher::_lookupMethodProvider(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& methodName,
    ProviderIdContainer** providerIdContainer)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_lookupMethodProvider");

    CIMInstance pInstance;
    CIMInstance pmInstance;
    String providerName;

    // lookup the provider. Returns provider instance and provider
    // module instance.
    if (_providerRegistrationManager->lookupMethodProvider(
            nameSpace, className, methodName, pInstance, pmInstance))
    {
        (*providerIdContainer) = _updateProviderContainer(
            nameSpace, pInstance, pmInstance);

        // get the provder name
        Uint32 pos = pInstance.findProperty(PEGASUS_PROPERTYNAME_NAME);

        if (pos != PEG_NOT_FOUND)
        {
            pInstance.getProperty(pos).getValue().get(providerName);
        }
    }
    // return providerName. Empty if method provider not found.
    PEG_METHOD_EXIT();
    return providerName;
}

/*  _lookupAllAssociation Providers
    Returns all of the association providers that exist for the defined class
    name.  Uses the referencenames function to get list of classes for which
    providers required and then looks up the providers for each     class
    @param nameSpace
    @param objectName
    @param - assocClass referenced parameter that is modified in this function.
    @param role String defining role for this association
    @param providerCount count of actual providers found, not the count of
    classes.  This differs from the count of classes in that the providerInfo
    list is all classes including those that would go to the repository.
    @returns List of ProviderInfo
    @exception - Exceptions From the Repository
*/
ProviderInfoList CIMOperationRequestDispatcher::_lookupAllAssociationProviders(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass,
        const String& role)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_lookupAllAssociationProviders");

    ProviderInfoList providerInfoList;

    CIMName className = objectName.getClassName();
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "Association Class Lookup for Class %s  and assocClass %s",
        CSTRING(className.getString()),
        CSTRING(assocClass.getString()) ));

    // The association class is the basis for association registration.
    // When an association class request is received by the CIMOM the target
    // class is the endpoint class or instance.  Prevously we also called
    // providers registered against this class.  Thus, typically this would
    // be the same class as the instance class.  For example ClassA with
    // Assoc1.  We would register an instance and Association provider
    // against Class A.  With this change, we register the Instance class
    // as before, but register the association class with the association
    // itself, i.e. register it against the class Assoc1.
    // In this case, the list of potential classes for providers is the same
    // as the reference names, i.e the association classes that have
    // associations pointing back to the target class.

    // ATTN: KS 20030515. After we test and complete the move to using this
    // option, go back and change the call to avoid the double
    // conversion to and from CIM Object path.

    Array<CIMObjectPath> objPaths;

    try
    {
        // Note:  We use assocClass because this is the association function.
        // The Reference(Name)s calls must put the resultClass here.
        objPaths = _repository->referenceNames(
            nameSpace,
            CIMObjectPath(String::EMPTY, CIMNamespaceName(), className),
            assocClass,
            role);
    }
    catch (...)
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "CIMOperationRequestDispatcher::lookupAllAssociationProvider "
                "exception.  Namespace: %s  Object Name: %s  Assoc Class: %s",
            CSTRING(nameSpace.getString()),
            CSTRING(objectName.toString()),
            CSTRING(assocClass.getString())));
        throw;
    }

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "Association Lookup: %u classes found", objPaths.size()));

    // lookup association providers for all classes returned by the
    // reference names lookup
    for (Uint32 i = 0; i < objPaths.size(); i++)
    {
        CIMName cn = objPaths[i].getClassName();

        // Create ProviderInfo object with default info and class name
        ProviderInfo pi(cn);

        // Use returned classname for the association classname
        // under the assumption that the registration is for the
        // association class, not the target class
        if (_lookupAssociationProvider(nameSpace, cn, pi))
        {
            providerInfoList.appendProvider(pi);

            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                "Association append: class = %s to list. count = %u",
                CSTRING(objPaths[i].getClassName().getString()),
                providerInfoList.size() ));
        }
    }
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "Association providerCount = %u.", providerInfoList.providerCount));

    PEG_METHOD_EXIT();
    return providerInfoList;
}

/* _lookupAssociationProvider - Look up the internal and/or registered
    association provider for the defined namespace and class and return a
    providerInfo struct containing information about the provider if found.
    @param nameSpace
    @param assocClass
    @param providerInfo ProviderInfo& container holding information about
    the provider found including name, etc.
    @return true if a provider is found for the defined class and namespace.
*/
Boolean CIMOperationRequestDispatcher::_lookupAssociationProvider(
    const CIMNamespaceName& nameSpace,
    const CIMName& assocClass,
    ProviderInfo& providerInfo)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_lookupAssociationProvider");

    ProviderIdContainer* providerIdContainer=NULL;
    String providerName;

    // Check for class provided by an internal provider
    if (!_lookupInternalProvider(nameSpace, assocClass, providerInfo))
    {
        // get provider for class from registered providers. Note that we
        // reduce it from multiple possible class names to a single one here.
        // This limit is defined by ProviderRegistrationManager in that
        // it will only return one provider since we allow only a single
        // provider for a class. (See bug 9581)
        Array<String> assocProviderList =
            _lookupRegisteredAssociationProvider(
                nameSpace, assocClass, &providerIdContainer);

        //  Provider registration should NEVER return more than a single
        //  provider for this lookup by design of provider registration today.
        if (assocProviderList.size() > 0)
        {
            providerInfo.addProviderInfo(_providerManagerServiceId,true,false);
            providerName = assocProviderList[0];
        }
    }

    // Fill out providerInfo providerIdContainer
    if (providerInfo.hasProvider)
    {
        providerInfo.providerIdContainer.reset(providerIdContainer);
    }
    else
    {
        providerInfo.providerIdContainer.reset();
    }

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "Association Provider %s for class=\"%s\"  in namespace \"%s\"."
            " servicename=\"%s\" provider = \"%s\" controlProvider = \"%s\"",
        (providerInfo.hasProvider? "found" : "NOT found"),
        CSTRING(assocClass.getString()),
        CSTRING(nameSpace.getString()),
        _getServiceName(providerInfo.serviceId),
        CSTRING(providerName),
        CSTRING(providerInfo.controlProviderName) ));

    PEG_METHOD_EXIT();
    return providerInfo.hasProvider;
}

// Lookup the registered Association provider(s) for this class and convert
// the result to an array of provider names.
// NOTE: The code allows for multiple providers but there is no reason
// to have this until we really support multiple providers per class.
// The multiple provider code was here because there was some code in to
// do the lookup in provider registration which was removed. Some day we
// will support multiple providers per class so it was logical to just leave
// the multiple provider code in place.
// NOTE: assocClass and resultClass are optional
//
Array<String>
   CIMOperationRequestDispatcher::_lookupRegisteredAssociationProvider(
   const CIMNamespaceName& nameSpace,
   const CIMName& assocClass,
   ProviderIdContainer** providerIdContainer)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_lookupRegisteredAssociationProvider");

    // instances of the provider class and provider module class for the
    // response
    Array<CIMInstance> pInstances; // Providers
    Array<CIMInstance> pmInstances; // ProviderModules

    Array<String> providerNames;
    String providerName;

    //get list of registered association providers
    Boolean returnValue =
        _providerRegistrationManager->lookupAssociationProvider(
            nameSpace, assocClass, pInstances, pmInstances);

    if (returnValue)
    {
        for (Uint32 i = 0, n = pInstances.size(); i < n; i++)
        {
            // At present only one provider per class or per association is
            // supported and the same provider is stored in the
            // providerIdContainer. So the array will actually have only one
            // item. And hence with the same element providerIdContainer will
            // be created. When we start supporting multiple providers per
            // class or assoc, we need to change the code to make
            // providerIdContainer accordingly.
            if (i == 0)
            {
                // Call function that depends on the Remote CMPI flag
                // to update ProviderIdContainer
                (*providerIdContainer) = _updateProviderContainer(
                    nameSpace, pInstances[i], pmInstances[i]);
            }

            // get the provider name
            Uint32 pos = pInstances[i].findProperty(PEGASUS_PROPERTYNAME_NAME);

            if ( pos != PEG_NOT_FOUND )
            {
                pInstances[i].getProperty(pos).getValue().get(providerName);

                PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                    "Association providerName = %s found for Class %s",
                    CSTRING(providerName),
                    CSTRING(assocClass.getString())));
                    providerNames.append(providerName);
            }
        }
    }

    if (providerNames.size() == 0)
    {
        PEG_TRACE((TRC_DISPATCHER,Tracer::LEVEL3,
            "Association Provider NOT found for Class %s in nameSpace %s",
            CSTRING(assocClass.getString()), CSTRING(nameSpace.getString()) ));
    }
    PEG_METHOD_EXIT();
    return providerNames;
}

// Service function to build the ProviderIdContainer.
// If remote CMPI enabled and if this is a remote namespace, it must
// place the remote info into the container.  Otherwise it just builds
// a new container with pmInstance and pInstance and returns it
// NOTE: nameSpace is only used in the case of REMOTE_CMPI

ProviderIdContainer* CIMOperationRequestDispatcher::_updateProviderContainer(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& pInstance,
    const CIMInstance& pmInstance)
{
        ProviderIdContainer* pc;
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
        String remoteInformation;
        Boolean isRemote = false;
        isRemote = _repository->isRemoteNameSpace(
            nameSpace,
            remoteInformation);
        if (isRemote)
        {
            pc = new ProviderIdContainer(
                pmInstance, pInstance, isRemote, remoteInformation);
        }
        else
        {
            pc = new ProviderIdContainer(pmInstance, pInstance);
        }
#else
        pc = new ProviderIdContainer(pmInstance, pInstance);
#endif
        return pc;
}

/*****************************************************************************
**
** Dispatcher callback functions.  These functions are called by the
** service/providerManager generating responses to return response information
** to the Dispatcher Response handlers.  The reference to the correct handler
** is provided by the Dispatcher request handlers when requests are passed
** forward for services or provider managers, i.e. _forwardRequest* Functions)
**
******************************************************************************/
/*  Dispatcher callback for response aggregation
    The userParameter contains the OperationAggregate for this operation
*/
void CIMOperationRequestDispatcher::_forwardedForAggregationCallback(
    AsyncOpNode* op,
    MessageQueue* q,
    void* userParameter)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_forwardForAggregationCallback");

    CIMOperationRequestDispatcher* service =
        static_cast<CIMOperationRequestDispatcher*>(q);

    AsyncRequest* asyncRequest = static_cast<AsyncRequest*>(op->getRequest());
    AsyncReply* asyncReply = static_cast<AsyncReply*>(op->removeResponse());

    OperationAggregate* poA =
        reinterpret_cast<OperationAggregate*>(userParameter);

    PEGASUS_ASSERT(asyncRequest != 0);
    PEGASUS_ASSERT(asyncReply != 0);

    PEGASUS_DEBUG_ASSERT(asyncReply->valid());
    PEGASUS_DEBUG_ASSERT(asyncRequest->valid());

    PEGASUS_ASSERT(poA != 0);
    PEGASUS_DEBUG_ASSERT(poA->valid());

    CIMResponseMessage* response = 0;

    MessageType msgType = asyncReply->getType();

    if (msgType == ASYNC_ASYNC_LEGACY_OP_RESULT)
    {
        response = reinterpret_cast<CIMResponseMessage*>(
            (static_cast<AsyncLegacyOperationResult*>(asyncReply))->
                get_result());
    }
    else if (msgType == ASYNC_ASYNC_MODULE_OP_RESULT)
    {
        response = reinterpret_cast<CIMResponseMessage*>(
            (static_cast<AsyncModuleOperationResult*>(asyncReply))->
                get_result());
    }
    else
    {
        // This should never happen.
        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
    }

    PEGASUS_ASSERT(response != 0);
    PEGASUS_DEBUG_ASSERT(response->valid());
    PEGASUS_ASSERT(response->messageId == poA->_messageId);

    delete asyncReply;

    // Before resequencing, the isComplete() flag represents the completion
    // status of one provider's response, not the entire response

    Boolean thisResponseIsComplete = response->isComplete();
    if (thisResponseIsComplete)
    {
        // these are per provider instantiations
        op->removeRequest();
        delete asyncRequest;
        service->return_op(op);
    }

    // KS_TODO this is a temporary diagnostic. Remove before release
    if (poA->_pullOperation)
    {
        PEGASUS_DEBUG_ASSERT(poA->_enumerationContext);
    }

    // After resequencing, this flag represents the completion status of
    // the ENTIRE response to the request.
    // Call the response handler for aggregating responses
    Boolean entireResponseIsComplete = service->_enqueueResponse(poA, response);

    if (entireResponseIsComplete)
    {
        // delete OperationAggregation and attached request.
        delete poA;
        poA = 0;
    }

    // FUTURE Consider removing this trace in the future
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "Provider thisResponse = %s. Entire response = %s",
        (thisResponseIsComplete? "complete": "incomplete"),
        (entireResponseIsComplete? "complete": "incomplete")  ));

    PEG_METHOD_EXIT();
}


/*  Dispatcher Callback function for nonAggregation calls.
    The userParameter contains the request message for this operation
*/
void CIMOperationRequestDispatcher::_forwardedRequestCallback(
    AsyncOpNode* op,
    MessageQueue* q,
    void* userParameter)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_forwardedRequestCallback");

    CIMOperationRequestDispatcher* service =
        static_cast<CIMOperationRequestDispatcher*>(q);

    AsyncRequest* asyncRequest =
        static_cast<AsyncRequest*>(op->removeRequest());
    AsyncReply* asyncReply = static_cast<AsyncReply*>(op->removeResponse());

    CIMOperationRequestMessage* request =
        reinterpret_cast<CIMOperationRequestMessage*>(userParameter);
    PEGASUS_ASSERT(request != 0);

    CIMResponseMessage* response = 0;

    PEGASUS_DEBUG_ASSERT(request->valid());

    MessageType msgType = asyncReply->getType();

    if (msgType == ASYNC_ASYNC_LEGACY_OP_RESULT)
    {
        response = reinterpret_cast<CIMResponseMessage*>(
            (static_cast<AsyncLegacyOperationResult*>(asyncReply))->
                get_result());
    }
    else if (msgType == ASYNC_ASYNC_MODULE_OP_RESULT)
    {
        response = reinterpret_cast<CIMResponseMessage*>(
            (static_cast<AsyncModuleOperationResult*>(asyncReply))->
                get_result());
    }
    else
    {
        // Error
    }

    PEGASUS_ASSERT(response != 0);
    PEGASUS_DEBUG_ASSERT(response->valid());

    Boolean isComplete = response->isComplete();

    service->_enqueueResponse(request, response);

    if (isComplete == true)
    {
        delete request;
        delete asyncRequest;
        delete asyncReply;
        service->return_op(op);
    }

    PEG_METHOD_EXIT();
}

/***************************************************************************
**
**  The _forwardRequest* functions forward CIMOperation Requests to
**  other services for processing. Normally this will be either
**  Control Providers, services, or Provider Managers
**
***************************************************************************/

/*  Forward requests for response aggregation( enums, etc. that
    call multiple providers) to other services (providerManager, etc).
    (i.e. requests where the callback is the function
    _forwardForAggregationCallback. These requests include an
    OperationAggregate structure that controls response handling.

    This function decides based on the controlProviderName Field
    whether to forward to Service or ControlProvider.
    If controlProviderName String empty, ToService, else toControlProvider.

    If a response is provided with the input, the caller wants to execute only
    the callback asychnonously but not call another service.
*/
void CIMOperationRequestDispatcher::_forwardRequestForAggregation(
    Uint32 serviceId,
    const String& controlProviderName,
    CIMOperationRequestMessage* request,
    OperationAggregate* poA,
    CIMResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_forwardRequestForAggregation");

    PEGASUS_ASSERT(serviceId);

    // create an AsyncOpNode
    AsyncOpNode* op = this->get_op();

    // if a response is provided, execute only the asynchronous callback,
    // rather than forward to the provider.
    if (response)
    {
        // constructor of object is putting itself into a linked list
        // DO NOT remove the new operator
        new AsyncLegacyOperationResult(op, response);

        // Setting this to complete, allows ONLY the callback to run
        // without going through the typical async request apparatus
        op->complete();
    }

    // If ControlProviderName empty, forward to service.
    if (controlProviderName.size() == 0)
    {
        // constructor of object is putting itself into a linked list
        // DO NOT remove the new operator
        new AsyncLegacyOperationStart(op,serviceId,request);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
            "Forwarding %s to service %s. Response should go to queue %s.",
            MessageTypeToString(request->getType()),
            _getServiceName(serviceId),
            ((MessageQueue::lookup(request->queueIds.top())) ?
            ((MessageQueue::lookup(request->queueIds.top()))->getQueueName()) :
                   "BAD queue name")));
    }
    else
    {
        // constructor of object is putting itself into a linked list
        // DO NOT remove the new operator
        new AsyncModuleOperationStart(
            op,
            serviceId,
            controlProviderName,
            request);

       PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
           "Forwarding %s to service %s, control provider %s. "
           "Response should go to queue %s.",
           MessageTypeToString(request->getType()),
           _getServiceName(serviceId),
           CSTRING(controlProviderName),
           ((MessageQueue::lookup(request->queueIds.top())) ?
           ((MessageQueue::lookup(request->queueIds.top()))->getQueueName()) :
                  "BAD queue name")));
    }

    SendAsync(
        op,
        serviceId,
        CIMOperationRequestDispatcher::_forwardedForAggregationCallback,
        this,
        poA);

    PEG_METHOD_EXIT();
}

/** _forwardRequestToSingleProvider
    This function forwards the request to a single provider,
    control provider or service. It decides based on
    the controlProviderName parameter whether to forward to
    Service/controlProvider or to the provider manager service.
    If controlProviderName String empty,
           ForwardToProviderManagerService,
       else
           ForwardtoControlProvider/service
    As part of that forwarding process in defines a callback
    function, _forwardRequestCallback(...) for responses to the
    request.
*/
void CIMOperationRequestDispatcher::_forwardRequestToSingleProvider(
    const ProviderInfo& providerInfo,
    CIMOperationRequestMessage* request,
    CIMOperationRequestMessage* requestCopy)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_forwardRequestToSingleProvider");

    PEGASUS_ASSERT(providerInfo.serviceId);

    AsyncOpNode* op = this->get_op();

    // If ControlProviderName empty, forward to service defined on input
    if (providerInfo.controlProviderName.size() == 0)
    {
        AsyncLegacyOperationStart* asyncRequest =
            new AsyncLegacyOperationStart(
                op,
                providerInfo.serviceId,
                request);

        asyncRequest->dest = providerInfo.serviceId;
    }
    // control provider name exists.
    else
    {
        // constructor of object is putting itself into a linked list
        // DO NOT remove the new operator
        new AsyncModuleOperationStart(
            op,
            providerInfo.serviceId,
            providerInfo.controlProviderName,
            request);
    }
    // Forward the request asynchronously with call back to
    // _forwardedRequestCallback()
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "Forwarding %s on class %s to service %s, control provider %s. "
        "Response to queue %s.",
        MessageTypeToString(request->getType()),
        CSTRING(providerInfo.className.getString()),
        _getServiceName(providerInfo.serviceId),
        (CSTRING(providerInfo.controlProviderName)),
        ((MessageQueue::lookup(request->queueIds.top())) ?
        ((MessageQueue::lookup(request->queueIds.top()))->getQueueName()) :
               "BAD queue name")));

    SendAsync(
       op,
       providerInfo.serviceId,
       CIMOperationRequestDispatcher::_forwardedRequestCallback,
       this,
       requestCopy);

    PEG_METHOD_EXIT();
}

/*
    Enqueue an Exception response
    Helper functions that create a response message with the defined
    exception and queues it.
*/
void CIMOperationRequestDispatcher::_enqueueExceptionResponse(
   CIMOperationRequestMessage* request,
   CIMException& exception)
{
    CIMResponseMessage* response = request->buildResponse();
    response->cimException = exception;
    _enqueueResponse(request, response);
}

void CIMOperationRequestDispatcher::_enqueueExceptionResponse(
   CIMOperationRequestMessage* request,
   TraceableCIMException& exception)
{
    CIMResponseMessage* response = request->buildResponse();
    response->cimException = exception;
    _enqueueResponse(request, response);
}

void CIMOperationRequestDispatcher::_enqueueExceptionResponse(
    CIMOperationRequestMessage* request,
    CIMStatusCode code,
    const String& ExtraInfo)
{
    CIMException exception = PEGASUS_CIM_EXCEPTION(code, ExtraInfo);
    _enqueueExceptionResponse(request, exception);
}

/*
   Enqueue the response provided with the call to destination defined
   by request.
   Logs this operation, assures resquest and response
   attributes are syncd, gets queue from request,
   gets queue name from request,
   if internal client (queuename) does
       base::_enqueueResponse(request,response)
   else
      calls queue->enqueue(response)
*/
void CIMOperationRequestDispatcher::_enqueueResponse(
   CIMOperationRequestMessage* request,
   CIMResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_enqueueResponse");

    response->dest = request->queueIds.top();

    // Ensure the response message attributes are synchonized with the request
    PEGASUS_ASSERT(request->getMask() == response->getMask());
    PEGASUS_ASSERT(request->getHttpMethod() == response->getHttpMethod());
    PEGASUS_ASSERT(request->getCloseConnect() == response->getCloseConnect());

    PEG_TRACE((TRC_HTTP, Tracer::LEVEL4,
        "_CIMOperationRequestDispatcher::_enqueueResponse - "
            "request->getCloseConnect() returned %d",
        request->getCloseConnect()));

    _logOperation(request, response);

    // Internal client does not have async capabilities, call enqueue()
    // for handling legacy messages directly.
    MessageQueue* queue = MessageQueue::lookup(request->queueIds.top());
    PEGASUS_ASSERT(queue != 0);

    queue->enqueue(response);

    PEG_METHOD_EXIT();
}

/*
    handleEnqueue is the Request input processor to the Dispatcher.
    It processes all incoming request messages and distributes them to
    the appropriate request handler functions based on the operation type.
    This function includes
    a common exception try/catch to account for exceptions in any of the
    individual operation request handlers. The function deletes the
    incoming request upon completion of the handler execution.
*/
void CIMOperationRequestDispatcher::handleEnqueue(Message* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleEnqueue(Message* request)");

    PEGASUS_ASSERT(request != 0);
    PEGASUS_DEBUG_ASSERT(request->valid());

    PEG_TRACE(( TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleEnqueue - MsgType: %u",
        request->getType() ));

    CIMOperationRequestMessage* opRequest =
        dynamic_cast<CIMOperationRequestMessage*>(request);

    if (!opRequest)
    {
        PEG_TRACE(( TRC_DISCARDED_DATA, Tracer::LEVEL2,
            "Ignored unexpected message of type %u in "
                "CIMOperationRequestDispatcher::handleEnqueue",
            request->getType() ));
        delete request;
        PEG_METHOD_EXIT();
        return;
    }

    //
    // This try/catch block ensures that any exception raised during the
    // processing of an operation is handled and translated into an operation
    // response.
    //

    CIMException cimException;

    // Flag to indicate that we should delete request when  handler
    // complete
    bool del = true;
    try
    {
        // Set the client's requested language into this service thread.
        // This will allow functions in this service to return messages
        // in the correct language.
        opRequest->updateThreadLanguages();

        switch (opRequest->getType())
        {
        case CIM_GET_CLASS_REQUEST_MESSAGE:
            handleGetClassRequest((CIMGetClassRequestMessage*)opRequest);
            break;

        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            handleGetInstanceRequest((CIMGetInstanceRequestMessage*)opRequest);
            break;

        case CIM_DELETE_CLASS_REQUEST_MESSAGE:
            handleDeleteClassRequest(
                (CIMDeleteClassRequestMessage*)opRequest);
            break;

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            handleDeleteInstanceRequest(
                (CIMDeleteInstanceRequestMessage*)opRequest);
            break;

        case CIM_CREATE_CLASS_REQUEST_MESSAGE:
            handleCreateClassRequest((CIMCreateClassRequestMessage*)opRequest);
            break;

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            handleCreateInstanceRequest(
                (CIMCreateInstanceRequestMessage*)opRequest);
            break;

        case CIM_MODIFY_CLASS_REQUEST_MESSAGE:
            handleModifyClassRequest((CIMModifyClassRequestMessage*)opRequest);
            break;

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            handleModifyInstanceRequest(
                (CIMModifyInstanceRequestMessage*)opRequest);
            break;

        case CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE:
            handleEnumerateClassesRequest(
                (CIMEnumerateClassesRequestMessage*)opRequest);
            break;

        case CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE:
            handleEnumerateClassNamesRequest(
                (CIMEnumerateClassNamesRequestMessage*)opRequest);
            break;

        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            handleEnumerateInstancesRequest(
                (CIMEnumerateInstancesRequestMessage*)opRequest);
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            handleEnumerateInstanceNamesRequest(
                (CIMEnumerateInstanceNamesRequestMessage*)opRequest);
            break;

        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
            handleExecQueryRequest(
                (CIMExecQueryRequestMessage*)opRequest);
            break;

        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
            handleAssociatorsRequest((CIMAssociatorsRequestMessage*)opRequest);
            break;

        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
            handleAssociatorNamesRequest(
                (CIMAssociatorNamesRequestMessage*)opRequest);
            break;

        case CIM_REFERENCES_REQUEST_MESSAGE:
            handleReferencesRequest((CIMReferencesRequestMessage*)opRequest);
            break;

        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
            handleReferenceNamesRequest(
                (CIMReferenceNamesRequestMessage*)opRequest);
            break;

        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
            handleGetPropertyRequest(
                (CIMGetPropertyRequestMessage*)opRequest);
            break;

        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
            handleSetPropertyRequest(
                (CIMSetPropertyRequestMessage*)opRequest);
            break;

        case CIM_GET_QUALIFIER_REQUEST_MESSAGE:
            handleGetQualifierRequest(
                (CIMGetQualifierRequestMessage*)opRequest);
            break;

        case CIM_SET_QUALIFIER_REQUEST_MESSAGE:
            handleSetQualifierRequest(
                (CIMSetQualifierRequestMessage*)opRequest);
            break;

        case CIM_DELETE_QUALIFIER_REQUEST_MESSAGE:
            handleDeleteQualifierRequest(
                (CIMDeleteQualifierRequestMessage*)opRequest);
            break;

        case CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE:
            handleEnumerateQualifiersRequest(
                (CIMEnumerateQualifiersRequestMessage*)opRequest);
            break;

        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            handleInvokeMethodRequest(
                (CIMInvokeMethodRequestMessage*)opRequest);
            break;

// KS_PULL_BEGIN
        case CIM_OPEN_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            del = handleOpenEnumerateInstancesRequest(
                (CIMOpenEnumerateInstancesRequestMessage*)opRequest);
            break;

        case CIM_OPEN_ENUMERATE_INSTANCE_PATHS_REQUEST_MESSAGE:
            del = handleOpenEnumerateInstancePathsRequest(
                (CIMOpenEnumerateInstancePathsRequestMessage*)opRequest);
            break;

        case CIM_OPEN_REFERENCE_INSTANCES_REQUEST_MESSAGE:
            del = handleOpenReferenceInstancesRequest(
                (CIMOpenReferenceInstancesRequestMessage*)opRequest);
            break;

        case CIM_OPEN_REFERENCE_INSTANCE_PATHS_REQUEST_MESSAGE:
            del = handleOpenReferenceInstancePathsRequest(
                (CIMOpenReferenceInstancePathsRequestMessage*)opRequest);
            break;

        case CIM_OPEN_ASSOCIATOR_INSTANCES_REQUEST_MESSAGE:
            del = handleOpenAssociatorInstancesRequest(
                (CIMOpenAssociatorInstancesRequestMessage*)opRequest);
            break;

        case CIM_OPEN_ASSOCIATOR_INSTANCE_PATHS_REQUEST_MESSAGE:
            del = handleOpenAssociatorInstancePathsRequest(
                (CIMOpenAssociatorInstancePathsRequestMessage*)opRequest);
            break;
        case CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE:
            del = handlePullInstancesWithPath(
                (CIMPullInstancesWithPathRequestMessage*) opRequest);
            break;

        case CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE:
            del = handlePullInstancePaths(
                (CIMPullInstancePathsRequestMessage*) opRequest);
            break;

        case CIM_PULL_INSTANCES_REQUEST_MESSAGE:
            del = handlePullInstances(
                (CIMPullInstancesRequestMessage*) opRequest);
            break;

        case CIM_CLOSE_ENUMERATION_REQUEST_MESSAGE:
            handleCloseEnumeration(
                (CIMCloseEnumerationRequestMessage*) opRequest);
            break;

        case CIM_ENUMERATION_COUNT_REQUEST_MESSAGE:
            handleEnumerationCount(
                (CIMEnumerationCountRequestMessage*) opRequest);
            break;

        case CIM_OPEN_QUERY_INSTANCES_REQUEST_MESSAGE:
            del = handleOpenQueryInstancesRequest(
                (CIMOpenQueryInstancesRequestMessage*)opRequest);
            break;
//KS_PULL_END

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
        }
    }
    catch (const CIMException& exception)
    {
        cimException = exception;
    }
    catch (const Exception& exception)
    {
        cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch (...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, String::EMPTY);
    }

    if (cimException.getCode() != CIM_ERR_SUCCESS)
    {
        AutoPtr<CIMResponseMessage> response(opRequest->buildResponse());
        response->cimException = cimException;
        _enqueueResponse(opRequest, response.release());
    }
    if (del)
    {
        delete request;
    }
    PEG_METHOD_EXIT();
}

/*
    CIMOperationDispatcher dequeue function to dequeue the
    next input operation request and send to the handle functions.
*/
void CIMOperationRequestDispatcher::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleEnqueue");

    Message* request = dequeue();

    PEGASUS_DEBUG_ASSERT(request->valid());

    if (request)
    {
        handleEnqueue(request);
    }

    PEG_METHOD_EXIT();
}

/****************************************************************************
**
**     Request Input Parameter Test and reject functions
**     Each function tests a particular possible reason for reject.
**     If the test fails, the reject message is generated and
**     a true response returned.  If the test passes, true returned.
**     Generally named with the prefix reject to indicate that they
**     reject and return true.
**     Each function should terminate the operation processing if true
**     is returned.
**
****************************************************************************/

// Test to determine if Association traversal is enabled.
// returns true if Not Enabled, false if enabled
Boolean CIMOperationRequestDispatcher::_rejectAssociationTraversalDisabled(
    CIMOperationRequestMessage* request,
    const String& opName)
{
    if (_enableAssociationTraversal)
    {
        // return when AssociationTraversal is enabled
        return false;
    }
    else
    {
        //// FUTURE add explicit international message. Low priority because
        //// this almost never happens. The name must be illegal, not just
        //// missing from target.
        _enqueueExceptionResponse(request,CIM_ERR_NOT_SUPPORTED, opName);
        return true;
    }
}

/* Test the roleParameter to determine if it exists and is a valid CIMName.
   Generate error if it exists and is not a valid CIMName.
   @return true if invalid and false if valid
*/
Boolean CIMOperationRequestDispatcher::_rejectInvalidRoleParameter(
    CIMOperationRequestMessage* request,
    const String& roleParameter,
    const String& parameterName)
{
    if (roleParameter.size() != 0 && (!CIMName::legal(roleParameter)))
    {
        //KS_TODO internationalize This
        String text = parameterName + " " + roleParameter;
        _enqueueExceptionResponse(request,
            CIM_ERR_INVALID_PARAMETER, text);

        return true;
    }
    return false;
}

// _rejectEnumerateTooBroad.  Checks providerCount against parameter. Generates
// exception of providerCount to large.
// This limits the number of provider invocations, not the number
// of instances returned. It throws exception because request not available
// in function where called.
void CIMOperationRequestDispatcher::_rejectEnumerateTooBroad(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Uint32 providerCount)
{
    if (providerCount > _maximumEnumerateBreadth)
    {
        PEG_TRACE((TRC_DISPATCHER,  Tracer::LEVEL1,
            "Operation too broad for class %s.  "
                "  Namespace: %s  Limit = %u, providerCount = %u",
            CSTRING(className.getString()),
            CSTRING(nameSpace.getString()),
            _maximumEnumerateBreadth,
            providerCount));

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_NOT_SUPPORTED,
            MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher.ENUM_REQ_TOO_BROAD",
                "Enumerate request too Broad"));
    }
}

Boolean CIMOperationRequestDispatcher::_CIMExceptionIfNoProvidersOrRepository(
    CIMOperationRequestMessage* request,
    const ProviderInfoList& providerInfos,
    CIMException& cimException)
{
    if ((providerInfos.providerCount == 0) &&
        !(_repository->isDefaultInstanceProvider()))
    {
        cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_NOT_SUPPORTED, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "REQUEST_CLASS_NOT_SUPPORTED",
                "No provider or repository defined for class $0.",
                request->className.getString()));
        return true;
    }
    else   // We have either providers or a repository
    {
        return false;
    }

}

Boolean CIMOperationRequestDispatcher::_rejectNoProvidersOrRepository(
    CIMOperationRequestMessage* request,
    const ProviderInfoList& providerInfos)
{
    if ((providerInfos.providerCount == 0) &&
        !(_repository->isDefaultInstanceProvider()))
    {
        CIMException cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_NOT_SUPPORTED, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "REQUEST_CLASS_NOT_SUPPORTED",
                "No provider or repository defined for class $0.",
                request->className.getString()));

        _enqueueExceptionResponse(request, cimException);
        return true;
    }
    else   // We have either providers or a repository
    {
        return false;
    }
}

// FUTURE(KS) - We have two apparently parallel class parameter checks
// One generates response, etc (_checkClassParameter) the other
// returns and has the user generate the error response.  Mostly these
// are generated with a throw.  Should reduce this to one function that
// is clean and uses minimum space. Also they generate different set
// of traces and we need common base for traces.
/*
    Test the validity of the class name parameter and get the corresponding
    class. Returns the class if found.  If not found, returns with return
    code false.
    @param request. NOTE: This depends on the namespace and class in
        the CIMOperationRequestMessage request.
    @param cimClass CIMConstClass containing the requested class if the
    class exists.
    @return false if class found or true if class not found
*/
Boolean CIMOperationRequestDispatcher::_rejectInvalidClassParameter(
    CIMOperationRequestMessage* request,
    CIMConstClass& targetClass)
{
    CIMException checkClassException;

    targetClass = _getClass(
        request->nameSpace,
        request->className,
        checkClassException);

    if (checkClassException.getCode() != CIM_ERR_SUCCESS)
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "CIMOperationRequestDispatcher - "
                "CIM class exist exception has occurred.  Namespace: %s  "
                "Class Name: %s  Exception message: \"%s\"",
            CSTRING(request->nameSpace.getString()),
            CSTRING(request->className.getString()),
            CSTRING(checkClassException.getMessage())));

        _enqueueExceptionResponse(request,checkClassException);
        return true;
    }
    return false;
}

/*
    Test the validity of the class name parameter and get the corresponding
    class. If not found, generates exception INVALID_PARAMETER response
    and returns true.
    @param request
    @param objectName which contains target className
    @param namespace CIMNamespaceName for this operation
    @return false if class found or true if class not found
*/

Boolean CIMOperationRequestDispatcher::_rejectInvalidClassParameter(
    CIMOperationRequestMessage* request,
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName)
{
    if (!_checkExistenceOfClass(nameSpace, objectName.getClassName()))
    {
        _enqueueExceptionResponse(request,
            CIM_ERR_INVALID_PARAMETER,
            objectName.getClassName().getString());
        return true;
    }
    return false;
}

/*  Build a property list of the properties in the class provided as an
    argument.
    @param class CIMClass with the properties to be put in list
    @return propertyList containing the properties in the class.
*/
void _buildPropertyListFromClass(CIMConstClass& thisClass,
                                 CIMPropertyList& propertyList)
{
    Array<String> pla;
    Uint32 numProperties = thisClass.getPropertyCount();
    for (Uint32 i = 0; i < numProperties; i++)
    {
        pla.append(thisClass.getProperty(i).getName().getString());
    }
    propertyList.append(pla);
}

//EXP_PULL_BEGIN

// Test of the common input parameters for CIMOpen... messages.
// tests filter, filterLanguage, continueOnError, MaxObjectCount
// and Operation timeout
bool CIMOperationRequestDispatcher::_rejectIfPullParametersFailTests(
    CIMOpenOperationRequestMessage* request,
    Uint32& operationMaxObjectCount,
    Boolean allowQueryFilter)
{
    if (_rejectInvalidFilterParameters(request,
          request->filterQueryLanguage,
          request->filterQuery,
          allowQueryFilter))
    {
        return true;
    }

    if (_rejectIfContinueOnError(request, request->continueOnError))
    {
        return true;
    }

    if (_rejectInvalidMaxObjectCountParam(request, request->maxObjectCount,
            false, operationMaxObjectCount, Uint32(0)))
    {
        return true;
    }

    // Test for valid values in OperationTimeout
    if (_rejectInvalidOperationTimeout(request, request->operationTimeout))
    {
        return true ;
    }
    return false;
}
/*
    Test if this is a valid Pull message corresponding to the type
    of the open. If not valid, put out error message and return false
    @param request
    @param valid Boolean = true if it is valid
    Return true if !valid, false if valid
*/
bool CIMOperationRequestDispatcher::_rejectInvalidPullRequest(
    CIMOperationRequestMessage* request, Boolean valid)
{
    if (!valid)
    {
        CIMResponseMessage* response = request->buildResponse();

        response->cimException = PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED, MessageLoaderParms(
                    "Server.CIMOperationRequestDispatcher."
                    "PULL_OPERATION_TYPE_ERR",
                "Open and Pull Message types do not match."));

        _enqueueResponse(request, response);
    }
    return !valid;
}

/* Test to assure that the filter query language amd filter query are as
   required.  At this point, Pegasus refuses any data in these parameters in
   accord with DSP0200 version 1.3.  This will change when the spec and
   Pegasus support these parameters.
*/
bool CIMOperationRequestDispatcher::_rejectInvalidFilterParameters(
    CIMOperationRequestMessage* request,
    const String& filterQueryLanguageParam,
    const String& filterQueryParam,
    Boolean allowQueryFilter)
{
    CIMResponseMessage* response = NULL;
    if (filterQueryLanguageParam.size() != 0 &&
            filterQueryParam.size() == 0)
    {
        response = request->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                "MISSING_FILTER_QUERY",
                "filterQueryLanguage parameter without filterQuery parameter"));
    }
    else if (filterQueryLanguageParam.size() == 0 &&
            filterQueryParam.size() != 0)
    {
        response = request->buildResponse();
        response->cimException = PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                "MISSING__FILTER_QUERY_LANGUAGE",
                "filterQuery parameter without filterQueryLanguage parameter"));
    }
    else if(filterQueryLanguageParam.size() != 0 ||
            filterQueryParam.size() != 0)
    {
        if (!allowQueryFilter)
        {
            //// KS_TODO internationalize
            response = request->buildResponse();
            response->cimException =
                PEGASUS_CIM_EXCEPTION(
                    CIM_ERR_FILTERED_ENUMERATION_NOT_SUPPORTED,
                    "Operation does not support FilterQuery parameters");
        }
    }

    if (response != NULL)
    {
        _enqueueResponse(request, response);
        return true;
    }
    return false;
}

/* Test to assure that the CIMObjectPath is, in fact, an object path
   and not just a classname.  Return CIM_ERR_INVALID_PARAMETER if not
   valid.  The ObjecPath must be a full instance name to assure that we
   do not try to follow the path for class based operations.
   */
bool CIMOperationRequestDispatcher::_rejectInvalidObjectPathParameter(
    CIMOperationRequestMessage* request,
    const CIMObjectPath& path)
{
    if (path.getKeyBindings().size() == 0)
    {
        CIMResponseMessage* response = request->buildResponse();

        response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_INVALID_PARAMETER, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "INVALID_MODEL_PATH",
                "Full Model Path with keys required."));

        _enqueueResponse(request, response);
        return true;
    }
    return false;
}

//
// Pegasus does not allow continueOnError parameter. Return Error
// if it exists.
//
bool CIMOperationRequestDispatcher::_rejectIfContinueOnError(
    CIMOperationRequestMessage* request,
    Boolean continueOnError)
{
    if (continueOnError)
    {
        CIMResponseMessage* response = request->buildResponse();

        response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_NOT_SUPPORTED, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "CONTINUE_ON_ERROR_NOT_SUPPORTED",
                "ContinueOnError = true argument not supported."));

        _enqueueResponse(request, response);
        return true;
    }
    return false;
}

// test and set maxObjectCount for this operation.
// If parameter is required, generate exception if parameter is NULL or
// test for within system max limit if exists.
// MaxObjectCount is optional for all open operations. If not
// supplied, set value = 0. Pegasus allows a systemwide
// maximum to be set.  Since this is only used within
// the operation handler the result is a local function.
// Return an Exception only if the value is outside max limit.
//
// If parameter is optional, set to defaultValue if does not exist or
// test against system max limit if exists.
// @param request The operation request packet.
// @param maxObjectCount - the input parameter from the request packet
// @param requiredParameter Boolean true if parameter required
//     for this operation. This is because it is optional on opens but
//     required on pull operations
// @param value Value to be used if operation returns true;
// @param defaultValue Uint32 value to be used if requiredParameter = false
// and maxObjectCountParam is NULL
// @return - Returns true if parameter OK for this operation or false if
// if fails test.  If true, the value to be used for maxObjectCount for
// this operation is placed in rtnValue. If false return, rtnValue is
// not changed.
// KS_TODO - We duplicate default value and _systemMaxPullOperationObjectCount
bool CIMOperationRequestDispatcher::_rejectInvalidMaxObjectCountParam(
    CIMOperationRequestMessage* request,
    Uint32 maxObjectCountParam,
    bool requiredParameter,
    Uint32& rtnValue,
    const Uint32 defaultValue)
{
    if (maxObjectCountParam > _systemPullOperationMaxObjectCount)
    {
        CIMException x = PEGASUS_CIM_EXCEPTION_L(
        CIM_ERR_INVALID_PARAMETER, MessageLoaderParms(
            "Server.CIMOperationRequestDispatcher."
                "MAXOBJECTCOUNT_OUT_OF_RANGE",
            "Operation maximum object count argument $0 too large."
            " Maximum allowed: $1.",
            maxObjectCountParam,
            _systemPullOperationMaxObjectCount));

        _enqueueExceptionResponse(request, x);
        return true;
    }
    else
    {
            rtnValue = maxObjectCountParam;
    }
    return false;
}

// Test validity of operation timeout parameter.
// If the input value is NULL, we use our own value.
// If the value is gt system parameter we reject.
// If the value is 0 we follow the dictates of a parameter that defines
// this decision. Some systems implementations may allow this value. Others
// may elect to reject 0 (which means no timeout).

bool CIMOperationRequestDispatcher::_rejectInvalidOperationTimeout(
    CIMOperationRequestMessage* request,
    const Uint32Arg& operationTimeout)
{
    // NULL is allowed.  The EnumerationContext creation will determine
    // the limit during creation based on system default. See
    // EnumerationContextTable class
    if (operationTimeout.isNull())
    {
        return false;
    }

    // If system does not allow zero value, send error.
    if (operationTimeout.getValue() == 0)
    {
        if (_rejectZeroOperationTimeoutValue)
        {
            CIMResponseMessage* response = request->buildResponse();

            response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_INVALID_OPERATION_TIMEOUT, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "TIMEOUT_ZERO_NOT_ALLOWED",
                "Operation timeout value of 0 not allowed."));

            _enqueueResponse(request, response);
            return true;
        }
        else
        {
            return false;
        }
    }

    // If the value is greater than the system allowed max, send error
    // response
    if (operationTimeout.getValue() > _pullOperationMaxTimeout)
    {
       CIMResponseMessage* response = request->buildResponse();

        response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_INVALID_OPERATION_TIMEOUT, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "ENUMERATION_TIMEOUT_TO_LARGE",
                "Operation timeout too large. Maximum allowed: $0.",
                _pullOperationMaxTimeout));

        _enqueueResponse(request, response);
        return true;
    }
    return false;
}
/* Generate error response message if context is invalid.
   @param valid Boolean = true if valid
   @return true if valid=true, false if valid=false.
*/
bool CIMOperationRequestDispatcher::_rejectInvalidEnumerationContext(
    CIMOperationRequestMessage* request,
    EnumerationContext* en)
{
    if (en == 0)
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "%s Invalid Context parameter Received",
             MessageTypeToString(request->getType()) ));

        CIMResponseMessage* response = request->buildResponse();

        response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_INVALID_ENUMERATION_CONTEXT, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "ENUMERATION_CONTEXT_UNDEFINED",
                "Context undefined."));

        _enqueueResponse(request, response);

        return true;
    }
    if (en->isClientClosed())
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "%s Enumeration context already closed",
             MessageTypeToString(request->getType()) ));

        CIMResponseMessage* response = request->buildResponse();

        response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_INVALID_ENUMERATION_CONTEXT, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "ENUMERATION_CONTEXT_CLOSED",
                "Enumeration context closed when request received."));

        _enqueueResponse(request, response);

        return true;
    }
    return false;
}

/* test if the parameter isTimedOut is true, If true an exception
   return is queued amd true is returned
   @param request - pointer to request
   @isTimedOut - Boolean = true if timeout has occurred
   @return true if timed out.
*/

bool CIMOperationRequestDispatcher::_rejectIfContextTimedOut(
    CIMOperationRequestMessage* request,
    Boolean isTimedOut)
{
    if (isTimedOut)
    {
        CIMResponseMessage* response = request->buildResponse();

        response->cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_INVALID_ENUMERATION_CONTEXT, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "ENUMERATION_CONTEXT_TIMED_OUT",
                "Enumeration Context timed out before request received."));

        _enqueueResponse(request, response);

        return true;
    }
    return false;
}

bool CIMOperationRequestDispatcher::_rejectIfEnumerationContextProcessing(
    CIMOperationRequestMessage* request, Boolean processing)
{
    if (processing)
    {
        CIMResponseMessage* response = request->buildResponse();
//// KS_TODO This may be wrong error message.
//// KS_TODO Internationalize
        response->cimException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_PULL_CANNOT_BE_ABANDONED,
                 "Server processing another request for this"
                 " Enumeration Context.");

        _enqueueResponse(request, response);
    }
    return processing;
}

void CIMOperationRequestDispatcher::_rejectCreateContextFailed(
    CIMOperationRequestMessage* request)
{
    CIMResponseMessage* response = request->buildResponse();

    response->cimException = PEGASUS_CIM_EXCEPTION_L(
        CIM_ERR_SERVER_LIMITS_EXCEEDED, MessageLoaderParms(
            "Server.CIMOperationRequestDispatcher."
                "ENUMERATION_CONTEXT_EXCEEDED_LIMIT",
            "Exceeded maximum number of simultaneous open Enumerations."));

    _enqueueResponse(request, response);
}
// EXP_PULL_END

/*****************************************************************************
**
**      Request Processing Helper Methods
**
*****************************************************************************/
/*
    Helper struct/methods to issue operations requests to groups of Providers
    defined by a ProviderInfoList. This struct issues requests of the type
    defined by input to the providers defined in the providerInfoList.

    There are separate functions for issuing:
        issueEnumRequests - enumerate operations (enumerate and enumerateNames)
        issueAssocRequests - association operations (includes references
            and associations and their corresponding name operations).

    This struct eliminates the repeated code for issuing requests in the
    handle***Request functions for those input operation requests that issue
    provider operation requests to multiple providers based on a
    ProviderInfoList. It reduces the previously repeated code for issuing
    requests to providers in the operation request processors for these
    functions to a single line.
*/

/*  This struct not part of CIMOperationRequestDispatcher class because it
    includes template methods.
*/
struct ProviderRequests
{
    // Set specific fields as part of the request type.  These are required
    // because we set different fields in the request message for each
    // operation type with the class for the defined provider.
    // References use the result class and Association requests, the
    // assocClass field.
    static void setSelectedRequestFields(
        CIMAssociatorsRequestMessage* request,
        const ProviderInfo& providerInfo)
    {
        request->assocClass = providerInfo.className;
    }

    static void setSelectedRequestFields(
        CIMAssociatorNamesRequestMessage* request,
        const ProviderInfo& providerInfo)
    {
        request->assocClass = providerInfo.className;
    }

    static void setSelectedRequestFields(
        CIMReferencesRequestMessage* request,
        const ProviderInfo& providerInfo)
    {
        request->resultClass = providerInfo.className;
    }

    static void setSelectedRequestFields(
        CIMReferenceNamesRequestMessage* request,
        const ProviderInfo& providerInfo)
    {
        request->resultClass = providerInfo.className;
    }

    // Set the appropriate data into the CIMResponseData container of the
    // response message.  This is used with the open handlers
    // to set the response data into the repository response messages
    // because some messages have paths and others objects.

    static void setCIMResponseData(
        CIMAssociatorsResponseMessage* response,
        Array<CIMObject>& repositoryData)
    {
        response->getResponseData().setObjects(repositoryData);
    }

    static void setCIMResponseData(
        CIMAssociatorNamesResponseMessage* response,
        Array<CIMObjectPath>& repositoryData)
    {
        response->getResponseData().setInstanceNames(repositoryData);;
    }

    static void setCIMResponseData(
        CIMReferencesResponseMessage* response,
        Array<CIMObject>& repositoryData)
    {
        response->getResponseData().setObjects(repositoryData);;
    }

    static void setCIMResponseData(
        CIMReferenceNamesResponseMessage* response,
        Array<CIMObjectPath>& repositoryData)
    {
        response->getResponseData().setInstanceNames(repositoryData);;
    }

    /**************************************************************************
    **
    ** issueAssocRequestsToProviders - Template method to issue requests for
    **     association/refernece operations.
    **
    **************************************************************************/
    /*  Template method to issue requests for association/reference
        operations. The function issues requests of the type defined for
        the template to the providers in the ProviderInfoList. It also
        execute common code for this set of request types.
        NOTE: The response may already have data in it from a repository
        request.
    */
    template<class REQ, class RSP>
    static void issueAssocRequestsToProviders(
        CIMOperationRequestDispatcher* dispatcher,
        REQ* request,
        AutoPtr<RSP>& response,
        ProviderInfoList& providerInfos,
        const char * reqMsgName)
    {
        // No providers and nothing from repository. Return empty
        if ((providerInfos.providerCount == 0) && (response.get() == 0))
        {
            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
                "%s Request, Returns nothing for %s", reqMsgName,
                CSTRING(request->className.getString()) ));

            response.reset(dynamic_cast<RSP*>(request->buildResponse()));

            dispatcher->_enqueueResponse(request, response.release());
        }
        else
        {
            OperationAggregate *poA = new OperationAggregate(
                new REQ(*request),
                request->objectName.getClassName(),
                request->nameSpace,
                providerInfos.providerCount,
                true);   // requiresHostNameCompletion = true

            // Include the repository response in the aggregation, if it
            // exists
            if (response.get() != 0)
            {
                poA->incTotalIssued();
                dispatcher->_forwardResponseForAggregation(
                    new REQ(*request),
                    poA,
                    response.release());
            }

            while (providerInfos.hasMore(true))
            {
                ProviderInfo& providerInfo = providerInfos.getNext();

                // Make copy of request
                REQ* requestCopy = new REQ(*request);

                // Insert the association class name to limit the provider
                // to this class. This is a template function that sets the
                // class fields as required for each msg type.
                setSelectedRequestFields(requestCopy, providerInfo);

                if (providerInfo.providerIdContainer.get() != 0)
                {
                    requestCopy->operationContext.insert(
                        *(providerInfo.providerIdContainer.get()));
                }

                PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                    "%s Forwarding to provider for class %s, messageId %s",
                   reqMsgName,
                   CSTRING(providerInfo.className.getString()),
                   CSTRING(request->messageId)        ));

                dispatcher->_forwardAggregatingRequestToProvider(
                    providerInfo,
                    requestCopy, poA);
                // Note: poA must not be referenced after last "forwardRequest
            }
        }
    } // end issueAssocRequest

    /**************************************************************************
    **
    ** issueOpenAssocRequestMsgsToProviders - Template method to issue requests
    **     for Associator and Reference operations\ to each provider
    **
    **************************************************************************/
    /*  Template method issues the provider requests, repository
        responses and provider requests for:
        OpenAssociators
        OpenAssociatorNames
        OpenReferences
        OpenReferenceNames
        It differs from the issueAssocRequests in that this builds the
        enumerationContext for the operations.
    */
    template<class IREQ>
    static void issueOpenAssocRequestMsgsToProviders(
       CIMOperationRequestDispatcher* dispatcher,
       IREQ* internalRequest,
       ProviderInfoList& providerInfos,
       OperationAggregate* poA,
       const char * reqMsgName)
    {
        // Issue requests to all providers defined.
        while (providerInfos.hasMore(true))
        {
            ProviderInfo& providerInfo = providerInfos.getNext();

            IREQ* requestCopy = new IREQ(*internalRequest);
            // Insert the association class name to limit the provider
            // to this class.
            setSelectedRequestFields(requestCopy, providerInfo);

            if (providerInfo.providerIdContainer.get() != 0)
            {
                requestCopy->operationContext.insert(
                    *(providerInfo.providerIdContainer.get()));
            }

            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                "%s Forwarding to provider for class %s, messageId %s",
                reqMsgName,
                CSTRING(providerInfo.className.getString()),
                CSTRING(internalRequest->messageId) ));

            dispatcher->_forwardAggregatingRequestToProvider(
               providerInfo, requestCopy, poA);
            // Note: poA must not be referenced after last "forwardRequest"
        }
    }    // end of issueOpenAssocRequestMessagesToProviders

    /**************************************************************************
    **
    ** IssueEnumerationRequests - Issue enumeration requests to providers
    **     for enumerateInstances and enumerateInstanceNames operations.
    **
    **************************************************************************/
    /*  Common Provider issue code for all
        enumerate functions (enumerateInstance, EnumerateInstanceNames
        openEnumerateInstancesWithPath openEnumerateInstancePaths).
        Issues a request to all providers in the ProviderInfoList provided.
        NOTE: Whereas today the assocRequests function starts with responses
        this one starts after repository responses processed.
    */
    template<class REQ>
    static void issueEnumerationRequestsToProviders(
        CIMOperationRequestDispatcher* dispatcher,
        REQ* request,
        ProviderInfoList providerInfos,
        OperationAggregate* poA,
        const char * reqMsgName)
    {
        // Loop through providerInfos, forwarding requests to providers
        while (providerInfos.hasMore(true))
        {
            ProviderInfo& providerInfo = providerInfos.getNext();

            // issue peg trace of routing info if required
            providerInfos.pegRoutingTrace(providerInfo, reqMsgName,
                request->messageId);

            // set this className into the new request
            REQ* requestCopy = new REQ(*request);

            // set this className into the new request
            requestCopy->className = providerInfo.className;

            // Forward to provider. If fails return empty response
            // FUTURE: Confirm that this short bypass is needed and works.
            // The _forwardEnumerationToProvider handles any class
            // checking and special tasks for NORMALIZATION
            if (!dispatcher->_forwardEnumerationToProvider(
               providerInfo, poA, requestCopy))
            {
                CIMResponseMessage* response = requestCopy->buildResponse();
                dispatcher->_forwardResponseForAggregation(
                   requestCopy,
                   poA,
                   response);
            }
        }
    }
};    // End Of ProviderRequests Struct

// EXP_PULL_BEGIN
#ifdef PEGASUS_ENABLE_FQL
//// Temporarily removed because we are losing the pointer to qx->_stmt when we
//// return from this function. Just set the code inline. KS_TODO
//FQLQueryExpressionRep* CIMOperationRequestDispatcher::handleFQLQueryRequest(
//    CIMOpenOperationRequestMessage* request)
//{
//    PEG_METHOD_ENTER(TRC_DISPATCHER,
//        "CIMOperationRequestDispatcher::parseFQLFilter");
//    bool exception = false;
//
//    AutoPtr<FQLQueryStatement> queryStatement(new FQLQueryStatement());
//
//    AutoPtr<FQLQueryExpressionRep> qx;
//
//        CIMException cimException;
//    if (request->filterQuery.size() != 0)
//    {
//        //// KS_TODO simplify by using FQLQueryExpressionRep to do this
//        if (request->filterQueryLanguage != "DMTF:FQL")
//        {
//            cimException = PEGASUS_CIM_EXCEPTION(
//                CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED,
//                request->filterQueryLanguage);
//            exception = true;
//        }
//        else
//        {
//            try
//            {
//                FQLParser::parse(request->filterQuery, *queryStatement.get());
//
//                qx.reset(new FQLQueryExpressionRep(
//                    request->filterQueryLanguage, queryStatement.get()));
//
//                queryStatement.release();
//                qx.release();
//            }
//            catch (ParseError& e)
//            {
//                String text = request->filterQuery + " " + e.getMessage();
//
//                cimException = PEGASUS_CIM_EXCEPTION(
//                        CIM_ERR_INVALID_QUERY, text);
//                exception=true;
//            }
//            catch (...)
//            {
//                cimException = PEGASUS_CIM_EXCEPTION(
//                        CIM_ERR_INVALID_QUERY, request->filterQuery);
//                exception=true;
//            }
//        }
//        if (exception)
//        {
//            CIMResponseMessage* response = request->buildResponse();
//            response->cimException = cimException;
//
//            XCOUT << "Gen Response " << endl;
//            _enqueueResponse(request, response);
//            PEG_METHOD_EXIT();
//            return NULL;
//        }
//    }
//
//    FQLQueryStatement* qsa = qx.get()->_stmt;
//    qsa->print();   //// DELETE
//    PEG_METHOD_EXIT();
//    return qx.get();
//}
#endif
/**************************************************************************
**
** processPullRequest - Handles pullInstancesWithPath,pullInstancePaths
**                      and pullInstances with a single function
**
**************************************************************************/

bool CIMOperationRequestDispatcher::processPullRequest(
    CIMPullOperationRequestMessage* request,
    CIMOpenOrPullResponseDataMessage*  pullResponse,
    const char* requestName)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::processPullRequest");

    AutoPtr<CIMOpenOrPullResponseDataMessage> responseDestroyer(pullResponse);

    PEG_TRACE(( TRC_DISPATCHER, Tracer::LEVEL4,
        "%s request for "
            "namespace: %s"
            "maxObjectCount: \"%u\" .  "
            "enumerationContext: \"%s\" .  ",
        requestName,
        CSTRING(request->nameSpace.getString()),
        request->maxObjectCount,
        CSTRING(request->enumerationContext) ));

    // Find the enumerationContext object from the request parameter
    EnumerationContext* en = _enumerationContextTable->find(
        request->enumerationContext);

    // If enumeration Context,value is zero, or is already in closed status
    // return Invalid Context exception with explanation.
    if (_rejectInvalidEnumerationContext(request, en))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (request->nameSpace != en->getNamespace())
    {
        CIMResponseMessage* response = request->buildResponse();

        response->cimException = PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms("Server.CIMOperationRequestDispatcher."
                "PULL_NAMESPACE_ERR",
            "Incorrect namespace (%s) in pull operation. Expected: %s.",
            CSTRING(request->nameSpace.getString()),
            CSTRING(en->getNamespace().getString()) ));

        _enqueueResponse(request, response);
        PEG_METHOD_EXIT();
        return true;
    }

    en->incrementRequestCount();

    // lock the context until we have set processing state to avoid
    // conflict with timer thread. Do not need to lock earlier
    // because not closed (if closed, it would be invalid).
    {
        AutoMutex contextLock(en->_contextLock);
        // reject  and set closed if this is a not valid request for the
        // originating  operation
        if (_rejectInvalidPullRequest(request,
            en->isValidPullRequestType(request->getType())))
        {
            en->setClientClosed();
            PEG_METHOD_EXIT();
            return true;
        }
        // reject if an operation is already active on this enumeration
        // context
        if (_rejectIfEnumerationContextProcessing(request,
            en->isProcessing()))
        {
            PEG_METHOD_EXIT();
            return true;
        }

        // reject and set client closed if context timed out
        if (_rejectIfContextTimedOut(request,
            en->isTimedOut()))
        {
            en->setClientClosed();
            PEG_METHOD_EXIT();
            return true;
        }

        // Set active state and stop state timer.
        en->setProcessingState(true);
        // this also removes lock
    }

    // Test limit of the maxObjectCount consecutive zero counter
    // The return is true if the limit is exceeded in which case
    // close client and generate exception.
    if (en->incAndTestPullCounters((request->maxObjectCount == 0)))
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "%s Exceeded maxObjectCount consecutive zero limit. ContextId=%s",
            requestName , (const char*)en->getContextId().getCString()));

        // Force continueOnError to false.
        en->setContinueOnError(false);

        CIMException cimException = PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_SERVER_LIMITS_EXCEEDED, MessageLoaderParms(
                "Server.CIMOperationRequestDispatcher."
                    "ZERO_LEN_PULL_EXCEEDED_LIMIT",
                "Maximum consecutive zero maxObjectCount pull requests"
                " exceeded."));

        en->setErrorState(cimException);
    }
    // KS_TODO determine if this is worthwhile trace
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,  // EXP_PULL_TEMP
        "%s get from cache. ContextId=%s isComplete=%s cacheSize=%u "
            "errorState=%s",
        requestName,
        (const char *)en->getContextId().getCString(),
        boolToString(en->providersComplete()),
        en->responseCacheSize(),
        boolToString(en->isErrorState())  ));

    // Issue the Response to the Request. This may be issued immediatly
    // or delayed by setting information into the enumeration context
    // if there are no responses from providers ready
    // to avoid issuing empty responses. Enumeration Context must not
    // be used after this call

    bool releaseRequest = issueOpenOrPullResponseMessage(
        request,
        responseDestroyer.release(),
        en,
        request->maxObjectCount,
        requireCompleteResponses);

    return releaseRequest;
} // end issuePullResponse

// EXP_PULL_BEGIN
/**************************************************************************
**
** issueOpenOrPullResponseMessage - Issue immediate or delayed response
**
**************************************************************************/
// Issue the response to an open or pull.  This function may issue the
// response immediatly if there are objects to send or may push the task
// off to the aggregrator if there is nothing to send immediatly.
// It is a template because there is one line that is message type
// dependent, the requirement to build a new request object.


bool CIMOperationRequestDispatcher::issueOpenOrPullResponseMessage(
    CIMOperationRequestMessage* openRequest,
    CIMOpenOrPullResponseDataMessage* openResponse,
    EnumerationContext* en,
    Uint32 operationMaxObjectCount,
    Boolean requireCompleteResponses)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_issueOpenOrPullResponseMessage");

    PEGASUS_ASSERT(en->valid());

    PEGASUS_ASSERT(en->_savedRequest == NULL);
    PEGASUS_ASSERT(en->_savedResponse == NULL);

    bool releaseRequest = true;
    en->lockContext();
    // Determine if there are any responses to send. Returns
    // immediatly if operationMaxObjectCount satisfies what is in cache
    // or maxObjectCount == 0
    if (en->testCacheForResponses(operationMaxObjectCount,
        requireCompleteResponses))
    {
        // Issue response immediatly
        _issueImmediateOpenOrPullResponseMessage(
            openRequest,
            openResponse,
            en,
            operationMaxObjectCount);

        if (en->isClientClosed() && en->providersComplete())
        {
            // en may be deleted in this call. Do not use
            // after this call. This call does the unlock.
            _enumerationContextTable->releaseContext(en);
        }
        else
        {
            en->unlockContext();
        }
    }
    else
    {
        PEGASUS_DEBUG_ASSERT(en->isProcessing());
        // Set up to issue upon provider response or timeout. Request passed
        // to the delay variable so mark to not release.
        en->setupDelayedResponse(
            openRequest,
            openResponse,
            operationMaxObjectCount);


        PEG_TRACE((TRC_DISPATCHER,Tracer::LEVEL4,
            "EnumerationContextLock unlock %s",  // KS_TODO DELETE
                   CSTRING(en->getContextId()) ));
        en->unlockContext();
        releaseRequest = false;
    }
    PEG_METHOD_EXIT();
    return releaseRequest;
}

/**************************************************************************
**
** _issueImmediateOpenOrPullResponseMessage - issues the response define
**                      on the current thread
**
**************************************************************************/
// Issue the Open or Pull response immediatly.
// This function:
//    1 Checks for error responses and if found issues the next
//      error response
//    2. If no errors, gets the next data from the cache and issues
//       it.
//   It assumes that the cache has already been tested for size, etc.
//   and that there enough objects in the cache to match the request
//   requirement.
// If it issues the response, it marks the response message for release
// upon completion of the send function

void CIMOperationRequestDispatcher::_issueImmediateOpenOrPullResponseMessage(
    CIMOperationRequestMessage* request,
    CIMOpenOrPullResponseDataMessage* response,
    EnumerationContext* en,
    Uint32 operationMaxObjectCount)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::"
            "_issueImmediateOpenOrPullResponseMessage");

    PEGASUS_DEBUG_ASSERT(en->valid());

    AutoPtr<CIMOpenOrPullResponseDataMessage> responseDestroyer(response);

    // KS TODO diagnostic. Remove before release.
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "%s issueResponseMessage ContextId=%s"
        " maxObjectCount=%u isComplete=%s, cacheSize=%u  errorState=%s",
        MessageTypeToString(request->getType()),
        CSTRING(en->getContextId()),
        operationMaxObjectCount,
        boolToString(en->providersComplete()),
        en->responseCacheSize(),
        boolToString(en->isErrorState()) ));

    CIMResponseData & to = response->getResponseData();

    // Returns false if error flag set.
    // Determine if this response is data or error
    Boolean errorFound = !en->getCache(operationMaxObjectCount, to);

    if (errorFound)
    {
        response->cimException = en->_cimException;
        // KS_TODO remove this diagnostic trace before release
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,  // EXP_PULL_TEMP
            "%s Response Error found. ContextId=%s cimException = %s",
            MessageTypeToString(request->getType()),
            CSTRING(en->getContextId()),
            cimStatusCodeToString(response->cimException.getCode()) ));
    }

    _enumerationContextTable->setRequestSizeStatistics(operationMaxObjectCount);

    // Check after processing the results of the get.
    // This function either closes the operation if providers are complete
    // and the response cache is empty or sets the processing state =
    // false to allow the next operation.
    // If errorFound = true, and !continueOnError, it sets the Client
    // Closed.
    // Context is current with provider response status
    // If return = true, enumerate on sequence complete.
    if (en->setNextEnumerationState(errorFound))
    {
        // FUTURE - Expand this for continueOnError=true (KS)
        response->endOfSequence = true;
    }
    else
    {
        response->enumerationContext = en->getContextId();
    }
    _enqueueResponse(request, responseDestroyer.release());

    PEG_METHOD_EXIT();
}

// Issue saved response. Used to send empty responses when an active
// enumerationContext times out.  This allows continuing operation when
// providers are not returning responses in a timely manner and eliminates
// client timeouts. NOTE: Generally, this only occurs when providers are VERY
// slow returning responses so that the client should probably delay before
// the next request.
// Returns true if a response was generated, false if not. The only reason for
// a response not to have been generated is if none are waiting.
// This should be an impossible condition.
// It is expected that any the context is locked external to this function
// KS_TODO make this common function for all cases were we issue the
// saved request.

void CIMOperationRequestDispatcher::issueSavedResponse(EnumerationContext* en)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::issueSavedResponse");

    PEGASUS_DEBUG_ASSERT(en->valid());

    PEGASUS_DEBUG_ASSERT(en->isProcessing());  // assert of en  is active

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "issueSavedResponse for ContextId=%s", CSTRING(en->getContextId()) ));

    if (en->_savedRequest != NULL)
    {
        // Force a response to be sent.  This allows sending a response
        // when there are no responses in the cache.
        _cimOperationRequestDispatcher->
            _issueImmediateOpenOrPullResponseMessage(
                en->_savedRequest,
                en->_savedResponse,
                en,
                0);

        // Delete the request; it was allocated for
        // this delayed response.
        delete en->_savedRequest;

        // clear the saved request to indicate it was used
        en->_savedRequest = NULL;
        en->_savedResponse = NULL;
        en->_savedOperationMaxObjectCount = 0;
    }
    PEG_METHOD_EXIT();
}

// EXP_PULL_END
/*
   Common processing for ExecQuery and OpenQueryInstances requests
  This code gets the provider lists and issues the requests.
*/
bool CIMOperationRequestDispatcher::handleQueryRequestCommon(
    CIMExecQueryRequestMessage* request,
    CIMException& cimException,
    EnumerationContext* enumerationContext,
    const char* queryLanguage,
    const CIMName& className,
    QueryExpressionRep* qx)
{
        PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleQueryRequestCommon");
        AutoPtr<QueryExpressionRep> destroyer(qx);
    //
    // Get names of descendent classes:
    //
    ProviderInfoList providerInfos;

    // This exception should not be required or we should apply for
    // all _lookupInstanceProvider Calls.

    try
    {
        providerInfos = _lookupAllInstanceProviders(
                request->nameSpace,
                className);
    }
    catch (CIMException& e)
    {
        // Return exception response if exception from getSubClasses
        cimException = e;
        PEG_METHOD_EXIT();
        return false;
    }

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED
    if (_CIMExceptionIfNoProvidersOrRepository(request, providerInfos,
                                               cimException))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    // We have instances for Providers and possibly repository.
    // Set up an aggregate object and save a copy of the original request.
    // NOTE: OperationAggregate released only when operation complete

    bool isPullOperation = (enumerationContext != NULL)? true : false;

    OperationAggregate* poA= new OperationAggregate(
        new CIMExecQueryRequestMessage(*request),
        className,
        request->nameSpace,
        providerInfos.providerCount,
        false, false,
        destroyer.release(),
        queryLanguage);

    if (isPullOperation)
    {
        poA->setPullOperation(enumerationContext);
    }

    // Build enum request for call to repository
    AutoPtr<CIMEnumerateInstancesRequestMessage> repRequest(
        new CIMEnumerateInstancesRequestMessage(
            request->messageId,
            request->nameSpace,
            CIMName(),
            false,false,false,
            CIMPropertyList(),
            request->queueIds,
            request->authType,
            request->userName));

    // Gather the repository responses and send as one response
    // with many instances
    //
    if (_enumerateFromRepository(repRequest.release(), poA, providerInfos))
    {
        CIMResponseMessage* response = poA->removeResponse(0);

        _forwardResponseForAggregation(
            new CIMExecQueryRequestMessage(*request),
            poA,
            response);
    }

    // Loop through providerInfos, forwarding requests to providers
    while (providerInfos.hasMore(true))
    {
        // If this class has a provider
        ProviderInfo& providerInfo = providerInfos.getNext();

        // KS_TODO we should show if it is a query or instance provider in trace
        providerInfos.pegRoutingTrace(providerInfo,
                         ((isPullOperation)? "OpenQueryInstances": "execQuery"),
                         request->messageId);

        ProviderIdContainer* providerIdContainer =
            providerInfo.providerIdContainer.get();

        // If not a provider with Query capability, execute
        // EnumerateInstances for the provider.
        if (providerInfo.hasNoQuery)
        {
            OperationContext* context = &request->operationContext;
            const OperationContext::Container* container = 0;
            container = &context->get(IdentityContainer::NAME);
            const IdentityContainer& identityContainer =
                dynamic_cast<const IdentityContainer&>(*container);

            AutoPtr<CIMEnumerateInstancesRequestMessage> enumReq(
                new CIMEnumerateInstancesRequestMessage(
                    request->messageId,
                    request->nameSpace,
                    providerInfo.className,
                    false,false,false,
                    CIMPropertyList(),
                    request->queueIds,
                    request->authType,
                    identityContainer.getUserName()));

            context = &enumReq->operationContext;

            if (providerIdContainer)
            {
                context->insert(*providerIdContainer);
            }

            context->insert(identityContainer);

            _forwardRequestForAggregation(
                providerInfo.serviceId,
                providerInfo.controlProviderName,
                enumReq.release(), poA);
        }
        else
        {
            AutoPtr<CIMExecQueryRequestMessage> requestCopy(
                new CIMExecQueryRequestMessage(*request));

            OperationContext* context = &request->operationContext;

            if (providerIdContainer)
            {
                context->insert(*providerIdContainer);
            }

            requestCopy->operationContext = *context;
            requestCopy->className = providerInfo.className;

            _forwardRequestForAggregation(
                providerInfo.serviceId,
                providerInfo.controlProviderName,
                requestCopy.release(), poA);
        }
    } // for all classes and derived classes

    PEG_METHOD_EXIT();
    return true;
}
// EXP_PULL_END

/****************************************************************************
**
** CIMOperationDispatcher request handlers.  There is a handler for each
** operation request type. These handlers process the requsts including:
**     - Validation of parameters (may generate error responses)
**     - Passage to the correct processor (service, control providers,
**       providers, repository) depending on message type)
**     - Generation of responses for those handlers that are synchronous.
**
****************************************************************************/

/**$*******************************************************
    handleGetClassRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleGetClassRequest(
    CIMGetClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleGetClassRequest");

    CIMClass cimClass =
        _repository->getClass(
            request->nameSpace,
            request->className,
            request->localOnly,
            request->includeQualifiers,
            request->includeClassOrigin,
            request->propertyList);

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleGetClassRequest - "
            "Namespace: %s  Class name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString())));

    AutoPtr<CIMGetClassResponseMessage> response(
        dynamic_cast<CIMGetClassResponseMessage*>(
            request->buildResponse()));
    response->cimClass = cimClass;

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleGetInstanceRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleGetInstanceRequest(
    CIMGetInstanceRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleGetInstanceRequest");

    // get the class name
    CIMName className = request->instanceName.getClassName();

    // Validate the class in the request. Returns class if not rejected
    CIMConstClass cimClass;
    if (_rejectInvalidClassParameter(request, cimClass))
    {
        PEG_METHOD_EXIT();
        return;
    }

    ProviderInfo providerInfo = _lookupInstanceProvider(
        request->nameSpace,
        className);

    if (providerInfo.hasProvider)
    {
        CIMGetInstanceRequestMessage* requestCopy =
            new CIMGetInstanceRequestMessage(*request);

        if (providerInfo.providerIdContainer.get() != 0)
        {
            requestCopy->operationContext.insert(
                *providerInfo.providerIdContainer.get());
        }

#ifdef PEGASUS_ENABLE_OBJECT_NORMALIZATION
        if (providerInfo.hasProviderNormalization)
        {
            requestCopy->operationContext.insert(
                CachedClassDefinitionContainer(cimClass));
        }
#endif

        CIMGetInstanceRequestMessage* requestCallbackCopy =
            new CIMGetInstanceRequestMessage(*requestCopy);

        _forwardRequestToSingleProvider(
            providerInfo,
            requestCopy,
            requestCallbackCopy);

         PEG_METHOD_EXIT();
         return;
    }

    // not internal or found provider, go to default

    if (_repository->isDefaultInstanceProvider())
    {
        CIMInstance cimInstance =
            _repository->getInstance(
                request->nameSpace,
                request->instanceName,
                request->includeQualifiers,
                request->includeClassOrigin,
                request->propertyList);

        AutoPtr<CIMGetInstanceResponseMessage> response(
            dynamic_cast<CIMGetInstanceResponseMessage*>(
                request->buildResponse()));
        response->getResponseData().setInstance(cimInstance);

        _enqueueResponse(request, response.release());
    }
    else // No provider is registered and the repository isn't the default
    {
        CIMResponseMessage* response = request->buildResponse();
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

        _enqueueResponse(request, response);
    }

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleDeleteClassRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleDeleteClassRequest(
    CIMDeleteClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleDeleteClassRequest");

    _repository->deleteClass(
        request->nameSpace,
        request->className);

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleDeleteClassRequest - "
            "Namespace: %s  Class Name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString())));

    AutoPtr<CIMDeleteClassResponseMessage> response(
        dynamic_cast<CIMDeleteClassResponseMessage*>(
            request->buildResponse()));

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleDeleteInstanceRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleDeleteInstanceRequest(
    CIMDeleteInstanceRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleDeleteInstanceRequest");

    // get the class name
    CIMName className = request->instanceName.getClassName();

    if (!_checkExistenceOfClass(request->nameSpace, className))
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "CIMOperationRequestDispatcher::handleDeleteInstanceRequest - "
                "CIM class does not exist exception has occurred.  "
                "Namespace: %s  Class Name: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(className.getString())));

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_CLASS, className.getString());
    }

    ProviderInfo providerInfo = _lookupInstanceProvider(
        request->nameSpace,
        className);

    if (providerInfo.hasProvider)
    {
        CIMDeleteInstanceRequestMessage* requestCopy =
            new CIMDeleteInstanceRequestMessage(*request);

        if (providerInfo.providerIdContainer.get() != 0)
        {
            requestCopy->operationContext.insert(
                *providerInfo.providerIdContainer.get());

            //delete providerIdContainer;
            //providerIdContainer = 0;
        }

        CIMDeleteInstanceRequestMessage* requestCallbackCopy =
            new CIMDeleteInstanceRequestMessage(*requestCopy);

        _forwardRequestToSingleProvider(
            providerInfo,
            requestCopy,
            requestCallbackCopy);

        PEG_METHOD_EXIT();
        return;
    }
    else if (_repository->isDefaultInstanceProvider())
    {
        _repository->deleteInstance(
            request->nameSpace,
            request->instanceName);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
            "CIMOperationRequestDispatcher::handleDeleteInstanceRequest - "
                "Namespace: %s  Instance Name: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(request->instanceName.toString())));

        AutoPtr<CIMDeleteInstanceResponseMessage> response(
            dynamic_cast<CIMDeleteInstanceResponseMessage*>(
                request->buildResponse()));

        _enqueueResponse(request, response.release());
    }
    else // No provider is registered and the repository isn't the default
    {
        CIMResponseMessage* response = request->buildResponse();
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

        _enqueueResponse(request, response);
    }

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleCreateClassRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleCreateClassRequest(
    CIMCreateClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleCreateClassRequest");

    removePropagatedAndOriginAttributes(request->newClass);

    _repository->createClass(request->nameSpace, request->newClass);

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleCreateClassRequest - "
            "Namespace: %s  Class Name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString())));

    AutoPtr<CIMCreateClassResponseMessage> response(
        dynamic_cast<CIMCreateClassResponseMessage*>(
            request->buildResponse()));

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleCreateInstanceRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleCreateInstanceRequest(
    CIMCreateInstanceRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleCreateInstanceRequest()");

    // get the class name
    CIMName className = request->newInstance.getClassName();

    if (!_checkExistenceOfClass(request->nameSpace, className))
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "CIMOperationRequestDispatcher::handleCreateInstanceRequest - "
                "CIM class does not exist exception has occurred.  "
                "Namespace: %s  Class Name: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(className.getString())));

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_CLASS, className.getString());
    }

    ProviderInfo providerInfo = _lookupInstanceProvider(
        request->nameSpace,
        className);

    if (providerInfo.hasProvider)
    {
        CIMCreateInstanceRequestMessage* requestCopy =
            new CIMCreateInstanceRequestMessage(*request);

        removePropagatedAndOriginAttributes(requestCopy->newInstance);

        if (providerInfo.providerIdContainer.get() != 0)
        {
            requestCopy->operationContext.insert(
                *providerInfo.providerIdContainer.get());
        }

        CIMCreateInstanceRequestMessage* requestCallbackCopy =
            new CIMCreateInstanceRequestMessage(*requestCopy);

        _forwardRequestToSingleProvider(
            providerInfo,
            requestCopy,
            requestCallbackCopy);

        PEG_METHOD_EXIT();
        return;
    }
    else if (_repository->isDefaultInstanceProvider())
    {
        removePropagatedAndOriginAttributes(request->newInstance);

        CIMObjectPath instanceName = _repository->createInstance(
            request->nameSpace,
            request->newInstance);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
            "CIMOperationRequestDispatcher::handleCreateInstanceRequest - "
                "Namespace: %s  Instance Name: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(request->newInstance.getClassName().getString())));

        AutoPtr<CIMCreateInstanceResponseMessage> response(
            dynamic_cast<CIMCreateInstanceResponseMessage*>(
                request->buildResponse()));
        response->instanceName = instanceName;

        _enqueueResponse(request, response.release());
    }
    else // No provider is registered and the repository isn't the default
    {
        CIMResponseMessage* response = request->buildResponse();
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

        _enqueueResponse(request, response);
    }

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleModifyClassRequest
    This request is analyzed and if correct, is passed directly to the
    repository
**********************************************************/
void CIMOperationRequestDispatcher::handleModifyClassRequest(
    CIMModifyClassRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
       "CIMOperationRequestDispatcher::handleModifyClassRequest");

    removePropagatedAndOriginAttributes(request->modifiedClass);

    _repository->modifyClass(request->nameSpace, request->modifiedClass);

    AutoPtr<CIMModifyClassResponseMessage> response(
        dynamic_cast<CIMModifyClassResponseMessage*>(
            request->buildResponse()));

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleModifyInstanceRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleModifyInstanceRequest(
    CIMModifyInstanceRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleModifyInstanceRequest");

    CIMName className = request->modifiedInstance.getClassName();

    if (!_checkExistenceOfClass(request->nameSpace, className))
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "CIMOperationRequestDispatcher::handleModifyInstanceRequest - "
                "CIM class does not exist exception has occurred.  "
                "Namespace: %s  Class Name: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(className.getString())));

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_CLASS, className.getString());
    }

    ProviderInfo providerInfo = _lookupInstanceProvider(
        request->nameSpace,
        className);

    if (providerInfo.hasProvider)
    {
        CIMModifyInstanceRequestMessage* requestCopy =
            new CIMModifyInstanceRequestMessage(*request);
        removePropagatedAndOriginAttributes(requestCopy->modifiedInstance);
        if (providerInfo.providerIdContainer.get() != 0)
        {
            requestCopy->operationContext.insert(
                *providerInfo.providerIdContainer.get());
        }

        CIMModifyInstanceRequestMessage* requestCallbackCopy =
            new CIMModifyInstanceRequestMessage(*requestCopy);

        _forwardRequestToSingleProvider(
            providerInfo,
            requestCopy,
            requestCallbackCopy);

        PEG_METHOD_EXIT();
        return;
    }
    else if (_repository->isDefaultInstanceProvider())
    {
        removePropagatedAndOriginAttributes(request->modifiedInstance);

        _repository->modifyInstance(
            request->nameSpace,
            request->modifiedInstance,
            request->includeQualifiers,request->propertyList);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
            "CIMOperationRequestDispatcher::handleModifyInstanceRequest - "
                "Namespace: %s  Instance Name: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(request->modifiedInstance.getClassName().getString())));

        AutoPtr<CIMModifyInstanceResponseMessage> response(
            dynamic_cast<CIMModifyInstanceResponseMessage*>(
                request->buildResponse()));

        _enqueueResponse(request, response.release());
    }
    else // No provider is registered and the repository isn't the default
    {
        CIMResponseMessage* response = request->buildResponse();
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

       _enqueueResponse(request, response);
    }

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleEnumerateClassesRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleEnumerateClassesRequest(
    CIMEnumerateClassesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleEnumerateClassesRequest");

    Array<CIMClass> cimClasses =
        _repository->enumerateClasses(
            request->nameSpace,
            request->className,
            request->deepInheritance,
            request->localOnly,
            request->includeQualifiers,
            request->includeClassOrigin);

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleEnumerateClassesRequest - "
            "Namespace: %s  Class name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString())));

    AutoPtr<CIMEnumerateClassesResponseMessage> response(
        dynamic_cast<CIMEnumerateClassesResponseMessage*>(
            request->buildResponse()));
    response->cimClasses = cimClasses;

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleEnumerateClassNamesRequest
**********************************************************/
void CIMOperationRequestDispatcher::handleEnumerateClassNamesRequest(
    CIMEnumerateClassNamesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleEnumerateClassNamesRequest");

    Array<CIMName> classNames =
        _repository->enumerateClassNames(
            request->nameSpace,
            request->className,
            request->deepInheritance);

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleEnumerateClassNamesRequest - "
            "Namespace: %s  Class name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString())));

    AutoPtr<CIMEnumerateClassNamesResponseMessage> response(
        dynamic_cast<CIMEnumerateClassNamesResponseMessage*>(
            request->buildResponse()));
    response->classNames = classNames;

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleEnumerateInstancesRequest

    if !validClassName
        generate exception response
        return
    get all subclasses to target class
    for all classes
        get Provider for Class
    if (number of providers > BreadthLimit)
        generate exception
        return
    if (no providers found) and !(repository is default provider)
        generate CIM_ERR_NOT_SUPPORTED response
        return
    for all targetclass and subclasses
        if (class has a provider)
            copy request
            substitute current class name
            forward request to provider
    if (repository is default provider)
        for all targetclass and subclasses
            if !(class has a provider)
                issue request for this class to repository
                put response on aggregate list
**********************************************************/

void CIMOperationRequestDispatcher::handleEnumerateInstancesRequest(
    CIMEnumerateInstancesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleEnumerateInstancesRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleEnumerateInstancesRequest - "
            "Namespace=%s  ClassName=%s messageId=%s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString()),
        CSTRING(request->messageId)));

    //
    // Validate the class in the request and get the target class to
    // be used later in the operation.
    //
    CIMConstClass cimClass;
    if (_rejectInvalidClassParameter(request,cimClass))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Set the propertylist to be forwarded in accord with the propertyList
    // parameter. The property list forwarded to providers
    // is constructed from properties in the top level class  when
    // !deepInheritance && propertyList.isNull() to assure
    // that providers will return only the properties of the requested class.
    // NOTE - The request class pl should really be null.  Subclasses should
    // get the propertyList. But we do not have a record today of the
    // original class in the providerInfo list so everybody gets the
    // list.

    if (!request->deepInheritance && request->propertyList.isNull())
    {
        _buildPropertyListFromClass(cimClass, request->propertyList);
    }

    //
    // Get names of descendent classes and list of providers
    // For Enumerate Operations; the list of providers is used for both
    // repository and provider requests.
    //
    ProviderInfoList providerInfos = _lookupAllInstanceProviders(
        request->nameSpace,
        request->className);

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED

    if (_rejectNoProvidersOrRepository(request, providerInfos))
    {
        PEG_METHOD_EXIT();
        return;
    }

    //
    // Set up an aggregate object with a copy of the original request.
    // NOTE: OperationAggregate released only when operation complete.
    // requiresHostnameCompletion false, hasPropList true
    //
    OperationAggregate* poA= new OperationAggregate(
        new CIMEnumerateInstancesRequestMessage(*request),
        request->className,
        request->nameSpace,
        providerInfos.providerCount,
        false,          // completeHostAndNamespace = false
        true);          // has propertyList = true

    // Gather the repository responses and send as one response
    // with many instances
    //

    if (_enumerateFromRepository(request, poA, providerInfos))
    {
        CIMResponseMessage* response = poA->removeResponse(0);
        _forwardResponseForAggregation(
            new CIMEnumerateInstancesRequestMessage(*request),
            poA, response);
    }
    ProviderRequests::issueEnumerationRequestsToProviders(
        this,
        request,
        providerInfos,
        poA,
        "enumerateInstances");

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleEnumerateInstanceNamesRequest

    if !validClassName
        generate exception response
        return
    get all subclasses to target class
    for all classes
        get Provider for Class
    if (number of providers > BreadthLimit)
        generate exception
        return
    if (no providers found) and !(repository is default provider)
        generate CIM_ERR_NOT_SUPPORTED response
        return
    for all targetclass and subclasses
        if (class has a provider)
            copy request
            substitute current class name
            forward request to provider
    if (repository is default provider)
        for all targetclass and subclasses
            if !(class has a provider)
                issue request for this class to repository
                put response on aggregate list
**********************************************************/

void CIMOperationRequestDispatcher::handleEnumerateInstanceNamesRequest(
    CIMEnumerateInstanceNamesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleEnumerateInstanceNamesRequest");

    //
    // Validate the class name in the request
    //
    CIMConstClass cimClass;
    if (_rejectInvalidClassParameter(request, cimClass))
    {
            PEG_METHOD_EXIT();
            return;
    }

    //
    // Get names of descendent classes and list of providers
    //
    ProviderInfoList providerInfos = _lookupAllInstanceProviders(
        request->nameSpace,
        request->className);

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED

    if (_rejectNoProvidersOrRepository(request, providerInfos))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // We have instances for Providers and possibly repository.
    // Set up an aggregate object and save a copy of the original request.
    // NOTE: OperationAggregate released only when operation complete
    //
    OperationAggregate* poA= new OperationAggregate(
        new CIMEnumerateInstanceNamesRequestMessage(*request),
        request->className,
        request->nameSpace,
        providerInfos.providerCount,
        false);   // completeHostAndNamespace = false, and no property list

    if (_enumerateFromRepository(request, poA, providerInfos))
    {
        CIMResponseMessage* response = poA->removeResponse(0);

        _forwardResponseForAggregation(
            new CIMEnumerateInstanceNamesRequestMessage(*request),
            poA,
            response);
    }

    // Template to issue EnumerationRequest messages to Providers.
    ProviderRequests::issueEnumerationRequestsToProviders(
        this,
        request,
        providerInfos,
        poA,
        "enumerateInstanceNames");

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleAssociatorsRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleAssociatorsRequest(
    CIMAssociatorsRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleAssociatorsRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleAssociators - "
            "Namespace=%s  ClassName=%s messageId=%s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->objectName.toString()),
        CSTRING(request->messageId)));

    //  validate className and objectClassName same.  This is
    //  an invariant for the handleAssoc, etc functions.
    PEGASUS_DEBUG_ASSERT(
        request->className == request->objectName.getClassName());

    if (_rejectAssociationTraversalDisabled(request, "Associators"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Validate role and resultrole parameters
    if (_rejectInvalidRoleParameter(request, request->role, "role"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    if (_rejectInvalidRoleParameter(request, request->resultRole, "resultrole"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    if (_rejectInvalidClassParameter(request, request->nameSpace,
                                request->objectName))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // The entity that creates the CIMMessage determines if this is a class
    // or instance operation.

    if (request->isClassRequest)
    {
        //
        // For Class requests, get the results from the repository
        //

        PEG_TRACE_CSTRING(TRC_DISPATCHER, Tracer::LEVEL3,
            "Associators executing Class request");

        Array<CIMObject> cimObjects =
            _repository->associators(
                request->nameSpace,
                request->objectName,
                request->assocClass,
                request->resultClass,
                request->role,
                request->resultRole,
                request->includeQualifiers,
                request->includeClassOrigin,
                request->propertyList);

        AutoPtr<CIMAssociatorsResponseMessage> response(
            dynamic_cast<CIMAssociatorsResponseMessage*>(
                request->buildResponse()));

        // Flag to indicate that these are class objects is passed
        // from request in buildResponse above

        response->getResponseData().setObjects(cimObjects);

        _enqueueResponse(request, response.release());
    }

    // This is an instance request
    else
    {
        //
        // For Instance requests, get results from providers and the repository
        // Determine list of providers for this request. Note that
        // providerCount may not be the same as the size of the returned
        // array because not all Class will have providers.
        //
        ProviderInfoList providerInfos = _lookupAllAssociationProviders(
            request->nameSpace,
            request->objectName,
            request->assocClass,
            String::EMPTY);

        // If no provider is registered and the repository isn't the default,
        // return CIM_ERR_NOT_SUPPORTED

        if (_rejectNoProvidersOrRepository(request, providerInfos))
        {
            PEG_METHOD_EXIT();
            return;
        }
        //
        // Get the instances from the repository, as necessary
        //

        // Hold the repository results in a response message.
        // If not using the repository, this pointer is null.
        AutoPtr<CIMAssociatorsResponseMessage> response;

        if (_repository->isDefaultInstanceProvider())
        {
            response.reset(dynamic_cast<CIMAssociatorsResponseMessage*>(
                request->buildResponse()));

            const Array<CIMObject>& cimObjects = _repository->associators(
                request->nameSpace,
                request->objectName,
                request->assocClass,
                request->resultClass,
                request->role,
                request->resultRole,
                request->includeQualifiers,
                request->includeClassOrigin,
                request->propertyList);

            response->getResponseData().setObjects(cimObjects);

            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                "Associators repository access: class = %s, count = %u.",
                    CSTRING(request->objectName.toString()),
                    response->getResponseData().getObjects().size()));
        }
        // Issue Provider requests and process the respository responseData
        ProviderRequests::issueAssocRequestsToProviders(
            this,
            request,
            response,
            providerInfos,
            "Associators");
    }  // End of instance processing

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleAssociatorNamesRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleAssociatorNamesRequest(
    CIMAssociatorNamesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleAssociatorNamesRequest");

    PEG_TRACE(( TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleAssociatorNames - "
            "Namespace: %s  Class name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->objectName.toString())));

    //  validate className and objectClassName same.  This is
    //  an invariant for the handleAssoc, etc functions.
    PEGASUS_DEBUG_ASSERT(
        request->className == request->objectName.getClassName());

    if (_rejectAssociationTraversalDisabled(request,"AssociatorNames"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Validate role parameters syntax

    if (_rejectInvalidRoleParameter(request, request->role, "role"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    if (_rejectInvalidRoleParameter(request, request->resultRole, "resultrole"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    if (_rejectInvalidClassParameter(request, request->nameSpace,
                                request->objectName))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // If Class request, get class data from repository
    if (request->isClassRequest)
    {
        //
        // For Class requests, get the results from the repository
        //

        PEG_TRACE_CSTRING(TRC_DISPATCHER, Tracer::LEVEL3,
            "AssociatorNames executing Class request");

        Array<CIMObjectPath> objectNames =
            _repository->associatorNames(
                request->nameSpace,
                request->objectName,
                request->assocClass,
                request->resultClass,
                request->role,
                request->resultRole);

        AutoPtr<CIMAssociatorNamesResponseMessage> response(
            dynamic_cast<CIMAssociatorNamesResponseMessage*>(
                request->buildResponse()));

        // Flag to indicate that these are class objects is passed
        // from request in buildResponse above.

        response->getResponseData().setInstanceNames(objectNames);

        _enqueueResponse(request, response.release());
    }
    // Else this is an Instance request
    else
    {
        //
        // For Instance requests, get results from providers and the repository
        // Determine list of providers for this request
        //
        ProviderInfoList providerInfos = _lookupAllAssociationProviders(
            request->nameSpace,
            request->objectName,
            request->assocClass,
            String::EMPTY);

        // If no provider is registered and the repository isn't the default,
        // return CIM_ERR_NOT_SUPPORTED

        if (_rejectNoProvidersOrRepository(request, providerInfos))
        {
            PEG_METHOD_EXIT();
            return;
        }

        // Get from repository if it exists.
        // Hold the repository results in a response message.
        // If not using the repository, this pointer is null.
        AutoPtr<CIMAssociatorNamesResponseMessage> response;

        if (_repository->isDefaultInstanceProvider())
        {
            response.reset(dynamic_cast<CIMAssociatorNamesResponseMessage*>(
                request->buildResponse()));

            Array<CIMObjectPath> objectNames = _repository->associatorNames(
                request->nameSpace,
                request->objectName,
                request->assocClass,
                request->resultClass,
                request->role,
                request->resultRole);
            response->getResponseData().setInstanceNames(objectNames);

            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                "AssociatorNames repository access: class = %s, count = %u.",
                CSTRING(request->objectName.toString()),
                objectNames.size()));
        }
        // Issue Provider requests and process the respository responseData
        ProviderRequests::issueAssocRequestsToProviders(
            this,
            request,
            response,
            providerInfos,
            "AssociatorNames");

    }  // End of instance processing

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleReferencesRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleReferencesRequest(
    CIMReferencesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleReferencesRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleReferences - "
            "Namespace: %s  Class name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->objectName.toString())));

    //  validate className and objectClassName same.  This is
    //  an invariant for the handleAssoc, etc functions.
    PEGASUS_DEBUG_ASSERT(
        request->className == request->objectName.getClassName());

    if (_rejectAssociationTraversalDisabled(request,"References"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Validate role parameter syntax
    if (_rejectInvalidRoleParameter(request, request->role, "role"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    if (_rejectInvalidClassParameter(request, request->nameSpace,
                                request->objectName))
    {
        PEG_METHOD_EXIT();
        return;
    }


    // If true, this is a class Request
    if (request->isClassRequest)
    {
        //
        // For Class requests, get the results from the repository
        //

        PEG_TRACE_CSTRING(TRC_DISPATCHER, Tracer::LEVEL4,
            "References executing Class request");

        Array<CIMObject> cimObjects =
            _repository->references(
                request->nameSpace,
                request->objectName,
                request->resultClass,
                request->role,
                request->includeQualifiers,
                request->includeClassOrigin,
                request->propertyList);

        AutoPtr<CIMReferencesResponseMessage> response(
            dynamic_cast<CIMReferencesResponseMessage*>(
                request->buildResponse()));

        // Flag to indicate that these are class objects is passed
        // from request in buildResponse above

        response->getResponseData().setObjects(cimObjects);

        _enqueueResponse(request, response.release());
    }

    // Else Instance Request
    else
    {
        //
        // For Instance requests, get results from providers and the repository
        // Determine list of Classes and providers for this request
        //
        ProviderInfoList providerInfos = _lookupAllAssociationProviders(
            request->nameSpace,
            request->objectName,
            request->resultClass,
            String::EMPTY);

        // If no provider is registered and the repository isn't the default,
        // return CIM_ERR_NOT_SUPPORTED

        if (_rejectNoProvidersOrRepository(request, providerInfos))
        {
            PEG_METHOD_EXIT();
            return;
        }
        //
        // Get the instances from the repository, as necessary.
        // Hold the repository results in a response message.
        // If not using the repository, this pointer is null.
        //
        AutoPtr<CIMReferencesResponseMessage> response;

        if (_repository->isDefaultInstanceProvider())
        {
            response.reset(dynamic_cast<CIMReferencesResponseMessage*>(
                request->buildResponse()));

            Array<CIMObject> cimObjects = _repository->references(
                request->nameSpace,
                request->objectName,
                request->resultClass,
                request->role,
                request->includeQualifiers,
                request->includeClassOrigin,
                request->propertyList);
            response->getResponseData().setObjects(cimObjects);

            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                "References repository access: class = %s, count = %u.",
                    CSTRING(request->objectName.toString()),
                    cimObjects.size()));
        }

        // Issue Provider requests and process the respository responseData
        ProviderRequests::issueAssocRequestsToProviders(
            this,
            request,
            response,
            providerInfos,
            "References");

    }  // End of instance processing

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleReferenceNamesRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleReferenceNamesRequest(
    CIMReferenceNamesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleReferenceNamesRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleReferenceNames - "
            "Namespace: %s  Class name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->objectName.toString())));

    //  validate className and objectClassName same.  This is
    //  an invariant for the handleAssoc, etc functions.
    PEGASUS_DEBUG_ASSERT(
        request->className == request->objectName.getClassName());

    if (_rejectAssociationTraversalDisabled(request,"ReferenceNames"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Validate role parameter syntax
    if (_rejectInvalidRoleParameter(request, request->role, "role"))
    {
        PEG_METHOD_EXIT();
        return;
    }

    if (_rejectInvalidClassParameter(request, request->nameSpace,
                                request->objectName))
    {
        PEG_METHOD_EXIT();
        return;
    }

    if (request->isClassRequest)
    {
        //
        // For Class requests, get the results from the repository
        //

        PEG_TRACE_CSTRING(TRC_DISPATCHER, Tracer::LEVEL4,
            "ReferenceNames executing Class request");

        Array<CIMObjectPath> objectNames =
            _repository->referenceNames(
                request->nameSpace,
                request->objectName,
                request->resultClass,
                request->role);

        AutoPtr<CIMReferenceNamesResponseMessage> response(
            dynamic_cast<CIMReferenceNamesResponseMessage*>(
                request->buildResponse()));

        // Flag to indicate that these are class objects is passed
        // from request in buildResponse above.

        response->getResponseData().setInstanceNames(objectNames);

        _enqueueResponse(request, response.release());
    }

    //  else Instance operation
    else
    {
        //
        // For Instance requests, get results from providers and the repository
        // Determine list of providers for this request
        //
        ProviderInfoList providerInfos = _lookupAllAssociationProviders(
            request->nameSpace,
            request->objectName,
            request->resultClass,
            String::EMPTY);

        // If no provider is registered and the repository isn't the default,
        // return CIM_ERR_NOT_SUPPORTED

        if (_rejectNoProvidersOrRepository(request, providerInfos))
        {
            PEG_METHOD_EXIT();
            return;
        }

        // Get  objectNames from repository if repository
        // is default provider.
        AutoPtr<CIMReferenceNamesResponseMessage> response;

        if (_repository->isDefaultInstanceProvider())
        {
            response.reset(dynamic_cast<CIMReferenceNamesResponseMessage*>(
                request->buildResponse()));

            Array<CIMObjectPath> objectNames = _repository->referenceNames(
                request->nameSpace,
                request->objectName,
                request->resultClass,
                request->role);

            response->getResponseData().setInstanceNames(objectNames);

            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                "ReferenceNames repository access: class = %s, count = %u.",
                CSTRING(request->objectName.toString()),
                objectNames.size()));
        }

        // Issue Provider requests and process the respository responseData
        // NOTE: This is a template
        ProviderRequests::issueAssocRequestsToProviders(
            this,
            request,
            response,
            providerInfos,
            "ReferenceNames");

    }  // End of instance processing

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleGetPropertyRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleGetPropertyRequest(
    CIMGetPropertyRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleGetPropertyRequest");

    CIMName className = request->instanceName.getClassName();

    ProviderInfo providerInfo = _lookupInstanceProvider(
        request->nameSpace,
        className);

    if (providerInfo.hasProvider)
    {
        CIMGetPropertyRequestMessage* requestCopy =
            new CIMGetPropertyRequestMessage(*request);

        if (providerInfo.providerIdContainer.get() != 0)
        {
            requestCopy->operationContext.insert(
                *providerInfo.providerIdContainer.get());
        }

        CIMGetPropertyRequestMessage* requestCallbackCopy =
            new CIMGetPropertyRequestMessage(*requestCopy);

        _forwardRequestToSingleProvider(
            providerInfo,
            requestCopy,
            requestCallbackCopy);
    }
    else if (_repository->isDefaultInstanceProvider())
    {
        CIMValue value =
            _repository->getProperty(
                request->nameSpace,
                request->instanceName,
                request->propertyName);

        AutoPtr<CIMGetPropertyResponseMessage> response(
            dynamic_cast<CIMGetPropertyResponseMessage*>(
                request->buildResponse()));
        response->value = value;

        _enqueueResponse(request, response.release());
    }
    else // No provider is registered and the repository isn't the default
    {
        CIMResponseMessage* response = request->buildResponse();
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

        _enqueueResponse(request, response);
    }
    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleSetPropertyRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleSetPropertyRequest(
    CIMSetPropertyRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleSetPropertyRequest");

    _fixSetPropertyValueType(request);

    CIMName className = request->instanceName.getClassName();

    ProviderInfo providerInfo = _lookupInstanceProvider(
        request->nameSpace,
        className);

    if (providerInfo.hasProvider)
    {
        CIMSetPropertyRequestMessage* requestCopy =
            new CIMSetPropertyRequestMessage(*request);

        if (providerInfo.providerIdContainer.get() != 0)
        {
            requestCopy->operationContext.insert(
                *providerInfo.providerIdContainer.get());
        }

        CIMSetPropertyRequestMessage* requestCallbackCopy =
            new CIMSetPropertyRequestMessage(*requestCopy);

        _forwardRequestToSingleProvider(
            providerInfo,
            requestCopy,
            requestCallbackCopy);
    }
    else if (_repository->isDefaultInstanceProvider())
    {
        _repository->setProperty(
            request->nameSpace,
            request->instanceName,
            request->propertyName,
            request->newValue);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
            "CIMOperationRequestDispatcher::handleSetPropertyRequest - "
                "Namespace: %s  Instance Name: %s  Property Name: %s  New "
                "Value: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(request->instanceName.getClassName().getString()),
            CSTRING(request->propertyName.getString()),
            CSTRING(request->newValue.toString())));

        AutoPtr<CIMSetPropertyResponseMessage> response(
            dynamic_cast<CIMSetPropertyResponseMessage*>(
                request->buildResponse()));

        _enqueueResponse(request, response.release());
    }
    else // No provider is registered and the repository isn't the default
    {
        CIMResponseMessage* response = request->buildResponse();
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

        _enqueueResponse(request, response);
    }

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleGetQualifierRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleGetQualifierRequest(
    CIMGetQualifierRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleGetQualifierRequest");

    CIMQualifierDecl cimQualifierDecl =
        _repository->getQualifier(
            request->nameSpace,
            request->qualifierName);

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleGetQualifierRequest - "
            "Namespace: %s  Qualifier Name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->qualifierName.getString())));

    AutoPtr<CIMGetQualifierResponseMessage> response(
        dynamic_cast<CIMGetQualifierResponseMessage*>(
            request->buildResponse()));
    response->cimQualifierDecl = cimQualifierDecl;

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleSetQualifierRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleSetQualifierRequest(
    CIMSetQualifierRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleSetQualifierRequest");

    _repository->setQualifier(
        request->nameSpace,
        request->qualifierDeclaration);

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleSetQualifierRequest - "
            "Namespace: %s  Qualifier Name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->qualifierDeclaration.getName().getString())));

    AutoPtr<CIMSetQualifierResponseMessage> response(
        dynamic_cast<CIMSetQualifierResponseMessage*>(
            request->buildResponse()));

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleDeleteQualifierRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleDeleteQualifierRequest(
    CIMDeleteQualifierRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleDeleteQualifierRequest");

    _repository->deleteQualifier(
        request->nameSpace,
        request->qualifierName);

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleDeleteQualifierRequest - "
            "Namespace: %s  Qualifier Name: %s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->qualifierName.getString())));

    AutoPtr<CIMDeleteQualifierResponseMessage> response(
        dynamic_cast<CIMDeleteQualifierResponseMessage*>(
            request->buildResponse()));

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleEnumerateQualifiersRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleEnumerateQualifiersRequest(
    CIMEnumerateQualifiersRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleEnumerateQualifiersRequest");

    Array<CIMQualifierDecl> qualifierDeclarations =
        _repository->enumerateQualifiers(request->nameSpace);

    PEG_TRACE((TRC_DISPATCHER,Tracer::LEVEL3,
        "CIMOperationRequestDispatcher::handleEnumerateQualifiersRequest - "
            "Namespace: %s",
        CSTRING(request->nameSpace.getString())));

    AutoPtr<CIMEnumerateQualifiersResponseMessage> response(
        dynamic_cast<CIMEnumerateQualifiersResponseMessage*>(
            request->buildResponse()));
    response->qualifierDeclarations = qualifierDeclarations;

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
}

/**$*******************************************************
    handleExecQueryRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleExecQueryRequest(
    CIMExecQueryRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleExecQueryRequest");

    AutoPtr<CIMExecQueryResponseMessage> response(
        dynamic_cast<CIMExecQueryResponseMessage*>(
            request->buildResponse()));

    Boolean exception = false;

#ifdef PEGASUS_DISABLE_EXECQUERY
    response->cimException =
        PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
    exception=true;
#else
    CIMException cimException;
    if (!QuerySupportRouter::routeHandleExecQueryRequest(this,request,
                                                         cimException))
    {
        response->cimException = cimException;
        exception = true;
    }
#endif

    if (exception)
    {
        _enqueueResponse(request, response.release());
        PEG_METHOD_EXIT();
        return;
    }

    PEG_METHOD_EXIT();
    return;
}

/**$*******************************************************
    handleInvokeMethodRequest
**********************************************************/

void CIMOperationRequestDispatcher::handleInvokeMethodRequest(
    CIMInvokeMethodRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleInvokeMethodRequest");

    _fixInvokeMethodParameterTypes(request);

    CIMName className = request->instanceName.getClassName();

    if (!_checkExistenceOfClass(request->nameSpace, className))
    {
        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "CIMOperationRequestDispatcher::handleInvokeMethodRequest - "
                "CIM class does not exist exception has occurred.  "
                "Namespace: %s  Class Name: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(className.getString()) ));

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, className.getString());
    }

    // Find Provider for this method
    ProviderIdContainer* providerIdContainer=NULL;
    ProviderInfo providerInfo(className);

    // First determine if there is an internal provider. If not, try
    // for a registered provider.
    if (!_lookupInternalProvider(request->nameSpace, className, providerInfo))
    {
        // check the class name for a registered provider for this namespace
        // classname and methodname. There can be only one.
        String providerName = _lookupMethodProvider(
            request->nameSpace,
            className,
            request->methodName,
            &providerIdContainer);

        // if provider name returned, set the provider info to
        // go to providerManagerService
        if (providerName.size() != 0)
        {
            providerInfo.addProviderInfo(
                _providerManagerServiceId,
                true,
                false);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
            "CIMOperationRequestDispatcher::invokeMethodProvider - "
                "Namespace: %s  Class Name: %s"
                " Provider Name: %s found. ",
            CSTRING(request->nameSpace.getString()),
            CSTRING(className.getString()),
            CSTRING(providerName) ));
        }
        else
        {
            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL1,
                "CIMOperationRequestDispatcher::invokeMethodProvider %s - "
                    "Namespace: %s  Class Name: %s  Service Name: %s  "
                    "Control Provider Name %s"
                    "Provider Name: %s found. ",
                ((providerInfo.controlProviderName.size() ==0)?
                    "Intern": "Extern"),
                CSTRING(request->nameSpace.getString()),
                CSTRING(className.getString()),
                _getServiceName(_providerManagerServiceId),
                CSTRING(providerInfo.controlProviderName),
                CSTRING(providerName) ));
        }
    }

    if (providerInfo.hasProvider)
    {
        CIMInvokeMethodRequestMessage* requestCopy =
            new CIMInvokeMethodRequestMessage(*request);

        CIMInvokeMethodRequestMessage* requestCallbackCopy =
            new CIMInvokeMethodRequestMessage(*requestCopy);

        if (providerIdContainer != NULL)
        {
            requestCopy->operationContext.insert(*providerIdContainer);
            delete providerIdContainer;
            providerIdContainer = NULL;
        }

        _forwardRequestToSingleProvider(
            providerInfo,
            requestCopy,
            requestCallbackCopy);
    }

    else // No provider for this method.
    {
        // No provider or control provider.  Generate exception
        CIMResponseMessage* response = request->buildResponse();
        response->cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_METHOD_NOT_AVAILABLE,
                request->methodName.getString());

    _enqueueResponse(request, response);
    }
    PEG_METHOD_EXIT();
}

//KS_PULL_BEGIN
/*************************************************************************
//
//  Pull operations introduced in Pegasus 2.14 per DMTF DSP200 - V 1.4
//
*************************************************************************/

/**$**************************************************************************
**
**    handleOpenEnumerateInstancesRequest
**
*****************************************************************************/

bool CIMOperationRequestDispatcher::handleOpenEnumerateInstancesRequest(
    CIMOpenEnumerateInstancesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleOpenEnumerateInstancesRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "OpenEnumerateInstances request namespace=%s class=%s "
            "deepInheritance=%s "
            "includeClassOrigin=%s "
            "propertyList=%s "
            "filterQueryLanguage=%s "
            "filterQuery=%s "
            "operationTimeout=%s "
            "continueOnError=%s "
            "maxObjectCount=%u "
            "messageId=%s",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString()),
        boolToString(request->deepInheritance),
        boolToString(request->includeClassOrigin),
        CSTRING(request->propertyList.toString()),
        CSTRING(request->filterQueryLanguage),
        CSTRING(request->filterQuery),
        CSTRING(request->operationTimeout.toString()),
        boolToString(request->continueOnError),
        request->maxObjectCount,
        CSTRING(request->messageId) ));

    // get the class name or generate error if class not found for target
    // namespace.
    CIMName className = request->className;
    CIMConstClass cimClass;
    if (_rejectInvalidClassParameter(request, cimClass))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // Test the common open request input parameters and reject if any
    // are incorrect. Tests filterQuery, continueOnError, operationTimeout
    // and maxObjectCount
    Uint32 operationMaxObjectCount;
    if (_rejectIfPullParametersFailTests(request,operationMaxObjectCount,
                                         ENABLE_FQL_FILTER))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // If DeepInheritance==false and no PropertyList was specified by the
    // client, the provider PropertyList should contain all the properties
    // in the specified class. This forces subclasses to handle defined
    // list rather than their full property list.
    if (!request->deepInheritance && request->propertyList.isNull())
    {
        _buildPropertyListWithTags(cimClass, request->propertyList);
    }

    //
    // Get names of descendent classes with indication of whether they
    // have providers. providerCount is count of number of entries that
    // actually have providers. Others are subclasses without providers.
    //
    ProviderInfoList providerInfos = _lookupAllInstanceProviders(
        request->nameSpace, request->className);

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED.
    // KS_PULL_QUESTION The following say if no provider and no
    // repository return error. Aggregation(
    //            providerInfo.se
    // KS_TODO Should we not just return nothing. See bug 9893

    if (_rejectNoProvidersOrRepository(request, providerInfos))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    //
    //  Try to compile the filter. Get an exception only if the FQL
    //  filter compiler generates an exception.
    //
#ifdef PEGASUS_ENABLE_FQL
    bool filterResponse = false;
////  XCOUT << "Calling handleFQLQueryRequest " << endl;
////  FQLQueryExpressionRep* qx = handleFQLQueryRequest(request);
////  if (qx == NULL)
////  {
////      // exception generated
////      XCOUT << "Exception generated " << endl;
////      return true;
////  }
////
////  XCOUT << "Called handleFQLQueryRequest " << endl;
////  printf("pointer %p\n",qx->_stmt);
////
////  FQLQueryStatement* qs = qx->_stmt;
////
////  XCOUT << "Called handleFQLQueryRequest " << endl;
    AutoPtr<FQLQueryExpressionRep> qx;
    AutoPtr<FQLQueryStatement> queryStatement;

    if (request->filterQueryLanguage.size() != 0)
    {
        bool exception = false;
        CIMException cimException;
        if (request->filterQueryLanguage != "DMTF:FQL")
        {
            cimException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED,
                request->filterQueryLanguage);
            exception = true;
        }
        else
        {
            try
            {
                queryStatement.reset(new FQLQueryStatement());
                FQLParser::parse(request->filterQuery, *queryStatement);

                qx.reset(new FQLQueryExpressionRep(request->filterQueryLanguage,
                                               queryStatement.get() ));
                filterResponse = true;
            }
            catch (ParseError& e)
            {
                String text = request->filterQuery + " " + e.getMessage();

                cimException =
                    PEGASUS_CIM_EXCEPTION(
                        CIM_ERR_INVALID_QUERY, text);
                exception=true;
            }
            catch (...)
            {
                exception=true;
            }
        }
        if (exception)
        {
            CIMResponseMessage* response = request->buildResponse();
            response->cimException = cimException;

            _enqueueResponse(request, response);
            PEG_METHOD_EXIT();
            return true;
        }
    }
#endif

    //
    // Setup the EnumerationContext. Returns pointer to object
    //
    EnumerationContext* enumerationContext =
        _enumerationContextTable->createContext(
            request,
            CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE,
            CIMResponseData::RESP_INSTANCES);

    if (enumerationContext == NULL)
    {
        _rejectCreateContextFailed(request);
        return true;
    }

    // Set properties from request into the context
    enumerationContext->setRequestProperties(
        request->includeClassOrigin, request->propertyList);

    // This trace connects the request parameters to the
    // ContextId which can be used to connect traces together.
    // This adds ContextId
    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "OpenEnumerateInstances request started "
            "ContextId=%s namespace=%s class=%s "
            "operationTimeout=%s "
            "maxObjectCount=%u",
        CSTRING(enumerationContext->getContextId()),
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString()),
        CSTRING(request->operationTimeout.toString()),
        request->maxObjectCount));

    // Build a corresponding EnumerateInstancesRequest to send to
    // providers. Do not pass the Pull operations request
    // on to Providers; create EnumerateInstancesRequest message to
    // activate providers.
    // NOTE: includeQualifiers NOT part of Pull operation so force to
    // false
    // The messageID is set to the enumerationContext contextID to
    // allow the ProviderAgentContainer to identify messages associated
    // with a particular enumerationSequence when called by the
    // idleTimeCleanup functions.

    CIMEnumerateInstancesRequestMessage* internalRequest =
        new CIMEnumerateInstancesRequestMessage(
            enumerationContext->getContextId(),
            request->nameSpace,
            request->className,
            request->deepInheritance,
            false,                       // include qualifiers false
            request->includeClassOrigin,
            request->propertyList,
            request->queueIds,
            request->authType,
            request->userName);

    internalRequest->operationContext = request->operationContext;
    internalRequest->internalOperation = true;

    // AutoPtr to delete internalRequest at end of handler
    AutoPtr<CIMEnumerateInstancesRequestMessage>
         requestDestroyer(internalRequest);

    // Set up an aggregate object and save a copy of the original request.
    // NOTE: Build the poA for the EnumerateRequest, not the corresponding
    // pull operation.

    OperationAggregate* poA= new OperationAggregate(
        new CIMEnumerateInstancesRequestMessage(*internalRequest),
        internalRequest->className,
        internalRequest->nameSpace,
        providerInfos.providerCount,
        true,       // completeHostAndNamespace = false
        true);      // _hasPropList = true

    //
    // Set Open... operation parameters into the operationAggregate
    //
    poA->setPullOperation(enumerationContext);

#ifdef PEGASUS_ENABLE_FQL
    if (filterResponse)
    {
        queryStatement.release();
        poA ->setFilterParameters(qx.release(),
            request->filterQueryLanguage);
    }
#endif
    //
    // If repository as instance provider is enabled, get instances
    // from the repository
    //

    if (_enumerateFromRepository(internalRequest, poA, providerInfos))
    {
        CIMResponseMessage* response = poA->removeResponse(0);
        // Forward the Enum response.  It will be mapped into
        // the EnumerateionContext cache.
        _forwardResponseForAggregation(
            new CIMEnumerateInstancesRequestMessage(*internalRequest),
            poA, response);
    }

    // Issue requests to providers before we send open response to get
    // provider responses before we build response.  This required to allow
    // building initial response of max requested size
    ProviderRequests::issueEnumerationRequestsToProviders(
        this,
        internalRequest,
        providerInfos,
        poA,
        "openEnumerateIntances");

    // Cannot use Operation Aggregate after this point since we are not
    // sure when it might be deleted unless the providerCount is != 0
    //
    // Issue the Response to the Open Request. This may be issued immediatly
    // or delayed by setting information into the enumeration context
    // if there are no responses from providers ready
    // to avoid issuing empty responses.  If there are no responses expected
    // at all from any source, the openResponse will be issued immediatly.

    CIMOpenEnumerateInstancesResponseMessage* openResponse(
        dynamic_cast<CIMOpenEnumerateInstancesResponseMessage*>(
            request->buildResponse()));

    // Enumeration Context must not be used after this call
    bool releaseRequest = issueOpenOrPullResponseMessage(
       request,
       openResponse,
       enumerationContext,
       operationMaxObjectCount,
       requireCompleteResponses);

    PEG_METHOD_EXIT();
    return releaseRequest;
}

/**$**************************************************************************
**
**    handleOpenEnumerateInstancePathsRequest
**
*****************************************************************************/

bool CIMOperationRequestDispatcher::handleOpenEnumerateInstancePathsRequest(
    CIMOpenEnumerateInstancePathsRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::"
            "handleOpenEnumerateInstancePathsRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "OpenEnumerateInstancePaths request namespace=%s class %s "
            "filterQueryLanguage=%s "
            "filterQuery=%s "
            "operationTimeout=%s "
            "continueOnError=%s "
            "maxObjectCount=%u",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->className.getString()),
        CSTRING(request->filterQueryLanguage),
        CSTRING(request->filterQuery),
        CSTRING(request->operationTimeout.toString()),
        boolToString(request->continueOnError),
        request->maxObjectCount ));

    // get the class name
    // FUTURE(KS) can we eliminate this completely??
    CIMName className = request->className;
    CIMConstClass cimClass;
    if (_rejectInvalidClassParameter(request, cimClass))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // Test the common open request input parameters and reject if any
    // are incorrect. Tests filterQuery, continueOnError, operationTimeout
    // and maxObjectCount
    Uint32 operationMaxObjectCount;
    if (_rejectIfPullParametersFailTests(request,operationMaxObjectCount,false))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Get names of descendent classes and list of providers
    // For Enumerate Operations, the list of providers is used for both
    // repository and provider requests.
    //

    ProviderInfoList providerInfos = _lookupAllInstanceProviders(
        request->nameSpace, request->className);

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED.
    // KS_PULL_QUESTION The following says if no provider and no
    // repository return error.  Should we not just return
    // nothing.

    if (_rejectNoProvidersOrRepository(request, providerInfos))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Setup the EnumerationContext. Returns pointer to object
    //
    EnumerationContext* enumerationContext =
        _enumerationContextTable->createContext(
            request,
            CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE,
            CIMResponseData::RESP_INSTNAMES);

    if (enumerationContext == NULL)
    {
        _rejectCreateContextFailed(request);
        PEG_METHOD_EXIT();
        return true;
    }


   // Build a corresponding EnumerateInstancesNamesRequest to send to
    // providers. We do not pass the Pull operations request
    // on to Providers but use the EnumerateInstancesRequest message to
    // activate providers.
    // NOTE: includeQualifiers NOT part of Pull operation

    CIMEnumerateInstanceNamesRequestMessage* internalRequest =
        new CIMEnumerateInstanceNamesRequestMessage(
            enumerationContext->getContextId(),
            request->nameSpace,
            request->className,
            request->queueIds,
            request->authType,
            request->userName);

    internalRequest->operationContext = request->operationContext;

    internalRequest->internalOperation = true;

    // AutoPtr to delete at end of handler
    AutoPtr<CIMEnumerateInstanceNamesRequestMessage> dummy(internalRequest);

    //
    // Set up op aggregate object and save a copy of the original request.
    // NOTE: Build the poA for the EnumerateRequest, not the corresponding
    // pull operation.
    //

    OperationAggregate* poA= new OperationAggregate(
        new CIMEnumerateInstanceNamesRequestMessage(*internalRequest),
        internalRequest->className,
        internalRequest->nameSpace,
        providerInfos.providerCount,
        true,           // completeHostAndNamespace = false
        false);         // No property List for this operation
    //
    // Set Open... operation parameters into the operationAggregate
    // Includes setting namespace that is used to complete host and naespace
    // in responses
    //
    poA->setPullOperation(enumerationContext);

    // gather up the repository responses and send it to out as one response
    // with many instances
    //
    if (_enumerateFromRepository(internalRequest, poA, providerInfos))
    {
        CIMResponseMessage* response = poA->removeResponse(0);
        _forwardResponseForAggregation(
            new CIMEnumerateInstanceNamesRequestMessage(*internalRequest),
            poA, response);
    }

    ProviderRequests::issueEnumerationRequestsToProviders(
        this,
        internalRequest,
        providerInfos,
        poA,
        "openEnumerateInstanceNames");

    // Cannot use Operation Aggregate after this point since we are not
    // sure when it might be deleted unless the providerCount is != 0
    //
    // Issue the Response to the Open Request. This may be issued immediatly
    // or delayed by setting information into the enumeration context
    // if there are no responses from providers ready
    // to avoid issuing empty responses.  If there are no responses expected
    // at all from any source, the openResponse will be issued immediatly.

    CIMOpenEnumerateInstancePathsResponseMessage* openResponse(
        dynamic_cast<CIMOpenEnumerateInstancePathsResponseMessage*>(
            request->buildResponse()));

    // Enumeration Context must not be used after this call
    bool releaseRequest = issueOpenOrPullResponseMessage(
       request,
       openResponse,
       enumerationContext,
       operationMaxObjectCount,
       requireCompleteResponses);

    PEG_METHOD_EXIT();
    return releaseRequest;
}

/**$*******************************************************
    handleOpenReferenceInstancesRequest
**********************************************************/

bool CIMOperationRequestDispatcher::handleOpenReferenceInstancesRequest(
    CIMOpenReferenceInstancesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleOpenReferenceInstancesRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "OpenReferenceInstances request namespace=%s object=%s "
            "resultClass=%s "
            "role=%s "
            "includeClassOrigin=%s "
            "propertyList=%s "
            "filterQueryLanguage=%s "
            "filterQuery=%s "
            "operationTimeout=%s "
            "continueOnError=%s "
            "maxObjectCount=%u",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->objectName.toString()),
        CSTRING(request->resultClass.getString()),
        CSTRING(request->role),
        boolToString(request->includeClassOrigin),
        CSTRING(request->propertyList.toString()),
        CSTRING(request->filterQueryLanguage),
        CSTRING(request->filterQuery),
        CSTRING(request->operationTimeout.toString()),
        boolToString(request->continueOnError),
        request->maxObjectCount ));

    //  validate className and objectClassName same.  This is
    //  an invariant for the handleAssoc, etc functions.
    PEGASUS_DEBUG_ASSERT(
        request->className == request->objectName.getClassName());

    if (_rejectAssociationTraversalDisabled(request,"OpenReferenceInstances"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidRoleParameter(request, request->role, "role"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidObjectPathParameter(request, request->objectName))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // Test the common open request input parameters and reject if any
    // are incorrect. Tests filterQuery, continueOnError, operationTimeout
    // and maxObjectCount
    Uint32 operationMaxObjectCount;
    if (_rejectIfPullParametersFailTests(request,operationMaxObjectCount,
                                         ENABLE_FQL_FILTER))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // KS_TODO - Should we use checkClassParameter here in place of
    // _checkExistence...

    if (!_checkExistenceOfClass(
            request->nameSpace, request->objectName.getClassName()))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_PARAMETER,
            request->objectName.getClassName().getString());
    }

    //
    //  Try to compile the filter. Get an exception only if the FQL
    //  filter compiler generates an exception.
    //  Result saved in OperationAggregate later in process
    //
#ifdef PEGASUS_ENABLE_FQL
    bool filterResponse = false;
////  FQLQueryExpressionRep* qx = handleFQLQueryRequest(request);
////  if (qx == NULL)
////  {
////      // exception generated
////      XCOUT << "Exception generated " << endl;
////      return true;
////  }
    AutoPtr<FQLQueryExpressionRep> qx;
    AutoPtr<FQLQueryStatement> queryStatement;
    if (request->filterQueryLanguage.size() != 0)
    {
        bool exception = false;
        CIMException cimException;
        if (request->filterQueryLanguage != "DMTF:FQL")
        {
            cimException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED,
                request->filterQueryLanguage);
            exception = true;
        }
        else
        {
            if (request->filterQuery.size() != 0)
            {
                try
                {
                    queryStatement.reset(new FQLQueryStatement());
                    FQLParser::parse(request->filterQuery, *queryStatement);
                    qx.reset(new FQLQueryExpressionRep(
                        request->filterQueryLanguage,queryStatement.get() ));
                    filterResponse = true;
                }
                catch (ParseError& e)
                {
                    String text = request->filterQuery + " " + e.getMessage();

                    cimException =
                        PEGASUS_CIM_EXCEPTION(
                            CIM_ERR_INVALID_QUERY, text);
                    exception=true;
                }
                catch (...)
                {
                    exception=true;
                }
            }
        }
        if (exception)
        {
            CIMResponseMessage* response = request->buildResponse();
            response->cimException = cimException;

            _enqueueResponse(request, response);
            PEG_METHOD_EXIT();
            return true;
        }
    }
#endif

    ProviderInfoList providerInfos = _lookupAllAssociationProviders(
        request->nameSpace,
        request->objectName,
        request->resultClass,
        String::EMPTY);

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED

    if (_rejectNoProvidersOrRepository(request, providerInfos))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Request the instances from the repository, as necessary.
    // Instances go directly into a response because the are returned as
    // a single array.
    //

    Array<CIMObject> cimObjects;
    if (_repository->isDefaultInstanceProvider())
    {
        cimObjects = _repository->references(
            request->nameSpace,
            request->objectName,
            request->resultClass,
            request->role,
            false,                       // force includeQualifiers to false
            request->includeClassOrigin,
            request->propertyList);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "OpenReferenceInstances repository access: class = %s, count = %u.",
                (const char*)request->objectName.toString().getCString(),
                cimObjects.size()));
        // FUTURE - Why not just put this directly to the enumcache.
        // If there was an exception is is caught outside of this and
        // not sure any of the other checking on response really gains
        // anything.
    }

    // Build Open Response message. We are going to issue some sort of
    // response whether there are providers, repository returns or not.
    AutoPtr<CIMOpenReferenceInstancesResponseMessage> openResponse;
    openResponse.reset(dynamic_cast<CIMOpenReferenceInstancesResponseMessage*>(
        request->buildResponse()));

    // If there are no providers and nothing from repository
    // return empty and exit
    // FUTURE(KS) We should be able to drop through to normal code
    // and drop this completely.

    if ((providerInfos.providerCount == 0) && (cimObjects.size() == 0))
    {
        // We have no providers to call and nothing from repository.
        // Return empty

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "Repository Empty, NO providers for %s",
            (const char *)request->className.getString().getCString()));

        openResponse->endOfSequence = true;
        _enqueueResponse(request, openResponse.release());

        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Process as multi operation enumeration. 1. Build context, build
    // internalRequest for providers and issue to providers, build open
    // response, get current objects (with wait) and enqueue open
    // response.
    //

    // Create new context object.
    EnumerationContext* enumerationContext =
        _enumerationContextTable->createContext(
            request,
            CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE,
            CIMResponseData::RESP_OBJECTS);

    if (enumerationContext == NULL)
    {
        _rejectCreateContextFailed(request);
        PEG_METHOD_EXIT();
        return true;
    }

    enumerationContext->setRequestProperties(
        request->includeClassOrigin, request->propertyList);

    // Build corresponding CIMReferencesRequestMessage to issue to
    // providers. We do not pass the Open operations request
    // on to Providers but use the EnumerateInstancesRequest message to
    // activate providers. NOTE: includeQualifiers NOT part of Pull operation

    CIMReferencesRequestMessage* internalRequest =
        new CIMReferencesRequestMessage(
            enumerationContext->getContextId(),
            request->nameSpace,
            request->objectName,
            request->resultClass,
            request->role,
            false,              // force includeQualifiers to false
            request->includeClassOrigin,
            request->propertyList,
            request->queueIds,
            false,              // Not a class request
            request->authType,
            request->userName);

    internalRequest->operationContext = request->operationContext;
    internalRequest->internalOperation = true;

    // AutoPtr to delete at end of handler
    AutoPtr<CIMReferencesRequestMessage> requestDesroyer(internalRequest);

    //
    // Set up an aggregate object and save the original request message
    //
    OperationAggregate* poA = new OperationAggregate(
        new CIMReferencesRequestMessage(*internalRequest),
        internalRequest->objectName.getClassName(),
        internalRequest->nameSpace,
        providerInfos.providerCount,
        true,   // requiresHostnameCompletion
        true);  // hasPropList

    //
    // Set Open... operation parameters into the operationAggregate
    // since operationAggregate is what is  returned from providers.
    //
    poA->setPullOperation(enumerationContext);

#ifdef PEGASUS_ENABLE_FQL
    if (filterResponse)
    {
        queryStatement.release();
        poA ->setFilterParameters(qx.release(),
           request->filterQueryLanguage);
    }
#endif

    // If response from repository not empty, forward for aggregation.
    if (cimObjects.size() != 0)
    {
        AutoPtr<CIMReferencesResponseMessage> internalResponse;
        internalResponse.reset(dynamic_cast<CIMReferencesResponseMessage*>(
            internalRequest->buildResponse()));
        internalResponse->getResponseData().setObjects(cimObjects);

        poA->incTotalIssued();

        // Send the repository results for aggregation
        // directly to callback (includes response).
        //
        _forwardResponseForAggregation(
            new CIMReferencesRequestMessage(*internalRequest),
            poA,
            internalResponse.release());
    }

    ProviderRequests::issueOpenAssocRequestMsgsToProviders(
        this,
        internalRequest,
        providerInfos,
        poA,
        "OpenReferenceInstances"
        );

    // Enumeration Context must not be used after this call
    bool releaseRequest = issueOpenOrPullResponseMessage(
       request,
       openResponse.release(),
       enumerationContext,
       operationMaxObjectCount,
       requireCompleteResponses);

    PEG_METHOD_EXIT();
    return releaseRequest;
}

/**$*******************************************************
    handleOpenReferenceInstancePathsRequest
**********************************************************/

bool CIMOperationRequestDispatcher::handleOpenReferenceInstancePathsRequest(
    CIMOpenReferenceInstancePathsRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
      "CIMOperationRequestDispatcher::handleOpenReferenceInstancePathsRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "OpenReferenceInstancePaths request for Namespace=%s objectName %s "
            "resultClass=%s "
            "role=%s "
            "filterQueryLanguage=%s "
            "filterQuery=%s "
            "operationTimeout=%s "
            "continueOnError=%s "
            "maxObjectCount=%u",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->objectName.toString()),
        CSTRING(request->resultClass.getString()),
        CSTRING(request->role),
        CSTRING(request->filterQueryLanguage),
        CSTRING(request->filterQuery),
        CSTRING(request->operationTimeout.toString()),
        boolToString(request->continueOnError),
        request->maxObjectCount ));

    //  validate className and objectClassName same.  This is
    //  an invariant for the handleAssoc, etc functions.
    PEGASUS_DEBUG_ASSERT(
        request->className == request->objectName.getClassName());

    if (_rejectAssociationTraversalDisabled(request,
        "OpenReferenceInstancePaths"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidRoleParameter(request, request->role, "role"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidObjectPathParameter(request, request->objectName))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // Test the common open request input parameters and reject if any
    // are incorrect. Tests filterQuery, continueOnError, operationTimeout
    // and maxObjectCount
    Uint32 operationMaxObjectCount;
    if (_rejectIfPullParametersFailTests(request,operationMaxObjectCount,false))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (!_checkExistenceOfClass(request->nameSpace,
                                request->objectName.getClassName()))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_PARAMETER,
            request->objectName.getClassName().getString());
    }

    //
    // Get results from providers and the repository
    // Determine list of providers for this request
    //

    ProviderInfoList providerInfos = _lookupAllAssociationProviders(
            request->nameSpace,
            request->objectName,
            request->resultClass,
            String::EMPTY);

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED

    if (_rejectNoProvidersOrRepository(request, providerInfos))
    {
        PEG_METHOD_EXIT();
        return true;
    }
    //
    // Get the instances from the repository, as necessary
    //
    Array<CIMObjectPath> instanceNames;
    if (_repository->isDefaultInstanceProvider())
    {
        instanceNames = _repository->referenceNames(
            request->nameSpace,
            request->objectName,
            request->resultClass,
            request->role);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "OpenReferenceInstamcePaths repository access:"
                   " class = %s, count = %u.",
            (const char*)request->objectName.toString().getCString(),
            instanceNames.size()));
    }
    // Build Open Response message.
    AutoPtr<CIMOpenReferenceInstancePathsResponseMessage> openResponse;
    openResponse.reset(
        dynamic_cast<CIMOpenReferenceInstancePathsResponseMessage*>(
            request->buildResponse()));

    if (providerInfos.providerCount == 0 && instanceNames.size() == 0)
    {
        //
        // No provider is registered and the repository isn't the
        // default.  Return CIM_ERR_NOT_SUPPORTED.

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "OpenReferenceInstancePaths Return Empty."));

        openResponse->endOfSequence = true;
        _enqueueResponse(request, openResponse.release());

        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Create new enumerationContext
    //
    EnumerationContext* enumerationContext =
        _enumerationContextTable->createContext(
            request,
            CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE,
            CIMResponseData::RESP_OBJECTPATHS);

    if (enumerationContext == NULL)
    {
        _rejectCreateContextFailed(request);
        PEG_METHOD_EXIT();
        return true;
    }

    // Build corresponding EnumerateInstancePathsRequest to send to
    // providers. We do not pass the Pull operations request
    // on to Providers but use the EnumerateInstancesRequest message to
    // activate providers. NOTE: includeQualifiers NOT part of Pull operation

    CIMReferenceNamesRequestMessage* internalRequest =
        new CIMReferenceNamesRequestMessage(
            enumerationContext->getContextId(),
            request->nameSpace,
            request->objectName,
            request->resultClass,
            request->role,
            request->queueIds,
            false,
            request->authType,
            request->userName);

    internalRequest->operationContext = request->operationContext;

    internalRequest->internalOperation = true;

    // AutoPtr to delete at end of handler
    AutoPtr<CIMReferenceNamesRequestMessage>
        requestDestroyer(internalRequest);

    //
    // Set up an aggregate object and save the created internalRequest
    //
    OperationAggregate* poA = new OperationAggregate(
        new CIMReferenceNamesRequestMessage(*internalRequest),
        internalRequest->objectName.getClassName(),
        internalRequest->nameSpace,
        providerInfos.providerCount,
        true,          // completeHostAndNamespace = false
        false);        // No property list for this operation

    //
    // Set Open... operation parameters into the operationAggregate
    //
    poA->setPullOperation(enumerationContext);

    // If any return from repository, send it to aggregator.
    // Send repository response for aggregation
    if (instanceNames.size() != 0)
    {
        AutoPtr<CIMReferenceNamesResponseMessage> internalResponse;
        internalResponse.reset(dynamic_cast<CIMReferenceNamesResponseMessage*>(
            internalRequest->buildResponse()));

        internalResponse->getResponseData().setInstanceNames(instanceNames);

        poA->incTotalIssued();
        // send the repository's results for aggregation
        // directly to callback (includes response).
        _forwardResponseForAggregation(
            new CIMReferenceNamesRequestMessage(*internalRequest),
            poA,
            internalResponse.release());
    }

     ProviderRequests::issueOpenAssocRequestMsgsToProviders(
        this,
        internalRequest,
        providerInfos,
        poA,
        "OpenReferenceInstancePaths"
        );

    // Issue the Response to the Open Request. This may be issued immediatly
    // or delayed by setting information into the enumeration context
    // if there are no responses from providers ready
    // to avoid issuing empty responses.
    // Enumeration Context must not be used after this call

    bool releaseRequest = issueOpenOrPullResponseMessage(
       request,
       openResponse.release(),
       enumerationContext,
       operationMaxObjectCount,
       requireCompleteResponses);

    PEG_METHOD_EXIT();
    return releaseRequest;
}


/**$*******************************************************
    handleOpenAssociatorInstancesRequest
**********************************************************/

bool CIMOperationRequestDispatcher::handleOpenAssociatorInstancesRequest(
    CIMOpenAssociatorInstancesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleOpenAssociatorInstancesRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "OpenAssociatorInstancesRequest request namespace=%s object=%s "
            "assocClass = \"%s\"  "
            "resultClass=%s "
            "role=%s "
            "resultRole = \"%s\"  "
            "includeClassOrigin=%s "
            "propertyList=%s "
            "filterQueryLanguage=%s "
            "filterQuery=%s "
            "operationTimeout=%s "
            "continueOnError=%s "
            "maxObjectCount=%u",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->objectName.toString()),
        CSTRING(request->assocClass.getString()),
        CSTRING(request->resultClass.getString()),
        CSTRING(request->role),
        CSTRING(request->resultClass.getString()),
        boolToString(request->includeClassOrigin),
        CSTRING(request->propertyList.toString()),
        CSTRING(request->filterQueryLanguage),
        CSTRING(request->filterQuery),
        CSTRING(request->operationTimeout.toString()),
        boolToString(request->continueOnError),
        request->maxObjectCount ));

    //  validate className and objectClassName same.  This is
    //  an invariant for the handleAssoc, etc functions.
    PEGASUS_DEBUG_ASSERT(
        request->className == request->objectName.getClassName());

    if (_rejectAssociationTraversalDisabled(request, "OpenAssociatorInstances"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidRoleParameter(request, request->role, "role"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidRoleParameter(request, request->resultRole, "resultRole"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidObjectPathParameter(request, request->objectName))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // Test the common open request input parameters and reject if any
    // are incorrect. Tests filterQuery, continueOnError, operationTimeout
    // and maxObjectCount
    Uint32 operationMaxObjectCount;
    if (_rejectIfPullParametersFailTests(request,operationMaxObjectCount,
                                         ENABLE_FQL_FILTER))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (!_checkExistenceOfClass(
            request->nameSpace, request->objectName.getClassName()))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_PARAMETER, request->objectName.toString());
    }

    //
    //  Try to compile the filter. Get an exception only if the FQL
    //  filter compiler generates an exception.
    //
#ifdef PEGASUS_ENABLE_FQL
    bool filterResponse = false;
    AutoPtr<FQLQueryExpressionRep> qx;
    AutoPtr<FQLQueryStatement> queryStatement;
    if (request->filterQueryLanguage.size() != 0)
    {
        bool exception = false;
        CIMException cimException;
        if (request->filterQueryLanguage != "DMTF:FQL")
        {
            cimException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED,
                request->filterQueryLanguage);
            exception = true;
        }
        else
        {
            if (request->filterQuery.size() != 0)
            {
                try
                {
                    queryStatement.reset(new FQLQueryStatement());
                    FQLParser::parse(request->filterQuery, *queryStatement);
                    qx.reset(new FQLQueryExpressionRep(
                        request->filterQueryLanguage, queryStatement.get() ));
                    filterResponse = true;
                }
                catch (ParseError& e)
                {
                    String text = request->filterQuery + " " + e.getMessage();

                    cimException =
                        PEGASUS_CIM_EXCEPTION(
                            CIM_ERR_INVALID_QUERY, text);
                    exception=true;
                }
                catch (...)
                {
                    exception=true;
                }
            }
        }
        if (exception)
        {
            CIMResponseMessage* response = request->buildResponse();
            response->cimException = cimException;

            _enqueueResponse(request, response);
            PEG_METHOD_EXIT();
            return true;
        }
    }
#endif

    ProviderInfoList providerInfos = _lookupAllAssociationProviders(
        request->nameSpace,
        request->objectName,
        request->assocClass,
        String::EMPTY);

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED

    if (_rejectNoProvidersOrRepository(request, providerInfos))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Get the instances from the repository, as necessary.
    // Hold the repository results in a response message.
    // If not using the repository, this pointer is null.
    //
    Array<CIMObject> cimObjects;
    if (_repository->isDefaultInstanceProvider())
    {
        cimObjects = _repository->associators(
            request->nameSpace,
            request->objectName,
            request->assocClass,
            request->resultClass,
            request->role,
            request->resultRole,
            false,                      // force includeQualifiers to false
            request->includeClassOrigin,
            request->propertyList);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "OpenAssociatorInstances repository access: class = %s,"
                " count = %u.",
                CSTRING(request->objectName.toString()),
                cimObjects.size() ));
    }

    // Build OpenResponse Message
    AutoPtr<CIMOpenAssociatorInstancesResponseMessage> openResponse;
    openResponse.reset(dynamic_cast<CIMOpenAssociatorInstancesResponseMessage*>(
        request->buildResponse()));

    // if there are no providers and nothing from repository
    // return with endOfSequence true. Do direct enqueue response.
    //
    // // KS TODO can we completely bypass this code and just use
    // the main delivery path even if the repository returns nothing?

    if ((providerInfos.providerCount == 0) && (cimObjects.size() == 0))
    {
        //
        // We have no providers to call.  Just return what we have.
        //

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "OpenAssociatorInstances. No Objects found for %s",
            (const char *)request->className.getString().getCString()));

        openResponse->endOfSequence = true;
        _enqueueResponse(request, openResponse.release());

        PEG_METHOD_EXIT();
        return true;
    }

    //
    // Process as multi operation enumeration. 1. Build context, build
    // internalRequest for providers and issue to providers, build open
    // response, get current objects (with wait) and enqueue open
    // response.
    //
    // Create a new enumeration context
    EnumerationContext* enumerationContext =
        _enumerationContextTable->createContext(
            request,
            CIM_PULL_INSTANCES_WITH_PATH_REQUEST_MESSAGE,
            CIMResponseData::RESP_OBJECTS);

    if (enumerationContext == NULL)
    {
        _rejectCreateContextFailed(request);
        PEG_METHOD_EXIT();
        return true;
    }

    enumerationContext->setRequestProperties(
        request->includeClassOrigin, request->propertyList);

    // Build corresponding CIMReferencesRequestMessage to issue to
    // providers. We do not pass the Pull operations request
    // on to Providers but use the EnumerateInstancesRequest message to
    // activate providers. NOTE: includeQualifiers NOT part of Pull operation

    CIMAssociatorsRequestMessage* internalRequest =
        new CIMAssociatorsRequestMessage(
            enumerationContext->getContextId(),
            request->nameSpace,
            request->objectName,
            request->assocClass,
            request->resultClass,
            request->role,
            request->resultRole,
            false,          // force includeQualifiers to false
            request->includeClassOrigin,
            request->propertyList,
            request->queueIds,
            false,          // By definition not a class request
            request->authType,
            request->userName);

    internalRequest->operationContext = request->operationContext;

    internalRequest->internalOperation = true;

    // AutoPtr to delete at end of handler
    AutoPtr<CIMAssociatorsRequestMessage> requestDestroyer(internalRequest);

    //
    // Set up an aggregate object and save the original request message
    //

    OperationAggregate* poA = new OperationAggregate(
        new CIMAssociatorsRequestMessage(*internalRequest),
        internalRequest->objectName.getClassName(),
        internalRequest->nameSpace,
        providerInfos.providerCount,
        true,             // completeHostAndNamespace = false
        true);            // Operation has property list

    //
    // Set Open... operation parameters into the operationAggregate
    //
    poA->setPullOperation(enumerationContext);

#ifdef PEGASUS_ENABLE_FQL
    if (filterResponse)
    {
        queryStatement.release();
        poA ->setFilterParameters(qx.release(),
            request->filterQueryLanguage);
    }
#endif

    // Send repository response for aggregation

    if (cimObjects.size() != 0)
    {
        AutoPtr<CIMAssociatorsResponseMessage> internalResponse;
        internalResponse.reset(dynamic_cast<CIMAssociatorsResponseMessage*>(
            internalRequest->buildResponse()));

        internalResponse->getResponseData().setObjects(cimObjects);

        poA->incTotalIssued();

        // send the repository's results for aggregation
        // directly to callback (i.e. includes response).

        _forwardResponseForAggregation(
            new CIMAssociatorsRequestMessage(*internalRequest),
            poA,
            internalResponse.release());
    }

    ProviderRequests::issueOpenAssocRequestMsgsToProviders(
        this,
        internalRequest,
        providerInfos,
        poA,
        "OpenAssociatorInstances"
        );

    // Issue the Response to the Open Request. This may be issued immediatly
    // or delayed by setting information into the enumeration context
    // if there are no responses from providers ready
    // to avoid issuing empty responses.

    // Enumeration Context must not be used after this call
    bool releaseRequest = issueOpenOrPullResponseMessage(
       request,
       openResponse.release(),
       enumerationContext,
       operationMaxObjectCount,
       requireCompleteResponses);

    PEG_METHOD_EXIT();
    return releaseRequest;
}

/**$*******************************************************
    handleOpenAssociatorInstancePathsRequest
**********************************************************/

bool CIMOperationRequestDispatcher::handleOpenAssociatorInstancePathsRequest(
    CIMOpenAssociatorInstancePathsRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::"
        "handleOpenAssociatorInstancePathsRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "OpenAssociatorInstancePaths request namespace=%s object=%s "
            "assocClass = %s "
            "resultClass=%s "
            "role=%s "
            "resultRole = %s "
            "filterQueryLanguage=%s "
            "filterQuery=%s "
            "operationTimeout=%s "
            "continueOnError=%s "
            "maxObjectCount=%u",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->objectName.toString()),
        CSTRING(request->assocClass.getString()),
        CSTRING(request->resultClass.getString()),
        CSTRING(request->role),
        CSTRING(request->resultRole),
        CSTRING(request->filterQueryLanguage),
        CSTRING(request->filterQuery),
        CSTRING(request->operationTimeout.toString()),
        boolToString(request->continueOnError),
        request->maxObjectCount ));

    //  validate className and objectClassName same.  This is
    //  an invariant for the handleAssoc, etc functions.
    PEGASUS_DEBUG_ASSERT(
        request->className == request->objectName.getClassName());

    if (_rejectAssociationTraversalDisabled(request,
        "OpenAssociatorInstancePaths"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidRoleParameter(request, request->role, "role"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidRoleParameter(request, request->resultRole, "roleRole"))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (_rejectInvalidObjectPathParameter(request, request->objectName))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // Test the common open request input parameters and reject if any
    // are incorrect. Tests filterQuery, continueOnError, operationTimeout
    // and maxObjectCount
    Uint32 operationMaxObjectCount;
    if (_rejectIfPullParametersFailTests(request,operationMaxObjectCount,false))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    if (!_checkExistenceOfClass(request->nameSpace,
                                request->objectName.getClassName()))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_PARAMETER,
            request->objectName.getClassName().getString());
    }

    //
    // Get results from providers and the repository
    // Determine list of providers for this request
    //

    ProviderInfoList providerInfos = _lookupAllAssociationProviders(
            request->nameSpace,
            request->objectName,
            request->resultClass,
            String::EMPTY);

    // If no provider is registered and the repository isn't the default,
    // return CIM_ERR_NOT_SUPPORTED

    if (_rejectNoProvidersOrRepository(request, providerInfos))
    {
        PEG_METHOD_EXIT();
        return true;
    }
    //
    // Get the instances from the repository, as necessary
    //
    Array<CIMObjectPath> objectNames;
    if (_repository->isDefaultInstanceProvider())
    {
        objectNames = _repository->associatorNames(
            request->nameSpace,
            request->objectName,
            request->assocClass,
            request->resultClass,
            request->role,
            request->resultRole);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "OpenAssociatorInstancePaths repository access: class = %s,"
            " count = %u.",
            (const char*)request->objectName.toString().getCString(),
            objectNames.size()));
    }

    // Build OpenResponse Message
    AutoPtr<CIMOpenAssociatorInstancePathsResponseMessage> openResponse;
    openResponse.reset(
        dynamic_cast<CIMOpenAssociatorInstancePathsResponseMessage*>(
            request->buildResponse()));

    if (providerInfos.providerCount == 0 && objectNames.size() == 0)
    {
        //
        // No provider is registered and the repository isn't the
        // default.  Return CIM_ERR_NOT_SUPPORTED.
        //

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "OpenAssociatorInstancePaths. No Objects found for %s",
            CSTRING(request->className.getString()) ));

        openResponse->endOfSequence = true;
        _enqueueResponse(request, openResponse.release());

        PEG_METHOD_EXIT();
        return true;
    }
    //
    // Process as multi operation enumeration. 1. Build context, build
    // internalRequest for providers and issue to providers, build open
    // response, get current objects (with wait) and enqueue open
    // response.
    //
    // Create new enumerationContext and enumerationContextString.
    EnumerationContext* enumerationContext =
        _enumerationContextTable->createContext(
            request,
            CIM_PULL_INSTANCE_PATHS_REQUEST_MESSAGE,
            CIMResponseData::RESP_OBJECTPATHS);

    if (enumerationContext == NULL)
    {
        _rejectCreateContextFailed(request);
        PEG_METHOD_EXIT();
        return true;
    }

    // Build corresponding EnumerateInstancePathsRequest to send to
    // providers. We do not pass the Pull operations request
    // on to Providers but use the EnumerateInstancesRequest message to
    // activate providers. NOTE: includeQualifiers NOT part of Pull operation

    CIMAssociatorNamesRequestMessage* internalRequest =
        new CIMAssociatorNamesRequestMessage(
            enumerationContext->getContextId(),
            request->nameSpace,
            request->objectName,
            request->assocClass,
            request->resultClass,
            request->role,
            request->resultRole,
            request->queueIds,
            false,
            request->authType,
            request->userName);

    internalRequest->operationContext = request->operationContext;
    internalRequest->internalOperation = true;

    AutoPtr<CIMAssociatorNamesRequestMessage> requestDestroyer(internalRequest);

    //
    // Set up an aggregate object and save the created internalRequest
    //

    OperationAggregate* poA = new OperationAggregate(
        new CIMAssociatorNamesRequestMessage(*internalRequest),
        internalRequest->objectName.getClassName(),
        internalRequest->nameSpace,
        providerInfos.providerCount,
        true,                  // completeHostAndNamespace = false
        false);                // No property list for this operation

    //
    // Set Open... operation parameters into the operationAggregate
    //
    poA->setPullOperation(enumerationContext);

    // If any return from repository, send it to aggregator.
    if (objectNames.size() != 0)
    {
        AutoPtr<CIMAssociatorNamesResponseMessage> internalResponse;
        internalResponse.reset(dynamic_cast<CIMAssociatorNamesResponseMessage*>(
            internalRequest->buildResponse()));
        internalResponse->getResponseData().setInstanceNames(objectNames);

        poA->incTotalIssued();
        // send the repository's results for aggregation
        // directly to callback (includes response).
        _forwardResponseForAggregation(
            new CIMAssociatorNamesRequestMessage(*internalRequest),
            poA,
            internalResponse.release());
    }

    ProviderRequests::issueOpenAssocRequestMsgsToProviders(
        this,
        internalRequest,
        providerInfos,
        poA,
        "OpenAssociatorInstancePaths"
        );

    // Issue the Response to the Open Request. This may be issued immediatly
    // or delayed by setting information into the enumeration context
    // if there are no responses from providers ready
    // to avoid issuing empty responses.

    // Enumeration Context must not be used after this call
    bool releaseRequest = issueOpenOrPullResponseMessage(
       request,
       openResponse.release(),
       enumerationContext,
       operationMaxObjectCount,
       requireCompleteResponses);

    PEG_METHOD_EXIT();
    return releaseRequest;
}

/**$**************************************************************************
**
**    handleOpenQueryInstancesRequest
**
*****************************************************************************/

bool CIMOperationRequestDispatcher::handleOpenQueryInstancesRequest(
    CIMOpenQueryInstancesRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleOpenQueryInstancesRequest");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "OpenQueryInstances request namespace=%s "
            "queryLanguage=%s "
            "query=%s "
            "returnQueryResultClass=%s "
            "operationTimeout=%s "
            "continueOnError=%s "
            "maxObjectCount=%u ",
        CSTRING(request->nameSpace.getString()),
        CSTRING(request->queryLanguage),
        CSTRING(request->query),
        boolToString(request->returnQueryResultClass),
        CSTRING(request->operationTimeout.toString()),
        boolToString(request->continueOnError),
        request->maxObjectCount ));

    if (_rejectIfContinueOnError(request, request->continueOnError))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    Uint32 operationMaxObjectCount;
    if (_rejectInvalidMaxObjectCountParam(request, request->maxObjectCount,
            false, operationMaxObjectCount, Uint32(0)))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    // Test for valid values in OperationTimeout
    if (_rejectInvalidOperationTimeout(request, request->operationTimeout))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    AutoPtr<CIMOpenQueryInstancesResponseMessage> openResponse(
        dynamic_cast<CIMOpenQueryInstancesResponseMessage*>(
            request->buildResponse()));

#ifdef PEGASUS_DISABLE_EXECQUERY
    openResponse->cimException =
        PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);
    _enqueueResponse(request, openResponse.release());
    PEG_METHOD_EXIT();
    return true;
#endif

    //
    // Setup the EnumerationContext. Returns pointer to object
    // Objects are passed through the system (they are from ExecQuery)
    // and mapped to instances on output

    EnumerationContext* en =
        _enumerationContextTable->createContext(
            request,
            CIM_PULL_INSTANCES_REQUEST_MESSAGE,
            CIMResponseData::RESP_OBJECTS);

    if (en == NULL)
    {
        _rejectCreateContextFailed(request);
        PEG_METHOD_EXIT();
        return true;
    }

    // Create the corresponding CIMExecQueryRequestMessage
    CIMExecQueryRequestMessage* internalRequest =
        new CIMExecQueryRequestMessage(
            en->getContextId(),
            request->nameSpace,
            request->queryLanguage,
            request->query,
            request->queueIds,
            request->authType,
            request->userName);

    internalRequest->operationContext = request->operationContext;
    internalRequest->internalOperation = true;

    AutoPtr<CIMExecQueryRequestMessage> requestDestroyer(internalRequest);

    if (!QuerySupportRouter::routeHandleExecQueryRequest(
       this, internalRequest,openResponse->cimException, en))
    {
        _enqueueResponse(request, openResponse.release());
        PEG_METHOD_EXIT();
        return true;
    }

    // Issue the Response to the Open Request. This may be issued immediatly
    // or delayed by setting information into the enumeration context
    // if there are no responses from providers ready
    // to avoid issuing empty responses.

    // Enumeration Context must not be used after this call
    bool releaseRequest = issueOpenOrPullResponseMessage(
       request,
       openResponse.release(),
       en,
       operationMaxObjectCount,
       requireCompleteResponses);

    PEG_METHOD_EXIT();
    return releaseRequest;

}

void CIMOperationRequestDispatcher::handleEnumerationCount(
        CIMEnumerationCountRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleEnumerationCount");

    PEGASUS_STD(cout) << "CIMOpDispatcher::handleEnumerationCount class = "
        << request->className.getString()
         << " enumContext= " << request->enumerationContext
         << PEGASUS_STD(endl);

    // Determine if the enumerationContext exists

    EnumerationContext* en = _enumerationContextTable->find(
       request->enumerationContext);

    // test for invalid context and if found, error out.
    if (_rejectInvalidEnumerationContext(request, en))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // Issue Not supported error since, in general, we do
    // not support this request operation
    CIMResponseMessage* response = request->buildResponse();
    response->cimException =
        PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

    _enqueueResponse(request, response);

    PEG_METHOD_EXIT();
    return;
}
/**$********************************************************
    handlePullInstancesWithPath
************************************************************/

bool CIMOperationRequestDispatcher::handlePullInstancesWithPath(
    CIMPullInstancesWithPathRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handlePullInstancesWithPath");

    // Both pull operations execute off of a single templated function.
    // Build and send response.  getCache used to wait for objects
    CIMPullInstancesWithPathResponseMessage* response(
        dynamic_cast<CIMPullInstancesWithPathResponseMessage*>(
            request->buildResponse()));

    bool releaseRequest = processPullRequest(request,
                          response, "pullInstancesWithPath");

    PEG_METHOD_EXIT();
    return releaseRequest;
}
/**$********************************************************
    handlePullInstancesPaths
************************************************************/

bool CIMOperationRequestDispatcher::handlePullInstancePaths(
    CIMPullInstancePathsRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handlePullInstancePaths");

    // Both pull operations execute off of a single templated
    // function.
    // Build and send response.  getCache used to wait for objects
    CIMPullInstancePathsResponseMessage* response(
        dynamic_cast<CIMPullInstancePathsResponseMessage*>(
            request->buildResponse()));

    bool releaseRequest = processPullRequest(request, response,
                          "pullInstancePaths");

    PEG_METHOD_EXIT();
    return releaseRequest;
}

/**$********************************************************
    handlePullInstances - Part of the OpenQueryInstances
          enumerationSequence
************************************************************/

bool CIMOperationRequestDispatcher::handlePullInstances(
    CIMPullInstancesRequestMessage* request)

{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handlePullInstances");

    // All pull operations execute off of a single templated
    // function.
    // Build and send response.  getCache used to wait for objects
    CIMPullInstancesResponseMessage* response(
        dynamic_cast<CIMPullInstancesResponseMessage*>(
            request->buildResponse()));

    bool releaseRequest = processPullRequest(request, response,
                                             "pullInstances");

    PEG_METHOD_EXIT();
    return releaseRequest;
}

/**$********************************************************
    handleCloseEnumeration
************************************************************/

void CIMOperationRequestDispatcher::handleCloseEnumeration(
        CIMCloseEnumerationRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleCloseEnumeration");

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "CloseEnumeration request for  "
            "contextId=%s .  ",
        (const char*)request->enumerationContext.getCString() ));

    EnumerationContext* en = _enumerationContextTable->find(
       request->enumerationContext);

    if (_rejectInvalidEnumerationContext(request, en))
    {
        PEG_METHOD_EXIT();
        return;
    }

    // KS_TODO should we test for received namespace here?

    en->incrementRequestCount();

    // lock the context until we have set processing state to avoid
    // conflict with timer thread and conducted all tests.
    Boolean providersComplete;
    {
        AutoMutex contextLock(en->_contextLock);

        if (_rejectIfContextTimedOut(request, en->isTimedOut()))
        {
            PEG_METHOD_EXIT();
            return;
        }

        // If another operation is active for this context, reject this
        // operation.
        // The specification allows as an option concurrent close (close
        // while pull request active) but we do not for now.  Complicates the
        // code to much for right now.
        if (_rejectIfEnumerationContextProcessing(request, en->isProcessing()))
        {
            PEG_METHOD_EXIT();
            return;
        }

        // Stop interoperation timer thread from reviewing this context.
        en->stopTimer();

        // Set the EnumerationContext Closed. No more requests will be
        // accepted for this enumerationContext
        en->setClientClosed();

        // If providers complete, we can release this context.
        if ((providersComplete = en->providersComplete()))
        {
            // Keep the context locked when exiting from AutoMutex contextLock
            // scope, _enumerationContextTable->releaseContext() below needs it
            // locked and it will unlock it before it destroys it.
            en->lockContext();
        }
    }
    if (providersComplete)
    {
        // Unlock and destroy the context.
        PEGASUS_ASSERT(_enumerationContextTable->releaseContext(en));
    }


    AutoPtr<CIMCloseEnumerationResponseMessage> response(
        dynamic_cast<CIMCloseEnumerationResponseMessage*>(
            request->buildResponse()));

    response->cimException = CIMException();

    _enqueueResponse(request, response.release());

    PEG_METHOD_EXIT();
    return;
}
//KS_PULL_END

/**************************************************************************/
// End of the Pull operation functions
/**************************************************************************/

//
//   Return Aggregated responses back to the Correct Aggregator
//
//   The aggregator includes an aggregation object that is used to
//   accumulate responses.  It is attached to each request sent and
//   received back as part of the response call back in the "parm"
//   Responses are aggregated until the count reaches the sent count and
//   then the aggregation code is called to create a single response from
//   the accumulated responses.
//   It uses information from the OperationAggregate to execute
//   special functions such as fixing the property list and
//   settiing hostName, etc.
//
/*********************************************************************/


void CIMOperationRequestDispatcher::handleOperationResponseAggregation(
    OperationAggregate* poA)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::"
            "handleOperationResponseAggregation");

    CIMResponseDataMessage* toResponse =
        (CIMResponseDataMessage*) poA->getResponse(0);
    PEG_TRACE(( TRC_DISPATCHER, Tracer::LEVEL3,
        "CIMOperationRequestDispatcher - "
            "RequestType=%s ResponseType=%s "
            "Namespace=%s ClassName=%s ResponseCount=%u "
            "messageId=%s",
        MessageTypeToString(poA->_msgRequestType),
        MessageTypeToString(toResponse->getType()),
        CSTRING(poA->_nameSpace.getString()),
        CSTRING(poA->_className.getString()),
        poA->numberResponses(),
        CSTRING(toResponse->messageId) ));

//// KS_TODO temporary while we finish pull testing
    PEG_TRACE(( TRC_DISPATCHER, Tracer::LEVEL4,
        "CIMOperationRequestDispatcher::handleOperationResponseAggregation"
        " - Type=%s requiresHostnameCompletion=%s _hasPropList=%s",
        MessageTypeToString(poA->_msgRequestType),
        boolToString(poA->_requiresHostnameCompletion),
        boolToString(poA->_hasPropList) ));

    CIMResponseData & to = toResponse->getResponseData();

    // Re-add the property list as stored from request after deepInheritance fix
    // since on OOP on the response message the property list gets lost
    // This is only done for EnumerateInstances type requests
    if (poA->_hasPropList)
    {
        switch (poA->_msgRequestType)
        {
            case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            {
                CIMEnumerateInstancesRequestMessage* request =
                    (CIMEnumerateInstancesRequestMessage*)poA->getRequest();
                to.setPropertyList(request->propertyList);
                break;
            }
            case CIM_ASSOCIATORS_REQUEST_MESSAGE:
            {
                CIMAssociatorsRequestMessage* request =
                    (CIMAssociatorsRequestMessage*)poA->getRequest();
                to.setPropertyList(request->propertyList);
                break;
            }
            case CIM_REFERENCES_REQUEST_MESSAGE:
            {
                CIMReferencesRequestMessage* request =
                    (CIMReferencesRequestMessage*)poA->getRequest();
                to.setPropertyList(request->propertyList);
                break;
            }
            default:
            {
                PEG_TRACE(( TRC_DISPATCHER, Tracer::LEVEL4,
                    "CIMOperationRequestDispatcher::"
                    "handleOperationResponseAggregation "
                    "ERRORNOTHANDLINGPROPERTYLIST -  Type=%s"
                    "requiresHostnameCompletion=%s _hasPropList=%s",
                    MessageTypeToString(poA->_msgRequestType),
                    boolToString(poA->_requiresHostnameCompletion),
                    boolToString(poA->_hasPropList) ));
            }
        }
    }

    // Work backward and delete each response off the end of the array
    // and append it to the new responseData
    for (Uint32 i = poA->numberResponses() - 1; i > 0; i--)
    {
        CIMResponseDataMessage* fromResponse =
            (CIMResponseDataMessage*)poA->getResponse(i);
        CIMResponseData & from = fromResponse->getResponseData();
        to.appendResponseData(from);
        poA->deleteResponse(i);
    }

    // If Response flagged for Filtering, Filter this response
    // CIMResponseData.
#ifdef PEGASUS_ENABLE_FQL
    if (poA->_filterResponse)
    {
        CIMException cimException;
        bool exception = false;
        QueryExpressionRep* query = poA->_query;

        FQLQueryStatement* qs = ((FQLQueryExpressionRep*)query)->_stmt;
        try
        {
            // If enumerate filter instances. Otherwise filter objects
            if (poA->_msgRequestType == CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE)
            {
                Array<CIMInstance> &insts = to.getInstances();

                for (int i = insts.size() - 1 ; i >= 0 ; i--)
                {
                    FQLInstancePropertySource ips(insts[i]);

                    if (!qs->evaluateQuery(&ips))
                    {
                        insts.remove(i);
                    }
                }
            }
            else
            {
                Array<CIMObject> &objcts = to.getObjects();

                for (int i = objcts.size() - 1 ; i >= 0 ; i--)
                {
                    FQLInstancePropertySource ips((CIMInstance)objcts[i]);

                    if (!qs->evaluateQuery(&ips))
                    {
                        objcts.remove(i);
                    }
                }
            }
        }
       // Should we catch parseError as a special KS_TODO
        catch (CIMException& e)
        {
            // KS_TODO should we keep the original query string
            String text = e.getMessage();

            cimException =
                PEGASUS_CIM_EXCEPTION(
                    CIM_ERR_INVALID_QUERY, text);
            exception=true;
        }
        catch (...)
        {
            // KS_TODO Should this be more general error.
            cimException =
                PEGASUS_CIM_EXCEPTION(
                    CIM_ERR_INVALID_QUERY, "General error caused Exception");
            exception = true;
        }
        // Reset the ResponseData size since we may have modified it.
        to.setSize();
        if (exception)
        {
            toResponse->cimException = cimException;
        }
    }
#endif
    if (poA->_requiresHostnameCompletion)
    {
        // fill in host, namespace on all instances on all elements of array
        // if they have been left out. Required because XML reader
        // will fail without them populated
        to.completeHostNameAndNamespace(System::getHostName(),
            poA->_nameSpace, poA->_pullOperation);
    }

    PEG_METHOD_EXIT();
}


// Aggregator for execQuery Response Aggregation.
// Aggregates responses into a single response and maps the responses back
// to ExecQuery responses.
void CIMOperationRequestDispatcher::handleExecQueryResponseAggregation(
    OperationAggregate* poA)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::handleExecQueryResponseAggregation");

    Uint32 numberResponses = poA->numberResponses();

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "CIMOperationRequestDispatcher::ExecQuery Response - "
            "NameSpace=%s  ClassName=%s ResponseCount=%u",
        CSTRING(poA->_nameSpace.getString()),
        CSTRING(poA->_className.getString()),
        numberResponses));

    if (numberResponses != 0)
    {
        CIMResponseMessage* response = poA->getResponse(0);
        CIMExecQueryResponseMessage* toResponse = 0;
        Uint32 startIndex = 0;
        Uint32 endIndex = numberResponses - 1;
        bool manyResponses = true;

        // Define pointer to the applyQueryToEnumeration function appropriate
        // for the query language defined.
        // NOTE: typedef applyQueryToEnumerationPtr is defined in
        // QuerySupportRouter.
        applyQueryFunctionPtr applyQueryToEnumeration =  NULL;
        applyQueryToEnumeration =
            QuerySupportRouter::getFunctPtr(this, poA->_query);

        if (response->getType() == CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE)
        {
            // Create an ExecQuery response from an EnumerateInstances request
            CIMOperationRequestMessage* request = poA->getRequest();
            AutoPtr<CIMExecQueryResponseMessage> query(
                new CIMExecQueryResponseMessage(
                    request->messageId,
                    CIMException(),
                    request->queueIds.copyAndPop()));
            query->syncAttributes(request);
            toResponse = query.release();
        }
        else
        {
            toResponse = (CIMExecQueryResponseMessage*) response;
            manyResponses = false;
        }

        // Work backward and delete each response off the end of the array
        // KS_TODO May 2104 - The manyResponses logic is convoluted and very
        // hard to understand. Not even certain is is correct since we do not
        // have a wealth of query providers in our tests.
        for (Uint32 i = endIndex; i >= startIndex; i--)
        {
            if (manyResponses)
            {
                response = poA->getResponse(i);
            }

            if (response->getType() == CIM_ENUMERATE_INSTANCES_RESPONSE_MESSAGE)
            {
                // convert enumerate instances response to exec query response
                // This is an indirect call to the appropriate function in
                // the WQL..., CQL, ... etc Class
                //
                // NULLness check is done as getFunctPtr may reurn NULL in case
                // of when query language is not CQL, WQL
                if (applyQueryToEnumeration)
                {
                    applyQueryToEnumeration(response, poA->_query);
                }
                CIMClass cimClass;
                Boolean clsRead = false;

                CIMEnumerateInstancesResponseMessage* fromResponse =
                    (CIMEnumerateInstancesResponseMessage*) response;
                Array<CIMInstance>& a =
                    fromResponse->getResponseData().getInstances();

                // Map instances to objects and add objectPath
                for (Uint32 j = 0, m = a.size(); j < m; j++)
                {
                    CIMObject co=CIMObject(a[j]);
                    CIMObjectPath op=co.getPath();
                    const Array<CIMKeyBinding>& kbs=op.getKeyBindings();

                    if (kbs.size() == 0)
                    {     // no path set
                        if (clsRead == false)
                        {
                            cimClass = _repository->getClass(
                                poA->_nameSpace, op.getClassName(),
                                false,true,false, CIMPropertyList());
                            clsRead=true;
                        }
                        // Trace enum responses that do not have
                        // path component.
                        // KS_TODO should this really be log???
                        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                            "query response, no path set in provider. "
                                   "ClassName=%s",
                            CSTRING(cimClass.getClassName().getString()) ));

                        op = a[j].buildPath(cimClass);
                    }
                    op.setNameSpace(poA->_nameSpace);
                    op.setHost(System::getHostName());
                    co.setPath(op);
                    if (manyResponses)
                    {
                        toResponse->getResponseData().appendObject(co);
                    }
                }
            }
            else   // This response is CIMExecQueryResponse
            {
                CIMExecQueryResponseMessage* fromResponse =
                    (CIMExecQueryResponseMessage*) response;

                CIMResponseData & from = fromResponse->getResponseData();

                from.completeHostNameAndNamespace(
                    System::getHostName(),
                    poA->_nameSpace);
                // KS_TODO this manyResponses makes no sense to me. Isn't
                // this always ExecQuery which is always false?
                if (manyResponses)
                {
                    toResponse->getResponseData().appendResponseData(from);
                }
            }
            if (manyResponses)
            {
                poA->deleteResponse(i);
            }

            if (i == 0)
            {
                break;
            }
        } // for all responses in response list

        // If we started with an enumerateInstances repsonse, then add it
        // to overall
        if ((startIndex == 0) && manyResponses)
        {
            poA->appendResponse(toResponse);
        }
    }
    PEG_METHOD_EXIT();
}
/*******End of the functions for aggregation***************************/

/**
    Convert the specified CIMValue to the specified type, and return it in
    a new CIMValue.
*/
CIMValue CIMOperationRequestDispatcher::_convertValueType(
    const CIMValue& value,
    CIMType type)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_convertValueType");

    CIMValue newValue;

    if (value.isArray())
    {
        Array<String> stringArray;
        Array<char*> charPtrArray;
        Array<const char*> constCharPtrArray;

        //
        // Convert the value to Array<const char*> to send to conversion method
        //
        // ATTN-RK-P3-20020221: Deal with TypeMismatchException
        // (Shouldn't really ever get that exception)
        value.get(stringArray);

        for (Uint32 k=0; k<stringArray.size(); k++)
        {
            // Need to build an Array<const char*> to send to the conversion
            // routine, but also need to keep track of them pointers as char*
            // because Windows won't let me delete a const char*.
            char* charPtr = strdup(stringArray[k].getCString());
            charPtrArray.append(charPtr);
            constCharPtrArray.append(charPtr);
        }

        //
        // Convert the value to the specified type
        //
        try
        {
            newValue =
                XmlReader::stringArrayToValue(0, constCharPtrArray, type);
        }
        catch (XmlSemanticError&)
        {
            for (Uint32 k=0; k<charPtrArray.size(); k++)
            {
                free(charPtrArray[k]);
            }

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "Server.CIMOperationRequestDispatcher."
                        "CIM_ERR_INVALID_PARAMETER",
                    "Malformed $0 value",
                    cimTypeToString(type)));
        }
        catch (...)
        {
            for (Uint32 k=0; k<charPtrArray.size(); k++)
            {
                free(charPtrArray[k]);
            }

            PEG_METHOD_EXIT();
            throw;
        }

        for (Uint32 k = 0; k < charPtrArray.size(); k++)
        {
            free(charPtrArray[k]);
        }
    }
    else
    {
        String stringValue;

        // ATTN-RK-P3-20020221: Deal with TypeMismatchException
        // (Shouldn't really ever get that exception)
        value.get(stringValue);

        try
        {
            newValue =
                XmlReader::stringToValue(0, stringValue.getCString(), type);
        }
        catch (XmlSemanticError&)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER,
                MessageLoaderParms(
                    "Server.CIMOperationRequestDispatcher."
                        "CIM_ERR_INVALID_PARAMETER",
                    "Malformed $0 value",
                    cimTypeToString(type)));
        }
    }

    PEG_METHOD_EXIT();
    return newValue;
}

/**
   Find the CIMParamValues in the InvokeMethod request whose types were
   not specified in the XML encoding, and convert them to the types
   specified in the method schema.
*/
void CIMOperationRequestDispatcher::_fixInvokeMethodParameterTypes(
    CIMInvokeMethodRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_fixInvokeMethodParameterTypes");

    Boolean gotMethodDefinition = false;
    CIMMethod method;

    //
    // Cycle through the input parameters, converting the untyped ones.
    //
    Array<CIMParamValue> inParameters = request->inParameters;
    Uint32 numInParamValues = inParameters.size();
    for (Uint32 i=0; i<numInParamValues; i++)
    {
        if (!inParameters[i].isTyped())
        {
            //
            // Retrieve the method definition, if we haven't already done so
            // (only look up the method if we have an untyped parameter value)
            //
            if (!gotMethodDefinition)
            {
                //
                // Get the class definition for this method
                //
                CIMClass cimClass =
                    _repository->getClass(
                        request->nameSpace,
                        request->instanceName.getClassName(),
                        false, //localOnly,
                        false, //includeQualifiers,
                        false, //includeClassOrigin,
                        CIMPropertyList());

                PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                    "CIMOperationRequestDispatcher::"
                        "_fixInvokeMethodParameterTypes - "
                        "Namespace=%s  ClassName=%s",
                    CSTRING(request->nameSpace.getString()),
                    CSTRING(request->instanceName.getClassName().getString())));

                //
                // Get the method definition from the class
                //
                Uint32 methodPos = cimClass.findMethod(request->methodName);
                if (methodPos == PEG_NOT_FOUND)
                {
                    throw PEGASUS_CIM_EXCEPTION(
                        CIM_ERR_METHOD_NOT_FOUND, String::EMPTY);
                }
                method = cimClass.getMethod(methodPos);

                gotMethodDefinition = true;
            }

            //
            // Find the parameter definition for this input parameter
            //
            CIMName paramName = inParameters[i].getParameterName();
            Uint32 numParams = method.getParameterCount();
            for (Uint32 j=0; j<numParams; j++)
            {
                CIMParameter param = method.getParameter(j);
                if (paramName == param.getName())
                {
                    //
                    // Retype the input parameter value according to the
                    // type defined in the class/method schema
                    //
                    CIMType paramType = param.getType();
                    CIMValue newValue;

                    if (inParameters[i].getValue().isNull())
                    {
                        newValue.setNullValue(
                            param.getType(),
                            param.isArray());
                    }
                    else if (inParameters[i].getValue().isArray() !=
                                 param.isArray())
                    {
                        // ATTN-RK-P1-20020222: Who catches this?  They aren't.
                        PEG_METHOD_EXIT();
                        throw PEGASUS_CIM_EXCEPTION(
                            CIM_ERR_TYPE_MISMATCH, String::EMPTY);
                    }
                    else
                    {
                        newValue = _convertValueType(
                            inParameters[i].getValue(),
                            paramType);
                    }

                    inParameters[i].setValue(newValue);
                    inParameters[i].setIsTyped(true);
                    break;
                }
            }
        }
    }

    PEG_METHOD_EXIT();
}

/**
    Convert the CIMValue given in a SetProperty request to the correct
    type according to the schema, because it is not possible to specify
    the property type in the XML encoding.
*/
void CIMOperationRequestDispatcher::_fixSetPropertyValueType(
    CIMSetPropertyRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_fixSetPropertyValueType");

    CIMValue inValue = request->newValue;

    //
    // Only do the conversion if the type is not already set
    //
    if ((inValue.getType() != CIMTYPE_STRING))
    {
        PEG_METHOD_EXIT();
        return;
    }

    //
    // Get the class definition for this property
    //
    CIMClass cimClass;
    try
    {
        cimClass = _repository->getClass(
            request->nameSpace,
            request->instanceName.getClassName(),
            false, //localOnly,
            false, //includeQualifiers,
            false, //includeClassOrigin,
            CIMPropertyList());

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
            "CIMOperationRequestDispatcher::_fixSetPropertyValueType - "
                "Namespace: %s  Class Name: %s",
            CSTRING(request->nameSpace.getString()),
            CSTRING(request->instanceName.getClassName().getString())));
    }
    catch (CIMException& exception)
    {
        // map CIM_ERR_NOT_FOUND to CIM_ERR_INVALID_CLASS
        if (exception.getCode() == CIM_ERR_NOT_FOUND)
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_CLASS,
                request->instanceName.getClassName().getString());
        }

        PEG_METHOD_EXIT();
        throw;
    }

    //
    // Get the property definition from the class
    //
    Uint32 propertyPos = cimClass.findProperty(request->propertyName);
    if (propertyPos == PEG_NOT_FOUND)
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NO_SUCH_PROPERTY, String::EMPTY);
    }
    CIMProperty property = cimClass.getProperty(propertyPos);

    //
    // Retype the input property value according to the
    // type defined in the schema
    //
    CIMValue newValue;

    if (inValue.isNull())
    {
        newValue.setNullValue(property.getType(), property.isArray());
    }
    else if (inValue.isArray() != property.isArray())
    {
        // ATTN-RK-P1-20020222: Who catches this?  They aren't.
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_TYPE_MISMATCH, String::EMPTY);
    }
    else
    {
        newValue = _convertValueType(inValue, property.getType());
    }

    //
    // Put the retyped value back into the message
    //
    request->newValue = newValue;

    PEG_METHOD_EXIT();
}

/*
    Check the existence of a class matching a classname.
    Returns true if class found in repository.
    Note that this code checks for the special classname
    PEGASUS_CLASSNAME___NAMESPACE and returns true.

    If the repository returns exception this function returns false
    if NOT CIM_ERR_NOT_FOUND. Else, rethrows the exception.
*/
Boolean CIMOperationRequestDispatcher::_checkExistenceOfClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_checkExistenceOfClass");

    if (className.equal(CIMName(PEGASUS_CLASSNAME___NAMESPACE)))
    {
        PEG_METHOD_EXIT();
        return true;
    }

    try
    {
        CIMClass cimClass = _repository->getClass(
                nameSpace,
                className,
                true,
                false,
                false,
                CIMPropertyList());
    }
    catch (const CIMException& exception)
    {
        // CIM_ERR_NOT_FOUND indicates that the class is not defined
        if (exception.getCode() == CIM_ERR_NOT_FOUND)
        {
            PEG_METHOD_EXIT();
            return false;
        }

        PEG_METHOD_EXIT();
        throw;
    }

    PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
        "CIMOperationRequestDispatcher::_checkExistenceOfClass - "
            "Namespace: %s  Class Name: %s found.",
        CSTRING(nameSpace.getString()),
        CSTRING(className.getString())));

    PEG_METHOD_EXIT();
    return true;
}

CIMConstClass CIMOperationRequestDispatcher::_getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    CIMException& cimException)
{
    if (className.equal(CIMName(PEGASUS_CLASSNAME___NAMESPACE)))
    {
        CIMClass __namespaceClass(PEGASUS_CLASSNAME___NAMESPACE);
        // ATTN: Qualifiers not added here, but they shouldn't be needed
        __namespaceClass.addProperty(
            CIMProperty(PEGASUS_PROPERTYNAME_NAME, String::EMPTY));
        return __namespaceClass;
    }

    CIMConstClass cimClass;

    // get the complete class, specifically not local only
    try
    {
        cimClass = _repository->getFullConstClass(
            nameSpace,
            className);

        PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL3,
            "CIMOperationRequestDispatcher::_getClass - "
                "Namespace: %s  Class Name: %s",
            CSTRING(nameSpace.getString()),
            CSTRING(className.getString())));
    }
    catch (const CIMException& exception)
    {
        // map CIM_ERR_NOT_FOUND to CIM_ERR_INVALID_CLASS
        if (exception.getCode() == CIM_ERR_NOT_FOUND)
        {
            cimException = PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_CLASS, className.getString());
        }
        else
        {
            cimException = exception;
        }
    }
    catch (const Exception& exception)
    {
        cimException =
            PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, exception.getMessage());
    }
    catch (...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED, String::EMPTY);
    }

    return cimClass;
}

template <class ObjectClass>
void removePropagatedAndOriginAttributes(ObjectClass& newObject)
{
    Uint32 numProperties = newObject.getPropertyCount();
    for (Uint32 i = 0; i < numProperties; i++)
    {
        CIMProperty currentProperty = newObject.getProperty(i);
        if (currentProperty.getPropagated() == true ||
            currentProperty.getClassOrigin().getString().size() > 0)
        {
            newObject.removeProperty(i);
            currentProperty.setPropagated(false);
            currentProperty.setClassOrigin(CIMName());
            newObject.addProperty(currentProperty);
            --i;
        }
    }
}

//Issue  enumerateInstance...ForClass calls to the repository for each class
// that doesn ot have a provider and put the returned responses into the
// ResponseData.
// Handles exceptions from the repository inserting them as CIMException
// into the response.
// This is common code for all Enumerates to the repository because they
// all use the same pattern, doing a enumerate...ForClass loop to get
// the instances/paths for each class that does not have a provider.
//
// TODO: Do we really need the exceptions here?  Actually, review to see
// if we really need the enumerate for classes itself and could not just
// use enumerateInstances Since there should NEVER be anything for both
// provider and repository.
//
// Returns a single response in the OperationAggregate object with the
// counters properly set up.
// This function throws an exception of there is an exception from the
// repository.
//

Boolean CIMOperationRequestDispatcher::_enumerateFromRepository(
    CIMOperationRequestMessage* request,
    OperationAggregate* poA,
    ProviderInfoList& providerInfos)
{
    PEG_METHOD_ENTER(TRC_DISPATCHER,
        "CIMOperationRequestDispatcher::_enumerateFromRepository");
    Boolean responsesExist = false;
    // Create AutoPtr to force delete if exception
    AutoPtr<OperationAggregate> lPoA(poA);

    if (_repository->isDefaultInstanceProvider())
    {
        // Loop through providerInfos, forwarding requests to repository

        while (providerInfos.hasMore(false))
        {
            ProviderInfo& providerInfo = providerInfos.getNext();
            // If this class is not registered for provider, get instances
            // from repository
            PEG_TRACE((TRC_DISPATCHER, Tracer::LEVEL4,
                "Routing  request for class %s to the "
                    "repository.  Class # %u of %u",
               CSTRING(providerInfo.className.getString()),
               (providerInfos.getIndex()), providerInfos.size() ));

            switch (request->getType())
            {
                case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
                {
                    const CIMEnumerateInstancesRequestMessage* req =
                        reinterpret_cast<const
                            CIMEnumerateInstancesRequestMessage*>(request);
                    // create response from request
                    AutoPtr<CIMEnumerateInstancesResponseMessage> response(
                        dynamic_cast<CIMEnumerateInstancesResponseMessage*>(
                            req->buildResponse()));

                    // Enumerate instances only for this class
                    response->getResponseData().setInstances(
                        _repository->enumerateInstancesForClass(
                            req->nameSpace,
                            providerInfo.className,
                            req->includeQualifiers,
                            req->includeClassOrigin,
                            req->propertyList));

                    poA->appendResponse(response.release());
                    break;
                }
                case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
                {
                    const CIMEnumerateInstanceNamesRequestMessage* req =
                        reinterpret_cast<const
                            CIMEnumerateInstanceNamesRequestMessage*>(request);
                    // create response from request
                    AutoPtr<CIMEnumerateInstanceNamesResponseMessage> response(
                        dynamic_cast<CIMEnumerateInstanceNamesResponseMessage*>(
                            req->buildResponse()));

                    // Enumerate instances only for this class
                    response->getResponseData().setInstanceNames(
                        _repository->enumerateInstanceNamesForClass(
                            req->nameSpace,
                            providerInfo.className));

                    poA->appendResponse(response.release());
                    break;
                }

                default:
                {
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0));
                }
            } // switch statement
        }  // while loop

        // Set the local number of expected responses counter
        responsesExist = (poA->numberResponses() != 0);

        if (responsesExist)
        {
            // Aggregate the responses into a single response object
            handleOperationResponseAggregation(poA);
            poA->incTotalIssued();
        }
    }

    // release poA and exit with flag indicating responses exist or not
    lPoA.release();
    PEG_METHOD_EXIT();
    return responsesExist;
}

// Forward request to provider unless class does not exist
// If class does not exist, return false
// KS_FUTURE Possibly change this to always send.  Sends exception message if
// class get error. No need for Boolean Return
Boolean CIMOperationRequestDispatcher::_forwardEnumerationToProvider(
    ProviderInfo &providerInfo,
    OperationAggregate* poA,
    CIMOperationRequestMessage* request)
{
    CIMException checkClassException;

    CIMConstClass cimClass = _getClass(
        poA->_nameSpace,
        providerInfo.className,
        checkClassException);

    // The following is not clean. Need better way to terminate.
    // This builds an error response message in the middle of the
    // normal response but lets rest of operation continue.
    if (checkClassException.getCode() != CIM_ERR_SUCCESS)
    {
        CIMResponseMessage* response = request->buildResponse();
        response->cimException = checkClassException;
        _forwardResponseForAggregation(request,  poA, response);
    }

    else
    {
        if (providerInfo.providerIdContainer.get() != 0)
        {
            request->operationContext.insert(
                *(providerInfo.providerIdContainer.get()));
        }

#ifdef PEGASUS_ENABLE_OBJECT_NORMALIZATION
        if (providerInfo.hasProviderNormalization)
        {
            request->operationContext.insert(
                CachedClassDefinitionContainer(cimClass));
        }
#endif
        _forwardAggregatingRequestToProvider(
           providerInfo, request, poA);
    }
    return true;
}

PEGASUS_NAMESPACE_END
