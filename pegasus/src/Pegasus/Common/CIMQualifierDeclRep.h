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

#ifndef Pegasus_QualifierDeclRep_h
#define Pegasus_QualifierDeclRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/CIMScope.h>
#include <Pegasus/Common/Buffer.h>

PEGASUS_NAMESPACE_BEGIN

class CIMConstQualifierDecl;
class CIMQualifierDecl;

class CIMQualifierDeclRep
{
public:

    CIMQualifierDeclRep(
        const CIMName& name,
        const CIMValue& value,
        const CIMScope & scope,
        const CIMFlavor & flavor,
        Uint32 arraySize);

    const CIMName& getName() const
    {
        return _name;
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


    const CIMScope & getScope () const
    {
        return _scope;
    }

    const CIMFlavor & getFlavor() const
    {
        return _flavor;
    }

    Uint32 getArraySize() const
    {
        return _arraySize;
    }

    Boolean identical(const CIMQualifierDeclRep* x) const;

    CIMQualifierDeclRep* clone() const
    {
        return new CIMQualifierDeclRep(*this);
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

    CIMQualifierDeclRep(const CIMQualifierDeclRep& x);

    CIMQualifierDeclRep();    // Unimplemented
    // Unimplemented
    CIMQualifierDeclRep& operator=(const CIMQualifierDeclRep& x);

    CIMName _name;
    CIMValue _value;
    CIMScope _scope;
    CIMFlavor _flavor;
    Uint32 _arraySize;

    // reference counter as member to avoid
    // virtual function resolution overhead
    AtomicInt _refCounter;
    friend class CIMBuffer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_QualifierDeclRep_h */
