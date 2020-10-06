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

#ifndef Pegasus_HTTPAuthenticatorDelegator_h
#define Pegasus_HTTPAuthenticatorDelegator_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/HTTPAcceptor.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Security/Authentication/AuthenticationManager.h>
#include <Pegasus/Server/Linkage.h>

#include <Pegasus/Repository/CIMRepository.h>

#ifdef PEGASUS_ENABLE_SESSION_COOKIES
#include <Pegasus/Security/Authentication/Cookies.h>
#endif

PEGASUS_NAMESPACE_BEGIN

/**
   This class parses the HTTP header in the HTTPMessage passed to it and
   performs authentication based on the authentication specified in the
   configuration. It sends the challenge for unauthorized requests and
   validates the response.
*/
class PEGASUS_SERVER_LINKAGE HTTPAuthenticatorDelegator :
    public MessageQueue
{
public:

    typedef MessageQueue Base;

    /** Constructor. */
    HTTPAuthenticatorDelegator(
        Uint32 cimOperationMessageQueueId,
        Uint32 cimExportMessageQueueId,
        CIMRepository* repository);

    /** Destructor. */
    ~HTTPAuthenticatorDelegator();

    /**
        This method is overridden here to force the message to be handled
        by the same thread that enqueues it.  See Bugzilla 641 for details.
     */
    virtual void enqueue(Message* message);

    virtual void handleEnqueue(Message*);
    virtual void handleEnqueue();

    void handleHTTPMessage(HTTPMessage* httpMessage, Boolean& deleteMessage);

    void setWsmQueueId(Uint32 wsmanOperationMessageQueueId)
    {
        _wsmanOperationMessageQueueId = wsmanOperationMessageQueueId;
    }
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    void setWebQueueId(Uint32 webOperationMessageQueueId)
    {
        _webOperationMessageQueueId = webOperationMessageQueueId;
    }
#endif

    void setRsQueueId(Uint32 rsOperationMessageQueueId)
    {
        _rsOperationMessageQueueId = rsOperationMessageQueueId;
    }

    void idleTimeCleanup();

private:

    void _sendResponse(
        Uint32 queueId,
        Buffer& message,
        Boolean closeConnect);

    void _sendChallenge(
        Uint32 queueId,
        const String& errorDetail,
        const String& authResponse,
        Boolean closeConnect);

    void _sendHttpError(
        Uint32 queueId,
        const String& status,
        const String& cimError = String::EMPTY,
        const String& pegasusError = String::EMPTY,
        Boolean closeConnect = false);

#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    /**
     * Create a cookie after successful authentication.
     */
    void _createCookie(HTTPMessage *httpMessage);
#endif

    Uint32 _cimOperationMessageQueueId;
    Uint32 _cimExportMessageQueueId;
    Uint32 _wsmanOperationMessageQueueId;
    Uint32 _rsOperationMessageQueueId;
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    Uint32 _webOperationMessageQueueId;
#endif

    AutoPtr<AuthenticationManager> _authenticationManager;
#ifdef PEGASUS_ENABLE_SESSION_COOKIES
    AutoPtr<HTTPSessionList> _sessions;
#endif

private:
      CIMRepository* _repository;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_HTTPAuthenticatorDelegator_h */
