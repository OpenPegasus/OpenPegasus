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
//%////////////////////////////////////////////////////////////////////////////

#include "Attribute.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

Attribute::Attribute(const String& attrEntry)
{
    _tag = attrEntry;
}

Attribute::~Attribute()
{
    if (_vals.size())
    {
        _vals.clear();
    }
}

const Array<String>& Attribute::getValues() const
{
    return _vals;
}

const String& Attribute::getTag() const
{
    return _tag;
}

void Attribute::addValue(const String& value)
{
    _vals.append(value);
}

PEGASUS_STD(ostream)& operator<<(
    PEGASUS_STD(ostream)& os,
    const Attribute& attr)
{
    os << attr.getTag();
    Array<String> vals = attr.getValues();
    for (Uint32 i = 0; i < vals.size(); i++)
    {
        if (i == 0)
        {
            os << " = ";
        }
        else
        {
            os << " | ";
        }
        os << vals[i];
    }
    return os;
}

#define PEGASUS_ARRAY_T Attribute
#include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END
