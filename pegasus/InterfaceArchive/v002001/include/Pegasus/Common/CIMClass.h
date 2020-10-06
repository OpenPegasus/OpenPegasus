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

#ifndef Pegasus_CIMClass_h
#define Pegasus_CIMClass_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMMethod.h>

PEGASUS_NAMESPACE_BEGIN

class CIMConstClass;
class CIMClassRep;
class Resolver;


/** The CIMClass class is used to represent CIM classes in Pegasus.  
*/
class PEGASUS_COMMON_LINKAGE CIMClass
{
public:

    /** Creates an uninitialized new CIM object representing a CIM class. 
        The class object created by this constructor can only be used 
        in an operation such as the copy constructor. It cannot be used 
        to create a class by appending names, properties, etc. since it 
        is uninitialized.

	Use one of the other constructors to create an initialized new 
        CIM class object.
        @exception UninitializedObjectException if an attempt is made to
        use the unitialized object.
    */
    CIMClass();

    /** Creates a new CIM object representing a CIMClass from the given
        CIMClass object.
	@param x  CIMClass Object from which to create the CIMClass
    */
    CIMClass(const CIMClass& x);

    /**	Creates a new CIMClass object representing a CIMClass from the given 
        CIMObject Object.
	@param x  CIMObject object from which to create the CIMClass object.
	@exception DynamicCastFailedException If a CIMClass can not be
        created from the given CIMObject.
    */
    PEGASUS_EXPLICIT CIMClass(const CIMObject& x);

    /**	Creates a new CIM object representing a CIMClass from inputs 
        of a classname and SuperClassName.
	@param className  CIMName representing name of the class being created.
	@param superClassName CIMName representing name of the SuperClass.
	<pre>
	    CIMClass NewClass("MyClass", "YourClass");
	</pre>
    */
    CIMClass(
	const CIMName& className,
	const CIMName& superClassName = CIMName());

    ///
    CIMClass& operator=(const CIMClass& x);

    /// Destructor
    ~CIMClass();

    /** Identifies whether or not this CIM class is an association. An 
        association is a relationship between two (or more) classes or 
        instances of two classes. The properties of an association class 
        include pointers, or references, to the two (or more) instances. 
        All CIM classes can be included in one or more associations.
	@return  true if this CIM class is an association, false
	otherwise.
    */
    Boolean isAssociation() const;

    /** Tests if the CIMClass is abstract.
	@return true if the CIMClass Object is abstract, false otherwise.
    */
    Boolean isAbstract() const;

    /** Gets the name of the class represented by this CIM object.
	@return CIMName with the class name.
    */
    const CIMName& getClassName() const;
        
    ///
    const CIMObjectPath& getPath() const;

    /** Sets the object path for the class.
        @param  path  CIMObjectPath containing the object path.
    */
    void setPath (const CIMObjectPath & path);

    /** Gets the name of the Parent class of this CIM Object.
	@return CIMName with parent class name.
    */
    const CIMName& getSuperClassName() const;

    /**	Sets the name of the parent class from the input 
        parameter.
	@param superClassName - CIMName defining parent class name.
    */
    void setSuperClassName(const CIMName& superClassName);

    /** Adds the specified qualifier to the class and increments the 
        qualifier count. It is illegal to add the same qualifier more 
        than one time.
	@param qualifier  CIMQualifier object representing the qualifier 
        to be added.
	@return the CIMClass object.
	@exception AlreadyExistsException if the qualifier already exists.
    */
    CIMClass& addQualifier(const CIMQualifier& qualifier);

    /**	Searches for a qualifier with the specified input name if it 
        exists in the class.
	@param name  CIMName of the qualifier to be found 
	@return Index of the qualifier found or PEG_NOT_FOUND
	if not found.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /**	Gets the CIMQualifier object defined by the input parameter.
	@param index  Index of the qualifier in the class from the
	findQualifier method.
	@return CIMQualifier object representing the qualifier found. 
        On error, CIMQualifier object will be null.
        @exception IndexOutOfBoundsException exception if the index is
        outside the range of parameters available from the CIMClass.
    */
    CIMQualifier getQualifier(Uint32 index);

    /** Gets the qualifier defined by the input parameter from the 
        qualifier list for this CIMClass.
	@param index  Index of the qualifier in the CIM class.
	@return CIMConstQualifier object representing the qualifier 
        found. On error, CIMConstQualifier object will be null.
        @exception IndexOutOfBoundsException exception if the index is
        outside the range of parameters available from the CIMClass.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /** Removes the qualifier defined by the index parameter.
        @param index  Defines the index of the qualifier to be removed.
        @exception IndexOutOfBoundsException if the index is outside
        the range of existing qualifier objects for this class.
    */
    void removeQualifier(Uint32 index);

    /** Gets the count of the number of qualifiers defined in the class.
	@return the number of qualifiers in the class definition (not
	those on properties or methods)
    */
    Uint32 getQualifierCount() const;

    /**	Adds the specified property object to the properties in 
        the CIM class
        @param x  CIMProperty to be added to the CIM Class.
        @return CIMClass object after the specified property is added.
    */
    CIMClass& addProperty(const CIMProperty& x);

    /** Finds the property object with the name defined by the 
        input parameter in the class.
	@param name  CIMName with the property name to be found.
	@return Index of the property object found or
	PEG_NOT_FOUND if the property is not found.
    */
    Uint32 findProperty(const CIMName& name) const;

    /** Gets a property object from the CIMClass.
	@param index  Index for the property object to get.
	@return CIMProperty object requested.
        @exception IndexOutOfBoundsException if the index is outside
        the range of properties in this class.
    */
    CIMProperty getProperty(Uint32 index);

    /** Gets a property object from the CIMClass.
	@param index  Index for the property object to get.
	@return CIMProperty object requested.
        @exception IndexOutOfBoundsException if the index is outside
        the range of properties in this class.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /** Removes the specified property from the class.
	@param index  Index of the property to be removed.
        @exception IndexOutOfBoundsException if the index is outside
        the range of properties in this class.
    */
    void removeProperty(Uint32 index);

    /** Gets the count of the number of properties defined in the class.
	@return count of number of properties in the class.
    */
    Uint32 getPropertyCount() const;

    /** Adds the method object defined by the input parameter to the 
        class and increments the count of the number of methods in the class.
	@param name  CIMMethod object representing the method to be added.
	@return the CIMClass object to which the method was added.
	@exception AlreadyExistsException if the method already exists.
	@exception UninitializedObjectException if the object is not initialized.
    */
    CIMClass& addMethod(const CIMMethod& x);

    /** Locate the method object defined by the input parameter.
	@param name  CIMName representing the name of the method to be found.
	@return Position of the method object in the class.
    */
    Uint32 findMethod(const CIMName& name) const;

    /** Gets the method object defined by the input parameter.
	@param index  Index to the method object to get.
	@return CIMMethod object of the method requested.
        @exception IndexOutOfBoundsException if the index is outside
        the range of methods in this class.
    */
    CIMMethod getMethod(Uint32 index);

    /** Gets the method object defined by the input parameter.
	@param index  Index to the method object to get.
	@return CIMConstMethod object of the method requested.
        @exception IndexOutOfBoundsException if the index is outside
        the range of methods in this class.
    */
    CIMConstMethod getMethod(Uint32 index) const;

    /** Removes the method defined by the index parameter.
        @param index  Defines the index of the method to be removed.
        @exception IndexOutOfBoundsException if the index is outside
        the range of methods in this class.
    */
    void removeMethod(Uint32 index);

    /** Gets the count of the number of methods in the class.
	@return the count of the number of methods in the class object.
    */
    Uint32 getMethodCount() const;

    /// Get names of all keys of this class.
    void getKeyNames(Array<CIMName>& keyNames) const;

    /** Determines if the object has keys.
        @return  true if the object has keys, false otherwise.
    */
    Boolean hasKeys() const;

    /** Makes a deep copy (clone) of the given object. 
        @return copy of the CIMClass object.
    */
    CIMClass clone() const;

    /** Compares with another CIM class.
	@param x  Class object for the class to be compared.
	@return true if the classes are identical, false otherwise.
    */
    Boolean identical(const CIMConstClass& x) const;

    /** Determines if the object has not been initialized.
        @return  true if the object has not been initialized, 
        false otherwise.
    */
    Boolean isUninitialized() const;

private:

    CIMClassRep* _rep;

    CIMClass(CIMClassRep* rep);

    void _checkRep() const;

    friend class CIMConstClass;
    friend class CIMObject;
    friend class CIMConstObject;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMClass
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

/// CIMConstClass
class PEGASUS_COMMON_LINKAGE CIMConstClass
{
public:

    ///
    CIMConstClass();

    ///
    CIMConstClass(const CIMConstClass& x);

    ///
    CIMConstClass(const CIMClass& x);

    ///
    PEGASUS_EXPLICIT CIMConstClass(const CIMObject& x);

    ///
    PEGASUS_EXPLICIT CIMConstClass(const CIMConstObject& x);

    ///
    CIMConstClass(
	const CIMName& className,
	const CIMName& superClassName = CIMName());

    ///
    CIMConstClass& operator=(const CIMConstClass& x);

    ///
    CIMConstClass& operator=(const CIMClass& x);

    ///
    ~CIMConstClass();

    ///
    Boolean isAssociation() const;

    ///
    Boolean isAbstract() const;

    ///
    const CIMName& getClassName() const;

    ///
    const CIMObjectPath& getPath() const;

    ///
    const CIMName& getSuperClassName() const;

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
    Uint32 findMethod(const CIMName& name) const;

    ///
    CIMConstMethod getMethod(Uint32 index) const;

    ///
    Uint32 getMethodCount() const;

    ///
    void getKeyNames(Array<CIMName>& keyNames) const;

    ///
    Boolean hasKeys() const;

    ///
    CIMClass clone() const;

    ///
    Boolean identical(const CIMConstClass& x) const;

    ///
    Boolean isUninitialized() const;

private:

    CIMClassRep* _rep;

    void _checkRep() const;

    friend class CIMClassRep;
    friend class CIMClass;
    friend class CIMInstanceRep;
    friend class CIMObject;
    friend class CIMConstObject;
    friend class XmlWriter;
    friend class MofWriter;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMClass_h */
