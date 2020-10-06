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

#ifndef Pegasus_Instance_h
#define Pegasus_Instance_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMPropertyList.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMInstance
//
////////////////////////////////////////////////////////////////////////////////

class CIMConstInstance;
class CIMInstanceRep;
class Resolver;

/** This class represents the instance of a CIM class. It is used manipulate
    instances and their members.
*/
class PEGASUS_COMMON_LINKAGE CIMInstance
{
public:

    /** Creates a CIMInstance object.
    */
    CIMInstance();

    /** Creates a CIMInstance object from another CIMInstance object.
	@param x - CIMInstance object from which the new instance is created.
    */
    CIMInstance(const CIMInstance& x);

    /**	Creates a CIMInstance object from the given CIMObject.
	@param x - CIMObject from which to create the CIMInstance.
	@exception DynamicCastFailedException If a CIMInstance can not be
        created from the given CIMObject.
    */
    PEGASUS_EXPLICIT CIMInstance(const CIMObject& x);

    /**	Creates a CIMInstance object of the class specified by
	the input parameter.
	@param className - CIMName to be used with new instance object.
    */
    CIMInstance(const CIMName& className);

    /** Copy Constructor. */
    CIMInstance& operator=(const CIMInstance& x);

    /** Destructor. */
    ~CIMInstance();

    /**	Gets the class name of the instance.
	@return CIMName with the class name.
    */
    const CIMName& getClassName() const;

    ///
    const CIMObjectPath& getPath() const;

    /** Sets the object path for the instance.
        @param  path - CIMObjectPath containing the object path.
    */
    void setPath (const CIMObjectPath & path);

    /**	Adds the CIMQualifier object to the instance.
	@param qualifier - CIMQualifier object to add to instance.
	@return the resulting CIMInstance.
	@exception AlreadyExistsException if the CIMQualifier 
        already exists in the instance.
    */
    CIMInstance& addQualifier(const CIMQualifier& qualifier);

    /**	Searches the instance for the qualifier object defined by 
        the input parameter.
	@param name - CIMName defining the qualifier object to be found.
	@return Position of the qualifier to be used in subsequent
	operations or PEG_NOT_FOUND if the qualifier is not found.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /**	Retrieves the qualifier object defined by the input parameter.  
	The index to qualifier objects is zero-origin and continuous
	so that incrementing loops can be used to get all qualifier
	objects in a CIMInstance.
        @param index - Index for the qualifier object.
	@return Qualifier object defined by index.
	@exception IndexOutOfBoundsException exception if the index
	is out of bounds.
    */
    CIMQualifier getQualifier(Uint32 index);

    /**	Retrieves the qualifier object defined by the input parameter.  
	The index to qualifier objects is zero-origin and continuous
	so that incrementing loops can be used to get all qualifier
	objects in a CIMInstance.
        @param index - Index for the qualifier object.
	@return Qualifier object defined by index.
	@exception IndexOutOfBoundsException exception if the index
	is out of bounds.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /** Removes the qualifier defined by the index parameter.
        @param index Defines the index of the qualifier to be removed.
        @exception IndexOutOfBoundsException if the index is outside
        the range of existing qualifier objects for this instance.
    */
    void removeQualifier(Uint32 index);

    /**	Gets the count of the CIMQualifier objects defined
        for this CIMInstance.
	@return	Count of the number of CIMQalifier objects in the
	CIMInstance.
    */
    Uint32 getQualifierCount() const;

    /**	Adds a property object defined by the input parameter to 
        the CIMInstance.
	@param x - Property Object to be added. See the CIMProperty
	class for definition of the property object.
	@return the resulting CIMInstance.
	@exception AlreadyExistsException if the property already exists.
    */
    CIMInstance& addProperty(const CIMProperty& x);

    /**	Searches the CIMProperty objects in the CIMInstance for 
        property object with the name defined by the input parameter.
	@param name - CIMName with the name of the property object to be found.
	@return Position in the CIM Instance to the property object if found or
	PEG_NOT_FOUND if no property object found with the name defined by the
	input parameter.
    */
    Uint32 findProperty(const CIMName& name) const;

    /**	Gets the CIMProperty object in the CIMInstance defined
	by the input parameter.
    	The index to qualifier objects is zero-origin and continuous
	so that incrementing loops can be used to get all qualifier
	objects in a CIMInstance.
	@param index - Index to the property object in the CIMInstance.
	@return CIMProperty object corresponding to the index.
	@exception IndexOutOfBoundsException if index is outside the range of
        properties in this instance.
    */
    CIMProperty getProperty(Uint32 index);

    /**	Gets the CIMproperty object in the CIMInstance defined
	by the input parameter.
    	The index to qualifier objects is zero-origin and continuous
	so that incrementing loops can be used to get all qualifier
	objects in a CIMInstance.
	@param index - Index to the property object in the CIMInstance.
	@return CIMProperty object corresponding to the index.
	@exception IndexOutOfBoundsException if index is outside the range of
        properties in this instance.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /** Removes the property defined by the input parameter 
        from the instance.
	@param index - Index to the property to be removed from the
	instance.  Normally this is obtained by getProperty().
	@exception IndexOutOfBoundsException if index is outside the range of
        properties in this instance.
    */
    void removeProperty(Uint32 index);

    /**	Gets the count of CIMProperty objects defined for 
        this CIMInstance.
	@return	Count of the number of CIMProperty objects in the
	CIMInstance. Zero indicates that no CIMProperty objects
	are contained in the CIMInstance.
    */
    Uint32 getPropertyCount() const;

    /** Builds the CIM object path for this instance. The class
	argument is used to determine which fields are keys. The instance
	name has this form:

	<PRE>
	    ClassName.key1=value1,...,keyN=valueN
	</PRE>

	The object path is in standard form (the class name and key name
	are all lowercase; the key-value pairs appear in sorted order by
	key name).

        Note that the path attribute of the CIMInstanceRep object is not 
        modified.
    */
    CIMObjectPath buildPath(const CIMConstClass& cimClass) const;

    /** Makes a deep copy (clone) of the CIMInstance object. */
    CIMInstance clone() const;

    /**	Compares the CIMInstance with another CIMInstance
	defined by the input parameter for equality of all components.
	@param x - CIMInstance to be compared.
	@return true if they are identical, false otherwise.
    */
    Boolean identical(const CIMConstInstance& x) const;

    /** Determines if the object has not been initialized.
        @return  true if the object has not been initialized,
                 false otherwise.
     */
    Boolean isUninitialized() const;

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /**  <I><B>Experimental Interface</B></I><BR>
    Filter the properties, qualifiers and class origin attributes from this 
    instance based on filtering criteria defined in the input parameters.  
    Note that this function does not add anything that was not in the instance 
    at the beginning of the call.  This function does NOT clone the instance 
    but modifies the existing instance.  The function was defined specifically 
    for providers to allow creating instances for a specific instance 
    operation response corresponding to the parameters provided with the 
    operation call (includeQualifiers, etc.) from a more general instance 
    template.  
    
    @param includeQualifiers If false, qualifiers are removed from the 
    instance and any properties included in the instance; otherwise no 
    qualifiers are removed.  Because there is still confusion over the exact 
    operation of this parameter in the CIM specifications and the concept of 
    instance level qualifiers, the behavior of this function when the 
    parameter is true MAY change in the future to match any future 
    clarifications of interoperable behavior in the CIM specifications.  
    
    @param includeClassOrigin If false, ClassOrigin attributes are removed from
    all properties.  Otherwise, ClassOrigin attributes are not filtered.
    
    @param propertyList This CIMPropertyList defines the list of properties that should be on the
    instance after completion of the call. If not NULL, properties absent from this list will
    be removed from the list. If NULL, no properties will be removed from the instance.
    If empty, all properties will be removed from the instance.
    
    @return The CIMInstance with properties and qualifiers from this
    instance based on the filtering criteria. 
    <p><b>Example:</b>
    <pre>
        CIMClass myClass .. a defined and complete CIMClass.
        // create instance with qualifiers, class origin and all properties
        CIMInstance myInstance =
                myClass.buildInstance(true, true, CIMPropertyList());
        // filter qualifiers off of the instance.
                myInstance.filterInstance(false, true, CIMPropertyList());
    </pre>
    */ 

    void filter(Boolean includeQualifiers,
            Boolean includeClassOrigin,
            const CIMPropertyList & propertyList);
#endif

private:

    CIMInstanceRep* _rep;

    CIMInstance(CIMInstanceRep* rep);

    void _checkRep() const;

    friend class CIMConstInstance;
    friend class CIMObject;
    friend class CIMConstObject;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstInstance
//
////////////////////////////////////////////////////////////////////////////////

///
class PEGASUS_COMMON_LINKAGE CIMConstInstance
{
public:

    ///
    CIMConstInstance();

    ///
    CIMConstInstance(const CIMConstInstance& x);

    ///
    CIMConstInstance(const CIMInstance& x);

    ///
    PEGASUS_EXPLICIT CIMConstInstance(const CIMObject& x);

    ///
    PEGASUS_EXPLICIT CIMConstInstance(const CIMConstObject& x);

    ///
    CIMConstInstance(const CIMName& className);

    ///
    CIMConstInstance& operator=(const CIMConstInstance& x);

    ///
    CIMConstInstance& operator=(const CIMInstance& x);

    ///
    ~CIMConstInstance();

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
    CIMObjectPath buildPath(const CIMConstClass& cimClass) const;

    ///
    CIMInstance clone() const;

    ///
    Boolean identical(const CIMConstInstance& x) const;

    ///
    Boolean isUninitialized() const;

private:

    CIMInstanceRep* _rep;

    void _checkRep() const;

    friend class CIMInstance;
    friend class CIMObject;
    friend class CIMConstObject;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
};

#define PEGASUS_ARRAY_T CIMInstance
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Instance_h */
