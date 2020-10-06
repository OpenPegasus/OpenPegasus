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
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/WQL/WQLParser.h>
#include <Pegasus/Common/Tracer.h>
#include "CimToWsmResponseMapper.h"

#ifdef PEGASUS_OS_VMS
# define PEGASUS_NAN "NaNQ"
# define PEGASUS_INF "Infinity"
# define PEGASUS_NEG_INF "-Infinity"
#else
# define PEGASUS_NAN "nan"
# define PEGASUS_INF "inf"
# define PEGASUS_NEG_INF "-inf"
#endif

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

CimToWsmResponseMapper::CimToWsmResponseMapper()
{
}

CimToWsmResponseMapper::~CimToWsmResponseMapper()
{
}

WsmResponse* CimToWsmResponseMapper::mapToWsmResponse(
    const WsmRequest* wsmRequest,
    const CIMResponseMessage* message)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER, "CimToWsmResponseMapper::mapToWsmResponse");
    AutoPtr<WsmResponse> wsmResponse;

    if (message->cimException.getCode() != CIM_ERR_SUCCESS)
    {
        wsmResponse.reset(_mapToWsmFaultResponse(wsmRequest, message));
    }
    else
    {
        switch (wsmRequest->getType())
        {
            case WS_TRANSFER_GET:
                wsmResponse.reset(_mapToWxfGetResponse(
                    (WxfGetRequest*) wsmRequest,
                    (CIMGetInstanceResponseMessage*) message));
                break;

            case WS_TRANSFER_PUT:
                wsmResponse.reset(_mapToWxfPutResponse(
                    (WxfPutRequest*) wsmRequest,
                    (CIMModifyInstanceResponseMessage*) message));
                break;

            case WS_SUBSCRIPTION_CREATE:
                wsmResponse.reset(_mapToWxfSubCreateResponse(
                    (WxfSubCreateRequest*) wsmRequest,
                    (CIMCreateInstanceResponseMessage*) message));
                break;

            case WS_TRANSFER_CREATE:
                wsmResponse.reset(_mapToWxfCreateResponse(
                    (WxfCreateRequest*) wsmRequest,
                    (CIMCreateInstanceResponseMessage*) message));
                break;

            case WS_TRANSFER_DELETE:
                wsmResponse.reset(_mapToWxfDeleteResponse(
                    (WxfDeleteRequest*) wsmRequest,
                    (CIMDeleteInstanceResponseMessage*) message));
                break;

            case WS_SUBSCRIPTION_DELETE:  
                wsmResponse.reset(_mapToWxfSubDeleteResponse(
                    (WxfSubDeleteRequest*) wsmRequest,
                    (CIMDeleteInstanceResponseMessage*) message));
                break;

            case WS_ENUMERATION_ENUMERATE:
                // Test for no association filter
                if (((WsenEnumerateRequest*)wsmRequest)->
                    wsmFilter.filterDialect != WsmFilter::ASSOCIATION)
                {
                    if (((WsenEnumerateRequest*) wsmRequest)->enumerationMode ==
                        WSEN_EM_OBJECT)
                    {
                        wsmResponse.reset(_mapToWsenEnumerateResponseObject(
                            (WsenEnumerateRequest*) wsmRequest,
                            (CIMEnumerateInstancesResponseMessage*) message));
                    }
                    else if (((WsenEnumerateRequest*)wsmRequest)->
                             enumerationMode ==
                                 WSEN_EM_OBJECT_AND_EPR)
                    {
                        wsmResponse.reset(
                            _mapToWsenEnumerateResponseObjectAndEPR(
                              (WsenEnumerateRequest*) wsmRequest,
                              (CIMEnumerateInstancesResponseMessage*) message));
                    }
                    else if (((WsenEnumerateRequest*) wsmRequest)->
                             enumerationMode == WSEN_EM_EPR)
                    {
                        wsmResponse.reset(_mapToWsenEnumerateResponseEPR(
                            (WsenEnumerateRequest*) wsmRequest,
                            (CIMEnumerateInstanceNamesResponseMessage*)
                                message));
                    }
                    else
                    {
                        PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                    }
                }
                else // association or reference response expected
                {
                    if (((WsenEnumerateRequest*)wsmRequest)->
                        wsmFilter.AssocFilter.assocFilterType ==
                           WsmFilter::ASSOCIATED_INSTANCES)
                    {
                        // Association responses
                        if (((WsenEnumerateRequest*) wsmRequest)->
                            enumerationMode == WSEN_EM_OBJECT)
                        {
                            wsmResponse.reset(
                                _mapToWsenEnumerateResponseObject(
                                    (WsenEnumerateRequest*) wsmRequest,
                                    (CIMAssociatorsResponseMessage*) message));
                        }
                        else if (((WsenEnumerateRequest*)
                                      wsmRequest)->enumerationMode ==
                                 WSEN_EM_OBJECT_AND_EPR)
                        {
                            wsmResponse.reset(
                                _mapToWsenEnumerateResponseObjectAndEPR(
                                    (WsenEnumerateRequest*) wsmRequest,
                                    (CIMAssociatorsResponseMessage*) message));
                        }
                        else if (((WsenEnumerateRequest*) wsmRequest)->
                                 enumerationMode == WSEN_EM_EPR)
                        {
                            wsmResponse.reset(_mapToWsenEnumerateResponseEPR(
                                (WsenEnumerateRequest*) wsmRequest,
                                (CIMAssociatorNamesResponseMessage*) message));
                        }
                        else
                        {
                            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                        }
                    }
                    else // references responses
                    {
                        if (((WsenEnumerateRequest*)
                             wsmRequest)->enumerationMode == WSEN_EM_OBJECT)
                        {
                            wsmResponse.reset(_mapToWsenEnumerateResponseObject(
                                (WsenEnumerateRequest*) wsmRequest,
                                (CIMReferencesResponseMessage*) message));
                        }
                        else if (((WsenEnumerateRequest*)
                                      wsmRequest)->enumerationMode ==
                                 WSEN_EM_OBJECT_AND_EPR)
                        {
                            wsmResponse.reset(
                                _mapToWsenEnumerateResponseObjectAndEPR(
                                    (WsenEnumerateRequest*) wsmRequest,
                                    (CIMReferencesResponseMessage*) message));
                        }
                        else if (((WsenEnumerateRequest*) wsmRequest)->
                                 enumerationMode == WSEN_EM_EPR)
                        {
                            wsmResponse.reset(_mapToWsenEnumerateResponseEPR(
                                (WsenEnumerateRequest*) wsmRequest,
                                (CIMReferenceNamesResponseMessage*) message));
                        }
                        else
                        {
                            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                        }
                    }
                }
                break;

            case WS_INVOKE:
            {
                wsmResponse.reset(_mapToWsInvokeResponse(
                    (WsInvokeRequest*)wsmRequest,
                    (CIMInvokeMethodResponseMessage*)message));
                break;
            }

            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }
    }
    PEG_METHOD_EXIT();
    return wsmResponse.release();
}

WsmFaultResponse* CimToWsmResponseMapper::_mapToWsmFaultResponse(
    const WsmRequest* wsmRequest,
    const CIMResponseMessage* response)
{
    WsmFault fault = mapCimExceptionToWsmFault(response->cimException);

    WsmFaultResponse* wsmResponse = new WsmFaultResponse(wsmRequest, fault);

    return wsmResponse;
}

WsmFault CimToWsmResponseMapper::mapCimExceptionToWsmFault(
    const CIMException& cimException)
{
    WsmFault::Subcode subcode;
    const ContentLanguageList& languageList =
        cimException.getContentLanguages();
    String reason = TraceableCIMException(cimException).getDescription();
    String faultDetail;

    switch (cimException.getCode())
    {
        case CIM_ERR_FAILED:                // Too general to specify fault
        case CIM_ERR_CLASS_HAS_CHILDREN:    // Only ModifyClass and DeleteClass
        case CIM_ERR_CLASS_HAS_INSTANCES:   // Only ModifyClass and DeleteClass
        case CIM_ERR_INVALID_SUPERCLASS:    // Only CreateClass and ModifyClass
        // These could map to wsa_ActionNotSupported if InvokeMethod is
        // supported through Custom Actions.
        case CIM_ERR_METHOD_NOT_FOUND:      // Only InvokeMethod
        case CIM_ERR_METHOD_NOT_AVAILABLE:  // Only InvokeMethod
        case CIM_ERR_NO_SUCH_PROPERTY:      // Only GetProperty and SetProperty
        case CIM_ERR_TYPE_MISMATCH:         // Only SetProperty
            subcode = WsmFault::wsman_InternalError;
            break;

        case CIM_ERR_ACCESS_DENIED:
            subcode = WsmFault::wsman_AccessDenied;
            break;

        case CIM_ERR_ALREADY_EXISTS:
            subcode = WsmFault::wsman_AlreadyExists;
            break;

        case CIM_ERR_INVALID_CLASS:
            subcode = WsmFault::wsa_DestinationUnreachable;
            faultDetail = WSMAN_FAULTDETAIL_INVALIDRESOURCEURI;
            break;

        case CIM_ERR_INVALID_NAMESPACE:
            // Consider wsman_InvalidSelectors?
            subcode = WsmFault::wsa_DestinationUnreachable;
            // faultDetail is not set to WSMAN_FAULTDETAIL_INVALIDRESOURCEURI
            // since this error reflects an incorrect Selector value rather
            // than an incorrect ResourceURI.
            break;

        case CIM_ERR_INVALID_PARAMETER:
            // For InvokeMethod, this would map to wsman_InvalidParameter.
            // It is difficult to discern the correct fault for other
            // operations.  It make sense to use wxf_InvalidRepresentation for
            // Create and Put, and wsman_InvalidSelectors for Get and Put.
            subcode = WsmFault::wsman_InvalidParameter;
            break;

        case CIM_ERR_INVALID_QUERY:
            // Note that Enumerate operations have a different subcode:
            // wsen_CannotProcessFilter.
            subcode = WsmFault::wsen_CannotProcessFilter;
            break;

        case CIM_ERR_NOT_FOUND:
            // DSP0226 Table 10 of master faults calls for
            // DestinationUnreachable in cases when the resource is not found.
            subcode = WsmFault::wsa_DestinationUnreachable;
            break;

        case CIM_ERR_NOT_SUPPORTED:
            subcode = WsmFault::wsa_ActionNotSupported;
            faultDetail = WSMAN_FAULTDETAIL_ACTIONMISMATCH;
            break;

        case CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED:
            // DSP0227 section 15.1.11 indicates that ExecuteQuery operations
            // through WS-Management use CQL filter dialect.  If this status
            // code results, it is fair to assume filtering is not supported
            // at all.  Another option would be to use
            // wsen_FilterDialectRequestedUnavailable.
            subcode = WsmFault::wsen_FilteringNotSupported;
            break;

        default:
            // Initialize to prevent uninitialized subcode error.
            subcode = WsmFault::wsman_InternalError;
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }

    return WsmFault(subcode, reason, languageList, faultDetail);
}

WxfGetResponse* CimToWsmResponseMapper::_mapToWxfGetResponse(
    const WxfGetRequest* wsmRequest,
    CIMGetInstanceResponseMessage* response)
{
    WsmInstance wsmInstance;

    convertCimToWsmInstance(
        wsmRequest->epr.resourceUri,
        response->getResponseData().getInstance(),
        wsmInstance,
        wsmRequest->epr.getNamespace());

    WxfGetResponse* wsmResponse =
        new WxfGetResponse(
            wsmInstance,
            wsmRequest,
            _getContentLanguages(response->operationContext));

    return wsmResponse;
}

WxfPutResponse* CimToWsmResponseMapper::_mapToWxfPutResponse(
    const WxfPutRequest* wsmRequest,
    const CIMModifyInstanceResponseMessage* response)
{
    WxfPutResponse* wsmResponse =
        new WxfPutResponse(
            wsmRequest,
            _getContentLanguages(response->operationContext));

    return wsmResponse;
}

WxfCreateResponse* CimToWsmResponseMapper::_mapToWxfCreateResponse(
    const WxfCreateRequest* wsmRequest,
    const CIMCreateInstanceResponseMessage* response)
{
    WsmEndpointReference epr;

    convertObjPathToEPR(
        wsmRequest->epr.resourceUri,
        response->instanceName,
        epr,
        wsmRequest->epr.getNamespace());

    WxfCreateResponse* wsmResponse =
        new WxfCreateResponse(
            epr,
            wsmRequest,
            _getContentLanguages(response->operationContext));

    return wsmResponse;
}

WxfSubCreateResponse* CimToWsmResponseMapper::_mapToWxfSubCreateResponse(
    const WxfSubCreateRequest* wsmRequest,
    const CIMCreateInstanceResponseMessage* response)
{
    WsmEndpointReference epr = wsmRequest->epr;

    WxfSubCreateResponse* wsmResponse =
        new WxfSubCreateResponse(
            epr,
            wsmRequest,
            _getContentLanguages(response->operationContext));

    return wsmResponse;
}

WxfDeleteResponse* CimToWsmResponseMapper::_mapToWxfDeleteResponse(
    const WxfDeleteRequest* wsmRequest,
    const CIMDeleteInstanceResponseMessage* response)
{
    WxfDeleteResponse* wsmResponse =
        new WxfDeleteResponse(
            wsmRequest,
            _getContentLanguages(response->operationContext));

    return wsmResponse;
}

WxfSubDeleteResponse* CimToWsmResponseMapper::_mapToWxfSubDeleteResponse(
    const WxfSubDeleteRequest* wsmRequest,
    const CIMDeleteInstanceResponseMessage* response)
{
     WxfSubDeleteResponse* wsmResponse =
        new WxfSubDeleteResponse(
            wsmRequest,
            _getContentLanguages(response->operationContext));

    return wsmResponse;
}

WsenEnumerateResponse*
    CimToWsmResponseMapper::_mapToWsenEnumerateResponseObject(
    const WsenEnumerateRequest* wsmRequest,
    CIMEnumerateInstancesResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseObject");
    Array<WsmInstance> instances;
    Array<WsmEndpointReference> EPRs;
    Array<CIMInstance>& namedInstances =
        response->getResponseData().getInstances();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "EnumerateInstances Returned %u instances",namedInstances.size() ));

    // if WQLFilter type
    if (wsmRequest->wsmFilter.filterDialect == WsmFilter::WQL)
    {
        // Filter out unwanted instances:

        for (Uint32 i = 0; i < namedInstances.size(); i++)
        {
            try
            {
                if (!wsmRequest->wsmFilter.WQLFilter.selectStatement->
                        evaluate(namedInstances[i]))
                {
                    continue;
                }
            }
            catch (...)
            {
                // This error is unreportable since all other instance
                // would have to  be aborted.
                continue;
            }

            CIMInstance instance = namedInstances[i].clone();

            try
            {
                wsmRequest->wsmFilter.WQLFilter.selectStatement->
                    applyProjection(instance, false);
            }
            catch (...)
            {
                // Ignore missing properties.
                continue;
            }

            WsmInstance wsmInstance;
            convertCimToWsmInstance(
                wsmRequest->epr.resourceUri,
                instance,
                wsmInstance,
                wsmRequest->epr.getNamespace());
            instances.append(wsmInstance);
        }

        WsenEnumerateResponse* wsmResponse =
            new WsenEnumerateResponse(
                instances,
                instances.size(),
                wsmRequest,
                _getContentLanguages(response->operationContext));
        PEG_METHOD_EXIT();
        return wsmResponse;
    }
    else
    {
        for (Uint32 i = 0; i < namedInstances.size(); i++)
        {
            WsmInstance wsmInstance;
            convertCimToWsmInstance(
                wsmRequest->epr.resourceUri,
                namedInstances[i],
                wsmInstance,
                wsmRequest->epr.getNamespace());
            instances.append(wsmInstance);
        }

        WsenEnumerateResponse* wsmResponse =
            new WsenEnumerateResponse(
                instances,
                instances.size(),
                wsmRequest,
                _getContentLanguages(response->operationContext));

        PEG_METHOD_EXIT();
        return wsmResponse;
    }
}

WsenEnumerateResponse*
    CimToWsmResponseMapper::_mapToWsenEnumerateResponseObjectAndEPR(
    const WsenEnumerateRequest* wsmRequest,
    CIMEnumerateInstancesResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseObjectAndEPR");

    Array<WsmInstance> instances;
    Array<WsmEndpointReference> EPRs;
    Array<CIMInstance>& namedInstances =
        response->getResponseData().getInstances();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "EnumerateInstances Returned %u instances ",namedInstances.size() ));

    for (Uint32 i = 0; i < namedInstances.size(); i++)
    {
        WsmInstance wsmInstance;
        convertCimToWsmInstance(
            wsmRequest->epr.resourceUri,
            namedInstances[i],
            wsmInstance,
            wsmRequest->epr.getNamespace());
        instances.append(wsmInstance);

        WsmEndpointReference epr;
        convertObjPathToEPR(
            wsmRequest->epr.resourceUri,
            namedInstances[i].getPath(),
            epr,
            wsmRequest->epr.getNamespace());
        EPRs.append(epr);
    }

    WsenEnumerateResponse* wsmResponse =
        new WsenEnumerateResponse(
            instances,
            EPRs,
            instances.size(),
            wsmRequest,
            _getContentLanguages(response->operationContext));
    PEG_METHOD_EXIT();
    return wsmResponse;
}

/****************************************************************************
**
**       _mapToWsenEnumerateResponse for enumerateInstances and
**           EnumerateInstanceNames responses
**
******************************************************************************/
WsenEnumerateResponse*
CimToWsmResponseMapper::_mapToWsenEnumerateResponseEPR(
    const WsenEnumerateRequest* wsmRequest,
    CIMEnumerateInstanceNamesResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseEPR");

    Array<WsmEndpointReference> EPRs;
    Array<CIMObjectPath>& instanceNames =
        response->getResponseData().getInstanceNames();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "EnumerateInstanceNames Returned %u instanceNames ",
               instanceNames.size() ));

    for (Uint32 i = 0; i < instanceNames.size(); i++)
    {
        WsmEndpointReference epr;
        convertObjPathToEPR(
            wsmRequest->epr.resourceUri,
            instanceNames[i],
            epr,
            wsmRequest->epr.getNamespace());
        EPRs.append(epr);
    }

    WsenEnumerateResponse* wsmResponse =
        new WsenEnumerateResponse(
            EPRs,
            EPRs.size(),
            wsmRequest,
            _getContentLanguages(response->operationContext));
    PEG_METHOD_EXIT();
    return wsmResponse;
}

/****************************************************************************
**
**       _mapToWsenEnumerateResponse for CIM Reference and ReferenceNames
**           responses
**
******************************************************************************/
WsenEnumerateResponse*
    CimToWsmResponseMapper::_mapToWsenEnumerateResponseObject(
    const WsenEnumerateRequest* wsmRequest,
    CIMReferencesResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseObject");

    Array<WsmInstance> instances;
    Array<WsmEndpointReference> EPRs;
    Array<CIMObject>& objects =
        response->getResponseData().getObjects();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "References Returned %u objects ",objects.size() ));

    for (Uint32 i = 0; i < objects.size(); i++)
    {
        WsmInstance wsmInstance;
        convertCimToWsmInstance(
            wsmRequest->wsmFilter.AssocFilter.object.resourceUri,
            (CIMInstance)objects[i],
            wsmInstance,
            wsmRequest->wsmFilter.AssocFilter.object.getNamespace());
        instances.append(wsmInstance);
    }

    WsenEnumerateResponse* wsmResponse =
        new WsenEnumerateResponse(
            instances,
            instances.size(),
            wsmRequest,
            _getContentLanguages(response->operationContext));

    PEG_METHOD_EXIT();
    return wsmResponse;
}

WsenEnumerateResponse*
    CimToWsmResponseMapper::_mapToWsenEnumerateResponseObjectAndEPR(
    const WsenEnumerateRequest* wsmRequest,
    CIMReferencesResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseObjectAndEPR");

    Array<WsmInstance> instances;
    Array<WsmEndpointReference> EPRs;
    Array<CIMObject>& objects =
        response->getResponseData().getObjects();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "Returned %u objects from References ",objects.size() ));

    for (Uint32 i = 0; i < objects.size(); i++)
    {
        WsmInstance wsmInstance;

        convertCimToWsmInstance(
            wsmRequest->wsmFilter.AssocFilter.object.resourceUri,
            (CIMInstance)objects[i],
            wsmInstance,
            wsmRequest->wsmFilter.AssocFilter.object.getNamespace());
        instances.append(wsmInstance);

        WsmEndpointReference epr;
        convertObjPathToEPR(
            wsmRequest->wsmFilter.AssocFilter.object.resourceUri,
            objects[i].getPath(),
            epr,
            wsmRequest->wsmFilter.AssocFilter.object.getNamespace());
        EPRs.append(epr);
    }

    WsenEnumerateResponse* wsmResponse =
        new WsenEnumerateResponse(
            instances,
            EPRs,
            instances.size(),
            wsmRequest,
            _getContentLanguages(response->operationContext));
    PEG_METHOD_EXIT();
    return wsmResponse;
}

WsenEnumerateResponse*
CimToWsmResponseMapper::_mapToWsenEnumerateResponseEPR(
    const WsenEnumerateRequest* wsmRequest,
    CIMReferenceNamesResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseEPR");

    Array<WsmEndpointReference> EPRs;
    Array<CIMObjectPath>& instanceNames =
        response->getResponseData().getInstanceNames();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "Returned %u names from ReferenceNames ",instanceNames.size() ));

    for (Uint32 i = 0; i < instanceNames.size(); i++)
    {
        WsmEndpointReference epr;

        convertObjPathToEPR(
            wsmRequest->wsmFilter.AssocFilter.object.resourceUri,
            instanceNames[i],
            epr,
            wsmRequest->wsmFilter.AssocFilter.object.getNamespace());
        EPRs.append(epr);
    }

    WsenEnumerateResponse* wsmResponse =
        new WsenEnumerateResponse(
            EPRs,
            EPRs.size(),
            wsmRequest,
            _getContentLanguages(response->operationContext));
    PEG_METHOD_EXIT();
    return wsmResponse;
}

/****************************************************************************
**
**       _mapToWsenEnumerateResponse for Associator and AssociatorNames
**           responses
**
******************************************************************************/
WsenEnumerateResponse*
    CimToWsmResponseMapper::_mapToWsenEnumerateResponseObject(
    const WsenEnumerateRequest* wsmRequest,
    CIMAssociatorsResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseObject");
    Array<WsmInstance> instances;
    Array<WsmEndpointReference> EPRs;
    Array<CIMObject>& objects =
        response->getResponseData().getObjects();

    for (Uint32 i = 0; i < objects.size(); i++)
    {
        WsmInstance wsmInstance;
        convertCimToWsmInstance(
            wsmRequest->epr.resourceUri,
            (CIMInstance)objects[i],
            wsmInstance,
            wsmRequest->epr.getNamespace());
        instances.append(wsmInstance);
    }

    WsenEnumerateResponse* wsmResponse =
        new WsenEnumerateResponse(
            instances,
            instances.size(),
            wsmRequest,
            _getContentLanguages(response->operationContext));
    PEG_METHOD_EXIT();
    return wsmResponse;
}

WsenEnumerateResponse*
    CimToWsmResponseMapper::_mapToWsenEnumerateResponseObjectAndEPR(
    const WsenEnumerateRequest* wsmRequest,
    CIMAssociatorsResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseObjectAndEPR");
    Array<WsmInstance> instances;
    Array<WsmEndpointReference> EPRs;
    Array<CIMObject>& objects =
        response->getResponseData().getObjects();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "Returned %u objects from Associators",objects.size() ));

    for (Uint32 i = 0; i < objects.size(); i++)
    {
        WsmInstance wsmInstance;

        convertCimToWsmInstance(
            wsmRequest->wsmFilter.AssocFilter.object.resourceUri,
            (CIMInstance)objects[i],
            wsmInstance,
            wsmRequest->wsmFilter.AssocFilter.object.getNamespace());
        instances.append(wsmInstance);

        WsmEndpointReference epr;
        convertObjPathToEPR(
            wsmRequest->wsmFilter.AssocFilter.object.resourceUri,
            objects[i].getPath(),
            epr,
            wsmRequest->wsmFilter.AssocFilter.object.getNamespace());
        EPRs.append(epr);
    }

    WsenEnumerateResponse* wsmResponse =
        new WsenEnumerateResponse(
            instances,
            EPRs,
            instances.size(),
            wsmRequest,
            _getContentLanguages(response->operationContext));

    PEG_METHOD_EXIT();
    return wsmResponse;
}

WsenEnumerateResponse*
CimToWsmResponseMapper::_mapToWsenEnumerateResponseEPR(
    const WsenEnumerateRequest* wsmRequest,
    CIMAssociatorNamesResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsenEnumerateResponseEPR");

    Array<WsmEndpointReference> EPRs;
    Array<CIMObjectPath>& instanceNames =
        response->getResponseData().getInstanceNames();

    PEG_TRACE((TRC_WSMSERVER, Tracer::LEVEL4,
        "Returned %u instanceNames",instanceNames.size() ));

    for (Uint32 i = 0; i < instanceNames.size(); i++)
    {
        WsmEndpointReference epr;

        convertObjPathToEPR(
            wsmRequest->wsmFilter.AssocFilter.object.resourceUri,
            instanceNames[i],
            epr,
            wsmRequest->wsmFilter.AssocFilter.object.getNamespace());
        EPRs.append(epr);
    }

    WsenEnumerateResponse* wsmResponse =
        new WsenEnumerateResponse(
            EPRs,
            EPRs.size(),
            wsmRequest,
            _getContentLanguages(response->operationContext));
    PEG_METHOD_EXIT();
    return wsmResponse;
}

WsInvokeResponse* CimToWsmResponseMapper::_mapToWsInvokeResponse(
    const WsInvokeRequest* wsmRequest,
    const CIMInvokeMethodResponseMessage* response)
{
    PEG_METHOD_ENTER(TRC_WSMSERVER,
        "CimToWsmResponseMapper::_mapToWsInvokeResponse");

    WsmInstance wsmInstance;
    String nameSpace = wsmRequest->epr.getNamespace();

    convertCimToWsmParameters(
        wsmRequest->epr.resourceUri,
        nameSpace,
        response->outParameters,
        response->retValue,
        wsmInstance);

    WsInvokeResponse* wsmResponse =
        new WsInvokeResponse(
            nameSpace,
            wsmRequest->className,
            response->methodName.getString(),
            wsmInstance,
            wsmRequest,
            _getContentLanguages(response->operationContext));
    PEG_METHOD_EXIT();
    return wsmResponse;
}

void CimToWsmResponseMapper::convertCimToWsmInstance(
    const String& resourceUri,
    const CIMConstInstance& cimInstance,
    WsmInstance& wsmInstance,
    const String& nameSpace)
{
    wsmInstance.setClassName(cimInstance.getClassName().getString());

    for (Uint32 i = 0, n = cimInstance.getPropertyCount(); i < n; i++)
    {
        CIMConstProperty cimProperty = cimInstance.getProperty(i);
        const String& propertyName = cimProperty.getName().getString();
        const CIMValue& cimValue = cimProperty.getValue();

        WsmValue wsmValue;
        convertCimToWsmValue(resourceUri, cimValue, wsmValue, nameSpace);

        WsmProperty wsmProperty(propertyName, wsmValue);
        wsmInstance.addProperty(wsmProperty);
    }
}

template<class T>
static void _convertCimToWsmArrayValue(
    const CIMValue& cimValue,
    WsmValue& wsmValue)
{
    Array<T> arr;
    Array<String> strs;

    cimValue.get(arr);
    for (Uint32 i = 0, n = arr.size(); i < n; i++)
    {
        CIMValue val(arr[i]);
        String str(val.toString());
        if (val.getType() == CIMTYPE_BOOLEAN)
        {
            str.toLower();
        }
        strs.append(str);
    }
    wsmValue.set(strs);
}

void CimToWsmResponseMapper::convertCimToWsmValue(
     const String& resourceUri,
     const CIMValue& cimValue,
     WsmValue& wsmValue,
     const String& nameSpace)
{
    if (cimValue.isNull())
    {
        wsmValue.setNull();
        return;
    }

    if (cimValue.isArray())
    {
        switch (cimValue.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                _convertCimToWsmArrayValue<Boolean>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_UINT8:
            {
                _convertCimToWsmArrayValue<Uint8>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_SINT8:
            {
                _convertCimToWsmArrayValue<Sint8>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_UINT16:
            {
                _convertCimToWsmArrayValue<Uint16>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_SINT16:
            {
                _convertCimToWsmArrayValue<Sint16>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_UINT32:
            {
                _convertCimToWsmArrayValue<Uint32>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_SINT32:
            {
                _convertCimToWsmArrayValue<Sint32>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_UINT64:
            {
                _convertCimToWsmArrayValue<Uint64>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_SINT64:
            {
                _convertCimToWsmArrayValue<Sint64>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_REAL32:
            {
                _convertCimToWsmArrayValue<Real32>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_REAL64:
            {
                _convertCimToWsmArrayValue<Real64>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_CHAR16:
            {
                _convertCimToWsmArrayValue<Char16>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_STRING:
            {
                _convertCimToWsmArrayValue<String>(cimValue, wsmValue);
                break;
            }

            case CIMTYPE_DATETIME:
            {
                Array<CIMDateTime> dates;
                Array<String> strs;
                cimValue.get(dates);
                for (Uint32 i = 0, n = dates.size(); i < n; i++)
                {
                    String wsmDT;
                    convertCimToWsmDatetime(dates[i], wsmDT);
                    strs.append(wsmDT);
                }
                wsmValue.set(strs);
                break;
            }

            case CIMTYPE_REFERENCE:
            {
                Array<WsmEndpointReference> eprs;
                Array<CIMObjectPath> objPaths;
                cimValue.get(objPaths);

                for (Uint32 i = 0, n = objPaths.size(); i < n; i++)
                {
                    WsmEndpointReference epr;
                    convertObjPathToEPR(resourceUri, objPaths[i], epr,
                        nameSpace);
                    eprs.append(epr);
                }
                wsmValue.set(eprs);
                break;
            }

            case CIMTYPE_OBJECT:
            {
                Array<WsmInstance> wsmInstances;
                Array<CIMObject> cimObjects;
                cimValue.get(cimObjects);
                for (Uint32 i = 0, n = cimObjects.size(); i < n; i++)
                {
                    if (cimObjects[i].isInstance())
                    {
                        WsmInstance wsmInstance;
                        convertCimToWsmInstance(
                            resourceUri,
                            CIMInstance(cimObjects[i]),
                            wsmInstance,
                            nameSpace);
                        wsmInstances.append(wsmInstance);
                    }
                    else
                    {
                        throw WsmFault(WsmFault::wsman_InternalError,
                            MessageLoaderParms(
                                "WsmServer.CimToWsmResponseMapper."
                                    "EMBEDDED_CLASS_NOT_SUPPORTED",
                                "Embedded class objects in WS-Management "
                                    "responses are not supported."));
                    }
                }
                wsmValue.set(wsmInstances);
                break;
            }

            case CIMTYPE_INSTANCE:
            {
                Array<WsmInstance> wsmInstances;
                Array<CIMInstance> cimInstances;
                cimValue.get(cimInstances);
                for (Uint32 i = 0, n = cimInstances.size(); i < n; i++)
                {
                    WsmInstance wsmInstance;
                    convertCimToWsmInstance(
                        resourceUri,
                        cimInstances[i],
                        wsmInstance,
                        nameSpace);
                    wsmInstances.append(wsmInstance);
                }
                wsmValue.set(wsmInstances);
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
        switch (cimValue.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                String str(cimValue.toString());
                str.toLower();
                wsmValue.set(str);
                break;
            }
            case CIMTYPE_UINT8:
            case CIMTYPE_SINT8:
            case CIMTYPE_UINT16:
            case CIMTYPE_SINT16:
            case CIMTYPE_UINT32:
            case CIMTYPE_SINT32:
            case CIMTYPE_UINT64:
            case CIMTYPE_SINT64:
            case CIMTYPE_CHAR16:
            case CIMTYPE_STRING:
            {
                wsmValue.set(cimValue.toString());
                break;
            }
            case CIMTYPE_REAL32:
            case CIMTYPE_REAL64:
            {
                String str(cimValue.toString());
                if (String::compareNoCase(str, PEGASUS_NAN) == 0)
                    wsmValue.set("NaN");
                else if (String::compareNoCase(str, PEGASUS_INF) == 0)
                    wsmValue.set("INF");
                else if (String::compareNoCase(str, PEGASUS_NEG_INF) == 0)
                    wsmValue.set("-INF");
                else
                    wsmValue.set(str);
                break;
            }
            case CIMTYPE_DATETIME:
            {
                String wsmDT;
                CIMDateTime cimDT;
                cimValue.get(cimDT);
                convertCimToWsmDatetime(cimDT, wsmDT);
                wsmValue.set(wsmDT);
                break;
            }
            case CIMTYPE_REFERENCE:
            {
                WsmEndpointReference epr;
                CIMObjectPath objPath;
                cimValue.get(objPath);
                convertObjPathToEPR(resourceUri, objPath, epr, nameSpace);
                wsmValue.set(epr);
                break;
            }
            case CIMTYPE_OBJECT:
            {
                CIMObject cimObject;
                cimValue.get(cimObject);
                if (cimObject.isInstance())
                {
                    WsmInstance wsmInstance;
                    convertCimToWsmInstance(
                        resourceUri,
                        CIMInstance(cimObject), wsmInstance, nameSpace);
                    wsmValue.set(wsmInstance);
                }
                else
                {
                    throw WsmFault(WsmFault::wsman_InternalError,
                        MessageLoaderParms(
                            "WsmServer.CimToWsmResponseMapper."
                                "EMBEDDED_CLASS_NOT_SUPPORTED",
                            "Embedded class objects in WS-Management "
                                "responses are not supported"));
                }
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                WsmInstance wsmInstance;
                CIMInstance cimInstance;
                cimValue.get(cimInstance);
                convertCimToWsmInstance(
                    resourceUri, cimInstance, wsmInstance, nameSpace);
                wsmValue.set(wsmInstance);
                break;
            }
            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            }
        }
    }
}

void CimToWsmResponseMapper::convertObjPathToEPR(
    const String& resourceUri,
    const CIMObjectPath& objPath,
    WsmEndpointReference& epr,
    const String& nameSpace)
{
    // Since the connection protocol is unknown at this point, we assume
    // it's http and form an EPR address by concatenating "http://" with
    // the host name in the object path,  with "/wsman".
    if (objPath.getHost().size() > 0)
        epr.address = "http://" + objPath.getHost() + "/wsman";
    else
        epr.address = WSM_ADDRESS_ANONYMOUS;

    epr.resourceUri = WsmUtils::getRootResourceUri(resourceUri) + "/" +
        objPath.getClassName().getString();

    CIMNamespaceName cimNS = objPath.getNameSpace();
    if (!cimNS.isNull())
    {
        WsmSelector selector(String("__cimnamespace"), cimNS.getString());
        epr.selectorSet->selectors.append(selector);
    }
    else
    {
        WsmSelector selector(String("__cimnamespace"), nameSpace);
        epr.selectorSet->selectors.append(selector);
    }

    const Array<CIMKeyBinding>& keyBindings = objPath.getKeyBindings();
    for (Uint32 i = 0, n = keyBindings.size(); i < n; i++)
    {
        CIMKeyBinding binding = keyBindings[i];
        if (binding.getType() == CIMKeyBinding::REFERENCE)
        {
            CIMObjectPath cimRef = binding.getValue();
            WsmEndpointReference wsmRef;
            convertObjPathToEPR(resourceUri, cimRef, wsmRef, nameSpace);
            WsmSelector selector(binding.getName().getString(), wsmRef);
            epr.selectorSet->selectors.append(selector);
        }
        else
        {
            WsmSelector selector(binding.getName().getString(),
                binding.getValue());
            epr.selectorSet->selectors.append(selector);
        }
    }
}

void CimToWsmResponseMapper::convertCimToWsmDatetime(
    const CIMDateTime& cimDT, String& wsmDT)
{
    char buffer[50];
    Uint32 size;

    String cimStrDT = cimDT.toString();
    CString cimCStrDT = cimStrDT.getCString();
    const char* cimStr = (const char*) cimCStrDT;
    Uint32 firstAsteriskPos = cimStrDT.find('*');

    // DSP0230.
    // 1. If CIM datetime string contains ":", use Interval cim:cimDateTime
    // element.
    // 2. If CIM datetime string contains "+" or "-" and does not contain any
    // asterisks, use Datetime cim:cimDateTime element.
    // 3. If CIM datetime string contains "+" or "-" and no asterisks in
    // the hhmmss.mmmmmm portion, and only asterisks in the yyyymmdd portion,
    // ATTN: this makes no sense. yyyymmdd cannot be wildcarded unless
    // previous sections are wildcarded.
    // use Time cim:cimDateTime element.
    // 4. If CIM datetime string contains "+" or "-" and no asterisks in the
    // yyyymmdd portion, and only asterisks in the hhmmss.mmmmmm portion,
    // use Date cim:cimDateTime element.
    // 5. In all other cases use CIM_DateTime element.

    if (cimStr[21] == ':')
    {
        // Interval
        Uint32 days = 0, hrs = 0, mins = 0, secs = 0, msecs = 0;
        int conversions = sscanf(cimStr, "%8u%2u%2u%2u.%u:000",
            &days, &hrs, &mins, &secs, &msecs);
        if (conversions == 0 && cimStr[0] == '*')
            days = 1;

        wsmDT = "P";
        if (conversions >= 1 && days)
        {
            wsmDT.append(Uint32ToString(buffer, days, size));
            wsmDT.append(Char16('D'));
        }
        if (conversions >= 2 )
        {
            wsmDT.append(Char16('T'));
            if (hrs)
            {
                wsmDT.append(Uint32ToString(buffer, hrs, size));
                wsmDT.append(Char16('H'));
            }
        }
        if (conversions >= 3 && mins)
        {
            wsmDT.append(Uint32ToString(buffer, mins, size));
            wsmDT.append(Char16('M'));
        }
        if (conversions >= 4 && secs)
        {
            wsmDT.append(Uint32ToString(buffer, secs, size));
            if (conversions >= 5 && msecs)
            {
                wsmDT.append(Char16('.'));
                wsmDT.append(Uint32ToString(buffer, msecs, size));
            }
            wsmDT.append(Char16('S'));
        }

        // According to spec, at least one number must be present, so if
        // we end up with "PT", then convert to "PT0S".
        if (wsmDT == "PT")
            wsmDT.append("0S");
    }
    else if ((cimStr[21] == '+' || cimStr[21] == '-') &&
             firstAsteriskPos == PEG_NOT_FOUND)
    {
        // Datetime
        Uint32 year = 0, month = 0, day = 0, utcoff = 0,
            hrs = 0, mins = 0, secs = 0, msecs = 0;
        char sign;
        
        PEGASUS_FCT_EXECUTE_AND_ASSERT(
            9,
            sscanf(
                cimStr,
                "%4u%2u%2u%2u%2u%2u.%6u%c%3u",
                &year, &month, &day, &hrs, &mins, &secs, &msecs, &sign, &utcoff)
            );

        if (utcoff == 0)
        {
            if (msecs)
            {
                sprintf(buffer, "%.4u-%.2u-%.2uT%.2u:%.2u:%.2u.%.6uZ",
                    year, month, day, hrs, mins, secs, msecs);
            }
            else
            {
                sprintf(buffer, "%.4u-%.2u-%.2uT%.2u:%.2u:%.2uZ",
                    year, month, day, hrs, mins, secs);
            }
        }
        else
        {
            Uint32 utch = utcoff / 60;
            Uint32 utcm = utcoff % 60;
            if (msecs)
            {
                sprintf(buffer, "%.4u-%.2u-%.2uT%.2u:%.2u:%.2u.%.6u%c%.2u:%.2u",
                    year, month, day, hrs, mins, secs, msecs,
                    sign, utch, utcm);
            }
            else
            {
                sprintf(buffer, "%.4u-%.2u-%.2uT%.2u:%.2u:%.2u%c%.2u:%.2u",
                    year, month, day, hrs, mins, secs, sign, utch, utcm);
            }
        }
        wsmDT = buffer;
    }
    else if ((cimStr[21] == '+' || cimStr[21] == '-') &&
             firstAsteriskPos == 8)
    {
        // Date
        Uint32 year = 0, month = 0, day = 0, utcoff = 0;
        char sign;
        
        PEGASUS_FCT_EXECUTE_AND_ASSERT(
            5,
            sscanf(
                cimStr,
                "%4u%2u%2u******.******%c%3u",
                &year, &month, &day, &sign, &utcoff));

        if (utcoff == 0)
        {
            sprintf(buffer, "%.4u-%.2u-%.2uZ", year, month, day);
        }
        else
        {
            Uint32 utch = utcoff / 60;
            Uint32 utcm = utcoff % 60;
            sprintf(buffer, "%.4u-%.2u-%.2u%c%.2u:%.2u",
                year, month, day, sign, utch, utcm);
        }
        wsmDT = buffer;
    }
    else
    {
        // CIM_DateTime
        wsmDT = cimStr;
    }
}

void CimToWsmResponseMapper::convertCimToWsmParameters(
    const String& resourceUri,
    const String& nameSpace,
    const Array<CIMParamValue>& parameters,
    const CIMValue& returnValue,
    WsmInstance& wsmInstance)
{
    // Convert output properties.

    for (Uint32 i = 0, n = parameters.size(); i < n; i++)
    {
        const CIMParamValue& cpv = parameters[i];
        const String& name = cpv.getParameterName();
        const CIMValue& value = cpv.getValue();

        WsmValue wvalue;
        convertCimToWsmValue(resourceUri, value, wvalue, nameSpace);
        wsmInstance.addProperty(WsmProperty(name, wvalue));
    }

    // Convert return value.
    {
        WsmValue wvalue;
        convertCimToWsmValue(resourceUri, returnValue, wvalue, nameSpace);
        wsmInstance.addProperty(WsmProperty("ReturnValue", wvalue));
    }
}

PEGASUS_NAMESPACE_END
