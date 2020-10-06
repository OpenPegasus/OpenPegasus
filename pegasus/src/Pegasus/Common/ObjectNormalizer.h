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

#ifndef Pegasus_ObjectNormalizer_h
#define Pegasus_ObjectNormalizer_h

#include <Pegasus/Common/SharedPtr.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE NormalizerContext
{
public:
    virtual ~NormalizerContext() {};

    virtual CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& name) = 0;

    virtual Array<CIMName> enumerateClassNames(
        const CIMNamespaceName& nameSpace, const CIMName& className,
        bool deepInheritance) = 0;

    virtual NormalizerContext* clone() = 0;
};

// TODO: add documentation
class PEGASUS_COMMON_LINKAGE ObjectNormalizer
{
public:
    ObjectNormalizer();
    ObjectNormalizer(
        const CIMClass& cimClass,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMNamespaceName& nameSpace,
        SharedPtr<NormalizerContext>& context);

    CIMObjectPath processClassObjectPath(
        const CIMObjectPath& cimObjectPath) const;
    CIMObjectPath processInstanceObjectPath(
        const CIMObjectPath& cimObjectPath) const;

    //CIMClass processClass(const CIMClass& cimClass) const;
    CIMInstance processInstance(const CIMInstance& cimInstance) const;
    //CIMIndication processIndication(const CIMIndication& cimIndication);

    static CIMProperty processProperty(
        CIMConstProperty& referenceProperty,
        CIMConstProperty& cimProperty,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        NormalizerContext * context,
        const CIMNamespaceName& nameSpace);

    static void setEnableNormalization(Boolean value);

    static Boolean getEnableNormalization();
private:
    CIMClass _cimClass;

    Boolean _includeQualifiers;
    Boolean _includeClassOrigin;
    SharedPtr<NormalizerContext> _context;
    CIMNamespaceName _nameSpace;
    static Boolean _enableNormalization;
};

PEGASUS_NAMESPACE_END

#endif
