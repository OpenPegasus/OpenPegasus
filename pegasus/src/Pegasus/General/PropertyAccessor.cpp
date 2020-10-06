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

#include "PropertyAccessor.h"

PEGASUS_NAMESPACE_BEGIN

/* template for local getter for value in named property of an instance.
   param inst CIMInstance target.
   @param name String name of property
   @param x Value from Template
   @return returns True if Property value exits. Else
   returns false if property value is null
*/
template<class T>
bool _Get(
    const CIMInstance& inst,
    const String& name,
    T& x)
{
    bool isNull;
    Uint32 pos = inst.findProperty(name);
    const CIMValue& value = inst.getProperty(pos).getValue();

    if ((isNull = value.isNull()))
        x = T();
    else
        value.get(x);
    return !isNull;
}

/* template for local setter for value in named property of an instance.
   param inst CIMInstance target.
   @param name String name of property
   @param x value to be set. Template representing the value type
   @return returns True if Property value exits. Else
   returns false if property value is null
*/
template<class T>
void _Set(
    CIMInstance& inst,
    const String& name,
    const T& x,
    bool null)
{
    Uint32 pos = inst.findProperty(name);
    CIMValue value;

    value.set(x);
    if (null)
        value.setNullValue(GetType((T*)0), IsArray((T*)0));

    inst.getProperty(pos).setValue(value);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Boolean& value)
{
    return _Get(inst, name, value);
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Boolean& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Boolean>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Boolean>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Uint8& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Uint8& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Uint8>& value)
{
    return _Get(inst, name, value);
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Uint8>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Sint8& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Sint8& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Sint8>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Sint8>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Uint16& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Uint16& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Uint16>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Uint16>& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Sint16& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Sint16& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Sint16>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Sint16>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Uint32& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Uint32& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Uint32>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Uint32>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Sint32& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Sint32& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Sint32>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Sint32>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Uint64& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Uint64& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Uint64>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Uint64>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Sint64& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Sint64& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Sint64>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Sint64>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Real32& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Real32& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Real32>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Real32>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Real64& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Real64& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Real64>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Real64>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    Char16& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Char16& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<Char16>& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<Char16>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    String& value)
{
    return _Get(inst, name, value);;
}

void Set(
    CIMInstance& inst,
    const String& name,
    const String& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<String>& value)
{
    return _Get(inst, name, value);
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<String>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
bool Get(
    const CIMInstance& inst,
    const String& name,
    CIMDateTime& value)
{
    return _Get(inst, name, value);
}

void Set(
    CIMInstance& inst,
    const String& name,
    const CIMDateTime& value,
    bool null)
{
    _Set(inst, name, value, null);
}

bool Get(
    const CIMInstance& inst,
    const String& name,
    Array<CIMDateTime>& value)
{
    return _Get(inst, name, value);
}

void Set(
    CIMInstance& inst,
    const String& name,
    const Array<CIMDateTime>& value,
    bool null)
{
    _Set(inst, name, value, null);
}
PEGASUS_NAMESPACE_END

