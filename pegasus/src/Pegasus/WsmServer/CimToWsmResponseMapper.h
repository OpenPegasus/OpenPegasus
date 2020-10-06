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

#ifndef Pegasus_CimToWsmResponseMapper_h
#define Pegasus_CimToWsmResponseMapper_h

#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/WsmServer/WsmResponse.h>
#include <Pegasus/WsmServer/WsmRequest.h>

PEGASUS_NAMESPACE_BEGIN

/**
    Converts CIMResponseMessage messages to WsmResponses.
*/
class PEGASUS_WSMSERVER_LINKAGE CimToWsmResponseMapper
{
public:

    CimToWsmResponseMapper();
    ~CimToWsmResponseMapper();

    WsmResponse* mapToWsmResponse(
        const WsmRequest* wsmRequest,
        const CIMResponseMessage* message);
    WsmFault mapCimExceptionToWsmFault(const CIMException& cimException);

    void convertCimToWsmInstance(
        const String& resourceUri,
        const CIMConstInstance& cimInstance,
        WsmInstance& wsmInstance,
        const String& nameSpace);

    void convertCimToWsmParameters(
        const String& resourceUri,
        const String& nameSpace,
        const Array<CIMParamValue>& parameters,
        const CIMValue& returnValue,
        WsmInstance& instance);

    void convertCimToWsmValue(
        const String& resourceUri,
        const CIMValue& cimValue,
        WsmValue& wsmValue,
        const String& nameSpace);

    void convertObjPathToEPR(
        const String& resourceUri,
        const CIMObjectPath& objPath,
        WsmEndpointReference& epr,
        const String& nameSpace);
    void convertCimToWsmDatetime(const CIMDateTime& cimDT, String& wsmDT);

private:

    WxfGetResponse* _mapToWxfGetResponse(
        const WxfGetRequest* wsmRequest,
        CIMGetInstanceResponseMessage* response);
    WxfPutResponse* _mapToWxfPutResponse(
        const WxfPutRequest* wsmRequest,
        const CIMModifyInstanceResponseMessage* response);
    WxfCreateResponse* _mapToWxfCreateResponse(
        const WxfCreateRequest* wsmRequest,
        const CIMCreateInstanceResponseMessage* response);
    WxfSubCreateResponse* _mapToWxfSubCreateResponse(
        const WxfSubCreateRequest* wsmRequest,
        const CIMCreateInstanceResponseMessage* response);
    WxfDeleteResponse* _mapToWxfDeleteResponse(
        const WxfDeleteRequest* wsmRequest,
        const CIMDeleteInstanceResponseMessage* response);
    WxfSubDeleteResponse* _mapToWxfSubDeleteResponse(
        const WxfSubDeleteRequest* wsmRequest,
        const CIMDeleteInstanceResponseMessage* response);

    WsenEnumerateResponse* _mapToWsenEnumerateResponseObject(
        const WsenEnumerateRequest* wsmRequest,
        CIMEnumerateInstancesResponseMessage* response);
    WsenEnumerateResponse* _mapToWsenEnumerateResponseObjectAndEPR(
        const WsenEnumerateRequest* wsmRequest,
        CIMEnumerateInstancesResponseMessage* response);
    WsenEnumerateResponse* _mapToWsenEnumerateResponseEPR(
        const WsenEnumerateRequest* wsmRequest,
        CIMEnumerateInstanceNamesResponseMessage* response);

    WsenEnumerateResponse* _mapToWsenEnumerateResponseObject(
        const WsenEnumerateRequest* wsmRequest,
        CIMReferencesResponseMessage* response);
    WsenEnumerateResponse* _mapToWsenEnumerateResponseObjectAndEPR(
        const WsenEnumerateRequest* wsmRequest,
        CIMReferencesResponseMessage* response);
    WsenEnumerateResponse* _mapToWsenEnumerateResponseEPR(
        const WsenEnumerateRequest* wsmRequest,
        CIMReferenceNamesResponseMessage* response);

    WsenEnumerateResponse* _mapToWsenEnumerateResponseObject(
        const WsenEnumerateRequest* wsmRequest,
        CIMAssociatorsResponseMessage* response);
    WsenEnumerateResponse* _mapToWsenEnumerateResponseObjectAndEPR(
        const WsenEnumerateRequest* wsmRequest,
        CIMAssociatorsResponseMessage* response);
    WsenEnumerateResponse* _mapToWsenEnumerateResponseEPR(
        const WsenEnumerateRequest* wsmRequest,
        CIMAssociatorNamesResponseMessage* response);

    WsmFaultResponse* _mapToWsmFaultResponse(
        const WsmRequest* wsmRequest,
        const CIMResponseMessage* message);

    WsInvokeResponse* _mapToWsInvokeResponse(
        const WsInvokeRequest* wsmRequest,
        const CIMInvokeMethodResponseMessage* response);

    ContentLanguageList _getContentLanguages(const OperationContext& context)
    {
        if (context.contains(ContentLanguageListContainer::NAME))
        {
            return ((ContentLanguageListContainer)
                context.get(ContentLanguageListContainer::NAME)).getLanguages();
        }

        return ContentLanguageList();
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CimToWsmResponseMapper_h */
