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
    \file array.c
    \brief Native CMPIArray implementation.

    This is the native CMPIArray implementation as used for remote
    providers. It reflects the well-defined interface of a regular CMPIArray
    object, however, it works independently from the management broker.

    It is part of a native broker implementation that simulates CMPI data
    types rather than interacting with the entities in a full-grown CIMOM.

    In contrast to a regular array, there exists an additional increase()
    method, which is only used by the native CMPIResult implementation to
    grow an array stepwise.

    \author Frank Scheffler
*/

#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "native.h"


struct native_array_item
{
    CMPIValueState state;
    CMPIValue      value;
};


struct native_array
{
    CMPIArray array;
    int mem_state;

    CMPICount size;
    CMPIType type;
    struct native_array_item * data;
};


static struct native_array * __new_empty_array (
    int, CMPICount,
    CMPIType,
    CMPIStatus * );


/*****************************************************************************/

static void __make_NULL (
    struct native_array * a,
    int from,
    int to,
    int release )
{
    for (; from <= to; from++)
    {
        a->data[from].state = CMPI_nullValue;

        if (release)
        {
            native_release_CMPIValue ( a->type,
                &a->data[from].value );
        }
    }
}


static CMPIStatus __aft_release ( CMPIArray * array )
{
    struct native_array * a = (struct native_array *) array;
    CMPIStatus rc = checkArgsReturnStatus(array);

    if (rc.rc == CMPI_RC_OK && a->mem_state == TOOL_MM_NO_ADD)
    {

        int i = a->size;

        tool_mm_add ( a );
        tool_mm_add ( a->data );

        while (i--)
        {
            if (! ( a->data[i].state & CMPI_nullValue ))
            {
                native_release_CMPIValue ( a->type,
                    &a->data[i].value );
            }
        }

    }

    return rc;
}



static CMPIArray * __aft_clone ( CONST CMPIArray * array, CMPIStatus * rc )
{
    CMPIStatus tmp;
    struct native_array * a   = (struct native_array *) array;
    struct native_array * new;
    int i;

    if (!checkArgs(array, rc))
    {
        return 0;
    }
    new  = __new_empty_array (
        TOOL_MM_NO_ADD,
        a->size,
        a->type,
        &tmp );
    i = a->size;

    while (i-- && tmp.rc == CMPI_RC_OK)
    {
        new->data[i].state = a->data[i].state;
        if (! ( new->data[i].state & CMPI_nullValue ))
        {

            new->data[i].value =
                native_clone_CMPIValue ( a->type,
                &a->data[i].value,
                &tmp );
        }
    }

    CMSetStatus ( rc, tmp.rc );

    return(CMPIArray *) new;
}


static CMPICount __aft_getSize ( CONST CMPIArray * array, CMPIStatus * rc )
{
    struct native_array * a = (struct native_array *) array;
    if (!checkArgs(array, rc))
    {
        return 0;
    }

    CMSetStatus ( rc, CMPI_RC_OK );
    return a->size;
}


static CMPIType __aft_getSimpleType (CONST CMPIArray * array, CMPIStatus * rc)
{
    struct native_array * a = (struct native_array *) array;
    if (!checkArgs(array, rc))
    {
        return 0;
    }

    CMSetStatus ( rc, CMPI_RC_OK );
    return a->type;
}


static CMPIData __aft_getElementAt (
    CONST CMPIArray * array,
    CMPICount index,
    CMPIStatus * rc )
{

    struct native_array * a = (struct native_array *) array;
    CMPIData result = checkArgsReturnData(array, rc);

    if (result.state == CMPI_badValue)
    {
        return result;
    }

    result.type = a->type;
    result.state = CMPI_badValue;

    if (index < a->size)
    {
        result.state = a->data[index].state;
        result.value = a->data[index].value;
        CMSetStatus ( rc, CMPI_RC_OK );
    }
    else
    {
        CMSetStatus (rc, CMPI_RC_ERR_NO_SUCH_PROPERTY);
    }

    return result;
}


static CMPIStatus __aft_setElementAt (
    CMPIArray * array,
    CMPICount index,
    CONST CMPIValue * val,
    CMPIType type )
{

    struct native_array * a = (struct native_array *) array;
    CMPIStatus rc = checkArgsReturnStatus(array);

    if (!val)
    {
        CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
    }

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    if (index < a->size)
    {
        CMPIValue v;

        if (type == CMPI_chars && a->type == CMPI_string)
        {

            v.string = native_new_CMPIString ((char*)val, NULL );
            type = CMPI_string;
            val  = &v;
        }

        if (type == a->type)
        {

            CMPIStatus rc = {CMPI_RC_OK, NULL};

            a->data[index].state = 0;
            a->data[index].value =
                ( a->mem_state == TOOL_MM_ADD )?
                *val:
                native_clone_CMPIValue ( type, val, &rc );

            return rc;
        }

        if (type == CMPI_null)
        {

            if (! ( a->data[index].state & CMPI_nullValue ))
            {

                __make_NULL ( a,
                    index,
                    index,
                    a->mem_state == TOOL_MM_NO_ADD );
            }

            CMReturn ( CMPI_RC_OK );
        }
    }

    CMReturn ( CMPI_RC_ERR_TYPE_MISMATCH );
}



void native_array_increase_size (
    CMPIArray * array,
    CMPICount increment )
{
    struct native_array * a = (struct native_array *) array;

    a->data =
        (struct native_array_item *)
        tool_mm_realloc ( a->data,
        ( a->size + increment ) *
        sizeof ( struct native_array_item ) );
    memset (
        &a->data[a->size],
        0,
        sizeof ( struct native_array_item ) * increment );

    a->size += increment;
}


static struct native_array * __new_empty_array (
    int mm_add,
    CMPICount size,
    CMPIType type,
    CMPIStatus * rc )
{
    static CMPIArrayFT aft = {
        NATIVE_FT_VERSION,
        __aft_release,
        __aft_clone,
        __aft_getSize,
        __aft_getSimpleType,
        __aft_getElementAt,
        __aft_setElementAt
    };
    static CMPIArray a = {
        "CMPIArray",
        &aft
    };

    struct native_array * array =
        (struct native_array *)
        tool_mm_alloc ( mm_add, sizeof ( struct native_array ) );

    array->array     = a;
    array->mem_state = mm_add;

    type &= ~CMPI_ARRAY;
    array->type  = ( type == CMPI_chars )? CMPI_string: type;
    array->size  = size;
    array->data  =
        (struct native_array_item *)
        tool_mm_alloc ( mm_add,
        size * sizeof ( struct native_array_item ) );

    __make_NULL ( array, 0, size - 1, 0 );

    CMSetStatus ( rc, CMPI_RC_OK );
    return array;
}


CMPIArray * native_new_CMPIArray (
    CMPICount size,
    CMPIType type,
    CMPIStatus * rc )
{
    return(CMPIArray *) __new_empty_array ( TOOL_MM_ADD, size, type, rc );
}


/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
