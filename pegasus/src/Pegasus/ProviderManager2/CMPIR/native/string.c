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
    \file string.c
    \brief Native CMPIString implementation.

    This is the native CMPIString implementation as used for remote
    providers. It reflects the well-defined interface of a regular
    CMPIString, however, it works independently from the management broker.

    It is part of a native broker implementation that simulates CMPI data
    types rather than interacting with the entities in a full-grown CIMOM.
*/

#include <string.h>

#include "mm.h"
#include "native.h"

struct native_string
{
    CMPIString string;
    int mem_state;
};


static struct native_string * __new_string ( int, const char *, CMPIStatus * );


/*****************************************************************************/

static CMPIStatus __sft_release ( CMPIString * string )
{
    struct native_string * s = (struct native_string *) string;
    CMPIStatus rc = checkArgsReturnStatus(string);

    if (rc.rc != CMPI_RC_OK &&  s->mem_state == TOOL_MM_NO_ADD)
    {

        tool_mm_add ( s );
        tool_mm_add ( s->string.hdl );

    }

    return rc;
}


static CMPIString * __sft_clone ( CONST CMPIString * string, CMPIStatus * rc )
{
    if (!checkArgs(string, rc))
    {
        return 0;
    }

    return(CMPIString * )
    __new_string ( TOOL_MM_NO_ADD,
        string->ft->getCharPtr ( string, rc ),
        rc );
}


static const char * __sft_getCharPtr(
    CONST CMPIString * string,
    CMPIStatus * rc)
{
    if (!checkArgs(string, rc))
    {
        return 0;
    }

    return(char *) string->hdl;
}


static struct native_string * __new_string (
    int mm_add,
    const char * ptr,
    CMPIStatus * rc )
{
    static CMPIStringFT sft = {
        NATIVE_FT_VERSION,
        __sft_release,
        __sft_clone,
        __sft_getCharPtr
    };

    struct native_string * string =
        (struct native_string *)
    tool_mm_alloc ( mm_add, sizeof ( struct native_string ) );

    string->string.hdl = ( ptr )? strdup ( ptr ): NULL;
    string->string.ft  = &sft;
    string->mem_state  = mm_add;

    if (mm_add == TOOL_MM_ADD)
    {
        tool_mm_add ( string->string.hdl );
    }

    CMSetStatus ( rc, CMPI_RC_OK );
    return string;
}


CMPIString * native_new_CMPIString ( const char * ptr, CMPIStatus * rc )
{
    return(CMPIString *) __new_string ( TOOL_MM_ADD, ptr, rc );
}


/****************************************************************************/

/*** Local Variables:  ***/
/*** mode: C           ***/
/*** c-basic-offset: 8 ***/
/*** End:              ***/
