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
    \file args.c
    \brief Native CMPIArgs implementation.

    This is the native CMPIArgs implementation as used for remote
    providers. It reflects the well-defined interface of a regular CMPIArgs
    object, however, it works independently from the management broker.

    It is part of a native broker implementation that simulates CMPI data
    types rather than interacting with the entities in a full-grown CIMOM.

    \author Frank Scheffler
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native.h"
#include "mm.h"


//! Native extension of the CMPIArgs data type.
/*!
    This structure stores the information needed to represent arguments for
    CMPI providers, i.e. within invokeMethod() calls.
*/
struct native_args
{
    CMPIArgs args;      /*! < the inheriting data structure  */
    int mem_state;      /*! < states, whether this object is
                            registered within the memory mangagement or
                            represents a cloned object */

    struct native_property * data;  /*!< argument content */
};


static struct native_args * __new_empty_args ( int, CMPIStatus * );

/****************************************************************************/


static CMPIStatus __aft_release ( CMPIArgs * args )
{
    struct native_args * a = (struct native_args *) args;
    CMPIStatus rc = checkArgsReturnStatus(args);

    if (rc.rc == CMPI_RC_OK && a->mem_state == TOOL_MM_NO_ADD)
    {
        tool_mm_add ( a );
        a->mem_state = TOOL_MM_ADD;
        propertyFT.release ( a->data );
    }


    return rc;
}


static CMPIArgs * __aft_clone ( CONST CMPIArgs * args, CMPIStatus * rc )
{
    struct native_args * a   = (struct native_args *) args;
    struct native_args * new;

    if (!checkArgs(args, rc))
    {
        return 0;
    }
    new = __new_empty_args ( TOOL_MM_NO_ADD, rc );

    if (rc->rc == CMPI_RC_OK)
    {
        new->data = propertyFT.clone ( a->data, rc );
    }

    return(CMPIArgs *) new;
}



static CMPIStatus __aft_addArg (
    CONST CMPIArgs * args,
    const char * name,
    CONST CMPIValue * value,
    CONST CMPIType type )
{
    struct native_args * a = (struct native_args *) args;

    CMPIStatus rc = checkArgsReturnStatus(args);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    CMReturn (
        ( propertyFT.addProperty (
        &a->data,
        a->mem_state,
        name,
        type,
        0,
        value ) )?
        CMPI_RC_ERR_ALREADY_EXISTS:
        CMPI_RC_OK );
}



static CMPIData __aft_getArg (
    CONST CMPIArgs * args,
    const char * name,
    CMPIStatus * rc )
{
    struct native_args * a = (struct native_args *) args;
    CMPIData data = checkArgsReturnData(args,rc);

    if (data.state == CMPI_badValue)
    {
        return data;
    }
    return propertyFT.getDataProperty ( a->data, name, rc );
}

static CMPIData __aft_getArgAt (
    CONST CMPIArgs * args,
    unsigned int index,
    CMPIString ** name,
    CMPIStatus * rc )
{
    struct native_args * a = (struct native_args *) args;
    CMPIData data = checkArgsReturnData(args,rc);

    if (data.state == CMPI_badValue)
    {
        return data;
    }
    return propertyFT.getDataPropertyAt ( a->data, index, name, rc );
}


static unsigned int __aft_getArgCount (
    CONST CMPIArgs * args,
    CMPIStatus * rc )
{
    struct native_args * a = (struct native_args *) args;

    if (!checkArgs(args, rc))
    {
        return 0;
    }
    return propertyFT.getPropertyCount ( a->data, rc );
}

static CMPIArgsFT aft =
{
    NATIVE_FT_VERSION,
    __aft_release,
    __aft_clone,
    __aft_addArg,
    __aft_getArg,
    __aft_getArgAt,
    __aft_getArgCount
};

CMPIArgsFT *CMPI_Args_FT = &aft;

static struct native_args * __new_empty_args ( int mm_add, CMPIStatus * rc )
{

    static CMPIArgs a = {
        "CMPIArgs",
        &aft
    };

    struct native_args * args =
        (struct native_args *) tool_mm_alloc (
        mm_add, sizeof ( struct native_args ) );

    args->args      = a;
    args->mem_state = mm_add;

    CMSetStatus ( rc, CMPI_RC_OK );
    return args;
}

extern char * value2Chars ( CMPIType type, CMPIValue * value );

CMPIString *args2String( CONST CMPIArgs *args, CMPIStatus *rc)
{
    char str[2048];
    CMPIData data;
    unsigned int i,m;
    CMPIString *name;
    char *v;

    sprintf(str,"%p: ",args);
    for (i=0,m=__aft_getArgCount(args,rc); i<m; i++)
    {
        data=__aft_getArgAt(args,i,&name,rc);
        strcat(str,(char*)name->hdl);
        strcat(str,":");
        v=value2Chars(data.type,&data.value);
        strcat(str,v);
        strcat(str,"\n");
        free(v);
    }
    return native_new_CMPIString ( str, rc );
}

CMPIArgs * native_new_CMPIArgs ( CMPIStatus * rc )
{
    return(CMPIArgs *) __new_empty_args ( TOOL_MM_ADD, rc );
}

/*****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
