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

#ifndef Pegasus_Name_h
#define Pegasus_Name_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Exception.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMName
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMName class represents the DMTF standard CIM name definition.
    The names of CIM classes, properties, qualifiers, and methods are all
    CIM names.

    <p>A CIM name must contain characters only from this set:
    <ul>
      <li>alphabetic (a-z and A-Z)
      <li>numeric (0-9)
      <li>underscore (_)
      <li>UCS-2 characters in the range 0x0080 to 0xFFEF
    </ul>
    The first character of a CIM name may not be numeric.
    A CIMName may be null, meaning that it has no value.
*/
class PEGASUS_COMMON_LINKAGE CIMName
{
public:

    /**
        Constructs a null CIMName.
    */
    CIMName();

    /**
        Constructs a non-null CIMName with the specified name.
        @param name A String containing the CIM name.
        @exception InvalidNameException If the String does not contain a
            valid CIM name.
    */
    CIMName(const String& name);

    /**
        Constructs a non-null CIMName with the specified name.
        @param name A character string containing the CIM name.
        @exception InvalidNameException If the character string does not
            contain a valid CIM name.
        @exception All exceptions thrown by String(const char* str) can be
            thrown here
    */
    CIMName(const char* name);

    /**
        Assigns the value of the specified CIMName object to this object.
        @param name The CIMName object from which to assign this
            CIMName object.
    */
    CIMName& operator=(const CIMName& name);

    /**
        Sets the CIMName with a String name.  The resulting CIMName object
        is non-null.
        <p><b>Example:</b>
        <pre>
        CIMName n;
        String type = "type";
        n = type;
        </pre>
        @param name A String containing the CIM name to set.
        @return A reference to this CIMName object.
        @exception InvalidNameException If the String does not contain a
            valid CIM name.
    */
    CIMName& operator=(const String& name);

    /**
        Gets a String form of the CIM name.
        <p><b>Example:</b>
        <pre>
        CIMName n("name");
        String s = n.getString();
        </pre>
        @return A reference to a String containing the CIM name.
    */
    const String& getString() const;

    /**
        Determines whether the CIM name is null.
        <p><b>Example:</b>
        <pre>
        CIMName n;
        assert(n.isNull());
        n = "name";
        assert(!n.isNull());
        </pre>
        @return True if the CIM name is null, false otherwise.
    */
    Boolean isNull() const;

    /**
        Sets the CIM name to a null value.
        <p><b>Example:</b>
        <pre>
        CIMName n("name");
        n.clear();
        assert(n.isNull());
        </pre>
    */
    void clear();

    /**
        Compares the CIMName with a specified CIMName.  Comparisons of
        CIM names are case-insensitive.
        <p><b>Example:</b>
        <pre>
        CIMName n1("name");
        CIMName n2("Name");
        assert(n1.equal(n2));
        </pre>
        @param name The CIMName to be compared.
        @return True if this name is equivalent to the specified name,
            false otherwise.
    */
    Boolean equal(const CIMName& name) const;

    /**
        Determines whether a name is a valid CIM name.
        <p><b>Example:</b>
        <pre>
        assert(CIMName::legal("name"));
        assert(!CIMName::legal("3types"));
        </pre>
        @param name A String containing the name to test.
        @return True if the specified name is a valid CIM name,
            false otherwise.
    */
    static Boolean legal(const String& name);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

    /**
        Compares the CIMName with a specified character string.  Comparisons of
        CIM names are case-insensitive.
        @param name The name to be compared.
        @return True if this name is equivalent to the specified name,
            false otherwise.
    */
    Boolean equal(const char* name) const;

    /**
        Sets the CIMName with a character string name.  The resulting CIMName
        object is non-null.
        @param name A character string containing the CIM name to set.
        @return A reference to this CIMName object.
        @exception InvalidNameException If the character string does not
            contain a valid CIM name.
        @exception All exceptions thrown by String(const char* str) can be
            thrown here
    */
    CIMName& operator=(const char* name);

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

private:
    String cimName;
};

/**
    Compares two CIM names for equality.
    <p><b>Example:</b>
    <pre>
    CIMName lowerCaseName("this_is_a_name");
    CIMName upperCaseName("THIS_IS_A_NAME");
    assert(lowerCaseName == upperCaseName);
    </pre>
    @param x The first CIMName to be compared.
    @param y The second CIMName to be compared.
    @return True if the CIM names are equivalent, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMName& name1,
    const CIMName& name2);

/**
    Compares two CIM names for inequality.
    @param x The first CIMName to be compared.
    @param y The second CIMName to be compared.
    @return False if the CIM names are equivalent, true otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(
    const CIMName& name1,
    const CIMName& name2);

#define PEGASUS_ARRAY_T CIMName
# include "ArrayInter.h"
#undef PEGASUS_ARRAY_T


////////////////////////////////////////////////////////////////////////////////
//
// CIMNamespaceName
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMNamespaceName class represents the DMTF standard CIM namespace
    name definition.

    <p>A CIM namespace name must match the following expression:
    <pre>
        &lt;CIMName&gt;[ / &lt;CIMName&gt; ]*
    </pre>
    A namespace name with a leading '/' character is accepted, but the
    leading character is removed.
    A CIMNamespaceName may be null, meaning that it has no value.
*/
class PEGASUS_COMMON_LINKAGE CIMNamespaceName
{
public:

    /**
        Constructs a null CIMName.
    */
    CIMNamespaceName();

    /**
        Constructs a non-null CIMNamespaceName with the specified name.
        @param name A String containing the CIM namespace name.
        @exception InvalidNameException If the String does not contain a
            valid CIM namespace name.
    */
    CIMNamespaceName(const String& name);

    /**
        Constructs a non-null CIMNamespaceName with the specified name.
        @param name A character string containing the CIM namespace name.
        @exception InvalidNameException If the character string does not
            contain a valid CIM namespace name.
        @exception All exceptions thrown by String(const char* str) can be
            thrown here
    */
    CIMNamespaceName(const char* name);

    /**
        Assigns the value of the specified CIMNamespaceName object to this
        object.
        @param name The CIMNamespaceName object from which to assign this
            CIMNamespaceName object.
    */
    CIMNamespaceName& operator=(const CIMNamespaceName& name);

    /**
        Sets the CIMNamespaceName with a String name.  The resulting
        CIMNamespaceName object is non-null.
        <p><b>Example:</b>
        <pre>
        CIMNamespaceName n;
        String name = "root/cimv2";
        n = name;
        </pre>
        @param name A String containing the CIM namespace name to set.
        @return A reference to this CIMNamespaceName object.
        @exception InvalidNameException If the String does not contain a
            valid CIM namespace name.
    */
    CIMNamespaceName& operator=(const String& name);

    /**
        Gets a String form of the CIM namespace name.
        <p><b>Example:</b>
        <pre>
        CIMNamespaceName n("test/ns1");
        String s = n.getString();
        </pre>
        @return A reference to a String containing the CIM namespace name.
    */
    const String& getString() const;

    /**
        Determines whether the CIM namespace name is null.
        <p><b>Example:</b>
        <pre>
        CIMNamespaceName n;
        assert(n.isNull());
        n = "root/test";
        assert(!n.isNull());
        </pre>
        @return True if the CIM namespace name is null, false otherwise.
    */
    Boolean isNull() const;

    /**
        Sets the CIM namespace name to a null value.
        <p><b>Example:</b>
        <pre>
        CIMNamespaceName n("root/test");
        n.clear();
        assert(n.isNull());
        </pre>
    */
    void clear();

    /**
        Compares the CIMNamespaceName with a specified CIMNamespaceName.
        Comparisons of CIM namespace names are case-insensitive.
        <p><b>Example:</b>
        <pre>
        CIMNamespaceName n1("root/cimv2");
        CIMNamespaceName n2("Root/CimV2");
        assert(n1.equal(n2));
        </pre>
        @param name The CIMNamespaceName to be compared.
        @return True if this name is equivalent to the specified name,
            false otherwise.
    */
    Boolean equal(const CIMNamespaceName& name) const;

    /**
        Determines whether a name is a valid CIM namespace name.
        <p><b>Example:</b>
        <pre>
        assert(CIMNamespaceName::legal("root/test"));
        assert(!CIMNamespaceName::legal("Wrong!"));
        </pre>
        @param name A String containing the name to test.
        @return True if the specified name is a valid CIM namespace name,
            false otherwise.
    */
    static Boolean legal(const String& name);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

    /**
        Compares the CIMNamespaceName with a specified character string.
        Comparisons of CIM namespace names are case-insensitive.
        @param name The name to be compared.
        @return True if this name is equivalent to the specified name,
            false otherwise.
    */
    Boolean equal(const char* name) const;

    /**
        Sets the CIMNamespaceName with a character string name.  The
        resulting CIMNamespaceName object is non-null.
        @param name A character string containing the CIM namespace name
            to set.
        @return A reference to this CIMNamespaceName object.
        @exception InvalidNameException If the character string does not
            contain a valid CIM namespace name.
        @exception All exceptions thrown by String(const char* str) can be
            thrown here
    */
    CIMNamespaceName& operator=(const char* name);

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACES */

private:
    String cimNamespaceName;
};

/**
    Compares two CIM namespace names for equality.
    <p><b>Example:</b>
    <pre>
    CIMNamespaceName n1("root/test");
    CIMNamespaceName n2("Root/TEST");
    assert(n1 == n2);
    </pre>
    @param x The first CIMNamespaceName to be compared.
    @param y The second CIMNamespaceName to be compared.
    @return True if the CIM namespace names are equivalent, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMNamespaceName& name1,
    const CIMNamespaceName& name2);

/**
    Compares two CIM namespace names for inequality.
    @param x The first CIMNamespaceName to be compared.
    @param y The second CIMNamespaceName to be compared.
    @return False if the CIM namespace names are equivalent, true otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(
    const CIMNamespaceName& name1,
    const CIMNamespaceName& name2);

#define PEGASUS_ARRAY_T CIMNamespaceName
# include "ArrayInter.h"
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#ifdef PEGASUS_INTERNALONLY
# include "CIMNameInline.h"
#endif

#endif /* Pegasus_Name_h */
