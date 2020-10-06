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

#ifndef Pegasus_CIMOperationRequestEncoder_h
#define Pegasus_CIMOperationRequestEncoder_h

#include <fstream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Client/ClientAuthenticator.h>
#include <Pegasus/Client/Linkage.h>
#include <Pegasus/Client/ClientPerfDataStore.h>

PEGASUS_NAMESPACE_BEGIN

/** This class receives CIM Operation Request messages on its input queue
    and encodes them into HTTP messages which it places on its output queue.
*/
class PEGASUS_CLIENT_LINKAGE CIMOperationRequestEncoder : public MessageQueue
{
public:

    /** Constuctor.
        @param outputQueue queue to receive encoded HTTP messages.
        @param hostName Name of the target host for the encoded requests.
            I.e., the value of the HTTP Host header.
    */
    CIMOperationRequestEncoder(
        MessageQueue* outputQueue,
        const String& hostName,
        ClientAuthenticator* authenticator,
        bool binaryRequest = false,
        bool binaryResponse = false);

    /** Destructor. */
    ~CIMOperationRequestEncoder();

    /** This method is called when a message is enqueued on this queue. */
    virtual void handleEnqueue();

    /**
        Gives the Encoder access to the ClientPerfDataStore that
        is in CIMClientRep. A pointer to the CIMClientRep::ClientPerfDataStore
        is passed in.
     */
    void setDataStorePointer(ClientPerfDataStore* perfDataStore_ptr);

private:

    void _encodeCreateClassRequest(
        CIMCreateClassRequestMessage* message);

    void _encodeGetClassRequest(
        CIMGetClassRequestMessage* message);

    void _encodeModifyClassRequest(
        CIMModifyClassRequestMessage* message);

    void _encodeEnumerateClassNamesRequest(
        CIMEnumerateClassNamesRequestMessage* message);

    void _encodeEnumerateClassesRequest(
        CIMEnumerateClassesRequestMessage* message);

    void _encodeDeleteClassRequest(
        CIMDeleteClassRequestMessage* message);

    void _encodeCreateInstanceRequest(
        CIMCreateInstanceRequestMessage* message);

    void _encodeGetInstanceRequest(
        CIMGetInstanceRequestMessage* message);

    void _encodeModifyInstanceRequest(
        CIMModifyInstanceRequestMessage* message);

    void _encodeEnumerateInstanceNamesRequest(
        CIMEnumerateInstanceNamesRequestMessage* message);

    void _encodeEnumerateInstancesRequest(
        CIMEnumerateInstancesRequestMessage* message);

    void _encodeDeleteInstanceRequest(
        CIMDeleteInstanceRequestMessage* message);

    void _encodeGetPropertyRequest(
        CIMGetPropertyRequestMessage* message);

    void _encodeSetPropertyRequest(
        CIMSetPropertyRequestMessage* message);

    void _encodeSetQualifierRequest(
        CIMSetQualifierRequestMessage* message);

    void _encodeGetQualifierRequest(
        CIMGetQualifierRequestMessage* message);

    void _encodeEnumerateQualifiersRequest(
        CIMEnumerateQualifiersRequestMessage* message);

    void _encodeDeleteQualifierRequest(
        CIMDeleteQualifierRequestMessage* message);

    void _encodeReferenceNamesRequest(
        CIMReferenceNamesRequestMessage* message);

    void _encodeReferencesRequest(
        CIMReferencesRequestMessage* message);

    void _encodeAssociatorNamesRequest(
        CIMAssociatorNamesRequestMessage* message);

    void _encodeAssociatorsRequest(
        CIMAssociatorsRequestMessage* message);

    void _encodeExecQueryRequest(
        CIMExecQueryRequestMessage* message);

    void _encodeInvokeMethodRequest(
        CIMInvokeMethodRequestMessage* message);

 // EXP_PULL_BEGIN
    void _encodeOpenEnumerateInstancesRequest(
        CIMOpenEnumerateInstancesRequestMessage* message);

    void _encodeOpenEnumerateInstancePathsRequest(
        CIMOpenEnumerateInstancePathsRequestMessage* message);

    void _encodeOpenReferenceInstancePathsRequest(
        CIMOpenReferenceInstancePathsRequestMessage* message);

    void _encodeOpenReferenceInstancesRequest(
        CIMOpenReferenceInstancesRequestMessage* message);

    void _encodeOpenAssociatorInstancePathsRequest(
        CIMOpenAssociatorInstancePathsRequestMessage* message);

    void _encodeOpenAssociatorInstancesRequest(
        CIMOpenAssociatorInstancesRequestMessage* message);

    void _encodePullInstancesWithPathRequest(
        CIMPullInstancesWithPathRequestMessage* message);

    void _encodePullInstancePathsRequest(
        CIMPullInstancePathsRequestMessage* message);

    void _encodePullInstancesRequest(
        CIMPullInstancesRequestMessage* message);

    void _encodeCloseEnumerationRequest(
        CIMCloseEnumerationRequestMessage* message);

    void _encodeOpenQueryInstancesRequest(
        CIMOpenQueryInstancesRequestMessage* message);

    void _encodeEnumerationCountRequest(
        CIMEnumerationCountRequestMessage* message);
 //EXP_PULL_END

    void _sendRequest(Buffer& buffer);

    MessageQueue* _outputQueue;
    CString _hostName;
    ClientAuthenticator* _authenticator;

    ClientPerfDataStore* dataStore_prt;
    bool _binaryRequest;
    bool _binaryResponse;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMOperationRequestEncoder_h */
