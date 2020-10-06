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

#ifndef Pegasus_WsmConstants_h
#define Pegasus_WsmConstants_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/WsmServer/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/PegasusVersion.h>

//
// Message Queue Names
//

#define PEGASUS_QUEUENAME_WSMREQDECODER       "WsmRequestDecoder"
#define PEGASUS_QUEUENAME_WSMRESPENCODER      "WsmResponseEncoder"
#define PEGASUS_QUEUENAME_WSMPROCESSOR        "WsmProcessor"

//
// Minimum allowable value for MaxEnvelopeSize
//

#define WSM_MIN_MAXENVELOPESIZE_VALUE 8192

//
// WS-Management URIs
//

#define WSM_ADDRESS_ANONYMOUS \
    "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous"

// See DSP0227 R5.2-1, R6-1, and R7.1-7.
#define WSM_RESOURCEURI_ALLCLASSES \
    "http://schemas.dmtf.org/wbem/wscim/1/*"

// WSM_RESOURCEURI_ALLCLASSES without the leading HOST URI prefix.
#define WSM_RESOURCEURI_ALLCLASSES_SUFFIX \
    "/wbem/wscim/1/*"

#define WSM_RESOURCEURI_CIMSCHEMAV2 \
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2"

// WSM_RESOURCEURI_CIMSCHEMAV2 without the host URI prefix.
#define WSM_RESOURCEURI_CIMSCHEMAV2_SUFFIX \
    "/wbem/wscim/1/cim-schema/2"

// RESOURCEURI while specifying existing filter for subscription
#define WSM_RESOURCEURI_INDICATION_FILTER \
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter"

// The WQL filter dialect URI.
#define WSMAN_FILTER_DIALECT_WQL "http://schemas.dmtf.org/wbem/wsman/1/WQL"

// WSMAN_FILTER_DIALECT_WQL without the host URI prefix.
#define WSMAN_FILTER_DIALECT_WQL_SUFFIX "/wbem/wsman/1/WQL"

//  The Association Filter dialect URI per DSP0027 Section 8.2
#define WSMAN_ASSOCIATION_FILTER_DIALECT \
    "http://schemas.dmtf.org/wbem/wsman/1/cimbinding/associationFilter"

// Association Filter without the host URI prefix.
#define WSMAN_ASSOCIATION_FILTER_SUFFIX \
    "/wbem/wsman/1/cimbinding/associationFilter"

//PUSH  Delivry mode for WSMAN event
#define WSMAN_DELIVEY_MODE_PUSH \
    "http://schemas.xmlsoap.org/ws/2004/08/eventing/DeliveryModes/Push"

#define WSMAN_DELIVERY_MODE_PUSH_WITH_ACK \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/PushWithAck"

//
// Action URIs
//

#define WSM_ACTION_GET \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/Get"

#define WSM_ACTION_GET_RESPONSE \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/GetResponse"

#define WSM_ACTION_PUT \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/Put"

#define WSM_ACTION_PUT_RESPONSE \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/PutResponse"

#define WSM_ACTION_CREATE \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/Create"

#define WSM_ACTION_CREATE_RESPONSE \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/CreateResponse"

#define WSM_ACTION_DELETE \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/Delete"

#define WSM_ACTION_DELETE_RESPONSE \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/DeleteResponse"

#define WSM_ACTION_ENUMERATE \
    "http://schemas.xmlsoap.org/ws/2004/09/enumeration/Enumerate"

#define WSM_ACTION_ENUMERATE_RESPONSE \
    "http://schemas.xmlsoap.org/ws/2004/09/enumeration/EnumerateResponse"

#define WSM_ACTION_PULL \
    "http://schemas.xmlsoap.org/ws/2004/09/enumeration/Pull"

#define WSM_ACTION_PULL_RESPONSE \
    "http://schemas.xmlsoap.org/ws/2004/09/enumeration/PullResponse"

#define WSM_ACTION_RELEASE \
    "http://schemas.xmlsoap.org/ws/2004/09/enumeration/Release"

#define WSM_ACTION_RELEASE_RESPONSE \
    "http://schemas.xmlsoap.org/ws/2004/09/enumeration/ReleaseResponse"

#define WSM_ACTION_WSMAN_FAULT \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/fault"

#define WSM_ACTION_WSA_FAULT \
    "http://schemas.xmlsoap.org/ws/2004/08/addressing/fault"

#define WSM_ACTION_WXF_FAULT \
    "http://schemas.xmlsoap.org/ws/2004/09/transfer/fault"

#define WSM_ACTION_WSEN_FAULT \
    "http://schemas.xmlsoap.org/ws/2004/09/enumeration/fault"

#define WSM_ACTION_WSE_FAULT \
    "http://schemas.xmlsoap.org/ws/2004/08/eventing/fault"

#define WSM_ACTION_WSMAN_EVENT\
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/Event" 

#define WSM_ACTION_WSMAN_SUBSCRIBE\
    "http://schemas.xmlsoap.org/ws/2004/08/eventing/Subscribe"

#define WSM_ACTION_SUBSCRIBE_RESPONSE\
    "http://schemas.xmlsoap.org/ws/2004/08/eventing/SubscribeResponse"

#define WSM_ACTION_WSMAN_UNSUBSCRIBE \
    "http://schemas.xmlsoap.org/ws/2004/08/eventing/Unsubscribe"

#define WSM_ACTION_UNSUBSCRIBE_RESPONSE \
    "http://schemas.xmlsoap.org/ws/2004/08/eventing/UnsubscribeResponse"

#define WSM_ACTION_SUBSCRIBE_RENEW\
    "http://schemas.xmlsoap.org/ws/2004/08/eventing/Renew"

#define WSM_ACTION_ACK\
    "http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Ack"

//
// WS-Management FaultDetail URI values
//

#define WSMAN_FAULTDETAIL_ACTIONMISMATCH \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/ActionMismatch"

#define WSMAN_FAULTDETAIL_ADDRESSINGMODE \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/AddressingMode"

#define WSMAN_FAULTDETAIL_CHARACTERSET \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/CharacterSet"

#define WSMAN_FAULTDETAIL_FRAGMENTLEVELACCESS \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/FragmentLevelAccess"

#define WSMAN_FAULTDETAIL_INVALIDRESOURCEURI \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI"

#define WSMAN_FAULTDETAIL_INVALIDVALUE \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidValue"

#define WSMAN_FAULTDETAIL_LOCALE \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/Locale"

#define WSMAN_FAULTDETAIL_OPERATIONTIMEOUT \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/OperationTimeout"

#define WSMAN_FAULTDETAIL_TYPEMISMATCH \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/TypeMismatch"

#define WSMAN_FAULTDETAIL_UNEXPECTEDSELECTORS \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/UnexpectedSelectors"

#define WSMAN_FAULTDETAIL_MINIMUMENVELOPELIMIT \
   "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/MinimumEnvelopeLimit"

#define WSMAN_FAULTDETAIL_MAXENVELOPESIZE \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/MaxEnvelopeSize"

#define WSMAN_FAULTDETAIL_UNREPORTABLESUCCESS \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/UnreportableSuccess"

#define WSMAN_FAULTDETAIL_ENUMERATION_MODE_UNSUPPORTED \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/EnumerationMode"

#define WSMAN_FAULTDETAIL_CONNECTION_RETRY_UNSUPPORTED \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/DeliveryRetries"

#define WSMAN_FAULTDETAIL_HEARTBEATS_UNSUPPORTED \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/Heartbeats"

#define WSMAN_FAULTDETAIL_BOOKMARKS_UNSUPPORTED \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/Bookmarks"

#define WSMAN_FAULTDETAIL_OPTION_LIMIT \
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/OptionLimit"

// The namespace to encode instance response data with.
#define PEGASUS_INSTANCE_NS "p"

// The namespace to encode invoke response data with.
#define PEGASUS_INVOKE_NS "n1"

// The current version URI for WS-Management.
#define WSMAN_PROTOCOL_VERSION "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"

// Vendor of this WS-Management implementation.
#define WSMAN_PRODUCT_VENDOR "The Open Group (OpenPegasus)"

// Version of this WS-Management implementation.

#define WSMAN_PRODUCT_VERSION PEGASUS_PRODUCT_VERSION

const int PEGASUS_WS_UUID_LENGTH=5;

enum WsmbPolymorphismMode
{
    WSMB_PM_UNKNOWN,
    WSMB_PM_NONE,
    WSMB_PM_EXCLUDE_SUBCLASS_PROPERTIES,
    WSMB_PM_INCLUDE_SUBCLASS_PROPERTIES
};

enum WsenEnumerationMode
{
    WSEN_EM_UNKNOWN,
    WSEN_EM_OBJECT,
    WSEN_EM_EPR,
    WSEN_EM_OBJECT_AND_EPR
};

PEGASUS_NAMESPACE_BEGIN

PEGASUS_WSMSERVER_LINKAGE extern const char* PEGASUS_WS_SUBSCRIBE;

PEGASUS_WSMSERVER_LINKAGE extern const char* PEGASUS_WS_DELIVERY;

PEGASUS_WSMSERVER_LINKAGE extern const char* PEGASUS_WS_DELMODE;

PEGASUS_WSMSERVER_LINKAGE extern const char* PEGASUS_WS_NOTIFYTO;

PEGASUS_WSMSERVER_LINKAGE extern const char* PEGASUS_WS_CIMNAMESPACE;

PEGASUS_WSMSERVER_LINKAGE extern const char* PEGASUS_WS_SUB_DURATION;

//
// CIM Namespace Names
//

extern const CIMNamespaceName PEGASUS_DEFAULT_WSM_NAMESPACE;

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmConstants_h */
