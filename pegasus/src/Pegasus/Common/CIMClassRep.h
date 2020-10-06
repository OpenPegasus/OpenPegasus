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

#ifndef Pegasus_CIMClassRep_h
#define Pegasus_CIMClassRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObjectRep.h>
#include <Pegasus/Common/CIMMethod.h>
#include <Pegasus/Common/CIMMethodRep.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class DeclContext;
class CIMClass;
class CIMConstClass;
class CIMInstanceRep;

// ATTN: KS P3 -document the CIMClass and CIMObjectRep  classes.

class CIMClassRep : public CIMObjectRep
{
public:

    CIMClassRep(
        const CIMName& className,
        const CIMName& superClassName);

    virtual ~CIMClassRep();

    Boolean isAssociation() const;

    Boolean isAbstract() const;

    const CIMName& getSuperClassName() const { return _superClassName; }

    void setSuperClassName(const CIMName& superClassName)
    {
        _superClassName = superClassName;
    }

    virtual void addProperty(const CIMProperty& x);

    void addMethod(const CIMMethod& x);

    Uint32 findMethod(const CIMName& name) const
    {
        return _methods.find(name, generateCIMNameTag(name));
    }

    CIMMethod getMethod(Uint32 index)
    {
        return _methods[index];
    }

    CIMConstMethod getMethod(Uint32 index) const
    {
        return ((CIMClassRep*)this)->getMethod(index);
    }

    void removeMethod(Uint32 index)
    {
        _methods.remove(index);
    }

    Uint32 getMethodCount() const
    {
        return _methods.size();
    }

    void resolve(
        DeclContext* context,
        const CIMNamespaceName& nameSpace);

    virtual Boolean identical(const CIMObjectRep* x) const;

    virtual CIMObjectRep* clone() const
    {
        return new CIMClassRep(*this);
    }

    void getKeyNames(Array<CIMName>& keyNames) const;

    Boolean hasKeys() const;

    CIMInstance buildInstance(Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList & propertyList) const;

private:

    CIMClassRep(const CIMClassRep& x);

    CIMClassRep();    // Unimplemented
    CIMClassRep& operator=(const CIMClassRep& x);    // Unimplemented

    CIMName _superClassName;
    typedef OrderedSet<CIMMethod,
                       CIMMethodRep,
                       PEGASUS_METHOD_ORDEREDSET_HASHSIZE> MethodSet;
    MethodSet _methods;

    friend class CIMClass;
    friend class CIMInstanceRep;
    friend class BinaryStreamer;
    friend class CIMBuffer;
    friend class SCMOClass;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMClassRep_h */
