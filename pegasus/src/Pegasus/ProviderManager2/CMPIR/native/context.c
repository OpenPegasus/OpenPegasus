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
    \file context.c
    \brief Native CMPIContext implementation.

    This is the native CMPIContext implementation as used for remote
    providers. It reflects the well-defined interface of a regular
    CMPIContext, however, it works independently from the management broker.

    It is part of a native broker implementation that simulates CMPI data
    types rather than interacting with the entities in a full-grown CIMOM.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmpir_common.h"
#include "mm.h"
#include "native.h"



//! Native extension of the CMPIContext data type.
/*!
    This structure stores the information needed to represent contexts for
    CMPI providers.
*/
struct native_context
{
    CMPIContext ctx;    /*! < the inheriting data structure  */
    int mem_state;      /*! < states, whether this object is
                            registered within the memory mangagement or
                            represents a cloned object */

    struct native_property * entries;   /*!< context content */
   struct native_property * containers; // Holds containers
                                  // ex "SnmpTrapOidContainer"
};


static struct native_context * __new_empty_context ( int );


/****************************************************************************/


static CMPIStatus __cft_release ( CMPIContext * ctx )
{
    CMPIStatus rc = checkArgsReturnStatus(ctx);
    return rc;
}


static CMPIContext * __cft_clone ( CONST CMPIContext * ctx, CMPIStatus * rc )
{
    if (!checkArgs(ctx, rc))
    {
        return 0;
    }
    CMSetStatus ( rc, CMPI_RC_ERR_NOT_SUPPORTED );
    return NULL;
}


static CMPIData __cft_getEntry (
    CONST CMPIContext * ctx,
    const char * name,
    CMPIStatus * rc )
{
    struct native_context * c = (struct native_context *) ctx;
    struct native_property *p = c->entries;

    CMPIData data = checkArgsReturnData(ctx, rc);

    if (data.state == CMPI_badValue)
    {
        return data;
    }
    // Check for containers
    if (!strcmp (name, "SnmpTrapOidContainer") )
    {
        p = c->containers;
    }

    return propertyFT.getDataProperty(p, name, rc);
}


static CMPIData __cft_getEntryAt (
    CONST CMPIContext * ctx,
    unsigned int index,
    CMPIString ** name,
    CMPIStatus * rc )
{
    struct native_context * c = (struct native_context *) ctx;

    CMPIData data = checkArgsReturnData(ctx, rc);

    if (data.state == CMPI_badValue)
    {
        return data;
    }

    return propertyFT.getDataPropertyAt ( c->entries, index, name, rc );
}


static unsigned int __cft_getEntryCount (
    CONST CMPIContext * ctx,
    CMPIStatus * rc )
{
    struct native_context * c = (struct native_context *) ctx;
    if (!checkArgs(ctx, rc))
    {
        return 0;
    }

    return propertyFT.getPropertyCount ( c->entries, rc );
}


static CMPIStatus __cft_addEntry (
    CONST CMPIContext * ctx,
    const char * name,
    CONST CMPIValue * value,
    CONST CMPIType type )
{
    struct native_context * c = (struct native_context *) ctx;
    struct native_property **p = &c->entries;

    CMPIStatus rc = checkArgsReturnStatus(ctx);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }
    // Check for containers, this method should be in sync with
    // contextAddEntry method on MB side.
    if (!strcmp (name, "SnmpTrapOidContainer") )
    {
        p = &c->containers;
    }
    CMReturn ( ( propertyFT.addProperty (
        p,
        c->mem_state,
        name,
        type,
        0,
        value ) )?
        CMPI_RC_ERR_ALREADY_EXISTS:
        CMPI_RC_OK );
}


static struct native_context * __new_empty_context ( int mm_add )
{
    static CMPIContextFT cft = {
        NATIVE_FT_VERSION,
        __cft_release,
        __cft_clone,
        __cft_getEntry,
        __cft_getEntryAt,
        __cft_getEntryCount,
        __cft_addEntry
    };
    static CMPIContext c = {
        "CMPIContext",
        &cft
    };

    struct native_context * ctx =
        (struct native_context *)
        tool_mm_alloc ( mm_add, sizeof ( struct native_context ) );

    ctx->ctx       = c;
    ctx->mem_state = mm_add;

    return ctx;
}



PEGASUS_EXPORT CMPIContext * PEGASUS_CMPIR_CDECL native_new_CMPIContext (
    int mem_state )
{
    return(CMPIContext *) __new_empty_context ( mem_state );
}


PEGASUS_EXPORT void PEGASUS_CMPIR_CDECL native_release_CMPIContext (
    CONST CMPIContext * ctx )
{
    struct native_context * c = (struct native_context *) ctx;

    if (c->mem_state == TOOL_MM_NO_ADD)
    {

        c->mem_state = TOOL_MM_ADD;
        tool_mm_add ( c );

        propertyFT.release ( c->entries );
    }
}



/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
