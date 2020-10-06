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

#ifndef Pegasus_Qualifier_h
#define Pegasus_Qualifier_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMValue.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMQualifier
//
////////////////////////////////////////////////////////////////////////////////

class CIMConstQualifier;
class CIMClassRep;
class Resolver;
class CIMQualifierRep;

/** The CIMQualifier class is used to represent CIM qualifiers in Pegasus.
    It is almost identical to \Ref{CIMQualifierDecl} except that it has no 
    scope member.
    This includes functions to create qualifiers and manipulate/test
    the individual components of the CIMQualifier.
    CIMQualifiers are accumulated into lists for use in CIMClasses and 
    CIMProperties using the \Ref{CIMQualifierList} Class and its
    functions.
*/
class PEGASUS_COMMON_LINKAGE CIMQualifier
{
public:

    /** Constructor - Creates a CIM qualifier object with empty name 
        value fields.
    */
    CIMQualifier();

    /** Constructor - Creates a CIM qualifier object from another 
        CIM qualifier object. 
        @param x - CIMQualifier object.
    */
    CIMQualifier(const CIMQualifier& x);

    /** Constructor - Creates a CIM qualifier object with the parameters 
        defined on input.
        @param name - CIMName representing name for the new qualifier.
        @param value - CIMValue from which to extract the value for the keys.
        @param flavor - Flavor defined for this qualifier definition. Default
        for this parameter is CIMFlavor::NONE.
        @param propagated - Boolean defining whether this is a propagated
        qualifier.  This is an optional parameter with default = false
    */
    CIMQualifier(
	const CIMName& name, 
	const CIMValue& value, 
	const CIMFlavor & flavor = CIMFlavor (CIMFlavor::NONE),
	Boolean propagated = false);

    /// Destructor
    ~CIMQualifier();

    /// 
    CIMQualifier& operator=(const CIMQualifier& x);

    /**	Returns the name field from the qualifier.
        @return CIMName containing the qualifier name.
    */
    const CIMName& getName() const;

    /**	Sets the qualifier name field in the qualifier object.
	@param name - CIMName containing the name for the qualifier.
    */
    void setName(const CIMName& name);

    /** Gets the type field from the qualifier object.
        @return CIMType containing the type for this qualifier.
    */
    CIMType getType() const;

    /**	Return true if the qualifier is an array.
	@return true if the qualifier is an array, false otherwise.
    */
    Boolean isArray() const;

    /**	Gets the value component of the qualifier object.
	@return CIMValue containing the value component.
    */
    const CIMValue& getValue() const;

    /**	Puts a CIMValue object into a CIMQualifier object.
	@param value - The CIMValue object to install.
    */
    void setValue(const CIMValue& value);

    /** Sets the bits defined on input into the Flavor variable
        for the Qualifier Object.
        @param flavor - CIMFlavor object defines the flavor bits to be set.
    */
    void setFlavor(const CIMFlavor & flavor);

    /** Resets the bits defined for the flavor 
        for the Qualifier Object with the input.
        @param flavor - CIMFlavor object defines the flavor bits to be unset.
    */
    void unsetFlavor(const CIMFlavor & flavor);

    /**	Gets the Flavor field from the Qualifier.
	@return CIMFlavor object from this qualifier.
    */
    const CIMFlavor & getFlavor() const;

    /** Gets the propagated qualifier.
        @return true if qualifier is propagated, false otherwise.
    */
    const Uint32 getPropagated() const;

    /**	Sets the Propagated flag for the object.
        @param propagated - Flag indicating propagation.
    */
    void setPropagated(Boolean propagated);

    /** Determines if the object has not been initialized.
        @return  true if the object has not been initialized,
                 false otherwise.
     */
    Boolean isUninitialized() const;

    /**	Compares two CIMQualifier objects.
        @return  true if the objects are identical, false otherwise.
    */
    Boolean identical(const CIMConstQualifier& x) const;

    /**	Creates an exact copy of the qualifier and returns the
	new object.
	@return copy of CIMQualifier object.
    */
    CIMQualifier clone() const;

private:

    CIMQualifier(CIMQualifierRep* rep);

    void _checkRep() const;

    CIMQualifierRep* _rep;

    friend class CIMConstQualifier;
    friend class CIMClassRep;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstQualifier
//
////////////////////////////////////////////////////////////////////////////////

///
class PEGASUS_COMMON_LINKAGE CIMConstQualifier
{
public:

    ///
    CIMConstQualifier();

    ///
    CIMConstQualifier(const CIMConstQualifier& x);

    ///
    CIMConstQualifier(const CIMQualifier& x);

    ///
    CIMConstQualifier(
	const CIMName& name, 
	const CIMValue& value, 
	const CIMFlavor & flavor = CIMFlavor (CIMFlavor::NONE),
	Boolean propagated = false);

    ///
    ~CIMConstQualifier();

    ///
    CIMConstQualifier& operator=(const CIMConstQualifier& x);

    ///
    CIMConstQualifier& operator=(const CIMQualifier& x);

    ///
    const CIMName& getName() const;

    ///
    CIMType getType() const;

    ///
    Boolean isArray() const;

    ///
    const CIMValue& getValue() const;

    ///
    const CIMFlavor & getFlavor() const;

    ///
    const Uint32 getPropagated() const;

    ///
    Boolean isUninitialized() const;

    ///
    Boolean identical(const CIMConstQualifier& x) const;

    ///
    CIMQualifier clone() const;

private:

    void _checkRep() const;

    CIMQualifierRep* _rep;

    friend class CIMQualifier;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMQualifier
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Qualifier_h */
