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

#ifndef Pegasus_CIMClass_h
#define Pegasus_CIMClass_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMMethod.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMPropertyList.h>

PEGASUS_NAMESPACE_BEGIN

class CIMConstClass;
class CIMClassRep;
class Resolver;


/**
    The CIMClass class represents the DMTF standard CIM class definition.

    <p>The CIMClass class uses a shared representation model, such that
    multiple CIMClass objects may refer to the same data copy.  Assignment
    and copy operators create new references to the same data, not distinct
    copies.  An update to a CIMClass object affects all the CIMClass
    objects that refer to the same data copy.  The data remains valid until
    all the CIMClass objects that refer to it are destructed.  A separate
    copy of the data may be created using the clone method.
*/
class PEGASUS_COMMON_LINKAGE CIMClass
{
public:

    /**
        Constructs an uninitialized CIMClass object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMClass();

    /**
        Constructs a CIMClass object from the value of a specified
        CIMClass object, so that both objects refer to the same data copy.
        @param x The CIMClass object from which to construct a new
            CIMClass object.
    */
    CIMClass(const CIMClass& x);

    /**
        Constructs a CIMClass object from the value of a specified
        CIMObject object, so that both objects refer to the same data copy.
        @param x The CIMObject object from which to construct the
            CIMClass object.
        @exception DynamicCastFailedException If a CIMClass can not be
            created from the given CIMObject.
    */
    explicit CIMClass(const CIMObject& x);

    /**
        Constructs a CIMClass object with the specified attributes.
        <p><b>Example:</b>
        <pre>
            CIMClass NewClass("MyClass", "YourClass");
        </pre>

        @param className A CIMName specifying the name of the class.
        @param superClassName A CIMName specifying name of the parent class.
            (A null value indicates no superclass.)
    */
    CIMClass(
        const CIMName& className,
        const CIMName& superClassName = CIMName());

    /**
        Assigns the value of the specified CIMClass object to this object,
        so that both objects refer to the same data copy.
        @param x The CIMClass object from which to assign this CIMClass
            object.
        @return A reference to this CIMClass object.
    */
    CIMClass& operator=(const CIMClass& x);

    /**
        Destructs the CIMClass object.
    */
    ~CIMClass();

    /**
        Indicates whether this class is an association class.  An
        association is a relationship between two (or more) classes or
        instances.  The Association qualifier is used to make this
        determination.
        @return True if this class is an association class, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isAssociation() const;

    /**
        Indicates whether this class is an abstract class.
        The Abstract qualifier is used to make this determination.
        @return True if this class is an abstract class, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isAbstract() const;

    /**
        Gets the name of the class.
        @return A CIMName containing the class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassName() const;

    /**
        Gets the object path for the class.
        @return A CIMObjectPath containing the object path.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMObjectPath& getPath() const;

    /**
        Sets the object path for the class.
        @param path A CIMObjectPath containing the object path.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setPath (const CIMObjectPath & path);

    /**
        Gets the name of the parent class of this class.
        @return A CIMName containing parent class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getSuperClassName() const;

    /**
        Sets the name of the parent class.
        @param superClassName A CIMName specifying the parent class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setSuperClassName(const CIMName& superClassName);

    /**
        Adds a qualifier to the class.
        @param qualifier The CIMQualifier to be added.
        @return A reference to this CIMClass object.
        @exception AlreadyExistsException If a qualifier with the
            same name already exists in the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMClass& addQualifier(const CIMQualifier& qualifier);

    /**
        Finds a qualifier by name.
        @param name A CIMName specifying the name of the qualifier to be found.
        @return Index of the qualifier if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /**
        Gets the qualifier at the specified index.
        @param index The index of the qualifier to be retrieved.
        @return The CIMQualifier at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of qualifiers available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifier getQualifier(Uint32 index);

    /**
        Gets the qualifier at the specified index.
        @param index The index of the qualifier to be retrieved.
        @return The CIMConstQualifier at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of qualifiers available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Removes a qualifier from the class.
        @param index The index of the qualifier to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeQualifier(Uint32 index);

    /**
        Gets the number of qualifiers in the class.
        @return An integer count of the qualifiers in the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Adds a property to the class.
        @param x The CIMProperty to be added.
        @return A reference to this CIMClass object.
        @exception AlreadyExistsException If a property with the
            same name already exists in the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMClass& addProperty(const CIMProperty& x);

    /**
        Finds a property by name.
        @param name A CIMName specifying the name of the property to be found.
        @return Index of the property if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findProperty(const CIMName& name) const;

    /**
        Gets the property at the specified index.
        @param index The index of the property to be retrieved.
        @return The CIMProperty at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of properties available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMProperty getProperty(Uint32 index);

    /**
        Gets the property at the specified index.
        @param index The index of the property to be retrieved.
        @return The CIMConstProperty at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of properties available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /**
        Removes a property from the class.
        @param index The index of the property to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of properties available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeProperty(Uint32 index);

    /**
        Gets the number of properties in the class.
        @return An integer count of the properties in the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getPropertyCount() const;

    /**
        Adds a method to the class.
        @param x The CIMMethod to be added.
        @return A reference to this CIMClass object.
        @exception AlreadyExistsException If a method with the
            same name already exists in the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMClass& addMethod(const CIMMethod& x);

    /**
        Finds a method by name.
        @param name A CIMName specifying the name of the method to be found.
        @return Index of the method if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findMethod(const CIMName& name) const;

    /**
        Gets the method at the specified index.
        @param index The index of the method to be retrieved.
        @return The CIMMethod at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of methods available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMMethod getMethod(Uint32 index);

    /**
        Gets the method at the specified index.
        @param index The index of the method to be retrieved.
        @return The CIMConstMethod at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of methods available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstMethod getMethod(Uint32 index) const;

    /**
        Removes a method from the class.
        @param index The index of the method to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of methods available for the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeMethod(Uint32 index);

    /**
        Gets the number of methods in the class.
        @return An integer count of the methods in the CIMClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getMethodCount() const;

    /**
        Gets the list of key properties in this class.  The Key qualifier
        on CIMProperty objects is used to locate key properties.
        @return An Array of CIMName objects containing the names of the key
        properties.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void getKeyNames(Array<CIMName>& keyNames) const;

    /**
        Indicates whether this class contains key properties.  The Key
        qualifier on CIMProperty objects is used to make this determination.
        @return True if this class contains key properties, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean hasKeys() const;

    /**
        Makes a deep copy of the class.  This creates a new copy of all
        the class attributes including qualifiers, properties, and methods.
        @return A new copy of the CIMClass object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMClass clone() const;

    /**
        Compares the CIMClass with a specified CIMConstClass.
        @param x The CIMConstClass to be compared.
        @return True if this class is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean identical(const CIMConstClass& x) const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Builds a CIMInstance based on this CIMClass.  Properties in the
        instance are initialized to the default values (if any) specified
        in the class definition.  The method arguments determine whether
        qualifiers are included, the class origin attributes are included,
        and which properties are included in the new instance.  This method
        is designed specifically for providers to allow them to easily build
        instance objects using the parameters provided with the CIM instance
        operations such as getInstance and enumerateInstances.

        <p><b>Example:</b>
        <pre>
            CIMClass myClass .. a defined and complete CIMClass.
            // create instance with qualifiers, class origin and all properties
            CIMInstance myInstance =
                myClass.buildInstance(true, true, CIMPropertyList());
        </pre>

        @param includeQualifiers A Boolean indicating whether qualifiers in
        the class definition (and its properties) are to be added to the
        instance.  If false, no qualifiers are added to the instance or its
        properties.  The TOINSTANCE flavor is ignored.  Because the CIM
        specification is unclear on the meaning of this parameter and its
        relationship to instance operations, the behavior when this parameter
        is true MAY change in the future based on clarifications of the
        CIM specification.

        @param includeClassOrigin A Boolean indicating whether ClassOrigin
        attributes are to be transferred from the class object to the
        instance.

        @param propertyList A CIMPropertyList defining the properties that
        are to be added to the created instance.  If the propertyList is not
        NULL, properties defined in the class and in this propertyList are
        added to the new instance.  If the propertyList is NULL, all
        properties are added to the instance.  If the propertyList is empty,
        no properties are added.  Note that this function does NOT generate
        an error if a property name is supplied that is NOT in the class;
        it simply does not add that property to the instance.

        @return CIMInstance of this class appropriately initialized.

        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMInstance buildInstance(Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList & propertyList) const;

private:

    CIMClassRep* _rep;

    CIMClass(CIMClassRep* rep);

    friend class CIMConstClass;
    friend class CIMObject;
    friend class CIMConstObject;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
    friend class SCMOClass;
};

#define PEGASUS_ARRAY_T CIMClass
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T


/**
    The CIMConstClass class provides a const interface to a CIMClass
    object.  This class is needed because the shared representation model
    used by CIMClass does not prevent modification to a const CIMClass
    object.  Note that the value of a CIMConstClass object could still be
    modified by a CIMClass object that refers to the same data copy.
*/
class PEGASUS_COMMON_LINKAGE CIMConstClass
{
public:

    /**
        Constructs an uninitialized CIMConstClass object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMConstClass();

    /**
        Constructs a CIMConstClass object from the value of a specified
        CIMConstClass object, so that both objects refer to the same data copy.
        @param x The CIMConstClass object from which to construct a new
            CIMConstClass object.
    */
    CIMConstClass(const CIMConstClass& x);

    /**
        Constructs a CIMConstClass object from the value of a specified
        CIMClass object, so that both objects refer to the same data copy.
        @param x The CIMClass object from which to construct a new
            CIMConstClass object.
    */
    CIMConstClass(const CIMClass& x);

    /**
        Constructs a CIMConstClass object from the value of a specified
        CIMObject object, so that both objects refer to the same data copy.
        @param x The CIMObject object from which to construct the
            CIMConstClass object.
        @exception DynamicCastFailedException If a CIMConstClass can not be
            created from the given CIMObject.
    */
    explicit CIMConstClass(const CIMObject& x);

    /**
        Constructs a CIMConstClass object from the value of a specified
        CIMConstObject object, so that both objects refer to the same data
        copy.
        @param x The CIMConstObject object from which to construct the
            CIMConstClass object.
        @exception DynamicCastFailedException If a CIMConstClass can not be
            created from the given CIMConstObject.
    */
    explicit CIMConstClass(const CIMConstObject& x);

    /**
        Constructs a CIMConstClass object with the specified attributes.
        @param className A CIMName specifying the name of the class.
        @param superClassName A CIMName specifying name of the parent class.
            (A null value indicates no superclass.)
    */
    CIMConstClass(
        const CIMName& className,
        const CIMName& superClassName = CIMName());

    /**
        Assigns the value of the specified CIMConstClass object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMConstClass object from which to assign this
            CIMConstClass object.
        @return A reference to this CIMConstClass object.
    */
    CIMConstClass& operator=(const CIMConstClass& x);

    /**
        Assigns the value of the specified CIMClass object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMClass object from which to assign this
            CIMConstClass object.
        @return A reference to this CIMConstClass object.
    */
    CIMConstClass& operator=(const CIMClass& x);

    /**
        Destructs the CIMConstClass object.
    */
    ~CIMConstClass();

    /**
        Indicates whether this class is an association class.  An
        association is a relationship between two (or more) classes or
        instances.  The Association qualifier is used to make this
        determination.
        @return True if this class is an association class, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isAssociation() const;

    /**
        Indicates whether this class is an abstract class.
        The Abstract qualifier is used to make this determination.
        @return True if this class is an abstract class, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isAbstract() const;

    /**
        Gets the name of the class.
        @return A CIMName containing the class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassName() const;

    /**
        Gets the object path for the class.
        @return A CIMObjectPath containing the object path.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMObjectPath& getPath() const;

    /**
        Gets the name of the parent class of this class.
        @return A CIMName containing parent class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getSuperClassName() const;

    /**
        Finds a qualifier by name.
        @param name A CIMName specifying the name of the qualifier to be found.
        @return Index of the qualifier if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /**
        Gets the qualifier at the specified index.
        @param index The index of the qualifier to be retrieved.
        @return The CIMConstQualifier at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of qualifiers available for the CIMConstClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Gets the number of qualifiers in the class.
        @return An integer count of the qualifiers in the CIMConstClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Finds a property by name.
        @param name A CIMName specifying the name of the property to be found.
        @return Index of the property if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findProperty(const CIMName& name) const;

    /**
        Gets the property at the specified index.
        @param index The index of the property to be retrieved.
        @return The CIMConstProperty at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of properties available for the CIMConstClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /**
        Gets the number of properties in the class.
        @return An integer count of the properties in the CIMConstClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getPropertyCount() const;

    /**
        Finds a method by name.
        @param name A CIMName specifying the name of the method to be found.
        @return Index of the method if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findMethod(const CIMName& name) const;

    /**
        Gets the method at the specified index.
        @param index The index of the method to be retrieved.
        @return The CIMConstMethod at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of methods available for the CIMConstClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstMethod getMethod(Uint32 index) const;

    /**
        Gets the number of methods in the class.
        @return An integer count of the methods in the CIMConstClass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getMethodCount() const;

    /**
        Gets the list of key properties in this class.  The Key qualifier
        on CIMConstProperty objects is used to locate key properties.
        @return An Array of CIMName objects containing the names of the key
        properties.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void getKeyNames(Array<CIMName>& keyNames) const;

    /**
        Indicates whether this class contains key properties.  The Key
        qualifier on CIMConstProperty objects is used to make this
        determination.
        @return True if this class contains key properties, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean hasKeys() const;

    /**
        Makes a deep copy of the class.  This creates a new copy of all
        the class attributes including qualifiers, properties, and methods.
        @return A CIMClass object with a separate copy of the
            CIMConstClass object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMClass clone() const;

    /**
        Compares the CIMConstClass with a specified CIMConstClass.
        @param x The CIMConstClass to be compared.
        @return True if this class is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean identical(const CIMConstClass& x) const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

private:

    CIMClassRep* _rep;

    friend class CIMClassRep;
    friend class CIMClass;
    friend class CIMInstanceRep;
    friend class CIMObject;
    friend class CIMConstObject;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
    friend class SCMOClass;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMClass_h */
