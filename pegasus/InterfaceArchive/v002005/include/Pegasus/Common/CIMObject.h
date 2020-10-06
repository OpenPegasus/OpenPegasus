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
class CIMProperty;
class CIMConstProperty;
class CIMQualifier;
class CIMConstQualifier;

////////////////////////////////////////////////////////////////////////////////
//
// CIMObject
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMObject class represents the DMTF standard CIM object definition,
    which may represent a CIMClass or a CIMInstance.

    <p>The CIMObject class uses a shared representation model, such that
    multiple CIMObject objects may refer to the same data copy.  Assignment
    and copy operators create new references to the same data, not distinct
    copies.  An update to a CIMObject object affects all the CIMObject
    objects that refer to the same data copy.  The data remains valid until
    all the CIMObject objects that refer to it are destructed.  A separate
    copy of the data may be created using the clone method.
*/
class PEGASUS_COMMON_LINKAGE CIMObject
{
public:

    /**
        Constructs an uninitialized CIMObject object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMObject();

    /**
        Constructs a CIMObject object from the value of a specified
        CIMObject object, so that both objects refer to the same data copy.
        @param x The CIMObject object from which to construct a new
            CIMObject object.
    */
    CIMObject(const CIMObject& x);

    /**
        Constructs a CIMObject object from the value of a specified
        CIMClass object, so that both objects refer to the same data copy.
        @param x The CIMClass object from which to construct the
            CIMObject object.
    */
    CIMObject(const CIMClass& x);

    /**
        Constructs a CIMObject object from the value of a specified
        CIMInstance object, so that both objects refer to the same data copy.
        @param x The CIMInstance object from which to construct the
            CIMObject object.
    */
    CIMObject(const CIMInstance& x);

    /**
        Assigns the value of the specified CIMObject object to this object,
        so that both objects refer to the same data copy.
        @param x The CIMObject object from which to assign this CIMObject
            object.
        @return A reference to this CIMObject object.
    */
    CIMObject& operator=(const CIMObject& x);

    /**
        Destructs the CIMObject object.
    */
    ~CIMObject();

    /**
        Gets the class name of the object.
        @return A CIMName containing the class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassName() const;

    /**
        Gets the object path for the object.
        @return A CIMObjectPath containing the object path.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMObjectPath& getPath() const;

    /**
        Sets the object path for the object.
        @param path A CIMObjectPath containing the object path.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setPath (const CIMObjectPath & path);

    /**
        Adds a qualifier to the object.
        @param qualifier The CIMQualifier to be added.
        @return A reference to this CIMObject object.
        @exception AlreadyExistsException If a qualifier with the
            same name already exists in the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMObject& addQualifier(const CIMQualifier& qualifier);

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
            the range of qualifiers available for the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifier getQualifier(Uint32 index);

    /**
        Gets the qualifier at the specified index.
        @param index The index of the qualifier to be retrieved.
        @return The CIMConstQualifier at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of qualifiers available for the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Removes a qualifier from the object.
        @param index The index of the qualifier to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeQualifier(Uint32 index);

    /**
        Gets the number of qualifiers in the object.
        @return An integer count of the qualifiers in the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Adds a property to the object.
        @param x The CIMProperty to be added.
        @return A reference to this CIMObject object.
        @exception AlreadyExistsException If a property with the
            same name already exists in the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMObject& addProperty(const CIMProperty& x);

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
            the range of properties available for the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMProperty getProperty(Uint32 index);

    /**
        Gets the property at the specified index.
        @param index The index of the property to be retrieved.
        @return The CIMConstProperty at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of properties available for the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /**
        Removes a property from the object.
        @param index The index of the property to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of properties available for the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeProperty(Uint32 index);

    /**
        Gets the number of properties in the object.
        @return An integer count of the properties in the CIMObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getPropertyCount() const;

    /**
        Makes a deep copy of the object.  This creates a new copy of all
        the object attributes including qualifiers and properties.
        @return A new copy of the CIMObject object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMObject clone() const;

    /**
        Compares the CIMObject with a specified CIMConstObject.
        @param x The CIMConstObject to be compared.
        @return True if this object is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean identical(const CIMConstObject& x) const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Generates a human-readable String representing the value of the
        CIMObject.  The String may be in MOF format, but the format is not
        guaranteed and may change without notice.
        @return A human-readable String representing the CIMObject value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    String toString() const;

    /**
        Indicates whether the object represents a CIMClass.
        @return True if the object represents a CIMClass; false otherwise.
    */
    Boolean isClass() const;

    /**
        Indicates whether the object represents a CIMInstance.
        @return True if the object represents a CIMInstance; false otherwise.
    */
    Boolean isInstance() const;

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

/**
    The CIMConstObject class provides a const interface to a CIMObject
    object.  This class is needed because the shared representation model
    used by CIMObject does not prevent modification to a const CIMObject
    object.  Note that the value of a CIMConstObject object could still be
    modified by a CIMObject object that refers to the same data copy.
*/
class PEGASUS_COMMON_LINKAGE CIMConstObject
{
public:

    /**
        Constructs an uninitialized CIMConstObject object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMConstObject();

    /**
        Constructs a CIMConstObject object from the value of a specified
        CIMConstObject object, so that both objects refer to the same data
        copy.
        @param x The CIMConstObject object from which to construct a new
            CIMConstObject object.
    */
    CIMConstObject(const CIMConstObject& x);

    /**
        Constructs a CIMConstObject object from the value of a specified
        CIMObject object, so that both objects refer to the same data copy.
        @param x The CIMObject object from which to construct a new
            CIMConstObject object.
    */
    CIMConstObject(const CIMObject& x);

    /**
        Constructs a CIMConstObject object from the value of a specified
        CIMClass object, so that both objects refer to the same data copy.
        @param x The CIMClass object from which to construct the
            CIMConstObject object.
    */
    CIMConstObject(const CIMClass& x);

    /**
        Constructs a CIMConstObject object from the value of a specified
        CIMInstance object, so that both objects refer to the same data copy.
        @param x The CIMInstance object from which to construct the
            CIMConstObject object.
    */
    CIMConstObject(const CIMInstance& x);

    /**
        Constructs a CIMConstObject object from the value of a specified
        CIMConstClass object, so that both objects refer to the same data copy.
        @param x The CIMConstClass object from which to construct the
            CIMConstObject object.
    */
    CIMConstObject(const CIMConstClass& x);

    /**
        Constructs a CIMConstObject object from the value of a specified
        CIMConstInstance object, so that both objects refer to the same data
        copy.
        @param x The CIMConstInstance object from which to construct the
            CIMConstObject object.
    */
    CIMConstObject(const CIMConstInstance& x);

    /**
        Assigns the value of the specified CIMConstObject object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMConstObject object from which to assign this
            CIMConstObject object.
        @return A reference to this CIMConstObject object.
    */
    CIMConstObject& operator=(const CIMConstObject& x);

    /**
        Destructs the CIMConstObject object.
    */
    ~CIMConstObject();

    /**
        Gets the class name of the object.
        @return A CIMName containing the class name.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassName() const;

    /**
        Gets the object path for the object.
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
            the range of qualifiers available for the CIMConstObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Gets the number of qualifiers in the object.
        @return An integer count of the qualifiers in the CIMConstObject.
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
            the range of properties available for the CIMConstObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstProperty getProperty(Uint32 index) const;

    /**
        Gets the number of properties in the object.
        @return An integer count of the properties in the CIMConstObject.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getPropertyCount() const;

    /**
        Makes a deep copy of the object.  This creates a new copy of all
        the object attributes including qualifiers and properties.
        @return A CIMObject object with a separate copy of the
            CIMConstObject object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMObject clone() const;

    /**
        Compares the CIMConstObject with a specified CIMConstObject.
        @param x The CIMConstObject to be compared.
        @return True if this object is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean identical(const CIMConstObject& x) const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Generates a human-readable String representing the value of the
        CIMObject.  The String may be in MOF format, but the format is not
        guaranteed and may change without notice.
        @return A human-readable String representing the CIMObject value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    String toString () const;

    /**
        Indicates whether the object represents a CIMConstClass.
        @return True if the object represents a CIMConstClass; false otherwise.
    */
    Boolean isClass() const;

    /**
        Indicates whether the object represents a CIMConstInstance.
        @return True if the object represents a CIMConstInstance; false
            otherwise.
    */
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
