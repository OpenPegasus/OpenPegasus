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

#include <Pegasus/Common/LanguageTag.h>
#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_NAMESPACE_BEGIN

class LanguageTagRep
{
public:
    LanguageTagRep() : refs(1) { }
    AtomicInt refs;
    String tag;           // complete language tag
    String language;      // language part of language tag
    String country;       // country code part of the language tag
    String variant;       // language variant part of the language tag
};

static inline void _ref(LanguageTagRep* rep)
{
    if (rep)
        (reinterpret_cast<LanguageTagRep*>(rep))->refs++;
}

static inline void _unref(LanguageTagRep* rep)
{
    if (rep &&
        (reinterpret_cast<LanguageTagRep*>(rep))->refs.decAndTestIfZero())
    {
        delete reinterpret_cast<LanguageTagRep*>(rep);
    }
}

LanguageTag::LanguageTag()
{
    _rep = 0;
}

LanguageTag::LanguageTag(const String& languageTagString)
{
    _rep = new LanguageTagRep();

    try
    {
        LanguageParser::parseLanguageTag(
            languageTagString,
            _rep->language,
            _rep->country,
            _rep->variant);
    }
    catch (...)
    {
        _unref(_rep);
        throw;
    }

    _rep->tag = languageTagString;
}

LanguageTag::LanguageTag(const LanguageTag& x)
{
    _ref(_rep = x._rep);
}

LanguageTag::~LanguageTag()
{
    _unref(_rep);
}

LanguageTag& LanguageTag::operator=(const LanguageTag& x)
{
    if (_rep != x._rep)
    {
        _unref(_rep);
        _ref(_rep = x._rep);
    }

    return *this;
}

String LanguageTag::getLanguage() const
{
    CheckRep(_rep);
    return _rep->language;
}

String LanguageTag::getCountry() const
{
    CheckRep(_rep);
    return _rep->country;
}

String LanguageTag::getVariant() const
{
    CheckRep(_rep);
    return _rep->variant;
}

String LanguageTag::toString() const
{
    CheckRep(_rep);
    return _rep->tag;
}

Boolean LanguageTag::operator==(const LanguageTag& languageTag) const
{
    return String::equalNoCase(toString(), languageTag.toString());
}

Boolean LanguageTag::operator!=(const LanguageTag& languageTag) const
{
    return !String::equalNoCase(toString(), languageTag.toString());
}

PEGASUS_NAMESPACE_END
