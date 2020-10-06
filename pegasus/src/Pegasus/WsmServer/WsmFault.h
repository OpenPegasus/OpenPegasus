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

#ifndef Pegasus_WsmFault_h
#define Pegasus_WsmFault_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/WsmServer/WsmRequest.h>
#include <Pegasus/WsmServer/WsmUtils.h>

PEGASUS_NAMESPACE_BEGIN


class PEGASUS_WSMSERVER_LINKAGE  WsmFault
{
public:

    enum Code
    {
        s_Sender,
        s_Receiver
    };

    enum Subcode
    {
        wsman_AccessDenied,
        wsa_ActionNotSupported,
        wsman_AlreadyExists,
        wsen_CannotProcessFilter,
        wsman_Concurrency,
        wse_DeliveryModeRequestedUnavailable,
        wsman_DeliveryRefused,
        wsa_DestinationUnreachable,
        wsman_EncodingLimit,
        wsa_EndpointUnavailable,
        wse_EventSourceUnableToProcess,
        wsen_FilterDialectRequestedUnavailable,
        wse_FilteringNotSupported,
        wsen_FilteringNotSupported,
        wse_FilteringRequestedUnavailable,
        wsman_InternalError,
        wsman_FragmentDialectNotSupported,
        wsman_InvalidBookmark,
        wsen_InvalidEnumerationContext,
        wse_InvalidExpirationTime,
        wsen_InvalidExpirationTime,
        wse_InvalidMessage,
        wsa_InvalidMessageInformationHeader,
        wsman_InvalidOptions,
        wsman_InvalidParameter,
        wxf_InvalidRepresentation,
        wsman_InvalidSelectors,
        wsa_MessageInformationHeaderRequired,
        wsman_NoAck,
        wsman_QuotaLimit,
        wsman_SchemaValidationError,
        wsen_TimedOut,
        wsman_TimedOut,
        wse_UnableToRenew,
        wse_UnsupportedExpirationType,
        wsen_UnsupportedExpirationType,
        wsman_UnsupportedFeature,
        wsmb_PolymorphismModeNotSupported
    };

    struct FaultTableEntry
    {
        Code code;
        Subcode subcode;
        WsmNamespaces::Type nsType;
        const char* subcodeStr;
    };

    WsmFault(
        Subcode subcode,
        const MessageLoaderParms& msgParms,
        const String& faultDetail = String::EMPTY);

    WsmFault(
        Subcode subcode,
        const String& reason = String::EMPTY,
        const ContentLanguageList& reasonLanguage = ContentLanguageList(),
        const String& faultDetail = String::EMPTY);

    WsmFault(const WsmFault& fault);

    ~WsmFault()
    {
    }

    const char* getAction() const;
    String getCode() const;
    String getSubcode() const;
    String getReason() const;
    ContentLanguageList getReasonLanguage() const;
    String getFaultDetail() const;
    // Note: Other types of s:Detail data are not supported.

private:

    static FaultTableEntry _faultTable[];

    Subcode _subcode;
    String _reason;
    ContentLanguageList _reasonLanguage;
    String _faultDetail;
};

class PEGASUS_WSMSERVER_LINKAGE SoapNotUnderstoodFault
{
public:

    /**
        Constructs a SOAP NotUnderstood fault for a specified element name and
        namespace.  If the element is not scoped within a namespace, the
        nameSpace parameter must be left blank.
    */
    SoapNotUnderstoodFault(
        const String& nameSpace,
        const String& headerName);

    String getNamespace() const;
    String getHeaderName() const;
    String getMessage() const;
    ContentLanguageList getMessageLanguage() const;

private:

    String _nameSpace;
    String _headerName;
    String _message;
    ContentLanguageList _messageLanguage;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmFault_h */
