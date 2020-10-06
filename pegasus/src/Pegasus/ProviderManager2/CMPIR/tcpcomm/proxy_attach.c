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
    \file proxy_attach.c
    \brief Context storage and retrieval module for the Proxy Provider.

    This module provides functions for communication layers to properly
    handle CMPIContext objects within MI and MB calls respectively.
    Since up-calls issued by remote providers typically cannot be relayed
    back to the same thread that is executing the appropriate MI call against
    the remote provider, it is necessary to save and retrieve CMPIContext
    objects. Communication layers may do so at the beginning of an MI call
    thus obtaining a context id, which is then being used in eventually
    resulting MB calls to attach the handling thread using the previously
    saved context.

    This module, however, simply stores the pointers to the CMPIContext
    object and not its contents. Thus, it is the responsibility of the
    communication layer to ensure that a retrieved context is still active
    and has not been garbage-collected for instance.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef PEGASUS_OS_TYPE_UNIX
# ifndef PEGASUS_OS_ZOS
#  include <error.h>
# endif
#endif

#include "cmpir_common.h"
#include "proxy.h"
#include "debug.h"

PEGASUS_IMPORT extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;

#define INIT_LOCK(l) if (l==NULL) l=CMPI_BrokerExt_Ftab->newMutex(0);


//! CMPIContext storage container.
/*!
    This struct is used to to store CMPIContext objects associated with their
    ID in a linked list.
*/
struct mi_context
{
    unsigned long int id;   /*!< the ID of the stored context */
    CONST CMPIContext * ctx;    /*!< the stored CMPIContext object */
    struct mi_context * next; /*!< pointer to the next element */
};

/****************************************************************************/

static CMPI_MUTEX_TYPE __context_lock = NULL;
static unsigned long int __context_id = 0;
static struct mi_context * __contexts = NULL;

/****************************************************************************/


//! Statically saves a CMPIContext returning a unique ID.
/*!
    The function sets up a new mi_context struct and attaches it to a linked
    list.

    \param ctx the context to be saved.

    \return a unique ID to retrieve and/or delete the context from the list
    later.
*/
unsigned long int save_context ( CONST CMPIContext * ctx )
{
    struct mi_context * tmp =
        (struct mi_context *) malloc ( sizeof ( struct mi_context ) );
    unsigned long int r;

    TRACE_NORMAL(("saving context."));

    INIT_LOCK(__context_lock);
    CMPI_BrokerExt_Ftab->lockMutex(__context_lock);

    tmp->id   = ( r = __context_id++ );
    tmp->ctx  = ctx;
    tmp->next = __contexts;

    __contexts = tmp;

    CMPI_BrokerExt_Ftab->unlockMutex(__context_lock);

    TRACE_INFO(("returned id: %ld", r ));
    return r;
}


//! Retrieves a previously saved CMPIContext using its ID.
/*!
    The function looks up the given ID in the linked list to find the
    requested CMPIContext object.

    \param id used to identify the context.

    \return the proper CMPIContext object or NULL.
*/
CONST CMPIContext * get_context ( unsigned long int id )
{
    struct mi_context * tmp;

    TRACE_NORMAL(("looking up context for id: %ld", id ));

    INIT_LOCK(__context_lock);
    CMPI_BrokerExt_Ftab->lockMutex(__context_lock);

    for (tmp = __contexts; tmp != NULL; tmp = tmp->next)
    {

        if (tmp->id == id)
        {
            CONST CMPIContext * ctx = tmp->ctx;
            CMPI_BrokerExt_Ftab->unlockMutex(__context_lock);

            TRACE_INFO(("returning context."));
            return ctx;
        }
    }

    CMPI_BrokerExt_Ftab->unlockMutex(__context_lock);

    TRACE_CRITICAL(("context not found, returning NULL."));
    return NULL;
}


//! Deletes a CMPIContext data container.
/*!
    Removes the mi_struct matching the given ID from the locally maintained
    linked list.

    \param id of the context to be removed.
*/
void remove_context ( unsigned long int id )
{
    struct mi_context ** tmp;

    TRACE_NORMAL(("trying to remove context for id: %ld", id ));

    INIT_LOCK(__context_lock);
    CMPI_BrokerExt_Ftab->lockMutex(__context_lock);

    for (tmp = &__contexts; *tmp != NULL; tmp = & (*tmp)->next)
    {
        if ((*tmp)->id == id)
        {
            struct mi_context * rm = (*tmp);
            (*tmp) = rm->next;

            CMPI_BrokerExt_Ftab->unlockMutex(__context_lock);
            free ( rm );

            TRACE_INFO(("successfully removed context."));
            return;
        }
    }

    TRACE_CRITICAL(("failed to remove context."));
    CMPI_BrokerExt_Ftab->unlockMutex(__context_lock);
}

void cleanup_context ()
{

    TRACE_NORMAL(("cleaning up context facility" ));

    CMPI_BrokerExt_Ftab->destroyMutex(__context_lock);
    __context_lock = NULL;

}
/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
