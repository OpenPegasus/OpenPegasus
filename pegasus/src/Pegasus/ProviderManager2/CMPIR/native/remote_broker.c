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
    \file remote_broker.c
    \brief Common Remote Broker functionality.

    This file contains common functionality to be used by communication
    layers on the remote side, i.e. at the actual provider location.
    It provides the look-up of providers, their activation and deactivation
    respectively, and filter tracking for indication providers.

    Remote providers are each attached to a unique remote broker handle,
    which is being identified by the following criteria:
    - the communication layer identifier
    - the broker response address used for broker service requests
    - the provider name (as used to find fixed entry points)
    - the provider module/library containing the provider
    - the communication ticket issued by the remote proxy

    If a handle cannot be found for these items, a new one is being created
    and added to a static linked list, thus providers can be reidentified
    for future MI requests. The activation of the underlying provider, based
    on the type, is done dynamically, whenever it is requested by the
    communication layer. Activated providers are kept alive for a certain
    period of time after their last usage (i.e. when their use-count is
    decreased to zero), before they get unloaded together with the MI
    library. Deactivation can be prevented by communication layers using
    the acquire() call, and implicitly by activateFilter() calls issued
    against indication providers.

*/

#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined PEGASUS_OS_TYPE_WINDOWS
# include <winsock2.h>
#else
# include <sys/time.h>
# include <dlfcn.h>
# include <pthread.h>    /*timespec structure define in z/OS */
#endif

#include "tool.h"
#include "mm.h"
#include "native.h"
#include "remote.h"
#include "ticket.h"
#include "debug.h"

extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;
#define INIT_LOCK(l) if (l==NULL) l=CMPI_BrokerExt_Ftab->newMutex(0);

//! CMPIIndicationMI extension to track filter de-/activations.
/*!
    This structure extends a regular CMPIIndicationMI and holds a reference
    to the provider's original MI. Furthermore, it has a reference to the
    remote broker contained within. This reference is used to modify its
    use_count to keep track of active filters, and to avoid preliminary
    deactivation. In other words, activating an indication filter is equal
    to acquiring the MI, and deactivating is equal to releasing it respectively.

    \sa __track_indicationMI()
    \sa remote_broker
*/
struct tracked_indication
{
    CMPIIndicationMI mi;           /*!< indication MI to be extended  */
    CMPIIndicationMI * saved_mi;   /*!< pointer to the provider's MI */
    remote_broker * rb;            /*!< reference to the remote broker  */
};


//! Data container for remote brokers.
/*!
    This remote broker structure is the counterpart of remote providers; they
    are one-to-one related. Therefore, it holds all the necessary information
    to identify and access the latter one. Once created, this handle is put
    into a linked list and is never removed. However, the MI module, i.e. the
    provider, associated with this handle may be deactivated and activated
    on a timely basis, depending on its usage.
*/
struct __remote_broker
{
    remote_broker rb;   /*!< holds the actual broker and FTs  */

    char * comm_layer_id;   /*!< the comm-layer associated with the MI */
    char * broker_address;  /*!< the broker address used for up-calls */
    char * provider;    /*!< the name of the associated provider */
    char * provider_module; /*!< the name of the MI library/module */
    comm_ticket ticket; /*!< the comm. ticket used for up-calls */
    void * library;     /*!< the library handle */

    unsigned int use_count;            /*!< number of comm-layer and
                         filters currently using this
                         provider */
    struct timeval last_used;          /*!< time of last usage  */
    CMPI_MUTEX_TYPE lock;              /*!< lock to provide clean access
                     to use_count  */

    CMPIInstanceMI * instanceMI;       /*!< instance MI handle */
    CMPIAssociationMI * associationMI; /*!< association MI handle */
    CMPIMethodMI * methodMI;           /*!< method MI handle */
    CMPIPropertyMI * propertyMI;       /*!< property MI handle */
    CMPIIndicationMI * indicationMI;   /*!< indication MI handle */

    struct __remote_broker * next;     /*!< pointer to next element  */
};



/****************************************************************************/

//! List of all remote_broker handles currently maintained.
static struct __remote_broker * __remote_brokers   = NULL;

//! Lock to assure exclusive access to __remote_brokers.
static CMPI_MUTEX_TYPE __remote_brokers_lock = NULL;

//! CMPIContext used for activation and deactivation of providers.
/*!
    This context is passed to providers upon their activation and
    deactivation respectively. It has to be separately initialized.

    \sa init_activation_context()
*/
static CMPIContext * __remote_brokers_context = NULL;


/****************************************************************************/
/*          indication tracking functions                                   */
/****************************************************************************/


//! Cleans up a tracked indication MI.
/*!
    The function calls the provider's cleanup() method, wrapped within the
    CMPIIndicationMI, and then frees the tracked_indication structure
    mapping to it.

    \return the status of the provider's cleanup() call.
*/
#ifdef CMPI_VER_100

static CMPIStatus __indication_cleanup (
    CMPIIndicationMI * mi,
    CONST CMPIContext * ctx,
    CMPIBoolean term)
#else
static CMPIStatus __indication_cleanup (
    CMPIIndicationMI * mi,
    CMPIContext * ctx)
#endif
{
    struct tracked_indication * __mi =
        (struct tracked_indication *) mi;
    CMPIStatus rc;

    TRACE_INFO(("relaying call to real provider."));
#ifdef CMPI_VER_100
    rc = __mi->saved_mi->ft->cleanup ( __mi->saved_mi, ctx, term);
#else
    rc = __mi->saved_mi->ft->cleanup ( __mi->saved_mi, ctx );
#endif

    TRACE_INFO(("freeing wrapper CMPIIndicationMIFT."));
    free ( __mi );
    return rc;
}


//! Relays the request to the actual provider.
/*!
    This request is simply forwarded to the MI saved within the
    tracked_indication struct passed in as CMPIIndicationMI.

    \return the status of the provider's authorizeFilter() call.
*/
static CMPIStatus __indication_authorizeFilter (
    CMPIIndicationMI * mi,
    CONST CMPIContext * ctx,
#ifndef CMPI_VER_100
    CMPIResult * result,
#endif
    CONST CMPISelectExp * sexp,
    const char * ns,
    CONST CMPIObjectPath * cop,
    const char * user )
{
    struct tracked_indication * __mi =
        (struct tracked_indication *) mi;

    TRACE_INFO(("relaying call to real provider."));
    return __mi->saved_mi->ft->authorizeFilter (
        __mi->saved_mi, ctx,
#ifndef CMPI_VER_100
        result,
#endif
        sexp,
        ns,
        cop,
        user );


}


//! Relays the request to the actual provider.
/*!
    This request is simply forwarded to the MI saved within the
    tracked_indication struct passed in as CMPIIndicationMI.

    \return the status of the provider's mustPoll() call.
*/
static CMPIStatus __indication_mustPoll (
    CMPIIndicationMI * mi,
    CONST CMPIContext * ctx,
#ifndef CMPI_VER_100
    CMPIResult * result,
#endif
    CONST CMPISelectExp * sexp,
    const char * ns,
    CONST CMPIObjectPath * cop )
{
    struct tracked_indication * __mi =
        (struct tracked_indication *) mi;

    TRACE_INFO(("relaying call to real provider."));
    return __mi->saved_mi->ft->mustPoll (
        __mi->saved_mi,
        ctx,
#ifndef CMPI_VER_100
        result,
#endif
        sexp,
        ns,
        cop );
}



//! Relays the request to the actual provider and increases the use count.
/*!
    This request is simply forwarded to the MI saved within the
    tracked_indication struct passed in as CMPIIndicationMI. Furthermore,
    the use count for the associated remote broker is increased by one, using
    the acquireMI() call from its function table.

    \return the status of the provider's activateFilter() call.
*/
static CMPIStatus __indication_activateFilter (
    CMPIIndicationMI * mi,
    CONST CMPIContext * ctx,
#ifndef CMPI_VER_100
    CMPIResult * result,
#endif
    CONST CMPISelectExp * sexp,
    const char * clsName,
    CONST CMPIObjectPath * cop,
    CMPIBoolean first )
{
    struct tracked_indication * __mi =
        (struct tracked_indication *) mi;

    TRACE_VERBOSE(("entered function."));

    RBAcquireMI ( __mi->rb );

    TRACE_INFO(("relaying call to real provider."));
    return __mi->saved_mi->ft->activateFilter (
        __mi->saved_mi,
        ctx,
#ifndef CMPI_VER_100
        result,
#endif
        sexp,
        clsName,
        cop,
        first );
}


//! Relays the request to the actual provider and decreases the use count.
/*!
    This request is simply forwarded to the MI saved within the
    tracked_indication struct passed in as CMPIIndicationMI. Furthermore,
    the use count for the associated remote broker is decreased by one, using
    the releaseMI() call from its function table.

    \return the status of the provider's deActivateFilter() call.
*/
static CMPIStatus __indication_deactivateFilter (
    CMPIIndicationMI * mi,
    CONST CMPIContext * ctx,
#ifndef CMPI_VER_100
    CMPIResult * result,
#endif
    CONST CMPISelectExp * sexp,
    const char * clsName,
    CONST CMPIObjectPath * cop,
    CMPIBoolean last )
{
    struct tracked_indication * __mi =
        (struct tracked_indication *) mi;

    TRACE_VERBOSE(("entered function."));

    RBReleaseMI ( __mi->rb );

    TRACE_INFO(("relaying call to real provider."));
    return __mi->saved_mi->ft->deActivateFilter (
        __mi->saved_mi,
        ctx,
#ifndef CMPI_VER_100
        result,
#endif
        sexp,
        clsName,
        cop,
        last );
}

static CMPIStatus __indication_enableIndications (
    CMPIIndicationMI* mi
#ifdef CMPI_VER_100
    , const CMPIContext *ctx
#endif
    )
{
    struct tracked_indication * __mi = (struct tracked_indication *) mi;

    TRACE_VERBOSE(("entered function."));

    TRACE_INFO(("relaying call to real provider."));
    return __mi->saved_mi->ft->enableIndications (
        __mi->saved_mi
#ifdef CMPI_VER_100
        ,ctx
#endif
        );
}

static CMPIStatus __indication_disableIndications (
    CMPIIndicationMI* mi
#ifdef CMPI_VER_100
    , const CMPIContext *ctx
#endif
    )
{
    struct tracked_indication * __mi = (struct tracked_indication *) mi;

    TRACE_VERBOSE(("entered function."));

    TRACE_INFO(("relaying call to real provider."));
    return __mi->saved_mi->ft->disableIndications (
        __mi->saved_mi
#ifdef CMPI_VER_100
        ,ctx
#endif
        );

}


//! Builds a wrapper tracked_indication struct from the given CMPIIndicationMI.
/*!
    The function allocates a wrapper CMPIIndicationMI intercepting MI requests
    from the communication layer. Thus, de-/activateFilter() calls can be
    tracked and use counts be modified properly, preventing indication
    providers from being unloaded in advance.

    \param mi the original CMPIIndicationMI.
    \param rb the remote broker to be linked with the MI.

    \return the wrapper CMPIIndicationMI.

    \sa __indication_activateFilter, __indication_deactivateFilter
*/
static CMPIIndicationMI * __track_indicationMI (
    CMPIIndicationMI * mi,
    remote_broker * rb)
{
    static CMPIIndicationMIFT __indicationMIFT = {
        0, 0, NULL,
        __indication_cleanup,
        __indication_authorizeFilter,
        __indication_mustPoll,
        __indication_activateFilter,
        __indication_deactivateFilter,
        __indication_enableIndications,
        __indication_disableIndications,
    };

    struct tracked_indication * __mi =
        (struct tracked_indication *)
    malloc ( sizeof ( struct tracked_indication ) );

    TRACE_NORMAL(("building wrapper CMPIIndicationMIFT."));

    __mi->mi.ft    = &__indicationMIFT;
    __mi->saved_mi =  mi;
    __mi->rb     = rb;

    return(CMPIIndicationMI *) __mi;
}


/*****************************************************************************/
/*            encapsulated remote broker functions                           */
/*****************************************************************************/

static char * __get_broker_address ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("returning broker address: %s", __rb->broker_address));
    return __rb->broker_address;
}


static char * __get_provider ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("returning provider name: %s", __rb->provider));
    return __rb->provider;
}


static comm_ticket * __get_ticket ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("returning broker ticket."));
    return &__rb->ticket;
}


static void __acquireMI ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("Acquiring remote broker handle."));

    CMPI_BrokerExt_Ftab->lockMutex(__rb->lock);

    __rb->use_count++;
    TRACE_INFO(("use count increased to: %d", __rb->use_count ));

    CMPI_BrokerExt_Ftab->unlockMutex(__rb->lock);
}


static void __releaseMI ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("releasing remote broker handle."));

    CMPI_BrokerExt_Ftab->lockMutex(__rb->lock);

    gettimeofday ( &__rb->last_used, NULL );
    __rb->use_count--;
    TRACE_INFO(("use count decreased to: %d", __rb->use_count ));

    CMPI_BrokerExt_Ftab->unlockMutex(__rb->lock);
}


static unsigned int __get_use_count ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    return __rb->use_count;
}


static void __cleanup_remote_broker ( struct __remote_broker * __rb )
{

#ifdef CMPI_VER_100
    CMPIBoolean term = 0;
#endif
    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("Deactivating remote provider."));
    if (__rb->instanceMI != NULL)
    {
        TRACE_INFO(("Calling cleanup() on instanceMI handle."));
        __rb->instanceMI->ft->cleanup (
            __rb->instanceMI,
            __remote_brokers_context
#ifdef CMPI_VER_100
            ,term
#endif
            );
    }

    if (__rb->associationMI != NULL)
    {
        TRACE_INFO(("Calling cleanup() on associationMI handle."));
        __rb->associationMI->ft->cleanup (
            __rb->associationMI,
            __remote_brokers_context
#ifdef CMPI_VER_100
            ,term
#endif
            );
    }

    if (__rb->methodMI != NULL)
    {
        TRACE_INFO(("Calling cleanup() on methodMI handle."));
        __rb->methodMI->ft->cleanup (
            __rb->methodMI,
            __remote_brokers_context
#ifdef CMPI_VER_100
            ,term
#endif
            );
    }

    if (__rb->propertyMI != NULL)
    {
        TRACE_INFO(("Calling cleanup() on propertyMI handle."));
        __rb->propertyMI->ft->cleanup (
            __rb->propertyMI,
            __remote_brokers_context
#ifdef CMPI_VER_100
            ,term
#endif
            );
    }

    if (__rb->indicationMI != NULL)
    {
        TRACE_INFO(("Calling cleanup() on indicationMI handle."));
        __rb->indicationMI->ft->cleanup (
            __rb->indicationMI,
            __remote_brokers_context
#ifdef CMPI_VER_100
            ,term
#endif
            );
    }
    tool_mm_flush ();


    TRACE_INFO(("Closing provider module library."));
    //invokes dlfree call on unix and FreeLibrary on windows
    PEGASUS_CMPIR_FREELIBRARY ( __rb->library );

    __rb->library = NULL;

    TRACE_INFO(("Freeing allocated memory."));
    free ( __rb->comm_layer_id );
    free ( __rb->broker_address );
    free ( __rb->provider );
    free ( __rb->provider_module );
    CMPI_BrokerExt_Ftab->destroyMutex(__rb->lock);
    free ( __rb );

    TRACE_VERBOSE(("leaving function."));
}


static CMPIInstanceMI * __get_instanceMI ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("Retrieving instanceMI handle."));

    if (__rb->instanceMI == NULL)
    {
        __rb->instanceMI =
            tool_load_InstanceMI (
            __rb->provider,
            __rb->library,
            (CMPIBroker *) rb,
            __remote_brokers_context );
    }
    return __rb->instanceMI;
}


static CMPIAssociationMI * __get_associationMI ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("Retrieving associationMI handle."));

    if (__rb->associationMI == NULL)
    {
        __rb->associationMI =
            tool_load_AssociationMI (
            __rb->provider,
            __rb->library,
            (CMPIBroker *) rb,
            __remote_brokers_context );
    }
    return __rb->associationMI;
}


static CMPIMethodMI * __get_methodMI ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("Retrieving methodMI handle."));

    if (__rb->methodMI == NULL)
    {
        __rb->methodMI =
            tool_load_MethodMI (
            __rb->provider,
            __rb->library,
            (CMPIBroker *) rb,
            __remote_brokers_context );
    }
    return __rb->methodMI;
}


static CMPIPropertyMI * __get_propertyMI ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;

    TRACE_NORMAL(("Retrieving propertyMI handle."));

    if (__rb->propertyMI == NULL)
    {
        __rb->propertyMI =
            tool_load_PropertyMI (
            __rb->provider,
            __rb->library,
            (CMPIBroker *) rb,
            __remote_brokers_context );
    }
    return __rb->propertyMI;
}


static CMPIIndicationMI * __get_indicationMI ( remote_broker * rb )
{
    struct __remote_broker * __rb =
        (struct __remote_broker *) rb;
    CMPIIndicationMI * mi;

    TRACE_NORMAL(("Retrieving indicationMI handle."));

    if (__rb->indicationMI == NULL &&
        ( mi = tool_load_IndicationMI (
        __rb->provider,
        __rb->library,
        (CMPIBroker *) rb,
        __remote_brokers_context ) )
        != NULL)
    {

        __rb->indicationMI =
            __track_indicationMI ( mi, (remote_broker *) __rb );
    }
    return __rb->indicationMI;
}


static struct __remote_broker * __new_remote_broker (
    const char * comm_layer_id,
    const char * broker_address,
    const char * provider,
    const char * provider_module,
    const comm_ticket * ticket,
    CMPIBrokerFT * brokerFT )
{
    static struct remote_brokerFT rbFT = {
        __get_broker_address,
        __get_provider,
        __get_ticket,
        __acquireMI,
        __releaseMI,
        __get_use_count,
        __get_instanceMI,
        __get_associationMI,
        __get_methodMI,
        __get_propertyMI,
        __get_indicationMI
    };

    struct __remote_broker * __rb;

    TRACE_VERBOSE(("entered function."));

    __rb = (struct __remote_broker *)
    calloc ( 1, sizeof ( struct __remote_broker ) );
    __rb->lock=CMPI_BrokerExt_Ftab->newMutex(0);

    TRACE_NORMAL(("Setting up new remote broker structure."));
    TRACE_INFO(("Storing:\ncomm_layer_id: %s\nbroker_address: %s\n"
        "provider: %s\nprovider_module: %s",
        comm_layer_id, broker_address, provider, provider_module));

    __rb->rb.ft         = &rbFT;
    __rb->rb.broker.bft = brokerFT;
    __rb->rb.broker.eft = &native_brokerEncFT;
    __rb->rb.broker.xft = CMPI_BrokerExt_Ftab;

    __rb->comm_layer_id =
        ( comm_layer_id )? strdup ( comm_layer_id ): NULL;
    __rb->broker_address =
        ( broker_address )? strdup ( broker_address ): NULL;
    __rb->provider = ( provider )? strdup ( provider ): NULL;
    __rb->provider_module =
        ( provider_module )? strdup ( provider_module ): NULL;
    __rb->ticket = *ticket;

    gettimeofday ( &__rb->last_used, NULL );

    {
        char dlName[512];

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
        strcpy(dlName,provider_module);
        strcat(dlName,".dll");
#elif defined (PEGASUS_PLATFORM_HPUX_PARISC_ACC)
        strcpy(dlName,"lib");
        strcat(dlName,provider_module);
        strcat(dlName,".sl");
#elif defined(PEGASUS_OS_DARWIN)
        strcpy(dlName,"lib");
        strcat(dlName,provider_module);
        strcat(dlName,".dylib");
#else
        strcpy(dlName,"lib");
        strcat(dlName,provider_module);
        strcat(dlName,".so");
#endif

        TRACE_INFO(("loading provider module: %s.", provider_module ));
        //invoke dlopen under unix and LoadLibrary under windows OS.
        __rb->library = PEGASUS_CMPIR_LOADLIBRARY ( dlName, RTLD_NOW );
    }

    TRACE_VERBOSE(("leaving function."));
    return __rb;
}


PEGASUS_EXPORT remote_broker * PEGASUS_CMPIR_CDECL find_remote_broker (
    const char * comm_layer_id,
    const char * broker_address,
    const char * provider,
    const char * provider_module,
    const comm_ticket * ticket,
    CMPIBrokerFT * brokerFT )
{
    struct __remote_broker * __rb;

    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("Looking up remote broker handle."));
    TRACE_INFO(("Matching:\n"
        "comm_layer_id: %s\nbroker_address: %s\n"
        "provider: %s\nprovider_module: %s",
        comm_layer_id, broker_address, provider,
        provider_module));

    INIT_LOCK(__remote_brokers_lock);
    CMPI_BrokerExt_Ftab->lockMutex(__remote_brokers_lock);

    for (__rb = __remote_brokers; __rb != NULL; __rb = __rb->next)
    {

        if (strcmp ( comm_layer_id,  __rb->comm_layer_id )  == 0 &&
            strcmp ( broker_address, __rb->broker_address ) == 0 &&
            strcmp ( provider,       __rb->provider )       == 0 &&
            strcmp ( provider_module,__rb->provider_module) == 0 &&
            compare_ticket ( ticket, &__rb->ticket ))
        {

            CMPI_BrokerExt_Ftab->unlockMutex(__remote_brokers_lock);

            TRACE_INFO(("returning existing handle."));

            RBAcquireMI ( (remote_broker *) __rb );

            TRACE_VERBOSE(("leaving function."));
            return(remote_broker *) __rb;
        }
    }

    __rb = __new_remote_broker (
        comm_layer_id,
        broker_address,
        provider,
        provider_module,
        ticket,
        brokerFT );
    __rb->next        = __remote_brokers;
    __remote_brokers  = __rb;

    CMPI_BrokerExt_Ftab->unlockMutex(__remote_brokers_lock);

    TRACE_INFO(("returning fresh handle."));

    RBAcquireMI ( (remote_broker *) __rb );

    TRACE_VERBOSE(("leaving function."));
    return(remote_broker *) __rb;
}


//! Cleans up unused remote brokers in specified time intervals.
/*!
    This function iterates through the list of currently maintained
    remote_broker handles, and deactivates those matching the following
    criteria:
    - the associated MI is currently active, i.e. its MI library is loaded
    - its use_count is zero
    - the time of its last usage has elapsed a specified timeout interval

    \param timeout the elapsed time (sec) before providers are being
                   deactivated.
    \param check_interval the time interval between two checks.

    \return never.

    \sa __deactivate_remote_broker()
*/
PEGASUS_EXPORT void PEGASUS_CMPIR_CDECL  cleanup_remote_brokers (
    long timeout,
    time_t check_interval )
{
    CMPI_COND_TYPE  c = NULL;
    CMPI_MUTEX_TYPE m = NULL;
    struct timespec wait = {0,0};
    struct timeval t;
    struct __remote_broker ** __rb, * tmp;
    int rc = 0;

    TRACE_VERBOSE(("entered function."));
    TRACE_NORMAL(("Cleaning up remote providers."));
    TRACE_INFO(("timeout interval: %ld (secs)", timeout));
    TRACE_INFO(("check interval: %ld (secs)", check_interval));

    c=CMPI_BrokerExt_Ftab->newCondition(0);
    m=CMPI_BrokerExt_Ftab->newMutex(0);

    do
    {
        TRACE_NORMAL(("Looking for remote providers "
            "to be cleaned up."));

        INIT_LOCK(__remote_brokers_lock);
        CMPI_BrokerExt_Ftab->lockMutex(__remote_brokers_lock);
        gettimeofday ( &t, NULL );

        for (__rb = &__remote_brokers; *__rb != NULL;)
        {

            if ((*__rb)->use_count == 0 &&
                t.tv_sec >= (*__rb)->last_used.tv_sec + timeout)
            {

                tmp = *__rb;
                *__rb = tmp->next;
                __cleanup_remote_broker ( tmp );

            }
            else
            {
                __rb = &(*__rb)->next;
            }
        }
        CMPI_BrokerExt_Ftab->unlockMutex(__remote_brokers_lock);

        wait.tv_sec = t.tv_sec + check_interval;
        wait.tv_nsec = 0;
        CMPI_BrokerExt_Ftab->lockMutex(m);
        rc = CMPI_BrokerExt_Ftab->timedCondWait( c, m, &wait );
        CMPI_BrokerExt_Ftab->unlockMutex(m);

    } while (rc);


    CMPI_BrokerExt_Ftab->destroyMutex(m);
    CMPI_BrokerExt_Ftab->destroyCondition(c);

    TRACE_CRITICAL(("Timed wait failed, leaving function."));
}


//! Initializes the CMPIContext to be used for activation of remote providers.
/*!
    The context given as argument is used to activate and deactivate providers
    respectively, since both calls expect a CMPIContext as argument.

    \sa __deactivate_remote_broker()
    \sa __get_instanceMI()
    \sa __get_associationMI()
    \sa __get_methodMI()
    \sa __get_propertyMI()
    \sa __get_indicationMI()
*/
PEGASUS_EXPORT void PEGASUS_CMPIR_CDECL init_activation_context (
    CMPIContext * ctx )
{
    __remote_brokers_context = ctx;
}


/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
