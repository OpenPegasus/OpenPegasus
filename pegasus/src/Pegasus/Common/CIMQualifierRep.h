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

#ifndef Pegasus_QualifierRep_h
#define Pegasus_QualifierRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Pair.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/OrderedSet.h>

PEGASUS_NAMESPACE_BEGIN

class CIMConstQualifier;
class CIMQualifier;

class CIMQualifierRep
{
public:

    CIMQualifierRep(
        const CIMName& name,
        const CIMValue& value,
        const CIMFlavor& flavor,
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
        return _value.getType();
    }

    Boolean isArray() const
    {
        return _value.isArray();
    }

    const CIMValue& getValue() const
    {
        return _value;
    }

    void setValue(const CIMValue& value)
    {
        _value = value;
    }

    const CIMFlavor & getFlavor() const
    {
        return _flavor;
    }

    void setFlavor(const CIMFlavor & flavor)
    {
        _flavor = flavor;
    }

    void unsetFlavor(const CIMFlavor & flavor)
    {
        _flavor.removeFlavor (flavor);
    }

    void resolveFlavor (const CIMFlavor & inheritedFlavor);

    Boolean getPropagated() const
    {
        return _propagated;
    }

    void setPropagated(Boolean propagated)
    {
        _propagated = propagated;
    }

    Boolean identical(const CIMQualifierRep* x) const;

    CIMQualifierRep* clone() const
    {
        return new CIMQualifierRep(*this);
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

    // Cloning constructor:

    CIMQualifierRep(const CIMQualifierRep& x);

    CIMQualifierRep();    // Unimplemented
    CIMQualifierRep& operator=(const CIMQualifierRep& x);    // Unimplemented

    CIMName _name;
    CIMValue _value;
    CIMFlavor _flavor;
    Boolean _propagated;
    Uint32 _nameTag;

    // reference counter as member to avoid
    // virtual function resolution overhead
    AtomicInt _refCounter;
    Uint32 _ownerCount;

    friend class CIMQualifierList;
    friend class CIMBuffer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_QualifierRep_h */
