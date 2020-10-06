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

#ifndef Pegasus_Object_h
#define Pegasus_Object_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMQualifier.h>

PEGASUS_NAMESPACE_BEGIN

class CIMConstObject;
class CIMObjectRep;
class CIMClass;
class CIMConstClass;
class CIMInstance;
class CIMConstInstance;

////////////////////////////////////////////////////////////////////////////////
//
// CIMObject
//
////////////////////////////////////////////////////////////////////////////////

/** The CIMObject class is the superclass for the CIMInstance and 
    CIMClass classes.

    The CIMObjectRep data member points to either a CIMInstanceRep or
    CIMClassRep.
*/
class PEGASUS_COMMON_LINKAGE CIMObject
{
public:

    /** Constructor.
    */
    CIMObject();

    /** Copy constructor.
    */
    CIMObject(const CIMObject& x);

    /** Construction from CIMClass.
    */
    CIMObject(const CIMClass& x);

    /** Construction from CIMInstance.
    */
    CIMObject(const CIMInstance& x);

    /** Assignment operator.
    */
    CIMObject& operator=(const CIMObject& x);

    /** Destructor.
    */
    ~CIMObject();

    /**	Gets the ClassName of the object.
	@return ClassName of the object in a CIMName parameter.
	<pre>
	    CIMName className;
	    CIMClass myclass("myclass", "superclass");
	    className = myclass.getClassName;
	</pre>
    */
    const CIMName& getClassName() const;

    ///
    const CIMObjectPath& getPath() const;

    /** Sets the object path for the object.
        @param  path -  CIMObjectPath containing the object path.
    */
    void setPath (const CIMObjectPath & path);

    /**	Adds the CIMQualifier object to the instance.
	@param qualifier - CIMQualifier object to add to instance.
	@return the resulting CIMObject.
	@exception AlreadyExistsException if the CIMQualifier already 
        exists in the instance.
    */
    CIMObject& addQualifier(const CIMQualifier& qualifier);

    /**	Searches the instance for the qualifier object
        defined by the input parameter.
	@param name - CIMName defining the qualifier object to be found.
	@return  Position of the qualifier to be used in subsequent
	operations or PEG_NOT_FOUND if the qualifier is not found.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /**	Retrieves the qualifier object defined by the input parameter. 
	@param  index - Index for the qualifier object. The index to 
        qualifier objects is zero-origin and continuous so that 
        incrementing loops can be used to get all qualifier
	objects in a CIMInstance.
	@return CIMQualifier object defined by the index.
	@exception IndexOutOfBoundsException exception if the index
	is out of bounds.
    */
    CIMQualifier getQualifier(Uint32 index);

    /** Retrieves the qualifier object defined by the input parameter.  
        @param index - Index for the qualifier object. The index to 
        qualifier objects is zero-origin and continuous so that 
        incrementing loops can be used to get all qualifier
	objects in a CIMInstance.
	@return CIMConstQualifier object defined by the index.
	@exception IndexOutOfBoundsException exception if the index
	is out of bounds.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    ///
    void removeQualifier(Uint32 index);
	
    /** Gets the number of CIMQualifier objects defined for 
        this CIMObject.
	@return	Count of the number of CIMQualifier objects in the
	CIMObject.
    */
    Uint32 getQualifierCount() const;

    /**	Adds a property object defined by the input parameter to 
        the CIMObject.
	@param x - Property Object to be added. See the CIM Property
	class for definition of the property object.
	@return the resulting CIMObject.
	@exception AlreadyExistsException if the property already exists.
    */
    CIMObject& addProperty(const CIMProperty& x);

    /**	Searches the CIMProperty objects in the
	CIMObject for a property object with the name defined by the
	input parameter.
	@param name - CIMName with the name of the property object to be found.
	@return Position in the CIM object of the property object if found or
	PEG_NOT_FOUND if no property object found with the name defined by the
	input parameter.
    */
    Uint32 findProperty(const CIMName& name) const;

    /**	Gets the CIMProperty object in the CIMObject defined
	by the input parameter.
	@param index - Index to the property object in the CIMObject.
    	The index to qualifier objects is zero-origin and continuous
	so that incrementing loops can be used to get all qualifier
	objects in a CIMObject.
	@return CIMProperty object corresponding to the index.
	@exception IndexOutOfBoundsException exception if the index
	is out of bounds.
    */
    CIMProperty getProperty(Uint32 index);

    /**	Gets the CIMproperty object in the CIMObject defined
        by the input parameter.
        @param index - Index to the property object in the CIMObject.
        @return CIMProperty object corresponding to the index.
        @exception IndexOutOfBoundsException if the index is outside the
        range of properties in this object.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /** Remove the specified property from the instance.
	@param index - Index to the property to be removed from the
	instance.  Normally this is obtained by findProperty();
	@exception IndexOutOfBoundsException if the index is outside the
        range of properties in this object.
    */
    void removeProperty(Uint32 index);

    /**	Gets the number of CIMProperty objects defined for this CIMObject.
	@return	Count of the number of CIMProperty objects in the
	CIMObject. Zero indicates that no CIMProperty objects
	are contained in the CIMObject.
    */
    Uint32 getPropertyCount() const;

    /** Makes a deep copy (clone) of the given object.
        @return copy of the CIMObject.
    */
    CIMObject clone() const;

    /** Compares with another CIM Object.
        @param x - CIM object for comparision.
        @return true if the objects are identical, false otherwise.
    */
    Boolean identical(const CIMConstObject& x) const;

    /** Determines if the object has not been initialized.
        @return  true if the object has not been initialized,
                 false otherwise.
     */
    Boolean isUninitialized() const;

    /** Determines if the object represents a CIMClass.
        @return  true if the object represents a CIMClass,
                 false otherwise.
     */
    Boolean isClass () const;

    /** Determines if the object represents a CIMInstance.
        @return  true if the object represents a CIMInstance,
                 false otherwise
     */
    Boolean isInstance () const;

private:

    CIMObjectRep* _rep;

    CIMObject(CIMObjectRep* rep);

    void _checkRep() const;

    friend class CIMConstObject;
    friend class CIMClass;
    friend class CIMConstClass;
    friend class CIMInstance;
    friend class CIMConstInstance;
};

#define PEGASUS_ARRAY_T CIMObject
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstObject
//
////////////////////////////////////////////////////////////////////////////////

///
class PEGASUS_COMMON_LINKAGE CIMConstObject
{
public:

    ///
    CIMConstObject();

    ///
    CIMConstObject(const CIMConstObject& x);

    ///
    CIMConstObject(const CIMObject& x);

    /** Constructs a CIMObject instance from a CIMClass instance.
    */
    CIMConstObject(const CIMClass& x);

    /** Constructs a CIMObject instance from a CIMInstance instance.
    */
    CIMConstObject(const CIMInstance& x);

    /** Constructs a CIMObject instance from a CIMConstClass instance.
    */
    CIMConstObject(const CIMConstClass& x);

    /** Constructs a CIMObject instance from a CIMConstInstance instance.
    */
    CIMConstObject(const CIMConstInstance& x);

    ///
    CIMConstObject& operator=(const CIMConstObject& x);

    ///
    ~CIMConstObject();

    ///
    const CIMName& getClassName() const;

    ///
    const CIMObjectPath& getPath() const;

    ///
    Uint32 findQualifier(const CIMName& name) const;

    ///
    CIMConstQualifier getQualifier(Uint32 index) const;

    ///
    Uint32 getQualifierCount() const;

    ///
    Uint32 findProperty(const CIMName& name) const;

    ///
    CIMConstProperty getProperty(Uint32 index) const;

    ///
    Uint32 getPropertyCount() const;

    ///
    CIMObject clone() const;

    ///
    Boolean identical(const CIMConstObject& x) const;

    ///
    Boolean isUninitialized() const;

    ///
    Boolean isClass() const;

    ///
    Boolean isInstance() const;

private:

    CIMObjectRep* _rep;

    void _checkRep() const;

    friend class CIMObject;
    friend class CIMClass;
    friend class CIMConstClass;
    friend class CIMInstance;
    friend class CIMConstInstance;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Object_h */
