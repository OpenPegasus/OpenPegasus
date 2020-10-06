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
#include <stdlib.h>
#include "Sharable.h"
#include "CIMBuffer.h"
#include "CIMPropertyRep.h"
#include "CIMInstanceRep.h"
#include "CIMClassRep.h"
#include "CIMQualifierRep.h"
#include "CIMQualifierDeclRep.h"
#include "CIMParameterRep.h"
#include "CIMMethodRep.h"
#include "CIMPropertyList.h"
#include "CIMParamValueRep.h"
#include "CIMKeyBindingRep.h"
#include "CIMObjectPathRep.h"
#include "CIMPropertyListRep.h"
#include "StringRep.h"
#include "CIMValueRep.h"
#include "StringRep.h"
#include "StringInline.h"
#include "Buffer.h"
#include "SCMOStreamer.h"
#include "Pegasus_inl.h"

#define INSTANCE_MAGIC 0xD6EF2219
#define CLASS_MAGIC 0xA8D7DE41
#define OBJECT_MAGIC 0xA8D7DE41
#define OBJECT_PATH_MAGIC 0x92320710
#define PROPERTY_MAGIC 0xBFEAA215
#define VALUE_MAGIC 0xE83E360A
#define PRESENT_MAGIC 0xF55A7330
#define ABSENT_MAGIC 0x77A0A639

#define FLAG_IS_NULL             (1 << 0)
#define FLAG_IS_ARRAY            (1 << 1)
#define FLAG_IS_PROPAGATED       (1 << 2)
#define FLAG_HAS_CLASS_ORIGIN    (1 << 3)
#define FLAG_HAS_REFERENCE_CLASS (1 << 4)
#define FLAG_HAS_QUALIFIERS      (1 << 5)

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static bool _validString(const Uint16* p, size_t n)
{
    const Uint16* start = p;

    while (n >= 8 && ((p[0]|p[1]|p[2]|p[3]|p[4]|p[5]|p[6]|p[7]) & 0xFF80) == 0)
    {
        p += 8;
        n -= 8;
    }

    while (n >= 4 && ((p[0]|p[1]|p[2]|p[3]) & 0xFF80) == 0)
    {
        p += 4;
        n -= 4;
    }

    while (n)
    {
        Uint16 c = *p;

        if (c >= 128)
        {
            if (c == 0xFFFE || c == 0xFFFF || (c >= 0xFDD0 && c <= 0xFDEF))
                return false;

            if (c >= 0xD800 && c <= 0xDBFF)
            {
                if (n == 1 || !(p[1] >= 0xDC00 && p[1] <= 0xDFFF))
                    return false;
            }

            if (c >= 0xDC00 && c <= 0xDFFF)
            {
                if (p == start || !(p[-1] >= 0xD800  && p[-1] <= 0xDBFF))
                    return false;
            }
        }

        n--;
        p++;
    }

    return true;
}

static inline bool _validName(const String& str)
{
    if (str.size() == 0)
        return true;

    return CIMName::legal(str);
}

static inline bool _validNamespaceName(const String& str)
{
    if (str.size() == 0)
        return true;

    return CIMNamespaceName::legal(str);
}

void CIMBuffer::_create(size_t size)
{
    if (size < 1024)
        size = 1024;

#if defined(PEGASUS_TEST_VALGRIND)
    // Valgrind complains that we leave uninitialized bytes in this buffer so
    _data = (char*)calloc(1, size);
#else
    _data = (char*)malloc(size);
#endif

#if defined(PEGASUS_DEBUG)
    memset(_data, 0xAA, size);
#endif

    if (!_data)
    {
        throw PEGASUS_STD(bad_alloc)();
    }

    _end = _data + size;
    _ptr = _data;
}

CIMBuffer::CIMBuffer(size_t size) : _swap(0), _validate(0)
{
    _create(size);
}

CIMBuffer::CIMBuffer() : _data(0), _end(0), _ptr(0), _swap(0), _validate(0)
{
}

CIMBuffer::~CIMBuffer()
{
    free(_data);
}

void CIMBuffer::_grow(size_t size)
{
    size_t n = _end - _data;
    size_t m = _ptr - _data;
    size_t cap = n * 2;

    if (cap <= 4096)
        cap = 4096;

    // Double the size of the buffer (n * 2). If size is greater than n, then
    // we will need yet more space so we increment cap by size.

    if (size > n)
        cap += size;

    _data = (char*)peg_inln_realloc(_data, cap);

    _end = _data + cap;
    _ptr = _data + m;

#if defined(PEGASUS_DEBUG)
    memset(_ptr, 0xAA, _end - _ptr);
#elif defined(PEGASUS_TEST_VALGRIND)
    // Valgrind complains that we leave uninitialized bytes in this buffer so
    // we clear all newly allocated memory when testing with Valgrind.
    memset(_ptr, 0, _end - _ptr);
#endif

}

bool CIMBuffer::getString(String& x)
{
    Uint32 n;

    if (!getUint32(n))
        return false;

    size_t r = round(n * sizeof(Char16));

    if (_end - _ptr < ptrdiff_t(r))
        return false;

    if (_swap)
    {
        _swapUint16Data((Uint16*)_ptr, n);
    }

    if (_validate)
    {
        if (!_validString((Uint16*)_ptr, n))
            return false;
    }

    if (n)
        x.assign((Char16*)_ptr, n);

    _ptr += r;
    return true;
}

bool CIMBuffer::getName(CIMName& x)
{
    String tmp;

    if (_validate)
    {
        // Get string without validation since we will validate name below.

        _validate = false;

        if (!getString(tmp))
            return false;

        _validate = true;

        if (!_validName(tmp))
            return false;
    }
    else
    {
        if (!getString(tmp))
            return false;
    }

    x = CIMNameCast(tmp);
    return true;
}

bool CIMBuffer::getNamespaceName(CIMNamespaceName& x)
{
    String tmp;

    if (_validate)
    {
        // Get string without validation and validate namespace below.

        _validate = false;

        if (!getString(tmp))
            return false;

        _validate = true;

        if (!_validNamespaceName(tmp))
            return false;
    }
    else
    {
        if (!getString(tmp))
            return false;
    }

    x = CIMNamespaceNameCast(tmp);
    return true;
}

void CIMBuffer::putValue(const CIMValue& x)
{
    CIMValueRep* rep = *((CIMValueRep**)&x);

    // Resolve null flag:

    bool isNull = rep->isNull;

    if (!isNull && rep->type == CIMTYPE_INSTANCE && !rep->isArray)
    {
        const CIMInstance& ci = *((CIMInstance*)rep->u._instanceValue);

        if (ci.isUninitialized())
        {
            isNull = true;
        }
    }

    // Magic:
    _putMagic(VALUE_MAGIC);

    // Put flags:
    {
        Uint32 flags = 0;

        if (isNull)
            flags |= FLAG_IS_NULL;

        if (rep->isArray)
            flags |= FLAG_IS_ARRAY;

        putUint32(flags);
    }

    // Type:
    putUint32(rep->type);

    if (isNull)
        return;

    // Put value:

    if (rep->isArray)
    {
        switch (rep->type)
        {
            case CIMTYPE_BOOLEAN:
                putBooleanA(*(reinterpret_cast<Array<Boolean>*>(&rep->u)));
                break;
            case CIMTYPE_UINT8:
                putUint8A(*(reinterpret_cast<Array<Uint8>*>(&rep->u)));
                break;
            case CIMTYPE_SINT8:
                putSint8A(*(reinterpret_cast<Array<Sint8>*>(&rep->u)));
                break;
            case CIMTYPE_UINT16:
                putUint16A(*(reinterpret_cast<Array<Uint16>*>(&rep->u)));
                break;
            case CIMTYPE_SINT16:
                putSint16A(*(reinterpret_cast<Array<Sint16>*>(&rep->u)));
                break;
            case CIMTYPE_UINT32:
                putUint32A(*(reinterpret_cast<Array<Uint32>*>(&rep->u)));
                break;
            case CIMTYPE_SINT32:
                putSint32A(*(reinterpret_cast<Array<Sint32>*>(&rep->u)));
                break;
            case CIMTYPE_UINT64:
                putUint64A(*(reinterpret_cast<Array<Uint64>*>(&rep->u)));
                break;
            case CIMTYPE_SINT64:
                putSint64A(*(reinterpret_cast<Array<Sint64>*>(&rep->u)));
                break;
            case CIMTYPE_REAL32:
                putReal32A(*(reinterpret_cast<Array<Real32>*>(&rep->u)));
                break;
            case CIMTYPE_REAL64:
                putReal64A(*(reinterpret_cast<Array<Real64>*>(&rep->u)));
                break;
            case CIMTYPE_CHAR16:
                putChar16A(*(reinterpret_cast<Array<Char16>*>(&rep->u)));
                break;
            case CIMTYPE_STRING:
                putStringA(*(reinterpret_cast<Array<String>*>(&rep->u)));
                break;
            case CIMTYPE_DATETIME:
                putDateTimeA(*(reinterpret_cast<Array<CIMDateTime>*>(&rep->u)));
                break;
            case CIMTYPE_REFERENCE:
                putObjectPathA(
                    *(reinterpret_cast<Array<CIMObjectPath>*>(&rep->u)));
                break;
            case CIMTYPE_INSTANCE:
                putInstanceA(*(reinterpret_cast<Array<CIMInstance>*>(&rep->u)),
                    false, false);
                break;
            case CIMTYPE_OBJECT:
                putObjectA(*(reinterpret_cast<Array<CIMObject>*>(&rep->u)),
                    false, false);
                break;
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }
    }
    else
    {
        switch (rep->type)
        {
            case CIMTYPE_BOOLEAN:
                putBoolean(rep->u._booleanValue);
                break;
            case CIMTYPE_UINT8:
                putUint8(rep->u._uint8Value);
                break;
            case CIMTYPE_SINT8:
                putSint8(rep->u._sint8Value);
                break;
            case CIMTYPE_UINT16:
                putUint16(rep->u._uint16Value);
                break;
            case CIMTYPE_SINT16:
                putSint16(rep->u._sint16Value);
                break;
            case CIMTYPE_UINT32:
                putUint32(rep->u._uint32Value);
                break;
            case CIMTYPE_SINT32:
                putSint32(rep->u._sint32Value);
                break;
            case CIMTYPE_UINT64:
                putUint64(rep->u._uint64Value);
                break;
            case CIMTYPE_SINT64:
                putSint64(rep->u._sint64Value);
                break;
            case CIMTYPE_REAL32:
                putReal32(rep->u._real32Value);
                break;
            case CIMTYPE_REAL64:
                putReal64(rep->u._real64Value);
                break;
            case CIMTYPE_CHAR16:
                putChar16(rep->u._char16Value);
                break;
            case CIMTYPE_STRING:
                putString(*((String*)rep->u._stringValue));
                break;
            case CIMTYPE_DATETIME:
                putDateTime(*((CIMDateTime*)rep->u._dateTimeValue));
                break;
            case CIMTYPE_REFERENCE:
                putObjectPath(*((CIMObjectPath*)rep->u._referenceValue));
                break;
            case CIMTYPE_INSTANCE:
                putInstance(*((CIMInstance*)rep->u._instanceValue),false,false);
                break;
            case CIMTYPE_OBJECT:
                putObject(*((CIMObject*)rep->u._instanceValue), false, false);
                break;
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }
    }
}

bool CIMBuffer::getValue(CIMValue& x)
{
    Uint32 type;
    Boolean isNull;
    Boolean isArray;

    // Magic:
    if (!_testMagic(VALUE_MAGIC))
        return false;

    // Flags:
    Uint32 flags;

    if (!getUint32(flags))
        return false;

    isNull = flags & FLAG_IS_NULL;
    isArray = flags & FLAG_IS_ARRAY;

    // Type:
    if (!getUint32(type))
        return false;

    if (isNull)
    {
        x.setNullValue(CIMType(type), isArray, 0);
        return true;
    }

    if (isArray)
    {
        switch (type)
        {
            case CIMTYPE_BOOLEAN:
            {
                Array<Boolean> tmp;
                if (!getBooleanA(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_UINT8:
            {
                Array<Uint8> tmp;
                if (!getUint8A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_SINT8:
            {
                Array<Sint8> tmp;
                if (!getSint8A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_UINT16:
            {
                Array<Uint16> tmp;
                if (!getUint16A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_SINT16:
            {
                Array<Sint16> tmp;
                if (!getSint16A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_UINT32:
            {
                Array<Uint32> tmp;
                if (!getUint32A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_SINT32:
            {
                Array<Sint32> tmp;
                if (!getSint32A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_UINT64:
            {
                Array<Uint64> tmp;
                if (!getUint64A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_SINT64:
            {
                Array<Sint64> tmp;
                if (!getSint64A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_REAL32:
            {
                Array<Real32> tmp;
                if (!getReal32A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_REAL64:
            {
                Array<Real64> tmp;
                if (!getReal64A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_CHAR16:
            {
                Array<Char16> tmp;
                if (!getChar16A(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_STRING:
            {
                Array<String> tmp;
                if (!getStringA(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_DATETIME:
            {
                Array<CIMDateTime> tmp;
                if (!getDateTimeA(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_REFERENCE:
            {
                Array<CIMObjectPath> tmp;
                if (!getObjectPathA(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_INSTANCE:
            {
                Array<CIMInstance> tmp;
                if (!getInstanceA(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_OBJECT:
            {
                Array<CIMObject> tmp;
                if (!getObjectA(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }
    }
    else
    {
        switch (type)
        {
            case CIMTYPE_BOOLEAN:
            {
                Boolean tmp;
                if (!getBoolean(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_UINT8:
            {
                Uint8 tmp;
                if (!getUint8(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_SINT8:
            {
                Sint8 tmp;
                if (!getSint8(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_UINT16:
            {
                Uint16 tmp;
                if (!getUint16(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_SINT16:
            {
                Sint16 tmp;
                if (!getSint16(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_UINT32:
            {
                Uint32 tmp;
                if (!getUint32(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_SINT32:
            {
                Sint32 tmp;
                if (!getSint32(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_UINT64:
            {
                Uint64 tmp;
                if (!getUint64(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_SINT64:
            {
                Sint64 tmp;
                if (!getSint64(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_REAL32:
            {
                Real32 tmp;
                if (!getReal32(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_REAL64:
            {
                Real64 tmp;
                if (!getReal64(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_CHAR16:
            {
                Char16 tmp;
                if (!getChar16(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_STRING:
            {
                String tmp;
                if (!getString(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_DATETIME:
            {
                CIMDateTime tmp;
                if (!getDateTime(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_REFERENCE:
            {
                CIMObjectPath tmp;
                if (!getObjectPath(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_INSTANCE:
            {
                CIMInstance tmp;
                if (!getInstance(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            case CIMTYPE_OBJECT:
            {
                CIMObject tmp;
                if (!getObject(tmp))
                    return false;
                x.set(tmp);
                return true;
            }
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
        }
    }

    // Unreachable!
    return false;
}

void CIMBuffer::putKeyBinding(const CIMKeyBinding& x)
{
    const CIMKeyBindingRep* kb = *(const CIMKeyBindingRep**)&x;
    putName(kb->_name);
    putString(kb->_value);
    putUint32(kb->_type);
}

bool CIMBuffer::getKeyBinding(CIMKeyBinding& x)
{
    CIMName name;
    String value;
    Uint32 type;

    if (!getName(name) || !getString(value) || !getUint32(type))
        return false;

    x.~CIMKeyBinding();
    new(&x) CIMKeyBinding(name, value, CIMKeyBinding::Type(type));

    return true;
}

void CIMBuffer::putObjectPath(
    const CIMObjectPath& x,
    bool includeHostAndNamespace,
    bool includeKeyBindings)
{
    const CIMObjectPathRep* rep = *((const CIMObjectPathRep**)&x);

    _putMagic(OBJECT_PATH_MAGIC);

    if (x.getClassName().isNull())
    {
        putBoolean(false);
        return;
    }
    else
        putBoolean(true);


    if (includeHostAndNamespace)
    {
        putString(rep->_host);
        putNamespaceName(rep->_nameSpace);
    }
    else
    {
        putString(String());
        putString(String());
    }

    putName(rep->_className);

    if (includeKeyBindings)
    {
        putUint32(rep->_keyBindings.size());

        for (Uint32 i = 0, n = rep->_keyBindings.size(); i < n; i++)
        {
            putKeyBinding(rep->_keyBindings[i]);
        }
    }
    else
        putUint32(0);
}

bool CIMBuffer::getObjectPath(CIMObjectPath& x)
{
    String host;
    CIMNamespaceName nameSpace;
    CIMName className;
    Uint32 size;
    Array<CIMKeyBinding> kbs;

    if (!_testMagic(OBJECT_PATH_MAGIC))
        return false;

    Boolean initialized;

    if (!getBoolean(initialized))
        return false;

    if (!initialized)
    {
        x = CIMObjectPath();
        return true;
    }

    if (!getString(host) || !getNamespaceName(nameSpace) || !getName(className))
        return false;

    if (!getUint32(size))
        return false;

    for (Uint32 i = 0; i < size; i++)
    {
        CIMKeyBinding kb;

        if (!getKeyBinding(kb))
            return false;

        kbs.append(kb);
    }

    x.set(
        host,
        *(reinterpret_cast<CIMNamespaceName*>(&nameSpace)),
        className,
        kbs);

    return true;
}

void CIMBuffer::putQualifier(const CIMQualifier& x)
{
    const CIMQualifierRep* rep = *((const CIMQualifierRep**)&x);

    putName(rep->_name);
    putValue(rep->_value);
    putUint32(*((Uint32*)&rep->_flavor));
    putBoolean(rep->_propagated);
}

bool CIMBuffer::getQualifier(CIMQualifier& x)
{
    CIMName name;
    CIMValue value;
    Uint32 flavor;
    Boolean propagated;

    if (!getName(name))
        return false;

    if (!getValue(value))
        return false;

    if (!getUint32(flavor))
        return false;

    if (!getBoolean(propagated))
        return false;

    x.~CIMQualifier();

    new(&x) CIMQualifier(
        name,
        value,
        *(reinterpret_cast<CIMFlavor*>(&flavor)),
        propagated);

    return true;
}

void CIMBuffer::putQualifierList(const CIMQualifierList& x)
{
    Uint32 n = x.getCount();
    putUint32(n);

    for (Uint32 i = 0; i < n; i++)
        putQualifier(x.getQualifier(i));
}

bool CIMBuffer::getQualifierList(CIMQualifierList& x)
{
    Uint32 n;

    if (!getUint32(n))
        return false;

    for (Uint32 i = 0; i < n; i++)
    {
        CIMQualifier tmp;

        if (!getQualifier(tmp))
            return false;

        x.addUnchecked(tmp);
    }

    return true;
}

void CIMBuffer::putQualifierDecl(const CIMQualifierDecl& x)
{
    const CIMQualifierDeclRep* rep = *((const CIMQualifierDeclRep**)&x);

    putName(rep->_name);
    putValue(rep->_value);
    putUint32(*((Uint32*)&rep->_scope));
    putUint32(*((Uint32*)&rep->_flavor));
    putUint32(rep->_arraySize);
}

bool CIMBuffer::getQualifierDecl(CIMQualifierDecl& x)
{
    CIMName name;
    CIMValue value;
    Uint32 scope;
    Uint32 flavor;
    Uint32 arraySize;

    if (!getName(name))
        return false;

    if (!getValue(value))
        return false;

    if (!getUint32(scope))
        return false;

    if (!getUint32(flavor))
        return false;

    if (!getUint32(arraySize))
        return false;

    x.~CIMQualifierDecl();

    new(&x) CIMQualifierDecl(
        name,
        value,
        *(reinterpret_cast<CIMScope*>(&scope)),
        *(reinterpret_cast<CIMFlavor*>(&flavor)),
        arraySize);

    return true;
}

void CIMBuffer::putProperty(const CIMProperty& x)
{
    const CIMPropertyRep* rep = *((const CIMPropertyRep**)&x);
    // PROPERTY_MAGIC
    _putMagic(PROPERTY_MAGIC);

    // Flags
    Uint32 flags = 0;
    {
        // CIMProperty.arraySize
        if (rep->_arraySize)
        {
            flags |= FLAG_IS_ARRAY;
        }

        // CIMProperty.referenceClassName
        if (rep->_referenceClassName.getString().size())
        {
            flags |= FLAG_HAS_REFERENCE_CLASS;
        }

        // CIMProperty.classOrigin
        if (rep->_classOrigin.getString().size())
        {
            flags |= FLAG_HAS_CLASS_ORIGIN;
        }

        // CIMProperty.propagated
        if (rep->_propagated)
        {
            flags |= FLAG_IS_PROPAGATED;
        }

        // CIMProperty.qualifiers
        if (rep->_qualifiers.getCount())
        {
            flags |= FLAG_HAS_QUALIFIERS;
        }

        putUint32(flags);
    }

    // CIMProperty.name
    putName(rep->_name);

    // CIMProperty.value
    putValue(rep->_value);

    // CIMProperty.arraySize
    if (flags & FLAG_IS_ARRAY)
    {
        putUint32(rep->_arraySize);
    }

    // CIMProperty.referenceClassName
    if (flags & FLAG_HAS_REFERENCE_CLASS)
    {
        putName(rep->_referenceClassName);
    }

    // CIMProperty.classOrigin
    if (flags & FLAG_HAS_CLASS_ORIGIN)
    {
        putName(rep->_classOrigin);
    }

    // CIMProperty.qualifiers
    if (flags & FLAG_HAS_QUALIFIERS)
    {
        putQualifierList(rep->_qualifiers);
        flags |= FLAG_HAS_QUALIFIERS;
    }
}

bool CIMBuffer::getProperty(CIMProperty& x)
{
    CIMName name;
    CIMValue value;
    Uint32 arraySize;
    CIMName referenceClassName;
    CIMName classOrigin;
    Boolean propagated;

    if (!_testMagic(PROPERTY_MAGIC))
        return false;

    // Flags:
    Uint32 flags;

    if (!getUint32(flags))
        return false;

    // CIMProperty.name
    if (!getName(name))
        return false;

    // CIMProperty.value
    if (!getValue(value))
        return false;

    // CIMProperty.arraySize

    if (flags & FLAG_IS_ARRAY)
    {
        if (!getUint32(arraySize))
            return false;
    }
    else
        arraySize = 0;

    // CIMProperty.referenceClassName

    if (flags & FLAG_HAS_REFERENCE_CLASS)
    {
        if (!getName(referenceClassName))
            return false;
    }

    // CIMProperty.classOrigin

    if (flags & FLAG_HAS_CLASS_ORIGIN)
    {
        if (!getName(classOrigin))
            return false;
    }

    // CIMProperty.propagated
    propagated = flags & FLAG_IS_PROPAGATED;

    // Create property:

    x.~CIMProperty();

    new(&x) CIMProperty(
        name,
        value,
        arraySize,
        referenceClassName,
        classOrigin,
        propagated);

    CIMPropertyRep* rep = *((CIMPropertyRep**)&x);

    // CIMProperty.qualifiers

    if (flags & FLAG_HAS_QUALIFIERS)
    {
        if (!getQualifierList(rep->_qualifiers))
            return false;
    }

    return true;
}

void CIMBuffer::putInstance(
    const CIMInstance& x,
    bool includeHostAndNamespace,
    bool includeKeyBindings)
{
    const CIMInstanceRep* rep = *((const CIMInstanceRep**)&x);

    // INSTANCE_MAGIC:
    _putMagic(INSTANCE_MAGIC);

    // CIMInstance.initialized:

    if (x.isUninitialized())
    {
        putBoolean(false);
        return;
    }
    else
        putBoolean(true);

    // CIMInstance.reference:
    putObjectPath(rep->_reference, includeHostAndNamespace, includeKeyBindings);

    // CIMInstance.qualifiers:
    putQualifierList(rep->_qualifiers);

    // CIMInstance.properties:
    {
        Uint32 n = rep->_properties.size();
        putUint32(n);

        for (Uint32 i = 0; i < n; i++)
            putProperty(rep->_properties[i]);
    }
}

bool CIMBuffer::getInstance(CIMInstance& x)
{
    CIMInstanceRep* rep;

    // INSTANCE_MAGIC:

    if (!_testMagic(INSTANCE_MAGIC))
        return false;

    // initialized:

    Boolean initialized;

    if (!getBoolean(initialized))
        return false;

    if (!initialized)
    {
        x = CIMInstance();
        return true;
    }

    // CIMIntsance.reference:

    CIMObjectPath reference;

    if (!getObjectPath(reference))
        return false;

    rep = new CIMInstanceRep(reference);

    // CIMInstance.qualifiers:

    if (!getQualifierList(rep->_qualifiers))
        return false;

    // CIMInstance.properties:
    {
        Uint32 n;

        if (!getUint32(n))
            return false;

        for (Uint32 i = 0; i < n; i++)
        {
            CIMProperty tmp;

            if (!getProperty(tmp))
            {
                return false;
            }

            rep->_properties.append(tmp);
        }
    }

    // Replace rep:
    CIMInstanceRep*& tmp = *((CIMInstanceRep**)&x);

    if (tmp)
        tmp->Dec();

    tmp = rep;

    return true;
}

void CIMBuffer::putClass(const CIMClass& x)
{
    const CIMClassRep* rep = *((const CIMClassRep**)&x);

    // CLASS_MAGIC:
    _putMagic(CLASS_MAGIC);

    // CIMClass.initialized:

    if (x.isUninitialized())
    {
        putBoolean(false);
        return;
    }
    else
        putBoolean(true);

    // CIMInstance.reference:
    putObjectPath(rep->_reference);

    // CIMClass.superClassName:
    putName(rep->_superClassName);

    // CIMClass.qualifiers:
    putQualifierList(rep->_qualifiers);

    // CIMClass.properties:
    {
        Uint32 n = rep->_properties.size();
        putUint32(n);

        for (Uint32 i = 0; i < n; i++)
            putProperty(rep->_properties[i]);
    }

    // CIMClass.methods:
    {
        Uint32 n = rep->_methods.size();
        putUint32(n);

        for (Uint32 i = 0; i < n; i++)
            putMethod(rep->_methods[i]);
    }
}

bool CIMBuffer::getClass(CIMClass& x)
{
    CIMClassRep* rep;
    CIMName superClassName;

    // CLASS_MAGIC:

    if (!_testMagic(CLASS_MAGIC))
        return false;

    // initialized:

    Boolean initialized;

    if (!getBoolean(initialized))
        return false;

    if (!initialized)
    {
        x = CIMClass();
        return true;
    }

    // CIMIntsance.reference:

    CIMObjectPath reference;

    if (!getObjectPath(reference))
        return false;

    // CIMIntsance.superClassName:

    if (!getName(superClassName))
        return false;

    rep = new CIMClassRep(reference.getClassName(),
        superClassName);

    rep->_reference = reference;

    // CIMClass.qualifiers:

    if (!getQualifierList(rep->_qualifiers))
        return false;

    // CIMClass.properties:
    {
        Uint32 n;

        if (!getUint32(n))
            return false;

        for (Uint32 i = 0; i < n; i++)
        {
            CIMProperty tmp;

            if (!getProperty(tmp))
                return false;

            rep->_properties.append(tmp);
        }
    }

    // CIMClass.methods:
    {
        Uint32 n;

        if (!getUint32(n))
            return false;

        for (Uint32 i = 0; i < n; i++)
        {
            CIMMethod tmp;

            if (!getMethod(tmp))
                return false;

            rep->_methods.append(tmp);
        }
    }

    // Replace rep:
    CIMClassRep*& tmp = *((CIMClassRep**)&x);

    if (tmp)
        tmp->Dec();

    tmp = rep;

    return true;
}

void CIMBuffer::putParameter(const CIMParameter& x)
{
    const CIMParameterRep* rep = *((const CIMParameterRep**)&x);

    // CIMParameter.name
    putName(rep->_name);

    // CIMParameter.type
    putUint32(rep->_type);

    // CIMParameter.isArray
    putBoolean(rep->_isArray);

    // CIMParameter.arraySize
    putUint32(rep->_arraySize);

    // CIMParameter.referenceClassName
    putName(rep->_referenceClassName);

    // CIMParameter.qualifiers
    putQualifierList(rep->_qualifiers);
}

bool CIMBuffer::getParameter(CIMParameter& x)
{
    CIMName name;
    Uint32 type;
    Boolean isArray;
    Uint32 arraySize;
    CIMName referenceClassName;

    // CIMParameter.name
    if (!getName(name))
        return false;

    // CIMParameter.type
    if (!getUint32(type))
        return false;

    // CIMParameter.isArray
    if (!getBoolean(isArray))
        return false;

    // CIMParameter.arraySize
    if (!getUint32(arraySize))
        return false;

    // CIMParameter.referenceClassName
    if (!getName(referenceClassName))
        return false;

    x.~CIMParameter();

    new(&x) CIMParameter(
        name,
        CIMType(type),
        isArray,
        arraySize,
        referenceClassName);

    CIMParameterRep* rep = *((CIMParameterRep**)&x);

    // CIMParameter.qualifiers
    if (!getQualifierList(rep->_qualifiers))
        return false;

    return true;
}

void CIMBuffer::putMethod(const CIMMethod& x)
{
    const CIMMethodRep* rep = *((const CIMMethodRep**)&x);

    // CIMParameter.name
    putName(rep->_name);

    // CIMParameter.type
    putUint32(rep->_type);

    // CIMProperty.classOrigin
    putName(rep->_classOrigin);

    // CIMProperty.propagated
    putBoolean(rep->_propagated);

    // CIMMethod.qualifiers:
    putQualifierList(rep->_qualifiers);

    // CIMMethod.parameters:
    {
        Uint32 n = rep->_parameters.size();
        putUint32(n);

        for (Uint32 i = 0; i < n; i++)
            putParameter(rep->_parameters[i]);
    }
}

bool CIMBuffer::getMethod(CIMMethod& x)
{
    CIMMethodRep* rep;
    CIMName name;
    Uint32 type;
    CIMName classOrigin;
    Boolean propagated;

    // CIMMethod.name
    if (!getName(name))
        return false;

    // CIMMethod.type
    if (!getUint32(type))
        return false;

    // CIMParameter.classOrigin
    if (!getName(classOrigin))
        return false;

    // CIMParameter.propagated
    if (!getBoolean(propagated))
        return false;

    rep = new CIMMethodRep(
        name, CIMType(type), classOrigin, propagated);

    // CIMMethod.qualifiers:
    if (!getQualifierList(rep->_qualifiers))
        return false;

    // CIMMethod.parameters:
    {
        Uint32 n;

        if (!getUint32(n))
            return false;

        for (Uint32 i = 0; i < n; i++)
        {
            CIMParameter tmp;

            if (!getParameter(tmp))
                return false;

            rep->_parameters.append(tmp);
        }
    }

    // Replace rep:

    CIMMethodRep*& tmp = *((CIMMethodRep**)&x);

    if (tmp)
        tmp->Dec();

    tmp = rep;

    return true;
}

void CIMBuffer::putPropertyList(const CIMPropertyList& x)
{
    CIMPropertyListRep* rep = *((CIMPropertyListRep**)&x);

    putBoolean(rep->isNull);

    if (!rep->isNull)
    {
        Uint32 n = rep->propertyNames.size();
        putUint32(n);

        for (Uint32 i = 0; i < n; i++)
            putName(rep->propertyNames[i]);

        Uint32 tagCount = rep->cimNameTags.size();
        putUint32(tagCount);
      
        for(Uint32 j = 0; j < tagCount; j++)
        {
            putUint32(rep->cimNameTags[j]);
        } 
    }
}

bool CIMBuffer::getPropertyList(CIMPropertyList& x)
{
    Boolean isNull;

    if (!getBoolean(isNull))
        return false;

    if (isNull)
    {
        x.~CIMPropertyList();
        new(&x) CIMPropertyList();
    }
    else
    {
        Array<CIMName> names;

        Uint32 n;

        if (!getUint32(n))
            return false;

        for (Uint32 i = 0; i < n; i++)
        {
            CIMName name;

            if (!getName(name))
                return false;

            names.append(name);
        }

        x.~CIMPropertyList();
        new(&x) CIMPropertyList(names);
      
        Uint32 tagCount;
        if (!getUint32(tagCount))
        {
            return false;
        }

        for(Uint32 j=0;j<tagCount;j++)
        {
            Uint32 tag;
            if (!getUint32(tag))
            {
                return false;
            }
            x.appendCIMNameTag(tag);
        }

    }

    return true;
}

void CIMBuffer::putObject(
    const CIMObject& x,
    bool includeHostAndNamespace,
    bool includeKeyBindings)
{
    _putMagic(OBJECT_MAGIC);

    if (x.isUninitialized())
    {
        putBoolean(false);
        return;
    }
    else
        putBoolean(true);

    if (x.isInstance())
    {
        putUint8('I');
        putInstance(
            CIMInstance(x), includeHostAndNamespace, includeKeyBindings);
    }
    else
    {
        putUint8('C');
        putClass(CIMClass(x));
    }
}

bool CIMBuffer::getObject(CIMObject& x)
{
    Boolean initialized;
    Uint8 tag;

    if (!_testMagic(OBJECT_MAGIC))
        return false;

    if (!getBoolean(initialized))
        return false;

    if (!initialized)
    {
        x = CIMObject();
        return true;
    }

    if (!getUint8(tag))
        return false;

    if (tag == 'I')
    {
        CIMInstance tmp;

        if (!getInstance(tmp))
            return false;

        x.~CIMObject();
        new(&x) CIMObject(tmp);
        return true;
    }
    else if (tag == 'C')
    {
        CIMClass tmp;

        if (!getClass(tmp))
            return false;

        x.~CIMObject();
        new(&x) CIMObject(tmp);
        return true;
    }
    else
        return false;

}

void CIMBuffer::putParamValue(const CIMParamValue& x)
{
    const CIMParamValueRep* rep = *((const CIMParamValueRep**)&x);

    // CIMParamValue.parameterName:
    putString(rep->_parameterName);

    // CIMParamValue.value:
    putValue(rep->_value);

    // CIMParamValue.isTyped:
    putBoolean(rep->_isTyped);
}

bool CIMBuffer::getParamValue(CIMParamValue& x)
{
    String parameterName;
    CIMValue value;
    Boolean isTyped;

    // CIMParamValue.parameterName:
    if (!getString(parameterName))
        return false;

    // CIMParamValue.value:
    if (!getValue(value))
        return false;

    // CIMParamValue.isTyped:
    if (!getBoolean(isTyped))
        return false;

    x.~CIMParamValue();
    new(&x) CIMParamValue(parameterName, value, isTyped);

    return true;
}

void CIMBuffer::putPresent(Boolean flag)
{
    if (flag)
        putUint32(PRESENT_MAGIC);
    else
        putUint32(ABSENT_MAGIC);
}

bool CIMBuffer::getPresent(Boolean& flag)
{
    Uint32 tmp;

    if (!getUint32(tmp))
        return false;

    if (tmp == PRESENT_MAGIC)
    {
        flag = true;
        return true;
    }
    else if (tmp == ABSENT_MAGIC)
    {
        flag = false;
        return true;
    }

    return false;
}

void CIMBuffer::putInstanceA(
    const Array<CIMInstance>& x,
    bool includeHostAndNamespace,
    bool includeKeyBindings)
{
    Uint32 n = x.size();
    putUint32(n);

    for (size_t i = 0; i < n; i++)
        putInstance(x[i], includeHostAndNamespace, includeKeyBindings);
}


void CIMBuffer::putSCMOClass(const SCMOClass& scmoClass)
{
    SCMOStreamer::serializeClass(*this, scmoClass);
}

bool CIMBuffer::getSCMOClass(SCMOClass& scmoClass)
{
    return SCMOStreamer::deserializeClass(*this, scmoClass);
}


void CIMBuffer::putSCMOInstanceA(Array<SCMOInstance>& x)
{
    Uint32 n = x.size();
    _grow(n<<13);

    putUint32(n);


    SCMOStreamer scmoStreamer(*this,x);
    scmoStreamer.serialize();
}

bool CIMBuffer::getSCMOInstanceA(Array<SCMOInstance>& x)
{
    Uint32 n;

    if (!getUint32(n))
        return false;

    SCMOStreamer scmoStreamer(*this,x);
    return scmoStreamer.deserialize();
}



PEGASUS_NAMESPACE_END
