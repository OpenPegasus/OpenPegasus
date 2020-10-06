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

#ifndef Pegasus_CIMOMHandleQueryContext_h
#define Pegasus_CIMOMHandleQueryContext_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Provider/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Query/QueryCommon/QueryContext.h>
#include <Pegasus/Provider/CIMOMHandle.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN


class PEGASUS_PROVIDER_LINKAGE CIMOMHandleQueryContext: public QueryContext
{
public:

    CIMOMHandleQueryContext(const CIMNamespaceName& inNS, CIMOMHandle& handle);

    CIMOMHandleQueryContext(const CIMOMHandleQueryContext& handle);

    ~CIMOMHandleQueryContext();

    QueryContext* clone() const;

    CIMOMHandleQueryContext& operator=(const CIMOMHandleQueryContext& rhs);

    CIMClass getClass (const CIMName& inClassName) const;

    Array<CIMName> enumerateClassNames(const CIMName& inClassName) const;

    // Returns true if the derived class is a subclass of the base class.
    // Note: this will return false if the classes are the same.
    // Note: the default namespace of the query is used.
    Boolean isSubClass(
        const CIMName& baseClass,
        const CIMName& derivedClass)const;

    // Returns the relationship between the anchor class and the related
    // class in the class schema of the query's default name space.
    ClassRelation getClassRelation(
        const CIMName& anchorClass,
        const CIMName& relatedClass) const;

private:
    CIMOMHandleQueryContext();

    // members
    CIMOMHandle _CH;
};

PEGASUS_NAMESPACE_END
#endif
#endif
