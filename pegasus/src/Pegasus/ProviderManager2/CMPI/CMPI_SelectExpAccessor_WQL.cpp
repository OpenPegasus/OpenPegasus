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

#include "CMPI_Version.h"

#include "CMPI_SelectExpAccessor_WQL.h"
#include <Pegasus/Provider/CMPI/cmpimacs.h>
PEGASUS_NAMESPACE_BEGIN

CMPI_SelectExpAccessor_WQL::CMPI_SelectExpAccessor_WQL(
    CMPIAccessor *acc,
    void *parm): accessor(acc), accParm(parm)
{

}

Boolean CMPI_SelectExpAccessor_WQL::getValue(
    const CIMName& propertyName,
    WQLOperand& value) const
{

    CMPIAccessor* get=(CMPIAccessor*) accessor;
    CMPIData data=get(propertyName.getString().getCString(),accParm);
    Boolean rv=true;

    if( data.type & CMPI_ARRAY )
    {
        rv=false;
    }
    else
    if( (data.type & (CMPI_UINT|CMPI_SINT))==CMPI_SINT )
    {
        switch( data.type )
        {
            case CMPI_sint32:
                value=WQLOperand(data.value.sint32, WQL_INTEGER_VALUE_TAG);
                break;
            case CMPI_sint16:
                value=WQLOperand(data.value.sint16, WQL_INTEGER_VALUE_TAG);
                break;
            case CMPI_sint8:
                value=WQLOperand(data.value.sint8, WQL_INTEGER_VALUE_TAG);
                break;
            case CMPI_sint64:
                value=WQLOperand(data.value.sint64, WQL_INTEGER_VALUE_TAG);
                break;
            default:
                rv=false;
        }
    }
    else
    if( data.type==CMPI_chars )
    {
        value=WQLOperand(String(data.value.chars), WQL_STRING_VALUE_TAG);
    }
    else
    if( data.type==CMPI_string )
    {
        value=WQLOperand(String(CMGetCharsPtr(data.value.string,NULL)),
        WQL_STRING_VALUE_TAG);
    }
    else
    if( (data.type & (CMPI_UINT|CMPI_SINT))==CMPI_UINT )
    {
        switch( data.type )
        {
            case CMPI_uint32:
                value=WQLOperand(data.value.sint32, WQL_INTEGER_VALUE_TAG);
                break;
            case CMPI_uint16:
                value=WQLOperand(data.value.sint16, WQL_INTEGER_VALUE_TAG);
                break;
            case CMPI_uint8:
                value=WQLOperand(data.value.sint8, WQL_INTEGER_VALUE_TAG);
                break;
            case CMPI_uint64:
                value=WQLOperand(data.value.sint64, WQL_INTEGER_VALUE_TAG);
                break;
            default:
                rv=false;
        }
    }
    else
    {    switch( data.type )
        {
            case CMPI_boolean:
                value=WQLOperand((Boolean)data.value.boolean,
                WQL_BOOLEAN_VALUE_TAG);
                break;
            case CMPI_real32:
                value=WQLOperand(data.value.real32, WQL_DOUBLE_VALUE_TAG);
                break;
            case CMPI_real64:
                value=WQLOperand(data.value.real64, WQL_DOUBLE_VALUE_TAG);
                break;
            default:
                rv=false;
        }
    }
    return rv;
}

PEGASUS_NAMESPACE_END

