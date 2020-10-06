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

#ifndef Pegasus_Method_h
#define Pegasus_Method_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMParameter.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMType.h>

PEGASUS_NAMESPACE_BEGIN

class Resolver;
class CIMConstMethod;
class CIMMethodRep;

/**
    The CIMMethod class represents the DMTF standard CIM method definition.
    A CIMMethod is generally defined in the context of a CIMClass.
    A CIMMethod consists of:
    <ul>
        <li>A CIMName containing the name of the method
        <li>A CIMType defining the method return type
        <li>Zero or more CIMQualifier objects
        <li>Zero or more CIMParameter objects defining the method parameters
    </ul>
    In addition, a CIMMethod has these internal attributes:
    <ul>
        <li><b>propagated</b> - An attribute defining whether this CIMMethod
            is propagated from a superclass.  Note that this is normally set
            as part of completing the definition of objects (resolving) when
            they are created as part of a CIM schema and is NOT automatically
            set when creating a method object.  It can only be logically set
            in context of the schema in which the CIMMethod is defined.
        <li><b>classOrigin</b> - An attribute defining the class in which
            this CIMMethod was originally defined.  This is normally set
            within the context of the schema in which the CIMMethod is
            defined.  This attribute is available from objects retrieved
            from the CIM Server, for example, and provides information on
            the defintion of this method in the class hierarchy.  The
            propagated and ClassOrigin attributes can be used together to
            determine if methods originated with this object or were
            inherited from higher levels of the hiearchy.
    </ul>

    <p>The CIMMethod class uses a shared representation model, such that
    multiple CIMMethod objects may refer to the same data copy.  Assignment
    and copy operators create new references to the same data, not distinct
    copies.  An update to a CIMMethod object affects all the CIMMethod
    objects that refer to the same data copy.  The data remains valid until
    all the CIMMethod objects that refer to it are destructed.  A separate
    copy of the data may be created using the clone method.
*/
class PEGASUS_COMMON_LINKAGE CIMMethod
{
public:

    /**
        Constructs an uninitialized CIMMethod object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMMethod();

    /**
        Constructs a CIMMethod object from the value of a specified
        CIMMethod object, so that both objects refer to the same data copy.

        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            const CIMMethod cm1(m1);
        </pre>

        @param x The CIMMethod object from which to construct a new
            CIMMethod object.
    */
    CIMMethod(const CIMMethod& x);

    /**
        Constructs a CIMMethod object with the specified attributes.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
        </pre>

        @param name A CIMName specifying the name of the method.
        @param type A CIMType defining the method return type.
        @param classOrigin A CIMName indicating the class in which the
            method is locally defined (optional).
        @param propagated A Boolean indicating whether the method definition
            is local to the CIMClass in which it appears or was propagated
            (without modification) from a superclass.
        @exception UninitializedObjectException If the method name is null.
    */
    CIMMethod(
        const CIMName& name,
        CIMType type,
        const CIMName& classOrigin = CIMName(),
        Boolean propagated = false);

    /**
        Destructs the CIMMethod object.
    */
    ~CIMMethod();

    /**
        Assigns the value of the specified CIMMethod object to this object,
        so that both objects refer to the same data copy.

        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMMethod m2;
            m2 = m1;
        </pre>

        @param x The CIMMethod object from which to assign this CIMMethod
            object.
        @return A reference to this CIMMethod object.
    */
    CIMMethod& operator=(const CIMMethod& x);

    /**
        Gets the name of the method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            assert(m1.getName() == CIMName ("getHostName"));
        </pre>
        @return A CIMName containing the name of the method.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getName() const;

    /**
        Sets the method name.
        <p><b>Example:</b>
        <pre>
            CIMMethod m2(CIMName ("test"), CIMTYPE_STRING);
            m2.setName(CIMName ("getVersion"));
        </pre>
        @return A CIMName containing the new name of the method.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setName(const CIMName& name);

    /**
        Gets the method return type.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            assert(m1.getType() == CIMTYPE_STRING);
        </pre>
        @return A CIMType containing the method return type.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMType getType() const;

    /**
        Sets the method return type to the specified CIMType.
        This is the type of the CIMValue
        that is returned on a CIM method invocation.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.setName(CIMName ("getVersion"));
            assert(m1.getName() == CIMName ("getVersion"));
        </pre>
        @param type CIMType to be set into the CIMMethod object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setType(CIMType type);

    /**
        Gets the class in which this method is locally defined.  This
        information is normally available with methods that are part of
        schema returned from a CIM Server.
        @return CIMName containing the classOrigin attribute.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassOrigin() const;

    /**
        Sets the classOrigin attribute with the specified class name.
        Normally this method is used internally by a CIM Server when
        defining methods in the context of a schema.
        @param classOrigin A CIMName specifying the name of the class of
            origin for the method.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setClassOrigin(const CIMName& classOrigin);

    /**
        Tests the propagated attribute of the object.  The propagated
        attribute indicates whether this method was propagated from a
        higher-level class.  Normally this attribute is set as part of
        defining a method in the context of a schema.  It is set in
        methods retrieved from a CIM Server.
        @return True if method is propagated; otherwise, false.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean getPropagated() const;

    /**
        Sets the propagated attribute.  Normally this is used by a CIM Server
        when defining a method in the context of a schema.
        @param propagated A Boolean indicating whether the method is
            propagated from a superclass.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void setPropagated(Boolean propagated);

    /**
        Adds a qualifier to the method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
        </pre>
        @param x The CIMQualifier to be added.
        @return A reference to this CIMMethod object.
        @exception AlreadyExistsException If a qualifier with the
            same name already exists in the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMMethod& addQualifier(const CIMQualifier& x);

    /**
        Finds a qualifier by name.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            assert(m1.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
        </pre>
        @param name A CIMName specifying the name of the qualifier to be found.
        @return Index of the qualifier if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /**
        Gets the qualifier at the specified index.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            Uint32 posQualifier = m1.findQualifier(CIMName ("stuff"));
            if (posQualifier != PEG_NOT_FOUND)
            {
                CIMQualifier q = m1.getQualifier(posQualifier);
            }
        </pre>
        @param index The index of the qualifier to be retrieved.
        @return The CIMQualifier object at the specified index.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMQualifier getQualifier(Uint32 index);

    /**
        Gets the qualifier at the specified index.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            const CIMMethod m2 = m1;
            Uint32 posQualifier = m2.findQualifier(CIMName ("stuff"));
            if (posQualifier != PEG_NOT_FOUND)
            {
                CIMConstQualifier q = m2.getQualifier(posQualifier);
            }
        </pre>
        @param index The index of the qualifier to be retrieved.
        @return The CIMConstQualifier object at the specified index.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Removes a qualifier from the method.
        <p><b>Example:</b>
        <pre>
            // remove all qualifiers from a class
            Uint32 count = 0;
            while((count = cimClass.getQualifierCount()) > 0)
                cimClass.removeQualifier(count - 1);
        </pre>
        @param index The index of the qualifier to remove.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeQualifier(Uint32 index);

    /**
        Gets the number of qualifiers in the method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            m1.addQualifier(CIMQualifier(CIMName ("stuff2"), true));
            assert(m1.getQualifierCount() == 2);
        </pre>
        @return An integer count of the qualifiers in the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Adds a parameter to the method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addParameter(CIMParameter(CIMName("ipaddress"), CIMTYPE_STRING));
        </pre>
        @param x The CIMParameter to be added.
        @return A reference to this CIMMethod object.
        @exception AlreadyExistsException If a parameter with the
            same name already exists in the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMMethod& addParameter(const CIMParameter& x);

    /**
        Finds a parameter by name.
        <p><b>Example:</b>
        <pre>
            Uint32 posParameter;
            posParameter = m1.findParameter(CIMName ("ipaddress"));
            if (posParameter != PEG_NOT_FOUND)
                ...
        </pre>
        @param name A CIMName specifying the name of the parameter to be found.
        @return Index of the parameter if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findParameter(const CIMName& name) const;

    /**
        Gets the parameter at the specified index.
        <p><b>Example:</b>
        <pre>
            CIMParameter cp;
            Uint32 parameterIndex = m1.findParameter(CIMName ("ipaddress"));
            if (parameterIndex != PEG_NOT_FOUND)
            {
                cp = m1.getParameter(parameterIndex);
            }
        </pre>
        @param index The index of the parameter to be retrieved.
        @return The CIMParameter at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of parameters available for the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMParameter getParameter(Uint32 index);

    /**
        Gets the parameter at the specified index.
        <p><b>Example:</b>
        <pre>
            CIMConstParameter cp;
            Uint32 parameterIndex = m1.findParameter(CIMName ("ipaddress"));
            if (parameterIndex != PEG_NOT_FOUND)
            {
                cp = m1.getParameter(parameterIndex);
            }
        </pre>
        @param index The index of the parameter to be retrieved.
        @return The CIMConstParameter at the specified index.
        @exception IndexOutOfBoundsException If the index is outside
            the range of parameters available for the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstParameter getParameter(Uint32 index) const;

    /**
        Removes a parameter from the method.
        @param index Index of the parameter to be removed.
        @exception IndexOutOfBoundsException If the index is outside the
            range of parameters available from the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    void removeParameter (Uint32 index);

    /**
        Gets the number of parameters in the method.
        @return An integer count of the CIMParameters in the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getParameterCount() const;

    /**
        Determines whether the object has been initialized.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1;
            assert(m1.isUninitialized());
        </pre>
        @return True if the object has not been initialized, false otherwise.
    */
    Boolean isUninitialized() const;

    /**
        Compares the method with another method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod m2(CIMName ("test"), CIMTYPE_STRING);
            assert(!m1.identical(m2));
        </pre>
        @param x The CIMConstMethod to be compared.
        @return True if this method is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstMethod& x) const;

    /**
        Makes a deep copy of the method.  This creates a new copy
        of all the method attributes including parameters and qualifiers.
        @return A new copy of the CIMMethod object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMMethod clone() const;

private:

    CIMMethod(CIMMethodRep* rep);

    /**
        This method is not implemented.  It is defined to explicitly disallow
        construction of a CIMMethod from a CIMConstMethod.  Because the
        CIMMethod class uses a shared representation model, allowing this
        construction would effectively allow modification of CIMConstMethod
        objects.
    */
    PEGASUS_EXPLICIT CIMMethod(const CIMConstMethod& x);

    void _checkRep() const;

    CIMMethodRep* _rep;
    friend class CIMConstMethod;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;
    friend class BinaryStreamer;
};

/**
    The CIMConstMethod class provides a const interface to a CIMMethod
    object.  This class is needed because the shared representation model
    used by CIMMethod does not prevent modification to a const CIMMethod
    object.  Note that the value of a CIMConstMethod object could still be
    modified by a CIMMethod object that refers to the same data copy.
*/
class PEGASUS_COMMON_LINKAGE CIMConstMethod
{
public:

    /**
        Constructs an uninitialized CIMConstMethod object.  A method
        invocation on an uninitialized object will result in the throwing
        of an UninitializedObjectException.  An uninitialized object may
        be converted into an initialized object only by using the assignment
        operator with an initialized object.
    */
    CIMConstMethod();

    /**
        Constructs a CIMConstMethod object from the value of a specified
        CIMConstMethod object, so that both objects refer to the same data
        copy.

        <p><b>Example:</b>
        <pre>
            CIMConstMethod cm1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod cm2(m1);
        </pre>

        @param x The CIMConstMethod object from which to construct a new
        CIMConstMethod object.
    */
    CIMConstMethod(const CIMConstMethod& x);

    /**
        Constructs a CIMConstMethod object from the value of a specified
        CIMMethod object, so that both objects refer to the same data
        copy.

        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod cm1(m1);
        </pre>

        @param x The CIMMethod object from which to construct a new
            CIMConstMethod object.
    */
    CIMConstMethod(const CIMMethod& x);

    /**
        Constructs a CIMConstMethod object with the specified attributes.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
        </pre>

        @param name CIMName defining the name for the method.
        @param type CIMType defining the method return type.
        @param classOrigin (optional) CIMName representing the class origin.
            Note that this should normally not be used.  If not provided set to
            CIMName() (Null name).
        @param propagated Optional flag indicating whether the definition of
            the CIM Method is local to the CIM Class (respectively, Instance)
            in which it appears, or was propagated without modification from
            a superclass. Default is false. Note that this attribute is
            normally not set by CIM Clients but is used internally within the
            CIM Server.
        @exception UninitializedObjectException If the method name is null.
    */
    CIMConstMethod(
        const CIMName& name,
        CIMType type,
        const CIMName& classOrigin = CIMName(),
        Boolean propagated = false);

    /**
        Destructs the CIMConstMethod object.
    */
    ~CIMConstMethod();

    /**
        Assigns the value of the specified CIMConstMethod object to this
        object, so that both objects refer to the same data copy.

        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod m2;
            m2 = m1;
        </pre>

        @param x The CIMConstMethod object from which to assign this
            CIMConstMethod object.
        @return A reference to this CIMConstMethod object.
    */
    CIMConstMethod& operator=(const CIMConstMethod& x);

    /**
        Assigns the value of the specified CIMMethod object to this
        object, so that both objects refer to the same data copy.

        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod m2;
            m2 = m1;
        </pre>

        @param x The CIMMethod object from which to assign this
            CIMConstMethod object.
        @return A reference to this CIMConstMethod object.
    */
    CIMConstMethod& operator=(const CIMMethod& x);

    /**
        Gets the name of the method.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            assert(m1.getName() == CIMName ("getHostName"));
        </pre>
        @return CIMName with the name of the method.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getName() const;

    /**
        Gets the method return type.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            assert(m1.getType() == CIMTYPE_STRING);
        </pre>
        @return A CIMType containing the method return type.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMType getType() const;

    /**
        Gets the class in which this method was defined. This information
        is normally available with methods that are part of a schema
        returned from a CIM Server.
        @return CIMName containing the classOrigin attribute.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    const CIMName& getClassOrigin() const;

    /**
        Tests the propagated attribute of the object.  The propagated
        attribute indicates whether this method was propagated from a
        higher-level class.  Normally this attribute is set as part of
        defining a method in the context of a schema.  It is set in
        methods retrieved from a CIM Server.
        @return True if method is propagated; otherwise, false.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Boolean getPropagated() const;

    /**
        Finds a qualifier by name.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            CIMConstMethod m2(m1);
            assert(m2.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
        </pre>
        @param name A CIMName specifying the name of the qualifier to be found.
        @return Index of the qualifier if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findQualifier(const CIMName& name) const;

    /**
        Gets the qualifier at the specified index.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            CIMConstMethod m2(m1);
            Uint32 posQualifier = m2.findQualifier(CIMName ("stuff"));
            if (posQualifier != PEG_NOT_FOUND)
            {
                CIMQualifier q = m2.getQualifier(posQualifier);
            }
        </pre>
        @param index The index of the qualifier to be retrieved.
        @return The CIMConstQualifier at the specified index.
        @exception IndexOutOfBoundsException If the index is
            outside the range of qualifiers available for the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /**
        Gets the number of qualifiers in the method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            m1.addQualifier(CIMQualifier(CIMName ("stuff2"), true));
            CIMConstMethod m2(m1);
            assert(m2.getQualifierCount() == 2);
        </pre>
        @return An integer count of the CIMQualifiers in the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getQualifierCount() const;

    /**
        Finds a parameter by name.
        <p><b>Example:</b>
        <pre>
            Uint32 posParameter;
            posParameter = m1.findParameter(CIMName ("ipaddress"));
            if (posParameter != PEG_NOT_FOUND)
                ...
        </pre>
        @param name A CIMName specifying the name of the parameter to be found.
        @return Index of the parameter if found or PEG_NOT_FOUND if not found.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 findParameter(const CIMName& name) const;

    /**
        Gets the parameter at the specified index.
        <p><b>Example:</b>
        <pre>
            CIMConstParameter cp;
            Uint32 parameterIndex = m1.findParameter(CIMName ("ipaddress"));
            if (parameterIndex != PEG_NOT_FOUND)
            {
                cp = m1.getParameter(parameterIndex);
            }
        </pre>
        @param index The index of the parameter to be retrieved.
        @return The CIMConstParameter at the specified index.
        @exception IndexOutOfBoundsException If the index is
            outside the range of parameters available for the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMConstParameter getParameter(Uint32 index) const;

    /**
        Gets the number of parameters in the method.
        @return An integer count of the CIMParameters in the CIMMethod.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    Uint32 getParameterCount() const;

    /**
        Determines whether the object has been initialized.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1;
            assert(m1.isUninitialized());
        </pre>
        @return True if the object has not been initialized, false otherwise.
     */
    Boolean isUninitialized() const;

    /**
        Compares the method with another method.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod m2(CIMName ("test"), CIMTYPE_STRING);
            assert(!m1.identical(m2));
        </pre>
        @param x The CIMConstMethod to be compared.
        @return True if this method is identical to the one specified,
            false otherwise.
        @exception UninitializedObjectException If either of the objects
            is not initialized.
    */
    Boolean identical(const CIMConstMethod& x) const;

    /**
        Makes a deep copy of the method.  This creates a new copy
        of all the method attributes including parameters and qualifiers.
        @return A CIMMethod object with a separate copy of the
            CIMConstMethod object.
        @exception UninitializedObjectException If the object is not
            initialized.
    */
    CIMMethod clone() const;

private:

    void _checkRep() const;

    CIMMethodRep* _rep;

    friend class CIMMethod;
    friend class CIMMethodRep;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMMethod
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Method_h */
