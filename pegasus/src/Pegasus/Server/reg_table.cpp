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

/*
    This file contains:
        - Registration definitions for all OpenPegasus control providers
          and services (ex. indication service).
        - Implementation of the class DynamicRoutingTable that is the
          runtime access point for this information by the
          CIMOperationRequestDispatcher.

    The purpose for the DynamicRoutingTable is to define operation request
    routing for Control Providers and Services that are integrated into
    OpenPegasus.

    The DynamicRoutingTable class builds the table from information in this
    file at system startup and subsequently provides routing information
    to the dispatcher.

    For each routing it relates class and namespace definition with
    service queues and control provider names to define where the requests
    are routed from the CIMOperationRequestDispatcher.

    This table exists because Control Provider and services registration
    is managed separately from the provider registration defined through
    the registrationManager and is fixed at build time.   The
    DynamicRoutingTable is considered fixed once it is built at system startup
    There is no dynamic modification of Control Provider or services routing.o

    To change registration of Control Providers and services modify
    the tables and rebuild:
        - CtlProviderRegTable
        - SvcsRegTable
*/

#include <Pegasus/Common/MessageQueue.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Sharable.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Constants.h>
#include "reg_table.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/***********************************************************************
**
** Define entries to the DynamicRouting table for ControlProviders and
** OpenPegasus Services.
** There are two separate initialization tables because the data
** provided for routing is slightly different. The services routing has
** a queue name that may be different for different
** services while the Control Provider Routing adds a ControlProvider
** Module name and uses a single service for routing (ControlProvider)
** The two tables are:
**    - CtlProviderRegTable - One entry defining each control provider.
**         Defines the className, namespace, and providerModule name for
**         the provider.
**     -SvcsRegTable - One entry for each OpenPegasus service defined.
**         Defines the Name, namespaces, and service name for the service
**
***********************************************************************/

// define wildcard namespace routing indicator (empty CIMNamespaceName object)
static CIMNamespaceName _WILD;

/*
    CtlProviderRegDefinition table - One entry for each control provider
    module defining:
       -- class name that the provider services
       -- namespaceName for the namespace in which operations may request this
          provider (wildcard allowed)
       -- name of the control provider module to which requests are routed
          by the dispatcher.
*/
struct CtlProviderRegDefinition
{
    const CIMName className;
    const CIMNamespaceName namespaceName;
    const String providerName;
};

/*
    Defintion of OpenPegasus ControlProvider registration. This is the
    source for building the DynamicRoutingTable at initialization.
*/
static CtlProviderRegDefinition CtlProviderRegDefinitionTable[] =
{
    // Insert a record for each Control Provider.  Note that
    // QueueIDs are included for many so that this function must be called
    // after initialization builds the queueIds.
    //
    // Definition of Control provider Registration. Classname, Namespace,
    // Provider name
    //
    {PEGASUS_CLASSNAME_CONFIGSETTING,PEGASUS_NAMESPACENAME_CONFIG,
        PEGASUS_MODULENAME_CONFIGPROVIDER},

    {PEGASUS_CLASSNAME_AUTHORIZATION, PEGASUS_NAMESPACENAME_AUTHORIZATION,
        PEGASUS_MODULENAME_USERAUTHPROVIDER},

    {PEGASUS_CLASSNAME_USER, PEGASUS_NAMESPACENAME_USER,
        PEGASUS_MODULENAME_USERAUTHPROVIDER},

    {PEGASUS_CLASSNAME_SHUTDOWN, PEGASUS_NAMESPACENAME_SHUTDOWN,
        PEGASUS_MODULENAME_SHUTDOWNPROVIDER},

    {PEGASUS_CLASSNAME___NAMESPACE,_WILD, PEGASUS_MODULENAME_NAMESPACEPROVIDER},

#ifdef PEGASUS_HAS_SSL
    {PEGASUS_CLASSNAME_CERTIFICATE, PEGASUS_NAMESPACENAME_CERTIFICATE,
        PEGASUS_MODULENAME_CERTIFICATEPROVIDER},

    {PEGASUS_CLASSNAME_CRL,  PEGASUS_NAMESPACENAME_CERTIFICATE,
        PEGASUS_MODULENAME_CERTIFICATEPROVIDER},
#endif

#ifndef PEGASUS_DISABLE_PERFINST
    {PEGASUS_CLASSNAME_CIMOMSTATDATA,  PEGASUS_NAMESPACENAME_CIMOMSTATDATA,
        PEGASUS_MODULENAME_CIMOMSTATDATAPROVIDER},
#endif

#ifdef PEGASUS_ENABLE_CQL
    {PEGASUS_CLASSNAME_CIMQUERYCAPABILITIES, _WILD,
        PEGASUS_MODULENAME_CIMQUERYCAPPROVIDER},
#endif

#if defined PEGASUS_ENABLE_INTEROP_PROVIDER
    // InteropProvider ObjectManager Class
    {PEGASUS_CLASSNAME_PG_OBJECTMANAGER, PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    // CIM_Namespace - Implemented to assure that it does not
    // access the repository and to allow  access to
    // common class CIM_Namespace for namespace creation.
    {PEGASUS_CLASSNAME_CIMNAMESPACE,
        _WILD, PEGASUS_MODULENAME_INTEROPPROVIDER},

    // PG_NAMESPACE - Subclass of CIM_Namespace managed by InteropProvider.
    {PEGASUS_CLASSNAME_PGNAMESPACE, _WILD, PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_CIMXMLCOMMUNICATIONMECHANISM,
        PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_COMMMECHANISMFORMANAGER,
        PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM,
        PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_HOSTEDOBJECTMANAGER,PEGASUS_NAMESPACENAME_INTEROP,
         PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_HOSTEDACCESSPOINT, PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_NAMESPACEINMANAGER,PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE, PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE, PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE, PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE, _WILD,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE_RP_RP, _WILD,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES, _WILD,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_SUBPROFILEREQUIRESPROFILE,
        PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY, PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_ELEMENTSOFTWAREIDENTITY,
        PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_INSTALLEDSOFTWAREIDENTITY,
        PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_MODULENAME_INTEROPPROVIDER},
#endif  // PEGASUS_ENABLE_INTEROP_PROVIDER

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    {PEGASUS_CLASSNAME_PG_ELEMENTCAPABILITIES,  PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_HOSTEDINDICATIONSERVICE,PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},

    {PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT,
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_MODULENAME_INTEROPPROVIDER},
#endif

    {PEGASUS_CLASSNAME_PROVIDERMODULE, PEGASUS_NAMESPACENAME_PROVIDERREG,
        PEGASUS_MODULENAME_PROVREGPROVIDER},

    {PEGASUS_CLASSNAME_PROVIDER, PEGASUS_NAMESPACENAME_PROVIDERREG,
        PEGASUS_MODULENAME_PROVREGPROVIDER},

    {PEGASUS_CLASSNAME_PROVIDERCAPABILITIES, PEGASUS_NAMESPACENAME_PROVIDERREG,
        PEGASUS_MODULENAME_PROVREGPROVIDER},

    {PEGASUS_CLASSNAME_CONSUMERCAPABILITIES, PEGASUS_NAMESPACENAME_PROVIDERREG,
        PEGASUS_MODULENAME_PROVREGPROVIDER}
};
/*
    Definition of services names used by the SvcsRegTable below
*/
const char* indicationServiceName = PEGASUS_QUEUENAME_INDICATIONSERVICE;
const char* handlerServiceName = PEGASUS_QUEUENAME_INDHANDLERMANAGER;

/*
    Services definition table entry. One entry for each service
    defining:
        -- the className services,
        -- namespaceName allowed for requests for this className,
        -- service name for the service.
    Service names are mapped to service QueueIDs when the table is initialized
*/
struct ServiceDefinition
{
    const CIMName className;
    const CIMNamespaceName namespaceName;
    const char* serviceName;
};

/*
    Services RegistrationDefiniton Table. Defines provider registration for all
    OpenPegasus internal services.
*/
ServiceDefinition SvcsRegDefinitionTable[] =
{
    // Add listener destination queue handler, CIM_IndicationService and
    // IndicationServiceCapabilities if DMTF or SNIA profile
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    {PEGASUS_CLASSNAME_PG_LSTNRDSTQUEUE, PEGASUS_NAMESPACENAME_INTERNAL,
        handlerServiceName},

    {PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE, PEGASUS_NAMESPACENAME_INTEROP,
        indicationServiceName},

    {PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES,
        PEGASUS_NAMESPACENAME_INTEROP, indicationServiceName},
#endif
    {PEGASUS_CLASSNAME_INDSUBSCRIPTION, _WILD, indicationServiceName},
    {PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION, _WILD,indicationServiceName},
    {PEGASUS_CLASSNAME_INDHANDLER, _WILD, indicationServiceName},
    {PEGASUS_CLASSNAME_LSTNRDST_CIMXML, _WILD,  indicationServiceName},
    {PEGASUS_CLASSNAME_INDHANDLER_CIMXML,  _WILD,  indicationServiceName},
    {PEGASUS_CLASSNAME_INDHANDLER_WSMAN,  _WILD,  indicationServiceName},
    {PEGASUS_CLASSNAME_INDHANDLER_SNMP,  _WILD,  indicationServiceName},
    {PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG,  _WILD, indicationServiceName},
    {PEGASUS_CLASSNAME_LSTNRDST_EMAIL, _WILD, indicationServiceName},
    {PEGASUS_CLASSNAME_LSTNRDST_FILE, _WILD, indicationServiceName},
    {PEGASUS_CLASSNAME_INDFILTER, _WILD, indicationServiceName},

#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    {PEGASUS_CLASSNAME_PROVIDERINDDATA, PEGASUS_NAMESPACENAME_INTERNAL,
        indicationServiceName},

    {PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA, PEGASUS_NAMESPACENAME_INTERNAL,
        indicationServiceName}
#endif
};
//
/***********\ END OF CONTROL PROVIDER AND SERVICES DEFINITION TABLES **********/
//

// Pointer to the singleton instance of DynamicRoutingTable once it is created
AutoPtr<DynamicRoutingTable> DynamicRoutingTable::_this;

#ifdef PEGASUS_DEBUG
// flag to indicate DynamicRoutingTable initialized. Debug only
Boolean DynamicRoutingTable::_tableInitialized = false;
#endif

/**************************************************************************
** Class RegTableRecord - Defines a single table entryfor the
**     dynamic registration table
**************************************************************************/

RegTableRecord::RegTableRecord(
    const CIMName& className_,
    const CIMNamespaceName& namespaceName_,
    const String& providerName_,
    Uint32 serviceId_)
    : className(className_),
      namespaceName(namespaceName_),
      providerName(providerName_),
      serviceId(serviceId_)
{
}

RegTableRecord::~RegTableRecord()
{
}

/*****************************************************************************
**
**    Class DynamicRoutingTable - Defines hash based table that defines routing
**    for Control Providers and Services.
**
*****************************************************************************/
DynamicRoutingTable::DynamicRoutingTable()
{
}

DynamicRoutingTable::~DynamicRoutingTable()
{
    for (RoutingTable::Iterator it = _routingTable.start(); it ; it++)
    {
        delete it.value();
    }
}

// Get the address of the DynamicRoutingTable or Create a new table.
// Note: The table is initialized later by the buildRoutingTable function
DynamicRoutingTable*  DynamicRoutingTable::getRoutingTable()
{
    if (!_this.get())
    {
        _this.reset(new DynamicRoutingTable());
    }
    return _this.get();
}

inline String DynamicRoutingTable::_getRoutingKey(
    const CIMName& className,
    const CIMNamespaceName& namespaceName) const
{
    //ATTN: We don't support wild class names.
    PEGASUS_ASSERT(!className.isNull());
    String key(namespaceName.getString());
    key.append(Char16(':'));
    key.append(className.getString());

    return key;
}

inline String DynamicRoutingTable::_getWildRoutingKey(
    const CIMName& className) const
{
    //ATTN: We don't support wild class names.
    PEGASUS_ASSERT(!className.isNull());
    String key(":");
    key.append(className.getString());

    return key;
}

Boolean DynamicRoutingTable::getRouting(
    const CIMName& className,
    const CIMNamespaceName& namespaceName,
    String& providerName,
    Uint32 &serviceId) const
{
    // Assure that table built. If not built, startup code error and
    // probably design error in initialization
    PEGASUS_DEBUG_ASSERT(_tableInitialized);

    RegTableRecord* routing = 0;
    if (_routingTable.lookup(_getRoutingKey(className, namespaceName), routing)
        || _routingTable.lookup(_getWildRoutingKey(className), routing))
    {
        providerName= routing->providerName;
        serviceId = routing->serviceId;
        return true;
    }
    return false;
}

void DynamicRoutingTable::_insertRecord(
    const CIMName& className,
    const CIMNamespaceName& namespaceName,
    const String& providerName,
    Uint32 serviceId)
{
    RegTableRecord *rec = new RegTableRecord(
        className, namespaceName, providerName, serviceId);
    String _routingKey = _getRoutingKey(className, namespaceName);
    
    PEGASUS_FCT_EXECUTE_AND_ASSERT(true,_routingTable.insert(_routingKey, rec));
}

#ifdef PEGASUS_DEBUG
void DynamicRoutingTable::dumpRegTable()
{
    PEGASUS_STD(cout) << "******** Dynamic Routing Table ********" <<
        PEGASUS_STD(endl);

    for (RoutingTable::Iterator it = _routingTable.start(); it ; it++)
    {
        RegTableRecord *rec = it.value();
        PEGASUS_STD(cout) << "--------------------------------" <<
            PEGASUS_STD(endl)
            << "Class name : " << rec->className.getString()
            << PEGASUS_STD(endl)
            << "Namespace : " << rec->namespaceName.getString()
            << PEGASUS_STD(endl)
            << "Provider name : " << rec->providerName << PEGASUS_STD(endl)
            << "Service name : "
            << MessageQueue::lookup(rec->serviceId)->getQueueName()
            << PEGASUS_STD(endl);
    }
    PEGASUS_STD(cout) << "---------------------------------"
        << PEGASUS_STD(endl);
}
#endif

/*
    Builds the DynamicRoutingTable from Control Provider and Services
    definition tables in this file:
      - SvcsRegDefinitionTable defines OpenPegasus services to be registered
      - CtlProviderRegDefinitionTable defines Control Providers

    This is a static method.
*/
void DynamicRoutingTable::buildRoutingTable()
{
    // Lock during table build
    static AtomicInt _initialized(0);
    static Mutex _monitor;

    // Get the address of the routing table
    DynamicRoutingTable * routingTable =
        DynamicRoutingTable::getRoutingTable();

    // Assure table built only once
    if (_initialized.get() == 0)
    {
        // lock and retest
        AutoMutex autoMut(_monitor);
        if (_initialized.get() == 0)
        {
            const Uint32 NUM_SVCS = sizeof(SvcsRegDefinitionTable) /
                    sizeof(SvcsRegDefinitionTable[0]);

            // Insert a record into the table for each entry in the
            // SvcsDefinitionTable
            for (Uint32 i = 0; i < NUM_SVCS; i++)
            {
                Uint32 serviceId = MessageQueue::lookup(
                    SvcsRegDefinitionTable[i].serviceName)->getQueueId();

                routingTable->_insertRecord(
                    SvcsRegDefinitionTable[i].className,
                    SvcsRegDefinitionTable[i].namespaceName,
                    String::EMPTY,
                    serviceId);
            }

            // Add Control Providers from CtlProviderRegDefintionTable
            const Uint32 NUM_CTL_PROVIDERS =
                sizeof(CtlProviderRegDefinitionTable) /
                 sizeof(CtlProviderRegDefinitionTable[0]);

            // all Control provider routing is through controlService
            Uint32 controlServiceId = MessageQueue::lookup(
                PEGASUS_QUEUENAME_CONTROLSERVICE)->getQueueId();

            for (Uint32 i = 0; i < NUM_CTL_PROVIDERS; i++)
            {
                routingTable->_insertRecord(
                    CtlProviderRegDefinitionTable[i].className,
                    CtlProviderRegDefinitionTable[i].namespaceName,
                    CtlProviderRegDefinitionTable[i].providerName,
                    controlServiceId);
            }
            // set initialized flag
            _initialized = 1;
#ifdef PEGASUS_DEBUG
            _tableInitialized = true;
#endif
        }
    }
    //// routingTable->dumpRegTable();
}
PEGASUS_NAMESPACE_END
