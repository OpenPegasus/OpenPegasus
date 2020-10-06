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

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include "IndicationOperationAggregate.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

IndicationOperationAggregate::IndicationOperationAggregate(
    CIMRequestMessage* origRequest,
    const String &controlProviderName,
    const Array<NamespaceClassList>& indicationSubclasses)
:   _origRequest(origRequest),
    _controlProviderName(controlProviderName),
    _indicationSubclasses(indicationSubclasses),
    _numberIssued(0)
{
}

IndicationOperationAggregate::~IndicationOperationAggregate()
{
    delete _origRequest;

    Uint32 numberRequests = _requestList.size();
    for (Uint32 i = 0; i < numberRequests; i++)
    {
        delete _requestList[i];
    }

    Uint32 numberResponses = _responseList.size();
    for (Uint32 j = 0; j < numberResponses; j++)
    {
        delete _responseList[j];
    }
}

CIMRequestMessage* IndicationOperationAggregate::getOrigRequest() const
{
    return _origRequest;
}

MessageType IndicationOperationAggregate::getOrigType() const
{
    if (_origRequest == 0)
    {
        return DUMMY_MESSAGE;
    }
    else
    {
        return _origRequest->getType();
    }
}

Boolean IndicationOperationAggregate::requiresResponse() const
{
    if ((getOrigType() == CIM_CREATE_INSTANCE_REQUEST_MESSAGE) ||
        (getOrigType() == CIM_MODIFY_INSTANCE_REQUEST_MESSAGE) ||
        (getOrigType() == CIM_DELETE_INSTANCE_REQUEST_MESSAGE))
    {
        return true;
    }
    else
    {
        return false;
    }
}

Array<NamespaceClassList>&
    IndicationOperationAggregate::getIndicationSubclasses()
{
    return _indicationSubclasses;
}

Uint32 IndicationOperationAggregate::getNumberIssued() const
{
    return _numberIssued;
}

void IndicationOperationAggregate::setNumberIssued(Uint32 i)
{
    _numberIssued = i;
}

Boolean IndicationOperationAggregate::appendResponse(
    CIMResponseMessage* response)
{
    AutoMutex autoMut(_appendResponseMutex);
    _responseList.append(response);
    Boolean returnValue = (getNumberResponses() == getNumberIssued());

    return returnValue;
}

Uint32 IndicationOperationAggregate::getNumberResponses() const
{
    return _responseList.size();
}

CIMResponseMessage* IndicationOperationAggregate::getResponse(Uint32 pos) const
{
    return _responseList[pos];
}

void IndicationOperationAggregate::appendRequest(
    CIMRequestMessage* request)
{
    AutoMutex autoMut(_appendRequestMutex);
    _requestList.append(request);
}

Uint32 IndicationOperationAggregate::getNumberRequests() const
{
    return _requestList.size();
}

CIMRequestMessage* IndicationOperationAggregate::getRequest(Uint32 pos) const
{
    return _requestList[pos];
}

ProviderClassList IndicationOperationAggregate::findProvider(
    const String& messageId) const
{
    //
    //  Look in the list of requests for the request with the message ID
    //  corresponding to the message ID in the response
    //
    ProviderClassList provider;
    Uint32 numberRequests = getNumberRequests();
    for (Uint32 i = 0; i < numberRequests; i++)
    {
        if (getRequest(i)->messageId == messageId )
        {
            //
            //  Get the provider and provider module from the matching request
            //
            switch (getRequest(i)->getType())
            {
                case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
                {
                    CIMCreateSubscriptionRequestMessage* request =
                        (CIMCreateSubscriptionRequestMessage *) getRequest(i);
                    ProviderIdContainer pidc = request->operationContext.get
                        (ProviderIdContainer::NAME);
                    provider.provider = pidc.getProvider();
                    provider.providerModule = pidc.getModule();
                    NamespaceClassList nscl;
                    nscl.nameSpace = request->nameSpace;
                    nscl.classList = request->classNames;
                    provider.classList.append(nscl);
                    provider.controlProviderName = _controlProviderName;
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
                    provider.isRemoteNameSpace = pidc.isRemoteNameSpace();
                    provider.remoteInfo = pidc.getRemoteInfo();
#endif
                    break;
                }

                case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
                {
                    CIMDeleteSubscriptionRequestMessage* request =
                        (CIMDeleteSubscriptionRequestMessage *) getRequest(i);
                    ProviderIdContainer pidc = request->operationContext.get
                        (ProviderIdContainer::NAME);
                    provider.provider = pidc.getProvider();
                    provider.providerModule = pidc.getModule();
                    NamespaceClassList nscl;
                    nscl.nameSpace = request->nameSpace;
                    nscl.classList = request->classNames;
                    provider.classList.append(nscl);
                    provider.controlProviderName = _controlProviderName;
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
                    provider.isRemoteNameSpace = pidc.isRemoteNameSpace();
                    provider.remoteInfo = pidc.getRemoteInfo();
#endif
                    break;
                }
                default:
                {
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
                    break;
                }
            }

            return provider;
        }
    }

    //
    //  No request found with message ID matching message ID from response
    //
    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
    return provider;
}

PEGASUS_NAMESPACE_END
