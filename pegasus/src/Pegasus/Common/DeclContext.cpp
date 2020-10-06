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

#include "DeclContext.h"
#include "CIMName.h"
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

DeclContext::~DeclContext()
{

}

SimpleDeclContext::~SimpleDeclContext()
{

}

void SimpleDeclContext::addQualifierDecl(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& x)
{
    if (!lookupQualifierDecl(nameSpace, x.getName()).isUninitialized())
    {
        MessageLoaderParms parms(
            "Common.DeclContext.DECLARATION_OF_QUALIFIER",
            "declaration of qualifier \"$0\"",
            x.getName().getString());
        throw AlreadyExistsException(parms);
    }

    _qualifierDeclarations.append(QPair(nameSpace, x));
}

void SimpleDeclContext::addClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& x)
{
    if (!lookupClass(nameSpace, x.getClassName()).isUninitialized())
    {
        MessageLoaderParms parms(
            "Common.DeclContext.CLASS",
            "class \"$0\"",
            x.getClassName().getString());
        throw AlreadyExistsException(parms);
    }

    _classDeclarations.append(CPair(nameSpace, x));
}

CIMQualifierDecl SimpleDeclContext::lookupQualifierDecl(
    const CIMNamespaceName& nameSpace,
    const CIMName& name) const
{
    for (Uint32 i = 0, n = _qualifierDeclarations.size(); i < n; i++)
    {
        const CIMNamespaceName& first = _qualifierDeclarations[i].first;
        const CIMQualifierDecl& second = _qualifierDeclarations[i].second;

        if (first.equal(nameSpace) &&
            second.getName().equal(name))
        {
            return second;
        }
    }

    // Not found:
    return CIMQualifierDecl();
}

CIMClass SimpleDeclContext::lookupClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& name) const
{
    for (Uint32 i = 0, n = _classDeclarations.size(); i < n; i++)
    {
        const CIMNamespaceName& first = _classDeclarations[i].first;
        const CIMClass& second = _classDeclarations[i].second;

        if (first.equal(nameSpace) &&
            second.getClassName().equal(name))
        {
            return second;
        }
    }

    // Not found:
    return CIMClass();
}

PEGASUS_NAMESPACE_END
