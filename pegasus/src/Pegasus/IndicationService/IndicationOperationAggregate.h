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

#ifndef Pegasus_IndicationOperationAggregate_h
#define Pegasus_IndicationOperationAggregate_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Magic.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Server/Linkage.h>

#include "ProviderClassList.h"

PEGASUS_NAMESPACE_BEGIN

/**

    IndicationOperationAggregate is the class that manages the aggregation of
    indication provider responses to requests sent by the IndicationService.
    This class is modeled on the OperationAggregate class used by the
    CIMOperationRequestDispatcher.

    @author  Hewlett-Packard Company

 */
class PEGASUS_SERVER_LINKAGE IndicationOperationAggregate
{
public:

    /**
        Constructs an IndicationOperationAggregate instance.

        @param   origRequest           the original request, if any, received by
                                           the Indication Service
        @param   controlProviderName   Name of the control provider if the
                                       request destination is control provider.
        @param   indicationSubclasses  the list of indication subclasses for the
                                           subscription
     */
    IndicationOperationAggregate(
        CIMRequestMessage* origRequest,
        const String &controlProviderName,
        const Array<NamespaceClassList>& indicationSubclasses);

    ~IndicationOperationAggregate();

    /**
        Gets the original request, if any,  received by the IndicationService
        for this aggregation.  The original request may be Create Instance,
        Modify Instance, or Delete Instance.  In the cases of Deletion of an
        Expired or Referencing Subscription, there is no original request.

        @return  a pointer to the request, if there is a request
                 0, otherwise
    */
    CIMRequestMessage* getOrigRequest() const;

    /**
        Gets the message type of the original request, if any, received by the
        IndicationService.

        @return  the request type, if there is a request
                 0, otherwise
    */
    MessageType getOrigType() const;

    /**
        Determines if the original request requires a response, based on the
        type of the original request.  Create Instance, Modify Instance, and
        Delete Instance requests require a response.

        @return  TRUE, if original request requires a response
                 FALSE, otherwise
    */
    Boolean requiresResponse() const;

    /**
        Gets the list of indication subclasses for the subscription.

        @return  the list of indication subclasses
    */
    Array<NamespaceClassList>& getIndicationSubclasses();

    /**
        Gets the number of requests to be issued for this aggregation.

        @return  number of requests to be issued
    */
    Uint32 getNumberIssued() const;

    /**
        Sets the number of requests to be issued for this aggregation.

        Note: It is the responsibility of the caller to set the number of
        requests correctly.

        @param   i                     the number of requests
    */
    void setNumberIssued(Uint32 i);

    /**
        Appends a new response to the response list for this aggregation.

        Note: The _appendResponseMutex is used to synchronize appending of
        responses by multiple threads.

        Note: The correctness of the return value from this method depends on
        the caller having correctly set the number of requests with the
        setNumberIssued() method.

        @param   response              the response

        @return  TRUE, if all expected responses have now been received
                 FALSE, otherwise
    */
    Boolean appendResponse(CIMResponseMessage* response);

    /**
        Gets the count of responses received for this aggregation.

        @return  count of responses received
    */
    Uint32 getNumberResponses() const;

    /**
        Gets the response at the specified position in the list for this
        aggregation.

        Note: It is the responsibility of the caller to ensure that all threads
        are done using the appendResponse() method before any thread uses the
        getResponse() method.

        @return  a pointer to the response
    */
    CIMResponseMessage* getResponse(Uint32 pos) const;

    /**
        Appends a new request to the request list for this aggregation.

        Note: The _appendRequestMutex is used to synchronize appending of
        requests by multiple threads.

        @param   request               the request
    */
    void appendRequest(CIMRequestMessage* request);

    /**
        Gets the count of requests issued for this aggregation.

        @return  count of requests issued
    */
    Uint32 getNumberRequests() const;

    /**
        Gets the request at the specified position in the list for this
        aggregation.

        Note: It is the responsibility of the caller to ensure that all threads
        are done using the appendRequest() method before any thread uses the
        getRequest() method.

        @return  a pointer to the request
    */
    CIMRequestMessage* getRequest(Uint32 pos) const;

    /**
        Finds the provider that sent the response with the specified message ID.

        Note: It is the responsibility of the caller to ensure that all threads
        are done using the appendRequest() method before any thread uses the
        findProvider() method.

        @return  a ProviderClassList struct for the provider that sent the
                     response
    */
    ProviderClassList findProvider(const String& messageId) const;

private:
    /**
        Hidden (unimplemented) default constructor
     */
    IndicationOperationAggregate();

    /**
        Hidden (unimplemented) copy constructor
     */
    IndicationOperationAggregate(const IndicationOperationAggregate& x);

    /**
        Hidden (unimplemented) assignment operator
     */
    IndicationOperationAggregate& operator==(
        const IndicationOperationAggregate& x);

    CIMRequestMessage* _origRequest;
    String _controlProviderName;
    Array<NamespaceClassList> _indicationSubclasses;
    Uint32 _numberIssued;
    Array<CIMRequestMessage*> _requestList;
    Mutex _appendRequestMutex;
    Array<CIMResponseMessage*> _responseList;
    Mutex _appendResponseMutex;
    Magic<0x872FB41C> _magic;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_IndicationOperationAggregate_h */
