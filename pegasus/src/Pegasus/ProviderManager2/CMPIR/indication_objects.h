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
    \file indication_objects.h
    \brief Manages pointers to the objects that are created on remote-side
    \ as part of indication processs.
*/

#ifndef _REMOTE_CMPI_INDICATION_OBJECTS_H
#define _REMOTE_CMPI_INDICATION_OBJECTS_H


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

#include "cmpir_common.h"
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>

// Object types

#define PEGASUS_INDICATION_OBJECT_TYPE_CMPI_SELECT_EXP  0
#define PEGASUS_INDICATION_OBJECT_TYPE_CMPI_SELECT_COND 1
#define PEGASUS_INDICATION_OBJECT_TYPE_CMPI_SUB_COND    2
#define PEGASUS_INDICATION_OBJECT_TYPE_CMPI_PREDICATE   3

#define PEGASUS_INDICATION_GLOBAL_CONTEXT 0
#define PEGASUS_INDICATION_LOCAL_CONTEXT 1

/*
    This struture holds the information about the objects that are
    created as part of invocation of Indication providers on Remote side.
    We do not create any objects (creted during the process of Indications)
    on remote-side, instead we make UP calls to MB and create the objects and
    pass pointer to remote machine. -V 5245
*/
typedef struct indication_objects indication_objects;
typedef struct ind_object ind_object;
struct ind_object
{
    CMPIUint64 id;
    void *ptr;
    CMPIUint8 type;
    ind_object *next;
};

struct indication_objects
{
    CMPIUint32 ctx_id;
    ind_object *objects;
    indication_objects *next;
};


/****************************************************************************/

PEGASUS_EXPORT CMPIUint64 create_indicationObject (
    void *obj,
    CMPIUint32 ctx_id,
    CMPIUint8 type);
PEGASUS_EXPORT int remove_indicationObject (void *obj, CMPIUint32 ctx_id);
PEGASUS_EXPORT void *get_indicationObject (CMPIUint64 id, CMPIUint32 ctx_id);
PEGASUS_EXPORT void cleanup_indicationObjects (CMPIUint32 ctx_id);
#endif

/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
