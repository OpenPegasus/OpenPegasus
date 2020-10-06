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

#include <cstring>
#include <cstdio>
#include <cctype>
#include "CIMValue.h"
#include "CIMInstance.h"
#include "Union.h"
#include "XmlWriter.h"
#include "CIMValueRep.h"
#include "Config.h"
#include "CIMType.h"
#include "String.h"
#include "CIMDateTime.h"
#include "CIMObjectPath.h"
#include "CIMObject.h"
#include "Array.h"
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMValue
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

// ATTN: By getting a CIMObject from a CIMValue, the client of CIMValue can
// modify the internals of that CIMObject and thus change what CIMValue
// itself refers to. There are two solutions: clone() at ever juncture or
// force CIMValue to make its own unique copy when the client calls get()
// to get a CIMObject.

//==============================================================================
//
// CIMValueRep
//
//==============================================================================

CIMValueRep CIMValueRep::_emptyRep((int*)0);

void CIMValueRep::release()
{
    if (isArray)
    {
        switch (type)
        {
            case CIMTYPE_BOOLEAN:
                CIMValueType<Boolean>::destructArray(this);
                break;

            case CIMTYPE_UINT8:
                CIMValueType<Uint8>::destructArray(this);
                break;

            case CIMTYPE_SINT8:
                CIMValueType<Sint8>::destructArray(this);
                break;

            case CIMTYPE_UINT16:
                CIMValueType<Uint16>::destructArray(this);
                break;

            case CIMTYPE_SINT16:
                CIMValueType<Sint16>::destructArray(this);
                break;

            case CIMTYPE_UINT32:
                CIMValueType<Uint32>::destructArray(this);
                break;

            case CIMTYPE_SINT32:
                CIMValueType<Sint32>::destructArray(this);
                break;

            case CIMTYPE_UINT64:
                CIMValueType<Uint64>::destructArray(this);
                break;

            case CIMTYPE_SINT64:
                CIMValueType<Sint64>::destructArray(this);
                break;

            case CIMTYPE_REAL32:
                CIMValueType<Real32>::destructArray(this);
                break;

            case CIMTYPE_REAL64:
                CIMValueType<Real64>::destructArray(this);
                break;

            case CIMTYPE_CHAR16:
                CIMValueType<Char16>::destructArray(this);
                break;

            case CIMTYPE_STRING:
                CIMValueType<String>::destructArray(this);
                break;

            case CIMTYPE_DATETIME:
                CIMValueType<CIMDateTime>::destructArray(this);
                break;

            case CIMTYPE_REFERENCE:
                CIMValueType<CIMObjectPath>::destructArray(this);
                break;

            case CIMTYPE_OBJECT:
                CIMValueType<CIMObject>::destructArray(this);
                break;
            case CIMTYPE_INSTANCE:
                CIMValueType<CIMInstance>::destructArray(this);
                break;
        }
    }
    else
    {
        switch (type)
        {
            case CIMTYPE_BOOLEAN:
            case CIMTYPE_UINT8:
            case CIMTYPE_SINT8:
            case CIMTYPE_UINT16:
            case CIMTYPE_SINT16:
            case CIMTYPE_UINT32:
            case CIMTYPE_SINT32:
            case CIMTYPE_UINT64:
            case CIMTYPE_SINT64:
            case CIMTYPE_REAL32:
            case CIMTYPE_REAL64:
            case CIMTYPE_CHAR16:
                break;

            case CIMTYPE_STRING:
                CIMValueType<String>::destruct(this);
                break;

            case CIMTYPE_DATETIME:
                CIMValueType<CIMDateTime>::destruct(this);
                break;

            case CIMTYPE_REFERENCE:
                CIMValueType<CIMObjectPath>::destruct(this);
                break;

            case CIMTYPE_OBJECT:
                CIMValueType<CIMObject>::destruct(this);
                break;
            case CIMTYPE_INSTANCE:
                CIMValueType<CIMInstance>::destruct(this);
                break;
        }
    }
}

//==============================================================================
//
// CIMValue
//
//==============================================================================

static inline void _release(CIMValueRep*& rep)
{
    if (rep->refs.get() == 1)
        rep->release();
    else
    {
        CIMValueRep::unref(rep);
        rep = new CIMValueRep;
    }
}

CIMValue::CIMValue(CIMType type, Boolean isArray, Uint32 arraySize)
{
    _rep = new CIMValueRep;

    switch (type)
    {
        case CIMTYPE_BOOLEAN:
            CIMValueType<Boolean>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_UINT8:
            CIMValueType<Uint8>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_SINT8:
            CIMValueType<Sint8>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_UINT16:
            CIMValueType<Uint16>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_SINT16:
            CIMValueType<Sint16>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_UINT32:
            CIMValueType<Uint32>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_SINT32:
            CIMValueType<Sint32>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_UINT64:
            CIMValueType<Uint64>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_SINT64:
            CIMValueType<Sint64>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_REAL32:
            CIMValueType<Real32>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_REAL64:
            CIMValueType<Real64>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_CHAR16:
            CIMValueType<Char16>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_STRING:
            CIMValueType<String>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_DATETIME:
            CIMValueType<CIMDateTime>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_REFERENCE:
            CIMValueType<CIMObjectPath>::setNull(_rep, type, isArray,arraySize);
            break;

        case CIMTYPE_OBJECT:
            CIMValueType<CIMObject>::setNull(_rep, type, isArray, arraySize);
            break;
        case CIMTYPE_INSTANCE:
            CIMValueType<CIMInstance>::setNull(_rep, type, isArray, arraySize);
            break;

        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }
}

CIMValue::CIMValue(Boolean x)
{
    _rep = new CIMValueRep;
    CIMValueType<Boolean>::set(_rep, x);
}

CIMValue::CIMValue(Uint8 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Uint8>::set(_rep, x);
}

CIMValue::CIMValue(Sint8 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint8>::set(_rep, x);
}

CIMValue::CIMValue(Uint16 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Uint16>::set(_rep, x);
}

CIMValue::CIMValue(Sint16 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint16>::set(_rep, x);
}

CIMValue::CIMValue(Uint32 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Uint32>::set(_rep, x);
}

CIMValue::CIMValue(Sint32 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint32>::set(_rep, x);
}

CIMValue::CIMValue(Uint64 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Uint64>::set(_rep, x);
}

CIMValue::CIMValue(Sint64 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint64>::set(_rep, x);
}

CIMValue::CIMValue(Real32 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Real32>::set(_rep, x);
}

CIMValue::CIMValue(Real64 x)
{
    _rep = new CIMValueRep;
    CIMValueType<Real64>::set(_rep, x);
}

CIMValue::CIMValue(const Char16& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Char16>::set(_rep, x);
}

CIMValue::CIMValue(const String& x)
{
    _rep = new CIMValueRep;
    CIMValueType<String>::set(_rep, x);
}

CIMValue::CIMValue(const CIMDateTime& x)
{
    _rep = new CIMValueRep;
    CIMValueType<CIMDateTime>::set(_rep, x);
}

CIMValue::CIMValue(const CIMObjectPath& x)
{
    _rep = new CIMValueRep;
    CIMValueType<CIMObjectPath>::set(_rep, x);
}

CIMValue::CIMValue(const CIMObject& x)
{
    if (x.isUninitialized())
    {
        // Bug 3373, throw exception if uninitialized object is passed to set().
        throw UninitializedObjectException();
    }

    _rep = new CIMValueRep;
    CIMValueType<CIMObject>::set(_rep, x.clone());
}
CIMValue::CIMValue(const CIMInstance& x)
{
    if (x.isUninitialized())
    {
        // Bug 3373, throw exception if uninitialized object is passed to set().
        throw UninitializedObjectException();
    }

    _rep = new CIMValueRep;
    CIMValueType<CIMInstance>::set(_rep, x.clone());
}
CIMValue::CIMValue(const Array<Boolean>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Boolean>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Uint8>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Uint8>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Sint8>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint8>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Uint16>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Uint16>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Sint16>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint16>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Uint32>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Uint32>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Sint32>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint32>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Uint64>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Uint64>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Sint64>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint64>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Real32>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Real32>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Real64>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Real64>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<Char16>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<Char16>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<String>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<String>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<CIMDateTime>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<CIMDateTime>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<CIMObjectPath>& x)
{
    _rep = new CIMValueRep;
    CIMValueType<CIMObjectPath>::setArray(_rep, x);
}

CIMValue::CIMValue(const Array<CIMObject>& x)
{
    Array<CIMObject> tmp;

    for (Uint32 i = 0, n = x.size(); i < n; i++)
    {
        if (x[i].isUninitialized())
        {
            // Bug 3373, throw exception on uninitialized object.
            _rep = &CIMValueRep::_emptyRep;
            throw UninitializedObjectException();
        }

        tmp.append(x[i].clone());
    }

    _rep = new CIMValueRep;
    CIMValueType<CIMObject>::setArray(_rep, tmp);
}

CIMValue::CIMValue(const Array<CIMInstance>& x)
{
    Array<CIMInstance> tmp;

    for (Uint32 i = 0, n = x.size(); i < n; i++)
    {
        if (x[i].isUninitialized())
        {
          // Bug 3373, throw exception on uninitialized object.
          _rep = &CIMValueRep::_emptyRep;
          throw UninitializedObjectException();
        }

        tmp.append(x[i].clone());
    }
    _rep = new CIMValueRep;
    CIMValueType<CIMInstance>::setArray(_rep, tmp);
}

void CIMValue::clear()
{
    CIMValueRep::unref(_rep);
    _rep = &CIMValueRep::_emptyRep;
}

CIMValue& CIMValue::operator=(const CIMValue& x)
{
    if (_rep != x._rep)
    {
        CIMValueRep::unref(_rep);
        CIMValueRep::ref(_rep = x._rep);
    }

    return *this;
}

void CIMValue::assign(const CIMValue& x)
{
    if (_rep != x._rep)
    {
        CIMValueRep::unref(_rep);
        CIMValueRep::ref(_rep = x._rep);
    }
}

Boolean CIMValue::typeCompatible(const CIMValue& x) const
{
    return (_rep->type == x._rep->type && _rep->isArray == x._rep->isArray);
}

Uint32 CIMValue::getArraySize() const
{
    if (!_rep->isArray)
        return 0;

    switch (_rep->type)
    {
        case CIMTYPE_BOOLEAN:
            return CIMValueType<Boolean>::arraySize(_rep);

        case CIMTYPE_UINT8:
            return CIMValueType<Uint8>::arraySize(_rep);

        case CIMTYPE_SINT8:
            return CIMValueType<Sint8>::arraySize(_rep);

        case CIMTYPE_UINT16:
            return CIMValueType<Uint16>::arraySize(_rep);

        case CIMTYPE_SINT16:
            return CIMValueType<Sint16>::arraySize(_rep);

        case CIMTYPE_UINT32:
            return CIMValueType<Uint32>::arraySize(_rep);

        case CIMTYPE_SINT32:
            return CIMValueType<Sint32>::arraySize(_rep);

        case CIMTYPE_UINT64:
            return CIMValueType<Uint64>::arraySize(_rep);

        case CIMTYPE_SINT64:
            return CIMValueType<Sint64>::arraySize(_rep);

        case CIMTYPE_REAL32:
            return CIMValueType<Real32>::arraySize(_rep);

        case CIMTYPE_REAL64:
            return CIMValueType<Real64>::arraySize(_rep);

        case CIMTYPE_CHAR16:
            return CIMValueType<Char16>::arraySize(_rep);

        case CIMTYPE_STRING:
            return CIMValueType<String>::arraySize(_rep);

        case CIMTYPE_DATETIME:
            return CIMValueType<CIMDateTime>::arraySize(_rep);

        case CIMTYPE_REFERENCE:
            return CIMValueType<CIMObjectPath>::arraySize(_rep);

        case CIMTYPE_OBJECT:
            return CIMValueType<CIMObject>::arraySize(_rep);
        case CIMTYPE_INSTANCE:
        return CIMValueType<CIMInstance>::arraySize(_rep);
        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }

    return 0;
}

void CIMValue::setNullValue(CIMType type, Boolean isArray, Uint32 arraySize)
{
    //
    // Release any memory:
    //

    _release(_rep);

    //
    // Set the null value:
    //

    switch (type)
    {
        case CIMTYPE_BOOLEAN:
            CIMValueType<Boolean>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_UINT8:
            CIMValueType<Uint8>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_SINT8:
            CIMValueType<Sint8>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_UINT16:
            CIMValueType<Uint16>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_SINT16:
            CIMValueType<Sint16>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_UINT32:
            CIMValueType<Uint32>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_SINT32:
            CIMValueType<Sint32>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_UINT64:
            CIMValueType<Uint64>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_SINT64:
            CIMValueType<Sint64>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_REAL32:
            CIMValueType<Real32>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_REAL64:
            CIMValueType<Real64>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_CHAR16:
            CIMValueType<Char16>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_STRING:
            CIMValueType<String>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_DATETIME:
            CIMValueType<CIMDateTime>::setNull(_rep, type, isArray, arraySize);
            break;

        case CIMTYPE_REFERENCE:
            CIMValueType<CIMObjectPath>::setNull(_rep, type, isArray,arraySize);
            break;

        case CIMTYPE_OBJECT:
            CIMValueType<CIMObject>::setNull(_rep, type, isArray, arraySize);
            break;
        case CIMTYPE_INSTANCE:
            CIMValueType<CIMInstance>::setNull(_rep, type, isArray, arraySize);
            break;
        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    }
}

void CIMValue::set(Boolean x)
{
    _release(_rep);
    CIMValueType<Boolean>::set(_rep, x);
}

void CIMValue::set(Uint8 x)
{
    _release(_rep);
    CIMValueType<Uint8>::set(_rep, x);
}

void CIMValue::set(Sint8 x)
{
    _release(_rep);
    CIMValueType<Sint8>::set(_rep, x);
}

void CIMValue::set(Uint16 x)
{
    _release(_rep);
    CIMValueType<Uint16>::set(_rep, x);
}

void CIMValue::set(Sint16 x)
{
    _release(_rep);
    CIMValueType<Sint16>::set(_rep, x);
}

void CIMValue::set(Uint32 x)
{
    _release(_rep);
    CIMValueType<Uint32>::set(_rep, x);
}

void CIMValue::set(Sint32 x)
{
    _release(_rep);
    CIMValueType<Sint32>::set(_rep, x);
}

void CIMValue::set(Uint64 x)
{
    _release(_rep);
    CIMValueType<Uint64>::set(_rep, x);
}

void CIMValue::set(Sint64 x)
{
    _release(_rep);
    CIMValueType<Sint64>::set(_rep, x);
}

void CIMValue::set(Real32 x)
{
    _release(_rep);
    CIMValueType<Real32>::set(_rep, x);
}

void CIMValue::set(Real64 x)
{
    _release(_rep);
    CIMValueType<Real64>::set(_rep, x);
}

void CIMValue::set(const Char16& x)
{
    _release(_rep);
    CIMValueType<Char16>::set(_rep, x);
}

void CIMValue::set(const String& x)
{
    _release(_rep);
    CIMValueType<String>::set(_rep, x);
}

void CIMValue::set(const CIMDateTime& x)
{
    _release(_rep);
    CIMValueType<CIMDateTime>::set(_rep, x);
}

void CIMValue::set(const CIMObjectPath& x)
{
    _release(_rep);
    CIMValueType<CIMObjectPath>::set(_rep, x);
}

void CIMValue::set(const CIMObject& x)
{
    if (x.isUninitialized())
    {
        // Bug 3373, throw exception on uninitialized object.
        throw UninitializedObjectException();
    }

    _release(_rep);
    CIMValueType<CIMObject>::set(_rep, x.clone());
}
void CIMValue::set(const CIMInstance& x)
{
    if (x.isUninitialized())
    {
        // Bug 3373, throw exception on uninitialized object.
        throw UninitializedObjectException();
    }

    _release(_rep);
    CIMValueType<CIMInstance>::set(_rep, x.clone());
}
void CIMValue::set(const Array<Boolean>& x)
{
    _release(_rep);
    CIMValueType<Boolean>::setArray(_rep, x);
}

void CIMValue::set(const Array<Uint8>& x)
{
    _release(_rep);
    CIMValueType<Uint8>::setArray(_rep, x);
}

void CIMValue::set(const Array<Sint8>& x)
{
    _release(_rep);
    CIMValueType<Sint8>::setArray(_rep, x);
}

void CIMValue::set(const Array<Uint16>& x)
{
    _release(_rep);
    CIMValueType<Uint16>::setArray(_rep, x);
}

void CIMValue::set(const Array<Sint16>& x)
{
    _release(_rep);
    CIMValueType<Sint16>::setArray(_rep, x);
}

void CIMValue::set(const Array<Uint32>& x)
{
    _release(_rep);
    CIMValueType<Uint32>::setArray(_rep, x);
}

void CIMValue::set(const Array<Sint32>& x)
{
    _release(_rep);
    CIMValueType<Sint32>::setArray(_rep, x);
}

void CIMValue::set(const Array<Uint64>& x)
{
    _release(_rep);
    CIMValueType<Uint64>::setArray(_rep, x);
}

void CIMValue::set(const Array<Sint64>& x)
{
    _release(_rep);
    CIMValueType<Sint64>::setArray(_rep, x);
}

void CIMValue::set(const Array<Real32>& x)
{
    _release(_rep);
    CIMValueType<Real32>::setArray(_rep, x);
}

void CIMValue::set(const Array<Real64>& x)
{
    _release(_rep);
    CIMValueType<Real64>::setArray(_rep, x);
}

void CIMValue::set(const Array<Char16>& x)
{
    _release(_rep);
    CIMValueType<Char16>::setArray(_rep, x);
}

void CIMValue::set(const Array<String>& x)
{
    _release(_rep);
    CIMValueType<String>::setArray(_rep, x);
}

void CIMValue::set(const Array<CIMDateTime>& x)
{
    _release(_rep);
    CIMValueType<CIMDateTime>::setArray(_rep, x);
}

void CIMValue::set(const Array<CIMObjectPath>& x)
{
    _release(_rep);
    CIMValueType<CIMObjectPath>::setArray(_rep, x);
}

void CIMValue::set(const Array<CIMObject>& a)
{
    Array<CIMObject> tmp;

    for (Uint32 i = 0, n = a.size(); i < n; i++)
    {
        if (a[i].isUninitialized())
        {
            // Bug 3373, throw exception on uninitialized object.
            throw UninitializedObjectException();
        }

        tmp.append(a[i].clone());
    }

    _release(_rep);
    CIMValueType<CIMObject>::setArray(_rep, tmp);
}
void CIMValue::set(const Array<CIMInstance>& a)
{
    Array<CIMInstance> tmp;

    for (Uint32 i = 0, n = a.size(); i < n; i++)
    {
        if (a[i].isUninitialized())
        {
                  // Bug 3373, throw exception on uninitialized object.
                  throw UninitializedObjectException();
        }

        tmp.append(a[i].clone());
    }

    _release(_rep);
    CIMValueType<CIMInstance>::setArray(_rep, tmp);
}
void CIMValue::get(Boolean& x) const
{
    if (_rep->type != CIMTYPE_BOOLEAN || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Boolean>::ref(_rep);
}

void CIMValue::get(Uint8& x) const
{
    if (_rep->type != CIMTYPE_UINT8 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Uint8>::ref(_rep);
}

void CIMValue::get(Sint8& x) const
{
    if (_rep->type != CIMTYPE_SINT8 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Sint8>::ref(_rep);
}

void CIMValue::get(Uint16& x) const
{
    if (_rep->type != CIMTYPE_UINT16 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Uint16>::ref(_rep);
}

void CIMValue::get(Sint16& x) const
{
    if (_rep->type != CIMTYPE_SINT16 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Sint16>::ref(_rep);
}


void CIMValue::get(Uint32& x) const
{
    if (_rep->type != CIMTYPE_UINT32 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Uint32>::ref(_rep);
}

void CIMValue::get(Sint32& x) const
{
    if (_rep->type != CIMTYPE_SINT32 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Sint32>::ref(_rep);
}

void CIMValue::get(Uint64& x) const
{
    if (_rep->type != CIMTYPE_UINT64 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Uint64>::ref(_rep);
}

void CIMValue::get(Sint64& x) const
{
    if (_rep->type != CIMTYPE_SINT64 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Sint64>::ref(_rep);
}

void CIMValue::get(Real32& x) const
{
    if (_rep->type != CIMTYPE_REAL32 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Real32>::ref(_rep);
}

void CIMValue::get(Real64& x) const
{
    if (_rep->type != CIMTYPE_REAL64 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Real64>::ref(_rep);
}

void CIMValue::get(Char16& x) const
{
    if (_rep->type != CIMTYPE_CHAR16 || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Char16>::ref(_rep);
}

void CIMValue::get(String& x) const
{
    if (_rep->type != CIMTYPE_STRING || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<String>::ref(_rep);
}

void CIMValue::get(CIMDateTime& x) const
{
    if (_rep->type != CIMTYPE_DATETIME || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<CIMDateTime>::ref(_rep);
}

void CIMValue::get(CIMObjectPath& x) const
{
    if (_rep->type != CIMTYPE_REFERENCE || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<CIMObjectPath>::ref(_rep);
}

void CIMValue::get(CIMObject& x) const
{
    if (_rep->type != CIMTYPE_OBJECT || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        // We have to clone our own unique copy since we are about to
        // return an object to the caller that he can modify; thereby,
        // changing the one we refer to as well.
        x = CIMValueType<CIMObject>::ref(_rep).clone();
}
void CIMValue::get(CIMInstance& x) const
{
    if (_rep->type != CIMTYPE_INSTANCE || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
    {
        // We have to clone our own unique copy since we are about to
        // return an object to the caller that he can modify; thereby,
        // changing the one we refer to as well.
        x = CIMValueType<CIMInstance>::ref(_rep).clone();
    }
}
void CIMValue::get(Array<Boolean>& x) const
{
    if (_rep->type != CIMTYPE_BOOLEAN || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Boolean>::aref(_rep);
}

void CIMValue::get(Array<Uint8>& x) const
{
    if (_rep->type != CIMTYPE_UINT8 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Uint8>::aref(_rep);
}

void CIMValue::get(Array<Sint8>& x) const
{
    if (_rep->type != CIMTYPE_SINT8 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Sint8>::aref(_rep);
}

void CIMValue::get(Array<Uint16>& x) const
{
    if (_rep->type != CIMTYPE_UINT16 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Uint16>::aref(_rep);
}

void CIMValue::get(Array<Sint16>& x) const
{
    if (_rep->type != CIMTYPE_SINT16 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Sint16>::aref(_rep);
}

void CIMValue::get(Array<Uint32>& x) const
{
    if (_rep->type != CIMTYPE_UINT32 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Uint32>::aref(_rep);
}

void CIMValue::get(Array<Sint32>& x) const
{
    if (_rep->type != CIMTYPE_SINT32 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Sint32>::aref(_rep);
}

void CIMValue::get(Array<Uint64>& x) const
{
    if (_rep->type != CIMTYPE_UINT64 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Uint64>::aref(_rep);
}

void CIMValue::get(Array<Sint64>& x) const
{
    if (_rep->type != CIMTYPE_SINT64 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Sint64>::aref(_rep);
}

void CIMValue::get(Array<Real32>& x) const
{
    if (_rep->type != CIMTYPE_REAL32 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Real32>::aref(_rep);
}

void CIMValue::get(Array<Real64>& x) const
{
    if (_rep->type != CIMTYPE_REAL64 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Real64>::aref(_rep);
}

void CIMValue::get(Array<Char16>& x) const
{
    if (_rep->type != CIMTYPE_CHAR16 || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<Char16>::aref(_rep);
}

void CIMValue::get(Array<String>& x) const
{
    if (_rep->type != CIMTYPE_STRING || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<String>::aref(_rep);
}

void CIMValue::get(Array<CIMDateTime>& x) const
{
    if (_rep->type != CIMTYPE_DATETIME || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<CIMDateTime>::aref(_rep);
}

void CIMValue::get(Array<CIMObjectPath>& x) const
{
    if (_rep->type != CIMTYPE_REFERENCE || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        x = CIMValueType<CIMObjectPath>::aref(_rep);
}

void CIMValue::get(Array<CIMObject>& x) const
{
    if (_rep->type != CIMTYPE_OBJECT || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
    {
        x.clear();

        // We have to clone our own unique copy since we are about to
        // return an object to the caller that he can modify; thereby,
        // changing the one we refer to as well.
        for (Uint32 i = 0, n = CIMValueType<CIMObject>::arraySize(_rep);
             i < n; i++)
        {
            x.append(CIMValueType<CIMObject>::aref(_rep)[i].clone());
        }
    }
}

void CIMValue::get(Array<CIMInstance>& x) const
{
    if (_rep->type != CIMTYPE_INSTANCE || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
    {
        x.clear();

        // We have to clone our own unique copy since we are about to
        // return an object to the caller that he can modify; thereby,
        // changing the one we refer to as well.
        for (Uint32 i = 0, n = CIMValueType<CIMInstance>::arraySize(_rep);
             i < n; i++)
        {
            x.append(CIMValueType<CIMInstance>::aref(_rep)[i].clone());
        }
    }
}

Boolean CIMValue::equal(const CIMValue& x) const
{
    if (!typeCompatible(x))
        return false;

    if (_rep->isNull != x._rep->isNull)
        return false;

    if (_rep->isNull)
        return true;

    if (_rep->isArray)
    {
        switch (_rep->type)
        {
            case CIMTYPE_BOOLEAN:
                return CIMValueType<Boolean>::equalArray(_rep, x._rep);

            case CIMTYPE_UINT8:
                return CIMValueType<Uint8>::equalArray(_rep, x._rep);

            case CIMTYPE_SINT8:
                return CIMValueType<Sint8>::equalArray(_rep, x._rep);

            case CIMTYPE_UINT16:
                return CIMValueType<Uint16>::equalArray(_rep, x._rep);

            case CIMTYPE_SINT16:
                return CIMValueType<Sint16>::equalArray(_rep, x._rep);

            case CIMTYPE_UINT32:
                return CIMValueType<Uint32>::equalArray(_rep, x._rep);

            case CIMTYPE_SINT32:
                return CIMValueType<Sint32>::equalArray(_rep, x._rep);

            case CIMTYPE_UINT64:
                return CIMValueType<Uint64>::equalArray(_rep, x._rep);

            case CIMTYPE_SINT64:
                return CIMValueType<Sint64>::equalArray(_rep, x._rep);

            case CIMTYPE_REAL32:
                return CIMValueType<Real32>::equalArray(_rep, x._rep);

            case CIMTYPE_REAL64:
                return CIMValueType<Real64>::equalArray(_rep, x._rep);

            case CIMTYPE_CHAR16:
                return CIMValueType<Char16>::equalArray(_rep, x._rep);

            case CIMTYPE_STRING:
                return CIMValueType<String>::equalArray(_rep, x._rep);

            case CIMTYPE_DATETIME:
                return CIMValueType<CIMDateTime>::equalArray(_rep, x._rep);

            case CIMTYPE_REFERENCE:
                return CIMValueType<CIMObjectPath>::equalArray(_rep, x._rep);

            case CIMTYPE_OBJECT:
                return CIMValueType<CIMObject>::equalArray(_rep, x._rep);
            case CIMTYPE_INSTANCE:
                return CIMValueType<CIMInstance>::equalArray(_rep, x._rep);
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
        }
    }
    else
    {
        switch (_rep->type)
        {
            case CIMTYPE_BOOLEAN:
                return CIMValueType<Boolean>::equal(_rep, x._rep);

            case CIMTYPE_UINT8:
                return CIMValueType<Uint8>::equal(_rep, x._rep);

            case CIMTYPE_SINT8:
                return CIMValueType<Sint8>::equal(_rep, x._rep);

            case CIMTYPE_UINT16:
                return CIMValueType<Uint16>::equal(_rep, x._rep);

            case CIMTYPE_SINT16:
                return CIMValueType<Sint16>::equal(_rep, x._rep);

            case CIMTYPE_UINT32:
                return CIMValueType<Uint32>::equal(_rep, x._rep);

            case CIMTYPE_SINT32:
                return CIMValueType<Sint32>::equal(_rep, x._rep);

            case CIMTYPE_UINT64:
                return CIMValueType<Uint64>::equal(_rep, x._rep);

            case CIMTYPE_SINT64:
                return CIMValueType<Sint64>::equal(_rep, x._rep);

            case CIMTYPE_REAL32:
                return CIMValueType<Real32>::equal(_rep, x._rep);

            case CIMTYPE_REAL64:
                return CIMValueType<Real64>::equal(_rep, x._rep);

            case CIMTYPE_CHAR16:
                return CIMValueType<Char16>::equal(_rep, x._rep);

            case CIMTYPE_STRING:
                return CIMValueType<String>::equal(_rep, x._rep);

            case CIMTYPE_DATETIME:
                return CIMValueType<CIMDateTime>::equal(_rep, x._rep);

            case CIMTYPE_REFERENCE:
                return CIMValueType<CIMObjectPath>::equal(_rep, x._rep);

            case CIMTYPE_OBJECT:
                return CIMValueType<CIMObject>::ref(_rep).identical(
                    CIMValueType<CIMObject>::ref(x._rep));
            case CIMTYPE_INSTANCE:
                return CIMValueType<CIMInstance>::ref(_rep).identical(
                    CIMValueType<CIMInstance>::ref(x._rep));
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
        }
    }

    return false;
}

String CIMValue::toString() const
{
    Buffer out;

    // ATTN: Not sure what we should do with getstring for Null CIMValues
    // Choice return empty string or exception out.

    if (_rep->isNull)
        return String();

    if (_rep->isArray)
    {
        switch (_rep->type)
        {
            case CIMTYPE_BOOLEAN:
            {
                const Array<Boolean>& a = CIMValueType<Boolean>::aref(_rep);
                Uint32 size = a.size();

                for (Uint32 i = 0; i < size; i++)
                {
                    _toString(out, a[i]);
                    out.append(' ');
                }
                break;
            }

            case CIMTYPE_UINT8:
            {
                const Array<Uint8>& a = CIMValueType<Uint8>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT8:
            {
                const Array<Sint8>& a = CIMValueType<Sint8>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT16:
            {
                const Array<Uint16>& a = CIMValueType<Uint16>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT16:
            {
                const Array<Sint16>& a = CIMValueType<Sint16>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT32:
            {
                const Array<Uint32>& a = CIMValueType<Uint32>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT32:
            {
                const Array<Sint32>& a = CIMValueType<Sint32>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT64:
            {
                const Array<Uint64>& a = CIMValueType<Uint64>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT64:
            {
                const Array<Sint64>& a = CIMValueType<Sint64>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REAL32:
            {
                const Array<Real32>& a = CIMValueType<Real32>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REAL64:
            {
                const Array<Real64>& a = CIMValueType<Real64>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_CHAR16:
            {
                const Array<Char16>& a = CIMValueType<Char16>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_STRING:
            {
                const Array<String>& a = CIMValueType<String>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_DATETIME:
            {
                const Array<CIMDateTime>& a =
                    CIMValueType<CIMDateTime>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REFERENCE:
            {
                const Array<CIMObjectPath>& a =
                    CIMValueType<CIMObjectPath>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_OBJECT:
            {
                const Array<CIMObject>& a = CIMValueType<CIMObject>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                const Array<CIMInstance>& a =
                    CIMValueType<CIMInstance>::aref(_rep);
                _toString(out, a.getData(), a.size());
                break;
            }
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
        }
    }
    else
    {
        switch (_rep->type)
        {
            case CIMTYPE_BOOLEAN:
                _toString(out, CIMValueType<Boolean>::ref(_rep));
                break;

            case CIMTYPE_UINT8:
                _toString(out, CIMValueType<Uint8>::ref(_rep));
                break;

            case CIMTYPE_SINT8:
                _toString(out, CIMValueType<Sint8>::ref(_rep));
                break;

            case CIMTYPE_UINT16:
                _toString(out, CIMValueType<Uint16>::ref(_rep));
                break;

            case CIMTYPE_SINT16:
                _toString(out, CIMValueType<Sint16>::ref(_rep));
                break;

            case CIMTYPE_UINT32:
                _toString(out, CIMValueType<Uint32>::ref(_rep));
                break;

            case CIMTYPE_SINT32:
                _toString(out, CIMValueType<Sint32>::ref(_rep));
                break;

            case CIMTYPE_UINT64:
                _toString(out, CIMValueType<Uint64>::ref(_rep));
                break;

            case CIMTYPE_SINT64:
                _toString(out, CIMValueType<Sint64>::ref(_rep));
                break;

            case CIMTYPE_REAL32:
                _toString(out, CIMValueType<Real32>::ref(_rep));
                break;

            case CIMTYPE_REAL64:
                _toString(out, CIMValueType<Real64>::ref(_rep));
                break;

            case CIMTYPE_CHAR16:
                _toString(out, CIMValueType<Char16>::ref(_rep));
                break;

            case CIMTYPE_STRING:
                _toString(out, CIMValueType<String>::ref(_rep));
                break;

            case CIMTYPE_DATETIME:
                _toString(out, CIMValueType<CIMDateTime>::ref(_rep));
                break;

            case CIMTYPE_REFERENCE:
                _toString(out, CIMValueType<CIMObjectPath>::ref(_rep));
                break;

            case CIMTYPE_OBJECT:
                _toString(out, CIMValueType<CIMObject>::ref(_rep));
                break;
            case CIMTYPE_INSTANCE:
                _toString(out, CIMValueType<CIMInstance>::ref(_rep));
                break;
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
        }
    }

    return out.getData();
}

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES

CIMValue::CIMValue(char x)
{
    _rep = new CIMValueRep;
    CIMValueType<Sint8>::set(_rep, (Sint8)x);
}

CIMValue::CIMValue(const Array<char>& x)
{
    _rep = new CIMValueRep;
    Array<Sint8> tmp((Sint8*)x.getData(), x.size());
    CIMValueType<Sint8>::setArray(_rep, tmp);
}

void CIMValue::set(char x)
{
    set(static_cast<Sint8>(x));
}

void CIMValue::set(const Array<char>& x)
{
    set(*reinterpret_cast<const Array<Sint8>*>(&x));
}

void CIMValue::get(char& x) const
{
    get(*reinterpret_cast<Sint8*>(&x));
}

void CIMValue::get(Array<char>& x) const
{
    get(*reinterpret_cast<Array<Sint8>*>(&x));
}

#endif /* PEGASUS_USE_DEPRECATED_INTERFACES */

void CIMValue::_get(const String*& data, Uint32& size) const
{
    const Array<String>& a = CIMValueType<String>::aref(_rep);
    data = a.getData();
    size = a.size();
}

Boolean operator==(const CIMValue& x, const CIMValue& y)
{
    return x.equal(y);
}

Boolean operator!=(const CIMValue& x, const CIMValue& y)
{
    return !x.equal(y);
}

PEGASUS_NAMESPACE_END
