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

#include "CIMParamValueRep.h"
#include "CIMParamValue.h"

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMParamValue
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T


CIMParamValue::CIMParamValue()
    : _rep(0)
{
}

CIMParamValue::CIMParamValue(const CIMParamValue& x)
{
    _rep = x._rep;
    if (_rep)
        _rep->Inc();
}

CIMParamValue::CIMParamValue(CIMParamValueRep* rep)
    : _rep(rep)
{
}

CIMParamValue& CIMParamValue::operator=(const CIMParamValue& x)
{
    if (x._rep != _rep)
    {
        if (_rep)
            _rep->Dec();
        _rep = x._rep;
        if (_rep)
            _rep->Inc();
    }
    return *this;
}

CIMParamValue::CIMParamValue(
    String parameterName,
    CIMValue value,
    Boolean isTyped)
{
    _rep = new CIMParamValueRep(parameterName, value, isTyped);
}

CIMParamValue::~CIMParamValue()
{
    if (_rep)
        _rep->Dec();
}

String CIMParamValue::getParameterName() const
{
    CheckRep(_rep);
    return _rep->getParameterName();
}

CIMValue CIMParamValue::getValue() const
{
    CheckRep(_rep);
    return _rep->getValue();
}

Boolean CIMParamValue::isTyped() const
{
    CheckRep(_rep);
    return _rep->isTyped();
}

void CIMParamValue::setParameterName(String& parameterName)
{
    CheckRep(_rep);
    _rep->setParameterName(parameterName);
}

void CIMParamValue::setValue(CIMValue& value)
{
    CheckRep(_rep);
    _rep->setValue(value);
}

void CIMParamValue::setIsTyped(Boolean isTyped)
{
    CheckRep(_rep);
    _rep->setIsTyped(isTyped);
}

Boolean CIMParamValue::isUninitialized() const
{
    return _rep == 0;
}

CIMParamValue CIMParamValue::clone() const
{
    CheckRep(_rep);
    return CIMParamValue(_rep->clone());
}

PEGASUS_NAMESPACE_END
