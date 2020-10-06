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

#ifndef Pegasus_WsmToCimRequestMapper_h
#define Pegasus_WsmToCimRequestMapper_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/WsmServer/WsmRequest.h>

PEGASUS_NAMESPACE_BEGIN

/**
    Converts WsmRequest messages to CIMOperationRequestMessages.
*/
class PEGASUS_WSMSERVER_LINKAGE WsmToCimRequestMapper
{
public:

    WsmToCimRequestMapper(CIMRepository* repository);
    ~WsmToCimRequestMapper();

    CIMOperationRequestMessage* mapToCimRequest(WsmRequest* request);

    CIMGetInstanceRequestMessage* mapToCimGetInstanceRequest(
        WxfGetRequest* request);
    CIMModifyInstanceRequestMessage* mapToCimModifyInstanceRequest(
        WxfPutRequest* request);
    CIMCreateInstanceRequestMessage* mapToCimCreateInstanceRequest(
        WsmRequest * request,Boolean isSubCreate=false);
    CIMDeleteInstanceRequestMessage* mapToCimDeleteInstanceRequest(
        WsmRequest* request,Boolean isSubDeleteReq=false);
    CIMEnumerateInstancesRequestMessage* mapToCimEnumerateInstancesRequest(
        WsenEnumerateRequest* request);
    CIMEnumerateInstanceNamesRequestMessage*
        mapToCimEnumerateInstanceNamesRequest(WsenEnumerateRequest* request);

    // Support Associated filter
    CIMReferencesRequestMessage* mapToCimReferencesRequest(
        WsenEnumerateRequest* request);
    CIMReferenceNamesRequestMessage* mapToCimReferenceNamesRequest(
        WsenEnumerateRequest* request);
    CIMAssociatorsRequestMessage* mapToCimAssociatorsRequest(
        WsenEnumerateRequest* request);
    CIMAssociatorNamesRequestMessage* mapToCimAssociatorNamesRequest(
        WsenEnumerateRequest* request);

    CIMInvokeMethodRequestMessage* mapToCimInvokeMethodRequest(
        WsInvokeRequest* request);

    static CIMName convertResourceUriToClassName(const String& resourceUri);
    String convertEPRAddressToHostname(const String& addr);

    static Boolean stringToReal64(const char* stringValue, Real64& x);

    void convertWsmToCimValue(
        WsmValue& wsmValue,
        const CIMNamespaceName& nameSpace,
        CIMValue& cimValue);

    static void convertStringToCimValue(
        const String& str,
        CIMType cimType,
        CIMValue& cimValue);

    void convertStringArrayToCimValue(
        const Array<String>& strs,
        CIMType cimType,
        CIMValue& cimValue);

    void convertWsmToCimInstance(
        WsmInstance& wsmInstance,
        const CIMNamespaceName& nameSpace,
        CIMInstance& cimInstance);

    void convertEPRToObjectPath(
        const WsmEndpointReference& epr,
        CIMObjectPath& objectPath);

    static void convertWsmToCimDatetime(
        const String& wsmDT, CIMDateTime& cimDT);

    void convertWsmToCimParameters(
        const CIMNamespaceName& nameSpace,
        const String& className,
        const String& methodName,
        WsmInstance& instance,
        Array<CIMParamValue>& parameters);

private:

    CIMRepository* _repository;

    void _disallowAllClassesResourceUri(const String& resourceUri);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmToCimRequestMapper_h */
