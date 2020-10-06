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

#include <cctype>
#include <cstdio>
#include <errno.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HostLocator.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/WsmServer/WsmFault.h>
#include "WsmToCimRequestMapper.h"

PEGASUS_NAMESPACE_BEGIN

static bool _isInputParameter(const CIMParameter& cp)
{
    // Check to see if the given parameter is an input parameter (whether
    // it bears a true "In" qualifier).

    static const CIMName inQualifier("In");
    Uint32 pos = cp.findQualifier(inQualifier);

    if (pos != PEG_NOT_FOUND)
    {
        CIMConstQualifier cq = cp.getQualifier(pos);
        const CIMValue& cv = cq.getValue();

        if (cv.getType() == CIMTYPE_BOOLEAN)
        {
            Boolean in;
            cv.get(in);
            return in;
        }
    }

    return false;
}

WsmToCimRequestMapper::WsmToCimRequestMapper(CIMRepository* repository)
    : _repository(repository)
{
}

WsmToCimRequestMapper::~WsmToCimRequestMapper()
{
}

CIMOperationRequestMessage* WsmToCimRequestMapper::mapToCimRequest(
    WsmRequest* request)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "WsmToCimRequestMapper::mapToCimRequest");
    AutoPtr<CIMOperationRequestMessage> cimRequest;
    switch (request->getType())
    {
        case WS_TRANSFER_GET:
            cimRequest.reset(mapToCimGetInstanceRequest(
                (WxfGetRequest*) request));
            break;

        case WS_TRANSFER_PUT:
            cimRequest.reset(mapToCimModifyInstanceRequest(
                (WxfPutRequest*) request));
            break;

        case WS_SUBSCRIPTION_CREATE:
            cimRequest.reset(mapToCimCreateInstanceRequest(request, true));
            break;

        case WS_TRANSFER_CREATE:
            cimRequest.reset(mapToCimCreateInstanceRequest(request));
            break;

        case WS_TRANSFER_DELETE:
            cimRequest.reset(mapToCimDeleteInstanceRequest(request));
            break;

        case WS_SUBSCRIPTION_DELETE:
            cimRequest.reset(mapToCimDeleteInstanceRequest(request, true));
            break;

        case WS_ENUMERATION_ENUMERATE:

            // Test for no association filter
            if (((WsenEnumerateRequest*)request)->wsmFilter.filterDialect !=
                WsmFilter::ASSOCIATION)
            {
                if (((WsenEnumerateRequest*) request)->enumerationMode ==
                    WSEN_EM_OBJECT ||
                    ((WsenEnumerateRequest*) request)->enumerationMode ==
                    WSEN_EM_OBJECT_AND_EPR)
                {
                    cimRequest.reset(mapToCimEnumerateInstancesRequest(
                        (WsenEnumerateRequest*) request));
                }
                else if (((WsenEnumerateRequest*) request)->enumerationMode ==
                         WSEN_EM_EPR)
                {
                    cimRequest.reset(mapToCimEnumerateInstanceNamesRequest(
                        (WsenEnumerateRequest*) request));
                }
                else
                {
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                }
            }
            else // association filter dialect
            {
                // decision on use of association or associated
                // associated calls the CIMAssociator functions
                if (((WsenEnumerateRequest*)request)->
                    wsmFilter.AssocFilter.assocFilterType ==
                        WsmFilter::ASSOCIATED_INSTANCES)
                {
                    if (((WsenEnumerateRequest*) request)->enumerationMode ==
                        WSEN_EM_OBJECT ||
                        ((WsenEnumerateRequest*) request)->enumerationMode ==
                        WSEN_EM_OBJECT_AND_EPR)
                    {
                        cimRequest.reset(mapToCimAssociatorsRequest(
                            (WsenEnumerateRequest*) request));
                    }
                    else if (((WsenEnumerateRequest*) request)->
                             enumerationMode == WSEN_EM_EPR)
                    {
                        cimRequest.reset(mapToCimAssociatorNamesRequest(
                            (WsenEnumerateRequest*) request));
                    }
                    else
                    {
                        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                    }
                }
                else // association calls the Reference Functions
                {
                    if (((WsenEnumerateRequest*) request)->enumerationMode ==
                        WSEN_EM_OBJECT ||
                        ((WsenEnumerateRequest*) request)->enumerationMode ==
                        WSEN_EM_OBJECT_AND_EPR)
                    {
                        cimRequest.reset(mapToCimReferencesRequest(
                            (WsenEnumerateRequest*) request));
                    }
                    else if (((WsenEnumerateRequest*) request)->
                             enumerationMode == WSEN_EM_EPR)
                    {
                        cimRequest.reset(mapToCimReferenceNamesRequest(
                            (WsenEnumerateRequest*) request));
                    }
                    else
                    {
                        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                    }
                }
            }
            break;

        case WS_ENUMERATION_PULL:
            break;

        case WS_ENUMERATION_RELEASE:
            break;

        case WS_INVOKE:
        {
            cimRequest.reset(mapToCimInvokeMethodRequest(
                (WsInvokeRequest*)request));
            break;
        }

        default:
             PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }

    if (cimRequest.get())
    {
        cimRequest->operationContext.insert(
            IdentityContainer(request->userName));
        cimRequest->operationContext.set(
            AcceptLanguageListContainer(request->acceptLanguages));
        cimRequest->operationContext.set(
            ContentLanguageListContainer(request->contentLanguages));
        cimRequest->setHttpMethod(request->httpMethod);
        cimRequest->setCloseConnect(request->httpCloseConnect);
        cimRequest->binaryRequest = true;
        cimRequest->binaryResponse = true;
    }
    PEG_METHOD_EXIT();
    return cimRequest.release();
}

CIMGetInstanceRequestMessage*
    WsmToCimRequestMapper::mapToCimGetInstanceRequest(
    WxfGetRequest* request)
{
    CIMNamespaceName nameSpace;
    CIMObjectPath instanceName;

    _disallowAllClassesResourceUri(request->epr.resourceUri);

    // EPR to object path does a generic conversion, including conversion
    // of EPR address to host and namespace selector to CIM namespace.
    // For GetInstance operation instance name should only contain a
    // class name and key bindings.
    convertEPRToObjectPath(request->epr, instanceName);
    nameSpace = instanceName.getNameSpace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    CIMGetInstanceRequestMessage* cimRequest =
        new CIMGetInstanceRequestMessage(
            XmlWriter::getNextMessageId(),
            nameSpace,
            instanceName,
            false,
            false,
            CIMPropertyList(),
            QueueIdStack(request->queueId),
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;

    return cimRequest;
}

CIMModifyInstanceRequestMessage*
    WsmToCimRequestMapper::mapToCimModifyInstanceRequest(
        WxfPutRequest* request)
{
    CIMNamespaceName nameSpace;
    CIMObjectPath instanceName;

    _disallowAllClassesResourceUri(request->epr.resourceUri);

    // EPR to object path does a generic conversion, including conversion
    // of EPR address to host and namespace selector to CIM namespace.
    // For ModifyInstance operation instance name should only contain a
    // class name and key bindings.
    convertEPRToObjectPath(request->epr, instanceName);
    nameSpace = instanceName.getNameSpace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    CIMInstance instance;
    convertWsmToCimInstance(request->instance, nameSpace, instance);
    instance.setPath(instanceName);

    CIMModifyInstanceRequestMessage* cimRequest =
        new CIMModifyInstanceRequestMessage(
            XmlWriter::getNextMessageId(),
            nameSpace,
            instance,
            false, // includeQualifiers
            CIMPropertyList(),
            QueueIdStack(request->queueId),
            request->authType,
            request->userName);

    cimRequest->ipAddress = request->ipAddress;

    return cimRequest;
}

CIMCreateInstanceRequestMessage*
    WsmToCimRequestMapper::mapToCimCreateInstanceRequest(
        WsmRequest* request,Boolean isSubscriptionCreateReq)
{
    CIMNamespaceName nameSpace;
    CIMObjectPath instanceName;
    WsmEndpointReference epr;
    WsmInstance wsmInstance;
    if(isSubscriptionCreateReq)
    {
        WxfSubCreateRequest * req = (WxfSubCreateRequest *)request;
        epr = req->epr;
        wsmInstance = req->instance;
    }
    else
    {
        WxfCreateRequest *createReq = (WxfCreateRequest *)request;
        epr = createReq->epr;
        wsmInstance = createReq->instance;
    }
    

    // EPR to object path does a generic conversion, including conversion
    // of EPR address to host and namespace selector to CIM namespace.
    // For CreateInstance operation instance name should only contain a
    // class name and key bindings.
    convertEPRToObjectPath(epr, instanceName);
    nameSpace = instanceName.getNameSpace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    CIMInstance instance;
    convertWsmToCimInstance(wsmInstance, nameSpace, instance);

    CIMCreateInstanceRequestMessage* cimRequest =
        new CIMCreateInstanceRequestMessage(
            XmlWriter::getNextMessageId(),
            nameSpace,
            instance,
            QueueIdStack(request->queueId),
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;

    return cimRequest;
}
CIMDeleteInstanceRequestMessage*
    WsmToCimRequestMapper::mapToCimDeleteInstanceRequest(
        WsmRequest* request , Boolean isSubDeleteReq)
{
    CIMNamespaceName nameSpace;
    CIMObjectPath instanceName;
    WsmEndpointReference epr;
    if(isSubDeleteReq)
    {
        WxfSubDeleteRequest * req = (WxfSubDeleteRequest *)request;
        epr = req->epr;
           
    }
    else
    {
        WxfDeleteRequest * deleteReq = (WxfDeleteRequest *)request;
        epr = deleteReq->epr;
    }


    _disallowAllClassesResourceUri(epr.resourceUri);

    // EPR to object path does a generic conversion, including conversion
    // of EPR address to host and namespace selector to CIM namespace.
    // For DeleteInstance operation instance name should only contain a
    // class name and key bindings.
    convertEPRToObjectPath(epr, instanceName);
    nameSpace = instanceName.getNameSpace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    CIMDeleteInstanceRequestMessage* cimRequest =
        new CIMDeleteInstanceRequestMessage(
            XmlWriter::getNextMessageId(),
            nameSpace,
            instanceName,
            QueueIdStack(request->queueId),
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;

    return cimRequest;
}

CIMEnumerateInstancesRequestMessage*
    WsmToCimRequestMapper::mapToCimEnumerateInstancesRequest(
        WsenEnumerateRequest* request)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmToCimRequestMapper::mapToCimEnumerateInstancesRequest");

    CIMObjectPath objPath;
    convertEPRToObjectPath(request->epr, objPath);

    CIMEnumerateInstancesRequestMessage* cimRequest =
        new CIMEnumerateInstancesRequestMessage(
            XmlWriter::getNextMessageId(),
            objPath.getNameSpace(),
            objPath.getClassName(),
            request->polymorphismMode == WSMB_PM_INCLUDE_SUBCLASS_PROPERTIES,
            false, // includeQualifiers
            false, // includeClassOrigin
            CIMPropertyList(),
            QueueIdStack(request->queueId),
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;
    PEG_METHOD_EXIT();
    return cimRequest;
}

CIMEnumerateInstanceNamesRequestMessage*
    WsmToCimRequestMapper::mapToCimEnumerateInstanceNamesRequest(
        WsenEnumerateRequest* request)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmToCimRequestMapper::mapToCimEnumerateInstanceNamesRequest");
    CIMObjectPath objPath;
    convertEPRToObjectPath(request->epr, objPath);

    CIMEnumerateInstanceNamesRequestMessage* cimRequest =
        new CIMEnumerateInstanceNamesRequestMessage(
            XmlWriter::getNextMessageId(),
            objPath.getNameSpace(),
            objPath.getClassName(),
            QueueIdStack(request->queueId),
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;
    PEG_METHOD_EXIT();
    return cimRequest;
}

CIMReferencesRequestMessage*
    WsmToCimRequestMapper::mapToCimReferencesRequest(
        WsenEnumerateRequest* request)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmToCimRequestMapper::mapToCimReferencesRequest");

    _disallowAllClassesResourceUri(
        request->wsmFilter.AssocFilter.object.resourceUri);

    CIMObjectPath  instanceName;
    convertEPRToObjectPath(request->wsmFilter.AssocFilter.object, instanceName);

    CIMNamespaceName ns = request->wsmFilter.AssocFilter.object.getNamespace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    // We check for instance in the filter parser so there should be no
    // need for another check here.  However, should we do a double check
    // in the maptoWsm return just in case?

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
               "References Request "
               "Namespace=%s "
               "instanceName=%s "
               "resultClassName=%s "
               "role=%s",
               (const char*)ns.getString().getCString(),
               (const char*)instanceName.toString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.resultClassName.getString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.role.getCString()));

    CIMReferencesRequestMessage* cimRequest =
        new CIMReferencesRequestMessage(
            XmlWriter::getNextMessageId(),
            ns,
            instanceName,
            request->wsmFilter.AssocFilter.resultClassName,
            request->wsmFilter.AssocFilter.role,
            false, // includeQualifiers
            false, // includeClassOrigin
            CIMPropertyList(),
            QueueIdStack(request->queueId),
            false,                        // WSMAN does not do class request
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;
    PEG_METHOD_EXIT();
    return cimRequest;
}

CIMReferenceNamesRequestMessage*
    WsmToCimRequestMapper::mapToCimReferenceNamesRequest(
        WsenEnumerateRequest* request)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmToCimRequestMapper::mapToCimReferenceNamesRequest");

    _disallowAllClassesResourceUri(
        request->wsmFilter.AssocFilter.object.resourceUri);

    CIMObjectPath  instanceName;
    convertEPRToObjectPath(request->wsmFilter.AssocFilter.object, instanceName);

    CIMNamespaceName ns = request->wsmFilter.AssocFilter.object.getNamespace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
               "ReferenceNames Request "
               "Namespace=%s "
               "instanceName=%s "
               "resultClassName=%s "
               "role=%s",
               (const char*)ns.getString().getCString(),
               (const char*)instanceName.toString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.resultClassName.getString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.role.getCString()));

    CIMReferenceNamesRequestMessage* cimRequest =
        new CIMReferenceNamesRequestMessage(
            XmlWriter::getNextMessageId(),
            ns,
            instanceName,
            request->wsmFilter.AssocFilter.resultClassName,
            request->wsmFilter.AssocFilter.role,
            QueueIdStack(request->queueId),
            false,            // WSMAN does not do class request
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;
    PEG_METHOD_EXIT();
    return cimRequest;
}

CIMAssociatorsRequestMessage*
    WsmToCimRequestMapper::mapToCimAssociatorsRequest(
        WsenEnumerateRequest* request)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmToCimRequestMapper::mapToCimAssociatorsRequest");

    _disallowAllClassesResourceUri(
        request->wsmFilter.AssocFilter.object.resourceUri);

    CIMObjectPath  instanceName;
    convertEPRToObjectPath(request->wsmFilter.AssocFilter.object, instanceName);

    CIMNamespaceName ns = request->wsmFilter.AssocFilter.object.getNamespace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
               "Associators Request "
               "Namespace=%s "
               "instanceName=%s "
               "assocClasName=%s "
               "resultClassName=%s "
               "role=%s "
               "resultRole=%s",
               (const char*)ns.getString().getCString(),
               (const char*)instanceName.toString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.assocClassName.getString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.resultClassName.getString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.role.getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.resultRole.getCString()));

    CIMAssociatorsRequestMessage* cimRequest =
        new CIMAssociatorsRequestMessage(
            XmlWriter::getNextMessageId(),
            ns,
            instanceName,
            request->wsmFilter.AssocFilter.assocClassName,
            request->wsmFilter.AssocFilter.resultClassName,
            request->wsmFilter.AssocFilter.role,
            request->wsmFilter.AssocFilter.resultRole,
            false, // includeQualifiers
            false, // includeClassOrigin
            CIMPropertyList(),
            QueueIdStack(request->queueId),
            false,          // WSMAN does not do class request
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;
    PEG_METHOD_EXIT();
    return cimRequest;
}

CIMAssociatorNamesRequestMessage*
    WsmToCimRequestMapper::mapToCimAssociatorNamesRequest(
        WsenEnumerateRequest* request)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "WsmToCimRequestMapper::mapToCimAssociatorNamesRequest");

    _disallowAllClassesResourceUri(
        request->wsmFilter.AssocFilter.object.resourceUri);

    CIMObjectPath  instanceName;
    convertEPRToObjectPath(
        request->wsmFilter.AssocFilter.object, instanceName);

    CIMNamespaceName ns = request->wsmFilter.AssocFilter.object.getNamespace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
               "AssociatorNames Request "
               "Namespace=%s "
               "instanceName=%s "
               "assocClasName=%s "
               "resultClassName=%s "
               "role=%s "
               "resultRole=%s",
               (const char*)ns.getString().getCString(),
               (const char*)instanceName.toString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.assocClassName.getString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.resultClassName.getString().getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.role.getCString(),
               (const char*)request->
               wsmFilter.AssocFilter.resultRole.getCString()));

    CIMAssociatorNamesRequestMessage* cimRequest =
        new CIMAssociatorNamesRequestMessage(
            XmlWriter::getNextMessageId(),
            ns,
            instanceName,
            request->wsmFilter.AssocFilter.assocClassName,
            request->wsmFilter.AssocFilter.resultClassName,
            request->wsmFilter.AssocFilter.role,
            request->wsmFilter.AssocFilter.resultRole,
            QueueIdStack(request->queueId),
            false,            // WSMAN does not do class request
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;
    PEG_METHOD_EXIT();
    return cimRequest;
}

CIMInvokeMethodRequestMessage*
WsmToCimRequestMapper::mapToCimInvokeMethodRequest(
    WsInvokeRequest* request)
{
    // EPR to object path does a generic conversion, including conversion
    // of EPR address to host and namespace selector to CIM namespace.
    // For CreateInstance operation instance name should only contain a
    // class name and key bindings.

    CIMNamespaceName nameSpace;
    CIMObjectPath instanceName;
    convertEPRToObjectPath(request->epr, instanceName);
    nameSpace = instanceName.getNameSpace();
    instanceName.setNameSpace(CIMNamespaceName());
    instanceName.setHost(String::EMPTY);

    // Convert to array of CIMParamValue.

    Array<CIMParamValue> parameters;

    convertWsmToCimParameters(
        nameSpace,
        request->className,
        request->methodName,
        request->instance,
        parameters);

    CIMInvokeMethodRequestMessage* cimRequest =
        new CIMInvokeMethodRequestMessage(
            XmlWriter::getNextMessageId(),
            nameSpace,
            instanceName,
            request->methodName,
            parameters,
            QueueIdStack(request->queueId),
            request->authType,
            request->userName);
    cimRequest->ipAddress = request->ipAddress;

    return cimRequest;
}

void WsmToCimRequestMapper::_disallowAllClassesResourceUri(
    const String& resourceUri)
{
    // DSP0227 R6-1:  A service should return a wsa:ActionNotSupported fault
    // if the "all classes" ResourceURI is used with any of the WS-Transfer
    // operations, even if this ResourceURI is supported for enumerations or
    // eventing.

    CString cstr(resourceUri.getCString());
    const char* suffix = WsmUtils::skipHostUri(cstr);

    if (strcmp(suffix, WSM_RESOURCEURI_ALLCLASSES_SUFFIX) == 0)
    {
        throw WsmFault(
            WsmFault::wsa_ActionNotSupported,
            MessageLoaderParms(
                "WsmServer.WsmToCimRequestMapper.ALLCLASSES_URI_NOT_ALLOWED",
                "The \"all classes\" ResourceURI cannot be used with this "
                    "operation."),
            WSMAN_FAULTDETAIL_ACTIONMISMATCH);
    }
}

CIMName WsmToCimRequestMapper::convertResourceUriToClassName(
    const String& resourceUri)
{
    CString cstr(resourceUri.getCString());
    const char* suffix = WsmUtils::skipHostUri(cstr);
    size_t n = sizeof(WSM_RESOURCEURI_CIMSCHEMAV2_SUFFIX) - 1;

    if (strncmp(suffix, WSM_RESOURCEURI_CIMSCHEMAV2_SUFFIX, n) == 0 &&
        suffix[n] == '/')
    {
        const char* className = &suffix[n + 1];

        if (CIMNameLegalASCII(className))
            return CIMNameCast(String(className));
    }
    if (strcmp(cstr, WSM_RESOURCEURI_INDICATION_FILTER) == 0 )
    {
        return CIMNameCast(String("CIM_IndicationFilter"));
    }

    throw WsmFault(
        WsmFault::wsa_DestinationUnreachable,
        MessageLoaderParms(
            "WsmServer.WsmToCimRequestMapper.UNRECOGNIZED_RESOURCEURI",
            "Unrecognized ResourceURI value: $0.",
            resourceUri),
        WSMAN_FAULTDETAIL_INVALIDRESOURCEURI);
}

String WsmToCimRequestMapper::convertEPRAddressToHostname(const String& addr)
{
    // EPR address is formed by adding '/wsman' to the host name.
    // E.g. http://localhost:5988/wsman
    // Extract the host name from the EPR address
    String hostName;
    if (addr != WSM_ADDRESS_ANONYMOUS)
    {
        Uint32 pos1 = 0;
        if (String::compare(addr, "http://", 7) == 0)
        {
            pos1 = 7;
        }
        else if (String::compare(addr, "https://", 8) == 0)
        {
            pos1 = 8;
        }

        Uint32 pos2 = addr.reverseFind('/');
        if (pos1 != 0 && pos2 != PEG_NOT_FOUND && pos2 > pos1)
        {
            // The string between "http[s]://" and "/wsman" must be
            // the host name
            HostLocator hostLoc(addr.subString(pos1, pos2 - pos1));
            if (hostLoc.isValid())
            {
                hostName = hostLoc.getHost();
            }
        }

        if (hostName.size() == 0)
        {
            // Invalid host name
            throw WsmFault(
                WsmFault::wsa_InvalidMessageInformationHeader,
                MessageLoaderParms(
                    "WsmServer.WsmToCimRequestMapper.INVALID_EPR_ADDRESS",
                    "The EPR address \"$0\" is not valid", addr),
                    WSMAN_FAULTDETAIL_INVALIDVALUE);
        }
    }
    return hostName;
}

void WsmToCimRequestMapper::convertEPRToObjectPath(
    const WsmEndpointReference& epr,
    CIMObjectPath& objectPath)
{
    Array<CIMKeyBinding> keyBindings;
    CIMNamespaceName namespaceName;

    // Convert the ResourceURI to a CIM class name
    CIMName className = convertResourceUriToClassName(epr.resourceUri);

    PEGASUS_ASSERT(epr.selectorSet);

    // Determine the namespace from the selector set
    for (Uint32 i = 0, n = epr.selectorSet->selectors.size(); i < n; i++)
    {
        if (String::equalNoCase(
                epr.selectorSet->selectors[i].name, "__cimnamespace"))
        {
            if (epr.selectorSet->selectors[i].type != WsmSelector::VALUE)
            {
                throw WsmFault(
                    WsmFault::wsman_InvalidSelectors,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper."
                            "SELECTOR_TYPE_MISMATCH",
                        "Selector \"$0\" is not of the correct type.",
                        epr.selectorSet->selectors[i].name),
                    WSMAN_FAULTDETAIL_TYPEMISMATCH);
            }

            if (!CIMNamespaceName::legal(epr.selectorSet->selectors[i].value))
            {
                throw WsmFault(
                    WsmFault::wsman_InvalidSelectors,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper."
                            "INVALID_SELECTOR_VALUE",
                        "The value \"$0\" is not valid for selector \"$1\".",
                        epr.selectorSet->selectors[i].value,
                        epr.selectorSet->selectors[i].name),
                    WSMAN_FAULTDETAIL_INVALIDVALUE);
            }

            namespaceName = epr.selectorSet->selectors[i].value;
            break;
        }
    }

    if (namespaceName.isNull())
    {
        namespaceName = PEGASUS_DEFAULT_WSM_NAMESPACE;
    }

    CIMClass cimClass = _repository->getClass(
        namespaceName,
        className,
        false /*localOnly*/);

    for (Uint32 i = 0, n = epr.selectorSet->selectors.size(); i < n; i++)
    {
        if (!String::equalNoCase(
                epr.selectorSet->selectors[i].name, "__cimnamespace"))
        {
            Uint32 propertyPos;

            if (!CIMName::legal(epr.selectorSet->selectors[i].name) ||
                ((propertyPos = cimClass.findProperty(CIMNameCast(
                    epr.selectorSet->selectors[i].name))) == PEG_NOT_FOUND))
            {
                throw WsmFault(
                    WsmFault::wsman_InvalidSelectors,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper.UNEXPECTED_SELECTOR",
                        "Selector \"$0\" is not expected for a resource of "
                            "class $1 in namespace $2.",
                        epr.selectorSet->selectors[i].name,
                        className.getString(),
                        namespaceName.getString()),
                    WSMAN_FAULTDETAIL_UNEXPECTEDSELECTORS);
            }

            CIMProperty property = cimClass.getProperty(propertyPos);
            CIMKeyBinding newKeyBinding(
                CIMNameCast(epr.selectorSet->selectors[i].name),
                property.getValue());

            if (((newKeyBinding.getType() == CIMKeyBinding::REFERENCE) &&
                 (epr.selectorSet->selectors[i].type != WsmSelector::EPR)) ||
                ((newKeyBinding.getType() != CIMKeyBinding::REFERENCE) &&
                 (epr.selectorSet->selectors[i].type == WsmSelector::EPR)))
            {
                throw WsmFault(
                    WsmFault::wsman_InvalidSelectors,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper."
                            "SELECTOR_TYPE_MISMATCH",
                        "Selector \"$0\" is not of the correct type.",
                        epr.selectorSet->selectors[i].name),
                    WSMAN_FAULTDETAIL_TYPEMISMATCH);
            }

            if (epr.selectorSet->selectors[i].type == WsmSelector::VALUE)
            {
                newKeyBinding.setValue(epr.selectorSet->selectors[i].value);
            }
            else
            {
                CIMObjectPath reference;
                convertEPRToObjectPath(
                    epr.selectorSet->selectors[i].epr,
                    reference);
                newKeyBinding.setValue(reference.toString());
            }

            keyBindings.append(newKeyBinding);
        }
    }
    objectPath = CIMObjectPath(
        convertEPRAddressToHostname(epr.address),
        namespaceName,
        className,
        keyBindings);
}

void WsmToCimRequestMapper::convertWsmToCimInstance(
    WsmInstance& wsmInstance,
    const CIMNamespaceName& nameSpace,
    CIMInstance& cimInstance)
{
    CIMName className(wsmInstance.getClassName());
    CIMClass cimClass =
        _repository->getClass(nameSpace, className, false /* localOnly */);

    cimInstance = CIMInstance(className);
    for (Uint32 i = 0, n = wsmInstance.getPropertyCount(); i < n; i++)
    {
        WsmProperty& wsmProperty = wsmInstance.getProperty(i);
        const String& wsmPropName = wsmProperty.getName();
        WsmValue& wsmPropValue = wsmProperty.getValue();

        if (CIMName::legal(wsmPropName))
        {
            Uint32 cimPropIdx = cimClass.findProperty(CIMName(wsmPropName));
            if (cimPropIdx != PEG_NOT_FOUND)
            {
                CIMProperty cimProperty = cimClass.getProperty(cimPropIdx);
                CIMValue cimPropValue(cimProperty.getValue());

                convertWsmToCimValue(wsmPropValue, nameSpace, cimPropValue);
                cimProperty.setValue(cimPropValue);
                cimInstance.addProperty(cimProperty);
                continue;
            }
        }

        throw WsmFault(
            WsmFault::wsman_SchemaValidationError,
            MessageLoaderParms(
                "WsmServer.WsmToCimRequestMapper.NO_SUCH_PROPERTY",
                "The $0 property does not exist.",
                wsmPropName));
    }
}

void WsmToCimRequestMapper::convertWsmToCimValue(
    WsmValue& wsmValue,
    const CIMNamespaceName& nameSpace,
    CIMValue& cimValue)
{
    if (wsmValue.isNull())
    {
        cimValue.setNullValue(
            cimValue.getType(), cimValue.isArray(), cimValue.getArraySize());
        return;
    }

    if (cimValue.isArray())
    {
        wsmValue.toArray();
        switch (wsmValue.getType())
        {
            case WSMTYPE_REFERENCE:
            {
                Array<WsmEndpointReference> eprs;
                Array<CIMObjectPath> objPaths;
                wsmValue.get(eprs);
                for (Uint32 i = 0, n = eprs.size(); i < n; i++)
                {
                    CIMObjectPath objPath;
                    convertEPRToObjectPath(eprs[i], objPath);
                    objPaths.append(objPath);
                }
                cimValue.set(objPaths);
                break;
            }

            case WSMTYPE_INSTANCE:
            {
                Array<WsmInstance> wsmInst;
                Array<CIMInstance> cimInst;
                wsmValue.get(wsmInst);
                for (Uint32 i = 0, n = wsmInst.size(); i < n; i++)
                {
                    CIMInstance inst;
                    convertWsmToCimInstance(wsmInst[i], nameSpace, inst);
                    cimInst.append(inst);
                }
                cimValue.set(cimInst);
                break;
            }

            case WSMTYPE_OTHER:
            {
                Array<String> strs;
                wsmValue.get(strs);
                convertStringArrayToCimValue(
                    strs, cimValue.getType(), cimValue);
                break;
            }

            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            }
        }
    }
    else
    {
        if (wsmValue.isArray())
        {
            throw TypeMismatchException();
        }

        switch (wsmValue.getType())
        {
            case WSMTYPE_REFERENCE:
            {
                WsmEndpointReference epr;
                CIMObjectPath objPath;
                wsmValue.get(epr);
                convertEPRToObjectPath(epr, objPath);
                cimValue.set(objPath);
                break;
            }

            case WSMTYPE_INSTANCE:
            {
                WsmInstance wsmInst;
                CIMInstance cimInst;
                wsmValue.get(wsmInst);
                convertWsmToCimInstance(wsmInst, nameSpace, cimInst);
                cimValue.set(cimInst);
                break;
            }

            case WSMTYPE_OTHER:
            {
                String str;
                wsmValue.get(str);
                convertStringToCimValue(str, cimValue.getType(), cimValue);
                break;
            }

            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            }
        }
    }
}

void WsmToCimRequestMapper::convertStringToCimValue(
    const String& str,
    CIMType cimType,
    CIMValue& cimValue)
{
    switch (cimType)
    {
        case CIMTYPE_BOOLEAN:
        {
            Boolean val;
            if (String::compare(str, "true") == 0 ||
                String::compare(str, "1") == 0)
            {
                val = 1;
            }
            else if (String::compare(str, "false") == 0 ||
                     String::compare(str, "0") == 0)
            {
                val = 0;
            }
            else
            {
                throw WsmFault(
                    WsmFault::wxf_InvalidRepresentation,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper.INVALID_BOOLEAN_VALUE",
                        "The boolean value \"$0\" is not valid", str),
                    WSMAN_FAULTDETAIL_INVALIDVALUE);
            }
            cimValue.set(val);
            break;
        }

        case CIMTYPE_UINT8:
        case CIMTYPE_UINT16:
        case CIMTYPE_UINT32:
        case CIMTYPE_UINT64:
        {
            Uint64 val;
            if (!StringConversion::stringToUnsignedInteger(
                (const char*) str.getCString(), val))
            {
                throw WsmFault(
                    WsmFault::wxf_InvalidRepresentation,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper.INVALID_UI_VALUE",
                        "The unsigned integer value \"$0\" is not valid", str),
                    WSMAN_FAULTDETAIL_INVALIDVALUE);
            }

            switch (cimType)
            {
                case CIMTYPE_UINT8:
                {
                    if (!StringConversion::checkUintBounds(val, cimType))
                    {
                        throw WsmFault(
                            WsmFault::wxf_InvalidRepresentation,
                            MessageLoaderParms(
                                "WsmServer.WsmToCimRequestMapper."
                                "VALUE_OUT_OF_RANGE",
                                "The $0 value \"$1\" is out of range",
                                "Uint8", str),
                            WSMAN_FAULTDETAIL_INVALIDVALUE);
                    }
                    cimValue.set(Uint8(val));
                    break;
                }
                case CIMTYPE_UINT16:
                {
                    if (!StringConversion::checkUintBounds(val, cimType))
                    {
                        throw WsmFault(
                            WsmFault::wxf_InvalidRepresentation,
                            MessageLoaderParms(
                                "WsmServer.WsmToCimRequestMapper."
                                "VALUE_OUT_OF_RANGE",
                                "The $0 value \"$1\" is out of range",
                                "Uint16", str),
                            WSMAN_FAULTDETAIL_INVALIDVALUE);
                    }
                    cimValue.set(Uint16(val));
                    break;
                }
                case CIMTYPE_UINT32:
                {
                    if (!StringConversion::checkUintBounds(val, cimType))
                    {
                        throw WsmFault(
                            WsmFault::wxf_InvalidRepresentation,
                            MessageLoaderParms(
                                "WsmServer.WsmToCimRequestMapper."
                                "VALUE_OUT_OF_RANGE",
                                "The $0 value \"$1\" is out of range",
                                "Uint32", str),
                            WSMAN_FAULTDETAIL_INVALIDVALUE);
                    }
                    cimValue.set(Uint32(val));
                    break;
                }
                case CIMTYPE_UINT64:
                {
                    cimValue.set(Uint64(val));
                    break;
                }
                default:
                {
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                }
            }
            break;
        }

        case CIMTYPE_SINT8:
        case CIMTYPE_SINT16:
        case CIMTYPE_SINT32:
        case CIMTYPE_SINT64:
        {
            Sint64 val;
            if (!StringConversion::stringToSignedInteger(
                (const char*) str.getCString(), val))
            {
                throw WsmFault(
                    WsmFault::wxf_InvalidRepresentation,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper.INVALID_SI_VALUE",
                        "The signed integer value \"$0\" is not valid", str),
                    WSMAN_FAULTDETAIL_INVALIDVALUE);
            }

            switch (cimType)
            {
                case CIMTYPE_SINT8:
                {
                    if (!StringConversion::checkSintBounds(val, cimType))
                    {
                        throw WsmFault(
                            WsmFault::wxf_InvalidRepresentation,
                            MessageLoaderParms(
                                "WsmServer.WsmToCimRequestMapper."
                                "VALUE_OUT_OF_RANGE",
                                "The $0 value \"$1\" is out of range",
                                "Sint8", str),
                            WSMAN_FAULTDETAIL_INVALIDVALUE);
                    }
                    cimValue.set(Sint8(val));
                    break;
                }
                case CIMTYPE_SINT16:
                {
                    if (!StringConversion::checkSintBounds(val, cimType))
                    {
                        throw WsmFault(
                            WsmFault::wxf_InvalidRepresentation,
                            MessageLoaderParms(
                                "WsmServer.WsmToCimRequestMapper."
                                "VALUE_OUT_OF_RANGE",
                                "The $0 value \"$1\" is out of range",
                                "Sint16", str),
                            WSMAN_FAULTDETAIL_INVALIDVALUE);
                    }
                    cimValue.set(Sint16(val));
                    break;
                }
                case CIMTYPE_SINT32:
                {
                    if (!StringConversion::checkSintBounds(val, cimType))
                    {
                        throw WsmFault(
                            WsmFault::wxf_InvalidRepresentation,
                            MessageLoaderParms(
                                "WsmServer.WsmToCimRequestMapper."
                                "VALUE_OUT_OF_RANGE",
                                "The $0 value \"$1\" is out of range",
                                "Sint32", str),
                            WSMAN_FAULTDETAIL_INVALIDVALUE);
                    }
                    cimValue.set(Sint32(val));
                    break;
                }
                case CIMTYPE_SINT64:
                {
                    cimValue.set(Sint64(val));
                    break;
                }
                default:
                {
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                }
            }
            break;
        }

        case CIMTYPE_REAL32:
        {
            Real64 val;
            if (!stringToReal64(
                (const char*) str.getCString(), val))
            {
                throw WsmFault(
                    WsmFault::wxf_InvalidRepresentation,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper.INVALID_RN_VALUE",
                        "The real number value \"$0\" is not valid", str),
                    WSMAN_FAULTDETAIL_INVALIDVALUE);
            }
            cimValue.set(Real32(val));
            break;
        }

        case CIMTYPE_REAL64:
        {
            Real64 val;
            if (!stringToReal64(
                (const char*) str.getCString(), val))
            {
                throw WsmFault(
                    WsmFault::wxf_InvalidRepresentation,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper.INVALID_RN_VALUE",
                        "The real number value \"$0\" is not valid", str),
                    WSMAN_FAULTDETAIL_INVALIDVALUE);
            }
            cimValue.set(val);
            break;
        }

        case CIMTYPE_CHAR16:
        {
            if (str.size() != 1)
            {
                throw WsmFault(
                    WsmFault::wxf_InvalidRepresentation,
                    MessageLoaderParms(
                        "WsmServer.WsmToCimRequestMapper.INVALID_CHAR16_VALUE",
                        "The char16 value \"$0\" is not valid", str),
                    WSMAN_FAULTDETAIL_INVALIDVALUE);
            }
            cimValue.set(str[0]);
            break;
        }

        case CIMTYPE_STRING:
        {
            cimValue.set(str);
            break;
        }

        case CIMTYPE_DATETIME:
        {
            CIMDateTime cimDT;
            convertWsmToCimDatetime(str, cimDT);
            cimValue.set(cimDT);
            break;
        }

        default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
        }
    }
}

template<class T>
void _convertStringArrayToCimValueAux(
    const Array<String>& strs,
    CIMType cimType,
    CIMValue& cimValue)
{
    Array<T> arr;
    for (Uint32 i = 0, n = strs.size(); i < n; i++)
    {
        T val;
        CIMValue tmp;
        WsmToCimRequestMapper::convertStringToCimValue(strs[i], cimType, tmp);
        tmp.get(val);
        arr.append(val);
    }
    cimValue.set(arr);
}

void WsmToCimRequestMapper::convertStringArrayToCimValue(
    const Array<String>& strs,
    CIMType cimType,
    CIMValue& cimValue)
{
        switch (cimValue.getType())
        {
            case CIMTYPE_BOOLEAN:
                _convertStringArrayToCimValueAux<Boolean>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_UINT8:
                _convertStringArrayToCimValueAux<Uint8>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_SINT8:
                _convertStringArrayToCimValueAux<Sint8>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_UINT16:
                _convertStringArrayToCimValueAux<Uint16>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_SINT16:
                _convertStringArrayToCimValueAux<Sint16>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_UINT32:
                _convertStringArrayToCimValueAux<Uint32>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_SINT32:
                _convertStringArrayToCimValueAux<Sint32>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_UINT64:
                _convertStringArrayToCimValueAux<Uint64>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_SINT64:
                _convertStringArrayToCimValueAux<Sint64>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_REAL32:
                _convertStringArrayToCimValueAux<Real32>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_REAL64:
                _convertStringArrayToCimValueAux<Real64>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_CHAR16:
                _convertStringArrayToCimValueAux<Char16>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_STRING:
                _convertStringArrayToCimValueAux<String>(
                    strs, cimType, cimValue);
                break;
            case CIMTYPE_DATETIME:
            {
                Array<CIMDateTime> cimDTs;
                for (Uint32 i = 0, n = strs.size(); i < n; i++)
                {
                    CIMDateTime cimDT;
                    convertWsmToCimDatetime(strs[i], cimDT);
                    cimDTs.append(cimDT);
                }
                cimValue.set(cimDTs);

                break;
            }
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
        }
}

#define illegalNumChar(c) (c == '+' || c == '-' || c == ' ' || c == '\t')

static Uint32 _getMicroseconds(const char* ptr, int* bytes)
{
    // Here we expect an unsigned integer with no white space or sign.
    // Essentially we expect to see a sequence of decimal digits.
    Uint32 ms;

    // Read only the first 6 digits, since we only want microseconds.
    int conversions = sscanf(ptr, "%6u%n", &ms, bytes);

    if (conversions == 0 || *bytes == 0 || illegalNumChar(*ptr))
    {
        throw InvalidDateTimeFormatException();
    }

    // Convert the fractional number of seconds we've just read into the
    // number of microseconds. Since we can only represent 6 decimal
    // positions, discard any additional digits or pad the number with
    // trailing 0's if we've read less than 6.
    if (*bytes == 6)
    {
        // Skip digits less significant than microseconds
        while (isdigit(*(ptr + *bytes)))
        {
            (*bytes)++;
        }
    }
    else
    {
        // Convert to microseconds by padding with 0's
        for (Sint32 i = 0; i < 6 - *bytes; i++)
        {
            ms *= 10;
        }
    }

    return ms;
}

void WsmToCimRequestMapper::convertWsmToCimDatetime(
    const String& wsmDT, CIMDateTime& cimDT)
{
    Uint32 strSize = wsmDT.size();
    CString wsmCStr = wsmDT.getCString();
    const char* wsmStr = (const char*) wsmCStr;

    try
    {
        // The shortest valid representation is an interval, e.g. P1Y
        // Negative datetime/intervals not supported
        if (strSize < 3 || wsmStr[0] == '-')
        {
            throw InvalidDateTimeFormatException();
        }

        Uint32 pos;
        if (wsmStr[0] == 'P')
        {
            // Interval
            // The format is PnYnMnDTnHnMnS, where
            // nY represents the number of years,
            // nM the number of months, nD the number of days,
            // 'T' is the date/time separator,
            // nH the number of hours,
            // nM the number of minutes and
            // nS the number of seconds.
            // The number of seconds can include decimal digits to
            // arbitrary precision.
            struct
            {
                Uint32 num;
                char id;
            } values[7] = {{0, 'Y'}, {0, 'M'}, {0, 'D'},
                {0, 'H'}, {0, 'M'}, {0, 'S'}, {0, 0}};

            const char* ptr = wsmStr + 1;
            Uint32 i = 0;
            Boolean seenT = false;
            while (ptr < wsmStr + strSize && i < 6)
            {
                if (*ptr == 'T')
                {
                    ptr++;
                    seenT = true;

                    // The 'T' separator cannot appear after 'H' and must
                    // be followed by hours, minutes or seconds
                    if (i > 3 || *ptr == '\0')
                    {
                        throw InvalidDateTimeFormatException();
                    }

                    i = 3;
                    continue;
                }

                // If we're processing hours, minutes or seconds but have not
                // seen the 'T' separator, it's an error.
                if (i >= 3 && !seenT)
                {
                    throw InvalidDateTimeFormatException();
                }

                int bytes = 0;
                Uint32 num = 0;
                int conversions = sscanf(ptr, "%u%n", &num, &bytes);

                // Here we expect a valid unsigned int with the maximum
                // value of 999,999,999
                if (conversions == 0 || bytes == 0 || bytes > 9 ||
                    illegalNumChar(*ptr))
                {
                    throw InvalidDateTimeFormatException();
                }

                char c = *(ptr + bytes);
                if (c == values[i].id)
                {
                    values[i].num = num;
                    ptr = ptr + bytes + 1;
                }
                else if (c == '.')
                {
                    // Special case: handle fractional seconds when the number
                    // of seconds is followed by '.' rather than 'S'
                    if (!seenT)
                    {
                        throw InvalidDateTimeFormatException();
                    }

                    values[5].num = num;
                    ptr = ptr + bytes + 1; // ptr points to byte after '.'
                    values[6].num = _getMicroseconds(ptr, &bytes);
                    if (*(ptr + bytes) != 'S')
                    {
                        throw InvalidDateTimeFormatException();
                    }
                    ptr = ptr + bytes + 1;
                    i = 5;
                }
                else if (c != 'Y' && c != 'M' && c != 'D' &&
                         c != 'H' && c != 'S')
                {
                    throw InvalidDateTimeFormatException();
                }

                i++;
            }

            // If at the end of the loop we still have unconsumed charachters,
            // it's an error.
            if (ptr < wsmStr + strSize)
            {
                throw InvalidDateTimeFormatException();
            }

            Uint32 msecs = values[6].num;
            Uint32 secs = values[5].num % 60;
            values[4].num += values[5].num / 60;
            Uint32 mins = values[4].num % 60;
            values[3].num += values[4].num / 60;
            Uint32 hrs = values[3].num % 24;
            values[2].num += values[3].num / 24;

            // It's impossible to calculate the exact number of days.
            // Here are the assumptions:
            // - a year has 365 days
            // - every 4th year adds an extra day
            // - a month has 30 days
            // - every other month adds an extra day
            Uint64 days = (values[0].num * 365) + (values[0].num / 4) +
                (values[1].num * 30) + (values[1].num / 2) + values[2].num;

            // Make sure that the value is within Uint32 bounds
            if (days & PEGASUS_UINT64_LITERAL(0xFFFFFFFF00000000))
            {
                throw InvalidDateTimeFormatException();
            }

            cimDT.setInterval((Uint32) days, hrs, mins, secs, msecs, 6);
        }
        else if (wsmDT.find('T') != PEG_NOT_FOUND)
        {
            // datetime
            // YYYY-MM-DDThh:mm:ss[.ssss...][Z][+/-hh:mm]
            Uint32 year = 0, month = 0, day = 0, hrs = 0, mins = 0, secs = 0,
                msecs = 0, utch = 0, utcm = 0;
            Sint32 utcoff = 0;
            char sign = 0;
            const char* ptr = wsmStr;
            int bytes = 0;

            // Read all fields up to but excluding potential fractional seconds
            int conversions = sscanf(ptr, "%4u-%2u-%2uT%2u:%2u:%2u%c%n",
                 &year, &month, &day, &hrs, &mins, &secs, &sign, &bytes);

            // Year, month, day, hours, minutes and seconds must be present
            if ((conversions < 6) ||
                // If only 6 fields read, the string must be 19 chareacters
                (conversions == 6 && strSize != 19) ||
                // If sign is present, it must be either 'Z', '+' or '-' UTC
                // seperators, or '.' if there are fractional seconds
                (conversions == 7 &&
                 sign != 'Z' && sign != '+' && sign != '-' && sign != '.') ||
                // If 'Z' is the sign, it must be the last char in the string
                (sign == 'Z' && strSize != 20) ||
                // Make sure that separators are in the proper positions
                ptr[4] != '-' || ptr[7] != '-' || ptr[10] != 'T' ||
                ptr[13] != ':' || ptr[16] != ':' ||
                // Make sure that numeric fields do not start with white
                // space, '+' or '-' signs
                illegalNumChar(ptr[0]) || illegalNumChar(ptr[5]) ||
                illegalNumChar(ptr[8]) || illegalNumChar(ptr[11]) ||
                illegalNumChar(ptr[14]) || illegalNumChar(ptr[17]))
            {
                throw InvalidDateTimeFormatException();
            }

            ptr += bytes;
            if (sign == '.')
            {
                msecs = _getMicroseconds(ptr, &bytes);
                ptr += bytes;

                conversions = sscanf(ptr, "%c", &sign);
                if ((conversions == 0) ||
                    (conversions == 1 &&
                     sign != 'Z' && sign != '+' && sign != '-'))
                {
                    throw InvalidDateTimeFormatException();
                }

                ptr++; // account for sign
            }

            // Read UTC offset
            if (sign == '+' || sign == '-')
            {
                conversions = sscanf(ptr, "%2u:%2u", &utch, &utcm);
                if (conversions != 2 || strlen(ptr) != 5 ||
                    // Make sure that numeric fields do not start with white
                    // space, '+' or '-' signs
                    illegalNumChar(ptr[0]) || illegalNumChar(ptr[2]) ||
                    // Hours and minutes must be within range
                    utch >= 24 || utcm >= 60)
                {
                    throw InvalidDateTimeFormatException();
                }

                utcoff = utch * 60 + utcm;
                if (sign == '-')
                {
                    utcoff *= -1;
                }
            }
            else if (sign == 'Z')
            {
                // No need to do anything here: the offset is already
                // initialized to 0.
                // Just make sure that 'Z' is the last char in the string.
                if (*ptr != 0)
                {
                    throw InvalidDateTimeFormatException();
                }
            }

            cimDT.setTimeStamp(year, month, day, hrs, mins, secs,
                msecs, 6, utcoff);
        }
        else if (((pos = wsmDT.find('-')) != PEG_NOT_FOUND) &&
                 (wsmDT.find(pos + 1, '-') != PEG_NOT_FOUND))
        {
            // date
            // The format is YYYY-MM-DD[Z][+/-hh:mm]
            Uint32 year = 0, month = 0, day = 0, utch = 0, utcm = 0;
            Sint32 utcoff = 0;
            char sign = 0;

            int conversions = sscanf(wsmStr, "%4u-%2u-%2u%c%2u:%2u",
                &year, &month, &day, &sign, &utch, &utcm);

            // Year, month and day must be present
            if ((conversions < 3) ||
                // The string with no UTC offset must be 10 chars
                (conversions == 3 && strSize != 10) ||
                // Make sure that seperators are in the proper positions
                wsmStr[4] != '-' || wsmStr[7] != '-' ||
                // Make sure that numeric fields do not start with white
                // space, '+' or '-' signs
                illegalNumChar(wsmStr[0]) || illegalNumChar(wsmStr[5]) ||
                illegalNumChar(wsmStr[8]) ||
                // Hours and minutes must be within range
                utch >= 24 || utcm >= 60)
            {
                throw InvalidDateTimeFormatException();
            }

            // Decode UTC offset
            if (conversions > 3)
            {
                if ((sign != 'Z' && sign != '+' && sign != '-') ||
                    ((conversions == 4 || sign == 'Z') && strSize != 11))
                {
                    throw InvalidDateTimeFormatException();
                }

                if (sign == '+' || sign == '-')
                {
                    if (conversions < 6 || strSize != 16 ||
                        wsmStr[13] != ':' ||
                        // Make sure that numeric fields do not start with
                        // white space, '+' or '-' signs
                        illegalNumChar(wsmStr[11]) ||
                        illegalNumChar(wsmStr[14]))
                    {
                        throw InvalidDateTimeFormatException();
                    }

                    utcoff = utch * 60 + utcm;
                    if (sign == '-')
                    {
                        utcoff *= -1;
                    }
                }
            }

            cimDT.setTimeStamp(year, month, day, CIMDateTime::WILDCARD,
                CIMDateTime::WILDCARD, CIMDateTime::WILDCARD, 0, 0, utcoff);
        }
        else
        {
            cimDT.set(wsmDT);
        }
    }
    catch (InvalidDateTimeFormatException&)
    {
        throw WsmFault(
            WsmFault::wxf_InvalidRepresentation,
            MessageLoaderParms(
                "WsmServer.WsmToCimRequestMapper.INVALID_DT_VALUE",
                "The datetime value \"$0\" is not valid", wsmDT),
            WSMAN_FAULTDETAIL_INVALIDVALUE);
    }
    catch (DateTimeOutOfRangeException&)
    {
        throw WsmFault(
            WsmFault::wxf_InvalidRepresentation,
            MessageLoaderParms(
                "WsmServer.WsmToCimRequestMapper.INVALID_DT_VALUE",
                "The datetime value \"$0\" is not valid", wsmDT),
            WSMAN_FAULTDETAIL_INVALIDVALUE);
    }
}

void WsmToCimRequestMapper::convertWsmToCimParameters(
    const CIMNamespaceName& nameSpace,
    const String& className,
    const String& methodName,
    WsmInstance& instance,
    Array<CIMParamValue>& parameters)
{
    parameters.clear();

    // Get class from repository.

    CIMClass cc = _repository->getClass(nameSpace, CIMName(className), false);

    // Look up the method.

    Uint32 pos = cc.findMethod(methodName);

    if (pos == PEG_NOT_FOUND)
    {
        MessageLoaderParms params(
            "WsmServer.WsmToCimRequestMapper.NO_SUCH_METHOD",
            "The $0 method does not exist.",
            methodName);
        throw WsmFault(WsmFault::wsman_SchemaValidationError, params);
    }

    CIMMethod cm = cc.getMethod(pos);

    // Translate parameters:

    for (Uint32 i = 0, n = instance.getPropertyCount(); i < n; i++)
    {
        WsmProperty& wp = instance.getProperty(i);
        const String& wname = wp.getName();
        WsmValue& wvalue= wp.getValue();

        if (CIMName::legal(wname))
        {
            Uint32 pos = cm.findParameter(CIMName(wname));

            if (pos != PEG_NOT_FOUND)
            {
                CIMParameter cp = cm.getParameter(pos);

                // Reject if not an input parameter.
                if (_isInputParameter(cp))
                {
                    const CIMName& cn = cp.getName();
                    CIMValue cv(cp.getType(), cp.isArray());
                    convertWsmToCimValue(wvalue, nameSpace, cv);
                    parameters.append(CIMParamValue(cn.getString(), cv));
                    continue;
                }
            }
        }

        MessageLoaderParms params(
            "WsmServer.WsmToCimRequestMapper.NO_SUCH_PARAMETER",
            "The $0 input parameter does not exist.", wname);
        throw WsmFault(WsmFault::wsman_SchemaValidationError, params);
    }
}

// Values have a lexical representation consisting of a mantissa followed,
// optionally, by the character "E" or "e", followed by an exponent.
// The exponent must be an integer. The mantissa must be a decimal number.
// The representations for exponent and mantissa must follow the lexical
// rules for integer and decimal. If the "E" or "e" and the following
// exponent are omitted, an exponent value of 0 is assumed.
// The special values positive and negative zero, positive and negative
// infinity and not-a-number have lexical representations 0, -0, INF, -INF
// and NaN, respectively.
Boolean WsmToCimRequestMapper::stringToReal64(
    const char* stringValue,
    Real64& x)
{
    char* end;
    const char* p = stringValue;

    if (!p || !*p)
    {
        return false;
    }

    errno = 0;
    if (!isdigit(*p))
    {
        // If it doesn't start with a digit, it can only be NaN, INF or -INF
        if (strlen(p) < 3 ||
            ((*p != 'N' || *(p + 1) != 'a' || *(p + 2) != 'N' || *(p + 3)) &&
            (*p != 'I' || *(p + 1) != 'N' || *(p + 2) != 'F' || *(p + 3)) &&
            (*p != '-' || *(p + 1) != 'I' || *(p + 2) != 'N' ||
             *(p + 3) != 'F' || *(p + 4))))
        {
            return false;
        }

        // Do the conversion
        x = strtod(stringValue, &end);
        // HP-UX strtod sets errno to ERANGE for NaN and INF
        return (!*end);
    }
    else
    {
        // It can't be a hex number
        if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X'))
        {
            return false;
        }

        // Do the conversion
        x = strtod(stringValue, &end);
        return (!*end && (errno != ERANGE));
    }
}

PEGASUS_NAMESPACE_END
