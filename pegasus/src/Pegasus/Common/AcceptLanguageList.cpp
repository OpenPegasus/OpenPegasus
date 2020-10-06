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

#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/Pair.h>

PEGASUS_NAMESPACE_BEGIN

typedef Pair<LanguageTag, Real32> AcceptLanguagePair;
typedef Array<AcceptLanguagePair> AcceptLanguageArray;

#define PEGASUS_ARRAY_T AcceptLanguagePair
# include <Pegasus/Common/ArrayInter.h>
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

//
// Implementation Notes:
// =====================
//
// The internal representation member (_rep) is a pointer to an
// AcceptLanguageListRep object. We could define a class with this name
// as follows:
//
//     class AcceptLanguageListRep
//     {
//         AcceptLanguageArray array;
//     };
//
// But this requires separate heap object to hold the array. Instead we use
// the following fact to eliminate the extra heap object:
//
//     sizeof(AcceptLanguageArray) == sizeof(AcceptLanguageListRep*)
//
// We know this since all arrays contain a single pointer to a representation
// object. Take for example the following structure:
//
//     class MyClass
//     {
//         void* rep;
//     };
//
// Clearly, sizeof(MyClass) == sizeof(void*). We eliminate the extra heap object
// by overlaying the AcceptLanguageList::_rep pointer with the array base. So
// AcceptLanguageList::_rep in fact refers to the Array<T>::_rep.
//

static inline AcceptLanguageArray& GetAcceptLanguageArray(
    AcceptLanguageList* list)
{
    return *reinterpret_cast<AcceptLanguageArray*>(list);
}

static inline const AcceptLanguageArray& GetAcceptLanguageArray(
    const AcceptLanguageList* list)
{
    return *reinterpret_cast<const AcceptLanguageArray*>(list);
}

AcceptLanguageList::AcceptLanguageList()
{
    AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    new (&self) AcceptLanguageArray;
}

AcceptLanguageList::AcceptLanguageList(const AcceptLanguageList& x)
{
    AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    const AcceptLanguageArray& other = GetAcceptLanguageArray(&x);
    new (&self) AcceptLanguageArray(other);
}

AcceptLanguageList::~AcceptLanguageList()
{
    AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    self.~AcceptLanguageArray();
}

AcceptLanguageList& AcceptLanguageList::operator=(const AcceptLanguageList& x)
{
    AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    const AcceptLanguageArray& other = GetAcceptLanguageArray(&x);

    if (&self != &other)
        self = other;
    return *this;
}

Uint32 AcceptLanguageList::size() const
{
    const AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    return self.size();
}

LanguageTag AcceptLanguageList::getLanguageTag(Uint32 index) const
{
    const AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    return self[index].first;
}

Real32 AcceptLanguageList::getQualityValue(Uint32 i) const
{
    const AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    return self[i].second;
}

void AcceptLanguageList::insert(
    const LanguageTag& languageTag,
    Real32 qualityValue)
{
    LanguageParser::validateQualityValue(qualityValue);

    AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    Uint32 i;
    Uint32 n = self.size();

    for (i = 0; i < n; i++)
    {
        if (self[i].second < qualityValue)
        {
            // Insert the new element before the element at this index
            break;
        }
    }

    self.insert(i, AcceptLanguagePair(languageTag, qualityValue));
}

void AcceptLanguageList::remove(Uint32 i)
{
    AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    self.remove(i);
}

Uint32 AcceptLanguageList::find(const LanguageTag& languageTag) const
{
    const AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    Uint32 n = self.size();

    for (Uint32 i = 0; i < n; i++)
    {
        if (languageTag == self[i].first)
            return i;
    }

    return PEG_NOT_FOUND;
}

void AcceptLanguageList::clear()
{
    AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    self.clear();
}

Boolean AcceptLanguageList::operator==(const AcceptLanguageList& x) const
{
    const AcceptLanguageArray& self = GetAcceptLanguageArray(this);
    const AcceptLanguageArray& other = GetAcceptLanguageArray(&x);

    Uint32 n = self.size();

    if (n != other.size())
        return false;

    for (Uint32 i = 0; i < n; i++)
    {
        if (self[i].first != other[i].first ||
            self[i].second != other[i].second)
        {
            return false;
        }
    }

    return true;
}

Boolean AcceptLanguageList::operator!=(const AcceptLanguageList& x) const
{
    return !operator==(x);
}

PEGASUS_NAMESPACE_END
