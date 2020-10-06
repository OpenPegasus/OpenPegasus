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

#ifndef CMPI_Query2Dnf_h
#define CMPI_Query2Dnf_h

#include <Pegasus/Common/Stack.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/WQL/WQLOperation.h>
#include <Pegasus/WQL/WQLOperand.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Query/QueryCommon/SelectStatement.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

class CMPI_QueryOperand
{
public:
    /**
       When there are different types supported, please add them in here. Also
       make sure to look at the implemention of CMPI_term_el::mapType.
     */
    enum Type
    {
        NULL_TYPE,
        SINT64_TYPE,
        UINT64_TYPE,
        STRING_TYPE,
        REAL_TYPE,
        DATETIME_TYPE,
        REFERENCE_TYPE,
        PROPERTY_TYPE,
        BOOLEAN_TYPE,
        OBJECT_TYPE
    };
    ~CMPI_QueryOperand ();

    CMPI_QueryOperand ()
    {
        _type = NULL_TYPE;
    }
    CMPI_QueryOperand (const CMPI_QueryOperand & o):_type (o._type),
        _stringValue (o._stringValue)
    {
    }

    CMPI_QueryOperand (const String & x, Type type);
    Type getType () const
    {
        return _type;
    }
    String getTypeValue () const
    {
        return _stringValue;
    }

private:
    Type _type;
    String _stringValue;
};


/**
 This class is used contain the terminal elements for elements of WHERE
 query.It contains vital information such as the type of operation, the value
 in string of the left-hand and right hand side.
 */

class CMPI_term_el
{
public:
    CMPI_term_el ()
    {
        mark = false;
        op = (CMPIPredOp) 0;
    }
    CMPI_term_el (const CMPI_term_el & e)
    {
        mark = e.mark;
        op = e.op;
        opn1 = e.opn1;
        opn2 = e.opn2;
    }

    CMPI_term_el (
        Boolean m,
        CMPIPredOp o,
        CMPI_QueryOperand & op1,
        CMPI_QueryOperand & op2):
    mark (m), op (o), opn1 (op1), opn2 (op2)
    {
    }
    ~CMPI_term_el ()
    {
    }
    int toStrings (CMPIType & typ, CMPIPredOp & opr, String & o1,
        String & o2) const;
protected:
    Boolean mark;
    CMPIPredOp op;
    CMPI_QueryOperand opn1;
    CMPI_QueryOperand opn2;

    Boolean isNegative ()
    {
        return mark;
    }
private:
    CMPIType mapType (CMPI_QueryOperand::Type type) const;
};


typedef Array < CMPI_term_el > CMPI_TableauRow;

typedef Array < CMPI_TableauRow > CMPI_Tableau;


PEGASUS_NAMESPACE_END
#endif /* CMPI_Query2Dnf_h */

