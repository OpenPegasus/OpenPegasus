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
    \file proxy_provider.c
    \brief Proxy Provider for CMPI to access remote providers.

    Remote providers are accessed by the MB through this proxy provider.
    The resolver is used to identify the remote peers either with a class-based
    object path or an instance-based one, depending on the provider call.

    Create() calls and cleanup() calls will not be relayed to remote locations,
    i.e. remote provider will be activated and cleaned up by their remote
    brokers automatically. Thus, these calls only gather necessary information,
    such as the broker handle.

    The results from the "real" MI calls will be either passed back directly,
    in case of an instance call with exactly one peer, or as a combined
    result, in case of enumerative calls.
*/

#include "cmpir_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
# include <error.h>
#endif

#include "proxy.h"
#include "resolver.h"
#include "ticket.h"
#include "debug.h"
#include "indication_objects.h"

PEGASUS_EXPORT CMPIBrokerExtFT *CMPI_BrokerExt_Ftab = NULL;
int nativeSide = 0;
#ifdef __GNUC__MULTI__

    //! Contains the entities to be passed to an enumerative remote MI request.
    struct enum_thread
    {
        provider_comm *comm;
        provider_address *addr;
        CMPIContext *ctx;
    };

    static CMPI_THREAD_TYPE spawn_enumerate_thread(
        provider_comm * comm,
        provider_address * addr,
        CMPIBroker * broker,
        CMPIContext * ctx,
        CMPIStatus * (*__thread) (struct enum_thread *))
    {
        CMPI_THREAD_TYPE t;
        struct enum_thread *et = (struct enum_thread *)
        malloc(sizeof(struct enum_thread));

        TRACE_NORMAL(("Spawning enumerative thread for "
            "comm-layer \"%s\", address \"%s\"",
            comm->id, addr->dst_address));
        et->comm = comm;
        et->addr = addr;
        et->ctx = CBPrepareAttachThread(broker, ctx);
        t=CMPI_BrokerExt_Ftab->newThread( (void *(*)(void *)) __thread, et,0);
        return t;
    }

#endif

void cleanup_if_last ()
{
    static int cleanup_count = 0;

    // If the counter has been called for all five cleanup
    // functions: Instance, Association, Indication, Property,
    // and Method..
    if (++cleanup_count >= 5)
    {
        // then call all of underlaying cleanup functions.
        cleanup_ticket();
        cleanup_count = 0;
    }
}

#ifdef CMPI_VER_100
    static CMPIStatus __InstanceMI_cleanup(
        CMPIInstanceMI * cThis,
        const CMPIContext * ctx,
        CMPIBoolean term)
#else
    static CMPIStatus __InstanceMI_cleanup(
        CMPIInstanceMI * cThis,
        CMPIContext * ctx)
#endif
    {
        RemoteCMPIInstanceMI *rcThis = (RemoteCMPIInstanceMI *) cThis;
        TRACE_NORMAL(
            ("Cleaning up proxy provider handle for: %s", rcThis->provider));

        unload_provider_comms();
        if (revoke_ticket(&rcThis->ticket))
        {
            TRACE_CRITICAL(("ticket could not be revoked."));
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "could not revoke ticket");
        }
        cleanup_if_last();
        free(rcThis->provider);
        free(rcThis);
        CMReturn(CMPI_RC_OK);
}

static CMPIStatus __InstanceMI_enumInstanceNames(
    CMPIInstanceMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIInstanceMI *rcThis = (RemoteCMPIInstanceMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm = load_provider_comm(
            addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->InstanceMI_enumInstanceNames(
                addr,
                rcThis,
                ctx,
                rslt,
                cop);
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
    CMReturnDone(rslt);
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __InstanceMI_enumInstances(
    CMPIInstanceMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    CONST char **props)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIInstanceMI *rcThis = (RemoteCMPIInstanceMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm = load_provider_comm(
            addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->InstanceMI_enumInstances(addr, rcThis, ctx, rslt,
                cop, props);
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }

        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
    CMReturnDone(rslt);
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __InstanceMI_getInstance(
    CMPIInstanceMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    CONST char **props)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIInstanceMI *rcThis = (RemoteCMPIInstanceMI *) cThis;
    provider_address *addr;
    provider_comm *comm;
    TRACE_NORMAL(("Executing single-targeted remote provider "
        "call for: %s.", rcThis->provider));
    if ((addr =
        resolve_instance(rcThis->broker, ctx, cop, rcThis->provider, NULL)))
    {
        comm = load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->InstanceMI_getInstance(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                props);
            CMReturnDone(rslt);
        }
        addr->destructor(addr);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __InstanceMI_createInstance(
    CMPIInstanceMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    CONST CMPIInstance * inst)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIInstanceMI *rcThis = (RemoteCMPIInstanceMI *) cThis;
    provider_address *addr;
    provider_comm *comm;
    TRACE_NORMAL(("Executing single-targeted remote provider "
        "call for: %s.", rcThis->provider));
    if ((addr =
        resolve_instance(rcThis->broker, ctx, cop, rcThis->provider, NULL)))
    {
        comm = load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->InstanceMI_createInstance(addr, rcThis, ctx, rslt,
                cop, inst);
            CMReturnDone(rslt);
        }
        addr->destructor(addr);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}


#ifdef CMPI_VER_100
/**
    This function prototype should be called 'modifyInstance'
    but since this proxy provider is not using the MI macro stub
    functions which use that, it is ok to leave it as is.
*/
static CMPIStatus __InstanceMI_setInstance(
    CMPIInstanceMI * cThis,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * inst,
    const char **props)
#else

static CMPIStatus __InstanceMI_setInstance(
    CMPIInstanceMI * cThis,
    CMPIContext * ctx,
    CMPIResult * rslt,
    CMPIObjectPath * cop,
    CMPIInstance * inst,
    char **props)
#endif
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIInstanceMI *rcThis = (RemoteCMPIInstanceMI *) cThis;
    provider_address *addr;
    provider_comm *comm;
    TRACE_NORMAL(("Executing single-targeted remote provider "
        "call for: %s.", rcThis->provider));
    if ((addr =
        resolve_instance(rcThis->broker, ctx, cop, rcThis->provider, NULL)))
    {
        comm = load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->InstanceMI_setInstance(addr, rcThis, ctx, rslt, cop,
                inst, props);
            CMReturnDone(rslt);
        }
        addr->destructor(addr);
    }
    else
    {
        CMReturnWithChars(rcThis->broker, CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __InstanceMI_deleteInstance(
    CMPIInstanceMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIInstanceMI *rcThis = (RemoteCMPIInstanceMI *) cThis;
    provider_address *addr;
    provider_comm *comm;
    TRACE_NORMAL(("Executing single-targeted remote provider "
        "call for: %s.", rcThis->provider));
    if ((addr =
        resolve_instance(rcThis->broker, ctx, cop, rcThis->provider, NULL)))
    {
        comm = load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->InstanceMI_deleteInstance(addr, rcThis, ctx, rslt, cop);
            CMReturnDone(rslt);
        }
        addr->destructor(addr);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __InstanceMI_execQuery(
    CMPIInstanceMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    CONST char *lang,
    CONST char *query)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIInstanceMI *rcThis = (RemoteCMPIInstanceMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->InstanceMI_execQuery(addr, rcThis, ctx, rslt, cop,
                lang, query);
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
    CMReturnDone(rslt);
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

#ifdef CMPI_VER_100
PEGASUS_EXPORT CMPIInstanceMI *_Generic_Create_InstanceMI(
    const CMPIBroker * broker,
    const CMPIContext * context,
    const char *provider,
    CMPIStatus *rc)
#else
PEGASUS_EXPORT CMPIInstanceMI *_Generic_Create_InstanceMI(
    CMPIBroker * broker,
    CMPIContext * context,
    const char *provider)
#endif
{
    static CMPIInstanceMIFT miFT =
    {
        CMPICurrentVersion,
        CMPICurrentVersion,
        "Instance" "RemoteCMPI",
        __InstanceMI_cleanup,
        __InstanceMI_enumInstanceNames,
        __InstanceMI_enumInstances,
        __InstanceMI_getInstance,
        __InstanceMI_createInstance,
        __InstanceMI_setInstance,
        __InstanceMI_deleteInstance,
        __InstanceMI_execQuery
    };
    RemoteCMPIInstanceMI *mi =
        (RemoteCMPIInstanceMI *) calloc(1, sizeof(RemoteCMPIInstanceMI));
    TRACE_NORMAL(("Creating proxy provider handle for: %s", provider));

    CMPI_BrokerExt_Ftab=broker->xft;
    mi->ref.ft = &miFT;
    mi->provider = strdup(provider);
    mi->broker = broker;
    mi->ticket = generate_ticket(broker);
    return(CMPIInstanceMI *) mi;
}

#ifdef CMPI_VER_100
static CMPIStatus __AssociationMI_cleanup(
    CMPIAssociationMI * cThis,
    const CMPIContext * ctx,
    CMPIBoolean term)
#else
static CMPIStatus __AssociationMI_cleanup(
    CMPIAssociationMI * cThis,
    CMPIContext * ctx)
#endif
{
    RemoteCMPIAssociationMI *rcThis = (RemoteCMPIAssociationMI *) cThis;
    TRACE_NORMAL(("Cleaning up proxy provider handle for: %s",
        rcThis->provider));

    unload_provider_comms();

    if (revoke_ticket(&rcThis->ticket))
    {
        TRACE_CRITICAL(("ticket could not be revoked."));
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not revoke ticket");
    };
    cleanup_if_last();

    free(rcThis->provider);
    free(rcThis);
    CMReturn(CMPI_RC_OK);
}

static CMPIStatus __AssociationMI_associators(
    CMPIAssociationMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    const char *assocclass,
    const char *resultclass,
    const char *role,
    const char *resultrole,
    CONST char **props)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIAssociationMI *rcThis = (RemoteCMPIAssociationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->AssociationMI_associators(addr, rcThis, ctx, rslt,
                cop, assocclass,
                resultclass, role,
                resultrole, props);
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
    CMReturnDone(rslt);
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __AssociationMI_associatorNames(
    CMPIAssociationMI *
    cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    const char *assocclass,
    const char *resultclass,
    const char *role,
    const char *resultrole)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIAssociationMI *rcThis = (RemoteCMPIAssociationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->AssociationMI_associatorNames(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                assocclass,
                resultclass,
                role,
                resultrole);
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
    CMReturnDone(rslt);
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __AssociationMI_references(
    CMPIAssociationMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    const char *assocclass,
    const char *role,
    CONST char **props)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIAssociationMI *rcThis = (RemoteCMPIAssociationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(
        ("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->AssociationMI_references(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                assocclass,
                role,
                props);
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
    CMReturnDone(rslt);
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __AssociationMI_referenceNames(
    CMPIAssociationMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    const char *assocclass,
    const char *role)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIAssociationMI *rcThis = (RemoteCMPIAssociationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(
        ("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->AssociationMI_referenceNames(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                assocclass,role);
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
    CMReturnDone(rslt);
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

#ifdef CMPI_VER_100
PEGASUS_EXPORT CMPIAssociationMI *_Generic_Create_AssociationMI(
    const CMPIBroker * broker,
    const CMPIContext * context,
    const char *provider,
    CMPIStatus *rc)

#else
PEGASUS_EXPORT CMPIAssociationMI *_Generic_Create_AssociationMI(
    CMPIBroker * broker,
    CMPIContext * context,
    const char *provider)
#endif
{
    static CMPIAssociationMIFT miFT =
    {
        CMPICurrentVersion,
        CMPICurrentVersion,
        "Association" "RemoteCMPI",
        __AssociationMI_cleanup,
        __AssociationMI_associators,
        __AssociationMI_associatorNames,
        __AssociationMI_references,
        __AssociationMI_referenceNames
    };
    RemoteCMPIAssociationMI *mi =
        (RemoteCMPIAssociationMI *) calloc(1, sizeof(RemoteCMPIAssociationMI));
    TRACE_NORMAL(("Creating proxy provider handle for: %s", provider));

    CMPI_BrokerExt_Ftab=broker->xft;
    mi->ref.ft = &miFT;
    mi->provider = strdup(provider);
    mi->broker = broker;
    mi->ticket = generate_ticket(broker);
    return(CMPIAssociationMI *) mi;
}


#ifdef CMPI_VER_100
static CMPIStatus __MethodMI_cleanup(
    CMPIMethodMI * cThis,
    const CMPIContext * ctx,
    CMPIBoolean term)
#else
static CMPIStatus __MethodMI_cleanup(CMPIMethodMI * cThis, CMPIContext * ctx)
#endif
{
    RemoteCMPIMethodMI *rcThis = (RemoteCMPIMethodMI *) cThis;
    TRACE_NORMAL(("Cleaning up proxy provider handle for: %s",
        rcThis->provider));

    unload_provider_comms();

    if (revoke_ticket(&rcThis->ticket))
    {
        TRACE_CRITICAL(("ticket could not be revoked."));
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not revoke ticket");
    };
    cleanup_if_last();
    free(rcThis->provider);
    free(rcThis);
    CMReturn(CMPI_RC_OK);
}

static CMPIStatus __MethodMI_invokeMethod(
    CMPIMethodMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    const char *method,
    CONST CMPIArgs * in,
    CMPIArgs * out)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIMethodMI *rcThis = (RemoteCMPIMethodMI *) cThis;
    provider_address *addr;
    provider_comm *comm;
    TRACE_NORMAL(("Executing single-targeted remote provider "
        "call for: %s.", rcThis->provider));
    if ((addr =
        resolve_instance(rcThis->broker, ctx, cop, rcThis->provider, NULL)))
    {
        comm = load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->MethodMI_invokeMethod(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                method,
                in,
                out);
            CMReturnDone(rslt);
        }
        addr->destructor(addr);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

#ifdef CMPI_VER_100
PEGASUS_EXPORT CMPIMethodMI *_Generic_Create_MethodMI(
    CMPIBroker * broker,
    const CMPIContext * context,
    const char *provider,
    CMPIStatus *rc)

#else
PEGASUS_EXPORT CMPIMethodMI *_Generic_Create_MethodMI(
    CMPIBroker * broker,
    CMPIContext * context,
    const char *provider)
#endif
{
    static CMPIMethodMIFT miFT =
    {
        CMPICurrentVersion,
        CMPICurrentVersion,
        "Method" "RemoteCMPI",
        __MethodMI_cleanup,
        __MethodMI_invokeMethod
    };
    RemoteCMPIMethodMI *mi =
        (RemoteCMPIMethodMI *) calloc(1, sizeof(RemoteCMPIMethodMI));
    TRACE_NORMAL(("Creating proxy provider handle for: %s", provider));

    CMPI_BrokerExt_Ftab=broker->xft;
    mi->ref.ft = &miFT;
    mi->provider = strdup(provider);
    mi->broker = broker;
    mi->ticket = generate_ticket(broker);
    return(CMPIMethodMI *) mi;
}


#ifdef CMPI_VER_100
static CMPIStatus __PropertyMI_cleanup(
    CMPIPropertyMI * cThis,
    const CMPIContext * ctx,
    CMPIBoolean term)

#else
static CMPIStatus __PropertyMI_cleanup(
    CMPIPropertyMI * cThis,
    CMPIContext * ctx)
#endif
{
    RemoteCMPIPropertyMI *rcThis = (RemoteCMPIPropertyMI *) cThis;
    TRACE_NORMAL(("Cleaning up proxy provider handle for: %s",
        rcThis->provider));

    unload_provider_comms();

    if (revoke_ticket(&rcThis->ticket))
    {
        TRACE_CRITICAL(("ticket could not be revoked."));
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not revoke ticket");
    };
    cleanup_if_last();
    free(rcThis->provider);
    free(rcThis);
    CMReturn(CMPI_RC_OK);
}

static CMPIStatus __PropertyMI_setProperty(
    CMPIPropertyMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    const char *name,
    CONST CMPIData data)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIPropertyMI *rcThis = (RemoteCMPIPropertyMI *) cThis;
    provider_address *addr;
    provider_comm *comm;
    TRACE_NORMAL(("Executing single-targeted remote provider "
        "call for: %s.", rcThis->provider));
    if ((addr =
        resolve_instance(rcThis->broker, ctx, cop, rcThis->provider, NULL)))
    {
        comm = load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->PropertyMI_setProperty(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                name,
                data);
            CMReturnDone(rslt);
        }
        addr->destructor(addr);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

static CMPIStatus __PropertyMI_getProperty(
    CMPIPropertyMI * cThis,
    CONST CMPIContext * ctx,
    CONST CMPIResult * rslt,
    CONST CMPIObjectPath * cop,
    const char *name)
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIPropertyMI *rcThis = (RemoteCMPIPropertyMI *) cThis;
    provider_address *addr;
    provider_comm *comm;
    TRACE_NORMAL(("Executing single-targeted remote provider "
        "call for: %s.", rcThis->provider));
    if ((addr =
        resolve_instance(rcThis->broker, ctx, cop, rcThis->provider, NULL)))
    {
        comm = load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
            rc = comm->PropertyMI_getProperty(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                name);
            CMReturnDone(rslt);
        }
        addr->destructor(addr);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }

    return rc;

}

#ifdef CMPI_VER_100
PEGASUS_EXPORT CMPIPropertyMI *_Generic_Create_PropertyMI(
    CMPIBroker * broker,
    const CMPIContext * context,
    const char *provider,
    CMPIStatus *rc)

#else
PEGASUS_EXPORT CMPIPropertyMI *_Generic_Create_PropertyMI(
    CMPIBroker * broker,
    CMPIContext * context,
    const char *provider)
#endif
{
    static CMPIPropertyMIFT miFT =
    {
        CMPICurrentVersion,
        CMPICurrentVersion,
        "Property" "RemoteCMPI",
        __PropertyMI_cleanup,
        __PropertyMI_setProperty,
        __PropertyMI_getProperty
    };
    RemoteCMPIPropertyMI *mi =
        (RemoteCMPIPropertyMI *) calloc(1, sizeof(RemoteCMPIPropertyMI));
    TRACE_NORMAL(("Creating proxy provider handle for: %s", provider));

    CMPI_BrokerExt_Ftab=broker->xft;
    mi->ref.ft = &miFT;
    mi->provider = strdup(provider);
    mi->broker = broker;
    mi->ticket = generate_ticket(broker);
    return(CMPIPropertyMI *) mi;
}

#ifdef CMPI_VER_100
static CMPIStatus __IndicationMI_cleanup(
    CMPIIndicationMI * cThis,
    const CMPIContext * ctx,
    CMPIBoolean term)
#else

static CMPIStatus __IndicationMI_cleanup(
    CMPIIndicationMI * cThis,
    CMPIContext * ctx)
#endif
{
    RemoteCMPIIndicationMI *rcThis = (RemoteCMPIIndicationMI *) cThis;
    TRACE_NORMAL(("Cleaning up proxy provider handle for: %s",
        rcThis->provider));

    unload_provider_comms();

    if (revoke_ticket(&rcThis->ticket))
    {
        TRACE_CRITICAL(("ticket could not be revoked."));
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not revoke ticket");
    };
    cleanup_if_last();
    free(rcThis->provider);
    free(rcThis);
    CMReturn(CMPI_RC_OK);
}

#ifdef CMPI_VER_100
static CMPIStatus __IndicationMI_authorizeFilter(
    CMPIIndicationMI * cThis,
    const CMPIContext * ctx,
    const CMPISelectExp * filter,
    const char *indType,
    const CMPIObjectPath * cop,
    const char *owner)
#else
static CMPIStatus __IndicationMI_authorizeFilter(
    CMPIIndicationMI * cThis,
    CMPIContext * ctx,
    CMPIResult * rslt,
    CMPISelectExp * filter,
    const char *indType,
    CMPIObjectPath * cop,
    const char *owner)
#endif
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIIndicationMI *rcThis = (RemoteCMPIIndicationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
#ifdef CMPI_VER_100
            rc = comm->IndicationMI_authorizeFilter(
                addr,
                rcThis,
                ctx,
                filter,
                indType,
                cop,
                owner);
#else
            // IBMKR: This looks like a wrong prototype.
            rc = comm->IndicationMI_authorizeFilter(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                filter,
                indType,
                owner);
#endif
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
#ifndef CMPI_VER_100
    CMReturnDone(rslt);
#endif
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

#ifdef CMPI_VER_100
static CMPIStatus __IndicationMI_mustPoll(
    CMPIIndicationMI * cThis,
    const CMPIContext * ctx,
    const CMPISelectExp * filter,
    const char *indType,
    const CMPIObjectPath * cop)

#else
static CMPIStatus __IndicationMI_mustPoll(
    CMPIIndicationMI * cThis,
    CMPIContext * ctx,
    CMPIResult * rslt,
    CMPISelectExp * filter,
    const char *indType,
    CMPIObjectPath * cop)
#endif
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIIndicationMI *rcThis = (RemoteCMPIIndicationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
#ifdef CMPI_VER_100
            rc = comm->IndicationMI_mustPoll(
                addr,
                rcThis,
                ctx,
                filter,
                indType,
                cop);

#else
            //IBMKR: The set of arguments looks wrong?
            rc = comm->IndicationMI_mustPoll(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                filter,
                indType);

#endif
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
#ifndef CMPI_VER_100
    CMReturnDone(rslt);
#endif
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

#ifdef CMPI_VER_100
static CMPIStatus __IndicationMI_activateFilter(
    CMPIIndicationMI * cThis,
    const CMPIContext * ctx,
    const CMPISelectExp * filter,
    const char *clsName,
    const CMPIObjectPath * cop,
    CMPIBoolean firstActivation)
#else

static CMPIStatus __IndicationMI_activateFilter(
    CMPIIndicationMI * cThis,
    CMPIContext * ctx,
    CMPIResult * rslt,
    CMPISelectExp * filter,
    const char *clsName,
    CMPIObjectPath * cop,
    CMPIBoolean firstActivation)
#endif
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIIndicationMI *rcThis = (RemoteCMPIIndicationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
#ifdef CMPI_VER_100
            rc = comm->IndicationMI_activateFilter(
                addr,
                rcThis,
                ctx,
                filter,
                clsName,
                cop,
                firstActivation);
#else
            // IBMKR: This function prototype looks wrong.
            rc = comm->IndicationMI_activateFilter(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                filter,
                clsName,
                firstActivation);

#endif
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
#ifndef CMPI_VER_100
    CMReturnDone(rslt);
#endif
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(rcThis->broker, CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

#ifdef CMPI_VER_100
static CMPIStatus __IndicationMI_deActivateFilter(
    CMPIIndicationMI * cThis,
    const CMPIContext * ctx,
    const CMPISelectExp * filter,
    const char *clsName,
    const CMPIObjectPath * cop,
    CMPIBoolean lastActivation)
#else
static CMPIStatus __IndicationMI_deActivateFilter(
    CMPIIndicationMI * cThis,
    CMPIContext * ctx,
    CMPIResult * rslt,
    CMPISelectExp * filter,
    const char *clsName,
    CMPIObjectPath * cop,
    CMPIBoolean lastActivation)
#endif
{
    CMPIStatus rc = { CMPI_RC_ERR_FAILED, NULL};
    RemoteCMPIIndicationMI *rcThis = (RemoteCMPIIndicationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class(rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm(addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
#if defined (CMPI_VER_100)
            rc = comm->IndicationMI_deActivateFilter(
                addr,
                rcThis,
                ctx,
                filter,
                clsName,
                cop,
                lastActivation);
#else
            rc = comm->IndicationMI_deActivateFilter(
                addr,
                rcThis,
                ctx,
                rslt,
                cop,
                filter,
                clsName,
                lastActivation);

#endif
        }
        else
        {
            tmp->destructor(tmp);
            CMReturnWithChars(
                rcThis->broker,
                CMPI_RC_ERR_FAILED,
                "comm-layer not found");
        }
        if (rc.rc != CMPI_RC_OK)
        {
            tmp->destructor(tmp);
            return rc;
        }
    }
#ifndef CMPI_VER_100
    CMReturnDone(rslt);
#endif
    if (tmp)
    {
        tmp->destructor(tmp);
    }
    else
    {
        CMReturnWithChars(
            rcThis->broker,
            CMPI_RC_ERR_FAILED,
            "could not resolve location");
    }
    return rc;
}

#ifdef CMPI_VER_100
static CMPIStatus __IndicationMI_enableIndications(
    CMPIIndicationMI * cThis,
    const CMPIContext *ctx)
#else
static CMPIStatus __IndicationMI_enableIndications(CMPIIndicationMI * cThis)
#endif
{
    CMPIObjectPath *cop = NULL;
    CMPIStatus rc;
    RemoteCMPIIndicationMI *rcThis = (RemoteCMPIIndicationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class (rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm (addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
#if defined (CMPI_VER_100)
            rc = comm->IndicationMI_enableIndications (addr, rcThis, ctx);
#else
            rc = comm->IndicationMI_enableIndications (addr, rcThis);
#endif
        }
        else
        {
            tmp->destructor (tmp);
            TRACE_CRITICAL(( "comm-layer not found"));
        }
    }
    if (tmp)
    {
        tmp->destructor (tmp);
    }
    else
    {
        TRACE_CRITICAL(( "could not resolve location"));
    }

    return rc;
}



#ifdef CMPI_VER_100
static CMPIStatus __IndicationMI_disableIndications(
    CMPIIndicationMI * cThis,
    const CMPIContext *ctx)
#else
static CMPIStatus __IndicationMI_disableIndications(CMPIIndicationMI * cThis)
#endif
{
    CMPIObjectPath *cop = NULL;
    CMPIStatus rc;
    RemoteCMPIIndicationMI *rcThis = (RemoteCMPIIndicationMI *) cThis;
    provider_address *addr, *tmp;
    TRACE_NORMAL(("Executing fan-out remote provider call " "for: %s.",
        rcThis->provider));
    tmp = addr =
        resolve_class (rcThis->broker, ctx, cop, rcThis->provider, NULL);
    for (; addr != NULL; addr = addr->next)
    {
        provider_comm *comm =
            load_provider_comm (addr->comm_layer_id, rcThis->broker, ctx);
        if (comm != NULL)
        {
#if defined (CMPI_VER_100)
            rc = comm->IndicationMI_disableIndications (addr, rcThis, ctx);
#else
            rc = comm->IndicationMI_disableIndications (addr, rcThis);
#endif
        }
        else
        {
            tmp->destructor (tmp);
            TRACE_CRITICAL(( "comm-layer not found"));
        }
    }
    if (tmp)
    {
        tmp->destructor (tmp);
    }
    else
    {
        TRACE_CRITICAL(( "could not resolve location"));
    }
    return rc;
}



#ifdef CMPI_VER_100
PEGASUS_EXPORT CMPIIndicationMI *_Generic_Create_IndicationMI(
    CMPIBroker * broker,
    const CMPIContext * context,
    const char *provider,
    CMPIStatus *rc)
#else
PEGASUS_EXPORT CMPIIndicationMI *_Generic_Create_IndicationMI(
    CMPIBroker * broker,
    CMPIContext * context,
    const char *provider)
#endif
{
    static CMPIIndicationMIFT miFT =
    {
        CMPICurrentVersion,
        CMPICurrentVersion,
        "Indication" "RemoteCMPI",
        __IndicationMI_cleanup,
        __IndicationMI_authorizeFilter,
        __IndicationMI_mustPoll,
        __IndicationMI_activateFilter,
        __IndicationMI_deActivateFilter,
        __IndicationMI_enableIndications,
        __IndicationMI_disableIndications,
    };
    RemoteCMPIIndicationMI *mi =
        (RemoteCMPIIndicationMI *) calloc(1, sizeof(RemoteCMPIIndicationMI));
    TRACE_NORMAL(("Creating proxy provider handle for: %s", provider));

    CMPI_BrokerExt_Ftab=broker->xft;
    mi->ref.ft = &miFT;
    mi->provider = strdup(provider);
    mi->broker = broker;
    mi->ticket = generate_ticket(broker);
    return(CMPIIndicationMI *) mi;
}
