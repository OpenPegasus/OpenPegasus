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

#ifndef _CMPI_Value_H_
#define _CMPI_Value_H_

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMObjectPath.h>

PEGASUS_NAMESPACE_BEGIN


CIMValue value2CIMValue(const CMPIValue* data,const CMPIType type, CMPIrc *rc);
SCMBUnion value2SCMOValue(
    const CMPIValue* data,
    const CMPIType type,
    Boolean &nullValue);

CMPIrc value2CMPIData(const CIMValue&,CMPIType,CMPIData *data);
CMPIType type2CMPIType(CIMType pt, int array);
//Function to convert CMPIType to CIMType
inline CIMType type2CIMType(CMPIType pt)
{
    switch( pt )
    {
        case CMPI_null:
            return(CIMType)0;

        case CMPI_boolean:
            return CIMTYPE_BOOLEAN;

        case CMPI_char16:
            return CIMTYPE_CHAR16;

        case CMPI_real32:
            return CIMTYPE_REAL32;

        case CMPI_real64:
            return CIMTYPE_REAL64;

        case CMPI_uint8:
            return CIMTYPE_UINT8;

        case CMPI_uint16:
            return CIMTYPE_UINT16;

        case CMPI_uint32:
            return CIMTYPE_UINT32;

        case CMPI_uint64:
            return CIMTYPE_UINT64;

        case CMPI_sint8:
            return CIMTYPE_SINT8;

        case CMPI_sint16:
            return CIMTYPE_SINT16;

        case CMPI_sint32:
            return CIMTYPE_SINT32;

        case CMPI_sint64:
            return CIMTYPE_SINT64;

        case CMPI_string:
            return CIMTYPE_STRING;

        case CMPI_chars:
            return CIMTYPE_STRING;

        case CMPI_charsptr:
            return CIMTYPE_STRING;

        case CMPI_dateTime:
            return CIMTYPE_DATETIME;

        case CMPI_ref:
            return CIMTYPE_REFERENCE;
        case CMPI_instance:
            return CIMTYPE_INSTANCE;
        default:
            return(CIMType)0;
    }
}

PEGASUS_NAMESPACE_END

#endif
