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

#ifndef Pegasus_CIMOperationRequestAuthorizer_h
#define Pegasus_CIMOperationRequestAuthorizer_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Server/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/*
 *
 * The CIM Operation request authorizer: checks if the user is authorized
 * to issue CIM operations
*/
class PEGASUS_SERVER_LINKAGE CIMOperationRequestAuthorizer : public MessageQueue
{
public:

    typedef MessageQueue Base;

    CIMOperationRequestAuthorizer(
        MessageQueueService* outputQueue);

    ~CIMOperationRequestAuthorizer();

    void sendResponse(
        Uint32 queueId,
        Buffer& message);

    void sendIMethodError(
        Uint32 queueId,
        HttpMethod httpMethod,
        const String& messageId,
        const CIMName& methodName,
        const CIMException& cimException);

    void sendMethodError(
        Uint32 queueId,
        HttpMethod httpMethod,
        const String& messageId,
        const CIMName& methodName,
        const CIMException& cimException);

    virtual void handleEnqueue(Message*);

    virtual void handleEnqueue();

    /** Sets the flag to indicate whether or not the CIMServer is
        shutting down.
    */
    void setServerTerminating(Boolean flag);

private:

    // Constant defining the user group name separator
    static const char _GROUPNAME_SEPARATOR;

    // Get a list of authorized user groups
    Array<String> _getAuthorizedUserGroups();

    Array<String> _authorizedUserGroups;
    // Do not make this an AutoPtr as the MQS has to be deleted
    // by somebody else, not this class.
    MessageQueueService* _outputQueue;

    // Flag to indicate whether or not the CIMServer is shutting down.
    Boolean _serverTerminating;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMOperationRequestAuthorizer_h */
