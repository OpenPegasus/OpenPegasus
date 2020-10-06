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

/*!
    \file proxy.h
    \brief Proxy Provider header file.
*/

#ifndef _REMOTE_CMPI_PROXY_H
#define _REMOTE_CMPI_PROXY_H

typedef struct _RemoteCMPIInstanceMI    RemoteCMPIInstanceMI;
typedef struct _RemoteCMPIAssociationMI RemoteCMPIAssociationMI;
typedef struct _RemoteCMPIMethodMI      RemoteCMPIMethodMI;
typedef struct _RemoteCMPIPropertyMI    RemoteCMPIPropertyMI;
typedef struct _RemoteCMPIIndicationMI  RemoteCMPIIndicationMI;

typedef struct provider_comm            provider_comm;

#ifndef CMPI_VER_100
# define CMPI_VER_100
#endif

#ifndef CONST
# ifdef CMPI_VER_100
#  define CONST const
# else
#  define CONST
# endif
#endif

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include "ticket.h"
#include "resolver.h"


typedef provider_comm * (* INIT_COMM_LAYER) \
    ( CONST CMPIBroker *, CONST CMPIContext * );
#define REMOTE_CMPI_MI(name) \
    struct _RemoteCMPI##name##MI { \
        CMPI##name##MI ref; \
        char * provider; \
        CONST CMPIBroker * broker; \
        comm_ticket ticket; \
    };

REMOTE_CMPI_MI(Instance)
REMOTE_CMPI_MI(Association)
REMOTE_CMPI_MI(Method)
REMOTE_CMPI_MI(Property)
REMOTE_CMPI_MI(Indication)

#ifdef CMPI_VER_100
    /**
        provides access to the communication layer for MI calls
    */
    struct provider_comm
    {

        char * id;
        // instance provider function pointers
        CMPIStatus (* InstanceMI_enumInstanceNames) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath * );
        CMPIStatus (* InstanceMI_enumInstances) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char ** );
        CMPIStatus (* InstanceMI_getInstance) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char ** );
        CMPIStatus (* InstanceMI_createInstance) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const CMPIInstance * );
        CMPIStatus (* InstanceMI_setInstance) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const CMPIInstance *,
            const char ** );
        CMPIStatus (* InstanceMI_deleteInstance) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath * );
        CMPIStatus (* InstanceMI_execQuery) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char *,
            const char * );

        // associator provider function pointers
        CMPIStatus (* AssociationMI_associators) (
            provider_address *,
            RemoteCMPIAssociationMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char *,
            const char *,
            const char *,
            const char *,
            const char ** );
        CMPIStatus (* AssociationMI_associatorNames) (
            provider_address *,
            RemoteCMPIAssociationMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char *,
            const char *,
            const char *,
            const char * );
        CMPIStatus (* AssociationMI_references) (
            provider_address *,
            RemoteCMPIAssociationMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char *,
            const char *,
            const char ** );
        CMPIStatus (* AssociationMI_referenceNames) (
            provider_address *,
            RemoteCMPIAssociationMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char *,
            const char * );

        // method provider function pointers
        CMPIStatus (* MethodMI_invokeMethod) (
            provider_address *,
            RemoteCMPIMethodMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char *,
            const CMPIArgs *,
            CMPIArgs * );

        // property provider function pointers
        CMPIStatus (* PropertyMI_setProperty) (
            provider_address *,
            RemoteCMPIPropertyMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char *,
            const CMPIData );
        CMPIStatus (* PropertyMI_getProperty) (
            provider_address *,
            RemoteCMPIPropertyMI *,
            const CMPIContext *,
            const CMPIResult *,
            const CMPIObjectPath *,
            const char * );

        // indication provider function pointers
        CMPIStatus (* IndicationMI_authorizeFilter) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            const CMPIContext *,
            const CMPISelectExp *,
            const char *,
            const CMPIObjectPath *,
            const char * );
        CMPIStatus (* IndicationMI_mustPoll) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            const CMPIContext *,
            const CMPISelectExp *,
            const char *,
            const CMPIObjectPath *);
        CMPIStatus (* IndicationMI_activateFilter) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            const CMPIContext *,
            const CMPISelectExp *,
            const char *,
            const CMPIObjectPath *,
            CMPIBoolean );
        CMPIStatus (* IndicationMI_deActivateFilter) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            const CMPIContext *,
            const CMPISelectExp *,
            const char *,
            const CMPIObjectPath *,
            CMPIBoolean );
        // IBMKR: General terminate function
        CMPIStatus  (* terminate) ();

        // Adding enable and disable indications -V 5245
        CMPIStatus (* IndicationMI_enableIndications) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            const CMPIContext *);
        CMPIStatus (* IndicationMI_disableIndications) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            const CMPIContext *);


        struct provider_comm * next;
        void * handle;
    };

#else
    /**
        provides access to the communication layer for MI calls
    */
    struct provider_comm
    {

        char * id;

        // instance provider function pointers
        CMPIStatus (* InstanceMI_enumInstanceNames) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath * );
        CMPIStatus (* InstanceMI_enumInstances) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            char ** );
        CMPIStatus (* InstanceMI_getInstance) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            char ** );
        CMPIStatus (* InstanceMI_createInstance) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            CMPIInstance * );
        CMPIStatus (* InstanceMI_setInstance) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            CMPIInstance *,
            char ** );
        CMPIStatus (* InstanceMI_deleteInstance) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath * );
        CMPIStatus (* InstanceMI_execQuery) (
            provider_address *,
            RemoteCMPIInstanceMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            char *,
            char * );

        // associator provider function pointers
        CMPIStatus (* AssociationMI_associators) (
            provider_address *,
            RemoteCMPIAssociationMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            const char *,
            const char *,
            const char *,
            const char *,
            char ** );
        CMPIStatus (* AssociationMI_associatorNames) (
            provider_address *,
            RemoteCMPIAssociationMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            const char *,
            const char *,
            const char *,
            const char * );
        CMPIStatus (* AssociationMI_references) (
            provider_address *,
            RemoteCMPIAssociationMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            const char *,
            const char *,
            char ** );
        CMPIStatus (* AssociationMI_referenceNames) (
            provider_address *,
            RemoteCMPIAssociationMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            const char *,
            const char * );

        // method provider function pointers
        CMPIStatus (* MethodMI_invokeMethod) (
            provider_address *,
            RemoteCMPIMethodMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            const char *,
            CMPIArgs *,
            CMPIArgs * );

        // property provider function pointers
        CMPIStatus (* PropertyMI_setProperty) (
            provider_address *,
            RemoteCMPIPropertyMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            const char *,
            CMPIData );
        CMPIStatus (* PropertyMI_getProperty) (
            provider_address *,
            RemoteCMPIPropertyMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            const char * );

        // indication provider function pointers
        CMPIStatus (* IndicationMI_authorizeFilter) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            CMPISelectExp *,
            const char *,
            const char * );
        CMPIStatus (* IndicationMI_mustPoll) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            CMPISelectExp *,
            const char * );
        CMPIStatus (* IndicationMI_activateFilter) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            CMPISelectExp *,
            const char *,
            CMPIBoolean );
        CMPIStatus (* IndicationMI_deActivateFilter) (
            provider_address *,
            RemoteCMPIIndicationMI *,
            CMPIContext *,
            CMPIResult *,
            CMPIObjectPath *,
            CMPISelectExp *,
            const char *,
            CMPIBoolean );
        // IBMKR: General terminate function
        CMPIStatus  (* terminate) ();

        // Adding enable and disable indications -V 5245
        CMPIStatus (* IndicationMI_enableIndications) (
            provider_address *,
            RemoteCMPIIndicationMI *);
        CMPIStatus (* IndicationMI_disableIndications) (
            provider_address *,
            RemoteCMPIIndicationMI *);

        struct provider_comm * next;
        void * handle;
    };
#endif

provider_comm * load_provider_comm (
    const char * comm_id,
    CONST CMPIBroker * broker,
    CONST CMPIContext * ctx );
void unload_provider_comms ();


unsigned long int save_context ( CONST CMPIContext * ctx );
CONST CMPIContext * get_context ( unsigned long int id );
void remove_context ( unsigned long int id );
void cleanup_context ();
#endif
