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

#ifndef Pegasus_Property_h
#define Pegasus_Property_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMQualifier.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMProperty
//
////////////////////////////////////////////////////////////////////////////////

class CIMConstProperty;
class CIMPropertyRep;
class CIMClassRep;
class CIMInstanceRep;
class CIMQualifier;
class CIMConstQualifier;
class Resolver;

/**
    The CIMProperty class represents the DMTF standard CIM property definition.
    A CIMProperty is generally defined in the context of a CIMClass or
    CIMInstance.
    A CIMProperty consists of:
    <ul>
        <li>A CIMName containing the name of the property
        <li>A CIMType defining the property type
        <li>A Boolean indicating whether it is an Array property
        <li>A Uint32 indicating the size of the Array, if the property is an
            Array property
        <li>A CIMName containing the reference class name for this property,
            if the property is of reference type
        <li>A CIMValue specifying the property value
        <li>Zero or more CIMQualifier objects
    </ul>
    In addition, a CIMProperty has these internal attributes:
    <ul>
        <li><b>propagated</b> - An attribute defining whether this CIMProperty
            is propagated from a superclass.  Note that this is normally set
            as part of completing the definition of objects (resolving) when
            they are created as part of a CIM schema and is NOT automatically
            set when creating a property object.  It can only be logically set
            in context of the schema in which the CIMProperty is defined.
        <li><b>classOrigin</b> - An attribute defining the class in which
            this CIMProperty was originally defined.  This is normally set
            within the context of the schema in which the CIMProperty is
            defined.  This attribute is available from objects retrieved
            from the CIM Server, for example, and provides information on
            the defintion of this property in the class hierarchy.  The
            propagated and ClassOrigin attributes can be used together to
            determine if properties originated with this object or were
            inherited from higher levels of the hiearchy.
    </ul>

    <p>The CIMProperty class uses a shared representation model, such that
    multiple CIMProperty objects may refer to the same data copy.  Assignment
    and copy operators create new references to the same data, not distinct
    copies.  An update to a CIMProperty object affects all the CIMProperty
    objects that refer to the same data copy.  The data remains valid until
    all the CIMProperty objects that refer to it are destructed.  A separate
    copy of the data may be created using the clone method.
*/
class PEGASUS_COMMON_LINKAGE CIMProperty
{
public:

    /**
        Constructs an uninitialized CIMProperty object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMProperty();

    /**
        Constructs a CIMProperty object from the value of a specified
        CIMProperty object, so that both objects refer to the same data copy.
        @param x The CIMProperty object from which to construct a new
            CIMProperty object.
    */
    CIMProperty(const CIMProperty& x);

    /**
        Constructs a CIMProperty object with the specified attributes.
        @param name A CIMName specifying the name of the property.
        @param value A CIMValue specifying the property value, and implicitly
            defining the property type and whether the property is an Array
            property.
        @param arraySize A Uint32 indicating the size of the Array, if the
            property is an Array property.  The default value of zero
            indicates a variable size array.
        @param referenceClassName A CIMName containing the reference class
            name for this property, if the property is of reference type.
        @param classOrigin A CIMName indicating the class in which the
            property is locally defined (optional).
        @param propagated A Boolean indicating whether the property is local
            to the class or instance in which it appears or was propagated
            (without modification) from other schema.
        @exception TypeMismatchException If the property is of reference
            type and referenceClassName is null or if the property is not of
            reference type and referenceClassName is not null.
        @exception TypeMismatchException If the property is an Array property
            and arraySize is not zero.
        @exception UninitializedObjectException If the property name is null.
    */
    CIMProperty(
        const CIMName& name,
        const CIMValue& value,
        Uint32 arraySize = 0,
        const CIMName& referenceClassName = CIMName(),
        const CIMName& classOrigin = CIMName(),
        Boolean propagated = false);

    /**
        Destructs the CIMProperty object.
    */
    ~CIMProperty();

    /**
        Assigns the value of the specified CIMProperty object to this object,
        so that both objects refer to the same data copy.
        @param x The CIMProperty object from which to assign this CIMProperty
            object.
        @return A reference to this CIMProperty object.
    */
    CIMProperty& operator=(const CIMProperty& x);

    /**
        Gets the name of the property.
        @return A CIMName containing the name of the property.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getName() const;

    /**
        Sets the property name.
        @param name A CIMName containing the new name of the property.
        @exception UninitializedObjectException If the object is not
            initialized.
        @exception Exception If the object is already contained by
            CIMClass, CIMInstance or CIMObject
    */
    void setName(const CIMName& name);

    /**
        Gets the property value.
        @return A CIMValue containing the property value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMValue& getValue() const;

    /**
        Gets the property type.
        @return A CIMType containing the property type.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMType getType() const;

    /**
        Checks whether the property is an Array property.
        @return True if the property is an Array property, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isArray() const;

    /**
        Sets the property value.
        @param value A CIMValue containing the new value of the property.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setValue(const CIMValue& value);

    /**
        Gets the array size for the property.
        @return Uint32 array size.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getArraySize() const;

    /**
        Gets the reference class name for the property.
        @return A CIMName containing the reference class name for the
            property if the property is of reference type, a null CIMName
            otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getReferenceClassName() const;

    /**
        Gets the class in which this property is locally defined.  This
        information is normally available with properties that are part of
        schema returned from a CIM Server.
        @return CIMName containing the classOrigin attribute.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassOrigin() const;

    /**
        Sets the classOrigin attribute with the specified class name.
        Normally this method is used internally by a CIM Server when
        defining properties in the context of a schema.
        @param classOrigin A CIMName specifying the name of the class of
            origin for the property.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setClassOrigin(const CIMName& classOrigin);

    /**
        Tests the propagated attribute of the property.  The propagated
        attribute indicates whether this property was propagated from a
        higher-level class.  Normally this attribute is set as part of
        defining a property in the context of a schema.  It is set in
        properties retrieved from a CIM Server.
        @return True if property is propagated; otherwise, false.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean getPropagated() const;

    /**
        Sets the propagated attribute.  Normally this is used by a CIM Server
        when defining a property in the context of a schema.
        @param propagated A Boolean indicating whether the property is
            propagated.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setPropagated(Boolean propagated);

    /**
        Adds a qualifier to the property.
        @param x The CIMQualifier to be added.
        @return A reference to this CIMProperty object.
        @exception AlreadyExistsException If a qualifier with the
            same name already exists in the CIMProperty.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMProperty& addQualifier(const CIMQualifier& x);

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
        @return The CIMQualifier object at the specified index.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMProperty.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifier getQualifier(Uint32 index);

    /**
        Gets the qualifier at the specified index.
        @param index The index of the qualifier to be retrieved.
        @return The CIMConstQualifier object at the specified index.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMProperty.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Removes a qualifier from the property.
        @param index The index of the qualifier to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMProperty.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeQualifier(Uint32 index);

    /**
        Gets the number of qualifiers in the property.
        @return An integer count of the qualifiers in the CIMProperty.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Compares the property with another property.
        @param x The CIMConstProperty to be compared.
        @return True if this property is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstProperty& x) const;

    /**
        Makes a deep copy of the property.  This creates a new copy
        of all the property attributes including qualifiers.
        @return A new copy of the CIMProperty object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMProperty clone() const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

private:

    CIMProperty(CIMPropertyRep* rep);

    friend class CIMConstProperty;
    friend class CIMClassRep;
    friend class CIMInstanceRep;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
    friend class CIMObjectRep;
    friend class CIMPropertyContainer;
    friend class CIMPropertyInternal;
    friend class SCMOClass;
    friend class SCMOInstance;

    CIMPropertyRep* _rep;
};


////////////////////////////////////////////////////////////////////////////////
//
// CIMConstProperty
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMConstProperty class provides a const interface to a CIMProperty
    object.  This class is needed because the shared representation model
    used by CIMProperty does not prevent modification to a const CIMProperty
    object.  Note that the value of a CIMConstProperty object could still be
    modified by a CIMProperty object that refers to the same data copy.
*/
class PEGASUS_COMMON_LINKAGE CIMConstProperty
{
public:

    /**
        Constructs an uninitialized CIMConstProperty object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMConstProperty();

    /**
        Constructs a CIMConstProperty object from the value of a specified
        CIMConstProperty object, so that both objects refer to the same data
        copy.
        @param x The CIMConstProperty object from which to construct a new
            CIMConstProperty object.
    */
    CIMConstProperty(const CIMConstProperty& x);

    /**
        Constructs a CIMConstProperty object from the value of a specified
        CIMProperty object, so that both objects refer to the same data copy.
        @param x The CIMProperty object from which to construct a new
            CIMConstProperty object.
    */
    CIMConstProperty(const CIMProperty& x);

    /**
        Constructs a CIMConstProperty object with the specified attributes.
        @param name A CIMName specifying the name of the property.
        @param value A CIMValue specifying the property value, and implicitly
            defining the property type and whether the property is an Array
            property.
        @param arraySize A Uint32 indicating the size of the Array, if the
            property is an Array property.  The default value of zero
            indicates a variable size array.
        @param referenceClassName A CIMName containing the reference class
            name for this property, if the property is of reference type.
        @param classOrigin A CIMName indicating the class in which the
            property is locally defined (optional).
        @param propagated A Boolean indicating whether the property is local
            to the class or instance in which it appears or was propagated
            (without modification) from other schema.
        @exception TypeMismatchException If the property is of reference
            type and referenceClassName is null or if the property is not of
            reference type and referenceClassName is not null.
        @exception TypeMismatchException If the property is an Array property
            and arraySize is not zero.
        @exception UninitializedObjectException If the property name is null.
    */
    CIMConstProperty(
        const CIMName& name,
        const CIMValue& value,
        Uint32 arraySize = 0,
        const CIMName& referenceClassName = CIMName(),
        const CIMName& classOrigin = CIMName(),
        Boolean propagated = false);

    /**
        Destructs the CIMConstProperty object.
    */
    ~CIMConstProperty();

    /**
        Assigns the value of the specified CIMConstProperty object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMConstProperty object from which to assign this
            CIMConstProperty object.
        @return A reference to this CIMConstProperty object.
    */
    CIMConstProperty& operator=(const CIMConstProperty& x);

    /**
        Assigns the value of the specified CIMProperty object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMProperty object from which to assign this
            CIMConstProperty object.
        @return A reference to this CIMConstProperty object.
    */
    CIMConstProperty& operator=(const CIMProperty& x);

    /**
        Gets the name of the property.
        @return A CIMName containing the name of the property.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getName() const;

    /**
        Gets the property value.
        @return A CIMValue containing the property value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMValue& getValue() const;

    /**
        Gets the property type.
        @return A CIMType containing the property type.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMType getType() const;

    /**
        Checks whether the property is an Array property.
        @return True if the property is an Array property, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isArray() const;

    /**
        Gets the array size for the property.
        @return Uint32 array size.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getArraySize() const;

    /**
        Gets the reference class name for the property.
        @return A CIMName containing the reference class name for the
            property if the property is of reference type, a null CIMName
            otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getReferenceClassName() const;

    /**
        Gets the class in which this property is locally defined.  This
        information is normally available with properties that are part of
        schema returned from a CIM Server.
        @return CIMName containing the classOrigin attribute.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassOrigin() const;

    /**
        Tests the propagated attribute of the object.  The propagated
        attribute indicates whether this property was propagated from a
        higher-level class.  Normally this attribute is set as part of
        defining a property in the context of a schema.  It is set in
        properties retrieved from a CIM Server.
        @return True if property is propagated; otherwise, false.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean getPropagated() const;

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
        @return The CIMConstQualifier object at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of qualifiers available for the CIMConstProperty.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Gets the number of qualifiers in the property.
        @return An integer count of the qualifiers in the CIMConstProperty.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Compares the property with another property.
        @param x The CIMConstProperty to be compared.
        @return True if this property is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstProperty& x) const;

    /**
        Makes a deep copy of the property.  This creates a new copy
        of all the property attributes including qualifiers.
        @return A CIMProperty object with a separate copy of the
            CIMConstProperty object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMProperty clone() const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

private:

    CIMPropertyRep* _rep;

    friend class CIMProperty;
    friend class CIMPropertyRep;
    friend class XmlWriter;
    friend class MofWriter;
    friend class CIMPropertyInternal;
    friend class SCMOClass;
    friend class SCMOInstance;
};

#define PEGASUS_ARRAY_T CIMProperty
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Property_h */
