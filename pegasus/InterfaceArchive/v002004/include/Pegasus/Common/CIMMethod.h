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

/** The CIMMethod class is used to represent CIM methods in Pegasus.
    A CIMMethod consists of the following entities:
    <ul>
        <li>Name of the method, a CIMName.

        <li>CIM type of the method return value, a CIMType.

        <li>Optional qualifiers (see CIMQualifier) for the method.
        A method can contain zero or more CIMQualifier objects.

        <li>Optional parameters (see CIMParameter) for the method.
        A CIMMethod may contain zero or more CIMParameter objects.
    </ul>
    In addition, a CIMMethod contains the following internal attributes:
    <ul>
        <li><b>propagated</b> - An attribute defining whether this CIMMethod is
        propagated from a superclass.  Note that this is normally set as part
        of completing the definition of objects (resolving) when they are
        created as part of a CIM schema and is NOT automatically set when
        creating a local object.  It can only be logically set in context of
        the schema in which the CIMMethod is defined.
        <li><b>classOrigin</b> - An attribute defining the class in which
        this CIMMethod was originally defined.  This is normally set within the
        context of the schema in which the CIMMethod is defined.
        This attribute is available from objects retrieved from the CIM
        Server, for example, and provides information on the defintion
        of this method in the class hierarchy.  Together the
        propagated and ClassOrigin attributes can be used to determine if
        methods originated with the current object or were inherited from
        higher levels in the hiearchy.
    </ul>
    A CIMMethod is generally defined in the context of a CIMClass.

    CIMMethod uses shared representations, meaning that multiple
    CIMMethod objects may refer to the same copy of data. Assignment and copy
    operators create new references to the same data, not distinct copies.
    A distinct copy may be created using the clone method.
    {@link Shared Classes}
    @see CIMConstMethod
    @see CIMParameter
    @see CIMQualifier
    @see CIMType
    @see CIMClass
*/
class PEGASUS_COMMON_LINKAGE CIMMethod
{
public:

    /** Creates a new uninitialized CIMMethod object.
        The only thing that can be done with this object is to copy another
        object into it.  Other methods, such as setName, will fail with an
        UninitializedObjectException.  The object has an uninitialized state,
        which can be tested with the isUninitialized method.
        @see isUninitialized()
        @see UninitializedObjectException
    */
    CIMMethod();

    /** Creates a new CIMMethod object from another CIMMethod object.
        The new CIMMethod object references the same copy of data as the
        specified object; no copy is made.
        @param x CIMMethod object from which to create the new CIMMethod object.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            const CIMMethod cm1(m1);
        </pre>
        {@link Shared Classes}
    */
    CIMMethod(const CIMMethod& x);

    /** Creates a CIMMethod object with the specified attributes.
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
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
        </pre>
    */
    CIMMethod(
        const CIMName& name,
        CIMType type,
        const CIMName& classOrigin = CIMName(),
        Boolean propagated = false);

    /** Destructor for the CIMMethod.  The shared data copy remains valid until
        all referring objects are destructed.
        {@link Shared Classes}
    */
    ~CIMMethod();

    /** The assignment operator assigns one CIMMethod to another.
        After the assignment, both CIMMethod objects refer to the same
        data copy; a distinct copy is not created.
        @param x CIMMethod object from which to assign this CIMMethod object.

        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMMethod m2;
            m2 = m1;
        </pre>
    */
    CIMMethod& operator=(const CIMMethod& x);

    /** Gets the name of the method.
        @return CIMName with the name of the method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            assert(m1.getName() == CIMName ("getHostName"));
        </pre>
    */
    const CIMName& getName() const;

    /** Sets the method name.
        @param name CIMName for the method name. Replaces any
            previously defined name for this method object.
        <p><b>Example:</b>
        <pre>
            CIMMethod m2(CIMName ("test"), CIMTYPE_STRING);
            m2.setName(CIMName ("getVersion"));
        </pre>
    */
    void setName(const CIMName& name);

    /** Gets the method return type.
        @return A CIMType containing the method return type.
        @exception UninitializedObjectException Thrown if the object is not
        initialized.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            assert(m1.getType() == CIMTYPE_STRING);
        </pre>
    */
    CIMType getType() const;

    /** Sets the method return type to the specified CIMType.
        This is the type of the CIMValue
        that is returned on a CIM method invocation.
        @param type CIMType to be set into the CIMMethod object.
        @exception UninitializedObjectException Thrown if the object is not
        initialized.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.setName(CIMName ("getVersion"));
            assert(m1.getName() == CIMName ("getVersion"));
        </pre>
    */
    void setType(CIMType type);

    /** Gets the class in which this method was defined. This information
        is normally available with methods that are part of a schema
        returned from a CIM Server.
        @return CIMName containing the classOrigin attribute.
    */
    const CIMName& getClassOrigin() const;

    /** Sets the classOrigin attribute with the specified class name.
        Normally this method is used internally by a CIM Server when
        defining methods in the context of a schema.
        @param classOrigin CIMName parameter defining the name
        of the origin class.
        @exception UninitializedObjectException Thrown if the object is not
        initialized.
    */
    void setClassOrigin(const CIMName& classOrigin);

    /** Tests the propagated attribute of the object.  The propagated
        attribute indicates whether this method was propagated from a
        higher-level class.  Normally this attribute is set as part of
        defining a method in the context of a schema.  It is set in
        methods retrieved from a CIM Server.
        @return True if method is propagated; otherwise, false.
    */
    Boolean getPropagated() const;

    /** Sets the propagated attribute.  Normally this is used by a CIM Server
        when defining a method in the context of a schema.
        @param propagated Flag indicating whether the method is propagated
        from a superclass.
        @exception UninitializedObjectException Thrown if the object is not
        initialized.
    */
    void setPropagated(Boolean propagated);

    /** Adds the specified qualifier to the CIMMethod object.
        @param x CIMQualifier object representing the qualifier
        to be added.
        @return The CIMMethod object after adding the specified qualifier.
        @exception AlreadyExistsException Thrown if the qualifier already
        exists in this CIMMethod.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
        </pre>
    */
    CIMMethod& addQualifier(const CIMQualifier& x);

    /** Searches for a qualifier with the specified input name.
        @param name CIMName of the qualifier to be found.
        @return Zero origin index of the qualifier found or PEG_NOT_FOUND
        if not found.
        @exception UninitializedObjectException Thrown if the object is not
        initialized.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            assert(m1.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
        </pre>
    */
    Uint32 findQualifier(const CIMName& name) const;

    /** Gets the CIMQualifier defined by the input parameter.
        @param index Zero origin index of the qualifier requested.
        @return CIMQualifier object representing the qualifier found.
        @exception IndexOutOfBoundsException Thrown if the index is
        outside the range of parameters available from the CIMMethod.
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
    */
    CIMQualifier getQualifier(Uint32 index);

    /** Gets the CIMQualifier defined by the input parameter.
        @param index Zero origin index of the qualifier requested.
        @return CIMQualifier object representing the qualifier found.
        @exception IndexOutOfBoundsException Thrown if the index is
        outside the range of parameters available from the CIMMethod.
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
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /** Removes the specified qualifier from this method.
        @param index Index of the qualifier to remove.
        @exception IndexOutOfBoundsException Thrown if the index is
            outside the range of parameters available from the CIMMethod.
        <p><b>Example:</b>
        <pre>
            // remove all qualifiers from a class
            Uint32 count = 0;
            while((count = cimClass.getQualifierCount()) > 0)
                cimClass.removeQualifier(count - 1);
        </pre>
    */
    void removeQualifier(Uint32 index);

    /** Returns the number of Qualifiers attached to this CIMMethod object.
        @return The number of qualifiers attached to the CIM Method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            m1.addQualifier(CIMQualifier(CIMName ("stuff2"), true));
            assert(m1.getQualifierCount() == 2);
        </pre>
    */
    Uint32 getQualifierCount() const;

    /** Adds the parameter defined by the input to the CIMMethod.
        @param x CIMParameter to be added to the CIM Method.
        @return CIMMethod object after the specified parameter is added.
        <p><b>Example:</b>
        @exception UninitializedObjectException Thrown if the object is not
        initialized.
        @exception AlreadyExistsException Thrown if the parameter already
        exists in this CIMMethod.
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addParameter(CIMParameter(CIMName ("ipaddress"), CIMTYPE_STRING));
        </pre>
    */
    CIMMethod& addParameter(const CIMParameter& x);

    /** Finds the parameter with the specified name.
        @param name CIMName of parameter to be found.
        @return Index of the parameter object found or PEG_NOT_FOUND
        if the property is not found.
        <p><b>Example:</b>
        <pre>
            Uint32 posParameter;
            posParameter = m1.findParameter(CIMName ("ipaddress"));
            if (posParameter != PEG_NOT_FOUND)
                ...
        </pre>
    */
    Uint32 findParameter(const CIMName& name) const;

    /** Gets the parameter defined by the specified index.
        @param index Index for the parameter to be returned.
        @return CIMParameter object requested.
        @exception IndexOutOfBoundsException Thrown if the index is outside
            the range of available parameters.
        <p><b>Example:</b>
        <pre>
            CIMParameter cp;
            Uint32 parameterIndex = m1.findParameter(CIMName ("ipaddress"));
            if (parameterIndex != PEG_NOT_FOUND)
            {
                cp = m1.getParameter(parameterIndex);
            }
        </pre>
    */
    CIMParameter getParameter(Uint32 index);

    /** Gets the parameter defined by the specified index.
        @param index Index for the parameter to be returned.
        @return CIMParameter object requested.
        @exception IndexOutOfBoundsException Thrown if the index is outside
            the range of available parameters.
        <p><b>Example:</b>
        <pre>
            CIMConstParameter cp;
            Uint32 parameterIndex = m1.findParameter(CIMName ("ipaddress"));
            if (parameterIndex != PEG_NOT_FOUND)
            {
                cp = m1.getParameter(parameterIndex);
            }
        </pre>
    */
    CIMConstParameter getParameter(Uint32 index) const;

    /** Removes the parameter defined by the specified index.
        @param index Index of the parameter to be removed.
        @exception IndexOutOfBoundsException Thrown if the index is outside the
            range of parameters available from the CIMMethod.
    */
    void removeParameter (Uint32 index);

    /** Gets the count of Parameters defined in the CIMMethod.
        @return Count of the number of parameters attached to the CIMMethod.
    */
    Uint32 getParameterCount() const;

    /** Determines if the object has not been initialized.
        @return True if the object has not been initialized;
                otherwise, false.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1;
            assert(m1.isUninitialized());
        </pre>
     */
    Boolean isUninitialized() const;

    /** Compares with a CIMConstMethod.
        @param x CIMConstMethod object for the method to be compared.
        @return True if this method is identical to the one specified;
                otherwise, false.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod m2(CIMName ("test"), CIMTYPE_STRING);
            assert(!m1.identical(m2));
        </pre>
    */
    Boolean identical(const CIMConstMethod& x) const;

    /** Makes a clone (deep copy) of this method. This creates
        a new copy of all of the components of the method including
        parameters and qualifiers.
        @return Independent copy of the CIMMethod object.
    */
    CIMMethod clone() const;

private:

    CIMMethod(CIMMethodRep* rep);

    /** This method is not implemented.  It is defined to explicitly disallow
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

/** The CIMConstMethod class is used to represent CIM methods in the
    same manner as the CIMMethod class except that the const attribute
    is applied to the objects created. This class includes equivalents
    to the methods from CIMMethod that are available in a const object,
    including constructors, accessor methods, and the destructor.

    Because the CIMMethod class uses a shared representation model, allowing
    the construction of a 'CIMMethod' from a 'const CIMMethod' would
    effectively allow modification of a 'const CIMMethod'.  The CIMConstMethod
    class is used to represent constant CIMMethod objects.  Since a
    CIMConstMethod cannot be converted to a CIMMethod, its value remains
    constant.

    @see CIMMethod()
*/
class PEGASUS_COMMON_LINKAGE CIMConstMethod
{
public:

    /** Creates a new uninitialized CIMConstMethod object.
        The only thing that can be done with this object is to copy another
        object into it.  Other methods, such as getName, will fail with an
        UninitializedObjectException.  The object has an uninitialized state,
        which can be tested with the isUninitialized method.
        @see isUninitialized()
        @see UninitializedObjectException
    */
    CIMConstMethod();

    /** Creates a new CIMConstMethod object from another CIMConstMethod object.
        The new CIMConstMethod object references the same copy of data as the
        specified object; no copy is made.
        @param x CIMConstMethod object from which to create the new
        CIMConstMethod object.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod cm1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod cm2(m1);
        </pre>
        {@link Shared Classes}
    */
    CIMConstMethod(const CIMConstMethod& x);

    /** Creates a new CIMConstMethod object from a CIMMethod object.
        The new CIMConstMethod object references the same copy of data as the
        specified object; no copy is made.
        @param x CIMMethod object from which to create the new
        CIMConstMethod object.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod cm1(m1);
        </pre>
        {@link Shared Classes}
    */
    CIMConstMethod(const CIMMethod& x);

    /** Creates a CIMConstMethod object with the specified attributes.
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
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
        </pre>
    */
    CIMConstMethod(
        const CIMName& name,
        CIMType type,
        const CIMName& classOrigin = CIMName(),
        Boolean propagated = false);

    /** Destructor for the CIMConstMethod.  The shared data copy remains valid
        until all referring objects are destructed.
        {@link Shared Classes}
    */
    ~CIMConstMethod();

    /** The assignment operator assigns one CIMConstMethod to another.
        After the assignment, both CIMConstMethod objects refer to the same
        data copy; a distinct copy is not created.
        @param x CIMConstMethod object from which to assign this
        CIMConstMethod object.

        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod m2;
            m2 = m1;
        </pre>
    */
    CIMConstMethod& operator=(const CIMConstMethod& x);

    /** The assignment operator assigns a CIMMethod object to a
        CIMConstMethod.
        After the assignment, both objects refer to the same
        data copy; a distinct copy is not created.
        @param x CIMConstMethod object from which to assign this
        CIMConstMethod object.

        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod m2;
            m2 = m1;
        </pre>
    */
    CIMConstMethod& operator=(const CIMMethod& x);

    /** Gets the name of the method.
        @return CIMName with the name of the method.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            assert(m1.getName() == CIMName ("getHostName"));
        </pre>
    */
    const CIMName& getName() const;

    /** Gets the method return type.
        @return A CIMType containing the method return type.
        @exception UninitializedObjectException Thrown if the object is not
        initialized.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            assert(m1.getType() == CIMTYPE_STRING);
        </pre>
    */
    CIMType getType() const;

    /** Gets the class in which this method was defined. This information
        is normally available with methods that are part of a schema
        returned from a CIM Server.
        @return CIMName containing the classOrigin attribute.
    */
    const CIMName& getClassOrigin() const;

    /** Tests the propagated attribute of the object.  The propagated
        attribute indicates whether this method was propagated from a
        higher-level class.  Normally this attribute is set as part of
        defining a method in the context of a schema.  It is set in
        methods retrieved from a CIM Server.
        @return True if method is propagated; otherwise, false.
    */
    Boolean getPropagated() const;

    /** Searches for a qualifier with the specified input name.
        @param name CIMName of the qualifier to be found.
        @return Zero origin index of the qualifier found or PEG_NOT_FOUND
        if not found.
        @exception UninitializedObjectException Thrown if the object is not
        initialized.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            CIMConstMethod m2(m1);
            assert(m2.findQualifier(CIMName ("stuff")) != PEG_NOT_FOUND);
        </pre>
    */
    Uint32 findQualifier(const CIMName& name) const;

    /** Gets the CIMQualifier defined by the input parameter.
        @param index Zero origin index of the qualifier requested.
        @return CIMQualifier object representing the qualifier found.
        @exception IndexOutOfBoundsException Thrown if the index is
        outside the range of parameters available from the CIMMethod.
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
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /** Returns the number of Qualifiers attached to this CIMConstMethod
        object.
        @return The number of qualifiers attached to the CIM method.
        <p><b>Example:</b>
        <pre>
            CIMMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            m1.addQualifier(CIMQualifier(CIMName ("stuff"), true));
            m1.addQualifier(CIMQualifier(CIMName ("stuff2"), true));
            CIMConstMethod m2(m1);
            assert(m2.getQualifierCount() == 2);
        </pre>
    */
    Uint32 getQualifierCount() const;

    /** Finds the parameter with the specified name.
        @param name CIMName of parameter to be found.
        @return Index of the parameter object found or PEG_NOT_FOUND
        if the property is not found.
        <p><b>Example:</b>
        <pre>
            Uint32 posParameter;
            posParameter = m1.findParameter(CIMName ("ipaddress"));
            if (posParameter != PEG_NOT_FOUND)
                ...
        </pre>
    */
    Uint32 findParameter(const CIMName& name) const;

    /** Gets the parameter defined by the specified index.
        @param index Index for the parameter to be returned.
        @return CIMConstParameter object requested.
        @exception IndexOutOfBoundsException Thrown if the index is outside
            the range of available parameters.
        <p><b>Example:</b>
        <pre>
            CIMConstParameter cp;
            Uint32 parameterIndex = m1.findParameter(CIMName ("ipaddress"));
            if (parameterIndex != PEG_NOT_FOUND)
            {
                cp = m1.getParameter(parameterIndex);
            }
        </pre>
    */
    CIMConstParameter getParameter(Uint32 index) const;

    /** Gets the count of Parameters defined in the CIMMethod.
        @return Count of the number of parameters attached to the CIMMethod.
    */
    Uint32 getParameterCount() const;

    /** Determines if the object has not been initialized.
        @return True if the object has not been initialized;
                otherwise, false.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1;
            assert(m1.isUninitialized());
        </pre>
     */
    Boolean isUninitialized() const;

    /** Compares with a CIMConstMethod.
        @param x CIMConstMethod object for the method to be compared.
        @return True if this method is identical to the one specified;
                otherwise, false.
        <p><b>Example:</b>
        <pre>
            CIMConstMethod m1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod m2(CIMName ("test"), CIMTYPE_STRING);
            assert(!m1.identical(m2));
        </pre>
    */
    Boolean identical(const CIMConstMethod& x) const;

    /** Makes a clone (deep copy) of this CIMConstMethod. This creates
        a new copy of all of the components of the method including
        parameters and qualifiers.
        @return Independent copy of the CIMConstMethod object.  Note that
        the copy is a non-constant CIMMethod.
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
