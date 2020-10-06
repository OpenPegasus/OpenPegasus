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

#ifndef Pegasus_Parameter_h
#define Pegasus_Parameter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMParameter
//
////////////////////////////////////////////////////////////////////////////////

class Resolver;
class CIMConstParameter;
class CIMParameterRep;

/**
    The CIMParameter class represents the DMTF standard CIM parameter
    definition.  A CIMParameter is generally defined in the context of
    a CIMMethod.
    A CIM Parameter consists of:
    <ul>
        <li>A CIMName containing the name of the parameter
        <li>A CIMType defining the parameter type
        <li>A Boolean indicating whether it is an Array parameter
        <li>A Uint32 indicating the size of the Array, if the parameter is an
            Array parameter
        <li>A CIMName containing the reference class name for this parameter,
            if the parameter is of reference type
        <li>Zero or more CIMQualifier objects
    </ul>

    <p>The CIMParameter class uses a shared representation model, such that
    multiple CIMParameter objects may refer to the same data copy.  Assignment
    and copy operators create new references to the same data, not distinct
    copies.  An update to a CIMParameter object affects all the CIMParameter
    objects that refer to the same data copy.  The data remains valid until
    all the CIMParameter objects that refer to it are destructed.  A separate
    copy of the data may be created using the clone method.
*/
class PEGASUS_COMMON_LINKAGE CIMParameter
{
public:

    /**
        Constructs an uninitialized CIMParameter object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMParameter();

    /**
        Constructs a CIMParameter object from the value of a specified
        CIMParameter object, so that both objects refer to the same data copy.
        @param x The CIMParameter object from which to construct a new
            CIMParameter object.
    */
    CIMParameter(const CIMParameter& x);

    /**
        Constructs a CIMParameter object with the specified attributes.
        @param name A CIMName specifying the name of the parameter.
        @param type A CIMType defining the parameter type.
        @param isArray A Boolean indicating whether it is an Array parameter.
        @param arraySize A Uint32 indicating the size of the Array, if the
            parameter is an Array parameter.  The default value of zero
            indicates a variable size array.
        @param referenceClassName A CIMName containing the reference class
            name for this parameter, if the parameter is of reference type.
        @exception TypeMismatchException If the parameter is of reference
            type and referenceClassName is null or if the parameter is not of
            reference type and referenceClassName is not null.
        @exception TypeMismatchException If isArray is true and arraySize is
            not zero.
        @exception UninitializedObjectException If the parameter name is null.
    */
    CIMParameter(
        const CIMName& name,
        CIMType type,
        Boolean isArray = false,
        Uint32 arraySize = 0,
        const CIMName& referenceClassName = CIMName());

    /**
        Destructs the CIMParameter object.
    */
    ~CIMParameter();

    /**
        Assigns the value of the specified CIMParameter object to this object,
        so that both objects refer to the same data copy.
        @param x The CIMParameter object from which to assign this
            CIMParameter object.
        @return A reference to this CIMParameter object.
    */
    CIMParameter& operator=(const CIMParameter& x);

    /**
        Gets the parameter name.
        @return A CIMName containing the name of the parameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getName() const;

    /**
        Sets the parameter name.
        @param name A CIMName indicating the new name for the parameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setName(const CIMName& name);

    /**
        Checks whether the parameter is an Array parameter.
        @return True if the parameter is an Array parameter, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isArray() const;

    /**
        Gets the array size for the parameter.
        @return Uint32 array size.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getArraySize() const;

    /**
        Gets the reference class name for the parameter.
        @return A CIMName containing the reference class name for the
            parameter if the parameter is of reference type, a null CIMName
            otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getReferenceClassName() const;

    /**
        Gets the parameter type.
        @return A CIMType indicating the type of this parameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMType getType() const;

    /**
        Adds a qualifier to the parameter.
        @param x The CIMQualifier to be added.
        @return A reference to this CIMParameter object.
        @exception AlreadyExistsException If a qualifier with the
            same name already exists in the CIMParameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMParameter& addQualifier(const CIMQualifier& x);

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
            the range of qualifiers available for the CIMParameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifier getQualifier(Uint32 index);

    /**
        Removes a qualifier from the parameter.
        @param index Index of the qualifier to be removed.
        @exception IndexOutOfBoundsException If the index is outside
            the range of qualifiers available for the CIMParameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeQualifier (Uint32 index);

    /**
        Gets the qualifier at the specified index.
        @param index The index of the qualifier to be retrieved.
        @return The CIMConstQualifier at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of qualifiers available for the CIMParameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Gets the number of qualifiers in the parameter.
        <pre>
             // loop to access all qualifiers in a CIMparameter
             CIMParameter parm;
             ....               // build the parameter
             for (Uint32 i = 0 ; i < parm.getQualifierCount() ; i++)
                 ....
        </pre>
        @return An integer count of the CIMQualifiers in the CIMParameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Compares the parameter with another parameter.
        @param x The CIMConstParameter to be compared.
        @return True if this parameter is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstParameter& x) const;

    /**
        Makes a deep copy of the parameter.  This creates a new copy
        of all the parameter attributes including qualifiers.
        @return A new copy of the CIMParameter object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMParameter clone() const;

private:

    CIMParameter(CIMParameterRep* rep);

    void _checkRep() const;

    CIMParameterRep* _rep;

    friend class CIMConstParameter;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
};


////////////////////////////////////////////////////////////////////////////////
//
// CIMConstParameter
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMConstParameter class provides a const interface to a CIMParameter
    object.  This class is needed because the shared representation model
    used by CIMParameter does not prevent modification to a const CIMParameter
    object.  Note that the value of a CIMConstParameter object could still be
    modified by a CIMParameter object that refers to the same data copy.
*/
class PEGASUS_COMMON_LINKAGE CIMConstParameter
{
public:

    /**
        Constructs an uninitialized CIMConstParameter object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMConstParameter();

    /**
        Constructs a CIMConstParameter object from the value of a specified
        CIMConstParameter object, so that both objects refer to the same data
        copy.
        @param x The CIMConstParameter object from which to construct a new
            CIMConstParameter object.
    */
    CIMConstParameter(const CIMConstParameter& x);

    /**
        Constructs a CIMConstParameter object from the value of a specified
        CIMParameter object, so that both objects refer to the same data
        copy.
        @param x The CIMParameter object from which to construct a new
            CIMConstParameter object.
    */
    CIMConstParameter(const CIMParameter& x);

    /**
        Constructs a CIMConstParameter object with the specified attributes.
        @param name A CIMName specifying the name of the parameter.
        @param type A CIMType defining the parameter type.
        @param isArray A Boolean indicating whether it is an Array parameter.
        @param arraySize A Uint32 indicating the size of the Array, if the
            parameter is an Array parameter.  The default value of zero
            indicates a variable size array.
        @param referenceClassName A CIMName containing the reference class
            name for this parameter, if the parameter is of reference type.
        @exception TypeMismatchException If the parameter is of reference
            type and referenceClassName is null.
        @exception TypeMismatchException If isArray is true and arraySize is
            not zero.
    */
    CIMConstParameter(
        const CIMName& name,
        CIMType type,
        Boolean isArray = false,
        Uint32 arraySize = 0,
        const CIMName& referenceClassName = CIMName());

    /**
        Destructs the CIMConstParameter object.
    */
    ~CIMConstParameter();

    /**
        Assigns the value of the specified CIMConstParameter object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMConstParameter object from which to assign this
            CIMConstParameter object.
        @return A reference to this CIMConstParameter object.
    */
    CIMConstParameter& operator=(const CIMConstParameter& x);

    /**
        Assigns the value of the specified CIMParameter object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMParameter object from which to assign this
            CIMConstParameter object.
        @return A reference to this CIMConstParameter object.
    */
    CIMConstParameter& operator=(const CIMParameter& x);

    /**
        Gets the parameter name.
        @return A CIMName containing the name of the parameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getName() const;

    /**
        Checks whether the parameter is an Array parameter.
        @return True if the parameter is an Array parameter, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isArray() const;

    /**
        Gets the array size for the parameter.
        @return Uint32 array size.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getArraySize() const;

    /**
        Gets the reference class name for the parameter.
        @return A CIMName containing the reference class name for the
            parameter if the parameter is of reference type, a null CIMName
            otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getReferenceClassName() const;

    /**
        Gets the parameter type.
        @return A CIMType indicating the type of this parameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMType getType() const;

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
            the range of qualifiers available for the CIMParameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Gets the number of qualifiers in the parameter.
        @return An integer count of the qualifiers in the CIMParameter.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Compares the parameter with another parameter.
        @param x The CIMConstParameter to be compared.
        @return True if this parameter is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstParameter& x) const;

    /**
        Makes a deep copy of the parameter.  This creates a new copy
        of all the parameter attributes including qualifiers.
        @return A CIMParameter object with a separate copy of the
            CIMConstParameter object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMParameter clone() const;

private:

    void _checkRep() const;

    CIMParameterRep* _rep;
    friend class CIMParameter;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMParameter
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Parameter_h */
