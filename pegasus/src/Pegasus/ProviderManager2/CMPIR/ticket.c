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
// Author: Frank Scheffler
//
//%/////////////////////////////////////////////////////////////////////////////

/*!
    \file ticket.c
    \brief Broker ticket management.

    This module is used to generate, verify and revoke tickets issued
    for remote providers. These tickets are needed to properly handle
    MB calls originating from remote providers, i.e. to relay them to
    the correct broker handle. Furthermore, providers that are executing
    MB calls after a ticket has been revoked, can be filtered.

    \sa proxy_provider.c
*/

#include <Pegasus/ProviderManager2/CMPI/CMPI_Version.h>
#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ticket.h"
#include "debug.h"
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Provider/CMPI/cmpift.h>

extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;

static comm_ticket * __tickets = NULL;

static CMPI_MUTEX_TYPE _tickets_lock=NULL;

#define INIT_LOCK(l) if (l==NULL) l=CMPI_BrokerExt_Ftab->newMutex(0);

/****************************************************************************/

//! Validates a ticket and returns the broker handle associated with it.
/*!
    The function checks the locally maintained list of tickets for the
    given one and returns the contained broker handle, if found.

    \param ticket the ticket to be verified.
    \return a valid broker handle or NULL.
*/
PEGASUS_EXPORT CONST CMPIBroker * verify_ticket ( const comm_ticket * ticket )
{
    comm_ticket * tmp;

    TRACE_NORMAL(("Verifying remote provider ticket."));
    TRACE_INFO(("id: %ld", ticket->id ));

    INIT_LOCK(_tickets_lock);
    CMPI_BrokerExt_Ftab->lockMutex(_tickets_lock);
    for (tmp = __tickets; tmp != NULL; tmp = tmp->next)
    {
        if (tmp->id     == ticket->id &&
            tmp->broker == ticket->broker)
        {
            CONST CMPIBroker * b = tmp->broker;
            TRACE_INFO(("ID accepted, returning broker: %p", b ));

            CMPI_BrokerExt_Ftab->unlockMutex(_tickets_lock);
            return b;
        }
    }

    CMPI_BrokerExt_Ftab->unlockMutex(_tickets_lock);

    TRACE_CRITICAL(("Invalid ticket, no broker handle found."));
    return NULL;
}


//! Generates a new ticket for the given CMPI broker handle.
/*!
    The function creates a new communication ticket and adds it to a locally
    maintained list.

    \param broker the broker handle to create the ticket for.
    \return the communication ticket.
*/
comm_ticket generate_ticket ( CONST CMPIBroker * broker )
{
    static unsigned long id = 0;

    comm_ticket * result =
        (comm_ticket *) malloc ( sizeof ( comm_ticket ) );

    TRACE_NORMAL(("generating remote provider ticket."));

    INIT_LOCK(_tickets_lock);
    CMPI_BrokerExt_Ftab->lockMutex(_tickets_lock);

    result->id     = id++;
    result->broker = broker;
    result->next   = __tickets;
    __tickets      = result;

    CMPI_BrokerExt_Ftab->unlockMutex(_tickets_lock);

    TRACE_INFO(("generated ticket with id: %ld for broker %p.",
        result->id, result->broker ));

    return *result;
}


//! Revokes a previously issued communication ticket.
/*!
    The function removes the given ticket from the locally maintained list.

    \param ticket the ticket to be removed.
    \return zero on success, -1 if the ticket cannot be found.
*/
int revoke_ticket ( comm_ticket * ticket )
{
    comm_ticket ** tmp;

    TRACE_NORMAL(("invalidating remote provider ticket."));
    INIT_LOCK(_tickets_lock);
    CMPI_BrokerExt_Ftab->lockMutex(_tickets_lock);
    for (tmp = &__tickets; *tmp != NULL; tmp = &( (*tmp)->next ))
    {
        if ((*tmp)->id == ticket->id)
        {
            comm_ticket * r = (*tmp);
            (*tmp)          = r->next;
            free ( r );

            CMPI_BrokerExt_Ftab->unlockMutex(_tickets_lock);
            return 0;
        }
    }

    CMPI_BrokerExt_Ftab->unlockMutex(_tickets_lock);
    return -1;
}


//! Compares two communication tickets.
/*!
    This function compares two communication tickets for equality.

    \param t1 first ticket.
    \param t2 second ticket.
    \return zero if t1 and t2 are equal.

    \sa remote_broker.c
 */
int compare_ticket ( const comm_ticket * t1, const comm_ticket * t2 )
{
    TRACE_NORMAL(("Comparing remote provider tickets."));
    return( t1->id == t2->id &&
        t1->broker == t2->broker );
}

void cleanup_ticket ()
{
    TRACE_NORMAL(("cleaning up ticket facility."));

    CMPI_BrokerExt_Ftab->destroyMutex(_tickets_lock);
    _tickets_lock = NULL;
}
/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
