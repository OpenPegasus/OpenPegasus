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
class Resolver;

/** The CIMProperty class is used to represent CIM properties in Pegasus. A CIM Property
    consists of the following entities;
    
    <ul>
        <li>Name of the property, a CIMName. Functions are provided to manipulate the name.
        The name must be a legal name for a CIMProperty {@link CIMName}. 
        <li>CIMType of the value of the property, a CIMType.
        <li>CIMValue - The value of the property corresponding to the Type defined.  Properties
        can have either single values or arrays of values. Functions are provided to
        allow definition and query of the characteristics of the value (isArray(),
        getArraySize()).
    
        <li>Optional CIMQualifiers for the property. A property can contain zero or
        more CIMQualifiers and functions are provided to manipulate the
        list of CIMQualifiers
    </ul>
    In addition, internally, there are the following additional attributes
    that are part of a CIMProperty.
    <ul>
        <li>propagated - attributed defining whether this CIMMethod is 
        propagated from a superclass.  Note that this is normally set as part of 
        completing the definition of objects (resolving) when they are placed in a 
        repository and is NOT automatically set when creating a local object.  It 
        is part of the context of the object within the repository.  It can only 
        logically be set in context of the superclass of which this CIMProperty is 
        defined.  
        <li>ClassOrigin - attribute defining the superclass in which this 
        CIMProperty was originally defined.  This is normally set as part of 
        resolving Class and instances in the context of other objects (i.e.  a 
        repository).  This attribute is available from objects retrieved from the 
        repository, for example and indicates the Class/Instance in the hiearchy 
        (this object or a superclass or instance of a superclass)was originally 
        defined.  Together the propagated and ClassOrigin attributes can be used 
        to determine if methods originated with the current object or were 
        inherited from higher levels in the hiearchy.  
    </ul>
    Normally CIMProperties are defined in the context of CIMClasses and CIMInstances.
    A CIMClass or CIMInstance can include zero or more CIMProperties.
    CIMProperty is a shared class so that assignment and copy operators do not
    create new copies of the data representing a CIMMethod object but point
    back to the original object and the lifecycle of the original object is
    controlled by the accumulative lifecycle of any copies and assigned
    objects.
    {@link Shared Classes}
    @see CIMConstProperty
    @see CIMQualifiers
    @see CIMType
*/
class PEGASUS_COMMON_LINKAGE CIMProperty
{
public:

    /** Creates a new Null CIMProperty object. CIMProperites created with
        this constructor are have no information the only operation that can
        be performed on them is to copy another object into the new object.
        @exception throws UninitializedObjectException() if any method except the copy
        function is executed against.
        @see CIMConstProperty()
        @see Unitialized()
    */
    CIMProperty();

    /** Constructs a CIMPropery object from another 
        CIMProperty object. This method assigns the new object to the representation
        in the parameter and increments the representation count.  It does NOT
        create a new independent object be creates a reference from the assigned object
        to the representation of the object being assigned.
        @param x CIMProperty object from which to create newCIMProperty object.
        <pre>
            CIMProperty p1(CIMName ("name"), CIMTYPE_STRING);
            const CIMProperty cp1(p1);
        </pre>
    */
    CIMProperty(const CIMProperty& x);

    /** Constructs a CIMProperty with the specified attributes. Note that all attributes
        are optional except for the name and value.
        @param name Specifies the name of the  CIMproperty. This must be a legal CIMProperty
        name.
        @param value Specifies the name of the CIMValue property.
        @param arraySize Specifies the size of array, if fixed array size (optional).
        @param referenceClassName CIMName parameter that defines the 
        reference class name for the property. This parameter is required if
        the property is type CIMObjectPath. (optional)
        @param classOrigin CIMName parameter to define the class 
        origin of the property (optional). 
        @param propagated If true, this parameter defines the property as 
        propagated (optional).
        @return    The constructed CIM property object.
    */
    CIMProperty(
        const CIMName& name,
        const CIMValue& value,
        Uint32 arraySize = 0,
        const CIMName& referenceClassName = CIMName(),
        const CIMName& classOrigin = CIMName(),
        Boolean propagated = false);

    /** Destructs the CIMProperty object.
    */
    ~CIMProperty();

    /** REVIEWERS: Insert description here.
        @param x REVIEWERS: Insert description here.
    */
    CIMProperty& operator=(const CIMProperty& x);

    /** Gets the name of the property.
        @return CIMName containing the property name. For example, 
        <pre>
        CIMProperty p1("count", Uint32(231));
        assert(p1.getName() == "count");
        </pre>
    */
    const CIMName& getName() const;

    /** Set the property name.
        @param name  Specifies the CIMName that contains the name for 
        the parameter name.
    */
    void setName(const CIMName& name);

    /** Get the value of the property. 
    */
    const CIMValue& getValue() const;

    /** Get the type of the property. 
    */
    CIMType getType() const;

    /** Check if the property is an array type. 
    */
    Boolean isArray() const;

    /** Sets the Value in the Property object from the input 
        parameter.
        @param value Specifies the CIMValue that contains the value to be put into the 
        property.
    */
    void setValue(const CIMValue& value);

    /** Gets the arraysize parameter from the property.
        @return Uint32 with the arraysize.
    */
    Uint32 getArraySize() const;

    /** Gets the referenceClassName.
        @return CIMName containing the referenceClassName if this 
        is a reference property or empty if another CIMType.
    */
    const CIMName& getReferenceClassName() const;

    /** Gets the classOrigin field from the property
        object. 
        @return CIMName with classOrigin name. This will be a string 
        with the name of the originating class for the property or 
        empty if this is the originating class.
    */
    const CIMName& getClassOrigin() const;

    /** Sets the Class Origin attribute.
        @param classOrigin Specifies the CIMName that contains the classOrigin.
    */
    void setClassOrigin(const CIMName& classOrigin);

    /** Tests if this property is propagated.
        @return True if the class is propagated; otherwise, false.
    */
    Boolean getPropagated() const;

    /** Sets the propagated attribute true or false.
        @param x Specifies a true or false value that represents
        the propagated state to be set.
    */
    void setPropagated(Boolean propagated);

    /** Adds a qualifier object to the property and
        increases the qualifier count.
        @param x CIMQualifier object to be added.
        @return The resulting CIMProperpty.
        @exception AlreadyExistsException True if the qualifier already 
        exists; otherwise, false..
    */
    CIMProperty& addQualifier(const CIMQualifier& x);

    /** Finds the qualifier object defined by the name parameter 
        if it is attached to this CIMProperty.
        @param name CIMName parameter that defines the name of Qualifier
        object.
        @return Position of the qualifier object or PEG_NOT_FOUND (-1),  
        if not found.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /** Gets the Qualifier object specified by the input parameter.
        @param index Index parameter for the Qualifier object to be
        retrieved.
        @return CIMQualifier object at specified index.
        @exception IndexOutOfBoundsException If index is outside range
        of Qualifiers in this property object; otherwise, false.
    */
    CIMQualifier getQualifier(Uint32 index);

    /** Returns the qualifier at the specified index.
        @param index Specifies the index of the qualifier. The index is obtained from the
        findQualifier method.
        @return The qualifier object.
        @exception IndexOutOfBoundsException If index is outside the range
        of qualifiers that exist for the property; otherwise, false.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /** Removes the CIMQualifier defined by the 
        index input as a parameter.
        @param index Index of the qualifier requested.
        @exception IndexOutOfBoundsException If the index is outside the
        range of qualifiers in this property object; otherwise, false.
    */
    void removeQualifier(Uint32 index);
    
    /** Returns count of the number of qualifiers attached to 
        the CIMProperty object.
        @return Count of the number of CIMQualifier objects attached
        to the CIMProperty object.
    */
    Uint32 getQualifierCount() const;

    /** Compares the CIMProperty object with
        another CIMProperty object defined by the input parameter.
        @param x CIMPropery object for comparison.
        @return True if the objects are identical; otherwise, false.
    */
    Boolean identical(const CIMConstProperty& x) const;

    /** Makes a deep copy (clone) of the given object.
        @return Copy of the CIMProperty object.
    */
    CIMProperty clone() const;

    /** Determines if the object has not been initialized.
        @return  True if the object has not been initialized;
        otherwise, false.
    */
    Boolean isUninitialized() const;

private:

    CIMProperty(CIMPropertyRep* rep);

    /** This constructor allows the CIMClassRep friend class to cast
    away constness.
    @param x Specifies the name of the CIMConstProperty instance.
    */ 
    PEGASUS_EXPLICIT CIMProperty(const CIMConstProperty& x);

    void _checkRep() const;

    friend class CIMConstProperty;
    friend class CIMClassRep;
    friend class CIMInstanceRep;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;

    CIMPropertyRep* _rep;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstProperty
//
////////////////////////////////////////////////////////////////////////////////

///
class PEGASUS_COMMON_LINKAGE CIMConstProperty
{
public:

    /** REVIEWERS: Insert text here.
    */
    CIMConstProperty();

    /** REVIEWERS: Insert text here.
    */
    CIMConstProperty(const CIMConstProperty& x);

    /** REVIEWERS: Insert text here.
    */
    CIMConstProperty(const CIMProperty& x);

    /** REVIEWERS: Insert text here.
    */
    CIMConstProperty(
        const CIMName& name,
        const CIMValue& value,
        Uint32 arraySize = 0,
        const CIMName& referenceClassName = CIMName(),
        const CIMName& classOrigin = CIMName(),
        Boolean propagated = false);

    /** REVIEWERS: Insert text here.
    */
    ~CIMConstProperty();

    /** REVIEWERS: Insert text here.
    */
    CIMConstProperty& operator=(const CIMConstProperty& x);

    /** REVIEWERS: Insert text here.
    */
    CIMConstProperty& operator=(const CIMProperty& x);

    /** REVIEWERS: Insert text here.
    */
    const CIMName& getName() const;

    /** REVIEWERS: Insert text here.
    */
    const CIMValue& getValue() const;

    /** REVIEWERS: Insert text here.
    */
    CIMType getType() const;

    /** REVIEWERS: Insert text here.
    */
    Boolean isArray() const;

    /** REVIEWERS: Insert text here.
    */
    Uint32 getArraySize() const;

    /** REVIEWERS: Insert text here.
    */
    const CIMName& getReferenceClassName() const;

    /** REVIEWERS: Insert text here.
    */
    const CIMName& getClassOrigin() const;

    /** REVIEWERS: Insert text here.
    */
    Boolean getPropagated() const;

    /** REVIEWERS: Insert text here.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /** REVIEWERS: Insert text here.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /** REVIEWERS: Insert text here.
    */
    Uint32 getQualifierCount() const;

    /** REVIEWERS: Insert text here.
    */
    Boolean identical(const CIMConstProperty& x) const;

    /** REVIEWERS: Insert text here.
    */
    CIMProperty clone() const;

    ///
    Boolean isUninitialized() const;

private:

    void _checkRep() const;

    CIMPropertyRep* _rep;

    friend class CIMProperty;
    friend class CIMPropertyRep;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMProperty
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Property_h */
