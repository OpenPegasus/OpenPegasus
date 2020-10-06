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
    \file remote.h
    \brief Remote Broker header file.
*/

#ifndef _REMOTE_CMPI_REMOTE_H
#define _REMOTE_CMPI_REMOTE_H

#include <time.h>
#include "cmpir_common.h"

#define CMPIBroker2remote_broker(b) ((remote_broker *) (b))
#define remote_broker2CMPIBroker(rb) ((CMPIBroker *) (rb))
#define RBGetBrokerAddress(rb) \
    (rb)->ft->get_broker_address((rb))
#define RBGetProvider(rb) \
    (rb)->ft->get_provider((rb))
#define RBGetTicket(rb) \
    (rb)->ft->get_ticket((rb))
#define RBAcquireMI(rb) \
    (rb)->ft->acquireMI((rb))
#define RBReleaseMI(rb) \
    (rb)->ft->releaseMI((rb))
#define RBGetUseCount(rb) \
    (rb)->ft->get_use_count((rb))
#define RBGetInstanceMI(rb) \
    (rb)->ft->get_instanceMI((rb))
#define RBGetAssociationMI(rb) \
    (rb)->ft->get_associationMI((rb))
#define RBGetMethodMI(rb) \
    (rb)->ft->get_methodMI((rb))
#define RBGetPropertyMI(rb) \
    (rb)->ft->get_propertyMI((rb))
#define RBGetIndicationMI(rb) \
    (rb)->ft->get_indicationMI((rb))

#define PEGASUS_CMPIR_DAEMON_STOP "04PEGASUS_CMPIR_DAEMON_STOP"
#define PEGASUS_CMPIR_DAEMON_IS_RUNNING "04PEGASUS_CMPIR_DAEMON_IS_RUNNING"

typedef struct remote_broker remote_broker;


#ifndef CMPI_VER_100
# define CMPI_VER_100
#endif

#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include "ticket.h"

//! Remote broker function table.
/*!
    This struct defines function pointers to access remote broker functionality,
    as done by remote communication layers.
*/
struct  remote_brokerFT
{
    char * (* get_broker_address) ( remote_broker * );
    char * (* get_provider) ( remote_broker * );
    comm_ticket * (* get_ticket) ( remote_broker * );

    void (* acquireMI) ( remote_broker * );
    void (* releaseMI) ( remote_broker * );
    unsigned int (* get_use_count) ( remote_broker * );

    CMPIInstanceMI * (* get_instanceMI) ( remote_broker * );
    CMPIAssociationMI * (* get_associationMI) ( remote_broker * );
    CMPIMethodMI * (* get_methodMI) ( remote_broker * );
    CMPIPropertyMI * (* get_propertyMI) ( remote_broker * );
    CMPIIndicationMI * (* get_indicationMI) ( remote_broker * );
};


//! Remote broker structure.
/*!
    This structure extends a regular CMPIBroker struct adding a
    remote_brokerFT pointer to it.
*/
struct  remote_broker
{
    CMPIBroker broker;  /*!< original broker to be used by remote
                  providers */
    struct remote_brokerFT * ft; /*!< function table pointer */
};


/****************************************************************************/

PEGASUS_EXPORT remote_broker * PEGASUS_CMPIR_CDECL find_remote_broker (
    const char * comm_layer_id,
    const char * broker_address,
    const char * provider,
    const char * provider_module,
    const comm_ticket * ticket,
    CMPIBrokerFT * brokerFT );

PEGASUS_EXPORT void PEGASUS_CMPIR_CDECL cleanup_remote_brokers (
    long timeout,
    time_t check_interval );

PEGASUS_EXPORT void PEGASUS_CMPIR_CDECL init_activation_context (
    CMPIContext * ctx );


#endif

/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
