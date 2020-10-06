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

#if !defined(Pegasus_CIMNameInline_cxx)
# if !defined(PEGASUS_INTERNALONLY) || defined(PEGASUS_DISABLE_INTERNAL_INLINES)
#   define Pegasus_CIMNameInline_h
# endif
#endif

#ifndef Pegasus_CIMNameInline_h
#define Pegasus_CIMNameInline_h

#include <Pegasus/Common/CIMName.h>

#ifdef Pegasus_CIMNameInline_cxx
# define PEGASUS_CIMNAME_INLINE
#else
# define PEGASUS_CIMNAME_INLINE inline
#endif

PEGASUS_NAMESPACE_BEGIN

PEGASUS_CIMNAME_INLINE CIMName::CIMName()
{
}

PEGASUS_CIMNAME_INLINE CIMName& CIMName::operator=(const CIMName& name)
{
    cimName = name.cimName;
    return *this;
}

PEGASUS_CIMNAME_INLINE const String& CIMName::getString() const
{
    return cimName;
}

PEGASUS_CIMNAME_INLINE Boolean CIMName::isNull() const
{
    return cimName.size() == 0;
}

PEGASUS_CIMNAME_INLINE void CIMName::clear()
{
    cimName.clear();
}

PEGASUS_CIMNAME_INLINE Boolean CIMName::equal(const CIMName& name) const
{
    return String::equalNoCase(cimName, name.cimName);
}

PEGASUS_CIMNAME_INLINE Boolean CIMName::equal(const char* name) const
{
    return String::equalNoCase(cimName, name);
}

PEGASUS_CIMNAME_INLINE Boolean operator==(
    const CIMName& name1, const CIMName& name2)
{
    return name1.equal(name2);
}

PEGASUS_CIMNAME_INLINE Boolean operator==(
    const CIMName& name1, const char* name2)
{
    return name1.equal(name2);
}

PEGASUS_CIMNAME_INLINE Boolean operator==(
    const char* name1, const CIMName& name2)
{
    return name2.equal(name1);
}

PEGASUS_CIMNAME_INLINE Boolean operator!=(
    const CIMName& name1, const CIMName& name2)
{
    return !name1.equal(name2);
}

PEGASUS_CIMNAME_INLINE Boolean operator!=(
    const CIMName& name1, const char* name2)
{
    return !name1.equal(name2);
}

PEGASUS_CIMNAME_INLINE Boolean operator!=(
    const char* name1, const CIMName& name2)
{
    return !name2.equal(name1);
}

PEGASUS_CIMNAME_INLINE CIMNamespaceName::CIMNamespaceName()
{
}

PEGASUS_CIMNAME_INLINE const String& CIMNamespaceName::getString() const
{
    return cimNamespaceName;
}

PEGASUS_CIMNAME_INLINE Boolean CIMNamespaceName::isNull() const
{
    return cimNamespaceName.size() == 0;
}

PEGASUS_CIMNAME_INLINE void CIMNamespaceName::clear()
{
    cimNamespaceName.clear();
}

PEGASUS_CIMNAME_INLINE Boolean CIMNamespaceName::equal(
    const CIMNamespaceName& name) const
{
    return String::equalNoCase(cimNamespaceName, name.cimNamespaceName);
}

PEGASUS_CIMNAME_INLINE Boolean CIMNamespaceName::equal(const char* name) const
{
    return String::equalNoCase(cimNamespaceName, name);
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMNameInline_h */
