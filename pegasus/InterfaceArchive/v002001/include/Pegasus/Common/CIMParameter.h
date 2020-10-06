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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Parameter_h
#define Pegasus_Parameter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMParameter
//
////////////////////////////////////////////////////////////////////////////////

class Resolver;
class CIMConstParameter;
class CIMParameterRep;

///
class PEGASUS_COMMON_LINKAGE CIMParameter
{
public:

    ///
    CIMParameter();

    ///
    CIMParameter(const CIMParameter& x);

    ///
    CIMParameter(
	const CIMName& name, 
	CIMType type,
	Boolean isArray = false,
	Uint32 arraySize = 0,
	const CIMName& referenceClassName = CIMName());

    ///
    ~CIMParameter();

    ///
    CIMParameter& operator=(const CIMParameter& x);

    ///
    const CIMName& getName() const ;

    ///
    void setName(const CIMName& name);

    ///
    Boolean isArray() const;

    ///
    Uint32 getArraySize() const;

    ///
    const CIMName& getReferenceClassName() const ;

    ///
    CIMType getType() const ;

    // Throws AlreadyExistsException.
    ///
    CIMParameter& addQualifier(const CIMQualifier& x);

    ///
    Uint32 findQualifier(const CIMName& name) const;

    ///
    CIMQualifier getQualifier(Uint32 index);

    /** Removes the CIMQualifier defined by the input parameter.
        @param index - Index of the qualifier to be removed.
        @exception IndexOutOfBoundsException if the index is outside
        the range of qualifiers available for the CIMParameter.
    */
    void removeQualifier (Uint32 index);

    ///
    CIMConstQualifier getQualifier(Uint32 index) const;

    ///
    Uint32 getQualifierCount() const;

    /** Determines if the object has not been initialized.
        @return  true if the object has not been initialized,
                 false otherwise.
     */
    Boolean isUninitialized() const;

    ///
    Boolean identical(const CIMConstParameter& x) const;

    ///
    CIMParameter clone() const;

private:

    CIMParameter(CIMParameterRep* rep);

    void _checkRep() const;

    CIMParameterRep* _rep;

    friend class CIMConstParameter;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstParameter
//
////////////////////////////////////////////////////////////////////////////////

///
class PEGASUS_COMMON_LINKAGE CIMConstParameter
{
public:

    ///
    CIMConstParameter();

    ///
    CIMConstParameter(const CIMConstParameter& x);

    ///
    CIMConstParameter(const CIMParameter& x);

    ///
    CIMConstParameter(
	const CIMName& name, 
	CIMType type,
	Boolean isArray = false,
	Uint32 arraySize = 0,
	const CIMName& referenceClassName = CIMName());

    ///
    ~CIMConstParameter();

    ///
    CIMConstParameter& operator=(const CIMConstParameter& x);

    ///
    CIMConstParameter& operator=(const CIMParameter& x);

    ///
    const CIMName& getName() const;

    ///
    Boolean isArray() const;

    ///
    Uint32 getArraySize() const;

    ///
    const CIMName& getReferenceClassName() const;

    ///
    CIMType getType() const;

    ///
    Uint32 findQualifier(const CIMName& name) const;

    ///
    CIMConstQualifier getQualifier(Uint32 index) const;

    ///
    Uint32 getQualifierCount() const;

    ///
    Boolean isUninitialized() const;

    ///
    Boolean identical(const CIMConstParameter& x) const;

    ///
    CIMParameter clone() const;

private:

    void _checkRep() const;

    CIMParameterRep* _rep;
    friend class CIMParameter;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMParameter
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Parameter_h */
