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

#ifndef Pegasus_ValueRep_h
#define Pegasus_ValueRep_h

#include <new>
#include "Union.h"
#include "AtomicInt.h"

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE CIMValueRep
{
public:

    CIMValueRep();

    CIMValueRep(int*);

    ~CIMValueRep();

    void release();

    static void ref(const CIMValueRep* rep);

    static void unref(const CIMValueRep* rep);

    AtomicInt refs;
    CIMType type;
    Boolean isArray;
    Boolean isNull;
    Union u;

    static CIMValueRep _emptyRep;
};

inline CIMValueRep::CIMValueRep() : refs(1)
{
}

inline CIMValueRep::CIMValueRep(int*) :
    refs(2), type(CIMTYPE_BOOLEAN), isArray(false), isNull(true)
{
    // This constructor is only used by the _emptyRep object.
    memset(&u, 0, sizeof(Union));
}

inline CIMValueRep::~CIMValueRep()
{
    release();
}

inline void CIMValueRep::ref(const CIMValueRep* rep)
{
    if (rep != &CIMValueRep::_emptyRep)
    {
        ((CIMValueRep*)rep)->refs++;
    }
}

inline void CIMValueRep::unref(const CIMValueRep* rep)
{
    if (rep != &CIMValueRep::_emptyRep &&
        ((CIMValueRep*)rep)->refs.decAndTestIfZero())
    {
        delete (CIMValueRep*)rep;
    }
}

//==============================================================================
//
// CIMValueType - template utilities used below.
//
//==============================================================================

// Functions to perform type deduction.
inline CIMType TypeOf(Boolean*) { return CIMTYPE_BOOLEAN; }
inline CIMType TypeOf(Uint8*) { return CIMTYPE_UINT8; }
inline CIMType TypeOf(Sint8*) { return CIMTYPE_SINT8; }
inline CIMType TypeOf(Uint16*) { return CIMTYPE_UINT16; }
inline CIMType TypeOf(Sint16*) { return CIMTYPE_SINT16; }
inline CIMType TypeOf(Uint32*) { return CIMTYPE_UINT32; }
inline CIMType TypeOf(Sint32*) { return CIMTYPE_SINT32; }
inline CIMType TypeOf(Uint64*) { return CIMTYPE_UINT64; }
inline CIMType TypeOf(Sint64*) { return CIMTYPE_SINT64; }
inline CIMType TypeOf(Real32*) { return CIMTYPE_REAL32; }
inline CIMType TypeOf(Real64*) { return CIMTYPE_REAL64; }
inline CIMType TypeOf(Char16*) { return CIMTYPE_CHAR16; }
inline CIMType TypeOf(String*) { return CIMTYPE_STRING; }
inline CIMType TypeOf(CIMDateTime*) { return CIMTYPE_DATETIME; }
inline CIMType TypeOf(CIMObjectPath*) { return CIMTYPE_REFERENCE; }
inline CIMType TypeOf(CIMObject*) { return CIMTYPE_OBJECT; }
inline CIMType TypeOf(CIMInstance*) { return CIMTYPE_INSTANCE; }

inline bool IsRaw(Boolean*) { return true; }
inline bool IsRaw(Uint8*) { return true; }
inline bool IsRaw(Sint8*) { return true; }
inline bool IsRaw(Uint16*) { return true; }
inline bool IsRaw(Sint16*) { return true; }
inline bool IsRaw(Uint32*) { return true; }
inline bool IsRaw(Sint32*) { return true; }
inline bool IsRaw(Uint64*) { return true; }
inline bool IsRaw(Sint64*) { return true; }
inline bool IsRaw(Real32*) { return true; }
inline bool IsRaw(Real64*) { return true; }
inline bool IsRaw(Char16*) { return true; }
inline bool IsRaw(String*) { return false; }
inline bool IsRaw(CIMDateTime*) { return false; }
inline bool IsRaw(CIMObjectPath*) { return false; }
inline bool IsRaw(CIMObject*) { return false; }
inline bool IsRaw(CIMInstance*) { return false; }

template<class T>
struct CIMValueType
{
public:

    static T* ptr(const CIMValueRep* rep)
    {
        return (T*)(&rep->u);
    }

    static T& ref(const CIMValueRep* rep)
    {
        return *((T*)((void*)&rep->u));
    }

    static Array<T>* aptr(const CIMValueRep* rep)
    {
        return (Array<T>*)(&rep->u);
    }

    static Array<T>& aref(const CIMValueRep* rep)
    {
        return *((Array<T>*)((void*)&rep->u));
    }

    static void defaultConstruct(CIMValueRep* rep)
    {
        if (IsRaw((T*)0))
            rep->u._uint64Value = (Uint64)0;
        else
            new((T*)((void*)&rep->u)) T();
    }

    static void copyConstruct(CIMValueRep* rep, const T& x)
    {
        new((T*)((void*)&rep->u)) T(x);
    }

    static void copyConstructArray(CIMValueRep* rep, const Array<T>& x)
    {
        new((Array<T>*)((void*)&rep->u)) Array<T>(x);
    }

    static void constructArrayWithSize(CIMValueRep* rep, Uint32 arraySize)
    {
        new((Array<T>*)((void*)&rep->u)) Array<T>(arraySize);
    }

    static void destruct(CIMValueRep* rep)
    {
        ((T*)((void*)&rep->u))->~T();
    }

    static void destructArray(CIMValueRep* rep)
    {
        ((Array<T>*)((void*)&rep->u))->~Array<T>();
    }

    static void setNull(
        CIMValueRep* rep, CIMType type_, bool isArray_, Uint32 arraySize_)
    {
        rep->type = type_;
        rep->isArray = isArray_;
        rep->isNull = true;

        if (isArray_)
            constructArrayWithSize(rep, arraySize_);
        else
            defaultConstruct(rep);
    }

    static void set(CIMValueRep* rep, const T& x)
    {
        rep->type = TypeOf((T*)0);
        rep->isArray = false;
        rep->isNull = false;
        copyConstruct(rep, x);
    }

    static void setArray(CIMValueRep* rep, const Array<T>& x)
    {
        rep->type = TypeOf((T*)0);
        rep->isArray = true;
        rep->isNull = false;
        copyConstructArray(rep, x);
    }

    static Uint32 arraySize(CIMValueRep* rep)
    {
        return aref(rep).size();
    }

    static bool equal(const CIMValueRep* r1, const CIMValueRep* r2)
    {
        return ref(r1) == ref(r2);
    }

    static bool equalArray(const CIMValueRep* r1, const CIMValueRep* r2)
    {
        return aref(r1) == aref(r2);
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ValueRep_h */
