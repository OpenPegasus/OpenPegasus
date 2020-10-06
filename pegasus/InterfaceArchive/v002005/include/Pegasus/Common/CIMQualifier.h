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

#ifndef Pegasus_Qualifier_h
#define Pegasus_Qualifier_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMValue.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMQualifier
//
////////////////////////////////////////////////////////////////////////////////

class CIMConstQualifier;
class CIMClassRep;
class Resolver;
class CIMQualifierRep;

/**
    A CIMQualifier represents a DMTF standard CIM qualifier.
    A CIMQualifier differs from a CIMQualifierDecl in that it has no scope
    attribute.

    <p>The CIMQualifier class uses a shared representation model, such that
    multiple CIMQualifier objects may refer to the same data copy.  Assignment
    and copy operators create new references to the same data, not distinct
    copies.  An update to a CIMQualifier object affects all the CIMQualifier
    objects that refer to the same data copy.  The data remains valid until
    all the CIMQualifier objects that refer to it are destructed.  A separate
    copy of the data may be created using the clone method.
*/
class PEGASUS_COMMON_LINKAGE CIMQualifier
{
public:

    /**
        Constructs an uninitialized CIMQualifier object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMQualifier();

    /**
        Constructs a CIMQualifier object from the value of a specified
        CIMQualifier object, so that both objects refer to the same data copy.
        @param x The CIMQualifier object from which to construct a new
            CIMQualifier object.
    */
    CIMQualifier(const CIMQualifier& x);

    /**
        Constructs a CIMQualifier object with the specified attributes.
        @param name A CIMName specifying the name of the qualifier.
        @param value A CIMValue specifying the qualifier value, and implicitly
            defining the qualifier type and whether the qualifier is an Array
            qualifier.
        @param flavor A CIMFlavor indicating the qualifier flavors.
        @param propagated A Boolean indicating whether the qualifier is local
            to the context in which it appears or was propagated
            (without modification) from other schema.
        @exception UninitializedObjectException If the qualifier name is null.
    */
    CIMQualifier(
	const CIMName& name, 
	const CIMValue& value, 
	const CIMFlavor & flavor = CIMFlavor (CIMFlavor::NONE),
	Boolean propagated = false);

    /**
        Destructs the CIMQualifier object.
    */
    ~CIMQualifier();

    /**
        Assigns the value of the specified CIMQualifier object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMQualifier object from which to assign this
            CIMQualifier object.
        @return A reference to this CIMQualifier object.
    */
    CIMQualifier& operator=(const CIMQualifier& x);

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
        Gets the qualifier value.
        @return A CIMValue containing the qualifier value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMValue& getValue() const;

    /**
        Sets the qualifier value.
        @param name A CIMValue containing the new value of the qualifier.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setValue(const CIMValue& value);

    /**
        Adds flavors to the qualifier.
        @param flavor A CIMFlavor indicating the flavors to add.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setFlavor(const CIMFlavor & flavor);

    /**
        Removes flavors from the qualifier.
        @param flavor A CIMFlavor indicating the flavors to remove.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void unsetFlavor(const CIMFlavor & flavor);

    /**
        Gets the qualifier flavors.
        @return A CIMFlavor containing the qualifier flavor settings.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMFlavor & getFlavor() const;

    /**
        Tests the propagated attribute of the qualifier.  The propagated
        attribute indicates whether this qualifier was propagated from a
        higher-level class.  Normally this attribute is set as part of
        defining a qualifier in the context of a schema.  It is set in
        qualifiers retrieved from a CIM Server.
        (Note: Although this method is intended to return a Boolean value,
        changing the return type would break interface compatibility.)
        @return Non-zero if qualifier is propagated, otherwise zero.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const Uint32 getPropagated() const;

    /**
        Sets the propagated attribute.  Normally this is used by a CIM Server
        when defining a qualifier in the context of a schema.
        @param propagated A Boolean indicating whether the qualifier is
            propagated.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setPropagated(Boolean propagated);

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Compares the qualifier with another qualifier.
        @param x The CIMConstQualifier to be compared.
        @return True if this qualifier is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstQualifier& x) const;

    /**
        Makes a deep copy of the qualifier.  This creates a new copy
        of all the qualifier attributes.
        @return A new copy of the CIMQualifier object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifier clone() const;

private:

    CIMQualifier(CIMQualifierRep* rep);

    void _checkRep() const;

    CIMQualifierRep* _rep;

    friend class CIMConstQualifier;
    friend class CIMClassRep;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
};


////////////////////////////////////////////////////////////////////////////////
//
// CIMConstQualifier
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMConstQualifier class provides a const interface to a CIMQualifier
    object.  This class is needed because the shared representation model
    used by CIMQualifier does not prevent modification to a const CIMQualifier
    object.  Note that the value of a CIMConstQualifier object could still be
    modified by a CIMQualifier object that refers to the same data copy.
*/
class PEGASUS_COMMON_LINKAGE CIMConstQualifier
{
public:

    /**
        Constructs an uninitialized CIMConstQualifier object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMConstQualifier();

    /**
        Constructs a CIMConstQualifier object from the value of a specified
        CIMConstQualifier object, so that both objects refer to the same data
        copy.
        @param x The CIMConstQualifier object from which to construct a new
            CIMConstQualifier object.
    */
    CIMConstQualifier(const CIMConstQualifier& x);

    /**
        Constructs a CIMConstQualifier object from the value of a specified
        CIMQualifier object, so that both objects refer to the same data
        copy.
        @param x The CIMQualifier object from which to construct a new
            CIMConstQualifier object.
    */
    CIMConstQualifier(const CIMQualifier& x);

    /**
        Constructs a CIMConstQualifier object with the specified attributes.
        @param name A CIMName specifying the name of the qualifier.
        @param value A CIMValue specifying the qualifier value, and implicitly
            defining the qualifier type and whether the qualifier is an Array
            qualifier.
        @param flavor A CIMFlavor indicating the qualifier flavors.
        @param propagated A Boolean indicating whether the qualifier is local
            to the context in which it appears or was propagated
            (without modification) from other schema.
        @exception UninitializedObjectException If the qualifier name is null.
    */
    CIMConstQualifier(
	const CIMName& name, 
	const CIMValue& value, 
	const CIMFlavor & flavor = CIMFlavor (CIMFlavor::NONE),
	Boolean propagated = false);

    /**
        Destructs the CIMQualifier object.
    */
    ~CIMConstQualifier();

    /**
        Assigns the value of the specified CIMConstQualifier object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMConstQualifier object from which to assign this
            CIMConstQualifier object.
        @return A reference to this CIMConstQualifier object.
    */
    CIMConstQualifier& operator=(const CIMConstQualifier& x);

    /**
        Assigns the value of the specified CIMQualifier object to this
        object, so that both objects refer to the same data copy.
        @param x The CIMQualifier object from which to assign this
            CIMConstQualifier object.
        @return A reference to this CIMConstQualifier object.
    */
    CIMConstQualifier& operator=(const CIMQualifier& x);

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
        Gets the qualifier value.
        @return A CIMValue containing the qualifier value.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMValue& getValue() const;

    /**
        Gets the qualifier flavors.
        @return A CIMFlavor containing the qualifier flavor settings.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMFlavor & getFlavor() const;

    /**
        Tests the propagated attribute of the qualifier.  The propagated
        attribute indicates whether this qualifier was propagated from a
        higher-level class.  Normally this attribute is set as part of
        defining a qualifier in the context of a schema.  It is set in
        qualifiers retrieved from a CIM Server.
        (Note: Although this method is intended to return a Boolean value,
        changing the return type would break interface compatibility.)
        @return Non-zero if qualifier is propagated, otherwise zero.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const Uint32 getPropagated() const;

    /**
        Determines whether the object has been initialized.
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Compares the qualifier with another qualifier.
        @param x The CIMConstQualifier to be compared.
        @return True if this qualifier is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstQualifier& x) const;

    /**
        Makes a deep copy of the qualifier.  This creates a new copy
        of all the qualifier attributes.
        @return A CIMQualifier object with a separate copy of the
            CIMConstQualifier object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifier clone() const;

private:

    void _checkRep() const;

    CIMQualifierRep* _rep;

    friend class CIMQualifier;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMQualifier
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Qualifier_h */
