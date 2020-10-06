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

#ifndef Pegasus_MethodRep_h
#define Pegasus_MethodRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMQualifierList.h>
#include <Pegasus/Common/CIMParameter.h>
#include <Pegasus/Common/CIMParameterRep.h>
#include <Pegasus/Common/Pair.h>
#include <Pegasus/Common/OrderedSet.h>

PEGASUS_NAMESPACE_BEGIN

class CIMConstMethod;
class DeclContext;

class CIMMethodRep
{
public:

    CIMMethodRep(
        const CIMName& name,
        CIMType type,
        const CIMName& classOrigin,
        Boolean propagated);

    const CIMName& getName() const
    {
        return _name;
    }

    Uint32 getNameTag() const
    {
        return _nameTag;
    }

    void increaseOwnerCount()
    {
        _ownerCount++;
        return;
    }

    void decreaseOwnerCount()
    {
        _ownerCount--;
        return;
    }

    void setName(const CIMName& name);

    CIMType getType() const
    {
        return _type;
    }

    void setType(CIMType type)
    {
        _type = type;
    }

    const CIMName& getClassOrigin() const
    {
        return _classOrigin;
    }

    void setClassOrigin(const CIMName& classOrigin)
    {
        _classOrigin = classOrigin;
    }

    Boolean getPropagated() const
    {
        return _propagated;
    }

    void setPropagated(Boolean propagated)
    {
        _propagated = propagated;
    }

    void addQualifier(const CIMQualifier& qualifier)
    {
        _qualifiers.add(qualifier);
    }

    Uint32 findQualifier(const CIMName& name) const
    {
        return _qualifiers.find(name);
    }

    CIMQualifier getQualifier(Uint32 index)
    {
        return _qualifiers.getQualifier(index);
    }


    CIMConstQualifier getQualifier(Uint32 index) const
    {
        return _qualifiers.getQualifier(index);
    }

    void removeQualifier(Uint32 index)
    {
        _qualifiers.removeQualifier(index);
    }


    Uint32 getQualifierCount() const
    {
        return _qualifiers.getCount();
    }

    void addParameter(const CIMParameter& x);

    Uint32 findParameter(const CIMName& name) const
    {
        return _parameters.find(name, generateCIMNameTag(name));
    }

    CIMParameter getParameter(Uint32 index)
    {
        return _parameters[index];
    }

    CIMConstParameter getParameter(Uint32 index) const
    {
        return ((CIMMethodRep*)this)->getParameter(index);
    }

    void removeParameter (Uint32 index)
    {
        _parameters.remove (index);
    }

    Uint32 getParameterCount() const
    {
        return _parameters.size();
    }

    void resolve(
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace,
        const CIMConstMethod& method);

    void resolve(
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace);

    Boolean identical(const CIMMethodRep* x) const;

    CIMMethodRep* clone() const
    {
        return new CIMMethodRep(*this);
    }

    void Inc()
    {
        _refCounter++;
    }

    void Dec()
    {
        if (_refCounter.decAndTestIfZero())
            delete this;
    }

private:

    CIMMethodRep(const CIMMethodRep& x);

    CIMMethodRep();    // Unimplemented
    CIMMethodRep& operator=(const CIMMethodRep& x);    // Unimplemented

    CIMName _name;
    CIMType _type;
    CIMName _classOrigin;
    Boolean _propagated;
    CIMQualifierList _qualifiers;
    Uint32 _nameTag;
    Uint32 _ownerCount;

    // reference counter as member to avoid
    // virtual function resolution overhead
    AtomicInt _refCounter;

    typedef OrderedSet<CIMParameter,
                       CIMParameterRep,
                       PEGASUS_PARAMETER_ORDEREDSET_HASHSIZE> ParameterSet;
    ParameterSet _parameters;

    friend class CIMClassRep;
    friend class CIMBuffer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_MethodRep_h */
