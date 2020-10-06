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

#ifndef Pegasus_QualifierDecl_h
#define Pegasus_QualifierDecl_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/CIMScope.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMValue.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMQualifierDecl
//
////////////////////////////////////////////////////////////////////////////////

class CIMConstQualifierDecl;
class CIMClassRep;
class CIMQualifierDeclRep;

/**
    A CIMQualifierDecl represents a DMTF standard CIM qualifier declaration.
    A CIMQualifierDecl differs from a CIMQualifier in that it has a scope
    attribute.  A CIMQualifierDecl defines a qualifier, whereas a CIMQualifier
    applies the qualifier.

    <p>The CIMQualifierDecl class uses a shared representation model, such
    that multiple CIMQualifierDecl objects may refer to the same data copy.
    Assignment and copy operators create new references to the same data, not
    distinct copies.  An update to a CIMQualifierDecl object affects all the
    CIMQualifierDecl objects that refer to the same data copy.  The data
    remains valid until all the CIMQualifierDecl objects that refer to it are
    destructed.  A separate copy of the data may be created using the clone
    method.
*/
class PEGASUS_COMMON_LINKAGE CIMQualifierDecl
{
public:

    /**
        Constructs an uninitialized CIMQualifierDecl object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMQualifierDecl();

    /**
        Constructs a CIMQualifierDecl object from the value of a specified
        CIMQualifierDecl object, so that both objects refer to the same data
        copy.
        @param x The CIMQualifierDecl object from which to construct a new
            CIMQualifierDecl object.
    */
    CIMQualifierDecl(const CIMQualifierDecl& x);

    /**
        Constructs a CIMQualifierDecl object with the specified attributes.
        @param name A CIMName specifying the name of the qualifier.
        @param value A CIMValue specifying the default qualifier value, and
            implicitly defining the qualifier type and whether the qualifier
            is an Array qualifier.
        @param scope A CIMScope indicating the qualifier scope.
        @param flavor A CIMFlavor indicating the qualifier flavors.
        @param arraySize A Uint32 indicating the size of the Array, if the
            qualifier is an Array qualifier.  The default value of zero
            indicates a variable size array.
        @exception UninitializedObjectException If the qualifier name is null.
    */
    CIMQualifierDecl(
        const CIMName& name, 
        const CIMValue& value, 
        const CIMScope & scope,
        const CIMFlavor & flavor = CIMFlavor (CIMFlavor::DEFAULTS),
        Uint32 arraySize = 0);

    /**
        Destructs the CIMQualifierDecl object.
    */
    ~CIMQualifierDecl();

    /**
        Assigns the value of the specified CIMQualifierDecl object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMQualifierDecl object from which to assign this
            CIMQualifierDecl object.
        @return A reference to this CIMQualifierDecl object.
    */
    CIMQualifierDecl& operator=(const CIMQualifierDecl& x);

    /**
        Gets the name of the qualifier.
        @return A CIMName containing the name of the qualifier.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getName() const;

    /**
        Sets the qualifier name.
        @param name A CIMName containing the new name of the qualifier.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setName(const CIMName& name);

    /**
        Gets the qualifier type.
        @return A CIMType containing the qualifier type.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMType getType() const;

    /**
        Checks whether the qualifier is an Array qualifier.
        @return True if the qualifier is an Array qualifier, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isArray() const;

    /**
        Gets the qualifier default value.
        @return A CIMValue containing the qualifier default value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMValue& getValue() const;

    /**
        Sets the qualifier default value.
        @param name A CIMValue containing the new default value of the
            qualifier.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setValue(const CIMValue& value);

    /**
        Gets the qualifier scope.
        @return A CIMScope containing the qualifier scope.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMScope & getScope() const;

    /**
        Gets the qualifier flavors.
        @return A CIMFlavor containing the qualifier flavor settings.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMFlavor & getFlavor() const;

    /**
        Gets the array size for the qualifier.
        @return Uint32 array size.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getArraySize() const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;
    
    /**
        Compares the qualifier declaration with another qualifier declaration.
        @param x The CIMConstQualifierDecl to be compared.
        @return True if this qualifier declaration is identical to the one
            specified, false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstQualifierDecl& x) const;

    /**
        Makes a deep copy of the qualifier declaration.  This creates a new
        copy of all the qualifier declaration attributes.
        @return A new copy of the CIMQualifierDecl object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifierDecl clone() const;

private:

    CIMQualifierDecl(CIMQualifierDeclRep* rep);

    void _checkRep() const;

    CIMQualifierDeclRep* _rep;

    friend class CIMConstQualifierDecl;
    friend class CIMClassRep;
    friend class XmlWriter;
    friend class MofWriter;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstQualifierDecl
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMConstQualifierDecl class provides a const interface to a
    CIMQualifierDecl object.  This class is needed because the shared
    representation model used by CIMQualifierDecl does not prevent
    modification to a const CIMQualifierDecl object.  Note that the value
    of a CIMConstQualifierDecl object could still be modified by a
    CIMQualifierDecl object that refers to the same data copy.
*/
class PEGASUS_COMMON_LINKAGE CIMConstQualifierDecl
{
public:

    /**
        Constructs an uninitialized CIMConstQualifierDecl object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMConstQualifierDecl();

    /**
        Constructs a CIMConstQualifierDecl object from the value of a
        specified CIMConstQualifierDecl object, so that both objects refer
        to the same data copy.
        @param x The CIMConstQualifierDecl object from which to construct a
            new CIMConstQualifierDecl object.
    */
    CIMConstQualifierDecl(const CIMConstQualifierDecl& x);

    /**
        Constructs a CIMConstQualifierDecl object from the value of a
        specified CIMQualifierDecl object, so that both objects refer
        to the same data copy.
        @param x The CIMQualifierDecl object from which to construct a
            new CIMConstQualifierDecl object.
    */
    CIMConstQualifierDecl(const CIMQualifierDecl& x);

    /**
        Constructs a CIMConstQualifierDecl object with the specified
        attributes.
        @param name A CIMName specifying the name of the qualifier.
        @param value A CIMValue specifying the default qualifier value, and
            implicitly defining the qualifier type and whether the qualifier
            is an Array qualifier.
        @param scope A CIMScope indicating the qualifier scope.
        @param flavor A CIMFlavor indicating the qualifier flavors.
        @param arraySize A Uint32 indicating the size of the Array, if the
            qualifier is an Array qualifier.  The default value of zero
            indicates a variable size array.
        @exception UninitializedObjectException If the qualifier name is null.
    */
    CIMConstQualifierDecl(
        const CIMName& name, 
        const CIMValue& value, 
        const CIMScope & scope,
        const CIMFlavor & flavor = CIMFlavor (CIMFlavor::DEFAULTS),
        Uint32 arraySize = 0);

    /**
        Destructs the CIMConstQualifierDecl object.
    */
    ~CIMConstQualifierDecl();

    /**
        Assigns the value of the specified CIMConstQualifierDecl object to
        this object, so that both objects refer to the same data copy.
        @param x The CIMConstQualifierDecl object from which to assign this
            CIMConstQualifierDecl object.
        @return A reference to this CIMConstQualifierDecl object.
    */
    CIMConstQualifierDecl& operator=(const CIMConstQualifierDecl& x);

    /**
        Assigns the value of the specified CIMQualifierDecl object to
        this object, so that both objects refer to the same data copy.
        @param x The CIMQualifierDecl object from which to assign this
            CIMConstQualifierDecl object.
        @return A reference to this CIMConstQualifierDecl object.
    */
    CIMConstQualifierDecl& operator=(const CIMQualifierDecl& x);

    /**
        Gets the name of the qualifier.
        @return A CIMName containing the name of the qualifier.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getName() const;

    /**
        Gets the qualifier type.
        @return A CIMType containing the qualifier type.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMType getType() const;

    /**
        Checks whether the qualifier is an Array qualifier.
        @return True if the qualifier is an Array qualifier, false otherwise.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean isArray() const;

    /**
        Gets the qualifier default value.
        @return A CIMValue containing the qualifier default value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMValue& getValue() const;

    /**
        Gets the qualifier scope.
        @return A CIMScope containing the qualifier scope.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMScope & getScope() const;

    /**
        Gets the qualifier flavors.
        @return A CIMFlavor containing the qualifier flavor settings.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMFlavor & getFlavor() const;

    /**
        Gets the array size for the qualifier.
        @return Uint32 array size.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getArraySize() const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Compares the qualifier declaration with another qualifier declaration.
        @param x The CIMConstQualifierDecl to be compared.
        @return True if this qualifier declaration is identical to the one
            specified, false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstQualifierDecl& x) const;

    /**
        Makes a deep copy of the qualifier declaration.  This creates a new
        copy of all the qualifier declaration attributes.
        @return A CIMQualifierDecl object with a separate copy of the
            CIMConstQualifierDecl object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifierDecl clone() const;

private:

    void _checkRep() const;

    CIMQualifierDeclRep* _rep;

    friend class CIMQualifierDecl;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMQualifierDecl
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_QualifierDecl_h */
