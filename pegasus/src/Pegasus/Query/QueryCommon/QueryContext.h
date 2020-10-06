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

#ifndef Pegasus_QueryContext_h
#define Pegasus_QueryContext_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Query/QueryCommon/Linkage.h>
#include <Pegasus/Query/QueryCommon/QueryChainedIdentifier.h>
#include <Pegasus/Query/QueryCommon/QueryIdentifier.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/String.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

typedef HashTable<String, QueryIdentifier, EqualNoCaseFunc, HashLowerCaseFunc>
    HT_Alias_Class;

class PEGASUS_QUERYCOMMON_LINKAGE QueryContext
{
public:

    enum ClassRelation
    {
        SAMECLASS,
        SUBCLASS,
        SUPERCLASS,
        NOTRELATED
    };

    virtual ~QueryContext();

    virtual QueryContext* clone() const = 0;

    String getHost(Boolean fullyQualified = true);

    CIMNamespaceName getNamespace() const;

    void insertClassPath(
        const QueryIdentifier& inIdentifier,
        String inAlias = String::EMPTY);

    void addWhereIdentifier(const QueryChainedIdentifier& inIdentifier) const;

    Array<QueryChainedIdentifier> getWhereList() const;

    QueryIdentifier findClass(const String& inAlias) const;

    Array<QueryIdentifier> getFromList() const;

    String getFromString() const;

    virtual CIMClass getClass(const CIMName& inClassName)const = 0;

    virtual Array<CIMName> enumerateClassNames(
        const CIMName& inClassName) const = 0;

    // Returns true if the derived class is a subclass of the base class.
    // Note: this will return false if the classes are the same.
    // Note: the default namespace of the query is used.
    virtual Boolean isSubClass(
        const CIMName& baseClass,
        const CIMName& derivedClass) const = 0;

    // Returns the relationship between the anchor class and the related
    // class in the class schema of the query's default name space.
    virtual ClassRelation getClassRelation(
        const CIMName& anchorClass,
        const CIMName& relatedClass) const = 0;

    void clear();

protected:

    QueryContext(const QueryContext& ctx);

    QueryContext(const CIMNamespaceName& inNS);

    QueryContext& operator=(const QueryContext& rhs);

private:

    QueryContext();

    // members
    CIMNamespaceName _NS;
    HT_Alias_Class _AliasClassTable;
    Array<QueryIdentifier> _fromList;
    mutable Array<QueryChainedIdentifier> _whereList;
};

PEGASUS_NAMESPACE_END

#endif

#endif
