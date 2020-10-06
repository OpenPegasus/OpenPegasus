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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include "WsmFault.h"

PEGASUS_NAMESPACE_BEGIN

//
// This table represents the normative fault list for WS-Management, as
// defined in DSP0226 section 14.6.
//

WsmFault::FaultTableEntry WsmFault::_faultTable[] =
{
    {
        WsmFault::s_Sender,
        WsmFault::wsman_AccessDenied,
        WsmNamespaces::WS_MAN,
        "AccessDenied",
        // The sender was not authorized to access the resource.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsa_ActionNotSupported,
        WsmNamespaces::WS_ADDRESSING,
        "ActionNotSupported",
        // The action is not supported by the service.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_AlreadyExists,
        WsmNamespaces::WS_MAN,
        "AlreadyExists",
        // The sender attempted to create a resource that already exists.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsen_CannotProcessFilter,
        WsmNamespaces::WS_ENUMERATION,
        "CannotProcessFilter",
        // The requested filter could not be processed.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_Concurrency,
        WsmNamespaces::WS_MAN,
        "Concurrency",
        // The action could not be completed due to concurrency or
        // locking problems.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wse_DeliveryModeRequestedUnavailable,
        WsmNamespaces::WS_EVENTING,
        "DeliveryModeRequestedUnavailable",
        // The requested delivery mode is not supported.
    },
    {
        WsmFault::s_Receiver,
        WsmFault::wsman_DeliveryRefused,
        WsmNamespaces::WS_MAN,
        "DeliveryRefused",
        // The receiver refuses to accept delivery of events and
        // requests that the subscription be canceled.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsa_DestinationUnreachable,
        WsmNamespaces::WS_ADDRESSING,
        "DestinationUnreachable",
        // No route can be determined to reach the destination role
        // defined by the WS-Addressing To header.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_EncodingLimit,
        WsmNamespaces::WS_MAN,
        "EncodingLimit",
        // An internal encoding limit was exceeded in a request or
        // would be violated if the message was processed.
    },
    {
        WsmFault::s_Receiver,
        WsmFault::wsa_EndpointUnavailable,
        WsmNamespaces::WS_ADDRESSING,
        "EndpointUnavailable",
        // The specified endpoint is currently unavailable.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wse_EventSourceUnableToProcess,
        WsmNamespaces::WS_EVENTING,
        "EventSourceUnableToProcess",
        // The event source cannot process the subscription.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsen_FilterDialectRequestedUnavailable,
        WsmNamespaces::WS_ENUMERATION,
        "FilterDialectRequestedUnavailable",
        // The requested filtering dialect is not supported.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wse_FilteringNotSupported,
        WsmNamespaces::WS_EVENTING,
        "FilteringNotSupported",
        // Filtering over the event source is not supported.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsen_FilteringNotSupported,
        WsmNamespaces::WS_ENUMERATION,
        "FilteringNotSupported",
        // Filtered enumeration is not supported.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wse_FilteringRequestedUnavailable,
        WsmNamespaces::WS_EVENTING,
        "FilteringRequestedUnavailable",
        // The requested filter dialect is not supported.
    },
    {
        WsmFault::s_Receiver,
        WsmFault::wsman_InternalError,
        WsmNamespaces::WS_MAN,
        "InternalError",
        // The service cannot comply with the request due to internal
        // processing errors.
    },
    {
        WsmFault::s_Receiver,
        WsmFault::wsman_FragmentDialectNotSupported,
        WsmNamespaces::WS_MAN,
        "FragmentDialectNotSupported",
        // The requested fragment filtering dialect or language
        // is not supported.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_InvalidBookmark,
        WsmNamespaces::WS_MAN,
        "InvalidBookmark",
        // The bookmark supplied with the subscription is not valid.
    },
    {
        WsmFault::s_Receiver,
        WsmFault::wsen_InvalidEnumerationContext,
        WsmNamespaces::WS_ENUMERATION,
        "InvalidEnumerationContext",
        // The supplied enumeration context is invalid.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wse_InvalidExpirationTime,
        WsmNamespaces::WS_EVENTING,
        "InvalidExpirationTime",
        // The expiration time is not valid.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsen_InvalidExpirationTime,
        WsmNamespaces::WS_ENUMERATION,
        "InvalidExpirationTime",
        // The expiration time is not valid.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wse_InvalidMessage,
        WsmNamespaces::WS_EVENTING,
        "InvalidMessage",
        // The request message has unknown or invalid content and
        // cannot be processed.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsa_InvalidMessageInformationHeader,
        WsmNamespaces::WS_ADDRESSING,
        "InvalidMessageInformationHeader",
        // A message information header is not valid and the message
        // cannot be processed.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_InvalidOptions,
        WsmNamespaces::WS_MAN,
        "InvalidOptions",
        // One or more options are not valid.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_InvalidParameter,
        WsmNamespaces::WS_MAN,
        "InvalidParameter",
        // An operation parameter is not valid.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wxf_InvalidRepresentation,
        WsmNamespaces::WS_TRANSFER,
        "InvalidRepresentation",
        // The XML content is not valid.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_InvalidSelectors,
        WsmNamespaces::WS_MAN,
        "InvalidSelectors",
        // The selectors for the resource are not valid.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsa_MessageInformationHeaderRequired,
        WsmNamespaces::WS_ADDRESSING,
        "MessageInformationHeaderRequired",
        // A required header is missing.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_NoAck,
        WsmNamespaces::WS_MAN,
        "NoAck",
        // The receiver did not acknowledge the event delivery.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_QuotaLimit,
        WsmNamespaces::WS_MAN,
        "QuotaLimit",
        // The service is busy servicing other requests.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_SchemaValidationError,
        WsmNamespaces::WS_MAN,
        "SchemaValidationError",
        // The supplied SOAP violates the corresponding XML schema definition.
    },
    {
        WsmFault::s_Receiver,
        WsmFault::wsen_TimedOut,
        WsmNamespaces::WS_ENUMERATION,
        "TimedOut",
        // The enumerator has timed out and is no longer valid.
    },
    {
        WsmFault::s_Receiver,
        WsmFault::wsman_TimedOut,
        WsmNamespaces::WS_MAN,
        "TimedOut",
        // The operation has timed out.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wse_UnableToRenew,
        WsmNamespaces::WS_EVENTING,
        "UnableToRenew",
        // The subscription could not be renewed.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wse_UnsupportedExpirationType,
        WsmNamespaces::WS_EVENTING,
        "UnsupportedExpirationType",
        // The specified expiration type is not supported.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsen_UnsupportedExpirationType,
        WsmNamespaces::WS_ENUMERATION,
        "UnsupportedExpirationType",
        // The specified expiration type is not supported.
    },
    {
        WsmFault::s_Sender,
        WsmFault::wsman_UnsupportedFeature,
        WsmNamespaces::WS_MAN,
        "UnsupportedFeature",
        // The specified feature is not supported.
    },
    {
        WsmFault::s_Sender,
        wsmb_PolymorphismModeNotSupported,
        WsmNamespaces::WS_CIM_BINDING,
        "PolymorphismModeNotSupported",
        // The specified polymorphism mode is not supported.
    }
};


WsmFault::WsmFault(
    Subcode subcode,
    const MessageLoaderParms& msgParms,
    const String& faultDetail)
    : _subcode(subcode),
      _faultDetail(faultDetail)
{
    _reason = MessageLoader::getMessage(
        const_cast<MessageLoaderParms &>(msgParms));
    // Must be after MessageLoader::getMessage call
    _reasonLanguage = msgParms.contentlanguages;
}

WsmFault::WsmFault(
    Subcode subcode,
    const String& reason,
    const ContentLanguageList& reasonLanguage,
    const String& faultDetail)
    : _subcode(subcode),
      _reason(reason),
      _reasonLanguage(reasonLanguage),
      _faultDetail(faultDetail)
{
}

WsmFault::WsmFault(const WsmFault& fault)
    : _subcode(fault._subcode),
      _reason(fault._reason),
      _reasonLanguage(fault._reasonLanguage),
      _faultDetail(fault._faultDetail)
{
}

const char* WsmFault::getAction() const
{
    if (_faultTable[_subcode].nsType == WsmNamespaces::WS_MAN)
    {
        return WSM_ACTION_WSMAN_FAULT;
    }

    if (_faultTable[_subcode].nsType == WsmNamespaces::WS_ADDRESSING)
    {
        return WSM_ACTION_WSA_FAULT;
    }

    if (_faultTable[_subcode].nsType == WsmNamespaces::WS_TRANSFER)
    {
        return WSM_ACTION_WXF_FAULT;
    }

    if (_faultTable[_subcode].nsType == WsmNamespaces::WS_ENUMERATION)
    {
        return WSM_ACTION_WSEN_FAULT;
    }

    PEGASUS_ASSERT(_faultTable[_subcode].nsType == WsmNamespaces::WS_EVENTING);
    return WSM_ACTION_WSE_FAULT;
}

String WsmFault::getCode() const
{
    String strCode = WsmNamespaces::supportedNamespaces[
        WsmNamespaces::SOAP_ENVELOPE].localName;
    if (_faultTable[_subcode].code == s_Sender)
        strCode = strCode + ":Sender";
    else
        strCode = strCode + ":Receiver";

    return strCode;
}

String WsmFault::getSubcode() const
{
    return
        String(WsmNamespaces::supportedNamespaces[
            _faultTable[_subcode].nsType].localName) +
        ":" +
        _faultTable[_subcode].subcodeStr;
}

String WsmFault::getReason() const
{
    return _reason;
}

ContentLanguageList WsmFault::getReasonLanguage() const
{
    return _reasonLanguage;
}

String WsmFault::getFaultDetail() const
{
    return _faultDetail;
}


//
// SoapNotUnderstoodFault
//

SoapNotUnderstoodFault::SoapNotUnderstoodFault(
    const String& nameSpace,
    const String& headerName)
    : _nameSpace(nameSpace),
      _headerName(headerName)
{
    MessageLoaderParms parms(
        "WsmServer.WsmFault.SOAP_HEADER_NOT_UNDERSTOOD",
        "Header not understood.");
    _message = MessageLoader::getMessage(parms);
    _messageLanguage = parms.contentlanguages;
}

String SoapNotUnderstoodFault::getNamespace() const
{
    return _nameSpace;
}

String SoapNotUnderstoodFault::getHeaderName() const
{
    return _headerName;
}

String SoapNotUnderstoodFault::getMessage() const
{
    return _message;
}

ContentLanguageList SoapNotUnderstoodFault::getMessageLanguage() const
{
    return _messageLanguage;
}

PEGASUS_NAMESPACE_END
