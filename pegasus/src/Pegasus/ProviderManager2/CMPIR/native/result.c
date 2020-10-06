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
    \file result.c
    \brief Native CMPIResult implementation.

    This is the native CMPIResult implementation as used for remote
    providers. It reflects the well-defined interface of a regular
    CMPIResult, however, it works independently from the management broker.

    It is part of a native broker implementation that simulates CMPI data
    types rather than interacting with the entities in a full-grown CIMOM.

*/

#include "cmpir_common.h"
#include "mm.h"
#include "native.h"


struct native_result
{
    CMPIResult result;
    int mem_state;

    CMPICount current;
    CMPIArray * data;
};


static struct native_result * __new_empty_result ( int, CMPIStatus * );


/*****************************************************************************/

static CMPIStatus __rft_release ( CMPIResult * result )
{
    struct native_result * r = (struct native_result *) result;
    CMPIStatus rc = checkArgsReturnStatus(result);

    if (rc.rc == CMPI_RC_OK && r->mem_state == TOOL_MM_NO_ADD)
    {
        tool_mm_add ( result );
        return r->data->ft->release ( r->data );
    }

    return rc;
}


static CMPIResult * __rft_clone ( CONST CMPIResult * result, CMPIStatus * rc )
{
    CMPIArray * a;
    struct native_result * r;

    if (!checkArgs(result, rc))
    {
        return 0;
    }

    a  = ( (struct native_result *) result )->data;
    r = __new_empty_result ( TOOL_MM_NO_ADD, rc );

    if (rc && rc->rc != CMPI_RC_OK)
    {
        return NULL;
    }

    r->data = a->ft->clone ( a, rc );

    return(CMPIResult *) r;
}


static CMPIStatus __rft_returnData (
    CONST CMPIResult * result,
    const CMPIValue * val,
    CONST CMPIType type )
{
    struct native_result * r = (struct native_result *) result;

    CMPIStatus rc = checkArgsReturnStatus(result);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    if (!val)
    {
        CMReturn (CMPI_RC_ERR_INVALID_PARAMETER);
    }


    if (r->current == 0)
    {

        CMPIStatus rc;
        r->data = native_new_CMPIArray ( 1, type, &rc );
        if (rc.rc != CMPI_RC_OK) return rc;

    }
    else
    {
        native_array_increase_size ( r->data, 1 );
    }


    return CMSetArrayElementAt (
        r->data,
        r->current++,
        val,
        type );
}


static CMPIStatus __rft_returnInstance (
    CONST CMPIResult * result,
    CONST CMPIInstance * instance )
{
    CMPIValue v;
    CMPIStatus rc = checkArgsReturnStatus(result);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    v.inst = (CMPIInstance *)instance;

    return __rft_returnData ( result, &v, CMPI_instance );
}


static CMPIStatus __rft_returnObjectPath (
    CONST CMPIResult * result,
    CONST CMPIObjectPath * cop )
{
    CMPIValue v;
    CMPIStatus rc = checkArgsReturnStatus(result);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    v.ref = (CMPIObjectPath *)cop;

    return __rft_returnData ( result, &v, CMPI_ref );
}


static CMPIStatus __rft_returnDone ( CONST CMPIResult * result )
{
    CMPIStatus rc = checkArgsReturnStatus(result);
    return rc;
}

#ifdef CMPI_VER_200
CMPIStatus __rft_returnError(CONST CMPIResult* rslt, CONST CMPIError* er)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}
#endif

static struct native_result * __new_empty_result (
    int mm_add,
    CMPIStatus * rc )
{
    static CMPIResultFT rft = {
        NATIVE_FT_VERSION,
        __rft_release,
        __rft_clone,
        __rft_returnData,
        __rft_returnInstance,
        __rft_returnObjectPath,
        __rft_returnDone,
#ifdef CMPI_VER_200
        __rft_returnError
#endif
    };
    static CMPIResult r = {
        "CMPIResult",
        &rft
    };

    struct native_result * result =
        (struct native_result *)
        tool_mm_alloc ( mm_add, sizeof ( struct native_result ) );

    result->result    = r;
    result->mem_state = mm_add;

    CMSetStatus ( rc, CMPI_RC_OK );
    return result;
}


PEGASUS_EXPORT CMPIResult *PEGASUS_CMPIR_CDECL native_new_CMPIResult (
    CMPIStatus * rc )
{
    return(CMPIResult *) __new_empty_result ( TOOL_MM_ADD, rc );
}


PEGASUS_EXPORT CMPIArray * PEGASUS_CMPIR_CDECL native_result2array (
    CMPIResult * result )
{
    struct native_result * r = (struct native_result *) result;

    return r->data;
}


/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
