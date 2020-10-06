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
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_IndicationConstants_h
#define Pegasus_IndicationConstants_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Server/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    Values for the Repeat Notification Policy property of the Subscription
    class, as defined in the CIM Events MOF
 */
enum RepeatNotificationPolicy {_POLICY_UNKNOWN = 0, _POLICY_OTHER = 1,
     _POLICY_NONE = 2, _POLICY_SUPPRESS = 3, _POLICY_DELAY = 4};

/**
    Values for the On Fatal Error Policy property of the Subscription
    class, as defined in the CIM Events MOF
 */
enum OnFatalErrorPolicy {_ERRORPOLICY_OTHER = 1, _ERRORPOLICY_IGNORE = 2,
    _ERRORPOLICY_DISABLE = 3, _ERRORPOLICY_REMOVE = 4};

/**
    Values for the Probable Cause property of the Alert Indication
    class, as defined in the CIM Events MOF
    Note: not all possible values have been included
 */
enum ProbableCause {_CAUSE_UNKNOWN = 0, _CAUSE_OTHER = 1};

/**
    Values for the Alert Type property of the Alert Indication class,
    as defined in the CIM Events MOF
 */
enum AlertType {_TYPE_OTHER = 1, _TYPE_COMMUNICATIONS = 2, _TYPE_QOS = 3,
     _TYPE_PROCESSING = 4, _TYPE_DEVICE = 5, _TYPE_ENVIRONMENTAL = 6,
     _TYPE_MODELCHANGE = 7, _TYPE_SECURITY = 8};


//
//  Class names
//

/**
    The name of the CIMOM Shutdown alert indication class
 */
//
//  ATTN: Update once CimomShutdownAlertIndication has been defined
//
extern const CIMName _CLASS_CIMOM_SHUTDOWN_ALERT;

/**
    The name of the No Provider alert indication class
 */
//
//  ATTN: Update once NoProviderAlertIndication has been defined
//
extern const CIMName _CLASS_NO_PROVIDER_ALERT;

/**
    The name of the CIMOM shutdown alert indication class
 */
//
//  ATTN: Update once ProviderTerminatedAlertIndication has been defined
//
extern const CIMName _CLASS_PROVIDER_TERMINATED_ALERT;


//
//  Property names
//

/**
    The name of the Other Subscription State property for Indication
    Subscription class
 */
extern const CIMName _PROPERTY_OTHERSTATE;

/**
    The name of the Repeat Notification Policy property for indication
    subscription class
 */
extern const CIMName _PROPERTY_REPEATNOTIFICATIONPOLICY;

/**
    The name of the Other Repeat Notification Policy property for
    indication subscription class
 */
extern const CIMName _PROPERTY_OTHERREPEATNOTIFICATIONPOLICY;

/**
    The name of the On Fatal Error Policy property for Indication Subscription
    class
 */
extern const CIMName _PROPERTY_ONFATALERRORPOLICY;

/**
    The name of the Other On Fatal Error Policy property for Indication
    Subscription class
 */
extern const CIMName _PROPERTY_OTHERONFATALERRORPOLICY;

/**
    The name of the Time Of Last State Change property for Indication
    Subscription class
 */
extern const CIMName _PROPERTY_LASTCHANGE;

/**
    The name of the Subscription Start Time property for Indication
    Subscription class
 */
extern const CIMName _PROPERTY_STARTTIME;

/**
    The name of the Subscription Duration property for Indication
    Subscription class
 */
extern const CIMName _PROPERTY_DURATION;

/**
    The name of the Subscription Time Remaining property for Indication
    Subscription class
 */
extern const CIMName _PROPERTY_TIMEREMAINING;

/**

    The name of the Caption property for Managed Element class
 */
extern const CIMName _PROPERTY_CAPTION;

/**
    The name of the Description property for Managed Element class
 */
extern const CIMName _PROPERTY_DESCRIPTION;

/**
    The name of the ElementName property for Managed Element class
 */
extern const CIMName _PROPERTY_ELEMENTNAME;

/**
    The name of the Source Namespace property for indication filter class
 */
extern const CIMName _PROPERTY_SOURCENAMESPACE;

/**
    The name of the Source Namespaces property for indication filter class
 */
extern const CIMName _PROPERTY_SOURCENAMESPACES;

/**
    The name of the System Name property for indication filter and indications
    handler classes
 */
extern const CIMName _PROPERTY_SYSTEMNAME;

/**
    The name of the System Creation Class Name property for indication filter
    and indications handler classes
 */
extern const CIMName _PROPERTY_SYSTEMCREATIONCLASSNAME;

/**
    The name of the Other Persistence Type property for Indication Handler
    class
 */
extern const CIMName _PROPERTY_OTHERPERSISTENCETYPE;

/**
    The name of the TargetHostFormat property for SNMP Mapper Indication
    Handler subclass
 */
extern const CIMName _PROPERTY_TARGETHOSTFORMAT;

/**
    The name of the OtherTargetHostFormat property for SNMP Mapper Indication
    Handler subclass
 */
extern const CIMName _PROPERTY_OTHERTARGETHOSTFORMAT;

/**
    The name of the Alert Type property for Alert Indication class
 */
extern const CIMName _PROPERTY_ALERTTYPE;

/**
    The name of the Other Alert Type property for Alert Indication class
 */
extern const CIMName _PROPERTY_OTHERALERTTYPE;

/**
    The name of the Perceived Severity property for Alert Indication class
 */
extern const CIMName _PROPERTY_PERCEIVEDSEVERITY;

/**
    The name of the Probable Cause property for Alert Indication class
 */
extern const CIMName _PROPERTY_PROBABLECAUSE;

/**
    The name of the CreationClassName property of CIM_Service class.
*/
extern const CIMName _PROPERTY_CREATIONCLASSNAME;

/**
    The name of the Name property of CIM_Service class.
*/
extern const CIMName _PROPERTY_NAME;

/**
    The name of the Started property of CIM_Service class.
*/
extern const CIMName _PROPERTY_STARTED;

/**
    The name of the InstanceID property of CIM_Capabilities
    class.
*/
extern const CIMName _PROPERTY_INSTANCEID;

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
/**
    The name of the FilterCreationEnabled property of IndicationService class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PROPERTY_FILTERCREATIONENABLED;

/**
    The name of the SubscriptionRemovalAction property of IndicationService
    class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PROPERTY_SUBSCRIPTIONREMOVALACTION;

/**
    The name of the DeliveryRetryAttempts property of
    IndicationService class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PROPERTY_DELIVERYRETRYATTEMPTS;

/**
    The name of the DeliveryRetryInterval property of
    IndicationService class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PROPERTY_DELIVERYRETRYINTERVAL;

/**
    The name of the FilterCreationEnabledIsSettable property of
    IndicationServiceCapabilities class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName
    _PROPERTY_FILTERCREATIONENABLEDISSETTABLE;

/**
    The name of the DeliveryRetryAttemptsIsSettable property of
    IndicationServiceCapabilities class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName
    _PROPERTY_DELIVERYRETRYATTEMPTSISSETTABLE;

/**
    The name of the DeliveryRetryIntervalIsSettable property of
    IndicationServiceCapabilities class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName
    _PROPERTY_DELIVERYRETRYINTERVALISSETTABLE;

/**
    The name of the SubscriptionRemovalActionIsSettable property of
    IndicationServiceCapabilities class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName
    _PROPERTY_SUBSCRIPTIONREMOVALACTIONISSETTABLE;

/**
    The name of the SubscriptionRemovalTimeIntervalIsSettable property of
    IndicationServiceCapabilities class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName
    _PROPERTY_SUBSCRIPTIONREMOVALTIMEINTERVALISSETTABLE;

/**
    The name of the MaxListenerDestinations property of
    IndicationServiceCapabilities class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName
    _PROPERTY_MAXLISTENERDESTINATIONS;

/**
    The name of the MaxActiveSubscriptions property of
    IndicationServiceCapabilities class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PROPERTY_MAXACTIVESUBSCRIPTIONS;

/**
    The name of the SubscriptionsPersisted property of
    IndicationServiceCapabilities class.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PROPERTY_SUBSCRIPTIONSPERSISTED;

/**
    CIM_IndicationService instance default values.
*/

const Boolean _PROPERTY_FILTERCREATIONENABLED_VALUE = true;
const Uint16 _PROPERTY_SUBSCRIPTIONREMOVALACTION_VALUE = 4;
const Uint32 _PROPERTY_SUBSCRIPTIONREMOVALTIMEINTERVAL_VALUE = 2592000;
const Uint16 _PROPERTY_DELIVERYRETRYATTEMPTS_VALUE = 3;
const Uint32 _PROPERTY_DELIVERYRETRYINTERVAL_VALUE = 30;

/**
    Return codes for CIM_EnabledLogicalElement.RequestStateChange() method.
*/

const Uint32 _RETURNCODE_COMPLETEDWITHNOERROR = 0;
const Uint32 _RETURNCODE_NOTSUPPORTED = 1;
const Uint32 _RETURNCODE_TIMEOUT = 3;
const Uint32 _RETURNCODE_FAILED = 4;
const Uint32 _RETURNCODE_INVALIDPARAMETER = 5;

/**
    The name of the CIM_EnabledLogicalElement.RequestStateChange() method.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _METHOD_REQUESTSTATECHANGE;

/**
    The name of the RequestedState parameter(input) of
    CIM_EnabledLogicalElement.RequestStateChange() method.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PARAM_REQUESTEDSTATE;

/**
    The name of the TimeoutPeriod parameter(input) of
    CIM_EnabledLogicalElement.RequestStateChange() method.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PARAM_TIMEOUTPERIOD;

/**
    The name of the CIM_EnabledLogicalElement.EnabledState property.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PROPERTY_ENABLEDSTATE;

/**
    The name of the CIM_ManagedSystemElement.EnabledState property.
*/
PEGASUS_SERVER_LINKAGE extern const CIMName _PROPERTY_HEALTHSTATE;

#endif

/**
    CIM_EnabledLogicalElement.EnabledState values.
*/
const Uint16 _ENABLEDSTATE_SHUTTINGDOWN = 4;
const Uint16 _ENABLEDSTATE_STARTING = 10;
const Uint16 _ENABLEDSTATE_ENABLED = 2;
const Uint16 _ENABLEDSTATE_DISABLED = 3;
const Uint16 _ENABLEDSTATE_UNKNOWN = 0;

/**
    Service health States. See CIM_ManagedSystemElement.HealthState
*/

const Uint16 _HEALTHSTATE_OK = 5;
const Uint16 _HEALTHSTATE_DEGRADEDWARNING = 10;


//
//  Qualifier names
//

/**
    The name of the Indication qualifier for classes
 */
extern const CIMName _QUALIFIER_INDICATION;


//
//  Other literal values
//

/**
    The WHERE keyword in WQL
 */
extern const char _QUERY_WHERE[];

/**
    A zero value CIMDateTime interval
 */
extern const char _ZERO_INTERVAL_STRING[];

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_IndicationConstants_h */
