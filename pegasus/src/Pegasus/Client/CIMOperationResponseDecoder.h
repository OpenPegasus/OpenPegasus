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

#ifndef Pegasus_CIMOperationResponseDecoder_h
#define Pegasus_CIMOperationResponseDecoder_h

#include <fstream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Client/ClientAuthenticator.h>
#include <Pegasus/Client/CIMClientException.h>
#include <Pegasus/Client/Linkage.h>
#include "ClientPerfDataStore.h"


PEGASUS_NAMESPACE_BEGIN

class XmlParser;

/**
    This message is sent from the response decoder to the CIMClient, indicating
    an error in issuing a CIM request.
*/
class PEGASUS_CLIENT_LINKAGE ClientExceptionMessage : public Message
{
public:
    ClientExceptionMessage(Exception* clientException_)
        :
        Message(CLIENT_EXCEPTION_MESSAGE),
        clientException(clientException_)
    {
    }

    Exception* clientException;
};


/** This class receives HTTP messages and decodes them into CIM Operation
    Responses messages which it places on its output queue.
*/
class PEGASUS_CLIENT_LINKAGE CIMOperationResponseDecoder : public MessageQueue
{
public:

    /** Constuctor.
        @param outputQueue queue to receive decoded HTTP messages.
    */
    CIMOperationResponseDecoder(
        MessageQueue* outputQueue,
        MessageQueue* encoderQueue,
        ClientAuthenticator* authenticator
        );

    /** Destructor. */
    ~CIMOperationResponseDecoder();

    /** Initializes the encoder queue */
    void setEncoderQueue(MessageQueue* encoderQueue);

    /** This method is called when a message is enqueued on this queue. */
    virtual void handleEnqueue();

    /**
        This methods gives the Decoder access to the ClientPerfDataStore that
        is in CIMClientRep. A pointer to  the CIMClientRep::ClientPerfDataStore
        is passed in
    */
    void setDataStorePointer(ClientPerfDataStore* perfDataStore_ptr);

private:

    void _handleHTTPMessage(HTTPMessage* message);

    void _handleMethodResponse(
        const char* content,
        Uint32 contentLength,
        const ContentLanguageList& contentLanguages,
        Boolean reconnect,
        bool binaryResponse);

    CIMCreateClassResponseMessage* _decodeCreateClassResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMGetClassResponseMessage* _decodeGetClassResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMModifyClassResponseMessage* _decodeModifyClassResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMEnumerateClassNamesResponseMessage* _decodeEnumerateClassNamesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMEnumerateClassesResponseMessage* _decodeEnumerateClassesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMDeleteClassResponseMessage* _decodeDeleteClassResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMCreateInstanceResponseMessage* _decodeCreateInstanceResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMGetInstanceResponseMessage* _decodeGetInstanceResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMModifyInstanceResponseMessage* _decodeModifyInstanceResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMEnumerateInstanceNamesResponseMessage*
        _decodeEnumerateInstanceNamesResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMEnumerateInstancesResponseMessage* _decodeEnumerateInstancesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMDeleteInstanceResponseMessage* _decodeDeleteInstanceResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMGetPropertyResponseMessage* _decodeGetPropertyResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMSetPropertyResponseMessage* _decodeSetPropertyResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMSetQualifierResponseMessage* _decodeSetQualifierResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMGetQualifierResponseMessage* _decodeGetQualifierResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMEnumerateQualifiersResponseMessage* _decodeEnumerateQualifiersResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMDeleteQualifierResponseMessage* _decodeDeleteQualifierResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMReferenceNamesResponseMessage* _decodeReferenceNamesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMReferencesResponseMessage* _decodeReferencesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMAssociatorNamesResponseMessage* _decodeAssociatorNamesResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMAssociatorsResponseMessage* _decodeAssociatorsResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMExecQueryResponseMessage* _decodeExecQueryResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMInvokeMethodResponseMessage* _decodeInvokeMethodResponse(
        XmlParser& parser,
        const String& messageId,
        const String& methodName,
        Boolean isEmptyMethodresponseTag);

// EXP_PULL_BEGIN
    CIMOpenEnumerateInstancesResponseMessage*
        _decodeOpenEnumerateInstancesResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMOpenEnumerateInstancePathsResponseMessage*
        _decodeOpenEnumerateInstancePathsResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMOpenReferenceInstancesResponseMessage*
        _decodeOpenReferenceInstancesResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMOpenReferenceInstancePathsResponseMessage*
        _decodeOpenReferenceInstancePathsResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMOpenAssociatorInstancesResponseMessage*
        _decodeOpenAssociatorInstancesResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMOpenAssociatorInstancePathsResponseMessage*
        _decodeOpenAssociatorInstancePathsResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMOpenQueryInstancesResponseMessage* _decodeOpenQueryInstancesResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMPullInstancesWithPathResponseMessage*
        _decodePullInstancesWithPathResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMPullInstancePathsResponseMessage* _decodePullInstancePathsResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMPullInstancesResponseMessage* _decodePullInstancesResponse(
            XmlParser& parser,
            const String& messageId,
            Boolean isEmptyImethodresponseTag);

    CIMCloseEnumerationResponseMessage* _decodeCloseEnumerationResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);

    CIMEnumerationCountResponseMessage* _decodeEnumerationCountResponse(
        XmlParser& parser,
        const String& messageId,
        Boolean isEmptyImethodresponseTag);
// EXP_PULL END

    MessageQueue*        _outputQueue;

    MessageQueue*        _encoderQueue;

    ClientAuthenticator* _authenticator;

    ClientPerfDataStore* dataStore;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMOperationResponseDecoder_h */
