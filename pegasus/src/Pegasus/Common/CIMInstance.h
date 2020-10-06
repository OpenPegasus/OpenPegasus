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

/**
    The CIMInstance class represents the DMTF standard CIM instance definition.

    <p>The CIMInstance class uses a shared representation model, such that
    multiple CIMInstance objects may refer to the same data copy.  Assignment
    and copy operators create new references to the same data, not distinct
    copies.  An update to a CIMInstance object affects all the CIMInstance
    objects that refer to the same data copy.  The data remains valid until
    all the CIMInstance objects that refer to it are destructed.  A separate
    copy of the data may be created using the clone method.
*/
class PEGASUS_COMMON_LINKAGE CIMInstance
{
public:

    /**
        Constructs an uninitialized CIMInstance object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMInstance();

    /**
        Constructs a CIMInstance object from the value of a specified
        CIMInstance object, so that both objects refer to the same data copy.
        @param x The CIMInstance object from which to construct a new
            CIMInstance object.
    */
    CIMInstance(const CIMInstance& x);

    /**
        Constructs a CIMInstance object from the value of a specified
        CIMObject object, so that both objects refer to the same data copy.
        @param x The CIMObject object from which to construct the
            CIMInstance object.
        @exception DynamicCastFailedException If a CIMInstance can not be
            created from the given CIMObject.
    */
    explicit CIMInstance(const CIMObject& x);

    /**
        Constructs a CIMInstance object with the specified class name.
        @param className A CIMName specifying the class name of the instance.
    */
    CIMInstance(const CIMName& className);

    /**
        Assigns the value of the specified CIMInstance object to this object,
        so that both objects refer to the same data copy.
        @param x The CIMInstance object from which to assign this CIMInstance
            object.
        @return A reference to this CIMInstance object.
    */
    CIMInstance& operator=(const CIMInstance& x);

    /**
        Destructs the CIMInstance object.
    */
    ~CIMInstance();

    /**
        Gets the class name of the instance.
        @return A CIMName containing the class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassName() const;

    /**
        Gets the object path for the instance.
        @return A CIMObjectPath containing the object path.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMObjectPath& getPath() const;

    /**
        Sets the object path for the instance.
        @param path A CIMObjectPath containing the object path.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setPath (const CIMObjectPath & path);

    /**
        Adds a qualifier to the instance.
        @param qualifier The CIMQualifier to be added.
        @return A reference to this CIMInstance object.
        @exception AlreadyExistsException If a qualifier with the
            same name already exists in the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMInstance& addQualifier(const CIMQualifier& qualifier);

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
            the range of qualifiers available for the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifier getQualifier(Uint32 index);

    /**
        Gets the qualifier at the specified index.
        @param index The index of the qualifier to be retrieved.
        @return The CIMConstQualifier at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of qualifiers available for the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Removes a qualifier from the instance.
        @param index The index of the qualifier to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeQualifier(Uint32 index);

    /**
        Gets the number of qualifiers in the instance.
        @return An integer count of the qualifiers in the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Adds a property to the instance.
        @param x The CIMProperty to be added.
        @return A reference to this CIMInstance object.
        @exception AlreadyExistsException If a property with the
            same name already exists in the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMInstance& addProperty(const CIMProperty& x);

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
            the range of properties available for the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMProperty getProperty(Uint32 index);

    /**
        Gets the property at the specified index.
        @param index The index of the property to be retrieved.
        @return The CIMConstProperty at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of properties available for the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /**
        Removes a property from the instance.
        @param index The index of the property to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of properties available for the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeProperty(Uint32 index);

    /**
        Gets the number of properties in the instance.
        @return An integer count of the properties in the CIMInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getPropertyCount() const;

    /**
        Builds the object path for this instance, based on the class name
        and property values in the instance and the Key qualifiers on the
        properties in the class definition.  The returned object path does
        not include hostname and namespace attributes.
        Note that this method does not update the path attribute of the
        CIMInstance.
        @return A CIMObjectPath containing the object path for the instance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMObjectPath buildPath(const CIMConstClass& cimClass) const;

    /**
        Makes a deep copy of the instance.  This creates a new copy of all
        the instance attributes including qualifiers and properties.
        @return A new copy of the CIMInstance object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMInstance clone() const;

    /**
        Compares the CIMInstance with a specified CIMConstInstance.
        @param x The CIMConstInstance to be compared.
        @return True if this instance is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean identical(const CIMConstInstance& x) const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES

// This function has been deprecated. Property filtering is now done by the
// CIMOM infrastructure.
    void filter(
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList & propertyList);
#endif
    /**
        This is a replacement function for filter(), it has  added
        for backward compatability and it is not optimized
        for performance.  We recommend that it not be used because:
        a) the response enviroment of OpenPegasus efficiently filters out
           properties that are not in the PropertyList
        b) If a provider wants to return just the properties in the propertyList
           it should NOT put them into the returned instances.
           Putting them into the instance and then removing them in
           the provider is a waste of energy.

        NOTE: As of CIM 2.14, added a new function to CIMPropertyList (contains)
        that allows efficient determination if a property is in the propertyList
        so that the provider can determine if a property is required easily
        before putting it into a response instance.

        @param includeQualifiers Boolean that determines if qualifiers are
        filtered out of the CIMInstance and any properties

        @param includClassOrigin Boolean that determines if the ClassOrigin
        attribute is filtered out of the CIMInstance

        @param propertyList CIMPropertyList that determines which properties
        are filtered out of the CIMInstance. Any property not in the
        propertyList is filtered out of the CIMInstance
    */
void instanceFilter(
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList & propertyList);

private:

    CIMInstanceRep* _rep;

    CIMInstance(CIMInstanceRep* rep);

    friend class CIMConstInstance;
    friend class CIMObject;
    friend class CIMConstObject;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
    friend class CIMClassRep;
    friend class SCMOInstance;
    friend class SCMOClass;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstInstance
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMConstInstance class provides a const interface to a CIMInstance
    object.  This class is needed because the shared representation model
    used by CIMInstance does not prevent modification to a const CIMInstance
    object.  Note that the value of a CIMConstInstance object could still be
    modified by a CIMInstance object that refers to the same data copy.
*/
class PEGASUS_COMMON_LINKAGE CIMConstInstance
{
public:

    /**
        Constructs an uninitialized CIMConstInstance object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMConstInstance();

    /**
        Constructs a CIMConstInstance object from the value of a specified
        CIMConstInstance object, so that both objects refer to the same data
        copy.
        @param x The CIMConstInstance object from which to construct a new
            CIMConstInstance object.
    */
    CIMConstInstance(const CIMConstInstance& x);

    /**
        Constructs a CIMConstInstance object from the value of a specified
        CIMInstance object, so that both objects refer to the same data
        copy.
        @param x The CIMInstance object from which to construct a new
            CIMConstInstance object.
    */
    CIMConstInstance(const CIMInstance& x);

    /**
        Constructs a CIMConstInstance object from the value of a specified
        CIMObject object, so that both objects refer to the same data copy.
        @param x The CIMObject object from which to construct the
            CIMConstInstance object.
        @exception DynamicCastFailedException If a CIMConstInstance can not be
            created from the given CIMObject.
    */
    explicit CIMConstInstance(const CIMObject& x);

    /**
        Constructs a CIMConstInstance object from the value of a specified
        CIMConstObject object, so that both objects refer to the same data
        copy.
        @param x The CIMConstObject object from which to construct the
            CIMConstInstance object.
        @exception DynamicCastFailedException If a CIMConstInstance can not be
            created from the given CIMConstObject.
    */
    explicit CIMConstInstance(const CIMConstObject& x);

    /**
        Constructs a CIMConstInstance object with the specified class name.
        @param className A CIMName specifying the class name of the instance.
    */
    CIMConstInstance(const CIMName& className);

    /**
        Assigns the value of the specified CIMConstInstance object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMConstInstance object from which to assign this
            CIMConstInstance object.
        @return A reference to this CIMConstInstance object.
    */
    CIMConstInstance& operator=(const CIMConstInstance& x);

    /**
        Assigns the value of the specified CIMInstance object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMInstance object from which to assign this
            CIMConstInstance object.
        @return A reference to this CIMConstInstance object.
    */
    CIMConstInstance& operator=(const CIMInstance& x);

    /**
        Destructs the CIMConstInstance object.
    */
    ~CIMConstInstance();

    /**
        Gets the class name of the instance.
        @return A CIMName containing the class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassName() const;

    /**
        Gets the object path for the instance.
        @return A CIMObjectPath containing the object path.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMObjectPath& getPath() const;

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
            the range of qualifiers available for the CIMConstInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Gets the number of qualifiers in the instance.
        @return An integer count of the qualifiers in the CIMInstance.
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
            the range of properties available for the CIMConstInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /**
        Gets the number of properties in the instance.
        @return An integer count of the properties in the CIMConstInstance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getPropertyCount() const;

    /**
        Builds the object path for this instance, based on the class name
        and property values in the instance and the Key qualifiers on the
        properties in the class definition.  The returned object path does
        not include hostname and namespace attributes.
        Note that this method does not update the path attribute of the
        CIMInstance.
        @return A CIMObjectPath containing the object path for the instance.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMObjectPath buildPath(const CIMConstClass& cimClass) const;

    /**
        Makes a deep copy of the instance.  This creates a new copy of all
        the instance attributes including qualifiers and properties.
        @return A CIMInstance object with a separate copy of the
            CIMConstInstance object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMInstance clone() const;

    /**
        Compares the CIMConstInstance with a specified CIMConstInstance.
        @param x The CIMConstInstance to be compared.
        @return True if this instance is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean identical(const CIMConstInstance& x) const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

private:

    CIMInstanceRep* _rep;

    friend class CIMInstance;
    friend class CIMObject;
    friend class CIMConstObject;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
    friend class SCMOInstance;
};

#define PEGASUS_ARRAY_T CIMInstance
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Instance_h */
