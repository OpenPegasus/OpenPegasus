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

#include "RepositoryDeclContext.h"
#include "CIMRepository.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

RepositoryDeclContext::RepositoryDeclContext()
{
}

RepositoryDeclContext::~RepositoryDeclContext()
{
}

CIMQualifierDecl RepositoryDeclContext::lookupQualifierDecl(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName) const
{
    // Ignore the exception since this routine is only supposed report
    // whether it can be found:

    try
    {
        return _repository->_getQualifier(nameSpace, qualifierName);
    }
    catch (Exception&)
    {
        return CIMQualifierDecl();
    }
}

CIMClass RepositoryDeclContext::lookupClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className) const
{
    // Ignore the exception since this routine is only supposed report
    // whether it can be found:

    try
    {
        return _repository->_getClass(
            nameSpace, className, false, true, true, CIMPropertyList());
    }
    catch (Exception&)
    {
        return CIMClass();
    }
}

PEGASUS_NAMESPACE_END
