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

#ifndef _Pegasus_StringInline_h
#define _Pegasus_StringInline_h

#include <Pegasus/Common/StringRep.h>
#include <cstring>

PEGASUS_NAMESPACE_BEGIN

#if !defined(PEGASUS_DISABLE_INTERNAL_INLINES)

#ifdef PEGASUS_INTERNALONLY
# define PEGASUS_STRING_INLINE inline
#else
# define PEGASUS_STRING_INLINE /* empty */
#endif

PEGASUS_STRING_INLINE CString::CString() : _rep(0)
{
}

PEGASUS_STRING_INLINE CString::CString(char* cstr) : _rep(cstr)
{
}

PEGASUS_STRING_INLINE CString::~CString()
{
    operator delete(_rep);
}

PEGASUS_STRING_INLINE CString::operator const char*() const
{
    return _rep;
}

PEGASUS_STRING_INLINE String::String()
{
    // Note: ref() and unref() never touch the reference count of _emptyRep.
    // This allows use to optimize the copy constructor by not incrementing
    // _emptyRep.refs here. Performance is critical in this function. Please
    // do not add any code to this function.
    _rep = &StringRep::_emptyRep;
}

PEGASUS_STRING_INLINE String::String(const String& str)
{
#ifdef PEGASUS_HAVE_BROKEN_GLOBAL_CONSTRUCTION
    //
    // Some compilers don't do a good job of initializing global
    //   constructors in the proper sequence.  This is one such case.
    // String::EMPTY is not initialized by the time this is first
    //   called during initialization of the executable.
    //
    if (!str._rep)
    {
      _rep = &StringRep::_emptyRep;
      return;
    }
#endif
    StringRep::ref(_rep = str._rep);
}

PEGASUS_STRING_INLINE String::~String()
{
    StringRep::unref(_rep);
}

PEGASUS_STRING_INLINE Uint32 String::size() const
{
    return (Uint32)_rep->size;
}

PEGASUS_STRING_INLINE const Char16* String::getChar16Data() const
{
    return (Char16*)&(_rep->data[0]);
}

PEGASUS_STRING_INLINE Char16& String::operator[](Uint32 i)
{
    _checkBounds(i, _rep->size);

    if (_rep->refs.get() != 1)
        _rep = StringRep::copyOnWrite(_rep);

    return (Char16&)_rep->data[i];
}

PEGASUS_STRING_INLINE const Char16 String::operator[](Uint32 i) const
{
    _checkBounds(i, _rep->size);
    return (Char16&)_rep->data[i];
}

PEGASUS_STRING_INLINE String& String::operator=(const String& str)
{
    return assign(str);
}

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
PEGASUS_STRING_INLINE String& String::operator=(const char* str)
{
    return assign(str);
}
#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

PEGASUS_STRING_INLINE String& String::assign(const Char16* str)
{
    return assign(str, StringRep::length((Uint16*)str));
}

PEGASUS_STRING_INLINE String& String::assign(const char* str)
{
    return assign(str, (Uint32)strlen(str));
}

PEGASUS_STRING_INLINE Uint32 String::find(const String& s) const
{
    return StringFindAux(_rep, (Char16*)&(s._rep->data[0]),
        (Uint32)s._rep->size);
}

PEGASUS_STRING_INLINE String& String::append(const Char16& c)
{
    if (_rep->size == _rep->cap || _rep->refs.get() != 1)
        StringAppendCharAux(_rep);

    _rep->data[_rep->size++] = c;
    _rep->data[_rep->size] = 0;
    return *this;
}

PEGASUS_STRING_INLINE Boolean String::equalNoCase(
    const String& s1, const String& s2)
{
#ifdef PEGASUS_HAS_ICU
    return StringEqualNoCase(s1, s2);
#else
    return s1._rep->size == s2._rep->size && StringEqualNoCase(s1, s2);
#endif
}

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
PEGASUS_STRING_INLINE String& String::append(const char* str)
{
    append(str, (Uint32)strlen(str));
    return *this;
}
#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

PEGASUS_STRING_INLINE Boolean operator==(const String& s1, const String& s2)
{
    return String::equal(s1, s2);
}

PEGASUS_STRING_INLINE Boolean operator==(const String& s1, const char* s2)
{
    return String::equal(s1, s2);
}

PEGASUS_STRING_INLINE Boolean operator==(const char* s1, const String& s2)
{
    return String::equal(s2, s1);
}

PEGASUS_STRING_INLINE Boolean operator!=(const String& s1, const String& s2)
{
    return !String::equal(s1, s2);
}

PEGASUS_STRING_INLINE Boolean operator!=(const String& s1, const char* s2)
{
    return !String::equal(s1, s2);
}

PEGASUS_STRING_INLINE Boolean operator!=(const char* s1, const String& s2)
{
    return !String::equal(s2, s1);
}

PEGASUS_STRING_INLINE Boolean operator<(const String& s1, const String& s2)
{
    return String::compare(s1, s2) < 0;
}

PEGASUS_STRING_INLINE Boolean operator<(const String& s1, const char* s2)
{
    return String::compare(s1, s2) < 0;
}

PEGASUS_STRING_INLINE Boolean operator<(const char* s1, const String& s2)
{
    return String::compare(s2, s1) > 0;
}

PEGASUS_STRING_INLINE Boolean operator>(const String& s1, const String& s2)
{
    return String::compare(s1, s2) > 0;
}

PEGASUS_STRING_INLINE Boolean operator>(const String& s1, const char* s2)
{
    return String::compare(s1, s2) > 0;
}

PEGASUS_STRING_INLINE Boolean operator>(const char* s1, const String& s2)
{
    return String::compare(s2, s1) < 0;
}

PEGASUS_STRING_INLINE Boolean operator<=(const String& s1, const String& s2)
{
    return String::compare(s1, s2) <= 0;
}

PEGASUS_STRING_INLINE Boolean operator<=(const String& s1, const char* s2)
{
    return String::compare(s1, s2) <= 0;
}

PEGASUS_STRING_INLINE Boolean operator<=(const char* s1, const String& s2)
{
    return String::compare(s2, s1) >= 0;
}

PEGASUS_STRING_INLINE Boolean operator>=(const String& s1, const String& s2)
{
    return String::compare(s1, s2) >= 0;
}

PEGASUS_STRING_INLINE Boolean operator>=(const String& s1, const char* s2)
{
    return String::compare(s1, s2) >= 0;
}

PEGASUS_STRING_INLINE Boolean operator>=(const char* s1, const String& s2)
{
    return String::compare(s2, s1) <= 0;
}

PEGASUS_STRING_INLINE String operator+(const String& s1, const String& s2)
{
#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    return String(s1, s2);
#else
    String tmp;
    tmp.reserveCapacity(s1.size() + s2.size());
    tmp.append(s1);
    tmp.append(s2);
    return tmp;
#endif
}

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
PEGASUS_STRING_INLINE String operator+(const String& s1, const char* s2)
{
    return String(s1, s2);
}
#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
PEGASUS_STRING_INLINE String operator+(const char* s1, const String& s2)
{
    return String(s1, s2);
}
#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

#endif /* !defined(PEGASUS_DISABLE_INTERNAL_INLINES) */

/**
    Fast way to assign a given character string consisting of ASCII only and
    legal characters for a CIMName (i.e. letter, numbers and underscore)
    to a String reference.

    @param s reference to the String object which will be changed
    @param str character string
    @param n number of characters which shall be assigned from str to s
*/
PEGASUS_COMMON_LINKAGE void AssignASCII(String& s, const char* str, Uint32 n);

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_StringInline_h */
