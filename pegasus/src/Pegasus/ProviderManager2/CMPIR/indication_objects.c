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


/*
    This file contains all necessary methods to keep track of the indication
    objects created in a particular context.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if defined(PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
# include <error.h>
#endif


#include "indication_objects.h"
#include "debug.h"

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Provider/CMPI/cmpift.h>

extern CMPIBrokerExtFT *CMPI_BrokerExt_Ftab;

static indication_objects *__indication_objects = NULL;

static CMPI_MUTEX_TYPE _indication_objects_lock = NULL;

#define INIT_LOCK(l) if (l==NULL) l=CMPI_BrokerExt_Ftab->newMutex(0);


// Helper function
void __free_ind_object (ind_object *obj)
{
    switch (obj->type)
    {
        case PEGASUS_INDICATION_OBJECT_TYPE_CMPI_SELECT_EXP :
            CMRelease ( (CMPISelectExp*)obj->ptr);
            break;
        case PEGASUS_INDICATION_OBJECT_TYPE_CMPI_SELECT_COND:
            CMRelease ( (CMPISelectCond*)obj->ptr);
            break;
        case PEGASUS_INDICATION_OBJECT_TYPE_CMPI_SUB_COND:
            CMRelease ( (CMPISubCond*)obj->ptr);
            break;
        case PEGASUS_INDICATION_OBJECT_TYPE_CMPI_PREDICATE:
            CMRelease ( (CMPIPredicate*)obj->ptr);
            break;
        default :
            TRACE_CRITICAL(("Unknown Object type: %d", obj->type ));
    }
}

/*
    This function creates reference to the Object created in MB.
    This reference is passed to Remote Daemon, any changes or any
    calls made on this object on daemon side, will make UP calls
    to MB and uses this object.
*/
CMPIUint64 create_indicationObject (
    void *obj,
    CMPIUint32 ctx_id,
    CMPIUint8 type)
{
    indication_objects *tmp;
    ind_object *ind_obj;

    TRACE_NORMAL(("Creating Indication Object."));

    INIT_LOCK(_indication_objects_lock);
    CMPI_BrokerExt_Ftab->lockMutex(_indication_objects_lock);

    tmp = __indication_objects;
    while (tmp)
    {
        if (tmp->ctx_id == ctx_id)
        {
            break;
        }
        tmp = tmp->next;
    }
    if (!tmp)
    {
        tmp = (indication_objects *)
        malloc ( sizeof ( indication_objects ) );
        tmp->ctx_id = ctx_id;
        tmp->next = __indication_objects;
        __indication_objects = tmp;
        tmp->objects = NULL;
    }
    ind_obj = (ind_object*) malloc ( sizeof ( ind_object ) );
    ind_obj->id = (ssize_t) obj;
    ind_obj->ptr = obj;
    ind_obj->type = type;
    ind_obj->next = tmp->objects;
    tmp->objects = ind_obj;

    CMPI_BrokerExt_Ftab->unlockMutex(_indication_objects_lock);

    TRACE_INFO(("Created object with id: %llu", ind_obj->id ));

    return ind_obj->id;
}

int remove_indicationObject (void *obj, CMPIUint32 ctx_id)
{
    ind_object **tmp;
    indication_objects *curr;
    CMPIUint64 id = ( (ind_object*)obj)->id;

    TRACE_NORMAL(("Deleting Indication Object."));

    INIT_LOCK(_indication_objects_lock);
    CMPI_BrokerExt_Ftab->lockMutex(_indication_objects_lock);
    curr = __indication_objects;
    while (curr)
    {
        if (curr->ctx_id == ctx_id)
        {
            break;
        }
        curr = curr->next;
    }
    for (tmp = &curr->objects; *tmp != NULL; *tmp = (*tmp)->next)
    {
        if ((*tmp)->id == id)
        {
            ind_object *r = (*tmp);
            (*tmp) = r->next;
            __free_ind_object (r);
            TRACE_INFO(("Deleted Indication Object with ID: %llu", id));
            CMPI_BrokerExt_Ftab->unlockMutex(_indication_objects_lock);
            return 0;
        }
    }
    CMPI_BrokerExt_Ftab->unlockMutex(_indication_objects_lock);
    return -1;
}

void* get_indicationObject (CMPIUint64 id, CMPIUint32 ctx_id)
{
    indication_objects *curr;
    ind_object *tmp;
    int global_context = 0;

    TRACE_NORMAL(("Getting the  Indication Object."));

    INIT_LOCK(_indication_objects_lock);
    CMPI_BrokerExt_Ftab->lockMutex(_indication_objects_lock);

    do
    {
        curr = __indication_objects;
        while (curr)
        {
            if (curr->ctx_id == ctx_id)
            {
                break;
            }
            curr = curr->next;
        }
        if (!curr && !global_context)
        {
            global_context = 1;
            ctx_id = PEGASUS_INDICATION_GLOBAL_CONTEXT;
        }
        else
        {
            break;
        }
    }while (global_context);

    for (tmp = curr ? curr->objects : NULL; tmp != NULL; tmp = tmp->next)
    {
        if (tmp->id == id)
        {
            TRACE_INFO(("Got the Indication Object with ID: %llu", id));
            CMPI_BrokerExt_Ftab->unlockMutex(_indication_objects_lock);
            return tmp->ptr;
        }
    }
    CMPI_BrokerExt_Ftab->unlockMutex(_indication_objects_lock);

    return NULL;

}

void cleanup_indicationObjects (CMPIUint32 ctx_id)
{
    indication_objects *tmp, *prev;
    ind_object *obj;

    TRACE_NORMAL(("Cleaning all Indication Objects."));

    INIT_LOCK(_indication_objects_lock);
    CMPI_BrokerExt_Ftab->lockMutex(_indication_objects_lock);
    tmp = __indication_objects;
    while (tmp)
    {
        if (tmp->ctx_id == ctx_id)
        {
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    if (!tmp)
    {
        TRACE_CRITICAL(("Indication context id (%u) not found.", ctx_id));
        return;
    }
    while (tmp->objects)
    {
        obj = tmp->objects;
        tmp->objects = tmp->objects->next;
        __free_ind_object (obj);
    }
    if (tmp == __indication_objects)
    {
        __indication_objects = __indication_objects->next;
    }
    else
    {
        prev->next = tmp->next;
    }
    free (tmp);
    CMPI_BrokerExt_Ftab->unlockMutex(_indication_objects_lock);
}

