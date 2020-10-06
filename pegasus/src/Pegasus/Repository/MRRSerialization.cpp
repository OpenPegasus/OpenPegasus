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

#include "MRRSerialization.h"

#define RETURN_FAILURE \
    do \
    { \
        printf("***** RETURN_FAILURE: %s(%d)\n", __FILE__, __LINE__); \
        return -1; \
    } \
    while (0)

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// Limitations:
//
//     1. Does not handle CIMObjects that contain CIMClass.
//
//==============================================================================

static Uint32 _INSTANCE_MAGIC = 0x20D54A36;
static Uint32 _NAMESPACE_MAGIC = 0x06979333;

class Str
{
public:
    Str(const String& s) : _cstr(s.getCString()) { }
    Str(const CIMName& n) : _cstr(n.getString().getCString()) { }
    Str(const CIMNamespaceName& n) : _cstr(n.getString().getCString()) { }
    Str(const Exception& e) : _cstr(e.getMessage().getCString()) { }
    Str(const CIMDateTime& x) : _cstr(x.toString().getCString()) { }
    Str(const CIMObjectPath& x) : _cstr(x.toString().getCString()) { }
    const char* operator*() const { return (const char*)_cstr; }
    operator const char*() const { return (const char*)_cstr; }
private:
    CString _cstr;
};

static void _PutUint8(Buffer& out, Uint8 x)
{
    out.append(x);
}

static void _PutBoolean(Buffer& out, Boolean x)
{
    _PutUint8(out, x ? 1 : 0);
}

static void _PutUint16(Buffer& out, Uint16 x)
{
    Uint8 x0 = Uint8((x >> 8) & 0x00FF);
    Uint8 x1 = Uint8((x >> 0) & 0x00FF);
    out.append(x0);
    out.append(x1);
}

static void _PutUint32(Buffer& out, Uint32 x)
{
    Uint8 x0 = Uint8((x >> 24) & 0x000000FF);
    Uint8 x1 = Uint8((x >> 16) & 0x000000FF);
    Uint8 x2 = Uint8((x >>  8) & 0x000000FF);
    Uint8 x3 = Uint8((x >>  0) & 0x000000FF);
    out.append(x0, x1, x2, x3);
}

static void _PutUint64(Buffer& out, Uint64 x)
{
    Uint8 x0 = Uint8((x >> 56) & 0x00000000000000FF);
    Uint8 x1 = Uint8((x >> 48) & 0x00000000000000FF);
    Uint8 x2 = Uint8((x >> 40) & 0x00000000000000FF);
    Uint8 x3 = Uint8((x >> 32) & 0x00000000000000FF);
    Uint8 x4 = Uint8((x >> 24) & 0x00000000000000FF);
    Uint8 x5 = Uint8((x >> 16) & 0x00000000000000FF);
    Uint8 x6 = Uint8((x >>  8) & 0x00000000000000FF);
    Uint8 x7 = Uint8((x >>  0) & 0x00000000000000FF);
    out.append(x0, x1, x2, x3, x4, x5, x6, x7);
}

static void _PutSint8(Buffer& out, Sint8 x)
{
    return _PutUint8(out, Uint8(x));
}

static void _PutSint16(Buffer& out, Sint16 x)
{
    return _PutUint16(out, Uint16(x));
}

static void _PutSint32(Buffer& out, Sint32 x)
{
    return _PutUint32(out, Uint32(x));
}

static void _PutSint64(Buffer& out, Sint64 x)
{
    return _PutUint64(out, Uint64(x));
}

static void _PutReal32(Buffer& out, Real32 x)
{
    return _PutUint32(out, *((Uint32*)(void*)&x));
}

static void _PutReal64(Buffer& out, Real64 x)
{
    return _PutUint64(out, *((Uint64*)(void*)&x));
}

static void _PutChar16(Buffer& out, Char16 x)
{
    return _PutUint16(out, Uint16(x));
}

static int _GetUint8(const Buffer& in, size_t& pos, Uint8& x)
{
    if (in.size() < 1)
        RETURN_FAILURE;

    const Uint8* p = (const Uint8*)(in.getData() + pos);
    x = p[0];
    pos++;

    return 0;
}

static int _GetBoolean(const Buffer& in, size_t& pos, Boolean& x)
{
    Uint8 tmp;

    if (_GetUint8(in, pos, tmp) != 0)
        RETURN_FAILURE;

    if (tmp != 0 && tmp != 1)
        RETURN_FAILURE;

    x = Boolean(tmp);
    return 0;
}

static int _GetUint16(const Buffer& in, size_t& pos, Uint16& x)
{
    if (in.size() < 2)
        RETURN_FAILURE;

    const Uint8* p = (const Uint8*)(in.getData() + pos);
    Uint16 x0 = p[0];
    Uint16 x1 = p[1];
    x = (x0 << 8) | x1;
    pos += 2;

    return 0;
}

static int _GetUint32(const Buffer& in, size_t& pos, Uint32& x)
{
    if (in.size() < 4)
        RETURN_FAILURE;

    const Uint8* p = (const Uint8*)(in.getData() + pos);
    Uint32 x0 = p[0];
    Uint32 x1 = p[1];
    Uint32 x2 = p[2];
    Uint32 x3 = p[3];
    x = (x0 << 24) | (x1 << 16) | (x2 << 8) | x3;
    pos += 4;

    return 0;
}

static int _GetUint64(const Buffer& in, size_t& pos, Uint64& x)
{
    if (in.size() < 8)
        RETURN_FAILURE;

    const Uint8* p = (const Uint8*)(in.getData() + pos);
    Uint64 x0 = p[0];
    Uint64 x1 = p[1];
    Uint64 x2 = p[2];
    Uint64 x3 = p[3];
    Uint64 x4 = p[4];
    Uint64 x5 = p[5];
    Uint64 x6 = p[6];
    Uint64 x7 = p[7];
    x = (x0 << 56) | (x1 << 48) | (x2 << 40) | (x3 << 32) |
        (x4 << 24) | (x5 << 16) | (x6 << 8) | x7;
    pos += 8;

    return 0;
}

static int _GetSint8(const Buffer& in, size_t& pos, Sint8& x)
{
    return _GetUint8(in, pos, *((Uint8*)&x));
}

static int _GetSint16(const Buffer& in, size_t& pos, Sint16& x)
{
    return _GetUint16(in, pos, *((Uint16*)&x));
}

static int _GetSint32(const Buffer& in, size_t& pos, Sint32& x)
{
    return _GetUint32(in, pos, *((Uint32*)&x));
}

static int _GetSint64(const Buffer& in, size_t& pos, Sint64& x)
{
    return _GetUint64(in, pos, *((Uint64*)&x));
}

static int _GetReal32(const Buffer& in, size_t& pos, Real32& x)
{
    return _GetUint32(in, pos, *((Uint32*)&x));
}

static int _GetReal64(const Buffer& in, size_t& pos, Real64& x)
{
    return _GetUint64(in, pos, *((Uint64*)&x));
}

static int _GetChar16(const Buffer& in, size_t& pos, Char16& x)
{
    return _GetUint16(in, pos, *((Uint16*)&x));
}

static void _PutString(Buffer& out, const String& str)
{
    CString cstr(str.getCString());
    Uint32 size = strlen(cstr);

    // Pack size:
    _PutUint32(out, size);

    // Pack UTF8 characters:
    out.append(cstr, size);
}

static void _PutDateTime(Buffer& out, const CIMDateTime& x)
{
    _PutString(out, x.toString());
}

static void _PutObjectPath(Buffer& out, const CIMObjectPath& x)
{
    // Serialize host:

    _PutString(out, x.getHost());

    //  Serialize namespace:

    _PutString(out, x.getNameSpace().getString());

    // Serialize class name:

    _PutString(out, x.getClassName().getString());

    // Serialize key bindings:

    const Array<CIMKeyBinding>& kbs = x.getKeyBindings();
    _PutUint32(out, kbs.size());

    for (Uint32 i = 0, n = kbs.size(); i < n; i++)
    {
        const CIMKeyBinding& kb = kbs[i];

        // Serialize name:

        _PutString(out, kb.getName().getString());

        // Serialize type:

        _PutUint8(out, kb.getType());

        // Serialize value:

        _PutString(out, kb.getValue());
    }
}

static int _PutValue(Buffer& out, const CIMValue& cv)
{
    // Serialize type (set MSB if array).

    Uint8 type = Uint8(cv.getType());

    if (cv.isArray())
        type |= 0x80;

    _PutUint8(out, type);

    // Serialize the value itself:

    if (cv.isArray())
    {
        switch (cv.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                Array<Boolean> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutBoolean(out, x[i]);
                break;
            }
            case CIMTYPE_UINT8:
            {
                Array<Uint8> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutUint8(out, x[i]);
                break;
            }
            case CIMTYPE_SINT8:
            {
                Array<Sint8> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutSint8(out, x[i]);
                break;
            }
            case CIMTYPE_UINT16:
            {
                Array<Uint16> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutUint16(out, x[i]);
                break;
            }
            case CIMTYPE_SINT16:
            {
                Array<Sint16> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutSint16(out, x[i]);
                break;
            }
            case CIMTYPE_UINT32:
            {
                Array<Uint32> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutUint32(out, x[i]);
                break;
            }
            case CIMTYPE_SINT32:
            {
                Array<Sint32> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutSint32(out, x[i]);
                break;
            }
            case CIMTYPE_UINT64:
            {
                Array<Uint64> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutUint64(out, x[i]);
                break;
            }
            case CIMTYPE_SINT64:
            {
                Array<Sint64> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutSint64(out, x[i]);
                break;
            }
            case CIMTYPE_REAL32:
            {
                Array<Real32> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutReal32(out, x[i]);
                break;
            }
            case CIMTYPE_REAL64:
            {
                Array<Real64> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutReal64(out, x[i]);
                break;
            }
            case CIMTYPE_CHAR16:
            {
                Array<Char16> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutChar16(out, x[i]);
                break;
            }
            case CIMTYPE_STRING:
            {
                Array<String> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutString(out, x[i]);
                break;
            }
            case CIMTYPE_DATETIME:
            {
                Array<CIMDateTime> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutDateTime(out, x[i]);
                break;
            }
            case CIMTYPE_REFERENCE:
            {
                Array<CIMObjectPath> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    _PutObjectPath(out, x[i]);
                break;
            }
            case CIMTYPE_OBJECT:
            {
                Array<CIMObject> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                {
                    if (!x[i].isInstance())
                        RETURN_FAILURE;

                    CIMInstance ci(x[i]);

                    MRRSerializeInstance(out, ci);
                }
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                Array<CIMInstance> x;
                cv.get(x);
                _PutUint32(out, x.size());

                for (Uint32 i = 0; i < x.size(); i++)
                    MRRSerializeInstance(out, x[i]);
                break;
            }
        }
    }
    else
    {
        switch (cv.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                Boolean x;
                cv.get(x);
                _PutBoolean(out, x);
                break;
            }
            case CIMTYPE_UINT8:
            {
                Uint8 x;
                cv.get(x);
                _PutUint8(out, x);
                break;
            }
            case CIMTYPE_SINT8:
            {
                Sint8 x;
                cv.get(x);
                _PutSint8(out, x);
                break;
            }
            case CIMTYPE_UINT16:
            {
                Uint16 x;
                cv.get(x);
                _PutUint16(out, x);
                break;
            }
            case CIMTYPE_SINT16:
            {
                Sint16 x;
                cv.get(x);
                _PutSint16(out, x);
                break;
            }
            case CIMTYPE_UINT32:
            {
                Uint32 x;
                cv.get(x);
                _PutUint32(out, x);
                break;
            }
            case CIMTYPE_SINT32:
            {
                Sint32 x;
                cv.get(x);
                _PutSint32(out, x);
                break;
            }
            case CIMTYPE_UINT64:
            {
                Uint64 x;
                cv.get(x);
                _PutUint64(out, x);
                break;
            }
            case CIMTYPE_SINT64:
            {
                Sint64 x;
                cv.get(x);
                _PutSint64(out, x);
                break;
            }
            case CIMTYPE_REAL32:
            {
                Real32 x;
                cv.get(x);
                _PutReal32(out, x);
                break;
            }
            case CIMTYPE_REAL64:
            {
                Real64 x;
                cv.get(x);
                _PutReal64(out, x);
                break;
            }
            case CIMTYPE_CHAR16:
            {
                Char16 x;
                cv.get(x);
                _PutChar16(out, x);
                break;
            }
            case CIMTYPE_STRING:
            {
                String x;
                cv.get(x);
                _PutString(out, x);
                break;
            }
            case CIMTYPE_DATETIME:
            {
                CIMDateTime x;
                cv.get(x);
                _PutDateTime(out, x);
                break;
            }
            case CIMTYPE_REFERENCE:
            {
                CIMObjectPath x;
                cv.get(x);
                _PutObjectPath(out, x);
                break;
            }
            case CIMTYPE_OBJECT:
            {
                CIMObject co;
                cv.get(co);

                if (!co.isInstance())
                    RETURN_FAILURE;

                CIMInstance ci(co);

                MRRSerializeInstance(out, ci);
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                CIMInstance ci;
                cv.get(ci);
                MRRSerializeInstance(out, ci);
                break;
            }
        }
    }

    return 0;
}

static int _GetString(
    const Buffer& in, size_t& pos, String& str)
{
    // Deserialize size:

    Uint32 size;

    if (_GetUint32(in, pos, size) != 0)
        RETURN_FAILURE;

    // Read string:

    str.append(&in[pos], size);
    pos += size;

    return 0;
}

static int _GetDateTime(const Buffer& in, size_t& pos, CIMDateTime& x)
{
    String str;

    if (_GetString(in, pos, str) != 0)
        RETURN_FAILURE;

    try
    {
        x.set(str);
    }
    catch (...)
    {
        RETURN_FAILURE;
    }
    return 0;
}

static int _GetObjectPath(
    const Buffer& in,
    size_t& pos,
    CIMObjectPath& cop)
{
    // Deserialize host:

    String host;

    if (_GetString(in, pos, host) != 0)
        RETURN_FAILURE;

    // Deserialize namespace:

    CIMNamespaceName nameSpace;
    {
        String nameSpaceString;

        if (_GetString(in, pos, nameSpaceString) != 0)
            RETURN_FAILURE;

        if (nameSpaceString.size() != 0)
            nameSpace = nameSpaceString;
    }

    // Deserialize className:

    CIMName className;
    {
        String classNameString;

        if (_GetString(in, pos, classNameString) != 0)
            RETURN_FAILURE;

        if (classNameString.size() != 0)
            className = classNameString;
    }

    // Deserialize the number of key bindings:

    Uint32 size;

    if (_GetUint32(in, pos, size) != 0)
        RETURN_FAILURE;

    // Deserialize the key bindings.

    Array<CIMKeyBinding> kbs;

    for (Uint32 i = 0; i < size; i++)
    {
        // Deserialize name:

        String name;

        if (_GetString(in, pos, name) != 0)
            RETURN_FAILURE;

        // Deserialize and check type:

        Uint8 type;

        if (_GetUint8(in, pos, type) != 0)
            RETURN_FAILURE;

        if (type != CIMKeyBinding::BOOLEAN && type != CIMKeyBinding::STRING &&
            type != CIMKeyBinding::NUMERIC && type != CIMKeyBinding::REFERENCE)
        {
            RETURN_FAILURE;
        }

        // Deserialize value:

        String value;

        if (_GetString(in, pos, value) != 0)
            RETURN_FAILURE;

        // Add key binding:

        try
        {
            kbs.append(CIMKeyBinding(name, value, CIMKeyBinding::Type(type)));
        }
        catch (...)
        {
            RETURN_FAILURE;
        }
    }

    // Create the object path:

    try
    {
        cop = CIMObjectPath(host, nameSpace, className, kbs);
    }
    catch (...)
    {
        RETURN_FAILURE;
    }

    return 0;
}

static int _GetValue(
    const Buffer& in,
    size_t& pos,
    CIMValue& value)
{
    value.clear();

    // Deserialize type and isArray:

    Boolean isArray;
    Uint32 type;
    {
        Uint8 tmp;

        if (_GetUint8(in, pos, tmp) != 0)
            RETURN_FAILURE;

        isArray = tmp & 0x80;
        type = CIMType(tmp & 0x7F);
    }

    // Deserialize the value itself:

    if (isArray)
    {
        Uint32 size;

        if (_GetUint32(in, pos, size) != 0)
            RETURN_FAILURE;

        switch (type)
        {
            case CIMTYPE_BOOLEAN:
            {
                Array<Boolean> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Boolean x;

                    if (_GetBoolean(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_UINT8:
            {
                Array<Uint8> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Uint8 x;

                    if (_GetUint8(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_SINT8:
            {
                Array<Sint8> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Sint8 x;

                    if (_GetSint8(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_UINT16:
            {
                Array<Uint16> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Uint16 x;

                    if (_GetUint16(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_SINT16:
            {
                Array<Sint16> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Sint16 x;

                    if (_GetSint16(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_UINT32:
            {
                Array<Uint32> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Uint32 x;

                    if (_GetUint32(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_SINT32:
            {
                Array<Sint32> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Sint32 x;

                    if (_GetSint32(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_UINT64:
            {
                Array<Uint64> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Uint64 x;

                    if (_GetUint64(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_SINT64:
            {
                Array<Sint64> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Sint64 x;

                    if (_GetSint64(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_REAL32:
            {
                Array<Real32> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Real32 x;

                    if (_GetReal32(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_REAL64:
            {
                Array<Real64> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Real64 x;

                    if (_GetReal64(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_CHAR16:
            {
                Array<Char16> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    Char16 x;

                    if (_GetChar16(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_STRING:
            {
                Array<String> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    String x;

                    if (_GetString(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_DATETIME:
            {
                Array<CIMDateTime> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    CIMDateTime x;

                    if (_GetDateTime(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_REFERENCE:
            {
                Array<CIMObjectPath> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    CIMObjectPath x;

                    if (_GetObjectPath(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            case CIMTYPE_OBJECT:
            {
                Array<CIMObject> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    CIMInstance x;

                    if (MRRDeserializeInstance(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(CIMObject(x));
                }
                value.set(a);
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                Array<CIMInstance> a;

                for (Uint32 i = 0; i < size; i++)
                {
                    CIMInstance x;

                    if (MRRDeserializeInstance(in, pos, x) != 0)
                        RETURN_FAILURE;

                    a.append(x);
                }
                value.set(a);
                break;
            }
            default:
                RETURN_FAILURE;
        }
    }
    else
    {
        switch (type)
        {
            case CIMTYPE_BOOLEAN:
            {
                Boolean x;

                if (_GetBoolean(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_UINT8:
            {
                Uint8 x;

                if (_GetUint8(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_SINT8:
            {
                Sint8 x;

                if (_GetSint8(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_UINT16:
            {
                Uint16 x;

                if (_GetUint16(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_SINT16:
            {
                Sint16 x;

                if (_GetSint16(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_UINT32:
            {
                Uint32 x;

                if (_GetUint32(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_SINT32:
            {
                Sint32 x;

                if (_GetSint32(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_UINT64:
            {
                Uint64 x;

                if (_GetUint64(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_SINT64:
            {
                Sint64 x;

                if (_GetSint64(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_REAL32:
            {
                Real32 x;

                if (_GetReal32(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_REAL64:
            {
                Real64 x;

                if (_GetReal64(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_CHAR16:
            {
                Char16 x;

                if (_GetChar16(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_STRING:
            {
                String x;

                if (_GetString(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_DATETIME:
            {
                CIMDateTime x;

                if (_GetDateTime(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_REFERENCE:
            {
                CIMObjectPath x;

                if (_GetObjectPath(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            case CIMTYPE_OBJECT:
            {
                CIMInstance x;

                if (MRRDeserializeInstance(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(CIMObject(x));
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                CIMInstance x;

                if (MRRDeserializeInstance(in, pos, x) != 0)
                    RETURN_FAILURE;

                value.set(x);
                break;
            }
            default:
                RETURN_FAILURE;
        }
    }

    return 0;
}

void MRRSerializeInstance(Buffer& out, const CIMInstance& ci)
{
    // Serialize magic number:

    _PutUint32(out, _INSTANCE_MAGIC);

    // Serialize object path:

    _PutObjectPath(out, ci.getPath());

    // Serialize properties:

    _PutUint32(out, ci.getPropertyCount());

    for (Uint32 i = 0, n = ci.getPropertyCount(); i < n; i++)
    {
        const CIMConstProperty cp = ci.getProperty(i);

        // Serialize property name:

        _PutString(out, cp.getName().getString());

        // Serialize the value:

        _PutValue(out, cp.getValue());
    }
}

int MRRDeserializeInstance(
    const Buffer& in,
    size_t& pos,
    CIMInstance& ci)
{
    // Deserialize magic number:

    Uint32 magic;

    if (_GetUint32(in, pos, magic) != 0 || magic != _INSTANCE_MAGIC)
        RETURN_FAILURE;

    // Deserialize object path:

    CIMObjectPath cop;

    if (_GetObjectPath(in, pos, cop) != 0)
        RETURN_FAILURE;

    // Create the instance:

    try
    {
        ci = CIMInstance(cop.getClassName());
        ci.setPath(cop);
    }
    catch (...)
    {
        RETURN_FAILURE;
    }

    // Get property count:

    Uint32 propertyCount = 0;

    if (_GetUint32(in, pos, propertyCount) != 0)
        RETURN_FAILURE;

    // Deserialize properties:

    for (Uint32 i = 0; i < propertyCount; i++)
    {
        // Deserialize property name:

        String name;

        if (_GetString(in, pos, name) != 0)
            RETURN_FAILURE;

        // Deserialize property value:

        CIMValue value;

        if (_GetValue(in, pos, value) != 0)
            RETURN_FAILURE;

        // Add property to instance.

        try
        {
            ci.addProperty(CIMProperty(name, value));
        }
        catch (Exception& e)
        {
            RETURN_FAILURE;
        }
    }

    return 0;
}

void MRRSerializeNameSpace(
    Buffer& out,
    const CIMNamespaceName& nameSpace)
{
    // Serialize magic number:

    _PutUint32(out, _NAMESPACE_MAGIC);

    // Serialize namespace string:

    _PutString(out, nameSpace.getString());
}

int MRRDeserializeNameSpace(
    const Buffer& in,
    size_t& pos,
    CIMNamespaceName& nameSpace)
{
    // Deserialize magic number:

    Uint32 magic;

    if (_GetUint32(in, pos, magic) != 0 || magic != _NAMESPACE_MAGIC)
        RETURN_FAILURE;

    // Deserialize namespace string:

    String tmp;

    if (_GetString(in, pos, tmp) != 0)
        RETURN_FAILURE;

    try
    {
        nameSpace = tmp;
    }
    catch (...)
    {
        return -1;
    }
    return 0;
}

PEGASUS_NAMESPACE_END
