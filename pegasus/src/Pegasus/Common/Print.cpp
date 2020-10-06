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

#include "Print.h"
#include "Pegasus_inl.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

// Build the functions in this file ONLY in debug mode.
#if defined(PEGASUS_DEBUG)

struct Ind
{
    Ind(Uint32 n_) : n(n_) { }
    Uint32 n;
};

inline PEGASUS_STD(ostream)& operator<<(PEGASUS_STD(ostream)& os, const Ind& x)
{
    for (Uint32 i = 0; i < x.n; i++)
        os << "    ";

    return os;
}

static const char* _typeStrings[] =
{
    "boolean",
    "uint8",
    "sint8",
    "uint16",
    "sint16",
    "uint32",
    "sint32",
    "uint64",
    "sint64",
    "real32",
    "real64",
    "char16",
    "string",
    "datetime",
    "reference",
    "object",
    "instance"
};

template<class T>
struct Print
{
    static void func(ostream& os, const T& x)
    {
        os << x;
    }
};

PEGASUS_TEMPLATE_SPECIALIZATION
struct Print<Boolean>
{
    static void func(ostream& os, const Boolean& x)
    {
        os << (x ? "true" : "false");
    }
};

PEGASUS_TEMPLATE_SPECIALIZATION
struct Print<Uint64>
{
    static void func(ostream& os, const Uint64& x)
    {
        char buf[32];
        sprintf(buf, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", x);
        os << buf;
    }
};

PEGASUS_TEMPLATE_SPECIALIZATION
struct Print<Sint64>
{
    static void func(ostream& os, const Sint64& x)
    {
        char buf[32];
        sprintf(buf, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d", x);
        os << buf;
    }
};

PEGASUS_TEMPLATE_SPECIALIZATION
struct Print<Uint16>
{
    static void func(ostream& os, const Char16& x)
    {
        os << Uint16(x);
    }
};

PEGASUS_TEMPLATE_SPECIALIZATION
struct Print<CIMDateTime>
{
    static void func(ostream& os, const CIMDateTime& x)
    {
        os << x.toString();
    }
};

template<class T>
struct PrintArray
{
    static void print(ostream& os, const CIMValue& cv)
    {
        Array<T> a;
        cv.get(a);

        os << "{ ";

        for (Uint32 i = 0; i < a.size(); i++)
        {
            const T& r = a[i];
            Print<T>::func(os, r);

            if (i + 1 != a.size())
                os << ", ";
            else
                os << " ";
        }

        os << "}" << endl;
    }
};

template<class T>
struct PrintScalar
{
    static void print(ostream& os, const CIMValue& cv)
    {
        T x;
        cv.get(x);
        Print<T>::func(os, x);
        os << endl;
    }
};

void _printValue(ostream& os, const CIMValue& cv, Uint32 n)
{
    os << Ind(n) << "value=";

    if (cv.isNull())
    {
        os << "null" << endl;
    }
    else if (cv.isArray())
    {
        switch (cv.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                PrintArray<Boolean>::print(os, cv);
                break;
            }
            case CIMTYPE_UINT8:
            {
                PrintArray<Uint8>::print(os, cv);
                break;
            }
            case CIMTYPE_SINT8:
            {
                PrintArray<Sint8>::print(os, cv);
                break;
            }
            case CIMTYPE_UINT16:
            {
                PrintArray<Uint16>::print(os, cv);
                break;
            }
            case CIMTYPE_SINT16:
            {
                PrintArray<Sint16>::print(os, cv);
                break;
            }
            case CIMTYPE_UINT32:
            {
                PrintArray<Uint32>::print(os, cv);
                break;
            }
            case CIMTYPE_SINT32:
            {
                PrintArray<Sint32>::print(os, cv);
                break;
            }
            case CIMTYPE_UINT64:
            {
                PrintArray<Uint64>::print(os, cv);
                break;
            }
            case CIMTYPE_SINT64:
            {
                PrintArray<Sint64>::print(os, cv);
                break;
            }
            case CIMTYPE_REAL32:
            {
                PrintArray<Real32>::print(os, cv);
                break;
            }
            case CIMTYPE_REAL64:
            {
                PrintArray<Real64>::print(os, cv);
                break;
            }
            case CIMTYPE_CHAR16:
            {
                PrintArray<Char16>::print(os, cv);
                break;
            }
            case CIMTYPE_STRING:
            {
                PrintArray<String>::print(os, cv);
                break;
            }
            case CIMTYPE_DATETIME:
            {
                PrintArray<CIMDateTime>::print(os, cv);
                break;
            }
            default:
                break;
        }
    }
    else
    {
        switch (cv.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                PrintScalar<Boolean>::print(os, cv);
                break;
            }
            case CIMTYPE_UINT8:
            {
                PrintScalar<Uint8>::print(os, cv);
                break;
            }
            case CIMTYPE_SINT8:
            {
                PrintScalar<Sint8>::print(os, cv);
                break;
            }
            case CIMTYPE_UINT16:
            {
                PrintScalar<Uint16>::print(os, cv);
                break;
            }
            case CIMTYPE_SINT16:
            {
                PrintScalar<Sint16>::print(os, cv);
                break;
            }
            case CIMTYPE_UINT32:
            {
                PrintScalar<Uint32>::print(os, cv);
                break;
            }
            case CIMTYPE_SINT32:
            {
                PrintScalar<Sint32>::print(os, cv);
                break;
            }
            case CIMTYPE_UINT64:
            {
                PrintScalar<Uint64>::print(os, cv);
                break;
            }
            case CIMTYPE_SINT64:
            {
                PrintScalar<Sint64>::print(os, cv);
                break;
            }
            case CIMTYPE_REAL32:
            {
                PrintScalar<Real32>::print(os, cv);
                break;
            }
            case CIMTYPE_REAL64:
            {
                PrintScalar<Real64>::print(os, cv);
                break;
            }
            case CIMTYPE_CHAR16:
            {
                PrintScalar<Char16>::print(os, cv);
                break;
            }
            case CIMTYPE_STRING:
            {
                PrintScalar<String>::print(os, cv);
                break;
            }
            case CIMTYPE_DATETIME:
            {
                PrintScalar<CIMDateTime>::print(os, cv);
                break;
            }
            default:
                break;
        }
    }
}

void PrintValue(PEGASUS_STD(ostream)& os, const CIMValue& x, Uint32 n)
{
    os << Ind(n) << "CIMValue" << endl;
    os << Ind(n) << "{" << endl;
    os << Ind(n) << "    type=" << _typeStrings[x.getType()];

    if (x.isArray())
        os << "[]";

    os << endl;

    if (x.getType() == CIMTYPE_INSTANCE)
    {
        if (x.isArray())
        {
            Array<CIMInstance> a;
            x.get(a);

            for (Uint32 i = 0; i < a.size(); i++)
            {
                CIMInstance ci = a[i];

                if (ci.isUninitialized())
                {
                    os << Ind(n) << "null" << endl;
                }
                else
                {
                    PrintInstance(os, ci, n + 1);
                }
            }
        }
        else
        {
            CIMInstance ci;
            x.get(ci);

            if (ci.isUninitialized())
            {
                os << Ind(n) << "null" << endl;
            }
            else
            {
                PrintInstance(os, ci, n + 1);
            }
        }
    }
    else if (x.getType() == CIMTYPE_OBJECT)
    {
        if (x.isArray())
        {
            Array<CIMObject> a;
            x.get(a);

            for (Uint32 i = 0; i < a.size(); i++)
            {
                if (a[i].isInstance())
                {
                    CIMInstance ci(a[i]);

                    if (ci.isUninitialized())
                    {
                        os << Ind(n) << "null" << endl;
                    }
                    else
                    {
                        PrintInstance(os, ci, n + 1);
                    }
                }
            }
        }
        else
        {
            CIMObject co;
            x.get(co);

            if (co.isInstance())
            {
                CIMInstance ci(co);

                if (ci.isUninitialized())
                {
                    os << Ind(n) << "null" << endl;
                }
                else
                {
                    PrintInstance(os, ci, n + 1);
                }
            }
        }
    }
    else
    {
        _printValue(os, x, n + 1);
    }

    os << Ind(n) << "}" << endl;
}

static const char* _keyTypes[] =
{
    "boolean",
    "string",
    "numeric",
    "reference",
};

void _printKeyBinding(
    PEGASUS_STD(ostream)& os,
    const CIMKeyBinding& x,
    Uint32 n)
{
    os << Ind(n) << "CIMKeyBinding" << endl;
    os << Ind(n) << "{" << endl;

    os << Ind(n) << "    name=" << x.getName().getString() << endl;
    os << Ind(n) << "    type=" << _keyTypes[x.getType()] << endl;
    os << Ind(n) << "    value=" << x.getValue() << endl;

    os << Ind(n) << "}" << endl;
}

void PrintObjectPath(
    PEGASUS_STD(ostream)& os,
    const CIMObjectPath& x,
    Uint32 n)
{
    os << Ind(n) << "CIMObjectPath" << endl;
    os << Ind(n) << "{" << endl;
    os << Ind(n) << "    host=" << x.getHost() << endl;
    os << Ind(n) << "    namespace=" << x.getNameSpace().getString() << endl;
    os << Ind(n) << "    classname=" << x.getClassName().getString() << endl;
    os << Ind(n) << "    keyBindings" << endl;
    os << Ind(n) << "    {" << endl;

    const Array<CIMKeyBinding>& a = x.getKeyBindings();

    for (Uint32 i = 0; i < a.size(); i++)
    {
        _printKeyBinding(os, a[i], n + 2);
    }

    os << Ind(n) << "    }" << endl;

    os << Ind(n) << "}" << endl;
}

void PrintProperty(
    PEGASUS_STD(ostream)& os,
    const CIMConstProperty& x,
    Uint32 n)
{
    os << Ind(n) << "CIMProperty" << endl;
    os << Ind(n) << "{" << endl;

    if (x.getQualifierCount() != 0)
    {
        for (Uint32 i = 0; i < x.getQualifierCount(); i++)
        {
            PrintQualifier(os, x.getQualifier(i), n + 1);
        }
    }
    os << Ind(n) << "    name=" << x.getName().getString() << endl;

    os << Ind(n) << "    CIMType=" << cimTypeToString(x.getType());

    if (x.isArray())
    {
        os << " Array";
        if (x.getArraySize() != 0)
        {
            os << ", size=" << x.getArraySize();
        }
        os << endl;
    }
    else
    {
        os << " Scalar" << endl;
    }

    PrintValue(os, x.getValue(), n + 1);
    os << Ind(n) << "}" << endl;
}

void PrintInstance(
    PEGASUS_STD(ostream)& os,
    const CIMConstInstance& x,
    Uint32 n)
{
    os << Ind(n) << "CIMInstance" << endl;
    os << Ind(n) << "{" << endl;

    if (x.getQualifierCount() != 0)
    {
        for (Uint32 i = 0; i < x.getQualifierCount(); i++)
        {
            PrintQualifier(os, x.getQualifier(i), n + 1);
        }
    }

    os << Ind(n) << "    class=" << x.getClassName().getString() << endl;

    PrintObjectPath(os, x.getPath(), n + 1);


    for (Uint32 i = 0; i < x.getPropertyCount(); i++)
    {
        PrintProperty(os, x.getProperty(i), n + 1);
    }

    os << Ind(n) << "}" << endl;
}

void PEGASUS_COMMON_LINKAGE PrintQualifierDecl(
    PEGASUS_STD(ostream)& os,
    const CIMConstQualifierDecl& x,
    Uint32 n)
{
    os << Ind(n) << "CIMQualifierDecl" << endl;
    os << Ind(n) << "{" << endl;
    os << Ind(n) << "    name=" << x.getName().getString() << endl;
    os << Ind(n) << "    type=" << _typeStrings[x.getType()] << endl;
    os << Ind(n) << "    scope=" << x.getScope().toString() << endl;
    os << Ind(n) << "    flavor=" << x.getFlavor().toString() << endl;
    os << Ind(n) << "    arraySize=" << x.getArraySize() << endl;
    PrintValue(os, x.getValue(), n + 1);
    os << Ind(n) << "}" << endl;
}

void PEGASUS_COMMON_LINKAGE PrintQualifier(
    PEGASUS_STD(ostream)& os,
    const CIMConstQualifier& x,
    Uint32 n)
{
    os << Ind(n) << "CIMQualifier" << endl;
    os << Ind(n) << "{" << endl;
    os << Ind(n) << "    name=" << x.getName().getString() << endl;
    os << Ind(n) << "    type=" << _typeStrings[x.getType()] << endl;
    os << Ind(n) << "    flavor=" << x.getFlavor().toString() << endl;
    os << Ind(n) << "    isArray=" << boolToString(x.isArray()) << endl;
    PrintValue(os, x.getValue(), n + 1);
    os << Ind(n) << "}" << endl;
}

PEGASUS_COMMON_LINKAGE void PrintParamValue(
    PEGASUS_STD(ostream)& os,
    const CIMParamValue& x,
    Uint32 n)
{
    os << Ind(n) << "CIMParamValue" << endl;
    os << Ind(n) << "{" << endl;
    os << Ind(n) << "    name=" << x.getParameterName() << endl;
    PrintValue(os, x.getValue(), n + 1);
    os << Ind(n) << "}" << endl;
}

PEGASUS_COMMON_LINKAGE void PrintParamValueArray(
    PEGASUS_STD(ostream)& os,
    const Array<CIMParamValue>& x,
    Uint32 n)
{
    os << Ind(n) << "Array<CIMParamValue>" << endl;
    os << Ind(n) << "{" << endl;

    for (Uint32 i = 0; i < x.size(); i++)
    {
        PrintParamValue(os, x[i], n + 1);
    }

    os << Ind(n) << "}" << endl;
}

#else  // compile empty functions


PEGASUS_COMMON_LINKAGE void PrintValue(
    PEGASUS_STD(ostream)&,
    const CIMValue&,
    Uint32)
{
}

PEGASUS_COMMON_LINKAGE void PrintProperty(
    PEGASUS_STD(ostream)&,
    const CIMConstProperty&,
    Uint32)
{
}

PEGASUS_COMMON_LINKAGE void PrintObjectPath(
    PEGASUS_STD(ostream)&,
    const CIMObjectPath&,
    Uint32)
{
}

PEGASUS_COMMON_LINKAGE void PrintInstance(
    PEGASUS_STD(ostream)&,
    const CIMConstInstance&,
    Uint32)
{
}

PEGASUS_COMMON_LINKAGE void PrintQualifierDecl(
    PEGASUS_STD(ostream)&,
    const CIMConstQualifierDecl&,
    Uint32)
{
}

PEGASUS_COMMON_LINKAGE void PrintParamValue(
    PEGASUS_STD(ostream)&,
    const CIMParamValue&,
    Uint32)
{
}

PEGASUS_COMMON_LINKAGE void PrintParamValueArray(
    PEGASUS_STD(ostream)&,
    const Array<CIMParamValue>&,
    Uint32)
{
}

PEGASUS_COMMON_LINKAGE void PrintQualifierDecl(
    PEGASUS_STD(ostream)&,
    const CIMConstQualifier&,
    Uint32)
{
}
#endif /* defined(PEGASUS_DEBUG) */

PEGASUS_NAMESPACE_END
