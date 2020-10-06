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

#include <Pegasus/Common/CIMNameCast.h>
#include "IndicationConstants.h"


PEGASUS_NAMESPACE_BEGIN
/**
    The name of the CIMOM Shutdown alert indication class
 */
//
//  ATTN: Update once CimomShutdownAlertIndication has been defined
//
const CIMName _CLASS_CIMOM_SHUTDOWN_ALERT =
    CIMNameCast("CIM_AlertIndication");

/**
    The name of the No Provider alert indication class
 */
//
//  ATTN: Update once NoProviderAlertIndication has been defined
//
const CIMName _CLASS_NO_PROVIDER_ALERT =
    CIMNameCast("CIM_AlertIndication");

/**
    The name of the CIMOM shutdown alert indication class
 */
//
//  ATTN: Update once ProviderTerminatedAlertIndication has been defined
//
const CIMName _CLASS_PROVIDER_TERMINATED_ALERT =
    CIMNameCast("CIM_AlertIndication");


//
//  Property names
//

/**
    The name of the Other Subscription State property for Indication
    Subscription class
 */
const CIMName _PROPERTY_OTHERSTATE = CIMNameCast("OtherSubscriptionState");

/**
    The name of the Repeat Notification Policy property for indication
    subscription class
 */
const CIMName _PROPERTY_REPEATNOTIFICATIONPOLICY =
    CIMNameCast("RepeatNotificationPolicy");

/**
    The name of the Other Repeat Notification Policy property for
    indication subscription class
 */
const CIMName _PROPERTY_OTHERREPEATNOTIFICATIONPOLICY =
    CIMNameCast("OtherRepeatNotificationPolicy");

/**
    The name of the On Fatal Error Policy property for Indication Subscription
    class
 */
const CIMName _PROPERTY_ONFATALERRORPOLICY =
    CIMNameCast("OnFatalErrorPolicy");

/**
    The name of the Other On Fatal Error Policy property for Indication
    Subscription class
 */
const CIMName _PROPERTY_OTHERONFATALERRORPOLICY =
    CIMNameCast("OtherOnFatalErrorPolicy");

/**
    The name of the Time Of Last State Change property for Indication
    Subscription class
 */
const CIMName _PROPERTY_LASTCHANGE = CIMNameCast("TimeOfLastStateChange");

/**
    The name of the Subscription Start Time property for Indication
    Subscription class
 */
const CIMName _PROPERTY_STARTTIME = CIMNameCast("SubscriptionStartTime");

/**
    The name of the Subscription Duration property for Indication
    Subscription class
 */
const CIMName _PROPERTY_DURATION = CIMNameCast("SubscriptionDuration");

/**
    The name of the Subscription Time Remaining property for Indication
    Subscription class
 */
const CIMName _PROPERTY_TIMEREMAINING =
    CIMNameCast("SubscriptionTimeRemaining");

/**
    The name of the Caption property for Managed Element class
 */
const CIMName _PROPERTY_CAPTION = CIMNameCast("Caption");

/**
    The name of the Description property for Managed Element class
 */
const CIMName _PROPERTY_DESCRIPTION = CIMNameCast("Description");

/**
    The name of the ElementName property for Managed Element class
 */
const CIMName _PROPERTY_ELEMENTNAME = CIMNameCast("ElementName");

/**
    The name of the Source Namespace property for indication filter class
 */
const CIMName _PROPERTY_SOURCENAMESPACE = CIMNameCast("SourceNamespace");

/**
    The name of the Source Namespaces property for indication filter class
 */
const CIMName _PROPERTY_SOURCENAMESPACES = CIMNameCast("SourceNamespaces");

/**
    The name of the System Name property for indication filter and indications
    handler classes
 */
const CIMName _PROPERTY_SYSTEMNAME = CIMNameCast("SystemName");

/**
    The name of the System Creation Class Name property for indication filter
    and indications handler classes
 */
const CIMName _PROPERTY_SYSTEMCREATIONCLASSNAME =
    CIMNameCast("SystemCreationClassName");

/**
    The name of the Other Persistence Type property for Indication Handler
    class
 */
const CIMName _PROPERTY_OTHERPERSISTENCETYPE =
    CIMNameCast("OtherPersistenceType");

/**
    The name of the TargetHostFormat property for SNMP Mapper Indication
    Handler subclass
 */
const CIMName _PROPERTY_TARGETHOSTFORMAT = CIMNameCast("TargetHostFormat");

/**
    The name of the OtherTargetHostFormat property for SNMP Mapper Indication
    Handler subclass
 */
const CIMName _PROPERTY_OTHERTARGETHOSTFORMAT =
    CIMNameCast("OtherTargetHostFormat");

/**
    The name of the Alert Type property for Alert Indication class
 */
const CIMName _PROPERTY_ALERTTYPE = CIMNameCast("AlertType");

/**
    The name of the Other Alert Type property for Alert Indication class
 */
const CIMName _PROPERTY_OTHERALERTTYPE = CIMName("OtherAlertType");

/**
    The name of the Perceived Severity property for Alert Indication class
 */
const CIMName _PROPERTY_PERCEIVEDSEVERITY =
    CIMNameCast("PerceivedSeverity");

/**
    The name of the Probable Cause property for Alert Indication class
 */
const CIMName _PROPERTY_PROBABLECAUSE = CIMNameCast("ProbableCause");

/**
    The name of the CreationClassName property of CIM_Service class.
*/
const CIMName _PROPERTY_CREATIONCLASSNAME =
    CIMNameCast("CreationClassName");

/**
    The name of the Name property of CIM_Service class.
*/
const CIMName _PROPERTY_NAME = CIMNameCast("Name");

/**
    The name of the Started property of CIM_Service class.
*/
const CIMName _PROPERTY_STARTED = CIMNameCast("Started");

/**
    The name of the  InstanceID property of CIM_Capabilities
    class.
*/
const CIMName _PROPERTY_INSTANCEID = CIMNameCast("InstanceID");

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
/**
    The name of the FilterCreationEnabled property of IndicationService class.
*/
const CIMName _PROPERTY_FILTERCREATIONENABLED =
    CIMNameCast("FilterCreationEnabled");

/**
    The name of the SubscriptionRemovalAction property of IndicationService
    class.
*/
const CIMName _PROPERTY_SUBSCRIPTIONREMOVALACTION =
    CIMNameCast("SubscriptionRemovalAction");

/**
    The name of the DeliveryRetryAttempts property of
    IndicationService class.
*/
const CIMName _PROPERTY_DELIVERYRETRYATTEMPTS =
    CIMNameCast("DeliveryRetryAttempts");

/**
    The name of the DeliveryRetryInterval property of
    IndicationService class.
*/
const CIMName _PROPERTY_DELIVERYRETRYINTERVAL =
    CIMNameCast("DeliveryRetryInterval");
/**
    The name of the FilterCreationEnabledIsSettable property of
    IndicationServiceCapabilities class.
*/
const CIMName _PROPERTY_FILTERCREATIONENABLEDISSETTABLE =
    CIMNameCast("FilterCreationEnabledIsSettable");

/**
    The name of the DeliveryRetryAttemptsIsSettable property of
    IndicationServiceCapabilities class.
*/
const CIMName _PROPERTY_DELIVERYRETRYATTEMPTSISSETTABLE =
    CIMNameCast("DeliveryRetryAttemptsIsSettable");

/**
    The name of the DeliveryRetryIntervalIsSettable property of
    IndicationServiceCapabilities class.
*/
const CIMName _PROPERTY_DELIVERYRETRYINTERVALISSETTABLE =
    CIMNameCast("DeliveryRetryIntervalIsSettable");

/**
    The name of the SubscriptionRemovalActionIsSettable property of
    IndicationServiceCapabilities class.
*/
const CIMName _PROPERTY_SUBSCRIPTIONREMOVALACTIONISSETTABLE =
    CIMNameCast("SubscriptionRemovalActionIsSettable");

/**
    The name of the SubscriptionRemovalTimeIntervalIsSettable property of
    IndicationServiceCapabilities class.
*/
const CIMName _PROPERTY_SUBSCRIPTIONREMOVALTIMEINTERVALISSETTABLE =
    CIMNameCast("SubscriptionRemovalTimeIntervalIsSettable");

/**
    The name of the MaxListenerDestinations property of
    IndicationServiceCapabilities class.
*/
const CIMName _PROPERTY_MAXLISTENERDESTINATIONS =
    CIMNameCast("MaxListenerDestinations");

/**
    The name of the MaxActiveSubscriptions property of
    IndicationServiceCapabilities class.
*/
const CIMName _PROPERTY_MAXACTIVESUBSCRIPTIONS =
    CIMNameCast("MaxActiveSubscriptions");

/**
    The name of the SubscriptionsPersisted property of
    IndicationServiceCapabilities class.
*/
const CIMName _PROPERTY_SUBSCRIPTIONSPERSISTED =
    CIMNameCast("SubscriptionsPersisted");

/**
    The name of the CIM_EnabledLogicalElement.RequestStateChange() method.
*/
const CIMName _METHOD_REQUESTSTATECHANGE =
    CIMNameCast("RequestStateChange");

/**
    The name of the RequestedState parameter(input) of
    CIM_EnabledLogicalElement.RequestStateChange() method.
*/
const CIMName _PARAM_REQUESTEDSTATE =
    CIMNameCast("RequestedState");

/**
    The name of the TimeoutPeriod parameter(input) of
    CIM_EnabledLogicalElement.RequestStateChange() method.
*/
const CIMName _PARAM_TIMEOUTPERIOD =
    CIMNameCast("TimeoutPeriod");

/**
    The name of the CIM_EnabledLogicalElement.EnabledState property.
*/
const CIMName _PROPERTY_ENABLEDSTATE =
    CIMNameCast("EnabledState");

/**
    The name of the CIM_ManagedSystemElement.EnabledState property.
*/
const CIMName _PROPERTY_HEALTHSTATE =
    CIMNameCast("HealthState");
#endif


//
//  Qualifier names
//

/**
    The name of the Indication qualifier for classes
 */
const CIMName _QUALIFIER_INDICATION = CIMNameCast("INDICATION");


//
//  Other literal values
//

/**
    The WHERE keyword in WQL
 */
const char   _QUERY_WHERE []         = "WHERE";

/**
    A zero value CIMDateTime interval
 */
const char _ZERO_INTERVAL_STRING [] = "00000000000000.000000:000";

PEGASUS_NAMESPACE_END
