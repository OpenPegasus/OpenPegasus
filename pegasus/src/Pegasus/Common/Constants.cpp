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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMNameCast.h>
#include "Constants.h"

PEGASUS_NAMESPACE_BEGIN

const CIMName PEGASUS_CLASSNAME_CONFIGSETTING        =
    CIMNameCast("PG_ConfigSetting");
const CIMName PEGASUS_CLASSNAME_AUTHORIZATION        =
    CIMNameCast("PG_Authorization");
const CIMName PEGASUS_CLASSNAME_USER                 =
    CIMNameCast("PG_User");
const CIMName PEGASUS_CLASSNAME_CERTIFICATE          =
    CIMNameCast("PG_SSLCertificate");
const CIMName PEGASUS_CLASSNAME_CRL                  =
    CIMNameCast("PG_SSLCertificateRevocationList");
const CIMName PEGASUS_CLASSNAME_PROVIDERMODULE       =
    CIMNameCast("PG_ProviderModule");
const CIMName PEGASUS_CLASSNAME_PROVIDER             =
    CIMNameCast("PG_Provider");
const CIMName PEGASUS_CLASSNAME_CAPABILITIESREGISTRATION =
    CIMNameCast("PG_CapabilitiesRegistration");
const CIMName PEGASUS_CLASSNAME_CONSUMERCAPABILITIES =
    CIMNameCast("PG_ConsumerCapabilities");
const CIMName PEGASUS_CLASSNAME_PROVIDERCAPABILITIES =
    CIMNameCast("PG_ProviderCapabilities");
const CIMName PEGASUS_CLASSNAME_INDSUBSCRIPTION      =
    CIMNameCast("CIM_IndicationSubscription");
const CIMName PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION =
    CIMNameCast("CIM_FormattedIndicationSubscription");
const CIMName PEGASUS_CLASSNAME_INDHANDLER           =
    CIMNameCast("CIM_IndicationHandler");
const CIMName PEGASUS_CLASSNAME_LSTNRDST             =
    CIMNameCast("CIM_ListenerDestination");
const CIMName PEGASUS_CLASSNAME_INDHANDLER_CIMXML    =
    CIMNameCast("CIM_IndicationHandlerCIMXML");
const CIMName PEGASUS_CLASSNAME_LSTNRDST_CIMXML      =
    CIMNameCast("CIM_ListenerDestinationCIMXML");
const CIMName PEGASUS_CLASSNAME_INDHANDLER_SNMP      =
    CIMNameCast("PG_IndicationHandlerSNMPMapper");
const CIMName PEGASUS_CLASSNAME_INDHANDLER_WSMAN =
    CIMNameCast("CIM_ListenerDestinationWSManagement");
const CIMName PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG  =
    CIMNameCast("PG_ListenerDestinationSystemLog");
const CIMName PEGASUS_CLASSNAME_LSTNRDST_FILE       =
    CIMNameCast("PG_ListenerDestinationFile");
const CIMName PEGASUS_CLASSNAME_LSTNRDST_EMAIL       =
    CIMNameCast("PG_ListenerDestinationEmail");
const CIMName PEGASUS_CLASSNAME_INDFILTER            =
    CIMNameCast("CIM_IndicationFilter");
const CIMName PEGASUS_CLASSNAME_PROVIDERMODULE_INSTALERT  =
    CIMNameCast("PG_ProviderModulesInstAlert");
const CIMName PEGASUS_CLASSNAME_SHUTDOWN             =
    CIMNameCast("PG_ShutdownService");
const CIMName PEGASUS_CLASSNAME___NAMESPACE          =
    CIMNameCast("__Namespace");

#ifndef PEGASUS_DISABLE_PERFINST
const CIMName PEGASUS_CLASSNAME_CIMOMSTATDATA        =
    CIMNameCast("CIM_CIMOMStatisticalData");
#endif

#ifdef PEGASUS_ENABLE_CQL
const CIMName PEGASUS_CLASSNAME_CIMQUERYCAPABILITIES =
    CIMNameCast("CIM_QueryCapabilities");
#endif

const CIMName PEGASUS_CLASSNAME_PROVIDERINDDATA      =
    CIMNameCast("PG_ProviderIndicationData");
const CIMName PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA  =
    CIMNameCast("PG_SubscriptionIndicationData");

// Interop Classes Accessed through Interop Control Provider

const CIMName PEGASUS_CLASSNAME_CIMNAMESPACE =
    CIMNameCast("CIM_Namespace");

#if defined PEGASUS_ENABLE_INTEROP_PROVIDER
const CIMName PEGASUS_CLASSNAME_OBJECTMANAGER =
    CIMNameCast("CIM_ObjectManager");
const CIMName PEGASUS_CLASSNAME_PGNAMESPACE =
    CIMNameCast("PG_Namespace");
const CIMName PEGASUS_CLASSNAME_OBJECTMANAGERCOMMUNICATIONMECHANISM =
    CIMNameCast("CIM_ObjectManagerCommunicationMechanism");
const CIMName PEGASUS_CLASSNAME_CIMXMLCOMMUNICATIONMECHANISM =
    CIMNameCast("CIM_CIMXMLCommunicationMechanism");
const CIMName PEGASUS_CLASSNAME_PG_CIMXMLCOMMUNICATIONMECHANISM =
    CIMNameCast("PG_CIMXMLCommunicationMechanism");
const CIMName PEGASUS_CLASSNAME_PROTOCOLADAPTER =
    CIMNameCast("CIM_ProtocolAdapter");
const CIMName PEGASUS_CLASSNAME_NAMESPACEINMANAGER =
    CIMNameCast("CIM_NamespaceInManager");
#endif

// slp Class which operates slp provider. Started by system

#ifdef PEGASUS_ENABLE_SLP
const CIMName PEGASUS_CLASSNAME_WBEMSLPTEMPLATE =
    CIMNameCast("PG_WBEMSLPTEMPLATE");

#endif

// DMTF Indications profile classes
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
const CIMName PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE =
    CIMNameCast("CIM_IndicationService");

const CIMName PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES =
    CIMNameCast("CIM_IndicationServiceCapabilities");

const CIMName PEGASUS_CLASSNAME_PG_ELEMENTCAPABILITIES =
    CIMNameCast("PG_ElementCapabilities");

const CIMName PEGASUS_CLASSNAME_PG_HOSTEDINDICATIONSERVICE =
    CIMNameCast("PG_HostedIndicationService");

const CIMName PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT =
    CIMNameCast("PG_ServiceAffectsElement");
const CIMName PEGASUS_CLASSNAME_PG_LSTNRDSTQUEUE =
    CIMNameCast("PG_ListenerDestinationQueue");
#endif

//
// Property Names
//

const CIMName PEGASUS_PROPERTYNAME_INDSUB_CREATOR =
    CIMNameCast("Creator");
const CIMName PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS =
    CIMNameCast("AcceptLanguages");
const CIMName PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS =
    CIMNameCast("ContentLanguages");
const CIMName PEGASUS_PROPERTYNAME_MODULE_USERCONTEXT =
    CIMNameCast("UserContext");
const CIMName PEGASUS_PROPERTYNAME_MODULE_DESIGNATEDUSER =
    CIMNameCast("DesignatedUserContext");
const CIMName PEGASUS_PROPERTYNAME_MODULE_MODULEGROUPNAME =
    CIMNameCast("ModuleGroupName");
const CIMName PEGASUS_PROPERTYNAME_MODULE_BITNESS =
    CIMNameCast("Bitness");

/**
    The name of the Destination property for CIM XML Indication Handler
    subclass
*/
const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION =
    CIMNameCast("Destination");


/**
    The property names for WSMAN Indication Handler
*/
// Delivery Mode
const CIMName PEGASUS_PROPERTYNAME_WSM_DELIVERY_MODE =
    CIMNameCast("DeliveryMode");

const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_FILE =
    CIMNameCast("File");

/**
    The name of the CreationTime property for PG_ListenerDestinationQueue.
*/

const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_CREATIONTIME =
    CIMNameCast("CreationTime");

/**
    The name of the TargetHost property for SNMP Mapper Indication
    Handler subclass
*/
const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST =
    CIMNameCast("TargetHost");

/**
    The name of the TextFormat property for Formatted Indication
    Subscription class
*/
const CIMName _PROPERTY_TEXTFORMAT = CIMNameCast("TextFormat");

/**
    The name of the TextFormatParameters property for Formatted
    Indication Subscription class
*/
const CIMName _PROPERTY_TEXTFORMATPARAMETERS =
    CIMNameCast("TextFormatParameters");

/**
    The name of the MailTo property for Email Handler subclass
*/
const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO =
    CIMNameCast("MailTo");

/**
    The name of the MailSubject property for Email Handler subclass
*/
const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_MAILSUBJECT =
    CIMNameCast("MailSubject");

/**
    The name of the MailCc  property for Email Handler subclass
*/
const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_MAILCC =
    CIMNameCast("MailCc");

/**
    The name of the Name property for PG_ProviderModule class
*/
const CIMName _PROPERTY_PROVIDERMODULE_NAME =
    CIMNameCast("Name");

/**
    The name of the operational status property
*/
const CIMName _PROPERTY_OPERATIONALSTATUS =
    CIMNameCast("OperationalStatus");

/**
    The name of the Filter reference property for indication subscription class
 */
const CIMName PEGASUS_PROPERTYNAME_FILTER =
    CIMNameCast("Filter");

/**
   The name of the Handler reference property for indication subscription class
 */
const CIMName PEGASUS_PROPERTYNAME_HANDLER =
    CIMNameCast("Handler");

/**
    The name of the Subscription State property for indication subscription
    class
 */
const CIMName PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE =
    CIMNameCast("SubscriptionState");

/**
    The name of the SubscriptionInfo property for Formatted Indication
    Subscription class
*/
const CIMName _PROPERTY_SUBSCRIPTION_INFO = CIMNameCast("SubscriptionInfo");

/**
    The name of the Query property for indication filter class
 */
const CIMName PEGASUS_PROPERTYNAME_QUERY =
    CIMNameCast("Query");

/**
    The name of the Query Language property for indication filter class
 */
const CIMName PEGASUS_PROPERTYNAME_QUERYLANGUAGE =
    CIMNameCast("QueryLanguage");

/**
    The name of the Name property
 */
const CIMName PEGASUS_PROPERTYNAME_NAME =
    CIMNameCast("Name");

/**
    The name of the Creation Class Name property for indication filter and
    indications handler classes
 */
const CIMName PEGASUS_PROPERTYNAME_CREATIONCLASSNAME =
    CIMNameCast("CreationClassName");

/**
    The name of the Persistence Type property for Indication Handler class
 */
const CIMName PEGASUS_PROPERTYNAME_PERSISTENCETYPE =
    CIMNameCast("PersistenceType");

/**
    The name of the SubscriptionRemovalTimeInterval property of
    IndicationService class.
*/
const CIMName _PROPERTY_SUBSCRIPTIONREMOVALTIMEINTERVAL =
    CIMNameCast("SubscriptionRemovalTimeInterval");

/**
    The name of the SNMP Version property for SNMP Mapper Indication Handler
    subclass
 */
const CIMName PEGASUS_PROPERTYNAME_SNMPVERSION =
    CIMNameCast("SNMPVersion");

//
// CIM Namespace Names
//

#if defined NS_ROOT_INTEROP
const CIMNamespaceName PEGASUS_NAMESPACENAME_INTEROP=
    CIMNamespaceName ("root/interop");   
#elif defined NS_INTEROP
const CIMNamespaceName PEGASUS_NAMESPACENAME_INTEROP=
    CIMNamespaceName ("interop");
#else
const CIMNamespaceName PEGASUS_NAMESPACENAME_INTEROP=
    CIMNamespaceName ("root/PG_InterOp");
#endif

const CIMNamespaceName PEGASUS_NAMESPACENAME_INTERNAL =
    CIMNamespaceName ("root/PG_Internal");
const CIMNamespaceName PEGASUS_NAMESPACENAME_CIMV2    =
    CIMNamespaceName ("root/cimv2");

const CIMNamespaceName PEGASUS_NAMESPACENAME_AUTHORIZATION =
    PEGASUS_NAMESPACENAME_INTERNAL;
const CIMNamespaceName PEGASUS_NAMESPACENAME_CONFIG        =
    PEGASUS_NAMESPACENAME_INTERNAL;
const CIMNamespaceName PEGASUS_NAMESPACENAME_PROVIDERREG   =
    PEGASUS_NAMESPACENAME_INTEROP;
const CIMNamespaceName PEGASUS_NAMESPACENAME_SHUTDOWN      =
    PEGASUS_NAMESPACENAME_INTERNAL;
const CIMNamespaceName PEGASUS_NAMESPACENAME_USER          =
    PEGASUS_NAMESPACENAME_INTERNAL;
const CIMNamespaceName PEGASUS_NAMESPACENAME_CERTIFICATE   =
    PEGASUS_NAMESPACENAME_INTERNAL;

#ifndef PEGASUS_DISABLE_PERFINST
const CIMNamespaceName PEGASUS_NAMESPACENAME_CIMOMSTATDATA =
    PEGASUS_NAMESPACENAME_CIMV2;
#endif

#ifdef PEGASUS_ENABLE_CQL
const CIMNamespaceName PEGASUS_NAMESPACENAME_CIMQUERYCAPABILITIES  =
    PEGASUS_NAMESPACENAME_CIMV2;
#endif

const CIMNamespaceName PEGASUS_VIRTUAL_TOPLEVEL_NAMESPACE =
    CIMNamespaceName("PG_Reserved");

//
// Server Profile-related class names
//
const CIMName PEGASUS_CLASSNAME_PG_OBJECTMANAGER =
    CIMNameCast("PG_ObjectManager");
const CIMName PEGASUS_CLASSNAME_PG_COMMMECHANISMFORMANAGER =
    CIMNameCast("PG_CommMechanismForManager");
const CIMName PEGASUS_CLASSNAME_PG_NAMESPACEINMANAGER =
    CIMNameCast("PG_NamespaceInManager");
const CIMName PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE =
    CIMNameCast("PG_RegisteredProfile");
const CIMName PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE =
    CIMNameCast("PG_RegisteredSubProfile");
const CIMName PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE =
    CIMNameCast("PG_ReferencedProfile");
const CIMName PEGASUS_CLASSNAME_CIM_ELEMENTCONFORMSTOPROFILE =
    CIMNameCast("CIM_ElementConformsToProfile");
const CIMName PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE =
    CIMNameCast("PG_ElementConformsToProfile");
const CIMName PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE_RP_RP =
    CIMNameCast("PG_ElementConformsToProfile_RP_RP");
const CIMName PEGASUS_CLASSNAME_PG_SUBPROFILEREQUIRESPROFILE =
    CIMNameCast("PG_SubProfileRequiresProfile");
const CIMName PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY =
    CIMNameCast("PG_SoftwareIdentity");
const CIMName PEGASUS_CLASSNAME_PG_ELEMENTSOFTWAREIDENTITY =
    CIMNameCast("PG_ElementSoftwareIdentity");
const CIMName PEGASUS_CLASSNAME_PG_INSTALLEDSOFTWAREIDENTITY =
    CIMNameCast("PG_InstalledSoftwareIdentity");
const CIMName PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM =
    CIMNameCast("PG_ComputerSystem");
const CIMName PEGASUS_CLASSNAME_PG_HOSTEDOBJECTMANAGER =
    CIMNameCast("PG_HostedObjectManager");
const CIMName PEGASUS_CLASSNAME_PG_HOSTEDACCESSPOINT =
    CIMNameCast("PG_HostedAccessPoint");

// Registration classes
const CIMName PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES =
    CIMNameCast("PG_ProviderProfileCapabilities");
const CIMName PEGASUS_CLASSNAME_PG_PROVIDERREFERENCEDPROFILES =
    CIMNameCast("PG_ProviderReferencedProfiles");

/**
    The qualifier name of embedded objects/instances.
*/
const CIMName PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT =
    CIMNameCast("EmbeddedObject");
const CIMName PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE =
    CIMNameCast("EmbeddedInstance");


PEGASUS_NAMESPACE_END
