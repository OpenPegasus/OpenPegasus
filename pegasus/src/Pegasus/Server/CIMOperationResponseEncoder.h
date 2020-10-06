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

#ifndef Pegasus_CIMOperationResponseEncoder_h
#define Pegasus_CIMOperationResponseEncoder_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/CIMMessage.h>

PEGASUS_NAMESPACE_BEGIN

/** This class encodes CIM operation requests and passes them up-stream.
 */
class CIMOperationResponseEncoder : public MessageQueue
{
private:
     static const String OUT_OF_MEMORY_MESSAGE;

public:

    typedef MessageQueue Base;

    CIMOperationResponseEncoder();

    ~CIMOperationResponseEncoder();

    void sendResponse(
        CIMResponseMessage* response,
        const String& name,
        Boolean isImplicit,
        Buffer* bodygiven = 0);

//EXP_PULL_BEGIN
    void sendResponsePull(
        CIMResponseMessage* response,
        const String& name,
        Boolean isImplicit,
        Buffer* bodyParams,
        Buffer* bodygiven = 0);
//EXP_PULL_END

    virtual void enqueue(Message*);

    virtual void handleEnqueue(Message*);

    virtual void handleEnqueue();

    void encodeCreateClassResponse(
        CIMCreateClassResponseMessage* response);

    void encodeGetClassResponse(
        CIMGetClassResponseMessage* response);

    void encodeModifyClassResponse(
        CIMModifyClassResponseMessage* response);

    void encodeEnumerateClassNamesResponse(
        CIMEnumerateClassNamesResponseMessage* response);

    void encodeEnumerateClassesResponse(
        CIMEnumerateClassesResponseMessage* response);

    void encodeDeleteClassResponse(
        CIMDeleteClassResponseMessage* response);

    void encodeCreateInstanceResponse(
        CIMCreateInstanceResponseMessage* response);

    void encodeGetInstanceResponse(
        CIMGetInstanceResponseMessage* response);

    void encodeModifyInstanceResponse(
        CIMModifyInstanceResponseMessage* response);

    void encodeEnumerateInstanceNamesResponse(
        CIMEnumerateInstanceNamesResponseMessage* response);

    void encodeEnumerateInstancesResponse(
        CIMEnumerateInstancesResponseMessage* response);

//EXP_PULL_BEGIN
    void encodeOpenEnumerateInstancesResponse(
        CIMOpenEnumerateInstancesResponseMessage* response);

    void encodeOpenEnumerateInstancePathsResponse(
        CIMOpenEnumerateInstancePathsResponseMessage* response);

    void encodeOpenReferenceInstancesResponse(
        CIMOpenReferenceInstancesResponseMessage* response);

    void encodeOpenReferenceInstancePathsResponse(
        CIMOpenReferenceInstancePathsResponseMessage* response);

    void encodeOpenAssociatorInstancesResponse(
        CIMOpenAssociatorInstancesResponseMessage* response);

    void encodeOpenAssociatorInstancePathsResponse(
        CIMOpenAssociatorInstancePathsResponseMessage* response);

    void encodePullInstancesWithPathResponse(
        CIMPullInstancesWithPathResponseMessage* response);

    void encodePullInstancePathsResponse(
        CIMPullInstancePathsResponseMessage* response);

    void encodePullInstancesResponse(
        CIMPullInstancesResponseMessage* response);

    void encodeCloseEnumerationResponse(
        CIMCloseEnumerationResponseMessage* response);

    void encodeEnumerationCountResponse(
        CIMEnumerationCountResponseMessage* response);

    void encodeOpenQueryInstancesResponse(
        CIMOpenQueryInstancesResponseMessage* response);
//EXP_PULL_END

    void encodeDeleteInstanceResponse(
        CIMDeleteInstanceResponseMessage* response);

    void encodeGetPropertyResponse(
        CIMGetPropertyResponseMessage* response);

    void encodeSetPropertyResponse(
        CIMSetPropertyResponseMessage* response);

    void encodeSetQualifierResponse(
        CIMSetQualifierResponseMessage* response);

    void encodeGetQualifierResponse(
        CIMGetQualifierResponseMessage* response);

    void encodeEnumerateQualifiersResponse(
        CIMEnumerateQualifiersResponseMessage* response);

    void encodeDeleteQualifierResponse(
        CIMDeleteQualifierResponseMessage* response);

    void encodeReferenceNamesResponse(
        CIMReferenceNamesResponseMessage* response);

    void encodeReferencesResponse(
        CIMReferencesResponseMessage* response);

    void encodeAssociatorNamesResponse(
        CIMAssociatorNamesResponseMessage* response);

    void encodeAssociatorsResponse(
        CIMAssociatorsResponseMessage* response);

    void encodeExecQueryResponse(
        CIMExecQueryResponseMessage* response);

    void encodeInvokeMethodResponse(
        CIMInvokeMethodResponseMessage* response);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMOperationResponseEncoder_h */
