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

#ifndef Pegasus_Method_h
#define Pegasus_Method_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMParameter.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMType.h>

PEGASUS_NAMESPACE_BEGIN

class Resolver;
class CIMConstMethod;
class CIMMethodRep;

/** The CIMMethod class is used to represent CIM methods in Pegasus.
*/
class PEGASUS_COMMON_LINKAGE CIMMethod
{
public:

    /** Creates a new CIMMethod object. */
    CIMMethod();

    /** Creates a new CIMMethod object from another method instance.
	@param x - CIMMethod instance from which to create CIMMethod object.
    */
    CIMMethod(const CIMMethod& x);

    /**	Creates a CIMMethod with the specified name, type, and classOrigin.
	@param name - CIMName representing the name for the method.
	@param type - CIMType representing data type of method to create.
	@param classOrigin - CIMName representing the class origin.
	@param propagated - Flag indicating whether the definition of the 
        CIM Method is local to the CIM Class (respectively, Instance) in which 
        it appears, or was propagated without modification from the underlying 
        Subclass (respectively, Class).
    */
    CIMMethod(
	const CIMName& name,
	CIMType type,
	const CIMName& classOrigin = CIMName(),
	Boolean propagated = false);

    /** Desctructor. */
    ~CIMMethod();

    /** Assignment operator */
    CIMMethod& operator=(const CIMMethod& x);

    /** Gets the name of the method.
	@return CIMName with the name of the method.
    */
    const CIMName& getName() const;

    /** Sets the method name.
	@param name - CIMName with method name.
    */
    void setName(const CIMName& name);

    /** Gets the method type.
	@return The CIMType containing the method type for this method.
    */
    CIMType getType() const;

    /** Sets the method type to the specified CIM method type 
        as defined in CIMType.
    */
    void setType(CIMType type);

    /** Gets the class in which this method was defined.
	@return CIMName containing the classOrigin field. 
    */
    const CIMName& getClassOrigin() const;

    /** Sets the ClassOrigin attribute with the classname defined on 
        the input parameter.
	@param classOrigin - CIMName parameter defining the name
	of the class origin.
    */
    void setClassOrigin(const CIMName& classOrigin);

    /** Tests the propagated qualifier.
        @return true if method is propagated, false otherwise.
    */
    Boolean getPropagated() const;

    /** Sets the Propagaged Qualifier 
        @param propagated - Flag indicating propagation.
    */
    void setPropagated(Boolean propagated);

    /** Adds the specified qualifier to the method and increments the
        qualifier count. 
        @param x - CIMQualifier object representing the qualifier
        to be added.
        @return the CIMMethod object after adding the specified qualifier.
        @exception AlreadyExistsException if the qualifier already exists.
    */
    CIMMethod& addQualifier(const CIMQualifier& x);

    /** Searches for a qualifier with the specified input name.
        @param name - CIMName of the qualifier to be found.
        @return Index of the qualifier found or PEG_NOT_FOUND
        if not found.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /** Gets the CIMQualifier defined by the input parameter.
	@param index - Index of the qualifier requested.
	@return CIMQualifier object representing the qualifier found.
	@exception IndexOutOfBoundsException exception if the index is
        outside the range of parameters available from the CIMMethod.
    */
    CIMQualifier getQualifier(Uint32 index);

    /** Gets the CIMQualifier defined by the input parameter.
	@param index - Index of the qualifier requested.
	@return CIMConstQualifier object representing the qualifier found.
	@exception IndexOutOfBoundsException exception if the index is
        outside the range of parameters available from the CIMMethod.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /** Removes the specified CIMQualifier from this method.
	@param index - Index of the qualifier to remove.
	@exception IndexOutOfBoundsException exception if the index is
        outside the range of parameters available from the CIMMethod.
    */
    void removeQualifier(Uint32 index);

    /** Returns the number of Qualifiers attached to this CIMMethod object.
        @return the number of qualifiers in the CIM Method.
    */
    Uint32 getQualifierCount() const;

    /** Adds the parameter defined by the input to the CIMMethod.
        @param x - CIMParameter to be added to the CIM Method.
        @return CIMMethod object after the specified parameter is added.
    */
    CIMMethod& addParameter(const CIMParameter& x);

    /** Finds the parameter with the specified name.
	@param name - Name of parameter to be found.
        @return Index of the parameter object found or PEG_NOT_FOUND 
        if the property is not found.
    */
    Uint32 findParameter(const CIMName& name) const;

    /** Gets the parameter defined by the specified index.
	@param index - Index for the parameter to be returned.
	@return CIMParameter object requested.
	@exception IndexOutOfBoundsException if the index is outside 
        the range of available parameters
    */
    CIMParameter getParameter(Uint32 index);

    /** Gets the parameter defined for the specified index.
	@param index - Index for the parameter to be returned.
	@return CIMConstParameter object requested.
	@exception IndexOutOfBoundsException if the index is outside 
        the range of available parameters
    */
    CIMConstParameter getParameter(Uint32 index) const;

    /** Removes the CIMParameter defined by the specified index.
	@param index - Index of the parameter to be removed.
	@exception IndexOutOfBoundsException if the index is outside the
        range of parameters available from the CIMMethod.
    */
    void removeParameter (Uint32 index);

    /** Gets the count of Parameters defined in the CIMMethod.
	@return - count of the number of parameters attached to the CIMMethod.
    */
    Uint32 getParameterCount() const;

    /** Determines if the object has not been initialized.
        @return  true if the object has not been initialized,
                 false otherwise
     */
    Boolean isUninitialized() const;

    /** Compares with another CIMConstMethod.
        @param x - CIMConstMethod object for the method to be compared.
        @return true if this method is identical to the one specified.
    */
    Boolean identical(const CIMConstMethod& x) const;

    /** Makes a deep copy (clone) of this method.
        @return copy of the CIMMethod object.
    */
    CIMMethod clone() const;

private:

    CIMMethod(CIMMethodRep* rep);

    PEGASUS_EXPLICIT CIMMethod(const CIMConstMethod& x);

    void _checkRep() const;

    CIMMethodRep* _rep;
    friend class CIMConstMethod;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
};

///
class PEGASUS_COMMON_LINKAGE CIMConstMethod
{
public:

    ///
    CIMConstMethod();

    ///
    CIMConstMethod(const CIMConstMethod& x);

    ///
    CIMConstMethod(const CIMMethod& x);

    ///
    CIMConstMethod(
	const CIMName& name,
	CIMType type,
	const CIMName& classOrigin = CIMName(),
	Boolean propagated = false);

    ///
    ~CIMConstMethod();

    ///
    CIMConstMethod& operator=(const CIMConstMethod& x);

    ///
    CIMConstMethod& operator=(const CIMMethod& x);

    ///
    const CIMName& getName() const;

    ///
    CIMType getType() const;

    ///
    const CIMName& getClassOrigin() const;

    ///
    Boolean getPropagated() const;

    ///
    Uint32 findQualifier(const CIMName& name) const;

    ///
    CIMConstQualifier getQualifier(Uint32 index) const;

    ///
    Uint32 getQualifierCount() const;

    ///
    Uint32 findParameter(const CIMName& name) const;

    ///
    CIMConstParameter getParameter(Uint32 index) const;

    ///
    Uint32 getParameterCount() const;

    ///
    Boolean isUninitialized() const;

    ///
    Boolean identical(const CIMConstMethod& x) const;

    ///
    CIMMethod clone() const;

private:

    void _checkRep() const;

    CIMMethodRep* _rep;

    friend class CIMMethod;
    friend class CIMMethodRep;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMMethod
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Method_h */
