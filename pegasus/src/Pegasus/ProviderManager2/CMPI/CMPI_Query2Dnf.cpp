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
#include "CMPI_Query2Dnf.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

CMPI_QueryOperand::~CMPI_QueryOperand ()
{
    _type = NULL_TYPE;
}

CMPI_QueryOperand::CMPI_QueryOperand (const String & x, Type type)
{
    _type = type;
    _stringValue = String (x);
}

int CMPI_term_el::toStrings (
    CMPIType & typ,
    CMPIPredOp & opr, String & o1,String & o2) const
{

    opr = op;
    o1 = opn1.getTypeValue ();
    o2 = opn2.getTypeValue ();
    if (opn1.getType () == CMPI_QueryOperand::PROPERTY_TYPE)
        typ = mapType (opn2.getType ());
    else
        typ = mapType (opn1.getType ());
    return 0;
}
CMPIType
    CMPI_term_el::mapType (CMPI_QueryOperand::Type type) const
{

    switch (type)
    {
        case CMPI_QueryOperand::NULL_TYPE:
            return CMPI_null;
        case CMPI_QueryOperand::SINT64_TYPE:
            return CMPI_sint64;
        case CMPI_QueryOperand::UINT64_TYPE:
            return CMPI_uint64;
        case CMPI_QueryOperand::STRING_TYPE:
            return CMPI_string;
        case CMPI_QueryOperand::DATETIME_TYPE:
            return CMPI_dateTime;
        case CMPI_QueryOperand::REFERENCE_TYPE:
            return CMPI_ref;
        case CMPI_QueryOperand::PROPERTY_TYPE:
            return CMPI_nameString;
        case CMPI_QueryOperand::BOOLEAN_TYPE:
            return CMPI_boolean;
        case CMPI_QueryOperand::OBJECT_TYPE:
            return CMPI_nameString;
        case CMPI_QueryOperand::REAL_TYPE:
            return CMPI_REAL;
        default:
            break;
    }
    return CMPI_null;
}

PEGASUS_NAMESPACE_END
