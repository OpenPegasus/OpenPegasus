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

#include <Pegasus/Common/Config.h>
#include <cstdio>
#include "CIMParamValueRep.h"
#include "StrLit.h"

PEGASUS_NAMESPACE_BEGIN

CIMParamValueRep::CIMParamValueRep(
    String parameterName,
    CIMValue value,
    Boolean isTyped)
    : _parameterName(parameterName), _value(value), _isTyped(isTyped),
      _refCounter(1)
{
    // ensure parameterName is not null
    if (parameterName.size() == 0)
    {
        throw UninitializedObjectException();
    }
}

CIMParamValueRep::CIMParamValueRep(const CIMParamValueRep& x) :
    _parameterName(x._parameterName),
    _value(x._value),
    _isTyped(x._isTyped),
    _refCounter(1)
{
}

void CIMParamValueRep::setParameterName(String& parameterName)
{
    // ensure parameterName is not null
    if (parameterName.size() == 0)
    {
        throw UninitializedObjectException();
    }

    _parameterName = parameterName;
}

void CIMParamValueRep::setValue(CIMValue& value)
{
    _value = value;
}

void CIMParamValueRep::setIsTyped(Boolean isTyped)
{
    _isTyped = isTyped;
}

PEGASUS_NAMESPACE_END
