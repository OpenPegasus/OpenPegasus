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

#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/CIMClassRep.h>
#include <Pegasus/Common/CIMInstanceRep.h>
#include <Pegasus/Common/CIMMethodRep.h>
#include <Pegasus/Common/CIMParameterRep.h>
#include <Pegasus/Common/CIMPropertyRep.h>
#include <Pegasus/Common/CIMQualifierRep.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMValueRep.h>
#include "Packer.h"
#include "BinaryStreamer.h"

#define MAGIC_BYTE Uint8(0x11)
#define VERSION_NUMBER Uint8(1)

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

enum BinaryObjectType
{
    BINARY_CLASS,
    BINARY_INSTANCE,
    BINARY_QUALIFIER_DECL
};

static inline void _packMagicByte(Buffer& out)
{
    Packer::packUint8(out, MAGIC_BYTE);
}

static void _checkMagicByte(const Buffer& in, Uint32& pos)
{
    Uint8 magicByte;
    Packer::unpackUint8(in, pos, magicByte);

    if (magicByte != MAGIC_BYTE)
        throw BinException("Bad magic byte");
}

struct Header
{
    // A version number for this message.
    Uint8 versionNumber;

    // The object type (see BinaryObjectType enum).
    Uint8 objectType;
};

static void _packHeader(Buffer& out, Uint8 objectType)
{
    Packer::packUint8(out, VERSION_NUMBER);
    Packer::packUint8(out, objectType);
}

static void _checkHeader(
    const Buffer& in, Uint32& pos, Uint8 expectedObjectType)
{
    Header header;
    Packer::unpackUint8(in, pos, header.versionNumber);
    Packer::unpackUint8(in, pos, header.objectType);

    if (header.objectType != expectedObjectType)
        throw BinException("Unexpected object type");

    if (header.versionNumber != VERSION_NUMBER)
        throw BinException("Unsupported version");
}

inline void _unpack(const Buffer& in, Uint32& pos, Boolean& x)
{
    Packer::unpackBoolean(in, pos, x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Uint8& x)
{
    Packer::unpackUint8(in, pos, x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Sint8& x)
{
    Packer::unpackUint8(in, pos, (Uint8&)x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Uint16& x)
{
    Packer::unpackUint16(in, pos, x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Sint16& x)
{
    Packer::unpackUint16(in, pos, (Uint16&)x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Uint32& x)
{
    Packer::unpackUint32(in, pos, x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Sint32& x)
{
    Packer::unpackUint32(in, pos, (Uint32&)x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Uint64& x)
{
    Packer::unpackUint64(in, pos, x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Sint64& x)
{
    Packer::unpackUint64(in, pos, (Uint64&)x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Real32& x)
{
    Packer::unpackReal32(in, pos, x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Real64& x)
{
    Packer::unpackReal64(in, pos, x);
}

inline void _unpack(const Buffer& in, Uint32& pos, Char16& x)
{
    Packer::unpackChar16(in, pos, x);
}

inline void _unpack(const Buffer& in, Uint32& pos, String& x)
{
    Packer::unpackString(in, pos, x);
}

void _unpack(const Buffer& in, Uint32& pos, CIMDateTime& x)
{
    String tmp;
    Packer::unpackString(in, pos, tmp);
    x.set(tmp);
}

void _unpack(const Buffer& in, Uint32& pos, CIMObjectPath& x)
{
    String tmp;
    Packer::unpackString(in, pos, tmp);
    x.set(tmp);
}

void _unpack(const Buffer& in, Uint32& pos, CIMObject& x)
{
    String tmp_String;
    Packer::unpackString(in, pos, tmp_String);

    if (tmp_String.size() == 0)
    {
        // This should not occur since _unpackValue() won't call _unpack()
        // if the value is Null.
        PEGASUS_ASSERT(false);
    }
    else
    {
        // Convert the non-NULL string to a CIMObject (containing either a
        // CIMInstance or a CIMClass).

        // First we need to create a new "temporary" XmlParser that is
        // just the value of the Embedded Object in String representation.
        CString cstr = tmp_String.getCString();
        char* tmp_buffer = (char*)(const char*)cstr;
        XmlParser tmp_parser(tmp_buffer);

        // The next bit of logic constructs a CIMObject from the Embedded
        // Object String.
        // It is similar to the method XmlReader::getValueObjectElement().
        CIMInstance cimInstance;
        CIMClass cimClass;

        if (XmlReader::getInstanceElement(tmp_parser, cimInstance))
        {
            x = CIMObject(cimInstance);
        }
        else if (XmlReader::getClassElement(tmp_parser, cimClass))
        {
            x = CIMObject(cimClass);
        }
        else
        {
            // change "element" to "embedded object"
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_INSTANCE_OR_CLASS_ELEMENT",
                "Expected INSTANCE or CLASS element");

            throw XmlValidationError(0, mlParms);
        }
    }
}
void _unpack(const Buffer& in, Uint32& pos, CIMInstance& x)
{
    CIMObject tmp;
    _unpack(in, pos, tmp);
    x = CIMInstance(tmp);
}

template<class T>
struct UnpackArray
{
    static void func(
        const Buffer& in, Uint32& pos, Uint32 n, CIMValue& value)
    {
        Array<T> array;
        array.reserveCapacity(n);

        for (Uint32 i = 0; i < n; i++)
        {
            T tmp;
            _unpack(in, pos, tmp);
            array.append(tmp);
        }

        value.set(array);
    }
};

template<class T>
struct UnpackScalar
{
    static void func(
        const Buffer& in, Uint32& pos, CIMValue& value)
    {
        T tmp;
        _unpack(in, pos, tmp);
        value.set(tmp);
    }
};

template<class OBJECT>
struct UnpackQualifiers
{
    static void func(const Buffer& in, Uint32& pos, OBJECT& x)
    {
        Uint32 n;
        Packer::unpackSize(in, pos, n);

        CIMQualifier q;

        for (size_t i = 0; i < n; i++)
        {
            BinaryStreamer::_unpackQualifier(in, pos, q);
            x.addQualifier(q);
        }
    }
};

template<class REP>
struct PackQualifiers
{
    static void func(Buffer& out, REP* rep)
    {
        Uint32 n = rep->getQualifierCount();
        Packer::packSize(out, n);

        for (Uint32 i = 0; i < n; i++)
            BinaryStreamer::_packQualifier(out, rep->getQualifier(i));
    }
};

template<class OBJECT>
struct UnpackProperties
{
    static void func(const Buffer& in, Uint32& pos, OBJECT& x)
    {
        Uint32 n;
        Packer::unpackSize(in, pos, n);

        CIMProperty p;

        for (size_t i = 0; i < n; i++)
        {
            BinaryStreamer::_unpackProperty(in, pos, p);
            x.addProperty(p);
        }
    }
};

template<class OBJECT>
struct UnpackMethods
{
    static void func(const Buffer& in, Uint32& pos, OBJECT& x)
    {
        Uint32 n;
        Packer::unpackSize(in, pos, n);

        CIMMethod m;

        for (size_t i = 0; i < n; i++)
        {
            BinaryStreamer::_unpackMethod(in, pos, m);
            x.addMethod(m);
        }
    }
};

void BinaryStreamer::_packName(Buffer& out, const CIMName& x)
{
    Packer::packString(out, x.getString());
}

void BinaryStreamer::_unpackName(
    const Buffer& in, Uint32& pos, CIMName& x)
{
    String tmp;
    Packer::unpackString(in, pos, tmp);
    x = tmp.size() ? CIMNameCast(tmp) : CIMName();
}

void BinaryStreamer::_packQualifier(Buffer& out, const CIMQualifier& x)
{
    CIMQualifierRep* rep = x._rep;

    _packMagicByte(out);
    _packName(out, rep->getName());
    _packValue(out, rep->getValue());
    _packFlavor(out, rep->getFlavor());
    Packer::packBoolean(out, rep->getPropagated());
}

void BinaryStreamer::_unpackQualifier(
    const Buffer& in, Uint32& pos, CIMQualifier& x)
{
    _checkMagicByte(in, pos);

    CIMName name;
    _unpackName(in, pos, name);

    CIMValue value;
    _unpackValue(in, pos, value);

    CIMFlavor flavor;
    BinaryStreamer::_unpackFlavor(in, pos, flavor);

    Boolean propagated;
    Packer::unpackBoolean(in, pos, propagated);

    x = CIMQualifier(name, value, flavor, propagated);
}

void BinaryStreamer::_packValue(Buffer& out, const CIMValue& x)
{
    CIMValueRep* rep = x._rep;

    _packMagicByte(out);
    _packType(out, x.getType());
    Packer::packBoolean(out, x.isArray());

    Uint32 n = x.getArraySize();

    if (x.isArray())
        Packer::packSize(out, n);

    Packer::packBoolean(out, x.isNull());

    if (x.isNull())
        return;

    if (x.isArray())
    {
        switch (x.getType())
        {
            case CIMTYPE_BOOLEAN:
                Packer::packBoolean(
                    out, CIMValueType<Boolean>::aref(rep).getData(), n);
                break;

            case CIMTYPE_SINT8:
            case CIMTYPE_UINT8:
                Packer::packUint8(
                    out, CIMValueType<Uint8>::aref(rep).getData(), n);
                break;

            case CIMTYPE_SINT16:
            case CIMTYPE_UINT16:
            case CIMTYPE_CHAR16:
                Packer::packUint16(
                    out, CIMValueType<Uint16>::aref(rep).getData(), n);
                break;

            case CIMTYPE_SINT32:
            case CIMTYPE_UINT32:
            case CIMTYPE_REAL32:
                Packer::packUint32(
                    out, CIMValueType<Uint32>::aref(rep).getData(), n);
                break;

            case CIMTYPE_SINT64:
            case CIMTYPE_UINT64:
            case CIMTYPE_REAL64:
                Packer::packUint64(
                    out, CIMValueType<Uint64>::aref(rep).getData(), n);
                break;

            case CIMTYPE_STRING:
                Packer::packString(out,
                    CIMValueType<String>::aref(rep).getData(), n);
                break;

            case CIMTYPE_DATETIME:
            {
                const Array<CIMDateTime>& a =
                    CIMValueType<CIMDateTime>::aref(rep);

                for (Uint32 i = 0; i < n; i++)
                    Packer::packString(out, a[i].toString());
                break;
            }

            case CIMTYPE_REFERENCE:
            {
                const Array<CIMObjectPath>& a =
                    CIMValueType<CIMObjectPath>::aref(rep);

                for (Uint32 i = 0; i < n; i++)
                    Packer::packString(out, a[i].toString());
                break;
            }

            case CIMTYPE_OBJECT:
            {
                const Array<CIMObject>& a =
                    CIMValueType<CIMObject>::aref(rep);

                for (Uint32 i = 0; i < n; i++)
                    Packer::packString(out, a[i].toString());
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                const Array<CIMInstance>& a =
                    CIMValueType<CIMInstance>::aref(rep);

                for (Uint32 i = 0; i < n; i++)
                {
                    CIMObject tmp(a[i]);
                    Packer::packString(out, tmp.toString());
                }
                break;
            }
        }
    }
    else
    {
        switch (x.getType())
        {
            case CIMTYPE_BOOLEAN:
                Packer::packBoolean(out, rep->u._booleanValue);
                break;

            case CIMTYPE_SINT8:
            case CIMTYPE_UINT8:
                Packer::packUint8(out, rep->u._uint8Value);
                break;

            case CIMTYPE_SINT16:
            case CIMTYPE_UINT16:
            case CIMTYPE_CHAR16:
                Packer::packUint16(out, rep->u._uint16Value);
                break;

            case CIMTYPE_SINT32:
            case CIMTYPE_UINT32:
            case CIMTYPE_REAL32:
                Packer::packUint32(out, rep->u._uint32Value);
                break;

            case CIMTYPE_SINT64:
            case CIMTYPE_UINT64:
            case CIMTYPE_REAL64:
                Packer::packUint64(out, rep->u._uint64Value);
                break;

            case CIMTYPE_STRING:
                Packer::packString(out, CIMValueType<String>::ref(rep));
                break;

            case CIMTYPE_DATETIME:
                Packer::packString(
                    out, CIMValueType<CIMDateTime>::ref(rep).toString());
                break;

            case CIMTYPE_REFERENCE:
                Packer::packString(
                    out, CIMValueType<CIMObjectPath>::ref(rep).toString());
                break;

            case CIMTYPE_OBJECT:
                Packer::packString(
                    out, CIMValueType<CIMObject>::ref(rep).toString());
                break;
            case CIMTYPE_INSTANCE:
            {
                CIMObject tmp(CIMValueType<CIMInstance>::ref(rep));
                Packer::packString(
                    out, tmp.toString());
                break;
            }
        }
    }
}

void BinaryStreamer::_unpackValue(
    const Buffer& in, Uint32& pos, CIMValue& x)
{
    _checkMagicByte(in, pos);

    CIMType type;
    _unpackType(in, pos, type);

    Boolean isArray;
    Packer::unpackBoolean(in, pos, isArray);

    Uint32 arraySize = 0;

    if (isArray)
        Packer::unpackSize(in, pos, arraySize);

    Boolean isNull;
    Packer::unpackBoolean(in, pos, isNull);

    if (isNull)
    {
        x = CIMValue(type, isArray, arraySize);
        return;
    }

    if (isArray)
    {
        CIMValue cimValue(type, isArray, arraySize);

        switch (type)
        {
            case CIMTYPE_BOOLEAN:
                UnpackArray<Boolean>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_UINT8:
                UnpackArray<Uint8>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_SINT8:
                UnpackArray<Sint8>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_UINT16:
                UnpackArray<Uint16>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_SINT16:
                UnpackArray<Sint16>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_UINT32:
                UnpackArray<Uint32>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_SINT32:
                UnpackArray<Sint32>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_UINT64:
                UnpackArray<Uint64>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_SINT64:
                UnpackArray<Sint64>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_REAL32:
                UnpackArray<Real32>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_REAL64:
                UnpackArray<Real64>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_CHAR16:
                UnpackArray<Char16>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_STRING:
                UnpackArray<String>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_DATETIME:
                UnpackArray<CIMDateTime>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_REFERENCE:
                UnpackArray<CIMObjectPath>::func(in, pos, arraySize, cimValue);
                break;

            case CIMTYPE_OBJECT:
                UnpackArray<CIMObject>::func(in, pos, arraySize, cimValue);
                break;
            case CIMTYPE_INSTANCE:
                UnpackArray<CIMInstance>::func(in, pos, arraySize, cimValue);
                break;
        }

        x = cimValue;
    }
    else
    {
        CIMValue cimValue(type, isArray);

        switch (type)
        {
            case CIMTYPE_BOOLEAN:
                UnpackScalar<Boolean>::func(in, pos, cimValue);
                break;

            case CIMTYPE_UINT8:
                UnpackScalar<Uint8>::func(in, pos, cimValue);
                break;

            case CIMTYPE_SINT8:
                UnpackScalar<Sint8>::func(in, pos, cimValue);
                break;

            case CIMTYPE_UINT16:
                UnpackScalar<Uint16>::func(in, pos, cimValue);
                break;

            case CIMTYPE_SINT16:
                UnpackScalar<Sint16>::func(in, pos, cimValue);
                break;

            case CIMTYPE_UINT32:
                UnpackScalar<Uint32>::func(in, pos, cimValue);
                break;

            case CIMTYPE_SINT32:
                UnpackScalar<Sint32>::func(in, pos, cimValue);
                break;

            case CIMTYPE_UINT64:
                UnpackScalar<Uint64>::func(in, pos, cimValue);
                break;

            case CIMTYPE_SINT64:
                UnpackScalar<Sint64>::func(in, pos, cimValue);
                break;

            case CIMTYPE_REAL32:
                UnpackScalar<Real32>::func(in, pos, cimValue);
                break;

            case CIMTYPE_REAL64:
                UnpackScalar<Real64>::func(in, pos, cimValue);
                break;

            case CIMTYPE_CHAR16:
                UnpackScalar<Char16>::func(in, pos, cimValue);
                break;

            case CIMTYPE_STRING:
                UnpackScalar<String>::func(in, pos, cimValue);
                break;

            case CIMTYPE_DATETIME:
                UnpackScalar<CIMDateTime>::func(in, pos, cimValue);
                break;

            case CIMTYPE_REFERENCE:
                UnpackScalar<CIMObjectPath>::func(in, pos, cimValue);
                break;

            case CIMTYPE_OBJECT:
                UnpackScalar<CIMObject>::func(in, pos, cimValue);
                break;
            case CIMTYPE_INSTANCE:
                UnpackScalar<CIMInstance>::func(in, pos, cimValue);
                break;
        }

        x = cimValue;
    }

    return;
}

void BinaryStreamer::_packProperty(Buffer& out, const CIMProperty& x)
{
    CIMPropertyRep* rep = x._rep;

    _packMagicByte(out);
    _packName(out, rep->getName());
    _packValue(out, rep->getValue());
    Packer::packSize(out, rep->getArraySize());
    _packName(out, rep->getReferenceClassName());
    _packName(out, rep->getClassOrigin());
    Packer::packBoolean(out, rep->getPropagated());
    PackQualifiers<CIMPropertyRep>::func(out, rep);
}

void BinaryStreamer::_unpackProperty(
    const Buffer& in, Uint32& pos, CIMProperty& x)
{
    _checkMagicByte(in, pos);

    CIMName name;
    _unpackName(in, pos, name);

    CIMValue value;
    _unpackValue(in, pos, value);

    Uint32 arraySize;
    Packer::unpackSize(in, pos, arraySize);

    CIMName referenceClassName;
    _unpackName(in, pos, referenceClassName);

    CIMName classOrigin;
    _unpackName(in, pos, classOrigin);

    Boolean propagated;
    Packer::unpackBoolean(in, pos, propagated);

    CIMProperty cimProperty(
        name, value, arraySize, referenceClassName, classOrigin, propagated);

    UnpackQualifiers<CIMProperty>::func(in, pos, cimProperty);
    if (cimProperty.getType() == CIMTYPE_STRING)
    {
        CIMType realType = CIMTYPE_STRING;
        if (cimProperty.findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE)
            != PEG_NOT_FOUND)
        {
            // Note that this condition should only happen for properties in
            // class definitions, and only NULL values are recognized. We
            // currently don't handle embedded instance types with default
            // values in the class definition.
            PEGASUS_ASSERT(value.isNull());
            realType = CIMTYPE_INSTANCE;
        }
        else if (cimProperty.findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT)
                 != PEG_NOT_FOUND)
        {
            // The binary protocol (unlike the XML protocol) successfully
            // transmits embedded object default values. But since they are
            // not handled elsewhere, we discard the value.
            cimProperty.setValue(
                CIMValue(value.getType(),value.isArray(),value.getArraySize()));

            realType = CIMTYPE_OBJECT;
        }

        if (realType != CIMTYPE_STRING)
        {
            CIMProperty tmpProperty(name, CIMValue(realType, value.isArray()),
                arraySize, referenceClassName, classOrigin, propagated);
            for (unsigned int i = 0, n = cimProperty.getQualifierCount();
                 i < n; ++i)
            {
                tmpProperty.addQualifier(cimProperty.getQualifier(i));
            }
            cimProperty = tmpProperty;
        }
    }
    x = cimProperty;
}

void BinaryStreamer::_packParameter(Buffer& out, const CIMParameter& x)
{
    CIMParameterRep* rep = x._rep;

    _packMagicByte(out);
    _packName(out, rep->getName());
    _packType(out, rep->getType());
    Packer::packBoolean(out, rep->isArray());
    Packer::packSize(out, rep->getArraySize());
    _packName(out, rep->getReferenceClassName());
    PackQualifiers<CIMParameterRep>::func(out, rep);
}

void BinaryStreamer::_unpackParameter(
    const Buffer& in, Uint32& pos, CIMParameter& x)
{
    _checkMagicByte(in, pos);

    CIMName name;
    _unpackName(in, pos, name);

    CIMType type;
    _unpackType(in, pos, type);

    Boolean isArray;
    Packer::unpackBoolean(in, pos, isArray);

    Uint32 arraySize;
    Packer::unpackSize(in, pos, arraySize);

    CIMName referenceClassName;
    _unpackName(in, pos, referenceClassName);

    CIMParameter cimParameter(
        name, type, isArray, arraySize, referenceClassName);

    UnpackQualifiers<CIMParameter>::func(in, pos, cimParameter);

    x = cimParameter;
}

void BinaryStreamer::_packParameters(Buffer& out, CIMMethodRep* rep)
{
    Uint32 n = rep->getParameterCount();
    Packer::packSize(out, n);

    for (Uint32 i = 0; i < n; i++)
        BinaryStreamer::_packParameter(out, rep->getParameter(i));
}

void BinaryStreamer::_unpackParameters(
    const Buffer& in, Uint32& pos, CIMMethod& x)
{
    Uint32 n;
    Packer::unpackSize(in, pos, n);

    for (size_t i = 0; i < n; i++)
    {
        CIMParameter q;
        _unpackParameter(in, pos, q);
        x.addParameter(q);
    }
}

void BinaryStreamer::_packMethod(Buffer& out, const CIMMethod& x)
{
    CIMMethodRep* rep = x._rep;

    _packMagicByte(out);
    _packName(out, rep->getName());
    _packType(out, rep->getType());
    _packName(out, rep->getClassOrigin());
    Packer::packBoolean(out, rep->getPropagated());
    PackQualifiers<CIMMethodRep>::func(out, rep);
    _packParameters(out, rep);
}

void BinaryStreamer::_unpackMethod(
    const Buffer& in, Uint32& pos, CIMMethod& x)
{
    _checkMagicByte(in, pos);

    CIMName name;
    _unpackName(in, pos, name);

    CIMType type;
    _unpackType(in, pos, type);

    CIMName classOrigin;
    _unpackName(in, pos, classOrigin);

    Boolean propagated;
    Packer::unpackBoolean(in, pos, propagated);

    CIMMethod cimMethod(name, type, classOrigin, propagated);
    UnpackQualifiers<CIMMethod>::func(in, pos, cimMethod);
    _unpackParameters(in, pos, cimMethod);

    x = cimMethod;
}

void BinaryStreamer::_packObjectPath(Buffer& out, const CIMObjectPath& x)
{
    Packer::packString(out, x.toString());
}

void BinaryStreamer::_unpackObjectPath(
    const Buffer& in, Uint32& pos, CIMObjectPath& x)
{
    String tmp;
    Packer::unpackString(in, pos, tmp);
    x = CIMObjectPath(tmp);
}

void BinaryStreamer::_packProperties(Buffer& out, CIMObjectRep* rep)
{
    Uint32 n = rep->getPropertyCount();
    Packer::packSize(out, n);

    for (Uint32 i = 0; i < n; i++)
        BinaryStreamer::_packProperty(out, rep->getProperty(i));
}

void BinaryStreamer::_packMethods(Buffer& out, CIMClassRep* rep)
{
    Uint32 n = rep->getMethodCount();
    Packer::packSize(out, n);

    for (Uint32 i = 0; i < n; i++)
        BinaryStreamer::_packMethod(out, rep->getMethod(i));
}

void BinaryStreamer::_packScope(Buffer& out, const CIMScope& x)
{
    Packer::packUint32(out, x.cimScope);
}

void BinaryStreamer::_unpackScope(
    const Buffer& in, Uint32& pos, CIMScope& x)
{
    Packer::unpackUint32(in, pos, x.cimScope);
}

void BinaryStreamer::_packFlavor(Buffer& out, const CIMFlavor& x)
{
    Packer::packUint32(out, x.cimFlavor);
}

void BinaryStreamer::_unpackFlavor(
    const Buffer& in, Uint32& pos, CIMFlavor& x)
{
    Packer::unpackUint32(in, pos, x.cimFlavor);
}

void BinaryStreamer::_packType(Buffer& out, const CIMType& x)
{
    Packer::packUint8(out, Uint8(x));
}

void BinaryStreamer::_unpackType(
    const Buffer& in, Uint32& pos, CIMType& x)
{
    Uint8 tmp;
    Packer::unpackUint8(in, pos, tmp);
    x = CIMType(tmp);
}

void BinaryStreamer::encode(
    Buffer& out,
    const CIMClass& x)
{
    CIMClassRep* rep = x._rep;
    _packMagicByte(out);
    _packHeader(out, BINARY_CLASS);
    _packName(out, x.getClassName());
    _packName(out, x.getSuperClassName());
    PackQualifiers<CIMClassRep>::func(out, rep);
    _packProperties(out, rep);
    _packMethods(out, rep);
}

void BinaryStreamer::decode(
    const Buffer& in,
    unsigned int pos,
    CIMClass& x)
{
    _checkMagicByte(in, pos);
    _checkHeader(in, pos, BINARY_CLASS);

    CIMName className;
    _unpackName(in, pos, className);

    CIMName superClassName;
    _unpackName(in, pos, superClassName);

    CIMClass cimClass(className, superClassName);

    UnpackQualifiers<CIMClass>::func(in, pos, cimClass);
    UnpackProperties<CIMClass>::func(in, pos, cimClass);
    UnpackMethods<CIMClass>::func(in, pos, cimClass);

    x = cimClass;
}

void BinaryStreamer::encode(
    Buffer& out,
    const CIMInstance& x)
{
    CIMInstanceRep* rep = x._rep;
    _packMagicByte(out);
    _packHeader(out, BINARY_INSTANCE);
    _packObjectPath(out, x.getPath());
    PackQualifiers<CIMInstanceRep>::func(out, rep);
    _packProperties(out, rep);
}

void BinaryStreamer::decode(
    const Buffer& in,
    unsigned int pos,
    CIMInstance& x)
{
    _checkMagicByte(in, pos);
    _checkHeader(in, pos, BINARY_INSTANCE);

    CIMObjectPath objectPath;
    _unpackObjectPath(in, pos, objectPath);
    CIMInstance cimInstance(objectPath.getClassName());
    cimInstance.setPath(objectPath);

    UnpackQualifiers<CIMInstance>::func(in, pos, cimInstance);
    UnpackProperties<CIMInstance>::func(in, pos, cimInstance);

    x = cimInstance;
}

void BinaryStreamer::encode(
    Buffer& out,
    const CIMQualifierDecl& x)
{
    _packMagicByte(out);
    _packHeader(out, BINARY_QUALIFIER_DECL);
    _packName(out , x.getName());
    _packValue(out , x.getValue());
    _packScope(out , x.getScope());
    _packFlavor(out , x.getFlavor());
    Packer::packSize(out, x.getArraySize());
}

void BinaryStreamer::decode(
    const Buffer& in,
    unsigned int pos,
    CIMQualifierDecl& x)
{
    _checkMagicByte(in, pos);
    _checkHeader(in, pos, BINARY_QUALIFIER_DECL);

    CIMName qualifierName;
    _unpackName(in, pos, qualifierName);

    CIMValue value;
    _unpackValue(in, pos, value);

    CIMScope scope;
    _unpackScope(in, pos, scope);

    CIMFlavor flavor;
    BinaryStreamer::_unpackFlavor(in, pos, flavor);

    Uint32 arraySize;
    Packer::unpackSize(in, pos, arraySize);

    x = CIMQualifierDecl(qualifierName, value, scope, flavor, arraySize);
}

PEGASUS_NAMESPACE_END
