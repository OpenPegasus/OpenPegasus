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

#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/ArrayInternal.h>

PEGASUS_NAMESPACE_BEGIN

//
// Implementation Notes:
// =====================
//
// See the implemetnation notes in AcceptLanguageList.cpp for an explanation
// of how we overlay the _rep pointer with the array class.
//

typedef Array<LanguageTag> LanguageTagArray;

static inline LanguageTagArray& GetLanguageTagArray(
    ContentLanguageList* list)
{
    return *reinterpret_cast<LanguageTagArray*>(list);
}

static inline const LanguageTagArray& GetLanguageTagArray(
    const ContentLanguageList* list)
{
    return *reinterpret_cast<const LanguageTagArray*>(list);
}

ContentLanguageList::ContentLanguageList()
{
    LanguageTagArray& self = GetLanguageTagArray(this);
    new (&self) LanguageTagArray();
}

ContentLanguageList::ContentLanguageList(
    const ContentLanguageList& x)
{
    LanguageTagArray& self = GetLanguageTagArray(this);
    const LanguageTagArray& other = GetLanguageTagArray(&x);
    new (&self) LanguageTagArray(other);
}

ContentLanguageList::~ContentLanguageList()
{
    LanguageTagArray& self = GetLanguageTagArray(this);
    self.~LanguageTagArray();
}

ContentLanguageList& ContentLanguageList::operator=(
    const ContentLanguageList& x)
{
    LanguageTagArray& self = GetLanguageTagArray(this);
    const LanguageTagArray& other = GetLanguageTagArray(&x);
    self = other;
    return *this;
}

Uint32 ContentLanguageList::size() const
{
    const LanguageTagArray& self = GetLanguageTagArray(this);
    return self.size();
}

LanguageTag ContentLanguageList::getLanguageTag(Uint32 index) const
{
    const LanguageTagArray& self = GetLanguageTagArray(this);
    return self[index];
}

void ContentLanguageList::append(const LanguageTag& languageTag)
{
    LanguageTagArray& self = GetLanguageTagArray(this);

    // Disallow "*" language tag
    if (languageTag.toString() == "*")
    {
        MessageLoaderParms parms(
            "Common.LanguageParser.INVALID_LANGUAGE_TAG",
            "Invalid language tag \"$0\".", languageTag.toString());
        throw InvalidContentLanguageHeader(MessageLoader::getMessage(parms));
    }

    self.append(languageTag);
}

void ContentLanguageList::remove(Uint32 index)
{
    LanguageTagArray& self = GetLanguageTagArray(this);
    self.remove(index);
}

Uint32 ContentLanguageList::find(const LanguageTag& languageTag) const
{
    const LanguageTagArray& self = GetLanguageTagArray(this);

    for (Uint32 i = 0; i < self.size(); i++)
    {
        if (languageTag == self[i])
            return i;
    }

    return PEG_NOT_FOUND;
}

void ContentLanguageList::clear()
{
    LanguageTagArray& self = GetLanguageTagArray(this);
    self.clear();
}

Boolean ContentLanguageList::operator==(const ContentLanguageList& x) const
{
    const LanguageTagArray& self = GetLanguageTagArray(this);
    const LanguageTagArray& other = GetLanguageTagArray(&x);

    if (self.size() != other.size())
        return false;

    for (Uint32 i = 0; i < self.size(); i++)
    {
        if (self[i] != other[i])
            return false;
    }

    return true;
}

Boolean ContentLanguageList::operator!=( const ContentLanguageList& x) const
{
    return !operator==(x);
}

PEGASUS_NAMESPACE_END
