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

#ifndef Pegasus_CIMExportRequestDecoder_h
#define Pegasus_CIMExportRequestDecoder_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/ExportServer/Linkage.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>

PEGASUS_NAMESPACE_BEGIN

class XmlParser;

/** This class decodes CIM operation requests and passes them down-stream.
 */
class PEGASUS_EXPORT_SERVER_LINKAGE CIMExportRequestDecoder :
    public MessageQueue
{
public:

    typedef MessageQueue Base;

    CIMExportRequestDecoder(
        MessageQueueService* outputQueue,
        Uint32 returnQueueId);

    ~CIMExportRequestDecoder();

    void sendResponse(
        Uint32 queueId,
        Buffer& message,
        Boolean closeConnect);

    void sendEMethodError(
        Uint32 queueId,
        HttpMethod httpMethod,
        const String& messageId,
        const String& methodName,
        const CIMException& cimException,
        Boolean closeConnect);

    void sendHttpError(
        Uint32 queueId,
        const String& status,
        const String& cimError = String::EMPTY,
        const String& messageBody = String::EMPTY,
        Boolean closeConnect = false);

    virtual void handleEnqueue(Message *);

    virtual void handleEnqueue();

    void handleHTTPMessage(HTTPMessage* httpMessage);

    void handleMethodRequest(
        Uint32 queueId,
        HttpMethod httpMethod,
        char* content,
        const String& requestUri,
        const char* cimProtocolVersionInHeader,
        const char* cimExportMethodInHeader,
        const String& userName,
        const String& ipAddress,
        const AcceptLanguageList& httpAcceptLanguages,
        const ContentLanguageList& httpContentLanguages,
        Boolean closeConnect);

    CIMExportIndicationRequestMessage* decodeExportIndicationRequest(
        Uint32 queueId,
        XmlParser& parser,
        const String& messageId,
        const String& nameSpace);

    /**
        Sets the flag to indicate whether or not the CIMServer is
        shutting down.
    */
    void setServerTerminating(Boolean flag);

private:

    MessageQueue* _outputQueue;

    // Queue where responses should be enqueued.
    Uint32 _returnQueueId;

    // Flag to indicate whether or not the CIMServer is shutting down.
    Boolean _serverTerminating;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMExportRequestDecoder_h */
